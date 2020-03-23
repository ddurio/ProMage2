#include "Engine/Renderer/UniformBuffer.hpp"


UniformBuffer::UniformBuffer( RenderContext* renderContext ) :
    RenderBuffer(renderContext) {
}


bool UniformBuffer::CopyCPUToGPU( const void* data, int byteSize ) {
    if( byteSize > GetSize() || IsStatic() ) {
        // Current buffer not big enough or static... create new one
        return Create( data, byteSize, byteSize, RBO_USAGE_UNIFORMS_BIT, GPU_MEMORY_USAGE_DYNAMIC );
    } else {
        ASSERT( IsDynamic() );

        // Current buffer is good to re-use... call copy
        if( RenderBuffer::CopyCPUToGPU( data, byteSize ) ) {
            return true;
        }
    }

    return false;
}
