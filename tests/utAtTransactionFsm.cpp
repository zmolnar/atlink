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

#include "atlink/core/fsm/AtTransactionFsm.h"

#include <catch2/catch_all.hpp>

namespace {

using ATL_NS::Core::ErrorCode;
namespace Fsm = ATL_NS::Core::Fsm;
using AtTransactionFsm = Fsm::AtTransactionFsm;
using PendingExchange = Fsm::PendingExchange;

template <typename T>
bool hasAction(AtTransactionFsm::actions_t const &actions) {
    for (auto const &a : actions) {
        if (std::holds_alternative<T>(a)) {
            return true;
        }
    }
    return false;
}

template <typename T>
T const *findAction(AtTransactionFsm::actions_t const &actions) {
    for (auto const &a : actions) {
        if (auto const *found = std::get_if<T>(&a)) {
            return found;
        }
    }
    return nullptr;
}

} // namespace

SCENARIO("FSM sends command immediately when TX window is open") {

    GIVEN("FSM in Ready with txOpen=true") {
        AtTransactionFsm fsm{std::in_place_type<Fsm::State::Ready>};
        PendingExchange req{};

        WHEN("RequestSubmitted is dispatched") {
            auto actions = fsm.dispatch(Fsm::Event::RequestSubmitted{req});

            THEN("FSM enters WaitingReply and emits SendCommand + StartTxWindowTimer") {
                REQUIRE(std::holds_alternative<Fsm::State::WaitingReply>(fsm.state()));
                auto const &st = std::get<Fsm::State::WaitingReply>(fsm.state());
                REQUIRE_FALSE(st.txOpen);

                REQUIRE(2U == actions.size());
                REQUIRE(std::holds_alternative<Fsm::Action::SendCommand>(actions[0]));
                REQUIRE(std::holds_alternative<Fsm::Action::StartTxWindowTimer>(actions[1]));

                auto const *send = findAction<Fsm::Action::SendCommand>(actions);
                REQUIRE(send != nullptr);
                REQUIRE(send->req.command == req.command);
                REQUIRE(send->req.response == req.response);
                REQUIRE(send->req.result == req.result);
            }
        }
    }
}

SCENARIO("FSM queues request when TX window is closed and sends after open") {

    GIVEN("FSM that transitions to Ready(txOpen=false) after one complete reply") {
        AtTransactionFsm fsm{std::in_place_type<Fsm::State::Ready>};

        (void)fsm.dispatch(Fsm::Event::RequestSubmitted{PendingExchange{}});
        (void)fsm.dispatch(Fsm::Event::ReplyComplete{});

        REQUIRE(std::holds_alternative<Fsm::State::Ready>(fsm.state()));
        REQUIRE_FALSE(std::get<Fsm::State::Ready>(fsm.state()).txOpen);

        PendingExchange req{};

        WHEN("Another RequestSubmitted arrives while txOpen=false") {
            auto queuedActions = fsm.dispatch(Fsm::Event::RequestSubmitted{req});

            THEN("FSM enters WaitingTxWindow without immediate send") {
                REQUIRE(std::holds_alternative<Fsm::State::WaitingTxWindow>(fsm.state()));
                REQUIRE(queuedActions.empty());
            }

            WHEN("TxWindowOpen is dispatched") {
                auto sendActions = fsm.dispatch(Fsm::Event::TxWindowOpen{});

                THEN("FSM enters WaitingReply and emits SendCommand + StartTxWindowTimer") {
                    REQUIRE(std::holds_alternative<Fsm::State::WaitingReply>(fsm.state()));
                    REQUIRE_FALSE(std::get<Fsm::State::WaitingReply>(fsm.state()).txOpen);

                    REQUIRE(2U == sendActions.size());
                    REQUIRE(std::holds_alternative<Fsm::Action::SendCommand>(sendActions[0]));
                    REQUIRE(
                        std::holds_alternative<Fsm::Action::StartTxWindowTimer>(sendActions[1]));
                }
            }
        }
    }
}

SCENARIO("TxWindowOpen while waiting for reply is preserved when returning to Ready") {

    GIVEN("FSM in WaitingReply after sending a command") {
        AtTransactionFsm fsm{std::in_place_type<Fsm::State::Ready>};
        (void)fsm.dispatch(Fsm::Event::RequestSubmitted{PendingExchange{}});

        REQUIRE(std::holds_alternative<Fsm::State::WaitingReply>(fsm.state()));
        REQUIRE_FALSE(std::get<Fsm::State::WaitingReply>(fsm.state()).txOpen);

        WHEN("TxWindowOpen arrives before reply completion") {
            auto openActions = fsm.dispatch(Fsm::Event::TxWindowOpen{});

            THEN("WaitingReply stores txOpen=true") {
                REQUIRE(openActions.empty());
                REQUIRE(std::holds_alternative<Fsm::State::WaitingReply>(fsm.state()));
                REQUIRE(std::get<Fsm::State::WaitingReply>(fsm.state()).txOpen);
            }

            WHEN("ReplyComplete is dispatched") {
                auto completeActions = fsm.dispatch(Fsm::Event::ReplyComplete{});

                THEN("Ready keeps txOpen=true and emits finalize + wake waiters") {
                    REQUIRE(std::holds_alternative<Fsm::State::Ready>(fsm.state()));
                    REQUIRE(std::get<Fsm::State::Ready>(fsm.state()).txOpen);

                    REQUIRE(hasAction<Fsm::Action::FinalizeExchange>(completeActions));
                    REQUIRE(hasAction<Fsm::Action::WakeCommandWaiters>(completeActions));

                    auto const *finalize =
                        findAction<Fsm::Action::FinalizeExchange>(completeActions);
                    REQUIRE(finalize != nullptr);
                    REQUIRE(finalize->ec == ErrorCode::NoError);
                }
            }
        }
    }
}

SCENARIO("ReplyFailed finalizes with the provided error") {

    GIVEN("FSM in WaitingReply") {
        AtTransactionFsm fsm{std::in_place_type<Fsm::State::Ready>};
        (void)fsm.dispatch(Fsm::Event::RequestSubmitted{PendingExchange{}});
        REQUIRE(std::holds_alternative<Fsm::State::WaitingReply>(fsm.state()));

        WHEN("ReplyFailed is dispatched") {
            auto actions = fsm.dispatch(Fsm::Event::ReplyFailed{ErrorCode::InternalError});

            THEN("FSM returns to Ready and emits FinalizeExchange with error") {
                REQUIRE(std::holds_alternative<Fsm::State::Ready>(fsm.state()));

                auto const *finalize = findAction<Fsm::Action::FinalizeExchange>(actions);
                REQUIRE(finalize != nullptr);
                REQUIRE(finalize->ec == ErrorCode::InternalError);
                REQUIRE(hasAction<Fsm::Action::WakeCommandWaiters>(actions));
            }
        }
    }
}

SCENARIO("StopRequested transitions FSM to Shutdown") {

    GIVEN("FSM in Ready") {
        AtTransactionFsm fsm{std::in_place_type<Fsm::State::Ready>};

        WHEN("StopRequested is dispatched") {
            auto actions = fsm.dispatch(Fsm::Event::StopRequested{});

            THEN("FSM enters Shutdown and emits SignalShutdownEntered") {
                REQUIRE(std::holds_alternative<Fsm::State::Shutdown>(fsm.state()));
                REQUIRE(hasAction<Fsm::Action::SignalShutdownEntered>(actions));
            }
        }
    }
}
