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

#include "atlink/core/Urc.h"
#include "atlink/core/fsm/Commands.h"
#include "atlink/core/fsm/Context.h"
#include "atlink/core/fsm/Events.h"
#include "atlink/platform/Facade.h"
#include "atlink/utils/Deserializer.h"
#include "atlink/utils/Overload.h"
#include "atlink/utils/Serializer.h"

#include "atlink/core/fsm/State.h"

namespace ATL_NS {
namespace Core {
namespace Fsm {

class Orchestrator : public Context, public Platform::Api::Subscriber {
    static constexpr Platform::Timer::Duration coolDownPeriod = std::chrono::milliseconds{20};

    Platform::DeviceIO &deviceIO;
    AUrcDispatcher &urcDispatcher;
    Platform::Mutex mtx{};
    Platform::CondVar condvar;
    State::Variant state{State::Idle{this}};

    Platform::MessageQueue<Fsm::Event> events{};
    Platform::Timer coolDown{};
    Platform::Logger logger{"orchestrator"};

    std::array<char, 512U> rxstorage{};
    MutableBuffer rxbuf{rxstorage};
    size_t leftover{0U};

    std::array<char, 512U> txstorage{};
    MutableBuffer txbuf{txstorage};

  public:
    void notify(Platform::Api::Subscriber::Event ev) {
        if (Platform::Api::Subscriber::Event::RxReady == ev) {
            events.put(Fsm::Event::RxReady);
        }
    }

    static void timerCallback(void *ctx) {
        auto *o = static_cast<Orchestrator *>(ctx);
        o->events.put(Fsm::Event::TxReady);
    }

    explicit Orchestrator(Platform::DeviceIO &io, AUrcDispatcher &udp)
        : deviceIO{io}, urcDispatcher{udp} {
        deviceIO.subscribe(*this);
        coolDown.setHandler(timerCallback, this);
        logger.setLogLevel(Platform::Api::Log::Level::Trace);
    }

    void loop() {
        while (true) {
            auto ev = events.get();
            if (Fsm::Event::ShutDown == ev) {
                logger.info() << "Shutting down";
                break;
            } else {
                handle(ev);
            }
        }
    }

    void handle(Fsm::Event event) {
        auto handlers = Utils::Overload{
            [&](State::Idle &idle) -> State::Variant {
                return idle.handle(event);
            },
            [&](State::SendCommand &s) -> State::Variant {
                return s.handle(event);
            },
            [&](State::WaitForResponse &w) -> State::Variant {
                return w.handle(event);
            },
        };

        Platform::Mutex::LockGuard g{mtx};

        const bool wasIdle = std::holds_alternative<State::Idle>(state);

        auto next = std::visit(handlers, state);
        state = std::move(next);

        const bool nowIdle = std::holds_alternative<State::Idle>(state);

        if (!wasIdle && nowIdle) {
            logger.info() << "FSM entered idle state";
            condvar.notifyAll();
        }
    }

    bool send(const ACommand &out) override {
        auto serializer = Utils::Serializer{txbuf};
        auto success = out.accept(serializer);
        if (success) {
            auto len = serializer.numberOfBytesWritten();
            auto n = deviceIO.write(serializer.output());

            logger.info() << "TX: command sent (" << len << " bytes)";

            success = (n == len);
            if (!success) {
                logger.error() << "TX: write failed (" << n << "/" << len << " bytes)";
            } else {
                logger.debug() << "TX: write ok (" << n << " bytes)";
            }
        } else {
            logger.error() << "TX: serialization failed";
        }
        return success;
    }

    bool receive(AResponsePack &frc, AResponse *in) override {

        auto n = deviceIO.read(rxbuf.subspan(leftover));
        auto input = ReadOnlyText{rxbuf.data(), leftover + n};

        auto tryResponse = [](AResponse *res, ReadOnlyText &txt) -> bool {
            if (res == nullptr) {
                return false;
            }
            Utils::Deserializer deserializer{txt};
            const bool success = res->accept(deserializer);
            if (success) {
                txt = txt.substr(deserializer.numberOfBytesConsumed());
            }
            return success;
        };

        auto tryResult = [](AResponsePack &frc, ReadOnlyText &txt) -> bool {
            Utils::Deserializer deserializer{txt};
            const bool success = frc.accept(deserializer);
            if (success) {
                txt = txt.substr(deserializer.numberOfBytesConsumed());
            }
            return success;
        };

        auto trySingleUrc = [&](ReadOnlyText &txt) -> bool {
            const auto consumed = dispatchSingleUrc(txt);
            if (consumed > 0U) {
                txt = txt.substr(consumed);
                return true;
            }
            return false;
        };

        bool haveResponse = (in == nullptr);
        bool haveResult = false;

        if (!haveResponse) {
            haveResponse = tryResponse(in, input);
        }

        while (true) {
            const auto before = input.size();

            if (!haveResponse) {
                if (tryResponse(in, input)) {
                    haveResponse = true;
                    continue;
                }
            }

            if (!haveResult) {
                if (tryResult(frc, input)) {
                    haveResult = true;
                    continue;
                }
            }

            if (trySingleUrc(input)) {
                continue;
            }

            const auto after = input.size();
            if (after == before) {
                break;
            }
        }

        for (size_t i = 0; i < input.size(); ++i) {
            rxbuf[i] = input[i];
        }

        leftover = input.size();

        if (!haveResult || !haveResponse) {
            logger.trace() << "RX: incomplete response, waiting (buffered=" << leftover
                           << " bytes)";
        }

        return haveResult && haveResponse;
    }

    bool canSend() override {
        return !coolDown.isRunning();
    }

    void dispatchUrcs() override {

        auto n = deviceIO.read(rxbuf.subspan(leftover));
        auto input = ReadOnlyText{rxbuf.data(), leftover + n};

        auto consumed = dispatchAllUrcs(input);
        input = input.substr(consumed);

        for (size_t i = 0; i < input.size(); ++i) {
            rxbuf[i] = input[i];
        }

        leftover = input.size();
    }

    size_t dispatchAllUrcs(ReadOnlyText input) {
        auto n = dispatchSingleUrc(input);
        auto consumed = n;

        while (0U < n) {
            input = input.substr(n);
            n = dispatchSingleUrc(input);
            consumed += n;
        }

        if (0U < input.size()) {
            logger.debug() << "URC: trailing partial data (" << input.size() << " bytes buffered)";
        }

        return consumed;
    }

    std::size_t dispatchSingleUrc(ReadOnlyText input) {
        return urcDispatcher.dispatch(input);
    }

    void shutDown() {
        events.put(Fsm::Event::ShutDown);
    }

    ErrorCode sendCommand(AResponsePack *result, ACommand *cmd, AResponse *res) {
        mtx.lock();
        while (!std::holds_alternative<State::Idle>(state)) {
            condvar.wait(mtx);
        }

        ErrorCode ec{ErrorCode::NoError};
        Platform::Semaphore sem{};

        logger.info() << "FSM: sending command";

        Command::SendCommand payload{};
        payload.ec = &ec;
        payload.result = result;
        payload.command = cmd;
        payload.response = res;
        payload.sem = &sem;

        if (auto idle = std::get_if<State::Idle>(&state)) {
            logger.debug() << "FSM: idle → sendcommand";
            auto next = idle->process(payload);
            state = std::move(next);
        } else {
            logger.error() << "FSM: not idle after gating — aborting";
            ec = ErrorCode::InternalError;
        }

        mtx.unlock();

        if (ErrorCode::NoError == ec) {
            sem.acquire();
            logger.info() << "FSM: command completed";
        } else {
            logger.error() << "FSM: command failed (" << static_cast<int>(ec) << ")";
        }

        return ec;
    }
};

} // namespace Fsm
} // namespace Core
} // namespace ATL_NS
