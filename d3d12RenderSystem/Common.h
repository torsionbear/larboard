#pragma once

// for string to wstring conversion
#include <locale>
#include <codecvt>

#include <Windows.h>

#include "core/Primitive.h"

namespace d3d12RenderSystem {

using core::uint8;
using core::uint32;
using core::uint64;
using core::int8;
using core::int16;
using core::int32;
using core::Float32;

inline auto ThrowIfFailed(HRESULT hr) -> void {
    if (FAILED(hr)) {
        throw;
    }
}

inline auto Align(uint8 * ptr, uint32 align) -> uint8 * {
    auto base = reinterpret_cast<unsigned long>(ptr);
    auto result = (base + align - 1) & ~(align - 1);
    return reinterpret_cast<uint8 *>(result);
}

inline auto Align(uint64 base, uint32 align) -> uint64 {
    return (base + align - 1) & ~(align - 1);
}

inline auto StringToWstring(std::string s) -> std::wstring {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(s);
}

}