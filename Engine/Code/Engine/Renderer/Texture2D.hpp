#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


class Image;
struct Rgba;
struct TextureInfo;


class Texture2D : public Texture {
    friend class RenderContext;
    friend class CopyTextureJob;

    public:
    IntVec2 GetDimensions() const;

    bool CopyToImage( ID3D11DeviceContext* d3dContext, Image* image ) const;

    private:
    explicit Texture2D( ID3D11Device* device, TextureInfo info );
    virtual ~Texture2D();

    static Texture2D* MakeNewImageTexture( ID3D11Device* device, const std::string& imageFilePath );
    static Texture2D* MakeNewImageTexture( ID3D11Device* device, Image* existingImage );
    static Texture2D* MakeNewImageTexture( ID3D11Device* device, const Rgba& textureColor );
    static Texture2D* MakeNewDepthTexture( ID3D11Device* device, const IntVec2& dimensions );
    static Texture2D* MakeNewRenderTexture( ID3D11Device* device, ID3D11Texture2D* texture );
    static Texture2D* MakeNewRenderTexture( ID3D11Device* device, const IntVec2& dimensions );
    static Texture2D* MakeNewStagingTexture( ID3D11Device* device, const IntVec2& dimensions );

    IntVec2 m_dimensions = IntVec2::ZERO;
};
