#include "CelestialBody.h"

CelestialBody::CelestialBody(float radius, float spinPeriod, int orbitAround, float orbitAltitude, float orbitPeriod, std::string name, Texture texture)
        : m_radius(radius),
          m_spinPeriod(spinPeriod),
          m_orbitAround(orbitAround),
          m_orbitAltitude(orbitAltitude),
          m_orbitPeriod(orbitPeriod),
          m_name(std::move(name)),
          m_texture(std::move(texture)) // 使用移动构造
{
}

CelestialBody::CelestialBody(CelestialBody&& other) noexcept
        : m_radius(other.m_radius),
          m_spinPeriod(other.m_spinPeriod),
          m_orbitAround(other.m_orbitAround),
          m_orbitAltitude(other.m_orbitAltitude),
          m_orbitPeriod(other.m_orbitPeriod),
          m_name(std::move(other.m_name)),
          m_texture(std::move(other.m_texture)) // 移动 Texture 成员
{
}

CelestialBody& CelestialBody::operator=(CelestialBody&& other) noexcept
{
    if (this != &other) {
        m_radius = other.m_radius;
        m_spinPeriod = other.m_spinPeriod;
        m_orbitAround = other.m_orbitAround;
        m_orbitAltitude = other.m_orbitAltitude;
        m_orbitPeriod = other.m_orbitPeriod;
        m_name = std::move(other.m_name);
        m_texture = std::move(other.m_texture);
    }
    return *this;
}

float CelestialBody::getRadius() const { return m_radius; }
float CelestialBody::getSpinPeriod() const { return m_spinPeriod; }
int CelestialBody::getOrbitAround() const { return m_orbitAround; }
float CelestialBody::getOrbitAltitude() const { return m_orbitAltitude; }
float CelestialBody::getOrbitPeriod() const { return m_orbitPeriod; }
const std::string& CelestialBody::getName() const { return m_name; }
const Texture& CelestialBody::getTexture() const { return m_texture; }
Texture& CelestialBody::getTexture() {
    return m_texture;
}


