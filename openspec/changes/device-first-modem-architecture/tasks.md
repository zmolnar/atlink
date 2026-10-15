## 1. Phase 1 — Device API Contract

- [x] 1.1 Define the device/feature vocabulary, two typed boundaries, lifecycle, ownership, callback, and direct-AT contracts.
- [x] 1.2 Define the no-heap boundary for core and higher layers while permitting dynamic or static platform backends.
- [x] 1.3 Add declaration-only C++20 reference signatures and verify them with GCC and Clang warning-as-error checks.
- [x] 1.4 Create and publish `phase/01-device-api-contract` as PR #1 without changing runtime behavior.
- [ ] 1.5 Complete user review and merge PR #1; do not create the phase-2 branch before this gate passes.

## 2. Phase 2 — Explicit Results

- [ ] 2.1 Create `phase/02-explicit-results` from the merged phase-1 `main`.
- [ ] 2.2 Implement allocation-free `Result<Success, Errors...>` with explicit alternative construction and duplicate-type diagnostics.
- [ ] 2.3 Implement const-lvalue, mutable-lvalue, and rvalue `match()` using the existing overload visitor pattern.
- [ ] 2.4 Add compile-success and compile-failure coverage for exhaustive handling, handler return compatibility, move-only payloads, and duplicate alternatives.
- [ ] 2.5 Verify `[[nodiscard]]` diagnostics and the absence of implicit bool conversion, unchecked extraction, default success, and public mutable variant access.
- [ ] 2.6 Instrument result construction, movement, matching, and destruction to verify no heap allocation.
- [ ] 2.7 Open the phase-2 PR, stop implementation, and wait for successful review and merge.

## 3. Phase 3 — Feature Data and Outcomes

- [ ] 3.1 Create `phase/03-feature-outcomes` from the merged phase-2 `main`.
- [ ] 3.2 Define bounded, value-owning common lifecycle/capacity/cancellation/timeout payloads.
- [ ] 3.3 Define network signal-quality and registration-event values with explicit unavailable/unknown semantics.
- [ ] 3.4 Define operation-specific network and direct-AT error alternatives with bounded diagnostic payloads.
- [ ] 3.5 Define and test lower-level-to-feature error translation, including modem rejection, transport failure, and protocol failure.
- [ ] 3.6 Verify public payload standard-layout/trivial-copy properties where applicable and verify no owned views escape temporary storage.
- [ ] 3.7 Open the phase-3 PR, stop implementation, and wait for successful review and merge.

## 4. Phase 4 — Device and Platform Composition

- [ ] 4.1 Create `phase/04-device-composition` from the merged phase-3 `main`.
- [ ] 4.2 Define compile-time `Limits` validation and bounded storage for RX, TX, pending operations, ordinary events, and reserved control delivery.
- [ ] 4.3 Separate platform execution/synchronization contracts from transport byte-I/O contracts without including inactive platform headers.
- [ ] 4.4 Implement caller-owned device execution resources that support both dynamically implemented Linux resources and fully static test/RTOS resources.
- [ ] 4.5 Implement noncopyable/nonmovable `Mc60` composition, resource binding, feature/direct-access handles, and explicit start/stop state.
- [ ] 4.6 Implement device-context detection and callback quiescence/lifetime rules.
- [ ] 4.7 Add deterministic static platform and in-memory transport test doubles and verify construction performs no I/O or activation.
- [ ] 4.8 Add lifecycle, admission-closure, restart-cycle, resource-lifetime, and full-queue shutdown tests.
- [ ] 4.9 Open the phase-4 PR, stop implementation, and wait for successful review and merge.

## 5. Phase 5 — Network Feature Vertical Slice

- [ ] 5.1 Create `phase/05-network-feature` from the merged phase-4 `main`.
- [ ] 5.2 Define typed network-machine states, requests, transaction-completion events, registration events, lifecycle events, and actions.
- [ ] 5.3 Implement nonblocking signal-quality workflow progression and exactly-once completion into bounded operation storage.
- [ ] 5.4 Implement registration-event state updates and non-owning application callback dispatch on the device context.
- [ ] 5.5 Implement rejection of blocking lifecycle, feature, and direct operations from the device callback/event context.
- [ ] 5.6 Test concurrent submissions, capacity rejection before command submission, cancellation, timeout, callback context, and callback quiescence.
- [ ] 5.7 Open the phase-5 PR, stop implementation, and wait for successful review and merge.

## 6. Phase 6 — Protocol Metadata and AT Integration

- [ ] 6.1 Create `phase/06-at-integration` from the merged phase-5 `main`.
- [ ] 6.2 Design the minimal macro-free external metadata descriptors for no-response commands, fielded commands, responses, multiline records, and URCs.
- [ ] 6.3 Implement bounded value-owning text/collection types and allocation-free serializers/deserializers driven by metadata.
- [ ] 6.4 Preserve numeric and textual enum mapping with validation, field-level representation overrides, and explicit unknown-value policy.
- [ ] 6.5 Define typed metadata for the signal-quality command/response and network-registration URC used by the vertical slice.
- [ ] 6.6 Adapt the transaction machine into a shared bounded executor with scheduling, cooldown, reply timeout, and explicit sequence coordination.
- [ ] 6.7 Implement incremental response/final-result assembly across fragmented reads and contextual solicited/URC routing.
- [ ] 6.8 Connect feature-generated and direct commands to the same admission, executor, routing, cancellation, and shutdown paths.
- [ ] 6.9 Audit typemachine action storage and all transitive core-side paths; replace or configure any heap-backed storage.
- [ ] 6.10 Test enum mappings, codec capacities, no partial writes, fragmented replies, interleaved URCs, shared prefixes, direct access, timeout, and exactly-once completion.
- [ ] 6.11 Run allocation instrumentation with the fully static backend and distinguish permitted Linux-platform allocation from forbidden core allocation.
- [ ] 6.12 Open the phase-6 PR, stop implementation, and wait for successful review and merge.

## 7. Phase 7 — End-to-End Adoption

- [ ] 7.1 Create `phase/07-device-example` from the merged phase-6 `main`.
- [ ] 7.2 Migrate the Linux example to construct `Mc60`, use the network feature, handle results exhaustively, receive a typed registration event, and shut down explicitly.
- [ ] 7.3 Add end-to-end scripted-transport tests for success, modem rejection, transport/protocol failure, capacity exhaustion, timeout, cancellation, interleaved URCs, restart, and shutdown with pending work.
- [ ] 7.4 Document application-facing device/feature usage separately from modem-engineer protocol/feature integration.
- [ ] 7.5 Register all tests with CTest and run the legacy and new suites on supported compilers.
- [ ] 7.6 Measure bounded object/storage sizes and document limit-sizing guidance for embedded integrations.
- [ ] 7.7 Open the phase-7 PR, stop implementation, and wait for successful review and merge.

## 8. Post-Vertical-Slice Cleanup

- [ ] 8.1 Inventory remaining users of virtual packet visitors, `ResponsePack`, raw `AUrcDispatcher`, `Core::Device::sendCommand`, and `Orchestrator`.
- [ ] 8.2 Migrate remaining modem commands and features incrementally to external metadata and typed device features.
- [ ] 8.3 Remove duplicate/superseded field headers and legacy APIs only after no supported path depends on them.
- [ ] 8.4 Reorganize public versus internal headers and standardize the agreed names without changing accepted behavior.
- [ ] 8.5 Validate and archive the OpenSpec change after all capability requirements are implemented and reviewed.
