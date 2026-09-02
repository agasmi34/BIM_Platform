// Private compilation anchor for bim_commands (Implementation Brief IC-004).
//
// bim_commands MUST NOT write SQLite directly (Architecture Gate section 8.2
// / Implementation Brief section 2.2). No SQLite include or dependency
// appears anywhere in this module; this is checked mechanically by
// tools/architecture_checker.cmake.

namespace bim::commands::detail {

[[maybe_unused]] void CompilationAnchor() {}

} // namespace bim::commands::detail
