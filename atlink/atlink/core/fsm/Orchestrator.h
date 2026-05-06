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
#include "atlink/core/fsm/AtTransactionFsm.h"
#include "atlink/platform/Facade.h"
#include "atlink/utils/Deserializer.h"
#include "atlink/utils/Overload.h"
#include "atlink/utils/Serializer.h"

#include <array>
#include <cstring>

namespace ATL_NS {
namespace Core {
namespace Fsm {

class Orchestrator : public Platform::Api::Subscriber {
    static constexpr Platform::Timer::Duration coolDownPeriod = std::chrono::milliseconds{20};

    Platform::DeviceIO &deviceIO;
    AUrcDispatcher &urcDispatcher;

    Platform::Mutex mtx{};
    Platform::CondVar condvar{};
    Platform::MessageQueue<EventVariant> events{};
    Platform::Timer coolDown{};
    Platform::Logger logger{"orchestrator"};
    bool canSubmitCommand{true};
    bool stopping{false};

    AtTransactionFsm fsm{std::in_place_type<State::Ready>};

    std::array<char, 512U> rxstorage{};
    MutableBuffer rxbuf{rxstorage};

    std::array<char, 512U> txstorage{};
    MutableBuffer txbuf{txstorage};

  public:
    Orchestrator(Platform::DeviceIO &io, AUrcDispatcher &urc) : deviceIO{io}, urcDispatcher{urc} {
        deviceIO.subscribe(*this);
        coolDown.setHandler(timerCallback, this);
        logger.setLogLevel(Platform::Api::Log::Level::Trace);
    }

    void notify(Platform::Api::Subscriber::Event ev) override {
        if (ev == Platform::Api::Subscriber::Event::RxReady) {
            post(EventVariant{Fsm::Event::RxReady{}});
        }
    }

    static void timerCallback(void *ctx) {
        auto *self = static_cast<Orchestrator *>(ctx);
        self->post(EventVariant{Fsm::Event::TxWindowOpen{}});
    }

    void loop() {
        while (!isStopping()) {
            auto ev = events.get();
            dispatch(ev);
        }
        logger.info() << "FSM: shutdown entered";
    }

    void stop() {
        post(EventVariant{Fsm::Event::StopRequested{}});
    }

    void shutDown() {
        stop();
    }

    ErrorCode sendCommand(AResponsePack *result, Core::Command *cmd, Response *res) {
        return sendCommandInternal(result, cmd, res);
    }

  private:
    ErrorCode sendCommandInternal(AResponsePack *result, Core::Command *cmd, Response *res) {
        PendingExchange req{};
        req.result = result;
        req.command = cmd;
        req.response = res;

        ErrorCode ec{ErrorCode::NoError};
        Platform::Semaphore sem{};

        req.ec = &ec;
        req.sem = &sem;

        {
            Platform::Mutex::LockGuard g{mtx};

            while (!canSubmitCommand && !stopping) {
                condvar.wait(mtx);
            }

            if (stopping) {
                ec = ErrorCode::DeviceUnavailable;
            } else {
                if (canSubmitCommand) {
                    post(EventVariant{Fsm::Event::RequestSubmitted{req}});
                }
            }
        }

        if (ErrorCode::NoError == ec) {
            sem.acquire();
        }

        return ec;
    }
    void post(EventVariant ev) {
        events.put(std::move(ev));
    }

    void dispatch(EventVariant const &ev) {
        std::visit(
            [&](auto const &concreteEv) {
                auto actions = fsm.dispatch(concreteEv);
                execute(actions);
            },
            ev);
    }

    void execute(AtTransactionFsm::actions_t const &actions) {
        for (auto const &action : actions) {
            std::visit(Utils::Overload{
                           [&](Action::SendCommand const &act) {
                               executeSendCommand(act);
                           },
                           [&](Action::FinalizeExchange const &act) {
                               executeFinalizeExchange(act);
                           },
                           [&](Action::StartTxWindowTimer const &) {
                               executeStartTxWindowTimer();
                           },
                           [&](Action::WakeCommandWaiters const &) {
                               wakeCommandWaiters();
                           },
                           [&](Action::BlockCommandSubmission const &) {
                               blockCommandSubmission();
                           },
                           [&](Action::HandleUrcs const &) {
                               handleUrcs();
                           },
                           [&](Action::HandleInput const &act) {
                               handleInput(act);
                           },
                           [&](Action::SignalShutdownEntered const &) {
                               handleShutdownEntered();
                           },
                       },
                       action);
        }
    }

    void handleShutdownEntered() {
        {
            Platform::Mutex::LockGuard g{mtx};
            stopping = true;
            canSubmitCommand = false;
        }
        condvar.notifyAll();
    }

    void handleUrcs() {
        dispatchUrcs();
    }

    void handleInput(Action::HandleInput const &act) {
        if (nullptr == act.req.result) {
            post(EventVariant{Fsm::Event::ReplyFailed{ErrorCode::InternalError}});
            return;
        }

        if (receive(*act.req.result, act.req.response)) {
            post(EventVariant{Fsm::Event::ReplyComplete{}});
        }
    }

    void executeSendCommand(Action::SendCommand const &act) {
        if (!send(*act.req.command)) {
            post(EventVariant{Fsm::Event::ReplyFailed{ErrorCode::InternalError}});
        }
    }

    void executeFinalizeExchange(Action::FinalizeExchange const &act) {
        if (act.req.ec != nullptr) {
            *act.req.ec = act.ec;
        }

        if (act.req.sem != nullptr) {
            act.req.sem->release();
        }
    }

    void executeStartTxWindowTimer() {
        coolDown.start(coolDownPeriod);
    }

    bool send(const Core::Command &out) {
        auto serializer = Utils::Serializer{txbuf};
        auto success = out.accept(serializer);
        if (success) {
            auto len = serializer.written();
            auto n = deviceIO.write(serializer.output());

            success = (n == len);
            if (!success) {
                logger.error() << "TX: write failed (" << n << "/" << len << " bytes)";
            }
        } else {
            logger.error() << "TX: serialization failed";
        }
        return success;
    }

    bool receive(AResponsePack &frc, Response *in) {
        auto n = readIntoRxBuffer();
        auto input = currentRxInput(n);

        auto tryResponse = [](Response *res, ReadOnlyText &txt) -> bool {
            if (res == nullptr) {
                return false;
            }
            Utils::Deserializer deserializer{txt};
            const bool success = res->accept(deserializer);
            if (success) {
                txt = txt.substr(deserializer.consumed());
            }
            return success;
        };

        auto tryResult = [](AResponsePack &frc, ReadOnlyText &txt) -> bool {
            Utils::Deserializer deserializer{txt};
            const bool success = frc.accept(deserializer);
            if (success) {
                txt = txt.substr(deserializer.consumed());
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

        commitRxRemainder(input);

        return haveResult && haveResponse;
    }

    void dispatchUrcs() {
        auto n = readIntoRxBuffer();
        auto input = currentRxInput(n);

        auto consumed = dispatchAllUrcs(input);
        input = input.substr(consumed);

        commitRxRemainder(input);
    }

    std::size_t readIntoRxBuffer() {
        if (rxbuf.empty()) {
            return 0U;
        }

        const auto n = deviceIO.read(rxbuf);
        rxbuf = rxbuf.subspan(n);
        return n;
    }

    ReadOnlyText currentRxInput(std::size_t /*newlyRead*/) const {
        return ReadOnlyText{rxstorage.data(), rxstorage.size() - rxbuf.size()};
    }

    void commitRxRemainder(ReadOnlyText remainder) {
        const auto used = remainder.size();
        if (used > 0U && remainder.data() != rxstorage.data()) {
            std::memmove(rxstorage.data(), remainder.data(), used);
        }
        rxbuf = MutableBuffer{rxstorage}.subspan(used);
    }

    std::size_t dispatchAllUrcs(ReadOnlyText input) {
        auto n = dispatchSingleUrc(input);
        auto consumed = n;

        while (0U < n) {
            input = input.substr(n);
            n = dispatchSingleUrc(input);
            consumed += n;
        }

        return consumed;
    }

    std::size_t dispatchSingleUrc(ReadOnlyText input) {
        return urcDispatcher.dispatch(input);
    }

    void wakeCommandWaiters() {
        {
            Platform::Mutex::LockGuard g{mtx};
            canSubmitCommand = true;
        }
        condvar.notifyAll();
    }

    void blockCommandSubmission() {
        Platform::Mutex::LockGuard g{mtx};
        canSubmitCommand = false;
    }

    bool isStopping() {
        Platform::Mutex::LockGuard g{mtx};
        return stopping;
    }
};

} // namespace Fsm
} // namespace Core
} // namespace ATL_NS
