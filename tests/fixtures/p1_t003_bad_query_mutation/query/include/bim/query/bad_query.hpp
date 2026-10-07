#pragma once

// tests/fixtures/p1_t003_bad_query_mutation/query/include/bim/query/bad_query.hpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R20/QUERY_READ_ONLY_BOUNDARY must reject this file. It sits under
// query/include/** (mirroring the real src/query/include/bim/query/ layout the
// rule scans) and breaks the one thing a query must never do: it takes the
// Document by a MUTABLE reference and then mutates it. The real query contract
// takes `const bim::document::Document&`, which R20 permits - what is rejected
// here is the non-const reference and the Document mutation call made through
// it. Every defect below is real code, not comment text, because R20 ignores
// comments. This file is never compiled - it exists only to be scanned by the
// architecture checker (see tests/architecture/CMakeLists.txt, test
// arch_p1_t003_query_mutation_fixture_rejected).

#include "bim/document/document.hpp"

namespace bim::query {

// FORBIDDEN: a query may only observe the document, so a non-const Document
// reference is rejected, and so is the mutation made through it.
inline bool EnsureLevel(bim::document::Document& document, const bim::model::Level& level) {
    return document.AddLevel(level).ok();
}

} // namespace bim::query
