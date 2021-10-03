#include "Systems/RenderableSystem.hxx"

#include "pivot/ecs/Components/Transform.hxx"
#include <pivot/ecs/Core/SceneManager.hxx>

#include "iostream"

extern SceneManager gSceneManager;

void RenderableSystem::Init() {}

void RenderableSystem::Update(float dt)
{
    for (auto const &entity: mEntities) {
        auto &transform = gSceneManager.getCurrentLevel().GetComponent<Transform>(entity);
        gSceneManager.getCurrentLevel().obj[entity].objectInformation.transform.translation = transform.position;
    }
}