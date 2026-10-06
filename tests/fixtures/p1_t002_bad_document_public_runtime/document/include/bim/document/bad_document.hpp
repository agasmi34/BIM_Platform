#pragma once

// tests/fixtures/p1_t002_bad_document_public_runtime/document/include/bim/document/bad_document.hpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R18/DOCUMENT_PUBLIC_RUNTIME_NEUTRAL must reject this file. It sits under
// document/include/** (mirroring the real
// src/document/include/bim/document/ layout the rule scans) and leaks the
// dependency-graph runtime layer into what R18 treats as bim::document's
// public surface: it includes a dependency-graph header and exposes the
// runtime-only node identity and graph result type in a public signature -
// exactly the defect class R18 exists to catch (the document's private
// runtime identity association creeping into its neutral public contract).
// Every leak below is real code, not comment text, because R18 ignores
// comments. This file is never compiled - it exists only to be scanned by
// the architecture checker (see tests/architecture/CMakeLists.txt, test
// arch_p1_t002_document_public_fixture_rejected).

#include "bim/dependency_graph/dependency_graph.hpp" // FORBIDDEN: public headers must not include the graph

namespace bim::document {

class BadDocument {
public:
    // FORBIDDEN: the runtime-only node identity and graph result are not public API.
    bim::dependency_graph::GraphResult RegisterNode(bim::dependency_graph::NodeId node);

private:
    bim::dependency_graph::DependencyGraph graph_;
};

} // namespace bim::document
