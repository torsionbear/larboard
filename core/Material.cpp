#include "Material.h"

#include "GlRuntimeHelper.h"

namespace core {

auto Material::ShaderData::Size() -> unsigned int {
    static unsigned int size = 0u;
    if (size == 0u) {
        size = GlRuntimeHelper::GetUboAlignedSize(sizeof(Material::ShaderData));
    }
    return size;
}

}