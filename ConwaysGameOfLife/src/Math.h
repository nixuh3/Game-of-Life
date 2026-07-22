#pragma once

#include <unordered_map>

namespace GoL {

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

    struct Hash {
        size_t operator()(const Vec2<T>& v) const noexcept {
            size_t h1 = std::hash<T>{}(v.x);
            size_t h2 = std::hash<T>{}(v.y);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;

}
