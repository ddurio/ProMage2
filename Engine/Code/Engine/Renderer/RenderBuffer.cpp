#include "Engine/Renderer/RenderBuffer.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )

// Engine Systems
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/RenderContext.hpp"


RenderBuffer::RenderBuffer( RenderContext* renderContext ) :
    m_renderContext(renderContext) {
}


RenderBuffer::~RenderBuffer() {
    DX_SAFE_RELEASE( m_bufferHandle );
}


size_t RenderBuffer::GetSize() const {
    return m_bufferSize;
}


bool RenderBuffer::IsStatic() const {
    return (m_memoryUsage == GPU_MEMORY_USAGE_STATIC);
}


bool RenderBuffer::IsDynamic() const {
    return (m_memoryUsage == GPU_MEMORY_USAGE_DYNAMIC);
}


ID3D11Buffer* RenderBuffer::GetBufferHandle() const {
    return m_bufferHandle;
}


unsigned int RenderBuffer::GetBufferUsageDXFromRBO( RboUsageBits bufferUsage ) {
    unsigned int dxUsage = 0u;

    if( bufferUsage & RBO_USAGE_VERTEX_STREAM_BIT ) {
        dxUsage |= D3D11_BIND_VERTEX_BUFFER;
    }

    if( bufferUsage & RBO_USAGE_INDEX_STREAM_BIT ) {
        dxUsage |= D3D11_BIND_INDEX_BUFFER;
    }

    if( bufferUsage & RBO_USAGE_UNIFORMS_BIT ) {
        dxUsage |= D3D11_BIND_CONSTANT_BUFFER;
    }

    return dxUsage;
}


bool RenderBuffer::Create( const void* initialData, size_t bufferSize, size_t elementSize, RboUsageBits bufferUsage, GPUMemoryUsage memoryUsage ) {
    // Free the old handle
    DX_SAFE_RELEASE( m_bufferHandle );

    // can not create a 0 sized buffer; 
    ASSERT_RETURN_VALUE( (bufferSize > 0U) && (elementSize > 0U), false );

    // static buffers MUST be supplied data.
    ASSERT_RETURN_VALUE( (memoryUsage != GPU_MEMORY_USAGE_STATIC) || (initialData != nullptr), false );

    // Setup the buffer and D3D Object description
    D3D11_BUFFER_DESC bufferDescription;
    memset( &bufferDescription, 0, sizeof( D3D11_BUFFER_DESC ) );

    bufferDescription.ByteWidth = (UINT)bufferSize;
    bufferDescription.StructureByteStride = (UINT)elementSize;
    bufferDescription.Usage = GetMemoryUsageDXFromGPU( memoryUsage );
    bufferDescription.BindFlags = GetBufferUsageDXFromRBO( bufferUsage );

    // give us write access to dynamic buffers
    // and read/write access to staging buffers; 
    bufferDescription.CPUAccessFlags = 0U;
    if( memoryUsage == GPU_MEMORY_USAGE_DYNAMIC ) {
        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    } else if( memoryUsage == GPU_MEMORY_USAGE_STAGING ) {
        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    }

    // DFS1FIXME: Support structured buffers (Summer Semester)

    // Default to nullptr, changed if initial data provided
    D3D11_SUBRESOURCE_DATA* dataPointer = nullptr;

    if( initialData != nullptr ) {
        D3D11_SUBRESOURCE_DATA d3dInitialData;
        memset( &d3dInitialData, 0, sizeof( D3D11_SUBRESOURCE_DATA ) );

        d3dInitialData.pSysMem = initialData;
        dataPointer = &d3dInitialData;
    }

    ID3D11Device* renderDevice = m_renderContext->m_d3dDevice;
    HRESULT hr = renderDevice->CreateBuffer( &bufferDescription, dataPointer, &m_bufferHandle );

    if( SUCCEEDED( hr ) ) {
        // save off options; 
        m_bufferUsage = bufferUsage;
        m_memoryUsage = memoryUsage;
        m_bufferSize = bufferSize;
        m_elementSize = elementSize;

        // name it for debugging
        float gameTime = (float)GetCurrentTimeSeconds();
        std::string usageName = GetBufferNameFromType();
        std::string bufferName = Stringf( "Type: %s, Time: %f", usageName.c_str(), gameTime );
        RenderContext::SetD3DDebugName( m_bufferHandle, bufferName );

        return true;
    } else {
        return false;
    }
}


std::string RenderBuffer::GetBufferNameFromType() const {
    std::string typeName = "";

    if( (m_bufferUsage & RBO_USAGE_VERTEX_STREAM_BIT) != 0 ) {
        typeName = Stringf( "%s/%s", typeName.c_str(), "Vertex" );
    }

    if( (m_bufferUsage & RBO_USAGE_INDEX_STREAM_BIT) != 0 ) {
        typeName = Stringf( "%s/%s", typeName.c_str(), "Index" );
    }

    if( (m_bufferUsage & RBO_USAGE_UNIFORMS_BIT) != 0 ) {
        typeName = Stringf( "%s/%s", typeName.c_str(), "Uniform" );
    }

    std::string finalName = std::string( typeName, 1 );
    return finalName;
}


bool RenderBuffer::CopyCPUToGPU( const void* data, int byteSize ) {
    ASSERT( !IsStatic() );
    ASSERT( byteSize <= m_bufferSize );

    ID3D11DeviceContext *d3dContext = m_renderContext->m_d3dContext;
    D3D11_MAPPED_SUBRESOURCE resource;

    HRESULT result = d3dContext->Map( m_bufferHandle, 0, D3D11_MAP_WRITE_DISCARD, 0u, &resource );

    if( SUCCEEDED( result ) ) { // Mapped correctly
        memcpy( resource.pData, data, byteSize );

        // unlock the resource (we're done writing)
        d3dContext->Unmap( m_bufferHandle, 0 );
        return true;
    }

    return false;
}
