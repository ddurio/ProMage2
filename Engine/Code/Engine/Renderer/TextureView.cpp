#include "Engine/Renderer/TextureView.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "dxguid.lib" )

// Engine Headers
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture2D.hpp"


ID3D11View* TextureView::GetView() const {
    if( m_shaderView != nullptr ) {
        return m_shaderView;
    } else if( m_depthView != nullptr ) {
        return m_depthView;
    } else if( m_renderView != nullptr ) {
        return m_renderView;
    }

    return nullptr;
}


ID3D11Resource* TextureView::GetResource() const {
    return m_source;
}


ID3D11ShaderResourceView* TextureView::GetShaderView() const {
    return m_shaderView;
}


ID3D11RenderTargetView* TextureView::GetRenderView() const {
    return m_renderView;
}


std::string TextureView::GetFilePath() const {
    return m_textureFilePath;
}


TextureView::TextureView( ID3D11Device* device, Texture* texture ) {
    ID3D11Resource* d3dTexture = texture->GetHandle();
    GUARANTEE_OR_DIE( d3dTexture != nullptr, "(TextureView) Input texture missing required resource." );

    TextureType type = texture->GetType();

    if( type == TEXTURE_TYPE_IMAGE || type == TEXTURE_TYPE_IMAGE_COLOR ) {
        // Shader View
        device->CreateShaderResourceView( d3dTexture, nullptr, &m_shaderView );
        GUARANTEE_OR_DIE( m_shaderView != nullptr, "(TextureView) Failed to create D3D ShaderResourceView." );

        m_textureFilePath = texture->GetFilePath();
        RenderContext::SetD3DDebugName( m_shaderView, m_textureFilePath );
    } else if( type == TEXTURE_TYPE_COLOR_TARGET ) {
        // Render View
        device->CreateRenderTargetView( d3dTexture, nullptr, &m_renderView );
        GUARANTEE_OR_DIE( m_renderView != nullptr, "(TextureView) Failed to create D3D RenderTargetView." );
        RenderContext::SetD3DDebugName( m_renderView, m_textureFilePath );

        // Shader View
        device->CreateShaderResourceView( d3dTexture, nullptr, &m_shaderView );
        GUARANTEE_OR_DIE( m_shaderView != nullptr, "(TextureView) Failed to create D3D ShaderResourceView." );

        RenderContext::SetD3DDebugName( m_shaderView, m_textureFilePath );
    } else if( type == TEXTURE_TYPE_DEPTH ) {
        // Depth View
        ID3D11DepthStencilView* dsv = nullptr;
        D3D11_DEPTH_STENCIL_VIEW_DESC description;

        memset( &description, 0, sizeof( D3D11_DEPTH_STENCIL_VIEW_DESC ) );
        description.Format          = DXGI_FORMAT_D24_UNORM_S8_UINT;
        description.ViewDimension   = D3D11_DSV_DIMENSION_TEXTURE2D;

        device->CreateDepthStencilView( d3dTexture, &description, &dsv );
        GUARANTEE_OR_DIE( dsv != nullptr, "(TextureView) Failed to create D3D DepthStencilView." );

        m_depthView = dsv;
        RenderContext::SetD3DDebugName( m_depthView, m_textureFilePath );
    }

    m_source = d3dTexture;
    d3dTexture->AddRef();
}


TextureView::~TextureView() {
    DX_SAFE_RELEASE( m_source );
    DX_SAFE_RELEASE( m_shaderView );
    DX_SAFE_RELEASE( m_depthView );
    DX_SAFE_RELEASE( m_renderView );
}
