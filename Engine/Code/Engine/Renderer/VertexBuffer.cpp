#include "Engine/Renderer/VertexBuffer.hpp"


VertexBuffer::VertexBuffer( RenderContext* renderContext ) :
    RenderBuffer(renderContext) {
}


bool VertexBuffer::CopyCPUToGPU( const void* vertices, int count, size_t vertexStride ) {
    int sizeNeeded = count * (int)vertexStride;

    // if we don't have enough room, or this is a static
    // buffer, recreate (Create should release the old buffer)
    if( sizeNeeded > GetSize() || IsStatic() ) {
        m_vertexStride = vertexStride;
        bool result = Create( vertices, sizeNeeded, vertexStride, RBO_USAGE_VERTEX_STREAM_BIT, GPU_MEMORY_USAGE_DYNAMIC );

        if( result ) {
            m_vertexCount = count;
        } else {
            m_vertexCount = 0;
        }

        return result;

    } else {
        // non-static and we have enough room
        ASSERT( IsDynamic() );

        if( RenderBuffer::CopyCPUToGPU( vertices, sizeNeeded ) ) {
            m_vertexCount = count;
            m_vertexStride = vertexStride;
            return true;
        }
    }

    return false;
}


size_t VertexBuffer::GetStride() const {
    return m_vertexStride;
}
