#include <chrono>
#include <iostream>
#include <random>

#include <pivot/graphics/Camera.hxx>
#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/Window.hxx>
#include <pivot/graphics/vk_utils.hxx>

#include <pivot/ecs/ecs.hxx>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/Transform.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Systems/PhysicsSystem.hxx>

#include <Logger.hpp>

#include "Scene.hxx"
#include "Components/Renderable.hxx"
#include "Systems/RenderableSystem.hxx"

Logger *logger = nullptr;

class Application : public VulkanApplication
{
public:
    Application(): VulkanApplication(), camera(glm::vec3(0, 200, 500)){};

    void init()
    {
        gCoordinator.Init();
        gCoordinator.RegisterComponent<Gravity>();
        gCoordinator.RegisterComponent<RigidBody>();
        gCoordinator.RegisterComponent<Transform>();
        gCoordinator.RegisterComponent<Renderable>();

        physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
        {
            Signature signature;
            signature.set(gCoordinator.GetComponentType<Gravity>());
            signature.set(gCoordinator.GetComponentType<RigidBody>());
            signature.set(gCoordinator.GetComponentType<Transform>());
            gCoordinator.SetSystemSignature<PhysicsSystem>(signature);
        }

        renderableSystem = gCoordinator.RegisterSystem<RenderableSystem>();
        {
            Signature signature;
            signature.set(gCoordinator.GetComponentType<Renderable>());
            signature.set(gCoordinator.GetComponentType<Transform>());
            gCoordinator.SetSystemSignature<RenderableSystem>(signature);
        }

        std::default_random_engine generator;
        std::uniform_real_distribution<float> randPositionY(0.0f, 50.0f);
        std::uniform_real_distribution<float> randPositionXZ(-50.0f, 50.0f);
        std::uniform_real_distribution<float> randRotation(0.0f, 3.0f);
        std::uniform_real_distribution<float> randColor(0.0f, 1.0f);
        std::uniform_real_distribution<float> randGravity(-10.0f, -1.0f);
        std::uniform_real_distribution<float> randVelocityY(10.0f, 200.0f);
        std::uniform_real_distribution<float> randVelocityXZ(-200.0f, 200.0f);
        std::uniform_int_distribution<int> randTexture(0, 7);

        std::vector<Entity> entities(MAX_OBJECT - 1);

        for (auto &_entity: entities) {
            auto entity = gCoordinator.CreateEntity();

            gCoordinator.AddComponent<Gravity>(entity, {
                .force = glm::vec3(0.0f, randGravity(generator), 0.0f)
            });

            gCoordinator.AddComponent<RigidBody>(entity, {
                    .velocity = glm::vec3(randVelocityXZ(generator), randVelocityY(generator), randVelocityXZ(generator)), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)
            });

            gCoordinator.AddComponent<Transform>(
                entity,
                {.position = glm::vec3(randPositionXZ(generator), randPositionY(generator), randPositionXZ(generator)),
                 .rotation = glm::vec3(randRotation(generator), randRotation(generator), randRotation(generator)),
                 .scale = glm::vec3(0.5f)});
            gCoordinator.AddComponent<Renderable>(entity, {.meshID = "cube", .textureIndex = 1});

            scene.obj.push_back({
                .meshID = "cube",
                .objectInformation =
                    {
                        .transform =
                            {
                                .translation = gCoordinator.GetComponent<Transform>(entity).position,
                                .rotation = gCoordinator.GetComponent<Transform>(entity).rotation,
                                .scale = gCoordinator.GetComponent<Transform>(entity).scale,
                            },
                        .textureIndex = (uint32_t)randTexture(generator),
                    },
            });
        }

        window.setUserPointer(this);
        window.captureCursor(true);
        window.setCursorPosCallback(Application::cursor_callback);
        window.setKeyCallback(Application::keyboard_callback);
        load3DModels({"../assets/cube.obj"});
        loadTexturess({"../assets/rouge.png",
                       "../assets/vert.png",
                       "../assets/bleu.png",
                       "../assets/cyan.png",
                       "../assets/orange.png",
                       "../assets/jaune.png",
                       "../assets/blanc.png",
                       "../assets/violet.png"
        });
    }

    void run()
    {
        float dt = 0.0f;
        this->VulkanApplication::init();
        while (!window.shouldClose()) {
            auto startTime = std::chrono::high_resolution_clock::now();

            window.pollEvent();
            if (!bInteractWithUi) {
                if (window.isKeyPressed(GLFW_KEY_W)) camera.processKeyboard(Camera::FORWARD);
                if (window.isKeyPressed(GLFW_KEY_S)) camera.processKeyboard(Camera::BACKWARD);
                if (window.isKeyPressed(GLFW_KEY_D)) camera.processKeyboard(Camera::RIGHT);
                if (window.isKeyPressed(GLFW_KEY_A)) camera.processKeyboard(Camera::LEFT);
                if (window.isKeyPressed(GLFW_KEY_SPACE)) camera.processKeyboard(Camera::UP);
                if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) camera.processKeyboard(Camera::DOWN);
            }

            physicsSystem->Update(dt);
            renderableSystem->Update(scene.obj);
            draw(scene, camera, 0);

            auto stopTime = std::chrono::high_resolution_clock::now();
            dt = std::chrono::duration<float>(stopTime - startTime).count();
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
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<RenderableSystem> renderableSystem;
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
