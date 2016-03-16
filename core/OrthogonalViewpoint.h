#pragma once

#include "Viewpoint.h"
#include "Movable.h"
#include "Aabb.h"

namespace core {

class OrthogonalViewpoint : public Viewpoint {
public:
    auto GetProjectTransform() const->Matrix4x4f const& override {
        return _projectTransform;
    }
    auto GetProjectTransformDx() const->Matrix4x4f const& override {
        return _projectTransformDx;
    }
public:
    auto SetViewVolume(Aabb const& aabb) -> void;
protected:
    Matrix4x4f _projectTransform;
    Matrix4x4f _projectTransformDx;
};

}