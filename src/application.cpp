//#include "Image.h"
#include "mesh.h"
#include "texture.h"
#include "CubeMapTexture.h"
#include "BezierCurve.h"
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
#include <functional>
#include <vector>
#include <ctime>
#include <cstdlib>

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
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        m_curves = {};
        m_ball = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/wall/ball.obj");
        m_wall = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/wall/wall.obj");

        try {

            ShaderBuilder wallBuilder;
            wallBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/wall_vert.glsl");
            wallBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "Shaders/wall_frag.glsl");
            m_wallShader = wallBuilder.build();

            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();



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
        while (!m_window.shouldClose()) {
            // This is your game loop
            // Put your real-time logic and rendering in here
            m_window.updateInput();

            // Use ImGui for easy input/output of ints, floats, strings, etc...
            std::array displayModeNames { "1: BALL", "2: Curve"};

            ImGui::Begin("Window");


            ImGui::Combo("Scene", &currentMode, displayModeNames.data(), displayModeNames.size());
            if (currentMode == 0) {
                ImGui::Checkbox("Normal Mapping", &m_useNormalMapping);
                ImGui::Checkbox("Texture", &m_useTexture);
                ImGui::Checkbox("Environment Map", &m_useEnvMap);
            }
            if (currentMode == 1) {
                ImGui::SliderInt("Number of Bullets", &num_bullets, 1, 10);
                // shoot button
                if (ImGui::Button("Shoot")) {
                    m_curves = genCurves();
                }
            }

            ImGui::End();

            // Clear the screen
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_projectionMatrix = m_trackball.projectionMatrix();
            m_viewMatrix = m_trackball.viewMatrix();
            switch (currentMode) {
                case 0:
                    renderBall();
                    break;
                case 1:
                    renderCurve();
                    break;
            }


            m_window.swapBuffers();
        }
    }

    void renderBall(){
        // ...
        glEnable(GL_DEPTH_TEST);


        const glm::mat4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
        const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));

        // Draw the wall
        for (GPUMesh& mesh : m_ball) {
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
    }

    float randomFloat(float min, float max) {

        float unit =  (float)rand()/RAND_MAX;
        return min + unit * (max - min);
    }

    std::vector<BezierCurve> genCurves(){
        std::vector<BezierCurve> curves;
        glm::vec3 start_pos = glm::vec3(6.0f, 0.0f, 0.0f);
        glm::vec3 end_pos = glm::vec3(-6.0f, 0.0f, 0.0f);
        glm::vec3 offset = glm::vec3(0.0f, 1.0f, 0.0f);
        // x1 <- (0, 7) y1 <- (-6 , 6) z1 <- (-6, 6)
        // x2 <- (-7, 0) y2 <- (-6 , 6) z2 <- (-6, 6)
        // y1y2 同号, z1z2同号
        for (int i = 0; i < num_bullets; i++) {
            float y1 = randomFloat(-6.0f, 6.0f);
            float z1 = randomFloat(-6.0f, 6.0f);
            float y2 = randomFloat(-6.0f, 6.0f);
            float z2 = randomFloat(-6.0f, 6.0f);

            if (z1 * z2 < 0) {
                z1 = -z1;
            }
            if (y1 * y2 < 0) {
                y1 = -y1;
            }
            glm::vec3 control1 = glm::vec3(randomFloat(0.0f, 7.0f), y1, z1);
            glm::vec3 control2 = glm::vec3(randomFloat(-7.0f, 0.0f), y2, z2);
            BezierCurve curve = BezierCurve(start_pos + offset,
                                            control1 + offset,
                                            control2 + offset,
                                            end_pos + offset);
            curves.push_back(curve);
        }
        return curves;

    }


    void renderCurve(){
        m_defaultShader.bind();
        glm::vec3 start_pos = glm::vec3(6.0f, 0.0f, 0.0f);
        glm::vec3 end_pos = glm::vec3(-6.0f, 0.0f, 0.0f);
        glm::vec3 offset = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 wallModelMatrix = glm::translate(m_modelMatrix, end_pos);
        glm::mat4 ballModelMatrix = glm::translate(m_modelMatrix, start_pos);
        glm::mat4 mvp = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
        // Draw the wall
        for (GPUMesh& mesh : m_wall) {

            glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"), 1,
                               GL_FALSE, glm::value_ptr(m_projectionMatrix * m_viewMatrix * wallModelMatrix));
            mesh.draw(m_defaultShader);
        }
        for (GPUMesh& mesh : m_ball) {
            glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"), 1,
                               GL_FALSE, glm::value_ptr(m_projectionMatrix * m_viewMatrix * ballModelMatrix));
            mesh.draw(m_defaultShader);
        }
        glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"), 1,
                           GL_FALSE, glm::value_ptr( mvp));
      //  BezierCurve curve = BezierCurve(start_pos + offset, glm::vec3(6.0f, 0.0f, 6.0f) + offset, glm::vec3(-6.0f, 0.0f, 6.0f) + offset, end_pos + offset);
      for (BezierCurve curve : m_curves) {
          curve.draw();
      }
    }


private:
    Window m_window;

    Shader m_wallShader;
    Shader m_defaultShader;
    std::vector<GPUMesh> m_ball;
    std::vector<GPUMesh> m_wall;
    bool m_useNormalMapping = false;
    bool m_useTexture = true;
    bool m_useEnvMap = false;
    Texture m_wall_texture;
    Texture m_wall_normal;
    CubeMapTexture m_env_map;
    Trackball m_trackball;
    std::vector<BezierCurve> m_curves;


    int currentMode = 1;
    int num_bullets = 3;
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
