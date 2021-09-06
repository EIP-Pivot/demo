#include <iostream>
#include <pivot/graphics/Camera.hxx>
#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/Window.hxx>
#include <pivot/graphics/vk_utils.hxx>

#include <Logger.hpp>

#include "Scene.hxx"

Logger *logger = nullptr;

class Application : public VulkanApplication
{
public:
    Application(): VulkanApplication(), camera(glm::vec3(0, 10, 0)){};

    void init()
    {
        window.setUserPointer(this);
        window.captureCursor(true);
        window.setCursorPosCallback(Application::cursor_callback);
        window.setKeyCallback(Application::keyboard_callback);
        scene.obj.push_back({
            .meshID = "plane",
            .objectInformation =
                {
                    .transform =
                        {
                            .translation = glm::vec3(0.0f, 0.0f, 0.0f),
                            .rotation = glm::vec3(0, 0, 0),
                            .scale = glm::vec3(1.0f),
                        },
                    .textureIndex = 0,
                },
        });
        load3DModels({"../assets/plane.obj"});
        loadTexturess({"../assets/greystone.png"});
    }

    void run()
    {
        this->VulkanApplication::init();
        while (!window.shouldClose()) {
            window.pollEvent();
            if (!bInteractWithUi) {
                if (window.isKeyPressed(GLFW_KEY_W)) camera.processKeyboard(Camera::FORWARD);
                if (window.isKeyPressed(GLFW_KEY_S)) camera.processKeyboard(Camera::BACKWARD);
                if (window.isKeyPressed(GLFW_KEY_D)) camera.processKeyboard(Camera::RIGHT);
                if (window.isKeyPressed(GLFW_KEY_A)) camera.processKeyboard(Camera::LEFT);
                if (window.isKeyPressed(GLFW_KEY_SPACE)) camera.processKeyboard(Camera::UP);
                if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) camera.processKeyboard(Camera::DOWN);
            }
            draw(scene, camera, 0);
        }
    }

    static void keyboard_callback(GLFWwindow *win, int key, int, int action, int)
    {
        auto *eng = (Application *)glfwGetWindowUserPointer(win);

        switch (action) {
            case GLFW_PRESS: {
                switch (key) {
                    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(win, true); break;
                    case GLFW_KEY_LEFT_ALT: {
                        if (eng->bInteractWithUi) {
                            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            eng->bInteractWithUi = false;
                            eng->bFirstMouse = true;
                        } else {
                            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                            eng->bInteractWithUi = true;
                        }
                    } break;
                    default: break;
                }
            } break;
            default: break;
        }
    }

    static void cursor_callback(GLFWwindow *win, double xpos, double ypos)
    {
        auto *eng = (Application *)glfwGetWindowUserPointer(win);
        if (eng->bInteractWithUi) return;

        if (eng->bFirstMouse) {
            eng->lastX = xpos;
            eng->lastY = ypos;
            eng->bFirstMouse = false;
        }
        auto xoffset = xpos - eng->lastX;
        auto yoffset = eng->lastY - ypos;

        eng->lastX = xpos;
        eng->lastY = ypos;
        eng->camera.processMouseMovement(xoffset, yoffset);
    }

public:
    float lastX;
    float lastY;

    bool bInteractWithUi = false;
    bool bFirstMouse = true;
    Scene scene;
    Camera camera;
};

int main()
{
    logger = new Logger(std::cout);
    logger->start();

    Application app;
    app.init();
    app.run();
    return 0;
}
