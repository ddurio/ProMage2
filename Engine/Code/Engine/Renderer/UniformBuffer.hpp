#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


class UniformBuffer : public RenderBuffer {
    public:
    UniformBuffer( RenderContext* renderContext );

    bool CopyCPUToGPU( const void* data, int byteSize );

    private:
};
