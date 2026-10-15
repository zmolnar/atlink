## Context

See `proposal.md` for motivation and the capability specs for observable requirements.

The current header-only C++20 implementation was built bottom-up. It has field objects with virtual parse/stringify methods, visitor-driven packets, response variants, URC parsing, Linux platform facades, and a recently introduced typemachine transaction FSM. The public `Core::Device::sendCommand()` still asks callers to coordinate command, response, and final-result objects, while `Orchestrator` combines scheduling, parsing, routing, synchronization, buffering, and transport effects.

The target must support Linux first and later Zephyr and FreeRTOS. Core and all higher layers must avoid heap allocation; platform and transport backends may allocate but must also permit fully static implementations. The existing behavior remains available during migration so the replacement can be validated as a vertical slice before broad removal or renaming.

## Goals / Non-Goals

**Goals:**

- Establish a top-down application boundary centered on concrete modem devices and higher-level features.
- Keep public and protocol payloads bounded, value-owning, and POD-style wherever practical.
- Make operation failures explicit and naturally exhaustive to handle.
- Separate feature workflow policy, shared AT transaction policy, protocol codecs/routing, transport, and platform execution.
- Provide deterministic bounded resource behavior suitable for static embedded deployments.
- Preserve typed direct AT access as an advanced path through the same coordination machinery.
- Migrate through independently reviewable, sequential phases.

**Non-Goals:**

- Implement every MC60 feature in the initial vertical slice.
- Build a general C++ reflection framework, introduce schema files, or add code generation.
- Guarantee Rust's language-level must-use semantics in C++20 beyond `[[nodiscard]]`, compile diagnostics, and exhaustive visitation.
- Require Linux to avoid all platform-side allocation.
- Treat each feature machine as a separate thread or task.
- Let raw direct AT access bypass scheduler or routing policy.
- Remove old abstractions before their replacement has been exercised end to end.

## Decisions

### 1. `Device` is the application-facing composition root

A concrete modem type such as `Devices::Mc60<Platform, Transport, Limits>` owns logical runtime state, features, feature machines, the shared executor, routing, and bounded operation slots. It exposes references such as `device.network()` and advanced `device.at()` access.

The device borrows transport and caller-owned platform execution resources. This makes storage and lifetimes visible, lets static RTOS integrations provide control blocks and stacks, and lets Linux resources allocate internally if desired. Construction binds resources only; `start()` and `stop()` perform fallible lifecycle work.

**Alternatives considered:**

- Rename the public object to `Client`: rejected because clients are applications using a physical/logical modem device, and the device also composes persistent capabilities and state.
- Expose AT commands as the primary API: retained only as advanced access because normal users should operate in SMS, network, call, and GNSS concepts.
- Let device construction start I/O: rejected because construction cannot report rich failure and hides resource activation.

### 2. There are two typed data boundaries

The application-feature boundary uses domain values such as signal quality, messages, call state, and positions. The feature-executor boundary uses command, response, final-result, and URC records. Both use bounded value ownership; feature results translate lower-level outcomes rather than exposing response packs.

This separation allows a feature operation to coordinate several AT transactions without leaking that workflow. It also makes success semantics precise: for example, SMS submission acceptance is not delivery.

### 3. Protocol data is plain; external traits describe wire behavior

Protocol records contain native integers, enums, bounded text, and bounded collections. They do not inherit packet classes or define virtual `accept()` methods. Integration engineers specialize ATLink-owned traits beside the modem support package to declare command-response association, tags/prefixes, field order, quoting, multiline records, and URC identity.

The trait layer uses standard C++20 templates, tuples/member pointers or equivalent small descriptors. It is intentionally engineer-facing; ordinary device clients never define it. Separate traits are preferred to in-struct metadata because the design explicitly keeps user-facing data free from wire and library machinery.

**Alternatives considered:**

- Macros: concise but hide generated behavior and weaken diagnostics/tooling; they may later wrap the macro-free interface but are not foundational.
- In-class metadata: keeps definitions together but couples otherwise plain data to ATLink.
- External reflection/schema dependency: rejected; explicit C++ metadata is sufficient and avoids another dependency.
- Keep visitor inheritance: useful during migration but rejected as the eventual data boundary because it combines data with codec behavior and runtime polymorphism.

### 4. Enum mapping remains first-class

Native enum members use bidirectional metadata. Numeric enum values are validated against a declared policy rather than accepted by unchecked cast. Text mappings are allocation-free static records. Each enum can have a default representation, and individual fields can override it when the same semantic enum is represented differently by a modem command.

Unknown values must be modeled explicitly: either map to a declared `Unknown` representation that preserves required raw information, or return a decoding failure. The integration engineer chooses the policy in metadata.

### 5. Results wrap a private inline variant

`Result<Success, Errors...>` owns `std::variant<Success, Errors...>` inline, is `[[nodiscard]]`, and provides ref-qualified `match()` overloads. `Utils::Overload` remains the user-facing visitor helper. Compile-time checks reject duplicate alternatives and visitors that do not accept all alternatives.

The initial API deliberately omits implicit bool conversion, unchecked success extraction, and public mutable variant access. Explicitly typed overloads force source updates when an alternative changes; callers can knowingly use a generic handler to group errors.

Feature-specific results translate executor errors into domain-specific alternatives while retaining useful bounded diagnostics. Exceptions are not used for operational failures.

**Alternatives considered:**

- Return `bool` plus output parameters: rejected because it discards error identity and makes ignored errors easy.
- Expose `std::variant` directly: rejected because it cannot enforce result invariants or provide a focused must-handle interface.
- `std::expected`: unavailable in the project's C++20 target and naturally models one error type rather than the requested error-alternative pattern.
- Destructor assertions for unhandled results: rejected as runtime punishment that does not provide Rust-like static guarantees.

### 6. One serialized device context advances many machines

The device initially has one serialized event-processing context. Linux may implement it with one thread; Zephyr and FreeRTOS may use one configured task. Feature machines and the AT transaction machine are independent stateful objects but not independent threads.

Platform/transport callbacks only signal or enqueue bounded events. The device context decodes and routes input, advances machines, and invokes typed application event callbacks. Feature machines emit actions and consume typed completions; they never parse strings, touch transport, or block the device context.

Blocking application APIs are wrappers around this event-driven core. They reject calls from the device context with `WrongExecutionContext` and wait using platform completion resources outside that context.

**Alternatives considered:**

- Thread/task per feature: rejected due to synchronization complexity, embedded cost, and nondeterminism.
- Let feature machines call transport synchronously: rejected because it couples workflow policy to I/O and prevents shared scheduling.

### 7. Shared AT executor separates transaction policy from feature policy

The shared executor owns bounded admission, command scheduling, command-sequence coordination, one active wire transaction as supported by the modem, reply deadlines, incremental response assembly, and typed completion delivery. The transaction typemachine decides transitions and emits actions; runtime storage owns platform resources and operation lifetimes.

Feature machines determine multi-command workflows. Direct commands use the same executor. An explicit coordination policy protects command sequences that must not be interleaved; features do not lock transport themselves.

Solicited data routes according to active transaction expectations. Final results terminate the transaction. URCs route independently to all interested features, including while a transaction is active. Routing uses context as well as decoded type because the same modem prefix can be solicited or unsolicited.

### 8. Allocation policy has a strict layer boundary

Core, codecs, protocol records, executor, routing, devices, feature machines, action/event/completion storage, and results do not use heap allocation. Bounded arrays, inline variants, fixed-capacity containers, static metadata, function-pointer/context callbacks, and explicitly scoped views are used instead.

`Limits` provides compile-time capacities such as RX bytes, TX bytes, pending operations, and queued events. Capacity exhaustion returns a typed result; it never silently truncates a semantic value, drops an accepted terminal event, or grows storage. Shutdown/cancellation needs reserved control capacity or an equivalent bounded path.

Platform and transport sit below that boundary. Their contracts permit either dynamically allocated Linux resources or caller-supplied static RTOS resources. End-to-end no-heap operation is proven with a static test platform. The pinned typemachine implementation and all transitive core-side storage must be audited; a heap-backed machine action collection would violate the boundary even though typemachine is an external dependency.

### 9. Platform and transport are independent compile-time policies

Platform supplies execution, synchronization, timers/clock, logging, and caller-wait primitives. Transport supplies bytes and readiness notifications. A build selects each independently, and selected-platform facades must not include unrelated platform headers.

FreeRTOS and Zephyr do not imply a UART implementation; board/SDK integrations supply transports. A deterministic in-memory transport and static test platform provide architecture-level tests without hardware.

### 10. Delivery is a review-gated vertical migration

Each phase is based on the merged predecessor, has a dedicated branch and PR, and stops for user review. There are no stacked phase branches and no automatic merge.

1. Device/API contract and usage sketches.
2. Allocation-free explicit result primitive.
3. Feature data and operation-specific outcome payloads.
4. Device composition and platform/transport seams, with Linux wiring.
5. Network feature vertical slice with signal quality and registration events.
6. External protocol metadata and shared AT executor integration.
7. End-to-end verification, Linux example migration, and audience-specific documentation.

The first network slice is intentionally small but exercises both solicited completion and unsolicited routing. Broad file moves, naming cleanup, and removal of legacy packet abstractions wait until the replacement is demonstrated.

## Risks / Trade-offs

- **[Template diagnostics become complex]** → Keep metadata vocabulary small, add concepts/static assertions with domain-specific messages, and compile representative misuse tests.
- **[Fixed capacities can reject bursts]** → Make every bound an integration setting, expose `CapacityExceeded`, document sizing, and verify reserved terminal-event capacity.
- **[Large inline variants increase object size]** → Keep error payloads compact and operation-specific; measure device/slot sizes and avoid a universal error union.
- **[`[[nodiscard]]` is weaker than Rust's must-use semantics]** → Treat warnings as errors in supported checks, center `match()` in examples, and avoid bypass APIs.
- **[Blocking wrappers can deadlock through callbacks]** → Detect the device context and return `WrongExecutionContext`; document callback execution and reentrancy.
- **[Direct AT can invalidate feature assumptions]** → Route it through the executor and require explicit integration policy for feature-owned configuration.
- **[External traits split data and wire definitions]** → Accept the separation as deliberate engineer-facing complexity; keep traits discoverable beside modem protocol types and provide worked examples.
- **[Old and new APIs coexist temporarily]** → Migrate one vertical slice, mark review boundaries clearly, and remove legacy paths only after equivalent coverage exists.
- **[Platform allocation can obscure core allocations]** → Use allocation instrumentation and a fully static test backend to distinguish permitted backend allocation from forbidden transitive core allocation.

## Migration Plan

1. Merge the phase-1 device/API contract after review. Current status: PR #1 is open from `phase/01-device-api-contract`; it contains declaration-only review artifacts and does not change runtime behavior.
2. Create phase 2 from the merged `main`, implement and verify `Result`, and stop for review.
3. Repeat the merge-gated sequence for feature outcomes and device composition.
4. Build the network vertical slice against deterministic platform/transport test doubles before wiring existing codecs and executor behavior.
5. Introduce external protocol metadata for only the records needed by the slice; retain adapters to legacy protocol definitions where they lower migration risk.
6. Integrate shared executor/routing, fixing only behavior required by the new contracts, including fragmented response progress and exactly-once completion.
7. Migrate the Linux example and documentation, then progressively convert remaining commands/features.
8. Remove superseded packet/visitor/orchestrator interfaces and perform broad naming/file cleanup only when no supported path depends on them.

Rollback is phase-local: because every phase starts from a reviewed merge and the old runtime remains during the vertical migration, an unmerged phase branch can be abandoned without reverting prior accepted contracts. Any post-merge incompatible contract correction requires a follow-up reviewed change rather than silently rewriting later phases.
