#include "BezierCurve.h"
#pragma once

#include <framework/disable_all_warnings.h>
#include <framework/mesh.h>
#include <framework/shader.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <exception>
#include <filesystem>
#include <framework/opengl_includes.h>


BezierCurve::BezierCurve(const glm::vec3& start, const glm::vec3& control1, const glm::vec3& control2, const glm::vec3& end)
            : p0(start), p1(control1), p2(control2), p3(end), vao(0), vbo(0) {
    int numSegments = 100;
    points.clear();
    for (int i = 0; i <= numSegments; ++i) {
        float t = i / (float)numSegments;
        points.push_back(getPoint(t));
    }
    startTime = std::chrono::high_resolution_clock::now();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void BezierCurve :: draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, points.size());
    glBindVertexArray(0);
}
void BezierCurve :: cleanup() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    vbo = vao = 0;
}
glm::vec3 BezierCurve:: getPoint(float t) const {
    float u = 1 - t;
    return u * u * u * p0 + 3 * u * u * t * p1 + 3 * u * t * t * p2 + t * t * t * p3;
}

glm::vec3 BezierCurve:: get_animation_point(float period) const {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<float>(now - startTime).count();
    if (elapsed > period) {
        return glm::vec3(-1.0f);
    }
    float t = elapsed / period;
    return getPoint(t);
}

