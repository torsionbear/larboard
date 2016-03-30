#pragma once

#include "Matrix.h"

namespace core {

struct Vertex {
    Vector3f coord;
    Vector3f normal;
    Vector2f texCoord;
};

struct VertexC3 {
    Vector3f coord;
};

}