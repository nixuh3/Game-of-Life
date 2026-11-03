#pragma once

#include "math.hpp"
#include <raylib.h>
#include <bit>
#include <array>

namespace gol {

struct Chunk {
    Chunk() = default;
    Chunk(const Chunk& other); // When copied, copy the cells, but reset the texture so the copy creates its
                               // own GPU texture lazily

    ~Chunk() { Unload(); }

    Chunk& operator=(const Chunk& other) {
        if (this != &other) {
            rows = other.rows;
            isDirty = true;
            // Keep our own existing texture (if we have one), just mark it dirty to overwrite it
        }
        return *this;
    }

    // When moved, steal the GPU texture ID
    Chunk(Chunk&& other) noexcept : rows(other.rows), texture(other.texture), isDirty(other.isDirty) {
        other.texture = {}; // Zero out 'other' so its destructor doesn't unload our texture!
    }

    Chunk& operator=(Chunk&& other) noexcept {
        if (this != &other) {
            Unload(); // Unload our current texture before stealing the new one
            rows = other.rows;
            texture = other.texture;
            isDirty = other.isDirty;
            other.texture = {};
        }
        return *this;
    }

    void InitTexture();
    void Unload() const;
    void UpdateChunkTexture();

    [[nodiscard]] bool GetCell(const Vec2i& localPos) const; // Check if cell is alive
    void SetCell(const Vec2i& localPos);                     // Set cell to alive
    void ClearCell(const Vec2i& localPos);                   // Clear cell to dead
    void ToggleCell(const Vec2i& localPos);                  // Toggle cell state
    [[nodiscard]] bool IsEmpty() const; // Returns true if the entire chunk contains zero alive cells

    std::array<uint64_t, 64> rows{};

    Texture2D texture{};
    bool isDirty = true; // Set to true whenever cells change
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
