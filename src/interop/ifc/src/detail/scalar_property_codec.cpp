#include "detail/scalar_property_codec.hpp"

#include <cstddef>

namespace bim::ifc::detail {

namespace {

void AppendEscaped(std::string& out, const std::string& raw) {
    for (const char c : raw) {
        if (c == '\\' || c == '=' || c == ';' || c == '|') {
            out += '\\';
        }
        out += c;
    }
}

// Reads one escaped field starting at `pos` (index into `encoded`), stopping
// at the first unescaped occurrence of `stop`. Advances `pos` past the
// consumed stop character. Returns false if `stop` is never found.
bool ReadEscapedField(const std::string& encoded, std::size_t& pos, char stop, std::string& out) {
    out.clear();
    const std::size_t n = encoded.size();
    while (pos < n) {
        const char c = encoded[pos];
        if (c == '\\' && pos + 1 < n) {
            out += encoded[pos + 1];
            pos += 2;
            continue;
        }
        if (c == stop) {
            ++pos;
            return true;
        }
        out += c;
        ++pos;
    }
    return false;
}

} // namespace

std::string EncodeScalarProperties(const ScalarProperties& properties) {
    std::string out = std::to_string(properties.size());
    out += '|';
    for (const auto& [name, value] : properties) {
        AppendEscaped(out, name);
        out += '=';
        AppendEscaped(out, value);
        out += ';';
    }
    return out;
}

bool DecodeScalarProperties(const std::string& encoded, ScalarProperties& out) {
    out.clear();

    const std::size_t bar_pos = encoded.find('|');
    if (bar_pos == std::string::npos) {
        return false;
    }

    std::size_t count = 0;
    for (std::size_t i = 0; i < bar_pos; ++i) {
        const char c = encoded[i];
        if (c < '0' || c > '9') {
            return false;
        }
        count = (count * 10) + static_cast<std::size_t>(c - '0');
    }

    std::size_t pos = bar_pos + 1;
    for (std::size_t i = 0; i < count; ++i) {
        std::string name;
        std::string value;
        if (!ReadEscapedField(encoded, pos, '=', name)) {
            return false;
        }
        if (!ReadEscapedField(encoded, pos, ';', value)) {
            return false;
        }
        out.emplace_back(std::move(name), std::move(value));
    }

    return pos == encoded.size();
}

} // namespace bim::ifc::detail
