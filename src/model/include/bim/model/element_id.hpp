#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// P1-T001 Core BIM Identity & Domain Model - durable, project-owned element
// identity (Architecture Gate BIM-AG-P1-T001 v1.0 sections 3-6; Implementation
// Brief BIM-TASK-P1-T001-CLAUDE v1.0 section 4).
//
// ElementId is a 128-bit value stored as sixteen ordered bytes. The byte
// order is the canonical left-to-right order used by the textual form below
// and by any future persistence representation; it is never reinterpreted
// through a host-endian integer, so equality, ordering and text conversion
// behave identically on every platform. The all-zero value is the single
// invalid identity. ElementId carries no pointer, database row id, kernel or
// GPU identity, external-format identifier, or dependency-graph node id.
//
// This header lives under bim_model's public surface and is scanned word for
// word by the mechanical architecture checker, so it stays strictly
// vendor-neutral and standard-library-only.

namespace bim::model {

struct ElementId {
    std::array<std::uint8_t, 16> bytes{};

    // Defaulted three-way comparison over a std::array of bytes is a
    // lexicographic comparison from byte 0 to byte 15: a deterministic,
    // host-endian-independent total order.
    friend constexpr bool operator==(const ElementId&, const ElementId&) noexcept = default;
    friend constexpr auto operator<=>(const ElementId&, const ElementId&) noexcept = default;
};

// True when at least one identity byte is non-zero. The all-zero value (which
// is also what a default-constructed ElementId holds) is invalid.
[[nodiscard]] constexpr bool IsValid(const ElementId& id) noexcept {
    for (const std::uint8_t byte : id.bytes) {
        if (byte != 0) {
            return true;
        }
    }
    return false;
}

// Generates a fresh UUID-v4-style identity: 128 random bits from the standard
// library's non-deterministic entropy source, with the version nibble set to 4
// and the variant bits set to binary 10. Generation failure is reported as an
// empty optional; no exception from the entropy provider escapes this
// function. The result is never all-zero (the version bits alone guarantee
// that). There is deliberately no injectable entropy hook in this public API.
[[nodiscard]] std::optional<ElementId> GenerateElementId() noexcept;

// Canonical text form: exactly 36 characters, lowercase hexadecimal, hyphens
// at character positions 8, 13, 18 and 23 (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).
[[nodiscard]] std::string ToString(const ElementId& id);

// Parses the canonical text form. Upper- and lowercase hexadecimal are both
// accepted. Anything malformed - wrong length, misplaced or missing hyphens,
// a non-hexadecimal character - and the all-zero identity fail closed with an
// empty optional. A valid non-zero identity is accepted whether or not it
// carries the version/variant bits the generator produces: the durable
// identity space is the full 128-bit value, not only generator output.
[[nodiscard]] std::optional<ElementId> ParseElementId(std::string_view text) noexcept;


} // namespace bim::model
