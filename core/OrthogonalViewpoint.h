#pragma once

#include "Movable.h"
#include "Aabb.h"

namespace core {

class OrthogonalViewpoint : public Movable {
public:
    auto SetViewVolume(Aabb const& aabb) -> void;
    auto GetProjectTransformDx() const->Matrix4x4f const& {
        return _projectTransformDx;
    }
protected:
    Matrix4x4f _projectTransformDx;
};

}