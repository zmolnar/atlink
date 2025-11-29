//
//  This file is part of ATLink.
//
//  ATLink is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ATLink is distributed in the hope that it will backend useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ATLink.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include "atlink/platform/api/DeviceIO.h"

#include "atlink/platform/api/Logger.h"
#include "atlink/platform/linux/Logger.h"

#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <gsl/span>
#include <poll.h>
#include <string_view>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class DeviceIO {
  public:
    DeviceIO();
    ~DeviceIO();
    using Subscriber = ATL_NS::Platform::Api::Subscriber;
    void subscribe(Subscriber &l);

    size_t write(std::string_view s);
    size_t read(gsl::span<char> buf);

  private:
    int fd{-1};
    std::thread poller;
    std::atomic<bool> run{false};
    std::atomic<Subscriber *> subscriber{nullptr};

    Api::Logger<Linux::Logger> logger;

    int openAndConfigureTty();
    void print(const char *prefix, const std::string_view str);

    void pollLoop();
    void notifyRx();

    // Disallow copy
    DeviceIO(const DeviceIO &) = delete;
    DeviceIO &operator=(const DeviceIO &) = delete;
};

inline DeviceIO::DeviceIO() : logger{"deviceio"} {
    fd = openAndConfigureTty();
    if (fd >= 0) {
        run = true;
        poller = std::thread(&DeviceIO::pollLoop, this);
        logger.setLogLevel(Api::Log::Level::Trace);
        logger.info() << "poller thread started";
    } else {
        logger.error() << "DeviceIO initialization failed";
    }
}

inline DeviceIO::~DeviceIO() {
    run = false;
    if (poller.joinable())
        poller.join();
    if (fd >= 0)
        ::close(fd);
    logger.info() << "device closed";
}

inline void DeviceIO::subscribe(Subscriber &s) {
    subscriber.store(&s, std::memory_order_release);
    logger.debug() << "subscriber registered";
}

inline size_t DeviceIO::write(std::string_view s) {
    if (fd < 0 || s.empty())
        return 0;

    print("deviceio-tx", s);

    ssize_t n = ::write(fd, s.data(), s.size());
    if (n < 0) {
        logger.error() << "write failed: " << strerror(errno);
        return 0;
    }

    logger.trace() << "tx complete (" << n << " bytes)";
    return static_cast<size_t>(n);
}

inline size_t DeviceIO::read(gsl::span<char> buf) {
    if (fd < 0 || buf.empty())
        return 0;

    // Use ssize_t to capture -1 correctly.
    ssize_t r = ::read(fd, buf.data(), buf.size());
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now
            return 0;
        }
        logger.error() << "read failed: " << strerror(errno);
        return 0;
    }

    const size_t len = static_cast<size_t>(r);

    // Log rx contents for debugging/trace
    print("deviceio-rx", {buf.data(), len});

    logger.trace() << "rx read " << len << " bytes";
    return len;
}

inline void DeviceIO::notifyRx() {
    if (auto *sub = subscriber.load(std::memory_order_acquire)) {
        sub->notify(DeviceIO::Subscriber::Event::RxReady);
    }
}

inline int DeviceIO::openAndConfigureTty() {
    const char *path = std::getenv("ATLINK_TTY");
    if (!path) {
        path = "/dev/ttyUSB0";
        logger.warn() << "ATLINK_TTY not set; defaulting to /dev/ttyUSB0";
    }

    int fd = ::open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        logger.error() << "open(\"" << path << "\") failed: " << strerror(errno);
        return -1;
    }

    termios tio{};
    if (tcgetattr(fd, &tio) != 0) {
        logger.error() << "tcgetattr failed: " << strerror(errno);
        ::close(fd);
        return -1;
    }

    cfmakeraw(&tio);
    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);

    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;

    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        logger.error() << "tcsetattr failed: " << strerror(errno);
        ::close(fd);
        return -1;
    }

    logger.info() << "TTY opened and configured (" << path << ")";
    return fd;
}

inline void DeviceIO::print(const char *prefix, const std::string_view str) {

    // Raw summary (length)
    logger.trace() << prefix << ": len=" << str.size();

    if (str.empty()) {
        return;
    }

    // Build an escaped, printable representation: <CR>, <LF>, other chars copied.
    std::string out;
    out.reserve(str.size() * 2 + 16);
    out.append("data=");

    for (unsigned char c : str) {
        if (c == '\r')
            out.append("<CR>");
        else if (c == '\n')
            out.append("<LF>");
        else if (c >= 32 && c < 127)
            out.push_back(static_cast<char>(c));
        else {
            // Non-printable: hex escape
            char buf[8];
            int n = snprintf(buf, sizeof(buf), "<0x%02X>", c);
            if (n > 0)
                out.append(buf, static_cast<size_t>(n));
        }
    }

    logger.debug() << out.c_str();
}

inline void DeviceIO::pollLoop() {
    if (fd < 0)
        return;

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    while (run.load(std::memory_order_relaxed)) {
        pfd.events = POLLIN;
        pfd.revents = 0;

        int rc = ::poll(&pfd, 1, 100); // 100ms tick
        if (rc <= 0) {
            // timeout or interrupted, continue
            continue;
        }

        if (pfd.revents & POLLIN) {
            logger.trace() << "poll: POLLIN";
            notifyRx();
        }
    }
}

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS
