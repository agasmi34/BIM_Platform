#include "bim/model/element_id.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <string_view>

// P1-T001 Core BIM Identity & Domain Model - ElementId generation and
// canonical text conversion (Implementation Brief BIM-TASK-P1-T001-CLAUDE
// v1.0 section 4). Standard library only; no third-party UUID facility.

namespace bim::model {

namespace {

constexpr std::size_t kTextLength = 36;

// Character positions (0-based) of the four hyphens in the canonical form
// xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx.
[[nodiscard]] constexpr bool IsHyphenPosition(std::size_t index) noexcept {
    return index == 8 || index == 13 || index == 18 || index == 23;
}

// Returns the value 0..15 of a hexadecimal digit (either case), or -1 for any
// other character.
[[nodiscard]] constexpr int HexValue(char c) noexcept {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

} // namespace

std::optional<ElementId> GenerateElementId() noexcept {
    // Every identity byte is drawn individually through the standard
    // distribution facility, so nothing here assumes how many bits a single
    // std::random_device call delivers. Constructing or using the device may
    // throw; the public contract is noexcept and fail-closed, so any
    // exception is contained and reported as an empty optional.
    try {
        std::random_device device;
        std::uniform_int_distribution<unsigned int> byte_distribution(0U, 255U);

        ElementId id;
        for (std::uint8_t& byte : id.bytes) {
            byte = static_cast<std::uint8_t>(byte_distribution(device));
        }

        // UUID-v4-style layout: version nibble (high nibble of byte 6) = 4;
        // variant (two high bits of byte 8) = binary 10. The version bits
        // alone make the result non-zero whatever the entropy supplied.
        id.bytes[6] = static_cast<std::uint8_t>((id.bytes[6] & 0x0FU) | 0x40U);
        id.bytes[8] = static_cast<std::uint8_t>((id.bytes[8] & 0x3FU) | 0x80U);
        return id;
    } catch (...) {
        return std::nullopt;
    }
}

std::string ToString(const ElementId& id) {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string text;
    text.reserve(kTextLength);
    std::size_t byte_index = 0;
    for (std::size_t i = 0; i < kTextLength; ++i) {
        if (IsHyphenPosition(i)) {
            text.push_back('-');
            continue;
        }
        // Two characters per byte: the high nibble first, then the low
        // nibble, walking the bytes from index 0 to index 15.
        const std::uint8_t byte = id.bytes[byte_index / 2];
        const unsigned nibble = (byte_index % 2 == 0) ? (byte >> 4U) : (byte & 0x0FU);
        text.push_back(kDigits[nibble]);
        ++byte_index;
    }
    return text;
}

std::optional<ElementId> ParseElementId(std::string_view text) noexcept {
    if (text.size() != kTextLength) {
        return std::nullopt;
    }
    ElementId id;
    std::size_t digit_index = 0;
    for (std::size_t i = 0; i < kTextLength; ++i) {
        const char c = text[i];
        if (IsHyphenPosition(i)) {
            if (c != '-') {
                return std::nullopt;
            }
            continue;
        }
        const int value = HexValue(c);
        if (value < 0) {
            return std::nullopt;
        }
        std::uint8_t& byte = id.bytes[digit_index / 2];
        if (digit_index % 2 == 0) {
            byte = static_cast<std::uint8_t>(value << 4);
        } else {
            byte = static_cast<std::uint8_t>(byte | static_cast<std::uint8_t>(value));
        }
        ++digit_index;
    }
    if (!IsValid(id)) {
        return std::nullopt;
    }
    return id;
}

} // namespace bim::model
