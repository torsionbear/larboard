#pragma once

#include <cstdint>
#include <Windows.h>

namespace d3d12RenderSystem {

using uint64 = std::uint64_t;
using uint8 = unsigned char;

inline auto ThrowIfFailed(HRESULT hr) -> void {
    if (FAILED(hr)) {
        throw;
    }
}

inline auto Align(uint8 * ptr, uint8 align) -> uint8 * {
    auto base = reinterpret_cast<unsigned long>(ptr);
    auto result = (base + align - 1) & ~(align - 1);
    return reinterpret_cast<uint8 *>(result);
}

}