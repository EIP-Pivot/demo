#include "Systems/RenderableSystem.hxx"

#include "pivot/ecs/Components/Transform.hxx"
#include "pivot/ecs/Core/Coordinator.hxx"

#include "iostream"

extern Coordinator gCoordinator;

void RenderableSystem::Init() {}

void RenderableSystem::Update(std::vector<RenderObject> &obj)
{
    for (auto const &entity: mEntities) {
        auto &transform = gCoordinator.GetComponent<Transform>(entity);
        obj[entity].objectInformation.transform.translation = transform.position;
    }
}