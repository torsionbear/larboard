#include "PngReader.h"

#include "zlib.h"

#include "Endian.h"

using std::ifstream;
using std::string;
using std::unique_ptr;
using std::make_unique;
using std::move;
using std::vector;

namespace core {

class PngReader::PngReaderImpl {
public:
	PngReaderImpl(string const& filename)
		: _file{ filename.c_str(), ifstream::binary } {
		_zStream.zalloc = Z_NULL;
		_zStream.zfree = Z_NULL;
		_zStream.opaque = Z_NULL;
		assert(_file);
	}
	PngReaderImpl(PngReader const&) = delete;
	PngReaderImpl& operator=(PngReader const&) = delete;
	~PngReaderImpl() = default;

public:
	auto ReadPng() {
		if (!_file) {
			throw("unable to open png file");
		}
		ReadHeader();
		for (auto hasMoreChunk = true; hasMoreChunk;) {
			hasMoreChunk = ReadChunk();
		}
		Reconstruct();
	}
	auto GetData() -> vector<Vector4f> {
		auto data = vector<Vector4f>{};
		data.reserve(_width * _height);

		auto texelLength = _channelSize * _channelCount;
		auto scanlineLength = _width * texelLength + 1;
		for (auto i = 0u; i < _height; ++i) {
			// opengl expect texture coordinate (0, 0) at bottom left. 
			// we reverse the scanline order so texel start at bottom left.
			auto scanlinePos = (_height - i - 1) * scanlineLength;
			for (auto j = scanlinePos + 1; j < scanlinePos + scanlineLength; j += texelLength) {
				data.push_back({
					static_cast<Float32>(_cache[j]) / 255.0f,
					static_cast<Float32>(_cache[j + 1]) / 255.0f,
					static_cast<Float32>(_cache[j + 2]) / 255.0f,
					static_cast<Float32>(_cache[j + 3]) / 255.0f });
			}
		}
		return data;
	}
	auto Height() {
		return _height;
	}
	auto Width() {
		return _width;
	}

private:
	auto ReadChunkLength() -> unsigned int {
		auto chunkLength = unsigned int{};
		_file.read(reinterpret_cast<char*>(&chunkLength), 4);
		chunkLength = Endian::ConvertBigEndian(chunkLength);
		return chunkLength;
	}
	auto ReadChunkType() -> string {
		char buffer[4];
		_file.read(buffer, 4);
		return string{ buffer, 4 };
	}
	auto ReadChunkData(unsigned int chunkLength) -> unique_ptr<char[]> {
		auto chunkData = make_unique<char[]>(chunkLength);
		_file.read(chunkData.get(), chunkLength);
		return chunkData;
	}
	auto ReadChunkCrc() -> bool {
		_file.ignore(4u);
		return true;
	}
	auto ReadChunk() -> bool {
		auto chunkLength = ReadChunkLength();
		auto chunkType = ReadChunkType();
		auto chunkData = ReadChunkData(chunkLength);
		ReadChunkCrc();

		if (chunkType == "IEND") {
			return false;
		} else if (chunkType == "IHDR") {
			ReadIhdr(move(chunkData));
		} else if (chunkType == "sRGB") {
			ReadSrgb(move(chunkData));
		} else if (chunkType == "gAMA") {
			ReadGama(move(chunkData));
		} else if (chunkType == "pHYs") {
			ReadPhys(move(chunkData));
		} else if (chunkType == "bKGD") {
			ReadBkgd(move(chunkData));
		} else if (chunkType == "IDAT") {
			ReadIDAT(move(chunkData), chunkLength);
		} else {
			return true;
		}
		return true;
	}
	auto ReadHeader() -> bool {
		_file.ignore(8);
		return true;
	}
	auto ReadIhdr(unique_ptr<char[]> && chunkData) -> void {
		_width = Endian::ConvertBigEndian(*reinterpret_cast<unsigned int*>(&chunkData[0]));
		_height = Endian::ConvertBigEndian(*reinterpret_cast<unsigned int*>(&chunkData[4]));
		auto bitDepth = *reinterpret_cast<unsigned char*>(&chunkData[8]);
		_channelSize = bitDepth / 8;
		auto colorType = *reinterpret_cast<unsigned char*>(&chunkData[9]);
		if (6u == colorType) {
			_channelCount = 4;
		} else if (2u == colorType) {
			_channelCount = 3;
		} else {
			assert(false); // only support RGB(2u == colorType) or RGBA(6u == colorType)
		}
		_compressionMethod = *reinterpret_cast<unsigned char*>(&chunkData[10]);
		assert(0u == _compressionMethod);
		_filterMethod = *reinterpret_cast<unsigned char*>(&chunkData[11]);
		assert(0u == _filterMethod);
		_interlaceMethod = *reinterpret_cast<unsigned char*>(&chunkData[12]);
		assert(0u == _interlaceMethod);

		auto cacheSize = _height * (_width * 4u + 1);
		_cache.resize(cacheSize);
		_zStream.avail_out = cacheSize;
		_zStream.next_out = _cache.data();
		auto result = inflateInit(&_zStream);
	}
	auto ReadSrgb(unique_ptr<char[]> &&) -> void {
		return;
	}
	auto ReadGama(unique_ptr<char[]> &&) -> void {
		return;
	}
	auto ReadPhys(unique_ptr<char[]> &&) -> void {
		return;
	}
	auto ReadBkgd(unique_ptr<char[]> &&) -> void {
		return;
	}
	auto ReadIDAT(unique_ptr<char[]> && chunkData, unsigned int length) -> void {
		_zStream.avail_in = length;
		_zStream.next_in = reinterpret_cast<unsigned char*>(chunkData.get());
		auto result = inflate(&_zStream, Z_SYNC_FLUSH);
		assert(result == 0u || result == 1u);
	}
	auto Reconstruct() -> void {
		assert(1u == _channelSize);	// only support 1-byte channel because we copy data char by char
		auto texelLength = _channelSize * _channelCount;
		auto scanlineLength = _width * texelLength + 1;
		for (auto i = 0u; i < _height; ++i) {
			auto scanlinePos = i * scanlineLength;
			auto filterType = _cache[scanlinePos];
			auto reconstructionFunction = GetReconstructionFunction(filterType);
			for (auto j = scanlinePos + 1; j < scanlinePos + scanlineLength; j += texelLength) {
				for (auto k = j; k < j + texelLength; k += _channelSize) {
					auto x = _cache[k];
					auto a = scanlinePos + 1 == j ? 0 : _cache[k - texelLength];
					auto b = 0u == i ? 0 : _cache[k - scanlineLength];
					auto c = scanlinePos + 1 == j || 0u == i ? 0 : _cache[k - scanlineLength - texelLength];
					_cache[k] = reconstructionFunction(x, a, b, c);
				}
			}
		}
	}
	auto GetReconstructionFunction(unsigned char filterType) -> unsigned char (*)(unsigned char, unsigned char, unsigned char, unsigned char){
		switch (filterType) {
		default:
		case 0:
			return [](unsigned char x, unsigned char a, unsigned char b, unsigned char c) -> unsigned char { return x; };
		case 1:
			return [](unsigned char x, unsigned char a, unsigned char b, unsigned char c) -> unsigned char { return x + a; };
		case 2:
			return [](unsigned char x, unsigned char a, unsigned char b, unsigned char c) -> unsigned char { return x + b; };
		case 3:
			return [](unsigned char x, unsigned char a, unsigned char b, unsigned char c) -> unsigned char { return x + (a + b) / 2; };
		case 4:
			return [](unsigned char x, unsigned char a, unsigned char b, unsigned char c) -> unsigned char {
				auto p = a + b - c;
				auto pa = abs(p - a);
				auto pb = abs(p - b);
				auto pc = abs(p - c);
				if (pa <= pb && pa <= pc) {
					return x + a;
				} else if (pb <= pc) {
					return x + b;
				} else {
					return x + c;
				}
			};
		}
	}

private:
	unsigned int _height;
	unsigned int _width;
	unsigned char _channelSize;
	unsigned char _channelCount;
	unsigned char _compressionMethod;
	unsigned char _filterMethod;
	unsigned char _interlaceMethod;
	ifstream _file;
	z_stream _zStream;
	vector<unsigned char> _cache;
};

PngReader::PngReader(string const& filename)
	: _impl{ make_unique<PngReaderImpl>(filename) } {
}

PngReader::~PngReader() = default;

auto PngReader::ReadPng() -> void {
	_impl->ReadPng();
}

auto PngReader::GetData() -> vector<Vector4f> {
	return _impl->GetData();
}

auto PngReader::Height() -> unsigned int {
	return _impl->Height();
}

auto PngReader::Width() -> unsigned int {
	return _impl->Width();
}

}
