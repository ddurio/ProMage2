#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


class Texture2D;

class TextureView2D : public TextureView {
    friend class Camera;
    friend class EngineCommon;
    friend class RenderContext;

    public:
    IntVec2 GetDimensions() const;
    int GetWidth() const;
    int GetHeight() const;
    float GetAspect() const;


    private:
    TextureView2D( ID3D11Device* device, Texture2D* texture );
    virtual ~TextureView2D();

    IntVec2 m_dimensions = IntVec2::ZERO;
};
