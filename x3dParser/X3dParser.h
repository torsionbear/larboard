#pragma once

#include <istream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "X3dNode.h"

namespace x3dParser {

class X3dParser {
public: 
    auto Parse(std::istream& is) ->std::vector<std::unique_ptr<X3dNode>>;
    
private:
    auto SplitToTags(std::istream& is) -> std::vector<std::string>;
	auto ParseTag(std::string &&) -> X3dNode *;
    auto BuildTree(std::vector<std::string>& vec) -> std::vector<std::unique_ptr<X3dNode>>;

private:
	std::vector<std::unique_ptr<X3dNode>> _nodes;
};

}