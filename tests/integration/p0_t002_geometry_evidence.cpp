// P0-T002 standalone geometry evidence executable (Implementation Brief
// section 20; Amendment 01 AA-C07).
//
// This executable links the SAME production geometry API implementation
// exercised by the integration tests (bim::geometry_api /
// bim::geometry_occt, compiled once into bim_geometry_occt - never a
// separate copy) and drives it through the mandatory corpus (Brief sections
// 14-17), the tolerance and coordinate matrices (Brief sections 12-13), the
// repeatability contract (Brief section 18) and the history experiment
// (Brief section 19; Amendment 01 AA-C06). Every case is recorded as a
// hand-rolled JSON evidence record - no new JSON dependency is introduced
// (Brief section 20: "Use standard-library output if necessary"); the
// serializer below uses only <sstream>/<iomanip>/<fstream>.
//
// Scope note (explicit, for CLAUDE_HANDOVER): this executable EXECUTES and
// RECORDS every repeatability/history repetition, and fails on a mismatch
// against a case's own fixed expected classification where one is declared.
// It does not itself additionally assert cross-repetition consistency (e.g.
// "all 20 repeats produced the identical code") - that specific consistency
// assertion is owned by the corresponding CTest
// (integration_geometry_occt_repeatability /
// integration_geometry_occt_history), which is exact-name-registered and
// required by scripts/ci/architecture.ps1 / Verification-RunbookC-v1.0.ps1.
// The JSON emitted here carries every individual repetition's outcome, so
// that consistency is independently checkable from the evidence file itself.
//
// Exit code: 0 when every case with a fixed expected classification matches
// AND every observational case (J06, F04 - Brief sections 16-17: "the exact
// resulting status is not predeclared") avoids KernelOperationFailed and, if
// classified None, reports a valid result; non-zero otherwise, including on
// any std::exception/unknown exception observed at this boundary (which
// would itself indicate an adapter defect per Brief section 29).
//
// Timing (std::chrono::steady_clock) is diagnostic evidence only (Brief
// section 20: "diagnostic only ... not a pass threshold") and never
// participates in the pass/fail decision below.
//
// Usage: p0_t002_geometry_evidence [--json <output-path>]
// With --json, the JSON evidence document is written to <output-path> and a
// short human-readable summary is printed to stdout. Without --json, the
// JSON document itself is printed to stdout.

#include "bim/geometry_api/geometry.hpp"
#include "bim/geometry_occt/spike_diagnostics.hpp"
#include "geometry_occt_test_constants.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace bim::geometry_api;
using namespace bim::geometry_occt::test_constants;
using bim::geometry_occt::spike_diagnostics::CaptureHistory;
using bim::geometry_occt::spike_diagnostics::FaceHistoryRecord;
using bim::geometry_occt::spike_diagnostics::HistoryInputSide;
using bim::geometry_occt::spike_diagnostics::HistoryOperation;
using bim::geometry_occt::spike_diagnostics::HistoryRunResult;

namespace {

constexpr int kRepeatabilityCount = 20; // Brief section 18.
constexpr int kHistoryRepeatCount = 10; // Brief section 19.

// --- minimal hand-rolled JSON serialization (Brief section 20: no new JSON
// dependency) --------------------------------------------------------------

[[nodiscard]] std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (c < 0x20) {
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c);
                    out += oss.str();
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

[[nodiscard]] std::string JsonString(const std::string& key, const std::string& value) {
    std::ostringstream oss;
    oss << '"' << key << "\":\"" << JsonEscape(value) << '"';
    return oss.str();
}

[[nodiscard]] std::string JsonNumber(const std::string& key, double value) {
    std::ostringstream oss;
    oss << '"' << key << "\":" << std::setprecision(17) << value;
    return oss.str();
}

[[nodiscard]] std::string JsonInt(const std::string& key, long long value) {
    std::ostringstream oss;
    oss << '"' << key << "\":" << value;
    return oss.str();
}

[[nodiscard]] std::string JsonBool(const std::string& key, bool value) {
    return "\"" + key + "\":" + (value ? "true" : "false");
}

[[nodiscard]] std::string JsonPoint3Field(const std::string& key, const Point3& p) {
    std::ostringstream oss;
    oss << '"' << key << "\":{\"x\":" << std::setprecision(17) << p.x
        << ",\"y\":" << std::setprecision(17) << p.y << ",\"z\":" << std::setprecision(17) << p.z
        << "}";
    return oss.str();
}

[[nodiscard]] std::string JoinFields(const std::vector<std::string>& fields) {
    std::string out = "{";
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i != 0) {
            out += ",";
        }
        out += fields[i];
    }
    out += "}";
    return out;
}

[[nodiscard]] std::string ToString(GeometryErrorCode code) {
    switch (code) {
        case GeometryErrorCode::None:
            return "None";
        case GeometryErrorCode::InvalidInput:
            return "InvalidInput";
        case GeometryErrorCode::DegenerateGeometry:
            return "DegenerateGeometry";
        case GeometryErrorCode::NoIntersection:
            return "NoIntersection";
        case GeometryErrorCode::KernelOperationFailed:
            return "KernelOperationFailed";
        case GeometryErrorCode::InvalidResult:
            return "InvalidResult";
        case GeometryErrorCode::UnsupportedOperation:
            return "UnsupportedOperation";
    }
    return "Unknown";
}

// --- evidence record shapes -------------------------------------------------
// Field set is the Brief section 20 / Amendment AA-C07 minimum (task,
// case_id, category, operation, origin_offset, linear_tolerance,
// angular_tolerance, expected_classification, actual_classification, valid,
// volume, solid_count, face_count, edge_count, elapsed_microseconds,
// repeat_iteration) plus one extra diagnostic field (`passed`) this
// executable adds for its own exit-code bookkeeping.

struct CaseRecord {
    std::string case_id;
    std::string category;
    std::string operation;
    Point3 origin_offset;
    double linear_tolerance = 0.0;
    double angular_tolerance = 0.0;
    std::string expected_classification;
    std::string actual_classification;
    bool valid = false;
    double volume = 0.0;
    int solid_count = 0;
    int face_count = 0;
    int edge_count = 0;
    long long elapsed_microseconds = 0;
    int repeat_iteration = 0;
    bool passed = false;

    [[nodiscard]] std::vector<std::string> Fields() const {
        return {
            JsonString("task", "P0-T002"),
            JsonString("case_id", case_id),
            JsonString("category", category),
            JsonString("operation", operation),
            JsonPoint3Field("origin_offset", origin_offset),
            JsonNumber("linear_tolerance", linear_tolerance),
            JsonNumber("angular_tolerance", angular_tolerance),
            JsonString("expected_classification", expected_classification),
            JsonString("actual_classification", actual_classification),
            JsonBool("valid", valid),
            JsonNumber("volume", volume),
            JsonInt("solid_count", solid_count),
            JsonInt("face_count", face_count),
            JsonInt("edge_count", edge_count),
            JsonInt("elapsed_microseconds", elapsed_microseconds),
            JsonInt("repeat_iteration", repeat_iteration),
            JsonBool("passed", passed),
        };
    }

    [[nodiscard]] std::string ToJson() const { return JoinFields(Fields()); }
};

struct HistoryEvidenceRecord {
    CaseRecord base;
    std::string input_side;
    int input_face_ordinal = 0;
    int generated_count = 0;
    int modified_count = 0;
    bool deleted = false;
    int history_repeat = 0;

    [[nodiscard]] std::string ToJson() const {
        std::vector<std::string> fields = base.Fields();
        fields.push_back(JsonString("input_side", input_side));
        fields.push_back(JsonInt("input_face_ordinal", input_face_ordinal));
        fields.push_back(JsonInt("generated_count", generated_count));
        fields.push_back(JsonInt("modified_count", modified_count));
        fields.push_back(JsonBool("deleted", deleted));
        fields.push_back(JsonInt("history_repeat", history_repeat));
        return JoinFields(fields);
    }
};

// --- case runners ------------------------------------------------------------

[[nodiscard]] CaseRecord RunExtrusionCase(const std::string& case_id, const std::string& category,
                                          const LinearExtrusionSpec& spec,
                                          const GeometryTolerance& tolerance, const Point3& offset,
                                          GeometryErrorCode expected_code,
                                          double expected_volume_or_negative, int repeat_iteration,
                                          double volume_relative_epsilon = 1.0e-4) {
    CaseRecord rec;
    rec.case_id = case_id;
    rec.category = category;
    rec.operation = "MakeLinearExtrusion";
    rec.origin_offset = offset;
    rec.linear_tolerance = tolerance.linear;
    rec.angular_tolerance = tolerance.angular_radians;
    rec.expected_classification = ToString(expected_code);
    rec.repeat_iteration = repeat_iteration;

    const auto start = std::chrono::steady_clock::now();
    const SolidResult result = MakeLinearExtrusion(spec, tolerance);
    MetricsResult metrics;
    if (result.ok()) {
        metrics = Inspect(result.solid);
    }
    const auto end = std::chrono::steady_clock::now();
    rec.elapsed_microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    rec.actual_classification = ToString(result.error.code);
    rec.valid = result.ok() && metrics.ok() && metrics.metrics.valid;
    rec.volume = (result.ok() && metrics.ok()) ? metrics.metrics.volume : 0.0;
    rec.solid_count = (result.ok() && metrics.ok()) ? metrics.metrics.solid_count : 0;
    rec.face_count = (result.ok() && metrics.ok()) ? metrics.metrics.face_count : 0;
    rec.edge_count = (result.ok() && metrics.ok()) ? metrics.metrics.edge_count : 0;

    rec.passed = (result.error.code == expected_code);
    if (rec.passed && result.ok() && expected_volume_or_negative >= 0.0) {
        const double band = std::max(1.0e-6, expected_volume_or_negative * volume_relative_epsilon);
        if (std::abs(rec.volume - expected_volume_or_negative) > band) {
            rec.passed = false;
        }
    }
    return rec;
}

[[nodiscard]] CaseRecord
RunBooleanCase(const std::string& case_id, const std::string& category,
               const std::string& operation_name, const LinearExtrusionSpec& a_spec,
               const LinearExtrusionSpec& b_spec, const GeometryTolerance& tolerance,
               const Point3& offset, GeometryErrorCode expected_code,
               double expected_volume_or_negative, bool observational, int repeat_iteration,
               double volume_relative_epsilon = 1.0e-4) {
    CaseRecord rec;
    rec.case_id = case_id;
    rec.category = category;
    rec.operation = operation_name;
    rec.origin_offset = offset;
    rec.linear_tolerance = tolerance.linear;
    rec.angular_tolerance = tolerance.angular_radians;
    rec.expected_classification =
        observational ? "NotPredeclared(observational)" : ToString(expected_code);
    rec.repeat_iteration = repeat_iteration;

    const auto start = std::chrono::steady_clock::now();
    const SolidResult a_result = MakeLinearExtrusion(a_spec, tolerance);
    const SolidResult b_result = MakeLinearExtrusion(b_spec, tolerance);

    SolidResult op_result;
    op_result.error =
        GeometryError{GeometryErrorCode::InvalidResult, "evidence: input construction failed"};
    if (a_result.ok() && b_result.ok()) {
        op_result = (operation_name == "Cut") ? Cut(a_result.solid, b_result.solid, tolerance)
                                              : Fuse(a_result.solid, b_result.solid, tolerance);
    }

    MetricsResult metrics;
    if (op_result.ok()) {
        metrics = Inspect(op_result.solid);
    }
    const auto end = std::chrono::steady_clock::now();
    rec.elapsed_microseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    rec.actual_classification = ToString(op_result.error.code);
    rec.valid = op_result.ok() && metrics.ok() && metrics.metrics.valid;
    rec.volume = (op_result.ok() && metrics.ok()) ? metrics.metrics.volume : 0.0;
    rec.solid_count = (op_result.ok() && metrics.ok()) ? metrics.metrics.solid_count : 0;
    rec.face_count = (op_result.ok() && metrics.ok()) ? metrics.metrics.face_count : 0;
    rec.edge_count = (op_result.ok() && metrics.ok()) ? metrics.metrics.edge_count : 0;

    if (!a_result.ok() || !b_result.ok()) {
        rec.passed = false;
    } else if (observational) {
        // Brief sections 16/17 (J06/F04): the exact resulting status is not
        // predeclared. The only fixed requirements are: never
        // KernelOperationFailed, and a valid result whenever classified
        // None.
        rec.passed = (op_result.error.code != GeometryErrorCode::KernelOperationFailed) &&
                     (op_result.error.code != GeometryErrorCode::None || rec.valid);
    } else {
        rec.passed = (op_result.error.code == expected_code);
        if (rec.passed && op_result.ok() && expected_volume_or_negative >= 0.0) {
            const double band =
                std::max(1.0e-6, expected_volume_or_negative * volume_relative_epsilon);
            if (std::abs(rec.volume - expected_volume_or_negative) > band) {
                rec.passed = false;
            }
        }
    }
    return rec;
}

void RunHistoryCase(const std::string& case_id, HistoryOperation operation,
                    const LinearExtrusionSpec& first_spec, const LinearExtrusionSpec& second_spec,
                    std::vector<CaseRecord>& case_records,
                    std::vector<HistoryEvidenceRecord>& history_records, bool& overall_passed) {
    for (int repeat = 0; repeat < kHistoryRepeatCount; ++repeat) {
        CaseRecord op_rec;
        op_rec.case_id = case_id;
        op_rec.category = "history";
        op_rec.operation = (operation == HistoryOperation::Cut) ? "Cut" : "Fuse";
        op_rec.origin_offset = kCoordinateOffsetC0;
        op_rec.linear_tolerance = kReferenceTolerance.linear;
        op_rec.angular_tolerance = kReferenceTolerance.angular_radians;
        op_rec.expected_classification = "None";
        op_rec.repeat_iteration = repeat;

        const auto start = std::chrono::steady_clock::now();
        // Freshly constructed inputs on every repetition (Brief section 19).
        const SolidResult first = MakeLinearExtrusion(first_spec, kReferenceTolerance);
        const SolidResult second = MakeLinearExtrusion(second_spec, kReferenceTolerance);

        HistoryRunResult history;
        history.error =
            GeometryError{GeometryErrorCode::InvalidResult, "evidence: input construction failed"};
        if (first.ok() && second.ok()) {
            history =
                CaptureHistory(operation, first.solid, second.solid, kReferenceTolerance, repeat);
        }
        const auto end = std::chrono::steady_clock::now();

        op_rec.elapsed_microseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        op_rec.actual_classification = ToString(history.error.code);
        op_rec.valid = history.error.ok() && !history.records.empty();
        op_rec.volume = 0.0; // Not applicable to a history capture.
        op_rec.solid_count = 0;
        op_rec.face_count = static_cast<int>(history.records.size());
        op_rec.edge_count = 0;
        op_rec.passed = first.ok() && second.ok() && history.error.ok() && !history.records.empty();

        if (!op_rec.passed) {
            overall_passed = false;
        }
        case_records.push_back(op_rec);

        for (const FaceHistoryRecord& record : history.records) {
            HistoryEvidenceRecord entry;
            entry.base = op_rec;
            // input_face_ordinal is a transient observation label only
            // (Amendment 01 AA-C06), never persistent topology identity.
            entry.input_side = (record.input_side == HistoryInputSide::First) ? "First" : "Second";
            entry.input_face_ordinal = record.input_face_ordinal;
            entry.generated_count = record.generated_count;
            entry.modified_count = record.modified_count;
            entry.deleted = record.deleted;
            entry.history_repeat = repeat;
            history_records.push_back(entry);
        }
    }
}

} // namespace

// Round 8 (P0-T002 static analysis correction, SA-R8-04): clang-tidy's
// bugprone-exception-escape flags int main(int, char**) because nothing in
// this translation unit previously guaranteed that every exception is
// caught before it would unwind out of main(). The fix is this small
// extraction, not a suppression: the body below is byte-for-byte the
// original main() body, unchanged, including its own existing inner
// try/catch around the section 14-19 corpus and history experiment (Brief
// section 29) - that inner catch's classification behavior, the JSON
// schema, case corpus, overall_passed logic, --json handling and exit-code
// semantics are all unchanged here. main() itself (below, after this
// function) does nothing but forward to RunEvidenceMain() inside its own
// try/catch, so no exception - std::exception-derived or otherwise - can
// ever escape main().
static int RunEvidenceMain(int argc, char** argv) {
    std::string json_path;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--json" && i + 1 < argc) {
            json_path = argv[++i];
        }
    }

    std::vector<CaseRecord> case_records;
    std::vector<HistoryEvidenceRecord> history_records;
    bool overall_passed = true;

    auto Track = [&](CaseRecord rec) {
        if (!rec.passed) {
            overall_passed = false;
        }
        case_records.push_back(std::move(rec));
    };

    try {
        // --- section 14: mandatory primitive corpus -----------------------
        Track(RunExtrusionCase("P01", "primitive", MakeP01(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::None, kP01ExpectedVolume,
                               0));
        Track(RunExtrusionCase("P02", "primitive", MakeP02(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::None, kP02ExpectedVolume,
                               0));
        Track(RunExtrusionCase("P03", "primitive", MakeP03(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::None, kP03ExpectedVolume,
                               0));
        Track(RunExtrusionCase("P04", "primitive", MakeP04(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::None, kP04ExpectedVolume,
                               0));

        // --- section 15: mandatory opening corpus (host = P01) -------------
        Track(RunBooleanCase("O01", "opening_cut", "Cut", MakeP01(), MakeO01(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::None, kO01ExpectedVolume,
                             false, 0));
        Track(RunBooleanCase("O02", "opening_cut", "Cut", MakeP01(), MakeO02(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::NoIntersection, -1.0, false,
                             0));
        Track(RunBooleanCase("O03", "opening_cut", "Cut", MakeP01(), MakeO03(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::None, kO03ExpectedVolume,
                             false, 0));
        Track(RunBooleanCase("O04", "opening_cut", "Cut", MakeP01(), MakeO04(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::NoIntersection, -1.0, false,
                             0));

        // --- section 16: mandatory join corpus ------------------------------
        Track(RunBooleanCase("J01", "join", "Fuse", MakeJ01A(), MakeJ01B(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::None, kJ01ExpectedVolume,
                             false, 0));
        Track(RunBooleanCase("J02", "join", "Fuse", MakeJ02A(), MakeJ02B(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::None, kJ02ExpectedVolume,
                             false, 0));
        Track(RunBooleanCase("J03", "join", "Fuse", MakeJ03A(), MakeJ03B(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::None, kJ03ExpectedVolume,
                             false, 0));
        Track(RunBooleanCase("J04", "join", "Fuse", MakeJ04A(), MakeJ04B(), kReferenceTolerance,
                             kCoordinateOffsetC0, GeometryErrorCode::None, kJ04ExpectedVolume,
                             false, 0));
        for (double linear : kLinearToleranceCandidates) {
            const GeometryTolerance tol{linear, kReferenceTolerance.angular_radians};
            Track(RunBooleanCase("J05", "join", "Fuse", MakeJ05A(), MakeJ05B(), tol,
                                 kCoordinateOffsetC0, GeometryErrorCode::NoIntersection, -1.0,
                                 false, 0));
        }
        {
            int iter = 0;
            for (double linear : kLinearToleranceCandidates) {
                const GeometryTolerance tol{linear, kReferenceTolerance.angular_radians};
                for (int run = 0; run < 3; ++run) {
                    Track(RunBooleanCase("J06", "join", "Fuse", MakeJ06A(), MakeJ06B(), tol,
                                         kCoordinateOffsetC0, GeometryErrorCode::None, -1.0, true,
                                         iter));
                    ++iter;
                }
            }
        }

        // --- section 17: mandatory failure corpus ---------------------------
        Track(RunExtrusionCase("F01", "failure", MakeF01ZeroDistance(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::InvalidInput, -1.0, 0));
        Track(RunExtrusionCase("F02", "failure", MakeF02ZeroDirection(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::DegenerateGeometry, -1.0,
                               0));
        Track(RunExtrusionCase("F03", "failure", MakeF03NegativeDimension(), kReferenceTolerance,
                               kCoordinateOffsetC0, GeometryErrorCode::InvalidInput, -1.0, 0));
        {
            int iter = 0;
            const std::array<Point3, 2> offsets{kCoordinateOffsetC0, kCoordinateOffsetC2};
            for (const Point3& offset : offsets) {
                for (double linear : kLinearToleranceCandidates) {
                    const GeometryTolerance tol{linear, kReferenceTolerance.angular_radians};
                    for (int run = 0; run < 2; ++run) {
                        Track(RunBooleanCase("F04", "failure", "Fuse", MakeF04A(offset),
                                             MakeF04B(offset), tol, offset, GeometryErrorCode::None,
                                             -1.0, true, iter));
                        ++iter;
                    }
                }
            }
        }

        // --- section 12: tolerance matrix ------------------------------------
        for (std::size_t i = 0; i < kLinearToleranceCandidates.size(); ++i) {
            const GeometryTolerance tol{kLinearToleranceCandidates[i], 1.0e-8};
            Track(RunExtrusionCase("TOL_LINEAR_P01", "tolerance_matrix", MakeP01(), tol,
                                   kCoordinateOffsetC0, GeometryErrorCode::None, kP01ExpectedVolume,
                                   static_cast<int>(i), 1.0e-3));
            Track(RunBooleanCase("TOL_LINEAR_O01_CUT", "tolerance_matrix", "Cut", MakeP01(),
                                 MakeO01(), tol, kCoordinateOffsetC0, GeometryErrorCode::None,
                                 kO01ExpectedVolume, false, static_cast<int>(i), 1.0e-3));
        }
        for (std::size_t i = 0; i < kAngularToleranceCandidates.size(); ++i) {
            const GeometryTolerance tol{1.0e-6, kAngularToleranceCandidates[i]};
            Track(RunExtrusionCase("TOL_ANGULAR_ORTHOGONAL", "tolerance_matrix", MakeP01(), tol,
                                   kCoordinateOffsetC0, GeometryErrorCode::None, -1.0,
                                   static_cast<int>(i)));
            LinearExtrusionSpec skewed = MakeP01();
            skewed.profile.v_axis = Vector3{std::sin(0.01), 0.0, std::cos(0.01)};
            Track(RunExtrusionCase("TOL_ANGULAR_SKEWED", "tolerance_matrix", skewed, tol,
                                   kCoordinateOffsetC0, GeometryErrorCode::DegenerateGeometry, -1.0,
                                   static_cast<int>(i)));
        }

        // --- section 13: coordinate matrix ------------------------------------
        for (std::size_t i = 0; i < kCoordinateOffsets.size(); ++i) {
            const Point3& offset = kCoordinateOffsets[i];
            Track(RunExtrusionCase("COORD_P01", "coordinate_matrix", MakeP01(offset),
                                   kReferenceTolerance, offset, GeometryErrorCode::None,
                                   kP01ExpectedVolume, static_cast<int>(i), 1.0e-2));
            Track(RunBooleanCase("COORD_O01_CUT", "coordinate_matrix", "Cut", MakeP01(offset),
                                 MakeO01(offset), kReferenceTolerance, offset,
                                 GeometryErrorCode::None, kO01ExpectedVolume, false,
                                 static_cast<int>(i), 1.0e-2));
            Track(RunBooleanCase("COORD_J01_FUSE", "coordinate_matrix", "Fuse", MakeJ01A(offset),
                                 MakeJ01B(offset), kReferenceTolerance, offset,
                                 GeometryErrorCode::None, kJ01ExpectedVolume, false,
                                 static_cast<int>(i), 1.0e-2));
        }

        // --- section 18: repeatability contract -------------------------------
        for (int i = 0; i < kRepeatabilityCount; ++i) {
            Track(RunExtrusionCase("REPEAT_P01", "repeatability", MakeP01(), kReferenceTolerance,
                                   kCoordinateOffsetC0, GeometryErrorCode::None, kP01ExpectedVolume,
                                   i));
            Track(RunBooleanCase("REPEAT_O01_CUT", "repeatability", "Cut", MakeP01(), MakeO01(),
                                 kReferenceTolerance, kCoordinateOffsetC0, GeometryErrorCode::None,
                                 kO01ExpectedVolume, false, i));
            Track(RunBooleanCase("REPEAT_O02_CUT", "repeatability", "Cut", MakeP01(), MakeO02(),
                                 kReferenceTolerance, kCoordinateOffsetC0,
                                 GeometryErrorCode::NoIntersection, -1.0, false, i));
            Track(RunBooleanCase("REPEAT_J01_FUSE", "repeatability", "Fuse", MakeJ01A(), MakeJ01B(),
                                 kReferenceTolerance, kCoordinateOffsetC0, GeometryErrorCode::None,
                                 kJ01ExpectedVolume, false, i));
            Track(RunBooleanCase("REPEAT_J04_FUSE", "repeatability", "Fuse", MakeJ04A(), MakeJ04B(),
                                 kReferenceTolerance, kCoordinateOffsetC0, GeometryErrorCode::None,
                                 kJ04ExpectedVolume, false, i));
            Track(RunBooleanCase("REPEAT_J05_FUSE", "repeatability", "Fuse", MakeJ05A(), MakeJ05B(),
                                 kReferenceTolerance, kCoordinateOffsetC0,
                                 GeometryErrorCode::NoIntersection, -1.0, false, i));
            Track(RunBooleanCase("REPEAT_J06_FUSE", "repeatability", "Fuse", MakeJ06A(), MakeJ06B(),
                                 kReferenceTolerance, kCoordinateOffsetC0, GeometryErrorCode::None,
                                 -1.0, true, i));
            Track(RunExtrusionCase("REPEAT_F02", "repeatability", MakeF02ZeroDirection(),
                                   kReferenceTolerance, kCoordinateOffsetC0,
                                   GeometryErrorCode::DegenerateGeometry, -1.0, i));
            Track(RunBooleanCase("REPEAT_F04_FUSE", "repeatability", "Fuse", MakeF04A(), MakeF04B(),
                                 kReferenceTolerance, kCoordinateOffsetC0, GeometryErrorCode::None,
                                 -1.0, true, i));
        }

        // --- section 19: history experiment (Amendment 01 AA-C06) -------------
        RunHistoryCase("O01_CUT_HISTORY", HistoryOperation::Cut, MakeP01(), MakeO01(), case_records,
                       history_records, overall_passed);
        RunHistoryCase("J01_FUSE_HISTORY", HistoryOperation::Fuse, MakeJ01A(), MakeJ01B(),
                       case_records, history_records, overall_passed);
        RunHistoryCase("J02_FUSE_HISTORY", HistoryOperation::Fuse, MakeJ02A(), MakeJ02B(),
                       case_records, history_records, overall_passed);
    } catch (const std::exception& ex) {
        // No third-party exception may cross the geometry_api boundary
        // (Brief section 29). Reaching this handler is itself evidence of
        // an adapter defect, not merely a failed case, so it is treated as
        // an unconditional failure of this executable.
        std::cerr << "p0_t002_geometry_evidence: unhandled std::exception escaped the geometry API "
                  << "boundary: " << ex.what() << "\n";
        overall_passed = false;
    } catch (...) {
        std::cerr
            << "p0_t002_geometry_evidence: unhandled unknown exception escaped the geometry API "
            << "boundary.\n";
        overall_passed = false;
    }

    long long elapsed_total_microseconds = 0;
    std::size_t failed = 0;
    for (const CaseRecord& rec : case_records) {
        elapsed_total_microseconds += rec.elapsed_microseconds;
        if (!rec.passed) {
            ++failed;
        }
    }
    const bool final_passed = overall_passed && failed == 0;

    std::ostringstream json;
    json << "{";
    json << JsonString("task", "P0-T002") << ",";
    json << JsonString("executable", "p0_t002_geometry_evidence") << ",";
    json << JsonBool("overall_passed", final_passed) << ",";
    json << JsonInt("case_count", static_cast<long long>(case_records.size())) << ",";
    json << JsonInt("failed_case_count", static_cast<long long>(failed)) << ",";
    json << JsonInt("history_record_count", static_cast<long long>(history_records.size())) << ",";
    json << JsonInt("elapsed_total_microseconds", elapsed_total_microseconds) << ",";
    json << "\"cases\":[";
    for (std::size_t i = 0; i < case_records.size(); ++i) {
        if (i != 0) {
            json << ",";
        }
        json << case_records[i].ToJson();
    }
    json << "],";
    json << "\"history_records\":[";
    for (std::size_t i = 0; i < history_records.size(); ++i) {
        if (i != 0) {
            json << ",";
        }
        json << history_records[i].ToJson();
    }
    json << "]";
    json << "}";
    const std::string json_text = json.str();

    if (!json_path.empty()) {
        // Write into the build tree, never the source tree (Brief section
        // 21). The parent directory is created here (rather than assumed
        // pre-created) so this executable behaves correctly whether it is
        // invoked directly by CTest (tests/integration/CMakeLists.txt) or
        // via scripts/ci/geometry-spike.ps1, which also pre-creates it.
        std::error_code mkdir_ec;
        const std::filesystem::path output_path(json_path);
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path(), mkdir_ec);
            if (mkdir_ec) {
                std::cerr << "p0_t002_geometry_evidence: failed to create output directory '"
                          << output_path.parent_path().string() << "': " << mkdir_ec.message()
                          << "\n";
                return 1;
            }
        }

        std::ofstream out(json_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "p0_t002_geometry_evidence: failed to open --json output path: "
                      << json_path << "\n";
            return 1;
        }
        out << json_text;
        out.close();
        if (!out) {
            std::cerr << "p0_t002_geometry_evidence: failed while writing --json output path: "
                      << json_path << "\n";
            return 1;
        }
        std::cout << "p0_t002_geometry_evidence: wrote evidence to " << json_path << "\n";
    } else {
        std::cout << json_text << "\n";
    }

    std::cout << "p0_t002_geometry_evidence: " << case_records.size() << " case(s), "
              << history_records.size() << " history record(s), " << failed
              << " failed case(s), overall_passed=" << (final_passed ? "true" : "false") << "\n";

    return final_passed ? 0 : 1;
}

// Round 8B (P0-T002 static analysis correction, SA-R8-04 follow-up):
// authoritative clang-tidy 19.1.5 still reported bugprone-exception-escape
// on this function after round 8's fix - not because RunEvidenceMain()'s
// call is unguarded (it is, by the try below), but because the round-8
// catch handlers' own bodies (std::cerr <<, ex.what(), implicit
// std::string/formatting work) are themselves potentially-throwing C++
// operations running directly in main()'s body with nothing further
// catching them. This boundary is now reduced to the minimum that
// guarantees no exception - std::exception-derived or otherwise - can
// ever escape main(): main() is declared noexcept, and both handlers
// perform nothing but a direct integer return - no stream output, no
// string construction, no ex.what() call, no other helper. Architecture
// Authority disposition (round 8B): a non-zero failure return is required
// here; a stderr diagnostic from this outer boundary is explicitly not
// required for round 8B (RunEvidenceMain()'s own inner catch, unchanged,
// still reports std::cerr diagnostics for the classification failures it
// actually handles). No evidence behavior, JSON schema, case corpus,
// overall_passed logic, --json handling or normal-path exit semantics
// changes.
int main(int argc, char** argv) noexcept {
    try {
        return RunEvidenceMain(argc, argv);
    } catch (const std::exception&) {
        return 1;
    } catch (...) {
        return 1;
    }
}
