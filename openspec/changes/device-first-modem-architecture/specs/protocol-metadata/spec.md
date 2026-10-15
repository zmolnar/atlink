## Purpose

Defines allocation-free typed protocol records and external metadata that let modem engineers describe AT wire formats without exposing parsing or formatting to clients.

## ADDED Requirements

### Requirement: Protocol records are value data
Command, response, URC, and feature payload types SHALL be value-owning data structures, standard-layout and trivially copyable wherever their contents permit. They SHALL NOT require inheritance from packet/visitor bases or contain codec methods, builders, dynamic strings, or dynamic collections.

#### Scenario: Client constructs a command
- **WHEN** a client or feature creates a typed command
- **THEN** it initializes ordinary bounded data members without manually constructing AT syntax

#### Scenario: Response outlives RX processing
- **WHEN** a transaction returns a response to its caller
- **THEN** the response owns its bounded data and contains no view into temporary RX or trial-parsing storage

### Requirement: Modem engineers define external protocol metadata
An integration engineer SHALL define external traits that associate commands with responses and describe prefixes, field order, delimiters, quoting, multiline structure, and URC identity. The metadata mechanism SHALL require no external reflection dependency, schema file, or code generation.

#### Scenario: Command response association
- **WHEN** a typed command is submitted
- **THEN** its response type is determined at compile time by external command metadata

#### Scenario: Codec processes fields
- **WHEN** the codec serializes or deserializes a protocol record
- **THEN** it applies the externally declared wire rules without requiring application-side string handling

### Requirement: Enum translation is bidirectional and explicit
Native enum members SHALL support numeric and textual modem representations through bidirectional metadata. A field SHALL be able to override its enum type's default representation, and unsupported incoming values SHALL follow an explicit unknown-value or decoding-error policy.

#### Scenario: Numeric enum is encoded
- **WHEN** a field uses numeric enum representation
- **THEN** serialization emits the mapped number and deserialization accepts only values allowed by the mapping policy

#### Scenario: Textual enum is decoded
- **WHEN** a modem emits a mapped textual enum value
- **THEN** deserialization stores the corresponding native enum without allocating a temporary string

#### Scenario: Same enum uses field override
- **WHEN** one modem field declares a representation different from the enum's default
- **THEN** the field-specific representation takes precedence for both serialization and deserialization

#### Scenario: Unknown enum value arrives
- **WHEN** input contains an enum value absent from the declared mapping
- **THEN** the codec returns the declared unknown representation or a typed decoding failure rather than applying an unchecked cast

### Requirement: Codec capacity failures are explicit
Serialization and deserialization SHALL use bounded caller- or object-owned storage. Insufficient capacity SHALL fail explicitly without growing storage, allocating, silently truncating a semantic value, or falling back to the heap.

#### Scenario: Encoded command exceeds TX capacity
- **WHEN** a complete command cannot fit in the configured TX buffer
- **THEN** execution returns an encoding/capacity failure and sends no partial command

#### Scenario: Incoming owned field exceeds capacity
- **WHEN** an incoming field cannot fit in its destination storage
- **THEN** decoding fails according to the field contract rather than returning a silently truncated value
