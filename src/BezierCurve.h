#pragma once


#include <framework/disable_all_warnings.h>
#include <framework/mesh.h>
#include <framework/shader.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <exception>
#include <filesystem>
#include <chrono>
#include <framework/opengl_includes.h>

class BezierCurve {
public:
    BezierCurve(const glm::vec3& start, const glm::vec3& control1, const glm::vec3& control2, const glm::vec3& end);

    void draw();
    void cleanup();
    glm::vec3 get_animation_point(float period) const;

private:
    glm::vec3 getPoint(float t) const;

    glm::vec3 p0, p1, p2, p3;
    GLuint vao, vbo;
    std::vector<glm::vec3> points;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};