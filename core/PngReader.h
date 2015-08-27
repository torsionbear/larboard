#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <vector>

#include "Primitive.h"

namespace core {

class PngReader {
private:
	class PngReaderImpl;

public:
	PngReader(std::string const&);
	PngReader(PngReader const&) = delete;
	PngReader& operator=(PngReader const&) = delete;
	~PngReader();

public:
	auto ReadPng() -> void;
	auto GetData() ->std::vector<Vector4f>;
	auto Height() -> unsigned int;
	auto Width() -> unsigned int;

private:
	std::unique_ptr<PngReaderImpl> _impl;
};

}