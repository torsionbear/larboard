#pragma once

#include "X3dNode.h"

namespace x3dParser {

class ImageTexture : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetUrl() const -> std::vector<std::string>;

private:
    auto SetUrl(std::string&&) -> void;

private:
    std::vector<std::string> _url;
};

}