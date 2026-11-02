#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <bit>

namespace GoL {

struct Vector2iHash {
    std::size_t operator()(sf::Vector2i v) const { return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1); }
};

struct Chunk {
    std::array<uint64_t, 64> rows{};
    sf::Texture texture{};
    bool isDirty = true; // Set to true whenever cells change

    Chunk() = default;

    // When copied, copy the cells, but reset the texture so the copy creates its own GPU texture lazily
    Chunk(const Chunk& other);
    Chunk& operator=(const Chunk& other);
    Chunk(Chunk&& other) noexcept            = default;
    Chunk& operator=(Chunk&& other) noexcept = default;

    void InitTexture() { texture.resize({64, 64}); }
    void UpdateChunkTexture();

    bool GetCell(sf::Vector2i localPos) const; // Check if cell is alive
    void SetCell(sf::Vector2i localPos);       // Set cell to alive
    void ClearCell(sf::Vector2i localPos);     // Clear cell to dead
    void ToggleCell(sf::Vector2i localPos);    // Toggle cell state
    bool IsEmpty() const;                      // Returns true if the entire chunk contains zero alive cells
};

using ChunkMap = std::unordered_map<sf::Vector2i, Chunk, Vector2iHash>;

struct LocalCellPos {
    sf::Vector2i chunkPos;
    sf::Vector2i localPos;
};

inline LocalCellPos GlobalToLocal(sf::Vector2i globalPos) {
    int chunkX = globalPos.x >> 6;
    int chunkY = globalPos.y >> 6;

    int localX = globalPos.x & 63;
    int localY = globalPos.y & 63;

    return {
        {chunkX, chunkY},
        {localX, localY}
    };
}

} // namespace GoL
