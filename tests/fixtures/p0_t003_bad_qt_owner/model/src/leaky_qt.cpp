// tests/fixtures/p0_t003_bad_qt_owner/model/src/leaky_qt.cpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R9/QT_DESKTOP_ONLY must reject this file. It sits under model/src/**
// (outside desktop/**, mirroring the ownership-partition pattern R7 and
// R10 already use) and uses a Qt token where only src/desktop/** is
// permitted to. This file is never compiled - it exists only to be
// scanned by the architecture checker (see tests/architecture/CMakeLists.txt,
// test arch_p0_t003_qt_fixture_rejected).

#include <QtCore/QString> // FORBIDDEN: only src/desktop/** may use Qt

namespace bim::model {

QString NotAllowedHere() {
    return QString::fromUtf8("model must never depend on Qt");
}

} // namespace bim::model
