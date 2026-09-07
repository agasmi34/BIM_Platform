#include "bim/geometry_occt/spike_diagnostics.hpp"

#include "solid_impl.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>

#include <exception>
#include <string>
#include <vector>

// P0-only OCCT operation-history diagnostic implementation (Implementation
// Brief section 19; Amendment 01 AA-C06). Never consumed by production BIM
// code.
//
// PROVISIONAL API NOTE (carried into CLAUDE_HANDOVER.md/.json): this file
// iterates and counts NCollection_List<TopoDS_Shape> values returned by
// BRepAlgoAPI_BuilderAlgo::Generated()/Modified() using range-based `for`
// and a manual counter. NCollection_List<TopoDS_Shape> itself is VERIFIED
// as the literal return type of Generated()/Modified() (BRepAlgoAPI_
// BuilderAlgo.hxx was read in full during the OCCT API verification round),
// but NCollection_List.hxx - the header declaring the container's own
// iteration surface (begin()/end()/Extent()) - was NOT included in the
// OCCT-8.0.1-READPACK and so was NOT independently read this round. Using
// range-based `for` here is a deliberate minimum-assumption choice (it only
// requires begin()/end(), not any specific named accessor such as
// Extent()), consistent with "use APIs verified from the supplied OCCT
// 8.0.1 headers ... or settled by authoritative Windows/MSVC compilation."
// If Windows/MSVC compilation shows this assumption wrong, the fix is
// localized to CountShapes() below.

namespace bim::geometry_occt::spike_diagnostics {

namespace {

[[nodiscard]] std::vector<TopoDS_Shape> CollectFacesInOrder(const TopoDS_Shape& shape) {
    std::vector<TopoDS_Shape> faces;
    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        faces.push_back(explorer.Current());
    }
    return faces;
}

template <class ShapeList> [[nodiscard]] int CountShapes(const ShapeList& list) {
    int count = 0;
    for (const auto& unused_shape : list) {
        (void)unused_shape;
        ++count;
    }
    return count;
}

template <class Algo>
void RecordSide(Algo& algo, const TopoDS_Shape& input_shape, HistoryInputSide side,
                std::vector<FaceHistoryRecord>& out) {
    const std::vector<TopoDS_Shape> faces = CollectFacesInOrder(input_shape);
    int ordinal = 1;
    for (const TopoDS_Shape& face : faces) {
        FaceHistoryRecord record;
        record.input_side = side;
        record.input_face_ordinal = ordinal;
        record.generated_count = CountShapes(algo.Generated(face));
        record.modified_count = CountShapes(algo.Modified(face));
        record.deleted = algo.IsDeleted(face);
        out.push_back(record);
        ++ordinal;
    }
}

} // namespace

HistoryRunResult CaptureHistory(HistoryOperation operation,
                                const bim::geometry_api::SolidHandle& first,
                                const bim::geometry_api::SolidHandle& second,
                                const bim::geometry_api::GeometryTolerance& tolerance,
                                int history_repeat) {
    using bim::geometry_api::GeometryError;
    using bim::geometry_api::GeometryErrorCode;

    HistoryRunResult result;
    result.history_repeat = history_repeat;

    if (first == nullptr || second == nullptr) {
        result.error =
            GeometryError{GeometryErrorCode::InvalidInput, "CaptureHistory: null solid handle"};
        return result;
    }

    try {
        const TopoDS_Shape& first_shape = first->shape;
        const TopoDS_Shape& second_shape = second->shape;

        if (operation == HistoryOperation::Cut) {
            BRepAlgoAPI_Cut algo(first_shape, second_shape);
            algo.SetFuzzyValue(tolerance.linear);
            algo.Build();
            if (algo.HasErrors()) {
                result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                             "CaptureHistory: Cut kernel reported errors"};
                return result;
            }
            RecordSide(algo, first_shape, HistoryInputSide::First, result.records);
            RecordSide(algo, second_shape, HistoryInputSide::Second, result.records);
        } else {
            BRepAlgoAPI_Fuse algo(first_shape, second_shape);
            algo.SetFuzzyValue(tolerance.linear);
            algo.Build();
            if (algo.HasErrors()) {
                result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                             "CaptureHistory: Fuse kernel reported errors"};
                return result;
            }
            RecordSide(algo, first_shape, HistoryInputSide::First, result.records);
            RecordSide(algo, second_shape, HistoryInputSide::Second, result.records);
        }
    } catch (const Standard_Failure& failure) {
        const char* what = failure.what();
        result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                     std::string("CaptureHistory: OCCT Standard_Failure: ") +
                                         (what != nullptr ? what : "<no message>")};
    } catch (const std::exception& ex) {
        result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                     std::string("CaptureHistory: std::exception: ") + ex.what()};
    } catch (...) {
        result.error = GeometryError{GeometryErrorCode::KernelOperationFailed,
                                     "CaptureHistory: unknown exception"};
    }

    return result;
}

} // namespace bim::geometry_occt::spike_diagnostics
