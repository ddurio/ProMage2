#include "Engine/Renderer/IndexBuffer.hpp"


IndexBuffer::IndexBuffer( RenderContext* renderContext ) :
    RenderBuffer(renderContext) {
}


bool IndexBuffer::CopyCPUToGPU( const int* indices, int count ) {
    int sizeNeeded = count * sizeof( int );

    // if we don't have enough room, or this is a static
    // buffer, recreate (Create should release the old buffer)
    if( sizeNeeded > GetSize() || IsStatic() ) {
        bool result = Create( indices, sizeNeeded, sizeof( int ), RBO_USAGE_INDEX_STREAM_BIT, GPU_MEMORY_USAGE_DYNAMIC );

        if( result ) {
            m_indexCount = count;
        } else {
            m_indexCount = 0;
        }

        return result;

    } else {
        // non-static and we have enough room
        ASSERT( IsDynamic() );

        if( RenderBuffer::CopyCPUToGPU( indices, sizeNeeded ) ) {
            m_indexCount = count;
            return true;
        }
    }

    return false;
}
