## Purpose

Defines explicit allocation-free operation outcomes that make success and failure handling visible and exhaustively matchable in C++20.

## ADDED Requirements

### Requirement: Fallible operations return typed variants
Every fallible public operation SHALL return a `Result<Success, Errors...>` whose alternatives represent one success type and the operation's declared failure types. The result object and all alternatives SHALL obey the core no-heap contract.

#### Scenario: Successful operation
- **WHEN** an operation succeeds
- **THEN** its result contains exactly the declared success alternative and owns the returned value

#### Scenario: Failed operation
- **WHEN** an operation fails for a declared reason
- **THEN** its result contains exactly the corresponding typed error alternative, preserving its documented diagnostic payload

### Requirement: Discarding a result is diagnosed
The result type SHALL be marked `[[nodiscard]]` with an actionable diagnostic. Supported verification builds SHALL treat result-discard diagnostics as errors.

#### Scenario: Caller ignores a result
- **WHEN** compiled client code discards a returned result without an explicit cast or suppression
- **THEN** the compiler emits the configured `nodiscard` diagnostic

### Requirement: Matching is exhaustive
The result SHALL provide `match()` operations implemented through variant visitation for const lvalues, mutable lvalues, and rvalues. A visitor that cannot handle every alternative SHALL fail compilation, and all handlers SHALL produce a compatible return type.

#### Scenario: Explicit handlers cover every alternative
- **WHEN** a client calls `match()` with one compatible handler for every result alternative
- **THEN** the handler for the active alternative runs and its value is returned

#### Scenario: Handler is omitted
- **WHEN** a client calls `match()` without a handler that accepts one of the alternatives
- **THEN** compilation fails

#### Scenario: Owned payload is consumed
- **WHEN** a client calls `match()` on an rvalue result containing a movable payload
- **THEN** the active payload can be moved into the selected handler without heap allocation by the result machinery

### Requirement: Initial API does not silently bypass errors
The initial result API SHALL NOT provide implicit boolean conversion, unchecked `value()` or `unwrap()`, default construction that implies success, or a public mutable variant escape hatch.

#### Scenario: Caller wants the success value
- **WHEN** a client needs to consume a successful operation value
- **THEN** it uses exhaustive matching and therefore defines behavior for all declared errors

### Requirement: Result alternatives are unambiguous
Result construction SHALL be explicit from a declared alternative, and duplicate alternative types SHALL be rejected at compile time.

#### Scenario: Duplicate error type is declared
- **WHEN** a result alias repeats the same alternative type
- **THEN** compilation fails with a diagnostic identifying the invalid result declaration
