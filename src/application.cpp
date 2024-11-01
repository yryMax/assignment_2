//#include "Image.h"
#include "mesh.h"
#include "texture.h"
#include "CubeMapTexture.h"
#include "framework/trackball.h"
// Always include window first (because it includes glfw, which includes GL which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>
#include <functional>
#include <iostream>
#include <vector>

std::vector<std::filesystem::path> faces = {
    RESOURCE_ROOT "resources/environment_map/right.jpg",
    RESOURCE_ROOT "resources/environment_map/left.jpg",
    RESOURCE_ROOT "resources/environment_map/top3.jpg",
    RESOURCE_ROOT "resources/environment_map/bottom5.jpg",
    RESOURCE_ROOT "resources/environment_map/front.jpg",
    RESOURCE_ROOT "resources/environment_map/back.jpg"
};

class Application {
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL41),
            m_wall_texture(RESOURCE_ROOT "resources/wall/texture.jpg"),
            m_wall_normal(RESOURCE_ROOT "resources/wall/normal.jpg"),
            m_env_map(faces),
            m_trackball { &m_window, glm::radians(50.0f) }
    {
        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods);
        });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods) {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods);
        });

        m_wall = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/wall/ball.obj");

        try {

            ShaderBuilder wallBuilder;
            wallBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/wall_vert.glsl");
            wallBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "Shaders/wall_frag.glsl");
            m_wallShader = wallBuilder.build();

        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }


        glm::vec3 lookAt = glm::vec3(-0.215165, 0.305033, 0.645589);
        glm::vec3 rotations = glm::vec3(0.450295, 0.617848, 0.0);
        float dist = 3.0f;
        m_trackball.setCamera(lookAt, rotations, dist);
    }

    void update()
    {
        int dummyInteger = 0; // Initialized to 0
        while (!m_window.shouldClose()) {
            // This is your game loop
            // Put your real-time logic and rendering in here
            m_window.updateInput();

            // Use ImGui for easy input/output of ints, floats, strings, etc...
            ImGui::Begin("Window");
            ImGui::Checkbox("Normal Mapping", &m_useNormalMapping);
            ImGui::Checkbox("Texture", &m_useTexture);
            ImGui::Checkbox("Environment Map", &m_useEnvMap);
            ImGui::End();

            // Clear the screen
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ...
            glEnable(GL_DEPTH_TEST);
            m_projectionMatrix = m_trackball.projectionMatrix();
            m_viewMatrix = m_trackball.viewMatrix();

            const glm::mat4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
            // Normals should be transformed differently than positions (ignoring translations + dealing with scaling):
            // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
            const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

            // Draw the wall
            for (GPUMesh& mesh : m_wall) {
                m_wallShader.bind();
                glUniformMatrix4fv(m_wallShader.getUniformLocation("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix));
                //Uncomment this line when you use the modelMatrix (or fragmentPosition)
                //glUniformMatrix4fv(m_defaultShader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
                glUniformMatrix3fv(m_wallShader.getUniformLocation("normalModelMatrix"), 1, GL_FALSE, glm::value_ptr(normalModelMatrix));
                m_wall_texture.bind(GL_TEXTURE0);
                glUniform1i(m_wallShader.getUniformLocation("colorMap"), 0);
                m_wall_normal.bind(GL_TEXTURE1);
                glUniform1i(m_wallShader.getUniformLocation("normalMap"), 1);
                glUniform1i(m_wallShader.getUniformLocation("useNormalMapping"), m_useNormalMapping ? GL_TRUE : GL_FALSE);
                glUniform1i(m_wallShader.getUniformLocation("useTexture"), m_useTexture ? GL_TRUE : GL_FALSE);
                glUniform1i(m_wallShader.getUniformLocation("useEnvMap"), m_useEnvMap ? GL_TRUE : GL_FALSE);
                m_env_map.bind(GL_TEXTURE2);
                glUniform1i(m_wallShader.getUniformLocation("envMap"), 2);
                glad_glUniform3fv(m_wallShader.getUniformLocation("cameraPos"), 1, glm::value_ptr(m_trackball.position()));


                mesh.draw(m_wallShader);
            }

            m_window.swapBuffers();
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
        std::cout << "Key released: " << key << std::endl;
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2& cursorPos)
    {
        std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        std::cout << "Pressed mouse button: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {
        std::cout << "Released mouse button: " << button << std::endl;
    }

private:
    Window m_window;

    Shader m_wallShader;
    std::vector<GPUMesh> m_wall;
    bool m_useNormalMapping = false;
    bool m_useTexture = true;
    bool m_useEnvMap = false;
    Texture m_wall_texture;
    Texture m_wall_normal;
    CubeMapTexture m_env_map;
    Trackball m_trackball;



    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 30.0f);
    glm::mat4 m_viewMatrix = glm::lookAt(glm::vec3(-3, 3, -3), glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 m_modelMatrix { 1.0f };
};

int main()
{
    Application app;
    app.update();
    return 0;
}
