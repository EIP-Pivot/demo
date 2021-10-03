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
// #include "Scene.hxx"
#include "Systems/RenderableSystem.hxx"
#include <pivot/ecs/Core/Scene.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>


Logger *logger = nullptr;

class Application : public VulkanApplication
{
public:
    Application(): VulkanApplication(){};

    void scene1Init()
    {
        _scene1 = gSceneManager.registerLevel();
        gSceneManager.setCurrentLevelId(_scene1);
        gSceneManager.getCurrentLevel().Init();
        gSceneManager.getCurrentLevel().RegisterComponent<Gravity>();
        gSceneManager.getCurrentLevel().RegisterComponent<RigidBody>();
        gSceneManager.getCurrentLevel().RegisterComponent<Transform>();
        gSceneManager.getCurrentLevel().RegisterComponent<Renderable>();
        gSceneManager.getCurrentLevel().RegisterComponent<Camera>();

        gSceneManager.getCurrentLevel().RegisterSystem<PhysicsSystem>();
        {
            Signature signature;
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Gravity>());
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<RigidBody>());
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Transform>());
            gSceneManager.getCurrentLevel().SetSystemSignature<PhysicsSystem>(signature);
        }

        gSceneManager.getCurrentLevel().RegisterSystem<RenderableSystem>();
        {
            Signature signature;
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Renderable>());
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Transform>());
            gSceneManager.getCurrentLevel().SetSystemSignature<RenderableSystem>(signature);
        }

        auto system = gSceneManager.getCurrentLevel().RegisterSystem<ControlSystem>();
        {
            Signature signature;
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Camera>());
            gSceneManager.getCurrentLevel().SetSystemSignature<ControlSystem>(signature);
        }

        system->Init();

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

        std::vector<Entity> entities(3000);

        for (auto &_entity: entities) {
            auto entity = gSceneManager.getCurrentLevel().CreateEntity();

            gSceneManager.getCurrentLevel().AddComponent<Gravity>(
                entity, {
                            .force = glm::vec3(0.0f, randGravity(generator), 0.0f),
                        });

            gSceneManager.getCurrentLevel().AddComponent<RigidBody>(
                entity, {
                            .velocity = glm::vec3(randVelocityXZ(generator), randVelocityY(generator),
                                                  randVelocityXZ(generator)),
                            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
                        });

            gSceneManager.getCurrentLevel().AddComponent<Transform>(
                entity,
                {
                    .position =
                        glm::vec3(randPositionXZ(generator), randPositionY(generator), randPositionXZ(generator)),
                    .rotation = glm::vec3(randRotation(generator), randRotation(generator), randRotation(generator)),
                    .scale = glm::vec3(randScale(generator)),
                });

            gSceneManager.getCurrentLevel().AddComponent<Renderable>(entity, {.meshID = "cube", .textureIndex = 1});

            gSceneManager.getCurrentLevel().obj.push_back({
                .meshID = "cube",
                .objectInformation =
                    {
                        .transform =
                            {
                                .translation = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity).position,
                                .rotation = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity).rotation,
                                .scale = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity).scale,
                            },
                        .textureIndex = textures[randTexture(generator)],
                        .materialIndex = "white",
                    },
            });
        }

        Entity camera = gSceneManager.getCurrentLevel().CreateEntity();
        gSceneManager.getCurrentLevel().AddComponent<Camera>(camera, Camera(glm::vec3(0, 200, 500)));
        gSceneManager.getCurrentLevel().AddComponent<Transform>(camera, {
                                                                            .position = glm::vec3(0.0f, 0.0f, 0.0f),
                                                                            .rotation = glm::vec3(0, 0, 0),
                                                                            .scale = glm::vec3(1.0f),
                                                                        });
        gSceneManager.getCurrentLevel().addCamera(camera);

        gSceneManager.getCurrentLevel().obj.push_back({
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
    }

    void scene2Init()
    {
        _scene2 = gSceneManager.registerLevel();
        gSceneManager.setCurrentLevelId(_scene2);
        gSceneManager.getCurrentLevel().Init();
        gSceneManager.getCurrentLevel().RegisterComponent<Gravity>();
        gSceneManager.getCurrentLevel().RegisterComponent<RigidBody>();
        gSceneManager.getCurrentLevel().RegisterComponent<Transform>();
        gSceneManager.getCurrentLevel().RegisterComponent<Renderable>();
        gSceneManager.getCurrentLevel().RegisterComponent<Camera>();

        gSceneManager.getCurrentLevel().RegisterSystem<RenderableSystem>();
        {
            Signature signature;
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Renderable>());
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Transform>());
            gSceneManager.getCurrentLevel().SetSystemSignature<RenderableSystem>(signature);
        }

        auto system = gSceneManager.getCurrentLevel().RegisterSystem<ControlSystem>();
        {
            Signature signature;
            signature.set(gSceneManager.getCurrentLevel().GetComponentType<Camera>());
            gSceneManager.getCurrentLevel().SetSystemSignature<ControlSystem>(signature);
        }

        system->Init();

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

        std::vector<Entity> entities(5);

        for (auto &_entity: entities) {
            auto entity = gSceneManager.getCurrentLevel().CreateEntity();

            gSceneManager.getCurrentLevel().AddComponent<Gravity>(
                entity, {
                            .force = glm::vec3(0.0f, randGravity(generator), 0.0f),
                        });

            gSceneManager.getCurrentLevel().AddComponent<RigidBody>(
                entity, {
                            .velocity = glm::vec3(randVelocityXZ(generator), randVelocityY(generator),
                                                  randVelocityXZ(generator)),
                            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
                        });

            gSceneManager.getCurrentLevel().AddComponent<Transform>(
                entity,
                {
                    .position =
                        glm::vec3(randPositionXZ(generator), randPositionY(generator), randPositionXZ(generator)),
                    .rotation = glm::vec3(randRotation(generator), randRotation(generator), randRotation(generator)),
                    .scale = glm::vec3(randScale(generator)),
                });

            gSceneManager.getCurrentLevel().AddComponent<Renderable>(entity, {.meshID = "cube", .textureIndex = 1});

            gSceneManager.getCurrentLevel().obj.push_back({
                .meshID = "cube",
                .objectInformation =
                    {
                        .transform =
                            {
                                .translation = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity).position,
                                .rotation = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity).rotation,
                                .scale = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity).scale,
                            },
                        .textureIndex = textures[randTexture(generator)],
                        .materialIndex = "white",
                    },
            });
        }

        Entity camera = gSceneManager.getCurrentLevel().CreateEntity();
        gSceneManager.getCurrentLevel().AddComponent<Camera>(camera, Camera(glm::vec3(0, 200, 500)));
        gSceneManager.getCurrentLevel().AddComponent<Transform>(camera, {
                                                                            .position = glm::vec3(0.0f, 0.0f, 0.0f),
                                                                            .rotation = glm::vec3(0, 0, 0),
                                                                            .scale = glm::vec3(1.0f),
                                                                        });
        gSceneManager.getCurrentLevel().addCamera(camera);

        Entity camera2 = gSceneManager.getCurrentLevel().CreateEntity();
        gSceneManager.getCurrentLevel().AddComponent<Camera>(camera2, Camera(glm::vec3(0, 200, 500)));
        gSceneManager.getCurrentLevel().AddComponent<Transform>(camera2, {
                                                                             .position = glm::vec3(0.0f, 0.0f, 0.0f),
                                                                             .rotation = glm::vec3(0, 0, 0),
                                                                             .scale = glm::vec3(1.0f),
                                                                         });
        gSceneManager.getCurrentLevel().addCamera(camera2);

        gSceneManager.getCurrentLevel().obj.push_back({
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
    }

    void init()
    {
        gSceneManager.Init();
        scene1Init();
        scene2Init();
        gSceneManager.setCurrentLevelId(_scene2);
        window.captureCursor(true);
        window.setKeyEventCallback(Window::Key::LEFT_ALT,
                                   [&](Window &window, const Window::Key key, const Window::KeyAction action) {
                                       if (action == Window::KeyAction::Release) {
                                           window.captureCursor(!window.captureCursor());
                                           bFirstMouse = window.captureCursor();
                                       }
                                   });
        window.setKeyEventCallback(Window::Key::LEFT_CTRL,
                                   [&](Window &window, const Window::Key key, const Window::KeyAction action) {
                                       if (action == Window::KeyAction::Release) {
                                           if (gSceneManager.getCurrentLevelId() == _scene1)
                                               gSceneManager.setCurrentLevelId(_scene2);
                                           else
                                               gSceneManager.setCurrentLevelId(_scene1);
                                       }
                                   });
        window.setKeyEventCallback(Window::Key::V,
                                   [&](Window &window, const Window::Key key, const Window::KeyAction action) {
                                       if (action == Window::KeyAction::Release) {
                                           gSceneManager.getCurrentLevel().switchCamera();
                                       }
                                   });
        auto key_lambda = [&](Window &window, const Window::Key key, const Window::KeyAction action) {
            switch (action) {
                case Window::KeyAction::Pressed: {
                    button.set(static_cast<std::size_t>(key));
                    Event event(Events::Window::INPUT);
                    event.SetParam(Events::Window::Input::INPUT, button);
                    gSceneManager.getCurrentLevel().SendEvent(event);
                } break;
                case Window::KeyAction::Release: {
                    button.reset(static_cast<std::size_t>(key));
                    Event event(Events::Window::INPUT);
                    event.SetParam(Events::Window::Input::INPUT, button);
                    gSceneManager.getCurrentLevel().SendEvent(event);
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
            ControlSystem::processMouseMovement(gSceneManager.getCurrentLevel().getCamera(),
                                                glm::dvec2(xoffset, yoffset));
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
            gSceneManager.getCurrentLevel().Update(dt);
            draw(gSceneManager.getCurrentLevel(), gSceneManager.getCurrentLevel().getCamera());

            auto stopTime = std::chrono::high_resolution_clock::now();
            dt = std::chrono::duration<float>(stopTime - startTime).count();
        }
    }

public:
    LevelId _scene1;
    LevelId _scene2;
    glm::dvec2 last;

    bool bFirstMouse = true;
    std::bitset<UINT16_MAX> button;
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
