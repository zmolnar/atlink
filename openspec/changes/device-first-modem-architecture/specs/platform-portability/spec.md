## Purpose

Defines compile-time platform and transport contracts that permit Linux convenience backends and fully static embedded deployments without changing portable core behavior.

## ADDED Requirements

### Requirement: Core and higher layers do not allocate
Codecs, protocol helpers and data, the AT executor, devices, features, feature machines, and core-side event/completion/action storage SHALL perform no heap allocation during construction, operation, error handling, or shutdown.

#### Scenario: Static backend runs a device workflow
- **WHEN** a device is instantiated with a fully static platform and transport backend
- **THEN** startup, a feature operation, URC delivery, and shutdown complete with zero heap allocations in the complete system

#### Scenario: Capacity is exhausted
- **WHEN** configured core capacity is unavailable
- **THEN** the request returns a typed capacity failure without growing storage, allocating, silently dropping an accepted terminal event, or using a heap fallback

### Requirement: Platform backends choose their allocation policy
The platform contract SHALL permit a backend to use dynamic allocation internally but SHALL NOT require it. Core interfaces SHALL NOT require heap-owned pointers, allocating callables, resizable containers, or allocation-capable platform implementations.

#### Scenario: Linux uses dynamic resources
- **WHEN** the selected Linux platform chooses heap-backed thread or queue implementation details
- **THEN** it remains compatible with the core capacity and outcome contract without making core allocate

#### Scenario: RTOS uses static resources
- **WHEN** the selected Zephyr or FreeRTOS platform uses caller-supplied task stacks, queues, and control blocks
- **THEN** it implements the same platform contract without heap allocation

### Requirement: Platform and transport are separate compile-time choices
The build SHALL select platform execution services and modem byte transport independently at compile time. Selecting one platform SHALL NOT require headers or implementations from another platform.

#### Scenario: Zephyr UART integration
- **WHEN** an integration selects Zephyr execution services and a board-specific UART transport
- **THEN** the device composes those choices without depending on Linux or assuming Zephyr supplies a universal modem transport

#### Scenario: In-memory test transport
- **WHEN** tests select a deterministic in-memory transport with a test platform
- **THEN** device and feature behavior can be exercised without a serial device or production OS backend

### Requirement: Capacities are compile-time configuration
An integration engineer SHALL provide compile-time capacities for RX bytes, TX bytes, pending operations, and queued events, with additional feature capacities added as needed. The implementation SHALL reserve enough control capacity to deliver cancellation and shutdown outcomes when normal request capacity is full.

#### Scenario: Integration defines limits
- **WHEN** a device type is instantiated
- **THEN** its relevant core storage bounds are determined at compile time and no implicit dynamic expansion is available

#### Scenario: Queue is full during stop
- **WHEN** ordinary event capacity is exhausted and shutdown begins
- **THEN** reserved control capacity or an equivalent bounded mechanism still permits accepted operations to receive terminal outcomes and the device to become quiescent

### Requirement: Restricted-context callbacks only enqueue work
Transport, timer, and interrupt-context callbacks SHALL not advance feature workflows directly. They SHALL perform only backend-safe notification needed to enqueue or signal work for the serialized device context.

#### Scenario: UART ISR reports input
- **WHEN** an embedded UART interrupt indicates received bytes
- **THEN** it signals bounded work for the device context without parsing, invoking feature callbacks, blocking, or allocating in core
