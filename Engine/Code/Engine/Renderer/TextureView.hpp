#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


class Texture;

class TextureView {
    public:
    ID3D11View* GetView() const;
    ID3D11Resource* GetResource() const;
    ID3D11ShaderResourceView* GetShaderView() const;
    ID3D11RenderTargetView* GetRenderView() const;
    std::string GetFilePath() const;

    protected:
    TextureView( ID3D11Device* device, Texture* texture );
    virtual ~TextureView();

    ID3D11Resource* m_source = nullptr;
    ID3D11ShaderResourceView* m_shaderView = nullptr;
    ID3D11DepthStencilView* m_depthView = nullptr;
    ID3D11RenderTargetView* m_renderView = nullptr;

    std::string m_textureFilePath = "";
};
