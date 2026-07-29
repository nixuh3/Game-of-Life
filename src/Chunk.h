#pragma once

#include "Math.h"
#include <raylib.h>
#include <bit>
#include <array>

namespace GoL {

struct Chunk {
    std::array<uint64_t, 64> rows{};

    Texture2D texture{};
    bool isDirty = true; // Set to true whenever cells change

    Chunk() = default;

    ~Chunk() { Unload(); }

    // When copied, copy the cells, but reset the texture so the copy creates its own GPU texture lazily
    Chunk(const Chunk& other) : rows(other.rows), texture{}, isDirty(true) {}

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

    void InitTexture() {
        // Create an uninitialized 64x64 RGBA texture on the GPU
        Image blankImage = GenImageColor(64, 64, BLANK);
        texture = LoadTextureFromImage(blankImage);
        UnloadImage(blankImage);

        // Use point filtering so cells render as crisp squares, not blurry gradients
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    }

    void Unload() const {
        if (texture.id != 0) {
            UnloadTexture(texture);
        }
    }

    void UpdateChunkTexture() {
        if (texture.id == 0) {
            InitTexture();
            isDirty = true;
        }

        if (!isDirty) {
            return;
        }

        // A 64x64 buffer of RGBA pixels
        Color pixels[64 * 64];

        // 1. Clear all pixels to transparent (DEAD cells)
        std::fill(std::begin(pixels), std::end(pixels), BLANK);

        // 2. Populate only the ALIVE cells
        for (int r = 0; r < 64; ++r) {
            uint64_t rowBits = rows[r];
            int rowOffset = r * 64;

            while (rowBits != 0) {
                int c = std::countr_zero(rowBits);

                // Set live cell pixel color
                pixels[rowOffset + c] = GREEN;

                rowBits &= (rowBits - 1);
            }
        }

        // 3. Upload the pixel array directly to GPU memory
        UpdateTexture(texture, pixels);
        isDirty = false;
    }

    // Check if cell is alive
    [[nodiscard]] bool GetCell(const Vec2i& localPos) const {
        return (rows[localPos.y] & (1ULL << localPos.x)) != 0;
    }

    // Set cell to alive
    void SetCell(const Vec2i& localPos) {
        rows[localPos.y] |= (1ULL << localPos.x);
        isDirty = true;
    }

    // Clear cell to dead
    void ClearCell(const Vec2i& localPos) {
        rows[localPos.y] &= ~(1ULL << localPos.x);
        isDirty = true;
    }

    // Toggle cell state
    void ToggleCell(const Vec2i& localPos) {
        rows[localPos.y] ^= (1ULL << localPos.x);
        isDirty = true;
    }

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
