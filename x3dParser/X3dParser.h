#pragma once

#include <istream>
#include <vector>
#include <string>
#include <memory>

#include "X3dNode.h"

namespace x3dParser {

class X3dParser {
public:
    static auto Parse(std::istream& is) -> std::unique_ptr<X3dNode>;
    
private:
    static auto SplitToTags(std::istream& is) -> std::vector<std::string>;
    static auto BuildTree(std::vector<std::string>& vec) -> std::unique_ptr<X3dNode>;
};

}