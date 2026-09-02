#include "bim/foundation/status.hpp"

namespace bim::foundation {

Status Status::Ok() {
    return Status(true, std::string{});
}

Status Status::Error(std::string message) {
    return Status(false, std::move(message));
}

} // namespace bim::foundation
