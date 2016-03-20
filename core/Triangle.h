#pragma once

#include "Matrix.h"
#include "Ray.h"

namespace core {

class Triangle {
public:
    static auto IntersectRay(Ray ray, std::array<Point4f, 3> const& triangle) -> Float32 {
        auto const& p0 = triangle[0];
        auto const& p1 = triangle[1];
        auto const& p2 = triangle[2];
        auto e1 = static_cast<Vector4f>(p1 - p0);
        auto e2 = static_cast<Vector4f>(p2 - p0);
        auto q = static_cast<Vector4f>(CrossProduct(ray.direction, e2));
        auto a = DotProduct(e1, q);
        if (abs(a) < std::numeric_limits<Float32>::min()) {
            return -1.0;
        }
        auto f = 1.0 / a;
        auto s = static_cast<Vector4f>(ray.origin - p0);
        auto u = f * DotProduct(s, q);
        if (u < 0) {
            return -1.0;
        }
        auto r = static_cast<Vector4f>(CrossProduct(s, e1));
        auto v = f * DotProduct(ray.direction, r);
        if (v < 0 || u + v > 1.0) {
            return -1.0;
        }
        return f * DotProduct(e2, r);
    }
};

}