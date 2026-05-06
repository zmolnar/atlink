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

#include "atlink/core/Command.h"
#include "atlink/core/ErrorCode.h"
#include "atlink/core/Response.h"
#include "atlink/core/ResponsePack.h"
#include "atlink/platform/Facade.h"
#include <typemachine/machine.h>

#include <variant>

namespace ATL_NS {
namespace Core {
namespace Fsm {

// ============================================================================
// FSM payload
// ============================================================================

struct PendingExchange {
    AResponsePack *result = nullptr;
    Core::Command *command = nullptr;
    Response *response = nullptr;
    ErrorCode *ec = nullptr;
    Platform::Semaphore *sem = nullptr;
};

// ============================================================================
// States
// ============================================================================

namespace State {

struct Ready {
    bool txOpen = true;
};

struct WaitingTxWindow {
    PendingExchange req{};
};

struct WaitingReply {
    PendingExchange req{};
    bool txOpen = false;
};

struct Shutdown {};

} // namespace State

// ============================================================================
// Events
// ============================================================================

namespace Event {

struct RequestSubmitted {
    PendingExchange req{};
};

struct TxWindowOpen {};

struct RxReady {};

struct ReplyComplete {};

struct ReplyFailed {
    ErrorCode ec{ErrorCode::InternalError};
};

struct StopRequested {};

} // namespace Event

using EventVariant = std::variant<Event::RequestSubmitted,
                                  Event::TxWindowOpen,
                                  Event::RxReady,
                                  Event::ReplyComplete,
                                  Event::ReplyFailed,
                                  Event::StopRequested>;

// ============================================================================
// Actions
// ============================================================================

namespace Action {

struct SendCommand {
    PendingExchange req{};
};

struct FinalizeExchange {
    PendingExchange req{};
    ErrorCode ec{ErrorCode::NoError};
};

struct StartTxWindowTimer {};

struct WakeCommandWaiters {};

struct BlockCommandSubmission {};

struct HandleUrcs {};

struct HandleInput {
    PendingExchange req{};
};

struct SignalShutdownEntered {};

} // namespace Action

using ActionVariant = std::variant<Action::SendCommand,
                                   Action::FinalizeExchange,
                                   Action::StartTxWindowTimer,
                                   Action::WakeCommandWaiters,
                                   Action::BlockCommandSubmission,
                                   Action::HandleUrcs,
                                   Action::HandleInput,
                                   Action::SignalShutdownEntered>;

} // namespace Fsm
} // namespace Core
} // namespace ATL_NS

namespace TM_NS {

template <>
struct state_traits<ATL_NS::Core::Fsm::State::Ready> {
    template <class Actions>
    static void on_entry(ATL_NS::Core::Fsm::State::Ready &, Actions &actions) {
        actions.push_back(
            ATL_NS::Core::Fsm::ActionVariant{ATL_NS::Core::Fsm::Action::WakeCommandWaiters{}});
    }

    template <class Actions>
    static void on_exit(ATL_NS::Core::Fsm::State::Ready &, Actions &actions) {
        actions.push_back(ATL_NS::Core::Fsm::ActionVariant{
            ATL_NS::Core::Fsm::Action::BlockCommandSubmission{}});
    }
};

template <>
struct state_traits<ATL_NS::Core::Fsm::State::Shutdown> {
    template <class Actions>
    static void on_entry(ATL_NS::Core::Fsm::State::Shutdown &, Actions &actions) {
        actions.push_back(
            ATL_NS::Core::Fsm::ActionVariant{ATL_NS::Core::Fsm::Action::SignalShutdownEntered{}});
    }

    template <class Actions>
    static void on_exit(ATL_NS::Core::Fsm::State::Shutdown &, Actions &) {}
};

} // namespace TM_NS

namespace ATL_NS {
namespace Core {
namespace Fsm {

// ============================================================================
// Transitions
// ============================================================================

inline constexpr auto t_ready_submit_open =
    TM_NS::from<State::Ready>::on<Event::RequestSubmitted>::to<State::WaitingReply>
        .when([](State::Ready const &s, Event::RequestSubmitted const &) { return s.txOpen; })
        .then([](State::Ready &, Event::RequestSubmitted const &ev, State::WaitingReply &to,
                 auto &actions) {
            to.req = ev.req;
            to.txOpen = false;
            actions.push_back(ActionVariant{Action::SendCommand{ev.req}});
            actions.push_back(ActionVariant{Action::StartTxWindowTimer{}});
        });

inline constexpr auto t_ready_submit_closed =
    TM_NS::from<State::Ready>::on<Event::RequestSubmitted>::to<State::WaitingTxWindow>
        .when([](State::Ready const &s, Event::RequestSubmitted const &) { return !s.txOpen; })
        .then([](State::Ready &, Event::RequestSubmitted const &ev, State::WaitingTxWindow &to,
                 auto &) {
            to.req = ev.req;
        });

inline constexpr auto t_ready_txwindow_open =
    TM_NS::from<State::Ready>::on<Event::TxWindowOpen>::to<State::Ready>.then(
        [](State::Ready &from, Event::TxWindowOpen const &, State::Ready &to, auto &) {
            to.txOpen = true;
        });

inline constexpr auto t_wait_tx_simple =
    TM_NS::from<State::WaitingTxWindow>::on<Event::TxWindowOpen>::to<State::WaitingReply>.then(
        [](State::WaitingTxWindow &from,
           Event::TxWindowOpen const &,
           State::WaitingReply &to,
           auto &actions) {
            to.req = from.req;
            to.txOpen = false;
            actions.push_back(ActionVariant{Action::SendCommand{from.req}});
            actions.push_back(ActionVariant{Action::StartTxWindowTimer{}});
        });

inline constexpr auto t_wait_final_txwindow_open =
    TM_NS::from<State::WaitingReply>::on<Event::TxWindowOpen>::to<State::WaitingReply>.then(
        [](State::WaitingReply &from,
           Event::TxWindowOpen const &,
           State::WaitingReply &to,
           auto &) {
            to.req = from.req;
            to.txOpen = true;
        });

inline constexpr auto t_ready_rx_ready =
    TM_NS::from<State::Ready>::on<Event::RxReady>::to<State::Ready>.then(
        [](State::Ready &, Event::RxReady const &, State::Ready &, auto &actions) {
            actions.push_back(ActionVariant{Action::HandleUrcs{}});
        });

inline constexpr auto t_wait_tx_rx_ready =
    TM_NS::from<State::WaitingTxWindow>::on<Event::RxReady>::to<State::WaitingTxWindow>.then(
        [](State::WaitingTxWindow &,
           Event::RxReady const &,
           State::WaitingTxWindow &,
           auto &actions) {
            actions.push_back(ActionVariant{Action::HandleUrcs{}});
        });

inline constexpr auto t_wait_final_rx_ready =
    TM_NS::from<State::WaitingReply>::on<Event::RxReady>::to<State::WaitingReply>.then(
        [](State::WaitingReply &from,
           Event::RxReady const &,
           State::WaitingReply &to,
           auto &actions) {
            to.req = from.req;
            actions.push_back(ActionVariant{Action::HandleInput{from.req}});
        });

inline constexpr auto t_wait_final_complete =
    TM_NS::from<State::WaitingReply>::on<Event::ReplyComplete>::to<State::Ready>.then(
        [](State::WaitingReply &from,
           Event::ReplyComplete const &,
           State::Ready &to,
           auto &actions) {
            actions.push_back(
                ActionVariant{Action::FinalizeExchange{from.req, ErrorCode::NoError}});
            to.txOpen = from.txOpen;
        });

inline constexpr auto t_wait_final_failed =
    TM_NS::from<State::WaitingReply>::on<Event::ReplyFailed>::to<State::Ready>.then(
        [](State::WaitingReply &from,
           Event::ReplyFailed const &ev,
           State::Ready &to,
           auto &actions) {
            actions.push_back(ActionVariant{Action::FinalizeExchange{from.req, ev.ec}});
            to.txOpen = from.txOpen;
        });

inline constexpr auto t_ready_stop =
    TM_NS::from<State::Ready>::on<Event::StopRequested>::to<State::Shutdown>.then(
        [](State::Ready &, Event::StopRequested const &, State::Shutdown &, auto &) {
        });

inline constexpr auto t_wait_tx_stop =
    TM_NS::from<State::WaitingTxWindow>::on<Event::StopRequested>::to<State::Shutdown>.then(
        [](State::WaitingTxWindow &, Event::StopRequested const &, State::Shutdown &, auto &) {
        });

inline constexpr auto t_wait_final_stop =
    TM_NS::from<State::WaitingReply>::on<Event::StopRequested>::to<State::Shutdown>.then(
        [](State::WaitingReply &, Event::StopRequested const &, State::Shutdown &, auto &) {
        });

inline constexpr auto t_shutdown_stop =
    TM_NS::from<State::Shutdown>::on<Event::StopRequested>::to<State::Shutdown>.then(
        [](State::Shutdown &, Event::StopRequested const &, State::Shutdown &, auto &) {
        });

// ============================================================================
// Machine type
// ============================================================================

using AtTransactionFsm = TM_NS::machine<ActionVariant,
                                        t_ready_submit_open,
                                        t_ready_submit_closed,
                                        t_ready_txwindow_open,
                                        t_ready_rx_ready,
                                        t_ready_stop,
                                        t_wait_tx_simple,
                                        t_wait_tx_rx_ready,
                                        t_wait_tx_stop,
                                        t_wait_final_txwindow_open,
                                        t_wait_final_rx_ready,
                                        t_wait_final_complete,
                                        t_wait_final_failed,
                                        t_wait_final_stop,
                                        t_shutdown_stop>;

} // namespace Fsm
} // namespace Core
} // namespace ATL_NS
