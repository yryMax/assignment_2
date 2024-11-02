#pragma once
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <exception>
#include <filesystem>
#include <framework/opengl_includes.h>
#include <vector>

class CubeMapTexture {
public:
    CubeMapTexture(std::vector<std::filesystem::path> faces);
    CubeMapTexture(CubeMapTexture&& other) noexcept;
    ~CubeMapTexture();

    void bind(GLint textureSlot = GL_TEXTURE0) const;

private:
    GLuint m_texture = 0;
    static constexpr GLuint INVALID = 0;
};
