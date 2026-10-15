# Device API contract — phase 1

**Review status:** proposed contract. This phase establishes signatures and
behavior before implementing the result type or the new device runtime.

The [reference declarations](contracts/device-api.h) accompany this document.
They are deliberately outside the installed library: they have no runtime
definitions, storage layout, or backend implementation. Existing `Core::Device`
continues to be the implemented interface until the new path is integrated.

## 1. Public model and vocabulary

An application is a **client of a device**. A concrete device such as `Mc60`
composes the features supported by that modem. The normal application boundary
is feature-level data and operations, not individual AT commands.

| Concept | Responsibility |
| --- | --- |
| Device | Feature composition, shared execution, routing, and lifecycle |
| Feature | An application-facing capability, such as network, SMS, call, or GNSS |
| Feature machine | A typemachine workflow for a feature operation or state |
| AT executor | Shared command scheduling and transaction execution |
| Transaction | One command, its response data if any, and its final result |
| Response | Command-specific modem data |
| Final result | Terminal modem status, such as `OK` or `+CMS ERROR` |
| URC | An unsolicited modem notification |
| Protocol traits | External descriptions of wire representation and type associations |
| Platform | Execution, synchronization, timers, and logging |
| Transport | Byte I/O and notifications from the connected modem |

The device owns its features, machines, bounded operation storage, and routing
state. The AT executor is shared by features and advanced direct access.
Feature machines are independent stateful objects, not independent threads.

There are two typed boundaries:

1. **Application to feature:** domain data, for example signal quality, an SMS
   message, or a position.
2. **Feature to AT executor:** command, response, and URC structures described by
   external traits.

Protocol records and feature data do not inherit from visitors or packet bases.
They are value-owning, standard-layout and trivially copyable wherever practical.
Behavioral objects such as devices, features, and results need not be POD.

## 2. Allocation contract

### Core and above: no heap allocation

This applies to codecs, protocol helpers and data, the AT executor, devices,
features, their machines, and their core-side event and completion storage.
It applies during construction, normal operation, error handling, and shutdown.

- No `new`, `malloc`, or owning heap-backed containers in these layers.
- No `std::vector`, `std::string`, allocating stream buffers, or owning
  `std::function` callbacks. Hiding allocation in a helper does not satisfy the
  contract.
- Use inline bounded storage, native values, and non-owning views with explicit
  lifetimes. A `std::variant` stores its alternative inline, but every permitted
  alternative must also obey the allocation contract.
- Enum mappings and wire literals are static metadata. Numeric and textual enum
  conversion remain supported without allocating temporary strings.
- Internal typemachine action storage must also be allocation-free. This must be
  verified for the pinned dependency during runtime integration; putting a
  heap-backed action collection inside a machine does not make it a platform
  allocation.
- Capacities are compile-time configuration. Exceeding a bound is an explicit
  outcome, never a request to grow storage, silently truncate, or fall back to
  the heap.

The integration engineer supplies a `Limits` type. The initial capacity names
are `rxBytes`, `txBytes`, `pendingOperations`, and `queuedEvents`, each a nonzero
compile-time size. Their exact minimum values and reserved internal event slots
are established with the executor. Completion, cancellation, and stop delivery
must remain possible when ordinary request capacity is exhausted. The runtime
must not silently discard an already accepted operation's terminal outcome.

### Platform: allocation policy is backend-specific

A platform **may** allocate. Linux can use heap-backed OS/thread/queue wrappers
where appropriate. A Zephyr, FreeRTOS, or custom backend can instead use static
tasks, stacks, queues, and synchronization objects.

The platform contract must permit both. No core signature requires a heap-owned
pointer, allocating callable, resizable container, or allocation-capable backend.
A backend remains subject to the same observable capacity and error rules even
if it chooses dynamic storage internally.

Transport backends belong on this platform side of the boundary. FreeRTOS is an
execution environment, not a universal UART driver; a board/SDK supplies that
transport implementation.

**End-to-end allocation freedom is obtained by selecting static backends.** A
heap-capable platform does not relax the restrictions on core and higher layers.
Application callbacks may allocate according to application policy; ATLink does
not allocate a wrapper around them.

The existing Linux/runtime implementation is not claimed to meet this contract
yet. Later integration must audit both direct and transitive allocations.

## 3. Construction, platform selection, and ownership

Platform and transport are separate compile-time choices. A modem integration
can provide a short `Mc60` alias for its selected platform, transport, and limits.
Ordinary application code does not need to carry those template arguments.

The proposed construction shape is:

```cpp
using Mc60 = atlink::Devices::Mc60<SelectedPlatform, SelectedTransport, Limits>;

SelectedTransport transport{/* backend configuration */};
Mc60::ExecutionResources resources{/* backend configuration */};

atlink::Devices::EventHandlers handlers{};
handlers.network.context = &application;
handlers.network.registrationChanged = onRegistrationChanged;

Mc60 device{transport, resources, handlers};
```

This is proposed usage, not a runnable example in this phase. Backend types and
configuration are defined in the platform phase.

- The device constructor only binds resources and initializes inline state. It
  performs no modem I/O and starts no worker or timer.
- `transport` and `resources` are borrowed and outlive the device. The application
  gives the device exclusive use of the transport while running; it must not also
  read, write, close, or reconfigure it behind the executor.
- `Platform::DeviceResources<Limits>` is caller-owned backend storage. A dynamic
  backend can own allocations inside it; a static backend can embed OS control
  blocks and stacks. Fallible activation is reported by `start()` or by an
  explicit backend setup result before construction, not hidden in core code.
- The device owns logical execution and shutdown even though the OS storage is
  supplied externally.
- Devices and feature handles cannot be copied or moved. Feature accessors return
  references owned by the device, valid until the device is destroyed.
- Callback bindings are copied as a function pointer plus a context pointer. The
  context remains alive through the last callback and device shutdown. This first
  interface binds handlers at construction, avoiding subscription allocation and
  concurrent replacement rules.
- Selecting one platform must not require headers from another platform.

## 4. Explicit lifecycle

`start()` and `stop()` are blocking, result-returning operations invoked outside
the device event context. Applications serialize lifecycle calls with each other.
Normal feature requests may race with shutdown and receive the outcomes below.

| Call | Outcome and postcondition |
| --- | --- |
| `start()` on a stopped device | Activates resources and the single processing context, then performs required modem setup. `Started` means feature requests can be accepted. |
| `start()` while already running | `AlreadyRunning`; no second context or initialization sequence is created. |
| Failed startup | `StartFailed`; partial activation is rolled back and the device is stopped/quiescent. A later retry is allowed. |
| `stop()` while running | Closes admission, resolves accepted operations, stops callbacks/timers/input delivery, and joins or otherwise quiesces the execution context. |
| `stop()` while stopped | `Stopped`; the operation is idempotent. |
| Shutdown reports a cleanup failure | `ShutdownFailed` retains the diagnostic, but execution is still quiescent. Platform stop/join must guarantee callback quiescence before returning; there is no detached worker accessing a destroyed device. |
| Blocking call from the device event context | `WrongExecutionContext`; no work is submitted and no wait occurs. For `stop()`, this outcome does **not** establish quiescence. |

Startup success does not imply SIM readiness, network registration, or that every
feature is currently usable. Those are feature-level states.

Once shutdown closes admission, new feature/direct requests receive
`DeviceUnavailable`. Accepted requests complete exactly once: a completion already
committed is preserved; otherwise shutdown resolves them as `Cancelled`.
On return from `stop()` (except `WrongExecutionContext`), no internal execution or
callback may access the device, its transport, resources, or callback contexts.
A subsequent `start()` starts a fresh execution cycle; stale completions from the
previous cycle must not enter it.

The destructor is non-throwing and requires a stopped/quiescent device. It is not a
replacement for explicit, fallible shutdown. An object that was never started or
whose startup failed is also safe to destroy.

## 5. Explicit results and exhaustive handling

Every fallible public operation returns a named alias of:

```cpp
Result<Success, Errors...>
```

The phase-2 implementation will own a private `std::variant`, provide `match()`,
and be `[[nodiscard]]`. No default construction silently means success. There is
no unchecked `value()`, `unwrap()`, implicit boolean conversion, or public mutable
variant escape hatch in the initial API. Operational failures are represented by
typed alternatives rather than exceptions or log messages alone.

Use the existing `Utils::Overload` pattern. Each explicitly typed visitor must
handle every alternative; omitting an alternative is a compile-time error.
`match()` supports const and mutable lvalues and rvalues, including moving an
owned success value. Handler return types must agree. This contract does not
require allocating callback wrappers or impose `noexcept` on application-side
result visitors.

The API sketch for startup is:

```cpp
using atlink::Utils::Overload;

const bool running = device.start().match(Overload{
    [](const atlink::Lifecycle::Started&) { return true; },
    [](const atlink::Lifecycle::AlreadyRunning&) { return true; },
    [](const atlink::Lifecycle::StartFailed& error) {
        reportStartFailure(error);
        return false;
    },
    [](const atlink::Errors::WrongExecutionContext&) {
        reportInvalidCallingContext();
        return false;
    },
});
```

`Result` and payload definitions do not exist yet; these usage sketches become
compiled examples as their implementations land.

C++20 cannot force use of a result in all programs. `[[nodiscard]]` diagnoses
discarding it, and supported checks treat that diagnostic as an error. Consumers
can still explicitly discard it or disable diagnostics. A generic visitor is
also an explicit choice to group alternatives; only individually typed handlers
force a new handler when a new alternative is introduced. The library does not
use destructor assertions to pretend to provide stronger guarantees.

## 6. First feature contract: network

The first concrete operation is a blocking signal-quality query:

```cpp
device.network().signalQuality().match(Overload{
    [](const atlink::Features::Network::SignalQuality& quality) {
        displaySignalQuality(quality);
    },
    [](const atlink::Errors::DeviceUnavailable&) { reportDeviceStopped(); },
    [](const atlink::Errors::CapacityExceeded&) { reportBusy(); },
    [](const atlink::Errors::WrongExecutionContext&) { reportInvalidCallingContext(); },
    [](const atlink::Errors::Cancelled&) { reportCancelled(); },
    [](const atlink::Errors::TimedOut&) { reportTimeout(); },
    [](const atlink::Features::Network::ModemRejected& error) { reportRejection(error); },
    [](const atlink::Features::Network::TransportFailure& error) { reportTransport(error); },
    [](const atlink::Features::Network::ProtocolFailure& error) { reportProtocol(error); },
});
```

These are semantic error categories. Phase 3 defines their allocation-free
payloads and the signal-quality representation, including unavailable/unknown
measurements. They do not expose parser state, platform synchronization, or
feature-machine internals. A modem final result and successful transport I/O are
not by themselves feature success.

Multiple application threads/tasks may request operations. Requests use bounded
device-owned slots; `CapacityExceeded` means a request was not accepted and no
command was sent for it. Every accepted operation eventually receives exactly
one terminal outcome, including cancellation or timeout. The feature's workflow
submits commands and consumes completion events; it never blocks the event loop.

Returned values own their data. No returned response contains a dangling view
into the RX buffer, an operation slot, or a temporary parsing candidate.

Feature-specific naming must describe success precisely. For later SMS support,
acceptance for submission and delivery are different outcomes; delivery may be a
later event rather than the result of `send()`.

### Unsolicited feature events

The first event is a registration-state change. A callback has this shape:

```cpp
void onRegistrationChanged(
    void* context,
    const atlink::Features::Network::RegistrationChanged& event) noexcept;
```

- Callbacks run serially on the device processing context, not an ISR, transport
  poller, or timer callback thread.
- The event reference is valid only during the callback. Its bounded value can
  be copied into application-owned storage for later processing.
- Callbacks are non-throwing, should return promptly, and must not call blocking
  feature, direct-AT, or lifecycle operations. Those operations reject this
  context explicitly instead of deadlocking.
- A null callback disables application notification, not internal URC processing.
- The device may route one URC to several interested internal features. Routing
  is independent of which feature owns the active transaction.

## 7. Advanced direct AT access

```cpp
auto result = device.at().execute(command);
// Handle every alternative with result.match(Overload{...}).
```

The modem engineer defines `Protocol::CommandTraits<Command>::Response` and the
external wire metadata. The application supplies a command value, not response
pointers, a final-result pack, a visitor, or manually formatted text.

Direct execution is blocking outside the event context and shares all admission,
scheduling, routing, capacity, cancellation, and shutdown rules with features.
It is not a way to bypass the transaction machine or write directly to transport.

The command is borrowed for the duration of the call. The caller must keep it
alive and unmodified until return. The executor must not retain references to it
after return, including cancellation and failure paths. The response is owned
by the returned result.

The direct error set distinguishes modem rejection, encoding failure, decoding
failure, transport failure, and shared execution failures. Feature operations
translate these into their own public outcomes.

Scheduling serializes bytes and protects explicitly coordinated command
sequences; it cannot infer every semantic effect of arbitrary custom commands.
Direct commands that change feature-owned modem configuration require an explicit
coordination policy from the integration engineer. The initial direct-access
contract does not promise that arbitrary configuration changes leave cached
feature state valid.

## 8. Protocol authoring boundary

Modem integration engineers provide ordinary data structures and external traits.
Metadata declares field order, prefixes, encoding, command/response associations,
and multiline structure. It requires no external reflection library, schema file,
or code generation. Serializers and deserializers own all wire handling.

Native enum members retain bidirectional numeric or textual translation through
mapping metadata. A field can override the enum's default encoding. Unknown
numeric/textual values have an explicit representation or error policy; an
unchecked cast must not silently accept unsupported values. The mapping/storage
details are designed in the protocol phase, not embedded in application data.

## 9. Verification and review boundary

For phase 1, verify that reference declarations are valid C++20 and that the
signatures preserve resource references, noncopyable devices, feature handles,
and typed command/response association. They can be checked without CMake or
external dependencies:

```sh
printf '#include "docs/contracts/device-api.h"\n' |
    c++ -std=c++20 -DATL_NS=atlink -I. -Wall -Wextra -Werror -pedantic -x c++ -fsyntax-only -
```

This is a declaration check, not a claim that device behavior or result matching
has been implemented. The Linux example and library behavior are migrated only
after the corresponding implementation phases.

Subsequent verification must include allocation instrumentation with a static
test platform, allocation-free result/data movement, all accepted operations
completing once, callback quiescence after shutdown, fragmented replies, URCs
during commands, capacity exhaustion, and missing-handler/discarded-result
compiler diagnostics. Heap use by a deliberately dynamic platform is permitted
and must be distinguished from core allocations.

## 10. Implementation sequence and merge gates

Each phase gets its own branch and PR. A successor is created from the merged
predecessor, only after its review is complete and its PR is successfully merged.
No stacked implementation or automatic merge is part of this workflow.

| Phase | Branch | Deliverable |
| --- | --- | --- |
| 1 | `phase/01-device-api-contract` | This public contract, reference declarations, and usage sketches |
| 2 | `phase/02-explicit-results` | Allocation-free `Result` and exhaustive matching |
| 3 | `phase/03-feature-outcomes` | Feature data, error payloads, and translation rules |
| 4 | `phase/04-device-composition` | Device ownership, compile-time platform/transport seams, and Linux wiring |
| 5 | `phase/05-network-feature` | Network typemachine, operation lifecycle, and event path |
| 6 | `phase/06-at-integration` | Typed protocol metadata and shared executor integration |
| 7 | `phase/07-device-example` | End-to-end verification, migrated Linux example, and user/engineer documentation |

Only phase 1 is proposed in this PR. Phase 2 begins after the user reviews and
merges phase 1.
