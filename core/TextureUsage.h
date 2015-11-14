#pragma once

namespace core {

class TextureUsage {
public:
    enum TextureType : int {
        DiffuseMap = 0,
        SpecularMap = 1,
        NormalMap = 2,
        ParallaxMap = 3,
        HeightMap = 4,
        CubeMap = 5,
        DiffuseTextureArray = 6,
    };

};
}