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

#include "atlink/core/Types.h"
#include <charconv>
#include <cstddef>
#include <type_traits>

namespace ATL_NS {
namespace Platform {
namespace Api {

namespace Log {
enum class Level : unsigned char {
    Error = 0,
    Warn,
    Info,
    Debug,
    Trace,
};

} // namespace Log

template <typename Backend>
class Logger {

  public:
    template <class B>
    using expr_setLogLevel = decltype(std::declval<B &>().setLogLevel(std::declval<Log::Level>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_setLogLevel, Backend>,
                  "Logger Backend must provide: 'void setLogLevel(Log::Level)'");

    template <class B>
    using expr_wouldLog = decltype(std::declval<const B &>().wouldLog(std::declval<Log::Level>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<bool, expr_wouldLog, const Backend>,
                  "Logger Backend must provide: 'bool wouldLog(Log::Level) const'");

    template <class B>
    using expr_log = decltype(std::declval<B &>().log(std::declval<Log::Level>(),
                                                      std::declval<const char *>(),
                                                      std::declval<const char *>(),
                                                      std::declval<std::size_t>()));
    static_assert(
        ATL_NS::Utils::is_detected_exact_v<void, expr_log, Backend>,
        "Logger Backend must provide: 'void log(Log::Level, const char*, const char*, size_t)'");

    explicit constexpr Logger(const char *name) : name(name), impl() {}

    void setLogLevel(Log::Level level) {
        impl.setLogLevel(level);
    }

    bool wouldLog(Log::Level level) const {
        return impl.wouldLog(level);
    }

    class Line {
        static constexpr std::size_t SIZE = 256U;

        Backend &backend;
        const char *name;
        Log::Level level;
        bool enabled;

        std::array<char, SIZE> buf{};
        std::size_t len{0U};
        bool flushed{false};

      public:
        Line(Backend &backend, const char *name, Log::Level level)
            : backend(backend), name(name), level(level), enabled(backend.wouldLog(level)) {}

        ~Line() {
            flush();
        }

        void flush() {
            if (enabled && (!flushed)) {
                backend.log(level, name, buf.data(), len);
                flushed = true;
            }
        }

        Line &operator<<(ATL_NS::Core::ReadOnlyText str) {
            if (enabled) {
                const auto n = (str.size() <= available()) ? str.size() : available();
                for (std::size_t i = 0U; i < n; ++i) {
                    if ('\r' == str[i]) {
                        len += snprintf(buf.data() + len, 5U, "<CR>");
                    } else if ('\n' == str[i]) {
                        len += snprintf(buf.data() + len, 5U, "<LF>");
                    } else {
                        buf[len++] = str[i];
                    }
                }
            }
            return *this;
        }
        Line &operator<<(ATL_NS::Core::MutableBuffer b) {
            if (enabled) {
                const auto n = (b.size() <= available()) ? b.size() : available();
                for (std::size_t i = 0; i < n; ++i)
                    buf[len + i] = b[i];
                len += n;
            }
            return *this;
        }

        Line &operator<<(const ATL_NS::Core::AEnum &e) {
            if (enabled) {
                Core::MutableBuffer rem{buf.data() + len, SIZE - len};
                const auto n = e.stringify(rem);
                if (n < rem.size()) {
                    len += n;
                }
            }
            return *this;
        }

        Line &operator<<(const char *s) {
            if (enabled && (nullptr != s)) {
                while (*s && (len < SIZE))
                    buf[len++] = *s++;
            }
            return *this;
        }

        Line &operator<<(char c) {
            if (enabled && (len < SIZE)) {
                buf[len++] = c;
            }
            return *this;
        }

        Line &operator<<(bool v) {
            return (*this) << (v ? "true" : "false");
        }

        // Arithmetic (integral & floating) via to_chars
        template <
            class T,
            typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value,
                                    int>::type = 0>
        Line &operator<<(T v) {
            if (enabled && (0U < available())) {
                char *first = buf.data() + len;
                char *last = buf.data() + SIZE;
                auto rc = std::to_chars(first, last, v);
                if (rc.ec == std::errc{}) {
                    len += static_cast<std::size_t>(rc.ptr - first);
                }
            }
            return *this;
        }

      private:
        std::size_t available() const {
            return SIZE - len;
        }
    };

    Line error() {
        return Line{impl, name, Log::Level::Error};
    }
    Line warn() {
        return Line{impl, name, Log::Level::Warn};
    }
    Line info() {
        return Line{impl, name, Log::Level::Info};
    }
    Line debug() {
        return Line{impl, name, Log::Level::Debug};
    }
    Line trace() {
        return Line{impl, name, Log::Level::Trace};
    }

  private:
    const char *name;
    Backend impl;
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS