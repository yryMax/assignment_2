//#include "Image.h"
#include "mesh.h"
#include "texture.h"
#include "CubeMapTexture.h"
#include "BezierCurve.h"
#include "CelestialBody.h"
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
#include <map>
#include <chrono>

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
        m_solarSystem = loadSolarSystem();
        m_planet = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/planet/planet.obj");
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
            std::array displayModeNames { "1: BALL", "2: Curve", "3: Solar System" };

            ImGui::Begin("Window");


            ImGui::Combo("Scene", &currentMode, displayModeNames.data(), displayModeNames.size());
            if (currentMode == 0) {
                ImGui::Checkbox("Normal Mapping", &m_useNormalMapping);
                ImGui::Checkbox("Texture", &m_useTexture);
                ImGui::Checkbox("Environment Map", &m_useEnvMap);
            }
            if (currentMode == 1) {
                ImGui::SliderInt("Number of Bullets", &num_bullets, 1, 10);
                ImGui::Checkbox("Show Curve", &m_showcurve);
                ImGui::SliderFloat("Speed", &m_speed, 0.1f, 10.0f);
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
                case 2:
                    renderSolarSystem();
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
        glUniform1i(m_defaultShader.getUniformLocation("useMaterial"), GL_FALSE);
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
        if(m_showcurve) {
            for (BezierCurve curve: m_curves) {
                curve.draw();
            }
        }
      for (BezierCurve curve : m_curves) {
          glm::vec3 pos = curve.get_animation_point(10.0 - m_speed);
     //     std::cout<< pos.x << " " << pos.y << " " << pos.z << std::endl;
            if (pos == glm::vec3(-1.0f)) continue;
          glm::mat4 bulletModelMatrix = translationMatrix(pos) * scaleMatrix(glm::vec3(0.1f, 0.1f, 0.1f));
            glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"), 1,
                                 GL_FALSE, glm::value_ptr(m_projectionMatrix * m_viewMatrix * bulletModelMatrix));
          for (GPUMesh& mesh : m_wall) {
                mesh.draw(m_defaultShader);
          }

      }
    }

    std::vector<CelestialBody> loadSolarSystem() {
        CelestialBody sun =  {
                10.0f,
                60.0f,
                -1,
                0.0f,
                0.0f,
                "Sun",
                Texture("resources/solar_system/2k_sun.jpg")
        };

        CelestialBody mercury = {
                2.5f,
                140.f,
                0,
                15.7f,
                8.8f,
                "Mercury",
                Texture("resources/solar_system/2k_mercury.jpg")
        };

        CelestialBody earth = {
                6.3f,
                2.4f,
                0,
                30.0f,
                36.5f,
                "Earth",
                Texture("resources/solar_system/2k_earth_clouds.jpg")
        };

        CelestialBody moon = {
                1.7f,
                65.0f,
                2,
                10.0f,
                65.0f,
                "Moon",
                Texture("resources/solar_system/2k_moon.jpg")
        };
        std::vector<CelestialBody> solarSystem;
        solarSystem.reserve(4);
        solarSystem.push_back(std::move(sun));
        solarSystem.push_back(std::move(mercury));
        solarSystem.push_back(std::move(earth));
        solarSystem.push_back(std::move(moon));
        return solarSystem;
    }

    static glm::mat4 rotationMatrix(float angle, const glm::vec3& axis)
    {
        return glm::rotate(glm::identity<glm::mat4>(), angle, axis);
    }
    static glm::mat4 translationMatrix(const glm::vec3& translation)
    {
        return glm::translate(glm::identity<glm::mat4>(), translation);
    }
    static glm::mat4 scaleMatrix(const glm::vec3& scale)
    {
        return glm::scale(glm::identity<glm::mat4>(), scale);
    }

    void calc(int u, std::map<int, glm::mat4>& trans, float time, glm::mat4 m)
    {
      //  std::cout << u << std::endl;
        trans[u] = m;
        int i = 0;
        for (CelestialBody &v : m_solarSystem) {

            if (v.getOrbitAround() != u){
                i++;
                continue;
            }
    //       std::cout << v.getName() << " "<< v.getOrbitAround() << " " << i << std::endl;
            float rotate =v.getOrbitPeriod();
            float altitude = v.getOrbitAltitude();
            float angle = ((time / rotate) - floor(time / rotate)) * 2.0f * glm::pi<float>();
            glm::mat4 mm = m * rotationMatrix(angle, glm::vec3(0, 1, 0)) * translationMatrix(glm::vec3(altitude, 0, 0)) * rotationMatrix(-angle, glm::vec3(0, 1, 0));
            calc(i, trans, time, mm);
            i++;
        }

    }
    std::vector<glm::mat4> computeCelestrialBodyTransformations(float time)
    {
        std::map<int, glm::mat4> trans;
        for (int i = 0; i < m_solarSystem.size(); i++) {
            if (m_solarSystem[i].getOrbitAround() == -1)
                calc(i, trans, time, glm::identity<glm::mat4>());
        }

        std::vector<glm::mat4> transforms;
        for (int i = 0; i < m_solarSystem.size();i++) {
            float spin = m_solarSystem[i].getSpinPeriod();
            float angle = ((time / spin) - floor(time / spin)) * 2 * glm::pi<float>();
            float r = m_solarSystem[i].getRadius();
            transforms.push_back(trans[i]*scaleMatrix(glm::vec3(r, r, r)) * rotationMatrix(angle, glm::vec3(0, 1, 0)));
        }
        return transforms;
    }

    void renderSolarSystem() {
        // time
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        // timespan in seconds
        float time = std::chrono::duration<float>(now - start).count();

        std::vector<glm::mat4> transforms = computeCelestrialBodyTransformations(time);
        m_defaultShader.bind();

        glUniform1i(m_defaultShader.getUniformLocation("useMaterial"), GL_TRUE);
        glUniform1i(m_defaultShader.getUniformLocation("colorMap"), 0);
        int i = 0;

        for (CelestialBody&body : m_solarSystem) {
            glm::mat4 mvp = m_projectionMatrix * m_viewMatrix * transforms[i];
            glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"), 1,
                               GL_FALSE, glm::value_ptr(mvp));
            body.getTexture().bind(GL_TEXTURE0);
            for (GPUMesh& mesh : m_planet) {
                mesh.draw(m_defaultShader);
            }
            i++;
        }

    }


private:
    Window m_window;

    Shader m_wallShader;
    Shader m_defaultShader;
    std::vector<GPUMesh> m_ball;
    std::vector<GPUMesh> m_wall;
    std::vector<GPUMesh> m_planet;
    bool m_useNormalMapping = false;
    bool m_useTexture = true;
    bool m_useEnvMap = false;
    float m_speed = 9.0f;
    bool m_showcurve = false;
    Texture m_wall_texture;
    Texture m_wall_normal;
    CubeMapTexture m_env_map;
    Trackball m_trackball;
    std::vector<BezierCurve> m_curves;
    std::vector<CelestialBody> m_solarSystem;

    // timer
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
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
