#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


class VertexBuffer : public RenderBuffer {
    public:
    VertexBuffer( RenderContext* renderContext );

    bool CopyCPUToGPU( const void* vertices, int count, size_t vertexStride );

    size_t GetStride() const;

    private:
    int m_vertexCount = 0;
    size_t m_vertexStride = 0;
};
