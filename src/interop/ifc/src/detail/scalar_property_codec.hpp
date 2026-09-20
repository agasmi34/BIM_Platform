#pragma once

// Private, first-party, dependency-free (no IfcOpenShell, no Boost) codec
// for this Phase-0 spike's simplified scalar-property encoding (see
// bim::ifc::IfcProjectSeed's own doc comment in
// src/interop/ifc/include/bim/ifc/probe.hpp for why a full
// IfcPropertySet/IfcRelDefinesByProperties graph was deliberately not used
// here). Deterministic (name, value) pairs are encoded into a single
// std::string carried as the element's IfcText Description attribute, and
// decoded back out on reopen. Kept as a standalone, IfcOpenShell-free
// translation unit specifically so it can be exercised and validated with
// plain C++ in an environment that has no IfcOpenShell/Boost dependency
// available (this file has no third-party include of any kind).
//
// Format: "<n>|" followed by n entries of
// "<esc(name)>=<esc(value)>;" where esc() backslash-escapes any literal
// '\\', '=', ';', or '|' byte in `name`/`value`. The leading count makes
// decoding unambiguous even for an empty properties list ("0|") and guards
// against a value that happens to contain an unescaped trailing delimiter
// being silently swallowed.

#include <string>
#include <utility>
#include <vector>

namespace bim::ifc::detail {

using ScalarProperties = std::vector<std::pair<std::string, std::string>>;

// Never fails on well-formed input; always produces a decodable string.
[[nodiscard]] std::string EncodeScalarProperties(const ScalarProperties& properties);

// Returns false (leaving `out` unspecified) only if `encoded` is not a
// well-formed EncodeScalarProperties() output - this codec is private and
// entirely first-party-controlled, so a decode failure here indicates a
// genuine round-trip defect, never third-party input.
[[nodiscard]] bool DecodeScalarProperties(const std::string& encoded, ScalarProperties& out);

} // namespace bim::ifc::detail
