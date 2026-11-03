#include "chunk.hpp"

namespace gol {

Chunk::Chunk(const Chunk& other) : rows(other.rows), texture{}, isDirty(true) {}

void Chunk::InitTexture() {
    // Create an uninitialized 64x64 RGBA texture on the GPU
    Image blankImage = GenImageColor(64, 64, BLANK);
    texture = LoadTextureFromImage(blankImage);
    UnloadImage(blankImage);

    // Use point filtering so cells render as crisp squares, not blurry gradients
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
}

void Chunk::Unload() const {
    if (texture.id != 0) {
        UnloadTexture(texture);
    }
}

void Chunk::UpdateChunkTexture() {
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

bool Chunk::GetCell(const Vec2i& localPos) const { return (rows[localPos.y] & (1ULL << localPos.x)) != 0; }

void Chunk::SetCell(const Vec2i& localPos) {
    rows[localPos.y] |= (1ULL << localPos.x);
    isDirty = true;
}

void Chunk::ClearCell(const Vec2i& localPos) {
    rows[localPos.y] &= ~(1ULL << localPos.x);
    isDirty = true;
}

void Chunk::ToggleCell(const Vec2i& localPos) {
    rows[localPos.y] ^= (1ULL << localPos.x);
    isDirty = true;
}

bool Chunk::IsEmpty() const {
    for (uint64_t row : rows) {
        if (row != 0) {
            return false;
        }
    }
    return true;
}

}
