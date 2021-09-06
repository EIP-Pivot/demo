#pragma once

#include <pivot/graphics/interface/I3DScene.hxx>

class Scene : public I3DScene
{
public:
    Scene() = default;
    virtual std::vector<RenderObject> getSceneInformations() const final { return obj; }

public:
    std::vector<RenderObject> obj;
};