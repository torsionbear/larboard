#pragma once

#include <vector>

#include "Movable.h"

namespace core {

class Model : public Movable {
public:
	Model() = default;
	Model(Model const &) = delete;
	Model(Model &&);
	Model & operator=(Model const &) = delete;
	Model & operator=(Model &&);
	friend auto swap(Model & first, Model & second) -> void;
};

}