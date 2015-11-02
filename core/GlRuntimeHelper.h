#pragma once

#include <GL/glew.h>

#include "Primitive.h"

namespace core {

class GlRuntimeHelper {
public:
    // There is an alignment restriction for UBOs when binding. 
    // Any glBindBufferRange/Base's offset must be a multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT. 
    // This alignment could be anything, so you have to query it before building your array of uniform buffers. 
    // That means you can't do it directly in compile-time C++ logic; it has to be runtime logic.
    // see http://stackoverflow.com/questions/13028852/issue-with-glbindbufferrange-opengl-3-1
    static auto GetUboAlignedSize(unsigned int dataSize) -> unsigned int {
        static openglInt uboAlignment = 0u;
        if (uboAlignment == 0u) {
            glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlignment);
        }
        auto remainder = dataSize % uboAlignment;
        return remainder == 0 ? dataSize : dataSize + uboAlignment - remainder;
    }
};

}