#pragma once

#include <array>
#include "Math.h"

namespace GoL {

struct Chunk {
    std::array<uint64_t, 64> rows{};

    // Check if cell is alive
    [[nodiscard]] bool GetCell(const Vec2i& localPos) const {
        return (rows[localPos.y] & (1ULL << localPos.x)) != 0;
    }

    // Set cell to alive
    void SetCell(const Vec2i& localPos) { rows[localPos.y] |= (1ULL << localPos.x); }

    // Clear cell to dead
    void ClearCell(const Vec2i& localPos) { rows[localPos.y] &= ~(1ULL << localPos.x); }

    // Toggle cell state
    void ToggleCell(const Vec2i& localPos) { rows[localPos.y] ^= (1ULL << localPos.x); }

    // Returns true if the entire chunk contains zero alive cells
    [[nodiscard]] bool IsEmpty() const {
        for (uint64_t row : rows) {
            if (row != 0) {
                return false;
            }
        }
        return true;
    }
};
using ChunkMap = std::unordered_map<Vec2i, Chunk, Vec2i::Hash>;

struct LocalCellPos {
    Vec2i chunkPos;
    Vec2i localPos;
};

inline LocalCellPos GlobalToLocal(const Vec2i& globalPos) {
    int chunkX = globalPos.x >> 6;
    int chunkY = globalPos.y >> 6;

    // Bitwise AND with 63 is equivalent to positive modulo [0, 63]
    int localX = globalPos.x & 63;
    int localY = globalPos.y & 63;

    return {
        { chunkX, chunkY },
        { localX, localY }
    };
}

}
