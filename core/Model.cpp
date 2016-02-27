#include "Model.h"

using std::vector;
using std::swap;

namespace core {

auto swap(Model & first, Model & second) -> void {
}

Model::Model(Model && other)
    : Model() {
}

Model & Model::operator=(Model && rhs) {
    //swap members
    return *this;
}

}