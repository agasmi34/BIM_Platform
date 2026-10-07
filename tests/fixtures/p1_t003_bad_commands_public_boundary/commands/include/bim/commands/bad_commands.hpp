#pragma once

// tests/fixtures/p1_t003_bad_commands_public_boundary/commands/include/bim/commands/bad_commands.hpp
//
// DELIBERATELY INVALID FIXTURE - tools/architecture_checker.cmake rule
// R19/COMMANDS_PUBLIC_BOUNDARY must reject this file. It sits under
// commands/include/** (mirroring the real src/commands/include/bim/commands/
// layout the rule scans) and couples the command module to layers it must never
// touch: it includes dependency-graph, transaction and persistence headers, and
// it exposes the runtime-only node identity, a graph result type and a journal
// transaction in a public signature - exactly the defect class R19 exists to
// catch (a command contract that reaches past bim::document into runtime graph,
// journal or storage internals). Every leak below is real code, not comment text,
// because R19 ignores comments. This file is never compiled - it exists only to
// be scanned by the architecture checker (see tests/architecture/CMakeLists.txt,
// test arch_p1_t003_commands_public_fixture_rejected).

#include "bim/dependency_graph/dependency_graph.hpp" // FORBIDDEN: commands must not include the graph
#include "bim/persistence/probe.hpp"                // FORBIDDEN: commands must not include persistence
#include "bim/transactions/journal.hpp"             // FORBIDDEN: commands must not include transactions

namespace bim::commands {

class BadCommands {
public:
    // FORBIDDEN: the runtime-only node identity and graph result are not command API.
    bim::dependency_graph::GraphResult CreateNode(bim::dependency_graph::NodeId node);

    // FORBIDDEN: a command has no business producing a journal transaction.
    bim::transactions::JournalTransaction Journal();
};

} // namespace bim::commands
