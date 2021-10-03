#include <chrono>
#include <iostream>
#include <random>

#include <pivot/graphics/VulkanApplication.hxx>
#include <pivot/graphics/Window.hxx>
#include <pivot/graphics/vk_utils.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Components/Transform.hxx>
#include <pivot/ecs/Core/Event.hxx>
#include <pivot/ecs/Systems/ControlSystem.hxx>
#include <pivot/ecs/Systems/PhysicsSystem.hxx>
#include <pivot/ecs/ecs.hxx>

#include <Logger.hpp>

#include "Components/Renderable.hxx"
#include "Scene.hxx"
#include "Systems/RenderableSystem.hxx"

Logger *logger = nullptr;

class Application : public VulkanApplication
{
public:
    Application(): VulkanApplication(){};

    void init()
    {
        gCoordinator.Init();
        gCoordinator.RegisterComponent<Gravity>();
        gCoordinator.RegisterComponent<RigidBody>();
        gCoordinator.RegisterComponent<Transform>();
        gCoordinator.RegisterComponent<Renderable>();
        gCoordinator.RegisterComponent<Camera>();

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

        controlSystem = gCoordinator.RegisterSystem<ControlSystem>();
        {
            Signature signature;
            signature.set(gCoordinator.GetComponentType<Camera>());
            gCoordinator.SetSystemSignature<ControlSystem>(signature);
        }

        controlSystem->Init();

        std::array<std::string, 8> textures = {"rouge", "vert", "bleu", "cyan", "orange", "jaune", "blanc", "violet"};
        std::default_random_engine generator;
        std::uniform_real_distribution<float> randPositionY(0.0f, 50.0f);
        std::uniform_real_distribution<float> randPositionXZ(-50.0f, 50.0f);
        std::uniform_real_distribution<float> randRotation(0.0f, 3.0f);
        std::uniform_real_distribution<float> randColor(0.0f, 1.0f);
        std::uniform_real_distribution<float> randGravity(-10.0f, -1.0f);
        std::uniform_real_distribution<float> randVelocityY(10.0f, 200.0f);
        std::uniform_real_distribution<float> randVelocityXZ(-200.0f, 200.0f);
        std::uniform_real_distribution<float> randScale(0.5f, 1.0f);
        std::uniform_int_distribution<int> randTexture(0, textures.size() - 1);

        std::vector<Entity> entities(MAX_OBJECT - 2);

        for (auto &_entity: entities) {
            auto entity = gCoordinator.CreateEntity();

            gCoordinator.AddComponent<Gravity>(entity, {
                                                           .force = glm::vec3(0.0f, randGravity(generator), 0.0f),
                                                       });

            gCoordinator.AddComponent<RigidBody>(
                entity, {
                            .velocity = glm::vec3(randVelocityXZ(generator), randVelocityY(generator),
                                                  randVelocityXZ(generator)),
                            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
                        });

            gCoordinator.AddComponent<Transform>(
                entity,
                {
                    .position =
                        glm::vec3(randPositionXZ(generator), randPositionY(generator), randPositionXZ(generator)),
                    .rotation = glm::vec3(randRotation(generator), randRotation(generator), randRotation(generator)),
                    .scale = glm::vec3(randScale(generator)),
                });

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
                        .textureIndex = textures[randTexture(generator)],
                        .materialIndex = "white",
                    },
            });
        }

        camera = gCoordinator.CreateEntity();
        gCoordinator.AddComponent<Camera>(camera, Camera(glm::vec3(0, 200, 500)));
        gCoordinator.AddComponent<Transform>(camera, {
                                                         .position = glm::vec3(0.0f, 0.0f, 0.0f),
                                                         .rotation = glm::vec3(0, 0, 0),
                                                         .scale = glm::vec3(1.0f),
                                                     });

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
                    .textureIndex = "blanc",
                    .materialIndex = "white",
                },
        });
        window.captureCursor(true);
        window.setKeyEventCallback(Window::Key::LEFT_ALT,
                                   [&](Window &window, const Window::Key key, const Window::KeyAction action) {
                                       if (action == Window::KeyAction::Release) {
                                           window.captureCursor(!window.captureCursor());
                                           bFirstMouse = window.captureCursor();
                                       }
                                   });
        auto key_lambda = [&](Window &window, const Window::Key key, const Window::KeyAction action) {
            switch (action) {
                case Window::KeyAction::Pressed: {
                    button.set(static_cast<std::size_t>(key));
                    Event event(Events::Window::INPUT);
                    event.SetParam(Events::Window::Input::INPUT, button);
                    gCoordinator.SendEvent(event);
                } break;
                case Window::KeyAction::Release: {
                    button.reset(static_cast<std::size_t>(key));
                    Event event(Events::Window::INPUT);
                    event.SetParam(Events::Window::Input::INPUT, button);
                    gCoordinator.SendEvent(event);
                } break;
                default: break;
            }
        };
        window.setKeyEventCallback(Window::Key::W, key_lambda);
        window.setKeyEventCallback(Window::Key::S, key_lambda);
        window.setKeyEventCallback(Window::Key::D, key_lambda);
        window.setKeyEventCallback(Window::Key::A, key_lambda);
        window.setKeyEventCallback(Window::Key::SPACE, key_lambda);
        window.setKeyEventCallback(Window::Key::LEFT_SHIFT, key_lambda);
        window.setMouseMovementCallback([&](Window &window, const glm::dvec2 pos) {
            if (!window.captureCursor()) return;

            if (bFirstMouse) {
                last = pos;
                bFirstMouse = false;
            }
            auto xoffset = pos.x - last.x;
            auto yoffset = last.y - pos.y;

            last = pos;
            Event event(Events::Window::MOUSE);
            event.SetParam(Events::Window::Mouse::MOUSE, glm::dvec2(xoffset, yoffset));
            gCoordinator.SendEvent(event);
        });
        load3DModels({"../assets/plane.obj", "../assets/cube.obj"});
        loadTextures({"../assets/rouge.png", "../assets/vert.png", "../assets/bleu.png", "../assets/cyan.png",
                      "../assets/orange.png", "../assets/jaune.png", "../assets/blanc.png", "../assets/violet.png"});
    }

    void run()
    {
        float dt = 0.0f;
        this->VulkanApplication::init();
        while (!window.shouldClose()) {
            auto startTime = std::chrono::high_resolution_clock::now();

            window.pollEvent();
            physicsSystem->Update(dt);
            controlSystem->Update(dt);
            renderableSystem->Update(scene.obj);
            draw(scene, gCoordinator.GetComponent<Camera>(camera));

            auto stopTime = std::chrono::high_resolution_clock::now();
            dt = std::chrono::duration<float>(stopTime - startTime).count();
        }
    }

public:
    glm::dvec2 last;

    bool bFirstMouse = true;
    std::bitset<UINT16_MAX> button;
    std::shared_ptr<PhysicsSystem> physicsSystem;
    std::shared_ptr<RenderableSystem> renderableSystem;
    std::shared_ptr<ControlSystem> controlSystem;

    Scene scene;
    Entity camera;
};

int main()
try {
    logger = new Logger(std::cout);
    logger->start();

    Application app;
    app.init();
    app.run();
    return 0;
} catch (std::exception &e) {
    logger->err("THROW") << e.what();
    LOGGER_ENDL;
    return 1;
}
