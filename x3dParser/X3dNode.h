#pragma once

#include <vector>
#include <memory>

namespace x3dParser {

class X3dNode {
public:
    static auto BuildNode(std::string const& nodeType) -> std::unique_ptr<X3dNode>;

public:
    X3dNode();
    X3dNode(X3dNode const&) = delete;
    virtual ~X3dNode() = default;
    X3dNode& operator=(X3dNode const&) = delete;

public:
    virtual auto AddChild(X3dNode *) -> void = 0;
    virtual auto SetAttribute(std::string const&, std::string&&) -> void = 0;
	auto GetDef() const -> std::string const&;
	auto GetUse() const -> std::string const&;

protected:
    auto SetDef(std::string &&) -> void;
	auto SetUse(std::string && use) -> void;

protected:
    std::string _def = "";
	std::string _use = "";
};

}