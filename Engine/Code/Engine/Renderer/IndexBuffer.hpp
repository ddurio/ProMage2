#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderBuffer.hpp"

class RenderContext;


class IndexBuffer : public RenderBuffer {
    public:
    IndexBuffer( RenderContext* renderContext );

    bool CopyCPUToGPU( const int* indices, int count );

    private:
    int m_indexCount = 0;
};
