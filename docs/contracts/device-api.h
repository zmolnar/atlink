// SPDX-License-Identifier: GPL-3.0-or-later
//
// Phase-1 reference declarations. This file describes API signatures only.
// It is not installed, has no runtime definitions or representative object
// layouts, and must not be used to construct a device. See ../device-api.md.
// Compile with the same ATL_NS definition as the library.

#pragma once

#include <cstddef>

namespace ATL_NS {

// Phase 2 supplies the implementation and exhaustive match() interface.
template <typename Success, typename... Errors>
class [[nodiscard("Handle the operation result")]] Result;

namespace Errors {

// Shared semantic categories. Payloads are specified in phase 3.
struct DeviceUnavailable;
struct CapacityExceeded;
struct WrongExecutionContext;
struct Cancelled;
struct TimedOut;

} // namespace Errors

namespace Lifecycle {

struct Started;
struct Stopped;
struct AlreadyRunning;
struct StartFailed;
struct ShutdownFailed;

using StartResult = Result<Started,
                           AlreadyRunning,
                           StartFailed,
                           Errors::WrongExecutionContext>;

using StopResult = Result<Stopped, ShutdownFailed, Errors::WrongExecutionContext>;

} // namespace Lifecycle

namespace Features {
namespace Network {

// Feature-level, owning value types. These are not AT response records.
struct SignalQuality;
struct RegistrationChanged;

// Phase 3 defines the details retained at the feature boundary.
struct ModemRejected;
struct TransportFailure;
struct ProtocolFailure;

using SignalQualityResult = Result<SignalQuality,
                                   Errors::DeviceUnavailable,
                                   Errors::CapacityExceeded,
                                   Errors::WrongExecutionContext,
                                   Errors::Cancelled,
                                   Errors::TimedOut,
                                   ModemRejected,
                                   TransportFailure,
                                   ProtocolFailure>;

using RegistrationHandler = void (*)(void *context, const RegistrationChanged &event) noexcept;

struct EventHandlers {
    void *context{nullptr};
    RegistrationHandler registrationChanged{nullptr};
};

template <typename Runtime>
class Feature {
  public:
    Feature(const Feature &) = delete;
    Feature &operator=(const Feature &) = delete;
    Feature(Feature &&) = delete;
    Feature &operator=(Feature &&) = delete;

    [[nodiscard]] SignalQualityResult signalQuality() noexcept;

  private:
    // Constructed by the owning device in the implementation.
    Feature();
};

} // namespace Network
} // namespace Features

namespace Protocol {

// Defined by the modem integration engineer, outside the command data type.
template <typename Command>
struct CommandTraits;

} // namespace Protocol

namespace At {

struct ModemRejected;
struct TransportFailure;
struct EncodingFailure;
struct DecodingFailure;

template <typename Response>
using CommandResult = Result<Response,
                             Errors::DeviceUnavailable,
                             Errors::CapacityExceeded,
                             Errors::WrongExecutionContext,
                             Errors::Cancelled,
                             Errors::TimedOut,
                             ModemRejected,
                             TransportFailure,
                             EncodingFailure,
                             DecodingFailure>;

template <typename Runtime>
class Access {
  public:
    Access(const Access &) = delete;
    Access &operator=(const Access &) = delete;
    Access(Access &&) = delete;
    Access &operator=(Access &&) = delete;

    template <typename Command>
    [[nodiscard]] CommandResult<typename Protocol::CommandTraits<Command>::Response>
    execute(const Command &command) noexcept;

  private:
    Access();
};

} // namespace At

namespace Devices {

struct EventHandlers {
    Features::Network::EventHandlers network{};
};

// An engineer-provided Limits type declares these compile-time capacities:
// rxBytes, txBytes, pendingOperations, queuedEvents. No implicit heap fallback.
// Additional capacities can be specified as more features are introduced.
template <typename Platform, typename Transport, typename Limits>
class Mc60 {
    struct Runtime;

  public:
    using ExecutionResources = typename Platform::template DeviceResources<Limits>;
    using Network = Features::Network::Feature<Runtime>;
    using DirectAt = At::Access<Runtime>;

    // Non-owning bindings. All supplied resources outlive this device.
    explicit Mc60(Transport &transport,
                  ExecutionResources &resources,
                  EventHandlers handlers = {}) noexcept;

    Mc60(const Mc60 &) = delete;
    Mc60 &operator=(const Mc60 &) = delete;
    Mc60(Mc60 &&) = delete;
    Mc60 &operator=(Mc60 &&) = delete;

    // Requires a stopped/quiescent device; fallible shutdown is explicit.
    ~Mc60() noexcept;

    [[nodiscard]] Lifecycle::StartResult start() noexcept;
    [[nodiscard]] Lifecycle::StopResult stop() noexcept;

    Network &network() noexcept;
    DirectAt &at() noexcept;
};

} // namespace Devices
} // namespace ATL_NS
