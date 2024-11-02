#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Texture.h"

class CelestialBody {
public:


    CelestialBody(const CelestialBody&) = default;
    CelestialBody& operator=(const CelestialBody&) = delete;


    CelestialBody(CelestialBody&& other) noexcept;


    CelestialBody& operator=(CelestialBody&& other) noexcept;


    CelestialBody(float radius, float spinPeriod, int orbitAround, float orbitAltitude, float orbitPeriod, std::string name, Texture texture);

    float getRadius() const;
    float getSpinPeriod() const;
    int getOrbitAround() const;
    float getOrbitAltitude() const;
    float getOrbitPeriod() const;
    const std::string& getName() const;
    const Texture& getTexture() const;
    Texture& getTexture();




private:
    float m_radius { 1.0f };
    float m_spinPeriod { 0.0f };

    int m_orbitAround { -1 };
    float m_orbitAltitude { 0.0f };
    float m_orbitPeriod { 0.0f };

    std::string m_name;
    Texture m_texture;
};






