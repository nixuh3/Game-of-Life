#pragma once

#include "Chunk.h"
#include "Rules.h"

namespace GoL {

namespace Simluation {

void Step(ChunkMap& current, ChunkMap& next, const RuleSet& rules);

}

} // namespace GoL
