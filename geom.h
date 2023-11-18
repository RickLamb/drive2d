#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

struct Vec2D
{
    Vec2D() : x(0.0f), y(0.0f) {}
    Vec2D(float _x, float _y) : x(_x), y(_y) {}

    float x, y;
};

typedef Vec2D Vertex;

static inline float DegToRad(float degrees) {
    const float kDegToRad = float(M_PI) / 180.0f;
    return degrees * kDegToRad;
}

static inline const Vec2D Mult(const Vec2D& v, float a) {
    return Vec2D(v.x * a, v.y * a);
}

static inline const Vec2D Add(const Vec2D& a, const Vec2D& b) {
    return Vec2D(a.x + b.x, a.y + b.y);
}
static inline const Vec2D Sub(const Vec2D& a, const Vec2D& b) {
    return Vec2D(a.x - b.x, a.y - b.y);
}

static inline float Length(const Vec2D& a) {
    return sqrtf((a.x * a.x) + (a.y * a.y));
}

static inline const Vec2D RotateRadians(const Vec2D& v, float radians)
{
    float ca = cosf(radians);
    float sa = sinf(radians);
    return Vec2D(ca * v.x - sa * v.y, sa * v.x + ca * v.y);
}

static inline const Vec2D Normalize(const Vec2D& a) {
    float len = Length(a);
    return Mult(a, 1.0f / len);
}