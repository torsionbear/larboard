#pragma once

#include <assert.h>
#include <type_traits>

class Endian {
public:
	constexpr static bool BigEndianSystem = false;

public:
	template <typename T>
	static inline auto ConvertBigEndian(T value) -> T {
		return BigEndianSystem ? value : ConvertEndian(value);
	}

	template <typename T>
	static inline auto ConvertLittleEndian(T value) -> T {
		return BigEndianSystem ? ConvertEndian(value) : value;
	}

private:
	template <typename T>
	static auto ConvertEndian(T value) -> T {
		static_assert(std::is_arithmetic<T>::value, "invalid argument for ConvertEndian().");
		constexpr auto size = sizeof(T);
		union {
			T value;
			unsigned char bytes[size];
		} original, converted;
		original.value = value;
		for (auto i = 0u; i < size; ++i) {
			converted.bytes[i] = original.bytes[size - i - 1];
		}
		return converted.value;
	}
};