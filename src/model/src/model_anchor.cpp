// Private compilation anchor for bim_model (Implementation Brief IC-004).
//
// bim_model has no public API yet in P0-T001: no BIM entities, no project
// schema, no placeholder domain classes are introduced (Implementation Brief
// section 2.2 / Phase I). This translation unit exists only so bim_model is
// a real, non-empty static library that participates correctly in the CMake
// target graph (see tests/unit/unit_model_links_foundation.cpp), without
// inventing a fake public contract.

namespace bim::model::detail {

// Intentionally private, intentionally trivial: gives this translation unit
// a symbol so the archive is non-empty on every toolchain.
[[maybe_unused]] void CompilationAnchor() {}

} // namespace bim::model::detail
