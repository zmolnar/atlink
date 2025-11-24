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

#pragma once

#include "atlink/platform/api/Logger.h"

#include <atomic>
#include <cstdio>
#include <ctime>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class Logger {
  public:
    using Level = typename ATL_NS::Platform::Api::Log::Level;

    Logger() : maxLevel(static_cast<unsigned>(Level::Info)) {}

    void setLogLevel(Level lvl) {
        maxLevel.store(static_cast<unsigned>(lvl), std::memory_order_release);
    }
    bool wouldLog(Level lvl) const {
        return static_cast<unsigned>(lvl) <= maxLevel.load(std::memory_order_acquire);
    }

    void log(Level lvl, const char *name, const char *data, std::size_t len) {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
        localtime_r(&t, &tm);

        char ts[16];
        std::snprintf(ts, sizeof(ts), "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

        const char *lv = levelName(lvl);

        // Select color based on severity
        static constexpr const char *COLOR_RED = "\033[31m";
        static constexpr const char *COLOR_YELLOW = "\033[33m";
        static constexpr const char *COLOR_RESET = "\033[0m";

        const char *color = "";
        const char *reset = "";

        switch (lvl) {
        case Level::Error:
            color = COLOR_RED;
            reset = COLOR_RESET;
            break;
        case Level::Warn:
            color = COLOR_YELLOW;
            reset = COLOR_RESET;
            break;
        default:
            break;
        }

        // Print header with color
        std::fprintf(stderr, "%s[%s] %-5s", color, ts, lv);
        if (name && *name) {
            std::fprintf(stderr, " [%s]", name);
        }
        std::fputc(' ', stderr);

        // Print message body
        if (data && len) {
            std::fwrite(data, 1, len, stderr);
        }

        // Reset color after line
        if (*color) {
            std::fprintf(stderr, "%s", reset);
        }

        std::fputc('\n', stderr);
        std::fflush(stderr);
    }

  private:
    static const char *levelName(Level lvl) {
        switch (lvl) {
        case Level::Error:
            return "ERROR";
        case Level::Warn:
            return "WARN";
        case Level::Info:
            return "INFO";
        case Level::Debug:
            return "DEBUG";
        case Level::Trace:
            return "TRACE";
        }
        return "LOG";
    }

    std::atomic<unsigned> maxLevel;
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS