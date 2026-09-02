// Private compilation anchor for bim_transactions (Implementation Brief
// IC-004). No transaction/undo-journal behavior is implemented in P0-T001;
// this target exists only to prove the CMake target graph
// (model + foundation -> transactions -> commands).

namespace bim::transactions::detail {

[[maybe_unused]] void CompilationAnchor() {}

} // namespace bim::transactions::detail
