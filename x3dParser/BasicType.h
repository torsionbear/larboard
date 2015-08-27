#pragma once

#include <string>

namespace x3dParser {

using Float = float;
    
template <typename T>
inline bool equal(T v1, T v2) {
    return v1 == v2;
}

template <>
inline bool equal<Float>(Float v1, Float v2) {
    return abs(v1 - v2) < std::numeric_limits<Float>::epsilon();
}

struct Float2 {
    Float2() = default;
    Float2(Float, Float);
    explicit Float2(std::string&&);
    auto operator==(const Float2&) const -> bool;

    Float x = 0.0f;
    Float y = 0.0f;
};

struct Float3 {
    Float3() = default;
    Float3(Float, Float, Float);
    explicit Float3(std::string&&);
    auto operator==(const Float3&) const -> bool;

    Float x = 0.0f;
    Float y = 0.0f;
    Float z = 0.0f;
};

struct Float4 {
    Float4() = default;
    Float4(Float, Float, Float, Float);
    explicit Float4(std::string&&);
    auto operator==(const Float4&) const -> bool;

    Float x = 0.0f;
    Float y = 0.0f;
    Float z = 0.0f;
    Float a = 0.0f;
};

using ULong = unsigned long;

struct ULong3 {
    ULong3() = default;
    ULong3(ULong, ULong, ULong);
    auto operator==(const ULong3&) const -> bool;

    ULong a = 0u;
    ULong b = 0u;
    ULong c = 0u;
};

}
