## Why

ATLink has useful bottom-up primitives for AT serialization, response parsing, URCs, and transactions, but its application boundary still exposes those mechanics and leaves device, feature, ownership, allocation, and error-handling concepts unclear. The architecture must be defined from the device downward before more modem functionality is added, so Linux, Zephyr, and FreeRTOS integrations can share one predictable, allocation-conscious core.

## What Changes

- **BREAKING** Replace the long-term application model of manually pairing commands, responses, and final-result packs with concrete modem devices that expose higher-level features such as network, SMS, call, and GNSS.
- Keep `Device` as the application-facing connected-modem concept; concrete modem types such as `Mc60` compose only their supported features.
- Add an advanced typed direct-AT interface that shares the same scheduler, routing, and transaction machinery as features rather than bypassing it.
- Introduce allocation-free, value-owning command, response, URC, and feature data structures, with external protocol traits maintained by modem integration engineers.
- Preserve bidirectional numeric and textual enum mappings, including field-specific encoding overrides and explicit unknown-value policy.
- Introduce `[[nodiscard]]` result variants with exhaustive `match()` handling through the overload visitor pattern; avoid unchecked success access as the default API.
- Give each feature its own typemachine workflow while retaining a shared AT transaction machine and one serialized device processing context.
- Define contextual routing: solicited responses return to the originating transaction, while URCs route independently to interested features.
- Separate compile-time platform selection from transport selection. Permit platform backends to allocate when appropriate while requiring core, protocol, feature, and device layers to avoid heap allocation.
- Define explicit construction, lifecycle, capacity, callback, cancellation, shutdown, and resource-lifetime contracts.
- Deliver the migration in sequential, review-gated branches and PRs; no phase begins before its predecessor is reviewed and merged.

## Capabilities

### New Capabilities

- `device-api`: Concrete device composition, feature access, lifecycle, ownership, callbacks, direct AT access, and routing semantics.
- `explicit-results`: Allocation-free success/error variants with `[[nodiscard]]` and exhaustive overload-based matching.
- `protocol-metadata`: POD-style protocol data plus external command, response, URC, field, and enum metadata with allocation-free codecs.
- `feature-machines`: Independent feature workflows coordinated through a shared AT executor and typed events/actions.
- `platform-portability`: Compile-time platform and transport wiring supporting dynamic Linux backends and fully static embedded backends.

### Modified Capabilities

None. This repository does not yet have baseline OpenSpec capabilities; the existing implementation will be migrated toward the new capabilities incrementally.

## Impact

- Public API: future `Mc60` device, feature interfaces, typed results, callbacks, and direct AT access.
- Core architecture: transaction scheduling, response assembly, URC routing, lifecycle, bounded storage, and machine ownership.
- Protocol authoring: external traits replace virtual packet behavior at the eventual application/protocol data boundary.
- Platform layer: Linux remains first, with contracts designed for Zephyr and FreeRTOS and no assumption that a platform owns a universal UART implementation.
- Existing implementation: retained while the new vertical slice is introduced; broad renaming and removal happen only after the replacement path is proven.
- Dependencies: no new reflection/schema dependency. C++20 standard facilities and the existing pinned `typemachine` dependency remain the basis, subject to a no-heap audit for core-side action storage.
- Delivery: phase 1 is represented by PR #1 (`phase/01-device-api-contract`); subsequent branches are blocked until that PR is reviewed and merged.
