#pragma once

#include <vector>

#include "rasterizer.h"


class DriveableArea {
public:
    GLuint m_tex;
    DriveableArea() {
        m_tex = glInitTexture();
    }

    GLuint glInitTexture()
    {
        GLuint t = 0;

        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        unsigned char data[] = { 80, 80, 80, 255 };
        GLsizei w = 1;
        GLsizei h = 1;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        return t;
    }

    void Render() {
        glEnable(GL_TEXTURE_2D);

        glLoadIdentity();                   // Reset The Current Modelview Matrix
        glTranslatef(0.0f, 0.0f, 0.0f);              // Move Right 1.5 Units And Into The Screen 6.0
        //glRotatef(m_rot, 0.0f, 0.0f, 1.0f);            // Rotate The Quad On The X axis ( NEW )
        glScalef(0.5f, 1.0f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, m_tex);
        
        glColor3f(1.0f, 1.0f, 1.0f);            // Set The Color To White and just use texture
        glBegin(GL_QUADS);                      // Start Drawing A Quad
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);         // Top Left Of The Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);           // Top Right Of The Quad
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);          // Bottom Right Of The Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);          // Bottom Left Of The Quad
        glEnd();                                // Done Drawing The Quad

        glDisable(GL_TEXTURE_2D);
    }
};

class Lane {
public:

    void Render() {
        glLoadIdentity();                   // Reset The Current Modelview Matrix
        glTranslatef(0.0f, 0.0f, 0.0f);              // Move Right 1.5 Units And Into The Screen 6.0
        //glRotatef(m_rot, 0.0f, 0.0f, 1.0f);            // Rotate The Quad On The X axis ( NEW )
        glScalef(0.25f, 1.0f, 1.0f);

        glColor3f(0.5f, 0.5f, 0.0f);            // Set The Color To A Nice Yellow Shade
        glBegin(GL_QUADS);                      // Start Drawing A Quad
        glVertex3f(-1.0f, 1.0f, 0.0f);         // Top Left Of The Quad
        glVertex3f(1.0f, 1.0f, 0.0f);           // Top Right Of The Quad
        glVertex3f(1.0f, -1.0f, 0.0f);          // Bottom Right Of The Quad
        glVertex3f(-1.0f, -1.0f, 0.0f);          // Bottom Left Of The Quad
        glEnd();                                // Done Drawing The Quad

    }
};

class Vehicle {
public:
    float m_rot;
    Vec2D m_pos;
    Vec2D m_v;
    float m_radius;
    bool m_orbit = false;
    float m_time = 0.0f;

    Vehicle(float x, float y, float rot)
    : m_pos(x, y)
    , m_v(0.0f, 0.0f)
    , m_rot(rot)
    , m_radius(2.0f) // m
    {

    }

    Vehicle()
        : m_pos(0.0f, 0.0f)
        , m_v(0.0f, 0.0f)
        , m_rot(0.0f)
        , m_radius(2.0f) //m
        , m_orbit(true)
    {

    }
    void Update(float dt) {
        m_time += dt;
        if (m_orbit) {
            float radius = 35.0f; // metres
            static float baseSpeed = 13.0f; // 30 mph in m/s
            float circumference = 2.0f * float(M_PI) * radius;
            float speed = (2.0f * float(M_PI) * baseSpeed) / circumference;
            Vec2D newPos = Mult(Vec2D(sinf(speed*m_time), cosf(speed*m_time)), radius);
            Vec2D movement = Sub(newPos, m_pos);
            m_pos = newPos;
            m_v = Mult(movement, 1.0f / dt);
            //float velocity = Length(m_v);
            //int i = 0; ++i;
        }
        else {
            m_v = Vec2D(0.0f, 0.0f);
            //m_rot += 0.1f;
        }

    }

	void Render() {
        glLoadIdentity();                   // Reset The Current Modelview Matrix
        glTranslatef(m_pos.x, m_pos.y, 0.0f);              // Move Right 1.5 Units And Into The Screen 6.0
        glRotatef(m_rot, 0.0f, 0.0f, 1.0f);            // Rotate The Quad On The X axis ( NEW )
        glScalef(m_radius, m_radius, 1.0f);

        glColor3f(0.5f, 0.0f, 0.0f);            // Set The Color To A Nice Blue Shade
        glBegin(GL_QUADS);                      // Start Drawing A Quad
        glVertex3f(-1.0f, 1.0f, 0.0f);         // Top Left Of The Quad
        glVertex3f(1.0f, 1.0f, 0.0f);           // Top Right Of The Quad
        glVertex3f(1.0f, -1.0f, 0.0f);          // Bottom Right Of The Quad
        glVertex3f(-1.0f, -1.0f, 0.0f);          // Bottom Left Of The Quad
        glEnd();                                // Done Drawing The Quad

	}

};

class Simulator {
public:
    Simulator()
    {
        m_vehicles.push_back(Vehicle(0.0f, 0.0f, 90.0f));
        m_vehicles.push_back(Vehicle());
        m_velocityObstacles.resize(2);
    }

    void Update() {
        const float dt = 1.0f / 60.0f;

        const size_t numVehicles = m_vehicles.size();
        for (size_t i = 0; i<numVehicles; ++i) {
            const auto& a = m_vehicles[i];
            m_velocityObstacles[i].Clear();
            VelocityObstacle::Obstacle va;
            va.position = a.m_pos;
            va.velocity = a.m_v;
            va.radius = a.m_radius;
            for (size_t j = 0; j < numVehicles; ++j) {
                if (i == j) {
                    continue;
                }
                const auto& b = m_vehicles[j];

                VelocityObstacle::Obstacle vb;
                vb.position = b.m_pos;
                vb.velocity = b.m_v;
                vb.radius = b.m_radius;

                va.bias = 0.0f; // 0.5f;
                vb.bias = 1.0f; // 0.5f;

                float dist = Length(Sub(vb.position, va.position));
                if (dist > FLT_EPSILON) {
                    VelocityObstacle ob(va, vb);
                    m_velocityObstacles[i].drawTriangle(ob);
                } else {
                    //TODO: actively colliding...
                }
            }

            m_velocityObstacles[i].UpdateDebugTexture();
        }

        for (auto& v : m_vehicles) {
            v.Update(dt);
        }

        m_road.Render();
        m_lane.Render();
        for (auto& v : m_vehicles) {
            v.Render();
        }

        static bool showDebugImage = true;
        if (showDebugImage)
        {
            ImGui::Begin("VelocityObstacle", &showDebugImage);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImTextureID my_tex_id = (ImTextureID) m_velocityObstacles[0].m_texId;
            float my_tex_w = float(4*m_velocityObstacles[0].kRange);
            float my_tex_h = float(4*m_velocityObstacles[0].kRange);
            ImVec2 uv_min = ImVec2(0.0f, 1.0f);                 // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 0.0f);                 // Lower-right
            ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
            ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);   // No border 
            ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
            ImGui::End();
        }
    }

    DriveableArea                   m_road;
    Lane                            m_lane;
    std::vector<Vehicle>            m_vehicles;

    std::vector<VORasterizer>       m_velocityObstacles;
};