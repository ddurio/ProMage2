#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderTypes.hpp"

class RenderContext;

class RenderBuffer {
    public:
    RenderBuffer( RenderContext* renderContext );
    ~RenderBuffer();

    size_t GetSize() const;
    bool IsStatic() const;
    bool IsDynamic() const;

    ID3D11Buffer* GetBufferHandle() const;

    protected:
    RenderContext* m_renderContext = nullptr;
    GPUMemoryUsage m_memoryUsage = GPU_MEMORY_USAGE_STATIC;

    RboUsageBits m_bufferUsage = 0u;
    size_t m_bufferSize = 0;
    size_t m_elementSize = 0;

    ID3D11Buffer* m_bufferHandle = nullptr;

    static unsigned int GetBufferUsageDXFromRBO( RboUsageBits bufferUsage );
    std::string GetBufferNameFromType() const;

    bool Create( const void* initialData, size_t bufferSize, size_t elementSize, RboUsageBits bufferUsage, GPUMemoryUsage memoryUsage );
    bool CopyCPUToGPU( const void* data, int byteSize );

};
