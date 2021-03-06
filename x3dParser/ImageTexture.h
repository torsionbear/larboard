#pragma once

#include "X3dNode.h"

namespace x3dParser {

class ImageTexture : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetUrl() const -> std::vector<std::string>;

private:
    auto SetUrl(std::string&&) -> void;

private:
    std::vector<std::string> _url;
};

}