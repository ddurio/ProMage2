#include "Engine/Renderer/Texture.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "dxguid.lib" )


ID3D11Resource* Texture::GetHandle() const {
    return m_d3dTexture;
}


std::string Texture::GetFilePath() const {
    return m_filePath;
}


TextureType Texture::GetType() const {
    return m_type;
}


// Protected constructors, called only by children classes
Texture::Texture( ID3D11Device* device, const TextureInfo& info ) :
    m_d3dDevice(device),
    m_filePath(info.imageFilePath),
    m_type(info.type) {
}


Texture::~Texture() {
    DX_SAFE_RELEASE( m_d3dTexture );
}
