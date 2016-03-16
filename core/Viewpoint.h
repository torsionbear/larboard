#pragma once

#include "Movable.h"

namespace core {

class Viewpoint : public Movable {
public:
    Viewpoint()
        : Movable(Vector4f{ 0.0f, 0.0f, -1.0f, 0.0f }, Vector4f{ 0.0f, 1.0f, 0.0f, 0.0f }) { 	// viewpoint looks at -z by default
    }
    virtual ~Viewpoint() = default;
public:
    virtual auto GetProjectTransform() const -> Matrix4x4f const& = 0;
    virtual auto GetProjectTransformDx() const -> Matrix4x4f const& = 0;
    auto GetRenderDataId() const -> unsigned int {
        return _renderDataId;
    }
    auto SetRenderDataId(unsigned int id) -> void {
        _renderDataId = id;
    }
protected:
    unsigned int _renderDataId;
};

}
