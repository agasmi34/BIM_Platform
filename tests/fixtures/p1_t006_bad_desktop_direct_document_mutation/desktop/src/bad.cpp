// Negative fixture for architecture rule R25 (DESKTOP_VS1_COMMAND_BOUNDARY).
// This file is never compiled. A desktop translation unit that mutates the
// Document directly, instead of going through bim::commands, must be rejected.

#include "bim/document/document.hpp"

namespace bim::desktop {

void BadDirectMutation(bim::document::Document& document, const bim::model::Level& level) {
    // Forbidden: a direct Document mutation entry point.
    document.AddLevel(level);
}

}  // namespace bim::desktop
