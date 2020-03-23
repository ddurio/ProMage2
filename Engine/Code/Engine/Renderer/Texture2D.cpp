#include "Engine/Renderer/Texture2D.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "dxguid.lib" )

// Engine Headers
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"


IntVec2 Texture2D::GetDimensions() const {
    return m_dimensions;
}


bool Texture2D::CopyToImage( ID3D11DeviceContext* d3dContext, Image* image ) const {
    D3D11_MAPPED_SUBRESOURCE resource;

    HRESULT result = d3dContext->Map( m_d3dTexture, 0, D3D11_MAP_READ, 0u, &resource );

    if( SUCCEEDED( result ) ) { // Mapped correctly
        if( image->m_rawData != nullptr ) {
            free( image->m_rawData );
        }

        int numTexels = m_dimensions.x * m_dimensions.y;
        int numBytes = numTexels * 4;
        image->m_rawData = (unsigned char*)malloc( numBytes );
        memcpy( image->m_rawData, resource.pData, numBytes );

        // unlock the resource (we're done writing)
        d3dContext->Unmap( m_d3dTexture, 0 );
        return true;
    }

    return false;
}


Texture2D::Texture2D( ID3D11Device* device, TextureInfo info ) :
    Texture(device, info) {
    Image* image = nullptr;
    bool imageNeedsDeletion = true;
    unsigned int cpuAccessFlag = 0u;

    if( info.type == TEXTURE_TYPE_IMAGE ) {
        if( info.existingImage == nullptr ) {
            image = new Image( info.imageFilePath );
            GUARANTEE_OR_DIE( image != nullptr, Stringf( "(Texture2D) Failed to create image from file %s", info.imageFilePath.c_str() ) );
        } else {
            image = info.existingImage;
            imageNeedsDeletion = false;
        }

        info.dimensions = image->GetDimensions();
    } else if( info.type == TEXTURE_TYPE_IMAGE_COLOR ) {
        image = new Image( Rgba(info.imageFilePath) );
        GUARANTEE_OR_DIE( image != nullptr, Stringf( "(Texture2D) Failed to create image from file %s", info.imageFilePath.c_str() ) );

        info.dimensions = image->GetDimensions();
        info.type = TEXTURE_TYPE_IMAGE;
    } else if( info.type == TEXTURE_TYPE_STAGING ) {
        cpuAccessFlag = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    }

    m_textureUsage = info.textureUsage;
    m_memoryUsage = info.memoryUsage;

    // Setup the Texture Description (what the resource will be like on the GPU)
    D3D11_TEXTURE2D_DESC texDesc;
    memset( &texDesc, 0, sizeof(D3D11_TEXTURE2D_DESC) );

    texDesc.Width = (unsigned int)info.dimensions.x;
    texDesc.Height = (unsigned int)info.dimensions.y;
    texDesc.MipLevels = 1;                                      // setting to 0 means there's a full chain (or can generate a full chain)
    texDesc.ArraySize = 1;                                      // only one texture
    texDesc.Usage = GetMemoryUsageDXFromGPU( m_memoryUsage );   // loaded from image - probably not changing
    texDesc.Format = info.format;
    texDesc.BindFlags = GetDXBindFromUsage( m_textureUsage );   // only allowing render target for mipmap generation
    texDesc.CPUAccessFlags = cpuAccessFlag;                     // Determines how I can access this resource CPU side 
    texDesc.MiscFlags = 0u;

    // If Multisampling - set this up.
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;


    // Setup Initial Data
    D3D11_SUBRESOURCE_DATA data;

    if( info.type == TEXTURE_TYPE_IMAGE ) {
        // pitch is how many bytes is a single row of pixels;  
        int pitch = info.dimensions.x * 4; // 4 bytes for an R8G8B8A8 format
        memset( &data, 0, sizeof( D3D11_SUBRESOURCE_DATA ) );
        data.pSysMem = image->GetRawData();
        data.SysMemPitch = pitch;
    }


    // Actually create it
    D3D11_SUBRESOURCE_DATA* dataPtr = (info.type == TEXTURE_TYPE_IMAGE ? &data : nullptr);
    ID3D11Texture2D* tex2D = nullptr;
    HRESULT hr = m_d3dDevice->CreateTexture2D( &texDesc, dataPtr, &tex2D );

    if( SUCCEEDED( hr ) ) {
        m_dimensions = info.dimensions;
        m_d3dTexture = tex2D;

        RenderContext::SetD3DDebugName( tex2D, info.imageFilePath );

        // SD2FIXME: later assignment, generate mips if option is set

        if( imageNeedsDeletion ) {
            delete image;
        }
    } else {
        ERROR_AND_DIE( Stringf("(Texture2D) Failed to create D3D texture from file %s", info.imageFilePath.c_str() ) );
        // Bad error message... not all textures are created from a file path
    }
}


Texture2D::~Texture2D() {

}


Texture2D* Texture2D::MakeNewImageTexture( ID3D11Device* device, const std::string& imageFilePath ) {
    TextureInfo info;
    info.imageFilePath = imageFilePath;

    return (new Texture2D( device, info ));
}


Texture2D* Texture2D::MakeNewImageTexture( ID3D11Device* device, Image* existingImage ) {
    TextureInfo info;
    info.existingImage = existingImage;
    info.imageFilePath = existingImage->GetImageFilePath();

    return (new Texture2D( device, info ));
}


Texture2D* Texture2D::MakeNewImageTexture( ID3D11Device* device, const Rgba& textureColor ) {
    TextureInfo info;
    info.type = TEXTURE_TYPE_IMAGE_COLOR;
    info.imageFilePath = textureColor.GetAsString();
    return (new Texture2D( device, info ));
}


Texture2D* Texture2D::MakeNewDepthTexture( ID3D11Device* device, const IntVec2& dimensions ) {
    TextureInfo info;
    info.textureUsage |= TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT;
    info.format = DXGI_FORMAT_R24G8_TYPELESS;
    info.dimensions = dimensions;
    info.type = TEXTURE_TYPE_DEPTH;

    return (new Texture2D( device, info ));
}


Texture2D* Texture2D::MakeNewRenderTexture( ID3D11Device* device, ID3D11Texture2D* texture ) {
    TextureInfo info;
    info.textureUsage |= TEXTURE_USAGE_RENDER_TARGET_BIT;

    D3D11_TEXTURE2D_DESC description;
    texture->GetDesc( &description );
    info.dimensions = IntVec2( description.Width, description.Height );

    info.type = TEXTURE_TYPE_COLOR_TARGET;

    return (new Texture2D( device, info ));
}


Texture2D* Texture2D::MakeNewRenderTexture( ID3D11Device* device, const IntVec2& dimensions ) {
    TextureInfo info;
    info.textureUsage |= TEXTURE_USAGE_RENDER_TARGET_BIT;
    info.dimensions = dimensions;
    info.type = TEXTURE_TYPE_COLOR_TARGET;

    return (new Texture2D( device, info ));
}


Texture2D* Texture2D::MakeNewStagingTexture( ID3D11Device* device, const IntVec2& dimensions ) {
    TextureInfo info;
    info.memoryUsage = GPU_MEMORY_USAGE_STAGING;
    info.textureUsage = 0u;
    info.dimensions = dimensions;
    info.type = TEXTURE_TYPE_STAGING;

    return (new Texture2D( device, info ));
}
