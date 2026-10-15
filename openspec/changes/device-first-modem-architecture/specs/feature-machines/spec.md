## Purpose

Defines independent feature workflows coordinated through one shared AT execution and routing core without requiring a thread per feature.

## ADDED Requirements

### Requirement: Each stateful feature owns its workflow machine
A stateful feature SHALL own a dedicated typemachine that consumes typed feature requests, transaction completions, relevant URCs, lifecycle events, and time events, and emits typed actions. Feature machines SHALL NOT parse wire text or operate the transport directly.

#### Scenario: Feature operation needs multiple AT commands
- **WHEN** a feature operation requires a sequence of AT transactions
- **THEN** its feature machine advances the workflow from typed completion events and submits subsequent typed command actions without blocking the device context

### Requirement: Device processing is serialized
All feature machines, transaction routing, and device-owned state SHALL be advanced from one serialized processing context per device. Independent machines SHALL NOT imply independent threads.

#### Scenario: Linux backend drives the device
- **WHEN** the Linux platform starts a device
- **THEN** one device processing thread serially dispatches device, feature, transaction, and routing events

#### Scenario: RTOS backend drives the device
- **WHEN** a Zephyr or FreeRTOS platform starts a device
- **THEN** one configured task or equivalent serialized context dispatches the same portable core events

### Requirement: AT execution is shared
Feature machines and direct access SHALL submit work to one shared AT executor. The executor SHALL serialize transport use, enforce transaction policy, and return typed completion to the originating operation.

#### Scenario: Features request commands concurrently
- **WHEN** multiple client threads submit operations whose feature workflows need AT commands
- **THEN** bounded admission accepts or rejects each request explicitly and accepted commands are scheduled through the shared executor

#### Scenario: Coordinated sequence is required
- **WHEN** a feature declares that a command sequence must not be interleaved by state-changing work
- **THEN** the executor honors an explicit coordination policy rather than requiring the feature to lock the transport

### Requirement: Solicited responses and URCs use contextual routing
Incoming records SHALL be decoded and routed using both type information and active transaction context. Solicited response data and final results SHALL advance the active transaction; URCs SHALL route independently to every interested internal feature.

#### Scenario: URC arrives during an active command
- **WHEN** a valid URC is interleaved with the active command's response
- **THEN** the URC reaches its interested feature and the active transaction continues assembling its own response

#### Scenario: Prefix can be solicited or unsolicited
- **WHEN** the modem uses the same record prefix in solicited and unsolicited contexts
- **THEN** routing uses active transaction expectations and context rather than prefix matching alone

### Requirement: Accepted operations terminate exactly once
Every accepted feature or direct operation SHALL eventually produce exactly one success, failure, cancellation, or timeout result. Progress SHALL survive fragmented input and separately arriving response and final-result records.

#### Scenario: Response and final result arrive separately
- **WHEN** response data arrives in one read and the final result arrives later
- **THEN** the transaction retains completion progress and completes only after all required records are present

#### Scenario: Reply timeout expires
- **WHEN** an accepted transaction does not receive its required reply before its configured deadline
- **THEN** its originating operation completes exactly once with `TimedOut`

### Requirement: Feature errors hide irrelevant AT internals
Feature boundaries SHALL translate lower-level modem, transport, protocol, timeout, capacity, and lifecycle outcomes into operation-specific feature result alternatives while preserving useful diagnostics.

#### Scenario: Modem rejects SMS submission
- **WHEN** an SMS workflow receives a `+CMS ERROR` final result
- **THEN** the SMS operation returns a typed SMS rejection containing the relevant modem code rather than exposing a generic response pack
