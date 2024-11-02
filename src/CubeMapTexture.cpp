#include "CubeMapTexture.h"
#include "framework/image.h"
#include <iostream>

CubeMapTexture::CubeMapTexture(std::vector<std::filesystem::path> faces)
{
    if (faces.size() != 6) {
        std::cerr << "Error: CubeMapTexture requires exactly 6 faces" << std::endl;
        throw std::runtime_error("Incorrect number of faces for CubeMapTexture");
    }

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);

    for (GLuint i = 0; i < faces.size(); i++) {
        Image faceImage { faces[i] };

        GLenum format;
        switch (faceImage.channels) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default:
                std::cerr << "Unsupported number of channels in cube map face" << std::endl;
                throw std::runtime_error("Unsupported texture format");
        }

        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                format,
                faceImage.width, faceImage.height,
                0,
                format, GL_UNSIGNED_BYTE,
                faceImage.get_data()
        );
        // pring the width and height of the image
    //    std::cout << "width: " << faceImage.width << " height: " << faceImage.height << std::endl;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

CubeMapTexture::CubeMapTexture(CubeMapTexture&& other) noexcept
        : m_texture(other.m_texture)
{
    other.m_texture = INVALID;
}

CubeMapTexture::~CubeMapTexture()
{
    if (m_texture != INVALID)
        glDeleteTextures(1, &m_texture);
}

void CubeMapTexture::bind(GLint textureSlot) const
{
    glActiveTexture(textureSlot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
}
