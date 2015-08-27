#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class Material : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetDiffuseColor() const -> Float3;
    auto GetSpecularColor() const -> Float3;
    auto GetEmissiveColor() const -> Float3;
    auto GetAmbientIntensity() const -> Float;
    auto GetShininess() const -> Float;
    auto GetTransparency() const -> Float;

private:
    auto SetDiffuseColor(std::string&&) -> void;
    auto SetSpecularColor(std::string&&) -> void;
    auto SetEmissiveColor(std::string&&) -> void;
    auto SetAmbientIntensity(std::string&&) -> void;
    auto SetShininess(std::string&&) -> void;
    auto SetTransparency(std::string&&) -> void;

private:
    Float3 _diffuseColor;
    Float3 _specularColor;
    Float3 _emissiveColor;
    Float _ambientIntensity;
    Float _shininess;
    Float _transparency;
};

}