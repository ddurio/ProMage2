#include "Engine/Async/CopyTextureJob.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Texture2D.hpp"


CopyTextureJob::CopyTextureJob( JobSystem* jobSystem, ID3D11DeviceContext* d3dContext, Texture2D* stagingTexture, Image* destination /*= nullptr */, const std::string& callbackEvent /*= "" */ ) :
    Job( jobSystem, JOB_CATEGORY_RENDER, callbackEvent ),
    m_d3dContext( d3dContext ),
    m_stagingTexture( stagingTexture ),
    m_destImage( destination ) {

    if( m_destImage == nullptr ) {
        IntVec2 dimensions = stagingTexture->GetDimensions();
        m_destImage = new Image( Rgba::MAGENTA, dimensions );
    }
}


CopyTextureJob::~CopyTextureJob() {
    CLEAR_POINTER( m_stagingTexture );
}


Image* CopyTextureJob::GetImage() const {
    return m_destImage;
}


void CopyTextureJob::Execute() {
    m_stagingTexture->CopyToImage( m_d3dContext, m_destImage );
}
