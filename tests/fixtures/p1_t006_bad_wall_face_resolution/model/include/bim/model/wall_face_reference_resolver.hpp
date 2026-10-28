// Negative fixture for architecture rule R27
// (PRODUCTION_WALL_FACE_RESOLUTION_BOUNDARY). This file is never compiled. A
// production wall-face resolver that reuses the P0-T008 spike identity types and
// header must be rejected.

#pragma once

#include "bim/model/persistent_face_reference.hpp"

namespace bim::model {

// Forbidden: P0-T008 spike identity used as production identity.
struct BadWallFaceResolution {
    FeatureOwnerId owner;
    FaceRole role;
};

}  // namespace bim::model
