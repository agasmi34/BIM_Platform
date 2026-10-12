// DELIBERATELY BROKEN FIXTURE - never built, never compiled, never linked.
// It is scanned only by tools/architecture_checker.cmake (rule R21,
// PERSISTENCE_DOCUMENT_STORE_BOUNDARY, P1-T004), which MUST reject it
// (CTest arch_p1_t004_persistence_document_dependency_fixture_rejected,
// WILL_FAIL TRUE).
//
// A persistence source must stay below the document layer. This simulated
// persistence/src file reaches UP into the document module and into the private
// runtime state that the store must never see or persist.
#include "bim/dependency_graph/graph.hpp"
#include "bim/document/document.hpp"

namespace bim::persistence {

// Wrong: the store reads the live Document and a runtime graph identity.
void SaveLiveDocument(const bim::document::Document& document, NodeId runtime_node) {
    (void)document;
    (void)runtime_node;
}

} // namespace bim::persistence
