// P1-T001 Core BIM Identity & Domain Model - ElementId proof (Implementation
// Brief BIM-TASK-P1-T001-CLAUDE v1.0 section 16.1). Links bim::model only;
// every case is pure value logic with no document, graph, persistence or
// geometry involvement.

#include "bim/model/element_id.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

using bim::model::ElementId;
using bim::model::GenerateElementId;
using bim::model::IsValid;
using bim::model::ParseElementId;
using bim::model::ToString;

namespace {

static_assert(sizeof(ElementId) == 16, "ElementId is exactly sixteen identity bytes");
static_assert(std::is_trivially_copyable_v<ElementId>, "ElementId is a plain value");
static_assert(std::is_standard_layout_v<ElementId>, "ElementId is a plain value");

ElementId MakeId(std::initializer_list<std::uint8_t> leading) {
    ElementId id;
    std::size_t i = 0;
    for (const std::uint8_t b : leading) {
        id.bytes[i++] = b;
    }
    return id;
}

ElementId SequentialId() {
    // 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff
    return MakeId({0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff});
}

constexpr const char* kSequentialText = "00112233-4455-6677-8899-aabbccddeeff";

} // namespace

TEST_CASE("ElementId: default-constructed and all-zero are invalid", "[unit][model][p1-t001][element-id]") {
    CHECK_FALSE(IsValid(ElementId{}));
    CHECK_FALSE(IsValid(MakeId({})));
    CHECK(ElementId{}.bytes == std::array<std::uint8_t, 16>{});
}

TEST_CASE("ElementId: any non-zero byte makes the identity valid", "[unit][model][p1-t001][element-id]") {
    for (std::size_t i = 0; i < 16; ++i) {
        ElementId id;
        id.bytes[i] = 0x01;
        INFO("non-zero byte at index " << i);
        CHECK(IsValid(id));
    }
    CHECK(IsValid(SequentialId()));
}

TEST_CASE("ElementId: equality compares all sixteen bytes", "[unit][model][p1-t001][element-id]") {
    CHECK(SequentialId() == SequentialId());
    CHECK(ElementId{} == ElementId{});
    for (std::size_t i = 0; i < 16; ++i) {
        ElementId changed = SequentialId();
        changed.bytes[i] = static_cast<std::uint8_t>(changed.bytes[i] ^ 0x01U);
        INFO("flipped byte index " << i);
        CHECK(changed != SequentialId());
    }
}

TEST_CASE("ElementId: ordering is lexicographic from byte 0, independent of host endianness",
          "[unit][model][p1-t001][element-id]") {
    ElementId low;
    low.bytes[15] = 0x01;
    ElementId high;
    high.bytes[15] = 0x02;
    CHECK(low < high);
    CHECK(high > low);
    CHECK_FALSE(high < low);
    CHECK(low <= low);

    // The first byte dominates every later byte: a leading 0x01 sorts after a
    // leading 0x00 even when every following byte favours the other value.
    ElementId leading_one = MakeId({0x01});
    ElementId leading_zero_rest_max = MakeId({0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
    CHECK(leading_zero_rest_max < leading_one);

    // A difference in a middle byte decides when earlier bytes are equal.
    ElementId a = MakeId({0x10, 0x20, 0x30});
    ElementId b = MakeId({0x10, 0x20, 0x31});
    CHECK(a < b);
    CHECK((a <=> b) == std::strong_ordering::less);
    CHECK((b <=> a) == std::strong_ordering::greater);
    CHECK((a <=> a) == std::strong_ordering::equal);
}

TEST_CASE("ElementId: generated identity is valid and carries the v4 version bits",
          "[unit][model][p1-t001][element-id][generator]") {
    const std::optional<ElementId> generated = GenerateElementId();
    REQUIRE(generated.has_value());
    CHECK(IsValid(*generated));
    CHECK((generated->bytes[6] >> 4U) == 0x4U);
    CHECK((generated->bytes[8] >> 6U) == 0x2U);
}

TEST_CASE("ElementId: every generated identity carries the v4 version and variant bits",
          "[unit][model][p1-t001][element-id][generator]") {
    // Bit-layout correctness checked on every sample - not uniqueness.
    for (int i = 0; i < 256; ++i) {
        const std::optional<ElementId> generated = GenerateElementId();
        REQUIRE(generated.has_value());
        INFO("sample " << i);
        CHECK(IsValid(*generated));
        CHECK((generated->bytes[6] >> 4U) == 0x4U);
        CHECK((generated->bytes[8] >> 6U) == 0x2U);
    }
}

TEST_CASE("ElementId: GenerateElementId has the explicit optional-return, noexcept contract",
          "[unit][model][p1-t001][element-id][generator]") {
    // Generation failure is representable only as an empty optional, and no
    // exception can escape the public function.
    static_assert(std::is_same_v<decltype(GenerateElementId()), std::optional<ElementId>>);
    static_assert(noexcept(GenerateElementId()));
    CHECK(GenerateElementId().has_value());
}

TEST_CASE("ElementId: canonical text is 36 lowercase characters with hyphens at 8, 13, 18, 23",
          "[unit][model][p1-t001][element-id][text]") {
    const std::string text = ToString(SequentialId());
    CHECK(text == kSequentialText);
    REQUIRE(text.size() == 36);
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        INFO("character index " << i);
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            CHECK(c == '-');
        } else {
            CHECK(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')));
        }
    }

    // Uppercase-looking nibbles are still emitted lowercase, and leading zero
    // nibbles are kept.
    CHECK(ToString(MakeId({0xAB, 0xCD, 0xEF, 0x01})) == "abcdef01-0000-0000-0000-000000000000");
    CHECK(ToString(ElementId{}) == "00000000-0000-0000-0000-000000000000");

    // Text follows byte order left to right regardless of host endianness.
    ElementId last_byte_only;
    last_byte_only.bytes[15] = 0x01;
    CHECK(ToString(last_byte_only) == "00000000-0000-0000-0000-000000000001");
}

TEST_CASE("ElementId: ElementId -> string -> ElementId round-trips", "[unit][model][p1-t001][element-id][text]") {
    const ElementId known = SequentialId();
    const auto reparsed = ParseElementId(ToString(known));
    REQUIRE(reparsed.has_value());
    CHECK(*reparsed == known);

    for (int i = 0; i < 16; ++i) {
        const auto generated = GenerateElementId();
        REQUIRE(generated.has_value());
        const auto round_trip = ParseElementId(ToString(*generated));
        REQUIRE(round_trip.has_value());
        CHECK(*round_trip == *generated);
    }
}

TEST_CASE("ElementId: parser accepts uppercase and mixed-case hexadecimal",
          "[unit][model][p1-t001][element-id][text]") {
    const auto upper = ParseElementId("00112233-4455-6677-8899-AABBCCDDEEFF");
    REQUIRE(upper.has_value());
    CHECK(*upper == SequentialId());

    const auto mixed = ParseElementId("00112233-4455-6677-8899-AaBbCcDdEeFf");
    REQUIRE(mixed.has_value());
    CHECK(*mixed == SequentialId());
}

TEST_CASE("ElementId: parser rejects malformed length", "[unit][model][p1-t001][element-id][text]") {
    CHECK_FALSE(ParseElementId("").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899-aabbccddeef").has_value());   // 35
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899-aabbccddeeff0").has_value()); // 37
    CHECK_FALSE(ParseElementId("00112233445566778899aabbccddeeff").has_value());      // 32, no hyphens
    CHECK_FALSE(ParseElementId(" 00112233-4455-6677-8899-aabbccddeeff").has_value()); // leading space
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899-aabbccddeeff ").has_value()); // trailing space
    CHECK_FALSE(ParseElementId("{00112233-4455-6677-8899-aabbccddeeff}").has_value()); // braced form
}

TEST_CASE("ElementId: parser rejects malformed separators", "[unit][model][p1-t001][element-id][text]") {
    // Each of the four hyphen positions replaced by another character.
    CHECK_FALSE(ParseElementId("00112233x4455-6677-8899-aabbccddeeff").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455x6677-8899-aabbccddeeff").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455-6677x8899-aabbccddeeff").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899xaabbccddeeff").has_value());
    // Right length, hyphens in the wrong places.
    CHECK_FALSE(ParseElementId("0011223-34455-6677-8899-aabbccddeeff").has_value());
    CHECK_FALSE(ParseElementId("00112233-445-56677-8899-aabbccddeeff").has_value());
    // Underscore and colon are not separators.
    CHECK_FALSE(ParseElementId("00112233_4455_6677_8899_aabbccddeeff").has_value());
    CHECK_FALSE(ParseElementId("00112233:4455:6677:8899:aabbccddeeff").has_value());
}

TEST_CASE("ElementId: parser rejects non-hexadecimal characters", "[unit][model][p1-t001][element-id][text]") {
    CHECK_FALSE(ParseElementId("0011223g-4455-6677-8899-aabbccddeeff").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899-aabbccddeefg").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899-aabbccddeeZ0").has_value());
    CHECK_FALSE(ParseElementId("00112233-4455-6677-8899-aabbccdd eeff").has_value());
    CHECK_FALSE(ParseElementId("0x112233-4455-6677-8899-aabbccddeeff").has_value());

    // An embedded NUL is a character like any other and is not hexadecimal.
    std::string with_nul = kSequentialText;
    with_nul[3] = '\0';
    CHECK(with_nul.size() == 36);
    CHECK_FALSE(ParseElementId(std::string_view(with_nul)).has_value());
}

TEST_CASE("ElementId: parser rejects the all-zero textual identity", "[unit][model][p1-t001][element-id][text]") {
    CHECK_FALSE(ParseElementId("00000000-0000-0000-0000-000000000000").has_value());
}

TEST_CASE("ElementId: parser accepts a valid non-zero identity without v4 version/variant bits",
          "[unit][model][p1-t001][element-id][text]") {
    // Version nibble 6 / variant 0x88 (10xx) - not a v4 generator layout.
    const auto sequential = ParseElementId(kSequentialText);
    REQUIRE(sequential.has_value());
    CHECK((sequential->bytes[6] >> 4U) != 0x4U);

    // Smallest possible non-zero identity: a single low bit.
    const auto minimal = ParseElementId("00000000-0000-0000-0000-000000000001");
    REQUIRE(minimal.has_value());
    CHECK(IsValid(*minimal));
    CHECK(minimal->bytes[15] == 0x01);

    // Variant bits not 10 and version nibble 0xF.
    const auto all_ones = ParseElementId("ffffffff-ffff-ffff-ffff-ffffffffffff");
    REQUIRE(all_ones.has_value());
    CHECK(IsValid(*all_ones));
    CHECK(ToString(*all_ones) == "ffffffff-ffff-ffff-ffff-ffffffffffff");
}
