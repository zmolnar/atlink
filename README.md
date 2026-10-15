# ATLink

ATLink is a C++20 library for typed AT-modem communication, inspired by Rust's
`atat`. Protocol definitions describe command, response, and URC data; the library
handles the wire representation and transaction management.

The application-facing direction is a **device with features**: an application
uses an MC60's network, SMS, call, or GNSS interface. Direct typed AT access remains
available for advanced use. Device and feature workflows use `typemachine`.

## Device-first API design

[Device API contract](docs/device-api.md) defines the proposed public API,
ownership, lifecycle, explicit result handling, and allocation boundary. Its
[reference declarations](docs/contracts/device-api.h) are for API review, not an
implemented or installed interface.

Core, protocol, feature, and device code must not allocate from the heap. Platform
backends may allocate, but must also be replaceable by fully static backends.

The existing implementation lives under `atlink/atlink/`; it is being migrated in
reviewed phases. The current Linux example uses the existing `Core::Device` API.
See the contract's implementation sequence for the migration order.
