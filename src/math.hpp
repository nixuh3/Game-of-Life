#pragma once

#include <unordered_map>

namespace gol {

template <typename T>
struct Vec2 {
    T x, y;

    bool operator==(const Vec2& rhs) const { return x == rhs.x && y == rhs.y; }

    Vec2& operator+=(const Vec2& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Vec2& operator-=(const Vec2& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    Vec2& operator*=(const Vec2& rhs) noexcept {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }
    Vec2& operator/=(const Vec2& rhs) noexcept {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    Vec2 operator+(const Vec2& rhs) const noexcept { return Vec2(*this) += rhs; }
    Vec2 operator-(const Vec2& rhs) const noexcept { return Vec2(*this) -= rhs; }
    Vec2 operator*(const Vec2& rhs) const noexcept { return Vec2(*this) *= rhs; }
    Vec2 operator/(const Vec2& rhs) const noexcept { return Vec2(*this) /= rhs; }

    Vec2 operator*(const T& scalar) const noexcept { return { x * scalar, y * scalar }; }
    Vec2 operator/(const T& scalar) const noexcept { return { x / scalar, y / scalar }; }

    struct Hash {
        size_t operator()(const Vec2<T>& v) const noexcept {
            uint64_t packed = (static_cast<uint64_t>(v.x) << 32) | (static_cast<uint32_t>(v.y));
            return std::hash<uint64_t>{}(packed);
        }
    };
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;

// Half Adder: Sums two 64-bit words bit-by-bit
constexpr void HalfAdd(uint64_t a, uint64_t b, uint64_t& sum, uint64_t& carry) {
    sum = a ^ b;
    carry = a & b;
}

// Full Adder: Sums three 64-bit words bit-by-bit
constexpr void FullAdd(uint64_t a, uint64_t b, uint64_t c, uint64_t& sum, uint64_t& carry) {
    sum = a ^ b ^ c;
    carry = (a & b) | (c & (a ^ b));
}

}
