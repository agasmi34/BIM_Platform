#include "bim/dwg/probe.hpp"

#include "detail/oda_adapter.hpp"

#include <system_error>

namespace bim::dwg {

namespace {

// Packet section 11's three up-front rejections. Checked with plain
// std::filesystem calls only - never opens the ODA runtime, never touches
// detail:: - so a rejected call here makes zero ODA calls and performs no
// write of any kind, exactly as the Packet requires.
[[nodiscard]] bim::foundation::Status RejectUnsafeRoundTripInputs(
    const std::filesystem::path& source_path, const std::filesystem::path& export_path) {
    std::error_code exists_ec;
    const bool source_exists = std::filesystem::exists(source_path, exists_ec);
    if (exists_ec || !source_exists) {
        return bim::foundation::Status::Error("bim::dwg: source_path does not exist: " +
                                               source_path.string());
    }

    std::error_code is_file_ec;
    const bool source_is_regular_file = std::filesystem::is_regular_file(source_path, is_file_ec);
    if (is_file_ec || !source_is_regular_file) {
        return bim::foundation::Status::Error("bim::dwg: source_path is not a regular file: " +
                                               source_path.string());
    }

    std::error_code equiv_ec;
    // std::filesystem::equivalent() requires both paths to exist, which
    // source_path is now known to (export_path may not - that is fine, a
    // failed equivalent() check just means "not equivalent" for our
    // purposes since export_path not existing is handled by the next
    // check below).
    if (std::filesystem::exists(export_path) &&
        std::filesystem::equivalent(source_path, export_path, equiv_ec) && !equiv_ec) {
        return bim::foundation::Status::Error(
            "bim::dwg: export_path must not be the same file as source_path: " +
            export_path.string());
    }
    // Lexical fallback: catches the common "identical path string" case
    // even when equivalent() above could not run (e.g. export_path does
    // not yet exist, so filesystem-level equivalence cannot be checked).
    if (source_path.lexically_normal() == export_path.lexically_normal()) {
        return bim::foundation::Status::Error(
            "bim::dwg: export_path must not equal source_path: " + export_path.string());
    }

    std::error_code export_exists_ec;
    if (std::filesystem::exists(export_path, export_exists_ec) && !export_exists_ec) {
        return bim::foundation::Status::Error(
            "bim::dwg: export_path already exists and would be overwritten: " +
            export_path.string());
    }

    return bim::foundation::Status::Ok();
}

} // namespace

bim::foundation::Status RunDwgRoundTripProbe(const std::filesystem::path& source_path,
                                              const std::filesystem::path& export_path,
                                              DwgRoundTripEvidence& out_evidence) {
    out_evidence = DwgRoundTripEvidence{};

    const bim::foundation::Status precheck_status =
        RejectUnsafeRoundTripInputs(source_path, export_path);
    if (!precheck_status.ok()) {
        return precheck_status;
    }

    return detail::RunControlledRoundTrip(source_path, export_path, out_evidence);
}

bim::foundation::Status OpenAndValidateDwgFile(const std::filesystem::path& path,
                                                std::string& out_version) {
    out_version.clear();

    std::error_code exists_ec;
    const bool exists = std::filesystem::exists(path, exists_ec);
    if (exists_ec || !exists) {
        return bim::foundation::Status::Error("bim::dwg: path does not exist: " + path.string());
    }

    std::error_code is_file_ec;
    const bool is_regular_file = std::filesystem::is_regular_file(path, is_file_ec);
    if (is_file_ec || !is_regular_file) {
        return bim::foundation::Status::Error("bim::dwg: path is not a regular file: " +
                                               path.string());
    }

    const bim::foundation::Status read_status = detail::ReadDwgHeaderVersion(path, out_version);
    if (!read_status.ok()) {
        return read_status;
    }

    // Packet section 9: the controlled accepted source version is
    // exactly "AC1018". This probe never claims support for any other
    // DWG version, even if the header parsed without error.
    if (out_version != "AC1018") {
        return bim::foundation::Status::Error(
            "bim::dwg: unsupported DWG version (expected AC1018, found '" + out_version +
            "'): " + path.string());
    }

    return bim::foundation::Status::Ok();
}

} // namespace bim::dwg
