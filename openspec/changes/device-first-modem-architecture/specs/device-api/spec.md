## Purpose

Defines the application-facing connected-modem abstraction, its feature interfaces, lifecycle, ownership, events, and advanced typed AT access.

## ADDED Requirements

### Requirement: Concrete device exposes supported features
The library SHALL represent a connected modem as a concrete device type that owns and exposes the higher-level features supported by that modem. Normal client code SHALL use feature operations rather than manually constructing AT transactions.

#### Scenario: Client uses a supported feature
- **WHEN** a client obtains the network feature from an `Mc60` device and requests signal quality
- **THEN** the client receives a typed feature result without supplying AT response objects, final-result packs, visitors, or wire strings

#### Scenario: Device omits unsupported feature
- **WHEN** a concrete modem integration does not support a feature
- **THEN** that feature is not exposed by the concrete device's public interface

### Requirement: Device binds caller-owned resources
Device construction SHALL bind a transport and platform execution resources that outlive the device. Construction SHALL perform no modem I/O and start no processing context, timer, or worker.

#### Scenario: Constructed but not started
- **WHEN** a client constructs a device with valid transport and execution-resource references
- **THEN** the device is stopped, has performed no modem I/O, and owns no hidden heap allocation in core or higher layers

### Requirement: Device lifecycle is explicit
The device SHALL provide explicit `start()` and `stop()` operations with typed results. Successful startup SHALL mean that feature requests may be accepted, but SHALL NOT imply SIM readiness, network registration, or availability of every feature operation.

#### Scenario: Successful startup
- **WHEN** `start()` activates the execution context and completes required modem initialization
- **THEN** it returns `Started` and the device can accept feature and direct-AT requests

#### Scenario: Repeated startup
- **WHEN** `start()` is called on a running device
- **THEN** it returns `AlreadyRunning` and does not create another processing context or initialization sequence

#### Scenario: Failed startup
- **WHEN** startup cannot complete
- **THEN** it returns a typed startup failure, rolls back partial activation, and leaves the device stopped and eligible for a later retry

#### Scenario: Idempotent stop
- **WHEN** `stop()` is called on a stopped device
- **THEN** it returns `Stopped` without activating resources

### Requirement: Shutdown resolves accepted work
Shutdown SHALL close admission, resolve each accepted operation exactly once, stop input delivery, timers, callbacks, and execution, and establish quiescence before successful return.

#### Scenario: Request arrives after admission closes
- **WHEN** a feature or direct-AT request arrives after shutdown has closed admission
- **THEN** it is not accepted and returns `DeviceUnavailable`

#### Scenario: Accepted operation during shutdown
- **WHEN** shutdown begins while an accepted operation is incomplete
- **THEN** an already committed completion is preserved or the operation completes as `Cancelled`, and no terminal outcome is silently discarded

#### Scenario: Stop returns
- **WHEN** `stop()` returns from a permitted context, including a cleanup-failure result
- **THEN** no internal execution or callback can access the device, transport, resources, or callback contexts

### Requirement: Blocking APIs reject the device event context
Blocking lifecycle, feature, and direct-AT operations SHALL detect invocation from the device event-processing context and return `WrongExecutionContext` without submitting work or waiting.

#### Scenario: Feature callback invokes blocking operation
- **WHEN** a feature event callback invokes a blocking feature operation
- **THEN** the operation returns `WrongExecutionContext` instead of deadlocking

### Requirement: Feature events use typed non-owning callbacks
Application event callbacks SHALL be configured without an owning, potentially allocating callback wrapper. They SHALL run serially on the device processing context, receive typed event data valid for the call duration, and cease before shutdown returns.

#### Scenario: Registration URC is delivered
- **WHEN** the device decodes a network-registration URC and a registration callback is configured
- **THEN** it invokes the callback on the device context with typed registration data

#### Scenario: Notification is disabled
- **WHEN** no application callback is configured for an event
- **THEN** application notification is skipped while internal URC processing still occurs

### Requirement: Direct AT access shares device coordination
The device SHALL expose advanced typed direct-AT execution. Direct commands SHALL use the same admission, scheduling, transaction, routing, capacity, cancellation, and shutdown mechanisms as feature-generated commands.

#### Scenario: Direct command executes
- **WHEN** a client executes a command with valid external protocol metadata
- **THEN** the response type is derived from that metadata and returned as an owned typed result

#### Scenario: Direct command changes feature-owned configuration
- **WHEN** a direct command changes modem state owned by a feature without a declared coordination policy
- **THEN** the library does not guarantee that the feature's cached state remains valid
