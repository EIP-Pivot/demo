#pragma once

#include "pivot/graphics/types/RenderObject.hxx"
#include <pivot/ecs/Core/System.hxx>
#include <vector>

class RenderableSystem : public System
{
public:
    void Init();

    void Update(float dt);
};