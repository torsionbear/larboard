#pragma once

namespace core {

class TextureUsage {
public:
    enum TextureType : int {
        DiffuseMap = 0,
        NormalMap,
        SpecularMap,
        EmissiveMap,
        ParallaxMap,
        HeightMap,
        CubeMap,
        DiffuseTextureArray,
    };

};
}