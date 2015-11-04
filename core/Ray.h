#pragma once

#include "Matrix.h"

namespace core {

struct Ray {
public:
    Point4f origin;
    Vector4f direction;
    Float32 length;
public:
    auto GetHead() const -> Point4f {
        return origin + direction * length;
    }
};

}