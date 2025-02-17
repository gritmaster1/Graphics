#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "app/App.h"
#include "shape/Line.h"
#include "shape/Mesh.h"
#include "shape/Sphere.h"
#include "shape/Tetrahedron.h"
#include "shape/icosahedron.h" 
#include "shape/Docahedron.h"
#include "util/Shader.h"

int RenderingMode = 7;
float TessGranularityforTorus = 15;
float GranularituySuperQuadric = 15;
bool UseFreeCamera = true;
bool HorizontalCamer = true;
int inDex = 0;
App & App::getInstance()
{
    static App instance;
    return instance;
}


void App::run()
{
    while (!glfwWindowShouldClose(pWindow))
    {
        // Per-frame logic
        perFrameTimeLogic(pWindow);
        processKeyInput(pWindow);

        // Send render commands to OpenGL server
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render();

        // Check and call events and swap the buffers
        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }
}


void App::cursorPosCallback(GLFWwindow * window, double xpos, double ypos)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    app.mousePos.x = xpos;
    app.mousePos.y = App::kWindowHeight - ypos;

    if (app.mousePressed)
    {
        // Note: Must calculate offset first, then update lastMouseLeftPressPos.
        glm::dvec2 offset = app.mousePos - app.lastMouseLeftPressPos;
        app.lastMouseLeftPressPos = app.mousePos;
        app.camera.processMouseMovement(offset.x, offset.y);
    }
}


void App::framebufferSizeCallback(GLFWwindow * window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void App::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_EQUAL && mods & GLFW_MOD_SHIFT) {
                // The + key (Shift + =) is pressed
                if (RenderingMode == 2)
                {
                    icosahedron* G =  (icosahedron*)App::getInstance().shapes_mode_2[0].get();
                    G->subDivide();
                    G = nullptr;
                }
                else if (RenderingMode == 3)
                {
                    icosahedron* G = (icosahedron*)App::getInstance().shapes_mode_3[0].get();
                    G->subDivide();
                    G = nullptr;
                }
                else if (RenderingMode == 5)
                {
                    TessGranularityforTorus *= 2;

                    App::getInstance().pSphereShader->use();
                    App::getInstance().pSphereShader->setFloat("tessLevelOuter", TessGranularityforTorus);

                }
                else if (RenderingMode == 6)
                {
                    GranularituySuperQuadric *= 2;

                    App::getInstance().pSphereShader->use();
                    App::getInstance().pSphereShader->setFloat("tessLevelOuter", GranularituySuperQuadric);


                    docadehedron* G = (docadehedron*)App::getInstance().shapes_mode_6[1].get();
                    G->subDivide();
                    G = nullptr;
                }


            }
            else if (key == GLFW_KEY_UP)
            {
                inDex++;
            }
            else if (key == GLFW_KEY_DOWN)
            {
                inDex--;
            }

        }

    
}


void App::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            app.mousePressed = true;
            app.lastMouseLeftClickPos = app.mousePos;
            app.lastMouseLeftPressPos = app.mousePos;
        }
        else if (action == GLFW_RELEASE)
        {
            app.mousePressed = false;
        }
    }
}


void App::scrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    app.camera.processMouseScroll(yoffset);
}


void App::perFrameTimeLogic(GLFWwindow * window)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    double currentFrame = glfwGetTime();
    app.timeElapsedSinceLastFrame = currentFrame - app.lastFrameTimeStamp;
    app.lastFrameTimeStamp = currentFrame;
}


void App::processKeyInput(GLFWwindow * window)
{
    // Camera control
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kLeft, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kRight, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kBackWard, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kForward, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kUp, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kDown, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_F1))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (glfwGetKey(window, GLFW_KEY_F2))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        app.pMeshShader->use();
        app.pMeshShader->setInt("displayMode", 1);
        app.pSphereShader->use();
        app.pSphereShader->setInt("displayMode", 1);
    }

    if (glfwGetKey(window, GLFW_KEY_F3))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        app.pMeshShader->use();
        app.pMeshShader->setInt("displayMode", 2);
        app.pSphereShader->use();
        app.pSphereShader->setInt("displayMode", 2);
    }

    if (glfwGetKey(window, GLFW_KEY_F4))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        app.pMeshShader->use();
        app.pMeshShader->setInt("displayMode", 0);
        app.pSphereShader->use();
        app.pSphereShader->setInt("displayMode", 0);
    }

    if (glfwGetKey(window, GLFW_KEY_1))
    {
        UseFreeCamera = true;

        RenderingMode = 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_2))
    {
        UseFreeCamera = true;

        RenderingMode = 2;
    }
    else if (glfwGetKey(window, GLFW_KEY_3))
    {
        UseFreeCamera = true;

        RenderingMode = 3;
    }
    else if (glfwGetKey(window, GLFW_KEY_4))
    {
        UseFreeCamera = true;

        RenderingMode = 4;
    }
    else if (glfwGetKey(window, GLFW_KEY_5))
    {
        UseFreeCamera = true;

        RenderingMode = 5;
    }
    else if (glfwGetKey(window, GLFW_KEY_6))
    {
        UseFreeCamera = true;

        RenderingMode = 6;
    }
    else if (glfwGetKey(window, GLFW_KEY_7))
    {
        RenderingMode = 7;
        UseFreeCamera = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_H))
    {
        HorizontalCamer = true;
       App::getInstance().HorizontalCamera->startKeyFrameCamera();

    }
    else if (glfwGetKey(window, GLFW_KEY_V))
    {
        HorizontalCamer = false;
        App::getInstance().VerticalCamera->startKeyFrameCamera();

    }


}


App::App() : Window(kWindowWidth, kWindowHeight, kWindowName, nullptr, nullptr)
{
    // GLFW boilerplate.
    glfwSetWindowUserPointer(pWindow, this);
    glfwSetCursorPosCallback(pWindow, cursorPosCallback);
    glfwSetFramebufferSizeCallback(pWindow, framebufferSizeCallback);
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetMouseButtonCallback(pWindow, mouseButtonCallback);
    glfwSetScrollCallback(pWindow, scrollCallback);

    // Global OpenGL pipeline settings
    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(2.0f);
    glPointSize(1.0f);
    glEnable(GL_DEPTH_TEST);

    initializeShadersAndObjects();
}




void App::initializeShadersAndObjects()
{
    pLineShader = std::make_unique<Shader>("src/shader/line.vert.glsl",
                                           "src/shader/line.frag.glsl");

    pMeshShader = std::make_unique<Shader>("src/shader/mesh.vert.glsl",
                                           "src/shader/phong.frag.glsl");

    pSphereShader = std::make_unique<Shader>("src/shader/sphere.vert.glsl",
                                             "src/shader/sphere.tesc.glsl",
                                             "src/shader/sphere.tese.glsl",
                                             "src/shader/phong.frag.glsl");

    shapes.emplace_back(
            std::make_unique<Line>(
                    pLineShader.get(),
                    std::vector<Line::Vertex> {
                            {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                            {{3.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                            {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                            {{0.0f, 3.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                            {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                            {{0.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 1.0f}},
                    },
                    glm::mat4(1.0f)
            )
    );

    shapes.emplace_back(
            std::make_unique<Tetrahedron>(
                    pMeshShader.get(),
                    "var/tetrahedron.txt",
                    glm::translate(glm::mat4(1.0f), {-2.5f, 0.0f, 0.0f})
            )
    );

    shapes.emplace_back(
        std::make_unique<Tetrahedron>(
            pMeshShader.get(),
            "var/octahedron.txt",
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
        )
    );

    glm::vec3 BoxColor(0, 0, 1);
    shapes.emplace_back(
        std::make_unique<Mesh>(
            pMeshShader.get(),
            std::vector<Mesh::Vertex> {
        // Front face (two triangles)
                {{-0.5f, -0.5f, 0.5f}, { 0.0f,  0.0f,  1.0f }, {BoxColor }},
                { { 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },
                { { 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },

                { {-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },
                { { 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },
                { {-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },

                    // Back face (two triangles)
                { {-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                { { 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                { { 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },

                { {-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                { { 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                { {-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },

                    // Left face (two triangles)
                { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                { {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },

                { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                { {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },

                    // Right face (two triangles)
                { { 0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                { { 0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                { { 0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },

                { { 0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                { { 0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                { { 0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },

                    // Top face (two triangles)
                { {-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                { { 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                { { 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },

                { {-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                { { 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                { {-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },

                    // Bottom face (two triangles)
                { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },

                { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
    },

                    glm::rotate(
                            glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 0.0f}),
                            glm::radians(45.0f), {0.0f, 1.0f, 0.0f}
                    )
            )
    );



    initializeObjects2(); initializeObjects3(); initializeObjects4(); initializeObjects5(); initializeObjects6();
    initializeObjects7();
 }

void App::initializeObjects2()
{

    shapes_mode_2.emplace_back(
        std::make_unique<icosahedron>(
            pMeshShader.get(),
            "var/icosahedron.txt",
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
        )
    );
}

void App::initializeObjects3()
{
    shapes_mode_3.emplace_back(
        std::make_unique<icosahedron>(
            pMeshShader.get(),
            "var/icosahedron.txt",
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)),
            glm::vec3(2, 1.0f, 1.0f)
        )
   
 ); 
}

void App::initializeObjects4()
{
    pSphereShader->use();
    pSphereShader->setFloat("tessLevelOuter", 64);

    shapes_mode_4.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(-2.50f, 0.0f, 0.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::mat4(1.0f),
            1
        )
    );

    shapes_mode_4.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(0.0f, 0.0f, 0.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::mat4(1.0f)
        )
    );

    shapes_mode_4.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(2.50f, 0.0f, 0.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::mat4(1.0f),
            2
        )
    );
}

void App::initializeObjects5()
{
    pSphereShader->use();
    pSphereShader->setFloat("tessLevelOuter", 15);

    shapes_mode_5.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(2.50f, 0.0f, 0.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::mat4(1.0f),
            3
        )
    );
}

void App::initializeObjects6()
{
    pSphereShader->use();
    pSphereShader->setFloat("tessLevelOuter", 15);

    shapes_mode_6.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(2.50f, 0.0f, 0.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::mat4(1.0f),
            4
        )
    );

        shapes_mode_6.emplace_back(
            std::make_unique<docadehedron>(
                pMeshShader.get(),
                "var/dodecahedron.txt",
                glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f)),
                5
            )

        );
}

void App::initializeObjects7()
{
    //urban city
    glm::vec3 BoxColor(0, 0, 1);

    glm::vec3 positions[] =
    {
        { 2.5f, 0.0f, 0.0f },
        { -3.0f, -3.0f, 0.0f },
        { -3.50f, -0.61f, 0.0f},
    };


    glm::vec3 scales[] =
    {
        glm::vec3(2.0f, 7.0f, 3.0f),
        glm::vec3(2.0f, 2.0f, 3.0f),
        glm::vec3(0.250f, 5.0f, .50f),
    };

    for (int i = 0; i < 4; i++) {
        shapes_mode_7.emplace_back(
            std::make_unique<Mesh>(
                pMeshShader.get(),
                std::vector<Mesh::Vertex> {
            // Front face (two triangles)
                    {{-0.5f, -0.5f, 0.5f}, { 0.0f,  0.0f,  1.0f }, { BoxColor }},
                    { { 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },
                    { { 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },

                    { {-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },
                    { { 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },
                    { {-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {BoxColor } },

                        // Back face (two triangles)
                    { {-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                    { { 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                    { { 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },

                    { {-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                    { { 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },
                    { {-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {BoxColor } },

                        // Left face (two triangles)
                    { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                    { {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                    { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },

                    { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                    { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },
                    { {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {BoxColor} },

                        // Right face (two triangles)
                    { { 0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                    { { 0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                    { { 0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },

                    { { 0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                    { { 0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },
                    { { 0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {BoxColor } },

                        // Top face (two triangles)
                    { {-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                    { { 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                    { { 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },

                    { {-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                    { { 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },
                    { {-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {BoxColor } },

                        // Bottom face (two triangles)
                    { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                    { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                    { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },

                    { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                    { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
                    { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {BoxColor } },
        },

                glm::scale(glm::rotate(
                    glm::translate(glm::mat4(1.0f), positions[i]),
                    glm::radians(45.0f), { 0.0f, 1.0f, 0.0f }
                ), scales[i])));
    }

    //plant
    pSphereShader->use();
    pSphereShader->setFloat("tessLevelOuter", 64);

    shapes_mode_7.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(-3.50f, 0.0f, 0.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 7.0f, 3.0f)),
            4
        )
    );


    shapes_mode_7.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(0.50f, -0.00f, -5.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 5.0f, 3.0f)),
            1
        )
    );


    shapes_mode_7.emplace_back(
        std::make_unique<Sphere>(
            pSphereShader.get(),
            glm::vec3(-3.50f, -0.61f, -5.0f),
            1.0f,
            glm::vec3(1.0f, 0.5f, 0.31f),
            glm::scale(glm::mat4(1.0f), glm::vec3(2.50f)),
            0
        )
    );


    shapes_mode_7.emplace_back(
        std::make_unique<docadehedron>(
            pMeshShader.get(),
            "var/dodecahedron.txt",
            glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, -3.0f, -0.0f)),
            5
        )

    );

    //camera setups
    float angle = 0.0f;

     HorizontalCamera = new KeyFrameCamera(glm::vec3(0.0f, 5.0f, 10.0f), glm::quat());

    glm::quat r = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(0, 1, 0)));
    glm::quat r2 = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(40.0f), glm::vec3(0, 1, 0)));
    glm::quat r3 = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(130.0f), glm::vec3(0, 1, 0)));
    glm::quat r4 = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)));



    // Now push back frames with these quaternions
    HorizontalCamera->pushBackFrame(glm::vec3(-20, 5, 15), r, 4);
    HorizontalCamera->pushBackFrame(glm::vec3(20, 5, 20), r2, 7);
    HorizontalCamera->pushBackFrame(glm::vec3(20, 5, -40), r3, 10);
    HorizontalCamera->pushBackFrame(glm::vec3(0, 5, -40), r4, 10);


    //vertical camera setup
    VerticalCamera = new KeyFrameCamera(glm::vec3(0.0f, 1.0f, 20.0f), glm::quat());

    glm::quat Vr = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f),  glm::vec3(1, 0, 0)));
    glm::quat Vr2 = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(-60.0f),  glm::vec3(1, 0, 0)));
    glm::quat Vr3 = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(-60.0f), glm::vec3(1, 0, 0)));
    glm::quat Vr4 = glm::quat_cast(glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0,1, 0)));



    // Now push back frames with these quaternions
    VerticalCamera->pushBackFrame(glm::vec3(-10, 15, 15), Vr, 4);
    VerticalCamera->pushBackFrame(glm::vec3(-10, 20, -5),  Vr2, 4);
    VerticalCamera->pushBackFrame(glm::vec3(0, 5, -40), r4, 15);
   // VerticalCamera->pushBackFrame(glm::vec3(-10, 20, -45), Vr4, 5);


}


void App::render()
{
    auto t = static_cast<float>(timeElapsedSinceLastFrame);

    // Update shader uniforms.
    view = camera.getViewMatrix();
    projection = glm::perspective(glm::radians(camera.zoom),
                                  static_cast<GLfloat>(kWindowWidth) / static_cast<GLfloat>(kWindowHeight),
                                  0.01f,
                                  100.0f);

    pLineShader->use();
    if (UseFreeCamera)
    {
        pLineShader->setMat4("view", view);
    }
    else {
        pLineShader->setMat4("view",(HorizontalCamer ? HorizontalCamera->GetView() : VerticalCamera->GetView()  ));
    }
    pLineShader->setMat4("projection", projection);

    pMeshShader->use();
    if (UseFreeCamera)
    {
        pMeshShader->setMat4("view", view);
    }
    else {
        pMeshShader->setMat4("view", (HorizontalCamer ? HorizontalCamera->GetView() : VerticalCamera->GetView()));
    }

    pMeshShader->setMat4("projection", projection);
    pMeshShader->setVec3("ViewPos", camera.position);
    pMeshShader->setVec3("lightPos", lightPos);
    pMeshShader->setVec3("lightColor", lightColor);

    pSphereShader->use();
    if (UseFreeCamera)
    {
        pSphereShader->setMat4("view", view);
    }
    else {
        pSphereShader->setMat4("view", (HorizontalCamer ? HorizontalCamera->GetView() : VerticalCamera->GetView()));
    }

    pSphereShader->setMat4("projection", projection);
    pSphereShader->setVec3("ViewPos", camera.position);
    pSphereShader->setVec3("lightPos", lightPos);
    pSphereShader->setVec3("lightColor", lightColor);

    // Render.
    if(RenderingMode == 1)
    {
        for (auto & s : shapes)
        {
            s->render(t);
        }
    }

    else if (RenderingMode == 2)
    {
        for (auto& s : shapes_mode_2)
        {
            s->render(t);
        }
    }

    else if (RenderingMode == 3)
    {
        for (auto& s : shapes_mode_3)
        {
            s->render(t);
        }
    }

    else if (RenderingMode == 4)
    {
        //reset Tesselation shader granularity for these shapes that dont use subdivision
        pSphereShader->use();
        pSphereShader->setFloat("tessLevelOuter", 64);

        for (auto& s : shapes_mode_4)
        {
            s->render(t);
        }
    }

    else if (RenderingMode == 5)
    {
        for (auto& s : shapes_mode_5)
        {
            s->render(t);
        }
    }

    else if (RenderingMode == 6)
    {
        for (auto& s : shapes_mode_6)
        {
            s->render(t);
        }
    }


    else if (RenderingMode == 7)
    {
        pSphereShader->setVec3("lightPos", (HorizontalCamer ? HorizontalCamera->GetlerpedPosition() : VerticalCamera->GetlerpedPosition()));


        for (auto& s : shapes_mode_7)
        {
            s->render(t);
        }
    }
}
