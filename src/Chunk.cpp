#include "Chunk.h"

namespace GoL {

Chunk::Chunk(const Chunk& other) : rows(other.rows), texture{}, isDirty(true) {}

Chunk& Chunk::operator=(const Chunk& other) {
    if (this != &other) {
        rows    = other.rows;
        isDirty = true;
        // Keep our own existing texture (if we have one), just mark it dirty to overwrite it
    }
    return *this;
}

void Chunk::UpdateChunkTexture() {
    if (texture.getSize() == sf::Vector2u{0, 0}) {
        InitTexture();
        isDirty = true;
    }

    if (!isDirty) {
        return;
    }

    // A 64x64 buffer of RGBA pixels
    sf::Color pixels[64 * 64];
    std::fill(std::begin(pixels), std::end(pixels), sf::Color::Transparent);

    // Populate only the ALIVE cells
    for (int r = 0; r < 64; ++r) {
        uint64_t rowBits = rows[r];
        int rowOffset    = r * 64;

        while (rowBits != 0) {
            int c = std::countr_zero(rowBits);

            // Set live cell pixel color
            pixels[rowOffset + c] = sf::Color::Green;

            rowBits &= (rowBits - 1);
        }
    }

    // Upload the pixel array directly to GPU memory
    texture.update(reinterpret_cast<const std::uint8_t*>(pixels), {64, 64}, {0, 0});
    isDirty = false;
}

bool Chunk::GetCell(sf::Vector2i localPos) const { return (rows[localPos.y] & (1ULL << localPos.x)) != 0; }

void Chunk::SetCell(sf::Vector2i localPos) {
    rows[localPos.y] |= (1ULL << localPos.x);
    isDirty = true;
}

void Chunk::ClearCell(sf::Vector2i localPos) {
    rows[localPos.y] &= ~(1ULL << localPos.x);
    isDirty = true;
}

void Chunk::ToggleCell(sf::Vector2i localPos) {
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

} // namespace GoL
