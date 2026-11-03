#pragma once

#include "chunk.hpp"
#include "rules.hpp"

namespace gol {

namespace Simluation {

void Step(ChunkMap& current, ChunkMap& next, const RuleSet& rules);

}

}
