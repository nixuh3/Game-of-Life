#include "simulation.hpp"
#include <unordered_set>

namespace gol {

void gol::Simluation::Step(ChunkMap& current, ChunkMap& next, const RuleSet& rules) {
    next.clear();

    // Any active chunk + all 8 of its surrounding empty chunks
    std::unordered_set<Vec2i, Vec2i::Hash> chunksToProcess;
    for (const auto& [pos, chunk] : current) {
        if (chunk.IsEmpty()) {
            continue;
        }

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                chunksToProcess.insert(pos + Vec2i{ dx, dy });
            }
        }
    }

    // Evaluate each chunk in parallel across its 64-bit rows
    for (const Vec2i& chunkPos : chunksToProcess) {
        // Pre-fetch pointers to the 3x3 neighborhood of chunks to make border lookups instant
        const Chunk* neighbors[3][3] = { nullptr };
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                auto it = current.find(chunkPos + Vec2i{ dx, dy });
                if (it != current.end()) {
                    neighbors[dy + 1][dx + 1] = &it->second;
                }
            }
        }

        // Safely retrieves a row word even across chunk vertical/horizontal boundaries
        auto GetChunkRow = [&](int dx, int r) -> uint64_t {
            int dy = 0;
            if (r < 0) {
                dy = -1;
                r = 63;
            } else if (r > 63) {
                dy = 1;
                r = 0;
            }

            const Chunk* c = neighbors[dy + 1][dx + 1];
            return c ? c->rows[r] : 0ULL;
        };

        Chunk nextChunk;
        bool hasLiveCells = false;

        for (int r = 0; r < 64; ++r) {
            // Fetch the 3 raw horizontal rows (North, Center, South) and their West/East neighbors
            uint64_t nRow = GetChunkRow(0, r - 1);
            uint64_t nWest = GetChunkRow(-1, r - 1);
            uint64_t nEast = GetChunkRow(1, r - 1);

            uint64_t cRow = GetChunkRow(0, r);
            uint64_t cWest = GetChunkRow(-1, r);
            uint64_t cEast = GetChunkRow(1, r);

            uint64_t sRow = GetChunkRow(0, r + 1);
            uint64_t sWest = GetChunkRow(-1, r + 1);
            uint64_t sEast = GetChunkRow(1, r + 1);

            // Construct the 8 directional neighbor bitboards by shifting across borders
            // (Assuming bit 0 is x=0, shifting left by 1 moves West neighbors into position)
            uint64_t nw = (nRow << 1) | (nWest >> 63);
            uint64_t n = nRow;
            uint64_t ne = (nRow >> 1) | (nEast << 63);

            uint64_t w = (cRow << 1) | (cWest >> 63);
            uint64_t e = (cRow >> 1) | (cEast << 63);

            uint64_t sw = (sRow << 1) | (sWest >> 63);
            uint64_t s = sRow;
            uint64_t se = (sRow >> 1) | (sEast << 63);

            // PARALLEL BITWISE ADDITION:
            // Sum the 8 neighbor boards into a 4-bit binary counter per cell (c3, c2, c1, c0)
            uint64_t w1a, w2a, w1b, w2b, w1c, w2c, w2d;
            uint64_t w2Sum, w4a, w4b;
            uint64_t c0, c1, c2, c3;

            FullAdd(nw, n, ne, w1a, w2a);
            FullAdd(w, e, sw, w1b, w2b);
            FullAdd(w1a, w1b, s, w1c, w2c);
            HalfAdd(w1c, se, c0, w2d); // c0 is the 1s bit of the count

            FullAdd(w2a, w2b, w2c, w2Sum, w4a);
            HalfAdd(w2Sum, w2d, c1, w4b); // c1 is the 2s bit of the count

            HalfAdd(w4a, w4b, c2, c3); // c2 is the 4s bit, c3 is the 8s bit

            // Decode the 4-bit counter into exact match masks for counts 0 through 8
            uint64_t eq[9];
            eq[0] = ~c3 & ~c2 & ~c1 & ~c0;
            eq[1] = ~c3 & ~c2 & ~c1 & c0;
            eq[2] = ~c3 & ~c2 & c1 & ~c0;
            eq[3] = ~c3 & ~c2 & c1 & c0;
            eq[4] = ~c3 & c2 & ~c1 & ~c0;
            eq[5] = ~c3 & c2 & ~c1 & c0;
            eq[6] = ~c3 & c2 & c1 & ~c0;
            eq[7] = ~c3 & c2 & c1 & c0;
            eq[8] = c3 & ~c2 & ~c1 & ~c0;

            uint64_t birth_mask = 0;
            uint64_t survival_mask = 0;
            for (int i = 0; i <= 8; ++i) {
                if (rules.birthMask & (1 << i)) {
                    birth_mask |= eq[i];
                }
                if (rules.survivalMask & (1 << i)) {
                    survival_mask |= eq[i];
                }
            }

            // A dead cell spawns if in birth_mask; a live cell survives if in survival_mask
            uint64_t nextRow = (~cRow & birth_mask) | (cRow & survival_mask);

            nextChunk.rows[r] = nextRow;
            if (nextRow != 0) {
                hasLiveCells = true;
            }
        }

        // Only save the chunk if it contains at least one living cell
        if (hasLiveCells) {
            // If this chunk already had a GPU texture last frame, reuse it!
            auto oldIt = current.find(chunkPos);
            if (oldIt != current.end() && oldIt->second.texture.id != 0) {
                nextChunk.texture = oldIt->second.texture;
                oldIt->second.texture = {}; // Nullify old so it doesn't get unloaded
                nextChunk.isDirty = true;
            }

            next[chunkPos] = std::move(nextChunk);
        }
    }

    std::swap(current, next);
}

}
