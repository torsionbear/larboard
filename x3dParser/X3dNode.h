#pragma once

#include <vector>
#include <memory>

namespace x3dParser {

class X3dNode {
public:
    using pNode = std::unique_ptr<X3dNode>;
    static auto BuildNode(std::string&&) -> pNode;

public:
    X3dNode();
    X3dNode(const X3dNode&) = delete;
    virtual ~X3dNode() = default;
    X3dNode& operator=(const X3dNode&) = delete;

public:
    virtual auto AddChild(pNode) -> void = 0;
    virtual auto SetAttribute(const std::string&, std::string&&) -> void = 0;
    auto GetDef() const -> const std::string&;

protected:
    auto SetDef(std::string&&) -> void;

protected:
    std::string _def = "";
};

}