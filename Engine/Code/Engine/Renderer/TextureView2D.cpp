#include "Engine/Renderer/TextureView2D.hpp"

#include "Engine/Renderer/Texture2D.hpp"


IntVec2 TextureView2D::GetDimensions() const {
    return m_dimensions;
}


int TextureView2D::GetWidth() const {
    return m_dimensions.x;
}


int TextureView2D::GetHeight() const {
    return m_dimensions.y;
}


float TextureView2D::GetAspect() const {
    return (float)m_dimensions.x / (float)m_dimensions.y;
}


TextureView2D::TextureView2D( ID3D11Device* device, Texture2D* texture ) :
    TextureView( device, texture ) {
    m_dimensions = texture->GetDimensions();
}


TextureView2D::~TextureView2D() {

}
