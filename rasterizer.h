#pragma once

// https://trenki2.github.io/blog/2017/06/06/developing-a-software-renderer-part1/
// TODO: add more optimzations from this article if needed

#include <algorithm>
#include <array>

#include "geom.h"

struct Triangle
{
    Vertex v0, v1, v2;
};

class VelocityObstacle {
public:

    // make edges long enough too not need to worry about triangle vs infinite cone
    const float m_infEdgeLen = 1000.0f;

    struct Obstacle
    {
        Vertex  position;
        Vec2D   velocity;
        float   radius;
        // obstacle has right of way bias = 1.0, no right of way established ~0.4
        // obstacle giving way bias = 0.0
        float   bias;
    };

    // e.g. B has complete right of way coef_b = 1.0, coef_a = 0.0
    // e.g. B has right of way, but A in encroaching on lane coef_b = 0.9, coef_a = 1.0
    // e.g. No right of way established coef_b = 0.4, coef_b = 0.4
    VelocityObstacle(const Obstacle& a, const Obstacle& b)
    : m_a(a)
    , m_b(b)
    , m_rightEdgeDir(0.0, 0.0)
    , m_leftEdgeDir(0.0, 0.0)
    , m_rightVertex(0.0, 0.0)
    , m_leftVertex(0.0, 0.0)
    , m_apex(0.0, 0.0)
    {
        m_apex = Add(Mult(m_a.velocity, m_a.bias), Mult(m_b.velocity, m_b.bias));
        Vec2D offset = Sub(m_b.position, m_a.position);
        float dist = Length(offset);
        float coneAngle = asinf((m_a.radius + m_b.radius) / dist);
        Vec2D leftEdge = RotateRadians(offset, -coneAngle);
        m_leftEdgeDir = Normalize(leftEdge);
        Vec2D rightEdge = RotateRadians(offset, coneAngle);
        m_rightEdgeDir = Normalize(rightEdge);
        m_leftVertex = Add(m_apex, Mult(m_leftEdgeDir, m_infEdgeLen));
        m_rightVertex = Add(m_apex, Mult(m_rightEdgeDir, m_infEdgeLen));

        m_tri.v0 = m_apex;
        m_tri.v1 = m_leftVertex;
        m_tri.v2 = m_rightVertex;
    }

    float CalcTimeToCollision(float x, float y) const {
        Vec2D newVelocity(x, y);
        Vec2D relativeVelocity = Sub(newVelocity, m_apex);
        Vec2D relativePosition = Sub(m_a.position, m_b.position);
        float r_total = m_a.radius + m_b.radius;
        float r_total_sqr = r_total * r_total;
        // setup quadratic to solve time of intersection
        float a = (relativeVelocity.x * relativeVelocity.x) + (relativeVelocity.y * relativeVelocity.y);
        float b = 2.0f * (relativeVelocity.x*relativePosition.x + relativeVelocity.y*relativePosition.y);
        float c = (relativePosition.x * relativePosition.x) + (relativePosition.y * relativePosition.y) - r_total_sqr;
        float discriminant = (b * b) - (4.0f * a * c);
        //b ^ 2 - 4ac is the discriminant
        //if the discriminant is negative the roots will be complex numbers (and different) so no collision and no ttc
        if (discriminant < 0.0f) {
            return FLT_MAX;
        }
        float root1 = (-b + sqrtf(discriminant)) / (2 * a);
        float root2 = (-b - sqrtf(discriminant)) / (2 * a);
        // handle negative roots
        float time1 = (root1 >= 0.0f) ? root1 : FLT_MAX;
        float time2 = (root2 >= 0.0f) ? root2 : FLT_MAX;
        if (time1 < time2) {
            return time1;
        }
        return time2;
    }

    Obstacle    m_a;    // current agent
    Obstacle    m_b;    // obstacle to avoid

    Vec2D       m_rightEdgeDir;
    Vec2D       m_leftEdgeDir;

    Vertex      m_apex;
    Vertex      m_leftVertex;
    Vertex      m_rightVertex;

    Triangle    m_tri;
};

class VORasterizer {
public:
    int m_minX, m_maxX;
    int m_minY, m_maxY;
    GLuint m_texId;

    static const int kRange = 128;
    static const int kHalfRange = kRange/2;

    // -50 ... 0 ... 50 inclusive
    std::array< std::array< float, kRange >, kRange >   m_map;

    unsigned char m_data[4*kRange*kRange] = { 128 };

    VORasterizer()
    : m_minX(-kHalfRange)
    , m_minY(-kHalfRange)
    , m_maxX(kHalfRange - 1)
    , m_maxY(kHalfRange - 1)
    {
        Clear();
        m_texId = glInitTexture();
    }

    void UpdateDebugTexture() {
        int runner = 0;
        for (int i = 0; i < kRange; ++i) {
            for (int j = 0; j < kRange; ++j) {
                //float v = m_map[i][kRange - j - 1]; // flip and swap row/col to match debug draw of texture TODO: this might be a bug
                float v = m_map[j][i]; // flip and swap row/col to match debug draw of texture TODO: this might be a bug
                m_data[runner++] = int(v * 255.0f);
                m_data[runner++] = int(v * 255.0f);
                m_data[runner++] = int(v * 255.0f);
                m_data[runner++] = int(255.0f);
            }
        }

        glBindTexture(GL_TEXTURE_2D, m_texId);
        GLsizei w = kRange;
        GLsizei h = kRange;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
    }

    GLuint glInitTexture()
    {
        GLuint t = 0;

        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //unsigned char data[] = { 80, 80, 80, 255 };
        GLsizei w = kRange;
        GLsizei h = kRange;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
        return t;
    }

    void Clear() {
        for (auto& row : m_map) {
            row.fill(1.0f);
        }
    }

struct EdgeEquation {
    float a;
    float b;
    float c;
    bool tie;

    EdgeEquation(const Vertex& v0, const Vertex& v1)
    {
        a = v0.y - v1.y;
        b = v1.x - v0.x;
        c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) / 2;
        tie = a != 0 ? a > 0 : b > 0;
    }

    /// Evaluate the edge equation for the given point.
    float evaluate(float x, float y)
    {
        return a * x + b * y + c;
    }

    /// Test if the given point is inside the edge.
    bool test(float x, float y)
    {
        return test(evaluate(x, y));
    }

    /// Test for a given evaluated value.
    bool test(float v)
    {
        return (v > 0 || v == 0 && tie);
    }
};

void drawTriangle(const VelocityObstacle& vo)
{
    const Vertex& v0 = vo.m_tri.v0;
    const Vertex& v1 = vo.m_tri.v1;
    const Vertex& v2 = vo.m_tri.v2;

    // Compute triangle bounding box.
    int minX = (int)std::min(std::min(v0.x, v1.x), v2.x);
    int maxX = (int)std::max(std::max(v0.x, v1.x), v2.x);
    int minY = (int)std::min(std::min(v0.y, v1.y), v2.y);
    int maxY = (int)std::max(std::max(v0.y, v1.y), v2.y);

    // Clip to scissor rect.
    minX = std::max(minX, m_minX);
    maxX = std::min(maxX, m_maxX);
    minY = std::max(minY, m_minY);
    maxY = std::min(maxY, m_maxY);

    // Compute edge equations.
    EdgeEquation e0(v1, v2);
    EdgeEquation e1(v2, v0);
    EdgeEquation e2(v0, v1);

    float area = 0.5f * (e0.c + e1.c + e2.c);

    // Check if triangle is backfacing.
    //assert(area >= 0.0f);
    if (area <= 0.0f) {
        return;
    }

    //ParameterEquation r(v0.r, v1.r, v2.r, e0, e1, e2, area);
    //ParameterEquation g(v0.g, v1.g, v2.g, e0, e1, e2, area);
    //ParameterEquation b(v0.b, v1.b, v2.b, e0, e1, e2, area);

    // Add 0.5 to sample at pixel centers.
    for (int x = minX, xm = maxX; x <= xm; x++) {
        for (int y = minY, ym = maxY; y <= ym; y++) {
            float xf = float(x) + 0.5f;
            float yf = float(y) + 0.5f;

            if (e0.test(x, y) && e1.test(x, y) && e2.test(x, y)) {
                int iX = x + kHalfRange;
                int iY = y + kHalfRange;
                //int rint = r.evaluate(x, y) * 255;
                //int gint = g.evaluate(x, y) * 255;
                //int bint = b.evaluate(x, y) * 255;
                //Uint32 color = SDL_MapRGB(m_surface->format, rint, gint, bint);
                //putpixel(m_surface, x, y, color);
                float timeToCollision = vo.CalcTimeToCollision(xf, yf);
                static float kTimeCutoff = 1.0f;
                if (timeToCollision < kTimeCutoff) {
                    m_map[iX][iY] = 0.0f;
                }
                int i = 0; ++i;
            }
        }
    }

}


};
