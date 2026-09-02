# ==============================================================================
# BimOptions.cmake - central first-party build options for BIM Platform.
#
# Keep this file limited to option() declarations and simple derived globals.
# Do not put module implementation logic here.
# ==============================================================================

option(BIM_WARNINGS_AS_ERRORS
    "Treat first-party compiler warnings as errors (enabled by CI presets)."
    OFF)

option(BIM_ENABLE_SANITIZERS
    "Enable sanitizer instrumentation for first-party targets where supported."
    OFF)
