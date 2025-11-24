//
//  This file is part of ATLink.
//
//  ATLink is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ATLink is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ATLink.  If not, see <https://www.gnu.org/licenses/>.
//

#include "atlink/core/Device.h"
#include "atlink/core/FinalResultCode.h"
#include "atlink/platform/Facade.h"
#include "atlink/protocols/standard/At.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <glob.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <thread>
#include <vector>

namespace {

std::atomic<bool> g_stop{false};

void sigintHandler(int) {
    g_stop.store(true, std::memory_order_relaxed);
}

std::vector<std::string> listSerialPorts() {
    std::vector<std::string> ports;

#if defined(__APPLE__)
    const char *patterns[] = {"/dev/tty.*", "/dev/cu.*"};
#else
    const char *patterns[] = {"/dev/ttyUSB*", "/dev/ttyACM*", "/dev/ttyS*"};
#endif

    for (const char *pattern : patterns) {
        glob_t g{};
        if (::glob(pattern, 0, nullptr, &g) == 0) {
            for (size_t i = 0; i < g.gl_pathc; ++i) {
                if (g.gl_pathv[i]) {
                    ports.emplace_back(g.gl_pathv[i]);
                }
            }
        }
        globfree(&g);
    }

    std::sort(ports.begin(), ports.end());
    ports.erase(std::unique(ports.begin(), ports.end()), ports.end());
    return ports;
}

int askUserToChoose(const std::vector<std::string> &items, std::ostream &os, std::istream &is) {
    if (items.empty())
        return -1;

    os << "Available serial ports:\n";
    for (size_t i = 0; i < items.size(); ++i) {
        os << "  [" << i << "] " << items[i] << "\n";
    }
    os << "Select index (Enter = 0): " << std::flush;

    std::string line;
    if (!std::getline(is, line) || line.empty())
        return 0;

    try {
        long idx = std::stol(line);
        if (idx < 0 || static_cast<size_t>(idx) >= items.size())
            return -1;
        return static_cast<int>(idx);
    } catch (...) {
        return -1;
    }
}

class UrcDispatcher : public ATL_NS::Core::AUrcDispatcher {
  public:
    size_t dispatch(ATL_NS::Core::ReadOnlyText str) override {
        return 0U;
    }
};

} // namespace

int main() {
    // Install Ctrl-C handler
    ::signal(SIGINT, sigintHandler);

    ATL_NS::Platform::Logger logger{"main"};
    logger.setLogLevel(ATL_NS::Platform::Api::Log::Level::Trace);
    logger.info() << "Atlink demo app started ...";

    logger.error() << "error message";
    logger.warn() << "warning message";
    logger.info() << "info message";
    logger.debug() << "debug message";
    logger.trace() << "trace message";

    auto ports = listSerialPorts();
    if (ports.empty()) {
        std::cerr << "No serial ports found. Set ATLINK_TTY env var or connect a device.\n";
        return 1;
    }

    int choice = askUserToChoose(ports, std::cout, std::cin);
    if (choice < 0) {
        std::cerr << "Invalid choice.\n";
        return 1;
    }

    const std::string &tty = ports[static_cast<size_t>(choice)];
    ::setenv("ATLINK_TTY", tty.c_str(), 1);
    std::cout << "Using port: " << tty << "\n";

    // DeviceIO backend is expected to read ATLINK_TTY
    auto deviceIO = atlink::Platform::DeviceIO{};
    auto urcDispatcher = UrcDispatcher{};
    auto device = atlink::Core::Device{"mc60", deviceIO, urcDispatcher};

    // FSM / device loop thread
    std::thread deviceThread([&device] {
        device.loop(); // likely runs forever
    });

    // Periodic AT sender thread
    std::thread atThread([&device, &logger] {
        using ATL_NS::Core::FinalResultCode;
        using ATL_NS::Proto::Std::At::Write::Command;

        while (!g_stop.load(std::memory_order_relaxed)) {
            Command cmd{};
            FinalResultCode<> frc{};

            bool ok = device.sendCommand(&frc, &cmd, nullptr);
            if (!ok) {
                logger.error() << "AT command failed (sendCommand error)";
            } else {
                // Inspect final result code if you want to log more detail
                logger.info() << "AT command succeeded";
            }

            for (int i = 0; i < 10 && !g_stop.load(std::memory_order_relaxed); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        logger.trace() << "Stop the device...";
        device.shutDown();
        logger.info() << "AT sender thread stopping...";
    });

    std::cout << "Sending 'AT' every second. Press Ctrl-C to stop.\n";

    // Wait for Ctrl-C
    while (!g_stop.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    logger.info() << "Ctrl-C received, shutting down...";

    if (atThread.joinable()) {
        atThread.join();
    }

    // device.loop() has no stop mechanism, so we just detach and let process exit
    if (deviceThread.joinable()) {
        deviceThread.detach();
    }

    return 0;
}