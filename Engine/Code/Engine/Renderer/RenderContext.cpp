#include "Engine/Renderer/RenderContext.hpp"

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "dxguid.lib" )

// Engine Headers
#include "Engine/Async/CopyTextureJob.hpp"
#include "Engine/Async/WriteImageJob.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Debug/Profiler.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture2D.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "regex"

void RenderContext::Startup( WindowContext* windowContext ) {
    m_consoleChannel  = g_theDevConsole->AddChannel( "RenderContext", Rgba::MAGENTA );
    m_consoleChannel |= DevConsole::CHANNEL_INFO;

    g_theDevConsole->PrintString( "(Renderer) Startup Begun...", m_consoleChannel );

    StartupD3D( windowContext->GetWindowHandle() );
    Shader* shader = GetOrCreateShader( SHADER_UNLIT );
    BindShader( shader );
    GetOrCreateShader( SHADER_CONSOLE );
    GetOrCreateShader( SHADER_INVALID );

    // Make Buffers
    m_immediateVBO = new VertexBuffer( this );
    m_frameUBO = new UniformBuffer( this );
    m_modelUBO = new UniformBuffer( this );
    m_lightUBO = new UniformBuffer( this );
    BindModelMatrix( m_modelMatrix );
    SetAmbientLight( Rgba::WHITE );

	UniformBuffer debugTint = UniformBuffer( this );
	debugTint.CopyCPUToGPU( &Rgba::WHITE, sizeof(Rgba) );
	BindUBO( &debugTint, UBO_SLOT_DEBUG_DRAW );

    memset( &m_lightBuffer, 0, sizeof( m_lightBuffer ) );
    m_lightBuffer.emissive = Rgba::CLEAR;
    m_lightBuffer.specularFactor = 1.f;
    m_lightBuffer.specularPower = 32.f;

    Texture2D* depthTexture = Texture2D::MakeNewDepthTexture( m_d3dDevice, GetBackBufferDimensions() );
    m_depthBuffer = new TextureView2D( m_d3dDevice, depthTexture );
    CLEAR_POINTER( depthTexture );

    // Make samplers
    FilterInfo pointFilter  = FilterInfo( FILTER_MODE_LINEAR, FILTER_MODE_POINT );
    FilterInfo linearFilter = FilterInfo( FILTER_MODE_LINEAR, FILTER_MODE_LINEAR );
    CreateSampler( pointFilter,  UVInfo(), SAMPLE_MODE_POINT ); // Default UV info
    CreateSampler( linearFilter, UVInfo(), SAMPLE_MODE_LINEAR );

    BindTexture( "White", TEXTURE_SLOT_COLOR );
    BindTexture( "Flat",  TEXTURE_SLOT_NORMAL );
    BindTexture( "White", TEXTURE_SLOT_SPECULAR );
    BindTexture( "Black", TEXTURE_SLOT_EMISSIVE );

    // Effect Camera
    m_effectCamera = new Camera();
    m_effectCamera->SetOrthoProjection( 2.f, -100.f, 100.f, 1.f ); // Clip space -1,-1 to 1,1

    // Screen shot event
    g_theEventSystem->Subscribe( EVENT_SCREENSHOT, this, &RenderContext::RequestScreenshot );

    g_theDevConsole->PrintString( "(Renderer) Startup Complete", m_consoleChannel );
}


void RenderContext::Shutdown() {
    g_theDevConsole->PrintString( "(Renderer) Shutdown Begun...", m_consoleChannel );

    EngineCommon::ClearMap( m_loadedTextures );
    EngineCommon::ClearMap( m_loadedFonts );
    EngineCommon::ClearMap( m_loadedShaders );
    EngineCommon::ClearMap( m_loadedMaterials );
    EngineCommon::ClearMap( m_loadedMeshes );

    EngineCommon::ClearVector( m_loadedSamplers );

    CLEAR_POINTER( m_immediateVBO );
    CLEAR_POINTER( m_frameUBO );
    CLEAR_POINTER( m_modelUBO );
    CLEAR_POINTER( m_lightUBO );

    //m_frameBufferView cleared via loadedTextures
    CLEAR_POINTER( m_frameBufferTexture );
    CLEAR_POINTER( m_depthBuffer );
    CLEAR_POINTER( m_effectCamera );

    // D3D Debug Info
    //PrintD3DDebug();
    ShutdownD3DDebug();

    DX_SAFE_RELEASE( m_d3dQueue );
    DX_SAFE_RELEASE( m_d3dSwapChain );
    DX_SAFE_RELEASE( m_d3dContext );
    DX_SAFE_RELEASE( m_d3dDevice );

    g_theEventSystem->Unsubscribe( EVENT_SCREENSHOT, this, &RenderContext::RequestScreenshot );
    g_theDevConsole->PrintString( "(Renderer) Shutdown Complete", m_consoleChannel );
}


void RenderContext::BeginFrame() {
    PROFILE_FUNCTION();

    // Get the back buffer
    ID3D11Texture2D* backBuffer = nullptr;
    m_d3dSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer );

    m_frameBufferTexture = Texture2D::MakeNewRenderTexture( m_d3dDevice, backBuffer );
    m_frameBufferView = new TextureView2D( m_d3dDevice, m_frameBufferTexture );
    m_loadedTextures[m_frameBufferID] = m_frameBufferView;

    DX_SAFE_RELEASE( backBuffer );

    ClearDepthStencilTarget();

    // Setup Frame UBO with time info
    FrameBuffer cpuData;
    cpuData.frameCount = ++m_frameCount;
    cpuData.gameTimeSeconds = (float)GetCurrentTimeSeconds();
    cpuData.cosTime = cos( cpuData.gameTimeSeconds );
    cpuData.sinTime = sin( cpuData.gameTimeSeconds );

    m_frameUBO->CopyCPUToGPU( &cpuData, sizeof( cpuData ) );
    BindUBO( m_frameUBO, UBO_SLOT_FRAME );

    PrintD3DDebug();
}


void RenderContext::EndFrame() {
    // Copy frame buffer to backBuffer
    ID3D11Texture2D* backBuffer = nullptr;
    m_d3dSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer );

    m_d3dContext->CopyResource( backBuffer, m_frameBufferTexture->GetHandle() );

    // Screen shots
    if( m_screenshotRequested != "" ) {
        IntVec2 backBuffDims = GetBackBufferDimensions();
        Texture2D* stagingTexture = Texture2D::MakeNewStagingTexture( m_d3dDevice, backBuffDims );
        m_d3dContext->CopyResource( stagingTexture->GetHandle(), backBuffer );

        CopyTextureJob* copyJob = new CopyTextureJob( m_screenshotJobSys, m_d3dContext, stagingTexture );
        WriteImageJob* writeJob = new WriteImageJob( m_screenshotJobSys, copyJob->GetImage(), m_screenshotRequested );
        writeJob->AddPrerequisite( copyJob );

        m_screenshotJobSys->StartJob( copyJob, writeJob );
        m_screenshotRequested = "";
    }

    DX_SAFE_RELEASE( backBuffer );
    CLEAR_POINTER( m_frameBufferTexture );
    CLEAR_POINTER( m_frameBufferView );
    m_loadedTextures[m_frameBufferID] = nullptr;

    // We're done rendering, so tell the swap chain they can copy the back buffer to the front (desktop/window) buffer
    m_d3dSwapChain->Present( 0, 0 );
}


Material* RenderContext::GetOrCreateMaterial( const std::string& filePath ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, Material*>::const_iterator materialIter = m_loadedMaterials.find( filePath );

    if( materialIter == m_loadedMaterials.end() ) {
        Strings splitID = SplitStringOnDelimeter( filePath, ':' );
        return CreateMaterial( splitID[0].c_str() );
    } else {
        return materialIter->second;
    }
}


CPUMesh* RenderContext::GetOrCreateMesh( const std::string& filePath ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, CPUMesh*>::const_iterator meshIter = m_loadedMeshes.find( filePath );

    if( meshIter == m_loadedMeshes.end() ) {
        Strings splitID = SplitStringOnDelimeter( filePath, ':' );
        return CreateMesh( splitID[0].c_str() );
    } else {
        return meshIter->second;
    }
     
}


void RenderContext::AddMeshToDatabase( const std::string& filePath, CPUMesh* existingMesh ) {
    m_loadedMeshes.insert( { filePath, existingMesh } );
}


void RenderContext::GetMeshXMLParams( const std::string& filePath, std::string& outObjFile, bool& outInvert, float& outScale, std::string& outTransform, std::string& outMatName ) const {
    tinyxml2::XMLDocument xmlFile = new tinyxml2::XMLDocument();
    const XMLElement& root = ParseXMLRootElement( filePath, xmlFile );

    std::string objFile = ParseXMLAttribute( root, "source", "" );
    GUARANTEE_OR_DIE( objFile != "", Stringf( "Missing source objFile path for model (%s)", filePath.c_str() ) );

    bool invert = ParseXMLAttribute( root, "invert", false );
    float scale = ParseXMLAttribute( root, "scale", 1.f );
    std::string transform = ParseXMLAttribute( root, "transform", "x y z" );

    const XMLElement* matEle = root.FirstChildElement( "Material" );
    std::string materialName = "";

    if( matEle != nullptr ) {
        materialName = ParseXMLAttribute( *matEle, "source", "" );
    }

    // Set outputs
    outObjFile = objFile;
    outInvert = invert;
    outScale = scale;
    outTransform = transform;
    outMatName = materialName;
}


void RenderContext::CreateTexture( const std::string& filePath ) {
    TextureView2D* bindable = GetOrCreateTextureView2D( filePath );
    UNUSED( bindable ); // Just know that it's stored internally
}


TextureView2D* RenderContext::GetOrCreateTextureView2D( const std::string& filePath ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::string refinedPath = (filePath == "") ? "White" : filePath;
    std::map<std::string, TextureView2D*>::const_iterator textureIter = m_loadedTextures.find( refinedPath );

    if( textureIter == m_loadedTextures.end() ) {
        return CreateTextureView2D( refinedPath );
    } else {
        return textureIter->second;
    }
}


TextureView2D* RenderContext::GetOrCreateTextureView2D( Image* existingImage ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::string filePath = existingImage->GetImageFilePath();
    std::string refinedPath = (filePath == "") ? "White" : filePath;
    std::map<std::string, TextureView2D*>::const_iterator textureIter = m_loadedTextures.find( refinedPath );

    if( textureIter == m_loadedTextures.end() ) {
        return CreateTextureView2D( existingImage );
    } else {
        return textureIter->second;
    }
}


TextureView2D* RenderContext::GetOrCreateTextureView2D( const Rgba& textureColor ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, TextureView2D*>::const_iterator textureIter = m_loadedTextures.find( textureColor.GetAsString() );

    if( textureIter == m_loadedTextures.end() ) {
        return CreateTextureView2D( textureColor );
    } else {
        return textureIter->second;
    }
}


TextureView2D* RenderContext::GetOrCreateRenderTarget( const std::string& targetID, const IntVec2& dimensions /*= IntVec2::NEGONE */ ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, TextureView2D*>::const_iterator viewIter = m_loadedTextures.find( targetID );

    if( viewIter == m_loadedTextures.end() ) {
        return CreateRenderTarget( targetID, dimensions );
    } else {
        return viewIter->second;
    }
}


TextureView2D* RenderContext::GetOrCreateDepthTarget( const std::string& targetID, const IntVec2& dimensions /*= IntVec2::NEGONE */ ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, TextureView2D*>::const_iterator viewIter = m_loadedTextures.find( targetID );

    if( viewIter == m_loadedTextures.end() ) {
        return CreateDepthTarget( targetID, dimensions );
    } else {
        return viewIter->second;
    }
}


BitmapFont* RenderContext::GetOrCreateBitmapFont( const std::string& fontName ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, BitmapFont*>::iterator fontIter = m_loadedFonts.find( fontName );

    if( fontIter == m_loadedFonts.end() ) {
        return CreateBitmapFont( fontName );
    } else {
        return fontIter->second;
    }
}


Shader* RenderContext::GetOrCreateShader( const std::string& shaderFilePath ) {
    std::scoped_lock< std::recursive_mutex > readLock( m_renderMutex );
    std::map<std::string, Shader*>::iterator shaderIter = m_loadedShaders.find( shaderFilePath );

    if( shaderIter == m_loadedShaders.end() ) {
        return CreateShader( shaderFilePath );
    } else {
        return shaderIter->second;
    }
}


Sampler* RenderContext::CreateSampler( FilterInfo filter, UVInfo uvInfo, SampleMode sampleMode /*= SAMPLE_MODE_INVALID*/ ) {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    Sampler* newSampler = new Sampler( filter, uvInfo, m_d3dDevice );

    if( sampleMode == SAMPLE_MODE_INVALID ) {
        m_loadedSamplers.push_back( newSampler );
    } else {
        // Verify the vector is large enough
        int currentSize = (int)m_loadedSamplers.size();

        if( currentSize <= (int)sampleMode ) {
            m_loadedSamplers.resize( NUM_SAMPLE_MODES, nullptr );
        } else {
            Sampler* oldSampler = m_loadedSamplers[sampleMode];

            if( oldSampler != nullptr ) {
                delete oldSampler;
                m_loadedSamplers[sampleMode] = nullptr;
            }
        }

        m_loadedSamplers[sampleMode] = newSampler;
    }

    return newSampler;
}


Shader* RenderContext::CloneShader( const Shader* cloneFrom ) {
    Shader* newShader = new Shader( *cloneFrom );
    std::map<std::string, Shader*>::const_iterator shaderIter;
    std::string id = "";
    int shaderIndex = 0;

    do {
        id = Stringf( "%s:%d", cloneFrom->m_shaderFilePath.c_str(), shaderIndex );
        shaderIter = m_loadedShaders.find( id );
        shaderIndex++;
    } while( shaderIter != m_loadedShaders.end() );

    m_loadedShaders.insert( { id, newShader } );

    return newShader;
}


Sampler* RenderContext::GetSampler( SampleMode sampleMode ) const {
    ASSERT_RETURN_VALUE( (int)m_loadedSamplers.size() > (int)sampleMode, nullptr );
    return m_loadedSamplers[sampleMode];
}


void RenderContext::DestroyTextureView2D( Image* existingImage ) {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );

    std::string filePath = existingImage->GetImageFilePath();
    std::map< std::string, TextureView2D* >::iterator texIter = m_loadedTextures.find( filePath );

    if( texIter != m_loadedTextures.end() ) {
        delete texIter->second;
        m_loadedTextures.erase( texIter );
    }
}


IntVec2 RenderContext::GetBackBufferDimensions() const {
    TextureView2D* ctv = GetCurrentRenderTarget();
    IntVec2 dimensions;

    if( ctv == nullptr ) {
        std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );

        // Get the back buffer
        ID3D11Texture2D* backBuffer = nullptr;
        m_d3dSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer );

        // Get texture dimensions
        D3D11_TEXTURE2D_DESC description;
        backBuffer->GetDesc( &description );
        dimensions = IntVec2( description.Width, description.Height );

        DX_SAFE_RELEASE( backBuffer );
    } else {
        dimensions = ctv->GetDimensions();
    }

    return dimensions;
}


TextureView2D* RenderContext::GetCurrentRenderTarget() const {
    // Look for bound camera CTV
    if( m_boundCamera != nullptr ) {
        TextureView2D* colorTarget = m_boundCamera->GetRenderTarget();

        if( colorTarget != nullptr ) {
            return colorTarget;
        }
    }

    // Default to the frameBuffer
    return m_frameBufferView;
}


TextureView2D* RenderContext::GetCurrentDepthBuffer() const {
    if( m_boundCamera != nullptr ) {
        TextureView2D* depthBuffer = m_boundCamera->GetDepthStencilView();

        if( depthBuffer != nullptr ) {
            return depthBuffer;
        }
    }

    // Default to RC depth buffer
    return m_depthBuffer;
}


std::string RenderContext::GetTextureID( const TextureView2D* viewToFind ) const {
    std::map<std::string, TextureView2D*>::const_iterator viewIter = m_loadedTextures.begin();

    for( viewIter; viewIter != m_loadedTextures.end(); viewIter++ ) {
        const TextureView2D* view = viewIter->second;

        if( view == viewToFind ) {
            return viewIter->first;
        }
    }

    ERROR_RECOVERABLE( "Could not find requested TextureView" );
    return ""; // Should never get here
}


Shader* RenderContext::GetCurrentShader() const {
    return m_boundShader;
}


void RenderContext::ClearDepthStencilTarget( float depth /*= 1.f*/, unsigned int stencil /*= 0u */ ) const {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );

    TextureView2D* dsv = GetCurrentDepthBuffer();
    ID3D11DepthStencilView* d3dView = (dsv != nullptr ? (ID3D11DepthStencilView*)dsv->GetView() : nullptr);
    m_d3dContext->ClearDepthStencilView( d3dView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, (UINT8)stencil );
}


LightDesc RenderContext::GetDynamicLight( int lightIndex ) const {
    return m_lightBuffer.lights[lightIndex];
}


void RenderContext::SetAmbientLight( const Rgba& color ) {
    if( m_lightBuffer.ambient == color ) {
        return;
    }

    m_lightBuffer.ambient = color;
    m_lightBufferDirty = true;
}


void RenderContext::SetEmissiveLight( const Rgba& color ) {
    if( m_lightBuffer.emissive == color ) {
        return;
    }

    m_lightBuffer.emissive = color;
    m_lightBufferDirty = true;
}


void RenderContext::SetRenderMode( int renderMode ) {
    float asFloat = (float)renderMode;
    if( m_lightBuffer.renderMode == asFloat ) {
        return;
    }

    m_lightBuffer.renderMode = asFloat;
    m_lightBufferDirty = true;
}


void RenderContext::SetDynamicLight( int lightIndex, const LightDesc& light ) {
    m_lightBuffer.lights[lightIndex] = light;
    m_lightBufferDirty = true;
}


void RenderContext::BindModelMatrix( Matrix44 model /*= Matrix44()*/ ) {
    m_modelMatrix = model;

    m_modelUBO->CopyCPUToGPU( &m_modelMatrix, sizeof( Matrix44 ) );
    BindUBO( m_modelUBO, UBO_SLOT_MODEL );
}


void RenderContext::BindMaterial( const Material* material ) {
    // Shader
    BindShader( material->GetShader() );

    // Samplers
    std::vector<Sampler*> samplers = material->GetSamplers();
    int numSamplers = (int)samplers.size();

    if( numSamplers == 0 ) {
        BindSampler( nullptr );
    } else {
        for( int samplerIndex = 0; samplerIndex < numSamplers; samplerIndex++ ) {
            Sampler* sampler = samplers[samplerIndex];

            if( sampler != nullptr ) {
                BindSampler( sampler, samplerIndex );
            }
        }
    }

    // Textures
    Strings textures = material->GetTextures();
    int numTextures = (int)textures.size();

    for( int textureIndex = 0; textureIndex < numTextures; textureIndex++ ) {
        const std::string& texture = textures[textureIndex];

        TextureView2D* textureView = GetOrCreateTextureView2D( texture );
        BindTextureView2D( textureView, textureIndex );
    }

    // User Uniform Buffer
    UniformBuffer* userBuffer = material->GetUserBuffer();

    if( userBuffer != nullptr ) {
        BindUBO( userBuffer, UBO_SLOT_USER_MATERIAL );
    }
}


void RenderContext::BindMaterial( const std::string& materialName ) {
    Material* material = GetOrCreateMaterial( materialName );
    BindMaterial( material );
}


void RenderContext::BindTexture( const std::string& filePath /*= ""*/, int slotIndex /*= 0*/ ) {
    TextureView2D* bindable = GetOrCreateTextureView2D( filePath );
    BindTextureView2D( bindable, slotIndex );
    BindSampler( nullptr );
}


void RenderContext::BindTextureView2D( const TextureView2D* textureView, int slotIndex /*= 0*/ ) {
    if( textureView == nullptr ) {
        textureView = GetOrCreateTextureView2D( "" );
    }

    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    ID3D11ShaderResourceView* srv = textureView->GetShaderView();
    m_d3dContext->PSSetShaderResources( slotIndex, 1, &srv );
}


void RenderContext::BindShader( const std::string& shaderPath ) {
    Shader* shader = GetOrCreateShader( shaderPath.c_str() );
    BindShader( shader );
}


void RenderContext::BindShader( Shader* shader ) {
    if( shader != nullptr ) {
        m_boundShader = shader;
    } else {
        m_boundShader = GetOrCreateShader( SHADER_UNLIT );
        m_boundShader->SetDepthMode( COMPARE_LESS_EQUAL, true );
        m_boundShader->SetBlendMode( BLEND_MODE_ALPHA );
        m_boundShader->SetFillMode( FILL_MODE_SOLID );
    }

    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    m_d3dContext->OMSetBlendState( m_boundShader->m_blendState, black, 0xffffffff );
    m_d3dContext->OMSetDepthStencilState( m_boundShader->m_depthState, 0u );
    m_d3dContext->RSSetState( m_boundShader->m_rasterState );
}


void RenderContext::BindSampler( Sampler* sampler, int slotIndex /*= 0*/ ) {
    if( sampler == nullptr ) {
        sampler = m_loadedSamplers[SAMPLE_MODE_DEFAULT];
    }

    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    ID3D11SamplerState *samplerState = sampler->GetHandle();
    m_d3dContext->PSSetSamplers( slotIndex, 1U, &samplerState );
}


void RenderContext::BindUBO( const UniformBuffer* ubo, int slotIndex ) {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );

   ID3D11Buffer* buffer = (ubo != nullptr) ? ubo->GetBufferHandle() : nullptr; 
   m_d3dContext->VSSetConstantBuffers( slotIndex, 1u, &buffer ); 
   m_d3dContext->PSSetConstantBuffers( slotIndex, 1u, &buffer ); 
}


void RenderContext::ApplyEffect( TextureView2D* source, const std::string& materialID, TextureView2D* destination /*= nullptr */ ) {
    Material* material = GetOrCreateMaterial( materialID );
    ApplyEffect( source, material, destination );
}


void RenderContext::ApplyEffect( TextureView2D* source, Material* effectMaterial, TextureView2D* destination /*= nullptr */ ) {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    Texture2D* destTexture = nullptr;
    bool copyToSource = false;

    if( destination == nullptr ) { // Make a scratch and then copy it back to source at the end
        destTexture = Texture2D::MakeNewRenderTexture( m_d3dDevice, (ID3D11Texture2D*)source->GetResource() );
        destination = new TextureView2D( m_d3dDevice, destTexture );
        copyToSource = true;
    }

    m_effectCamera->SetRenderTarget( destination );
    BeginCamera( m_effectCamera );
    BindMaterial( effectMaterial );
    BindTextureView2D( source );

    Draw( 3 );

    BindTextureView2D( nullptr );
    EndCamera( m_effectCamera );

    if( copyToSource ) {
        m_d3dContext->CopyResource( source->GetResource(), destination->GetResource() );

        CLEAR_POINTER( destTexture );
        CLEAR_POINTER( destination );
    }
}


void RenderContext::CopyD3DResource( const TextureView2D* source, const TextureView2D* destination ) {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    m_d3dContext->CopyResource( destination->GetResource(), source->GetResource() );
}

void RenderContext::ClearRenderTarget( const Rgba& clearColor ) {
    TextureView2D* colorTarget = GetCurrentRenderTarget();

	// Clear all screen (back buffer) pixels to a color
    float clear_color[4];
    clearColor.GetAsArray( clear_color );

    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    m_d3dContext->ClearRenderTargetView( colorTarget->m_renderView, clear_color );
}


void RenderContext::BeginCamera( Camera* camera ) {
    if( m_boundCamera == camera ) {
        return;
    }

    m_boundCamera = camera;
    TextureView2D* colorTarget = GetCurrentRenderTarget();
    TextureView2D* depthBuffer = GetCurrentDepthBuffer();

    GUARANTEE_OR_DIE( colorTarget->GetDimensions() == depthBuffer->GetDimensions(), "(RenderContext) Cannot use color target and depth buffer of different sizes." );

    // Set it as our output
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    m_d3dContext->OMSetRenderTargets( 1, &(colorTarget->m_renderView), depthBuffer->m_depthView );

    // Next, we have to describe what part of the texture we're rendering to (called the viewport)
    // This is also usually managed by the camera, but for now, we will just render to the whole texture
    D3D11_VIEWPORT viewport;
    memset( &viewport, 0, sizeof( viewport ) );
    viewport.TopLeftX = 0U;
    viewport.TopLeftY = 0U;
    viewport.Width = (float)colorTarget->GetWidth();
    viewport.Height = (float)colorTarget->GetHeight();
    viewport.MinDepth = 0.0f;        // must be between 0 and 1 (defualt is 0);
    viewport.MaxDepth = 1.0f;        // must be between 0 and 1 (default is 1)
    m_d3dContext->RSSetViewports( 1, &viewport );

    m_boundCamera->UpdateUBO( this );
    BindUBO( camera->GetUBO(), UBO_SLOT_CAMERA );
}


void RenderContext::EndCamera( const Camera* camera ) {
    if( m_boundCamera == camera ) {
        std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
        m_d3dContext->OMSetRenderTargets( 0, nullptr, nullptr );
        m_boundCamera = nullptr;
    } else {
        ERROR_RECOVERABLE( "(RenderContext) Ending camera that was not bound." );
    }
}


void RenderContext::DrawVertexArray( int numVertexes, const VertexPCU* vertexes, BlendMode mode /*= DRAW_MODE_ALPHA*/ ) {
    // VBO
    m_immediateVBO->CopyCPUToGPU( vertexes, numVertexes, sizeof(VertexPCU) );
    BindVertexStream( m_immediateVBO );

    // Shader & Blending
    if( m_boundShader == nullptr ) {
        BindShader( nullptr ); // Will set a default shader
    }

    m_boundShader->SetBlendMode( mode );
    BindShader( m_boundShader ); // Will set the draw mode

    BindModelMatrix();

    // Draw!
    Draw( numVertexes );
}


void RenderContext::DrawVertexArray( const VertexList& vertexes, BlendMode mode /*= DRAW_MODE_ALPHA*/ ) {
    int size = (int)vertexes.size();
    const VertexPCU* data = vertexes.data();
    DrawVertexArray( size, data, mode );
}


void RenderContext::DrawMesh( const GPUMesh* mesh, const Matrix44& modelMatrix /*= Matrix44::ZERO*/, BlendMode mode /*= BLEND_MODE_ALPHA */ ) {
    BindVertexStream( mesh->m_vertexBuffer );
    BindIndexStream( mesh->m_indexBuffer );

    // Model Matrix
    if( modelMatrix != Matrix44::ZERO ) {
        BindModelMatrix( modelMatrix );
    } // Else assume something was bound manually


    // Shader & Blending
    if( m_boundShader == nullptr ) {
        BindShader( nullptr ); // Will set a default shader
    }

    m_boundShader->SetBlendMode( mode );
    BindShader( m_boundShader ); // Will set the draw mode


    // Draw!
    Draw( mesh->GetElementCount(), 0, true );
}


void RenderContext::DrawModel( const Model* model ) {
    GPUMesh* mesh = model->GetMesh();

    if( mesh != nullptr ) {
        BindMaterial( model->GetMaterial() );
        DrawMesh( model->GetMesh(), model->GetModelMatrix() );
    }
}


const ID3D11Device* RenderContext::GetRenderDevice() const {
    return m_d3dDevice;
}


void RenderContext::SetD3DDebugName( ID3D11DeviceChild* child, const std::string& name ) {
    if( child != nullptr ) {
        child->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str() );
    }
}


void RenderContext::SetupImGuiInternals( ImGuiSystem* imGui ) const {
    imGui->m_d3dDevice = m_d3dDevice;
    imGui->m_d3dContext = m_d3dContext;
}


bool RenderContext::RequestScreenshot( EventArgs& args ) {
    JobSystem* jobSys = args.GetValue( "jobSystem", (JobSystem*)nullptr );

    if( jobSys == nullptr ) {
        std::string errorStr = "(RenderContext): Screenshot requires pointer to job system to run!";
        g_theDevConsole->PrintString( errorStr, m_consoleChannel | DevConsole::CHANNEL_ERROR );

        return false;
    }

    m_screenshotJobSys = jobSys;

    std::string filePath = args.GetValue( "filePath", "" ); // args doesn't support filesystem::path...
    m_screenshotRequested = filePath;                       // just copy it around
    return true;
}


void RenderContext::StartupD3D( void* windowHandleIn ) {
    std::scoped_lock< std::recursive_mutex > localLock( m_renderMutex );
    HWND windowHandle = (HWND)windowHandleIn;
    // Creation Flags
    unsigned int device_flags = 0;

    #if defined(_DEBUG)
        device_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    // Setup our Swap Chain
    DXGI_SWAP_CHAIN_DESC swap_desc;
    memset( &swap_desc, 0, sizeof( swap_desc ) );

    // fill the swap chain description struct
    swap_desc.BufferCount = 2;                                    // two buffers (one front, one back?)

    swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT       // how swap chain is to be used
        | DXGI_USAGE_BACK_BUFFER;
    swap_desc.OutputWindow = (HWND)windowHandle;                  // the window to be copied to on present
    swap_desc.SampleDesc.Count = 1;                               // how many multisamples (1 means no multi sampling)
                                                              
    RECT client_rect;
    GetClientRect( (HWND)windowHandle, &client_rect );
    unsigned int width = client_rect.right - client_rect.left;
    unsigned int height = client_rect.bottom - client_rect.top;

    // Default options.
    swap_desc.Windowed = TRUE;                                    // windowed/full-screen mode
    swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    swap_desc.BufferDesc.Width = width;
    swap_desc.BufferDesc.Height = height;

    // Actually Create
    HRESULT hr = D3D11CreateDeviceAndSwapChain( nullptr, // Adapter, if nullptr, will use adapter window is primarily on.
        D3D_DRIVER_TYPE_HARDWARE,  // Driver Type - We want to use the GPU (HARDWARE)
        nullptr,                   // Software Module - DLL that implements software mode (we do not use)
        device_flags,              // device creation options
        nullptr,                   // feature level (use default)
        0U,                        // number of feature levels to attempt
        D3D11_SDK_VERSION,         // SDK Version to use
        &swap_desc,                // Description of our swap chain
        &m_d3dSwapChain,           // Swap Chain we're creating
        &m_d3dDevice,              // [out] The device created
        nullptr,                   // [out] Feature Level Acquired
        &m_d3dContext              // Context that can issue commands on this pipe.
    );

    GUARANTEE_OR_DIE( SUCCEEDED( hr ), "(RenderContext) D3D11 Creation Failed" );
    StartupD3DDebug();
}


void RenderContext::StartupD3DDebug() {
    #if defined(_DEBUG)
        HRESULT queueResult = m_d3dDevice->QueryInterface( __uuidof(ID3D11InfoQueue), (void**)&m_d3dQueue );
        GUARANTEE_OR_DIE( SUCCEEDED( queueResult ), "ERROR: Failed to create D3D11 Info Queue" );
    #endif
}


void RenderContext::PrintD3DDebug() {
    #if defined(_DEBUG)
    UINT64 numMessages = m_d3dQueue->GetNumStoredMessages();
    Strings messages;
    messages.reserve( numMessages );

    for( int messageIndex = 0; messageIndex < numMessages; messageIndex++ ) {
        SIZE_T messageLength = 0;
        HRESULT lengthResult = m_d3dQueue->GetMessage( messageIndex, NULL, &messageLength );

        if( SUCCEEDED( lengthResult ) ) {
            D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc( messageLength );

            HRESULT messageResult = m_d3dQueue->GetMessage( messageIndex, message, &messageLength );

            if( SUCCEEDED( messageResult ) ) {
                std::string messageText = message->pDescription;
                int numPrinted = (int)m_d3dPrintedWarnings.size();

                if( numPrinted == 0 ) {
                    messages.push_back( messageText );
                    m_d3dPrintedWarnings.push_back( messageText );
                } else {
                    for( int msgIndex = 0; msgIndex < numPrinted; msgIndex++ ) {
                        std::string printed = m_d3dPrintedWarnings[msgIndex];

                        if( printed == messageText ) { // Already printed
                            break;
                        } else if( msgIndex == numPrinted - 1 ) { // Has not been printed
                            messages.push_back( messageText );
                            m_d3dPrintedWarnings.push_back( messageText );
                        }
                    }
                }
            }

            free( message );
        }
    }

    m_d3dQueue->ClearStoredMessages();
    int numWarnings = (int)messages.size();

    if( numWarnings > 0 ) {
        for( int messageIndex = 0; messageIndex < numWarnings; messageIndex++ ) {
            std::string messageText = messages[messageIndex];
            g_theDevConsole->PrintString( messageText, m_consoleChannel | DevConsole::CHANNEL_ERROR );
        }
    }
    #endif
}


void RenderContext::ShutdownD3DDebug() const {
    if( m_d3dDevice == nullptr ) {
        return;
    }

    #if defined(_DEBUG)
    ID3D11Debug* debug = nullptr;
    HRESULT debugResult = m_d3dDevice->QueryInterface( __uuidof(ID3D11Debug), (void**)&debug );

    if( SUCCEEDED(debugResult) ) {
        m_d3dQueue->SetMuteDebugOutput( true );
        debug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

        UINT64 numMessages = m_d3dQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
        Strings messages;
        messages.reserve( numMessages );

        for( int messageIndex = 0; messageIndex < numMessages; messageIndex++ ) {
            SIZE_T messageLength = 0;
            HRESULT lengthResult = m_d3dQueue->GetMessage( messageIndex, NULL, &messageLength );

            if( SUCCEEDED( lengthResult ) ) {
                D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc( messageLength );

                HRESULT messageResult = m_d3dQueue->GetMessage( messageIndex, message, &messageLength );

                if( SUCCEEDED( messageResult ) ) {
                    std::string messageText = message->pDescription;
                    std::regex hasRefCount( ".*Refcount: [1-9][0-9]*.*" );
                    std::regex isExpected( ".*(ID3D11(Context|Device)|IDXGISwapChain).*" );

                    if( regex_match( messageText, hasRefCount ) && !regex_match( messageText, isExpected ) ) {
                        // Has a RefCount > 0, and is something besides a Context, Device, or SwapChain
                        messages.push_back( messageText );
                    }
                }

                free( message );
            }
        }

        int numWarnings = (int)messages.size();

        if( numWarnings > 0 ) {
            DebuggerPrintf( "\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n" );
            DebuggerPrintf( "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=D3D11 WARNING=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n" );
            for( int messageIndex = 0; messageIndex < numWarnings; messageIndex++ ) {
                std::string messageText = messages[messageIndex];
                DebuggerPrintf( Stringf( "D3D11 WARNING: %s\n", messageText.c_str() ).c_str() );
            }
            DebuggerPrintf( "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n" );
        }

        // Without calling this, the debug output remains OFF... Seems ok since most of it was useless
        //queue->SetMuteDebugOutput( false );
    }

    DX_SAFE_RELEASE( debug );
    #endif
}


void RenderContext::BindVertexStream( VertexBuffer* vbo ) {
    // Bind the input stream; 
    UINT stride = (UINT)vbo->GetStride();
    UINT byteOffset = 0;
    ID3D11Buffer* buffer = nullptr;

    if( vbo != nullptr ) {
        buffer = vbo->GetBufferHandle();
    }

    m_d3dContext->IASetVertexBuffers( 0, 1, &buffer, &stride, &byteOffset );
}


void RenderContext::BindIndexStream( IndexBuffer* ibo ) {
    ID3D11Buffer *buffer = nullptr;

    if( ibo != nullptr ) {
        buffer = ibo->GetBufferHandle();
    }

    m_d3dContext->IASetIndexBuffer( buffer, DXGI_FORMAT_R32_UINT, 0 );
}


void RenderContext::Draw( int numElements, int byteOffset /*= 0 */, bool useIndexBuffer /*= false */ ) {
    // Shader Programs we compiled earlier
    ID3D11VertexShader* vertexShader = (ID3D11VertexShader*)(m_boundShader->m_vertexShader->m_resource);
    ID3D11PixelShader* pixelShader = (ID3D11PixelShader*)(m_boundShader->m_pixelShader->m_resource);

    m_d3dContext->VSSetShader( vertexShader, nullptr, 0U );
    m_d3dContext->PSSetShader( pixelShader, nullptr, 0U );

    // Topology Type
    m_d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Light UBO
    if( m_lightBufferDirty ) {
        m_lightUBO->CopyCPUToGPU( &m_lightBuffer, sizeof( LightBuffer ) );
        BindUBO( m_lightUBO, UBO_SLOT_LIGHTING );
        m_lightBufferDirty = false;
    }

    // Input Layout
    m_d3dContext->IASetInputLayout( m_boundShader->m_inputLayout );

    // Draw
    if( useIndexBuffer ) {
        m_d3dContext->DrawIndexed( numElements, byteOffset, 0 );
    } else {
        m_d3dContext->Draw( numElements, byteOffset );
    }
}


Material* RenderContext::CreateMaterial( const std::string& filePath ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );

    tinyxml2::XMLDocument materialDoc = new tinyxml2::XMLDocument();
    tinyxml2::XMLError loadSuccess = materialDoc.LoadFile( filePath.c_str() );
    //GUARANTEE_OR_DIE( loadSuccess == tinyxml2::XML_SUCCESS, Stringf( "Failed to load XML file: %s", filePath.c_str() ) );

    if( loadSuccess != tinyxml2::XML_SUCCESS ) { // Failed to load, assume they wanted an empty one to construct from code
        Material* material = new Material( this );
        m_loadedMaterials.insert( { filePath, material } );
        return material;
    }

    XMLElement* root = materialDoc.FirstChildElement( "Materials" );
    GUARANTEE_OR_DIE( root != nullptr, Stringf( "ERROR: Missing materials root element for XML file: %s", filePath.c_str() ) );

    XMLElement* matEle = root->FirstChildElement( "Material" );
    Material* material = nullptr;

    while( matEle != nullptr ) {
        material = new Material( this, matEle );
        std::string id = Stringf( "%s:%s", filePath.c_str(), material->GetID().c_str() );
        m_loadedMaterials.insert( { id, material } );

        matEle = matEle->NextSiblingElement( "Material" );
    }

    return material;
}


CPUMesh* RenderContext::CreateMesh( const std::string& filePath ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );

    // Parse XML
    std::string objFile = "";
    bool invert = false;
    float scale = 1.f;
    std::string transform = "";
    std::string materialName = "";

    GetMeshXMLParams( filePath, objFile, invert, scale, transform, materialName );

    // Apply parameters
    CPUMesh* mesh = new CPUMesh();
    mesh->LoadObjFile( objFile, invert, scale, transform );
    mesh->SetDefaultMaterial( materialName );


    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new mesh from file (%s)", filePath.c_str() ), m_consoleChannel );
    m_loadedMeshes.insert( { filePath, mesh } );

    return mesh;
}


TextureView2D* RenderContext::CreateTextureView2D( const std::string& filePath ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );
    Texture2D* texture = Texture2D::MakeNewImageTexture( m_d3dDevice, filePath );
    TextureView2D* textureView = new TextureView2D( m_d3dDevice, texture );

    delete texture;
    m_loadedTextures.insert( { filePath, textureView } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new texture view from file (%s)", filePath.c_str() ), m_consoleChannel );
    return textureView;
}


TextureView2D* RenderContext::CreateTextureView2D( Image* existingImage ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );
    Texture2D* texture = Texture2D::MakeNewImageTexture( m_d3dDevice, existingImage );
    TextureView2D* textureView = new TextureView2D( m_d3dDevice, texture );

    delete texture;
    std::string filePath = existingImage->GetImageFilePath();
    m_loadedTextures.insert( { filePath, textureView } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new texture view from file (%s)", filePath.c_str() ), m_consoleChannel );
    return textureView;
}


TextureView2D* RenderContext::CreateTextureView2D( const Rgba& textureColor ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );
    Texture2D* texture = Texture2D::MakeNewImageTexture( m_d3dDevice, textureColor );
    TextureView2D* textureView = new TextureView2D( m_d3dDevice, texture );

    delete texture;
    std::string colorStr = textureColor.GetAsString();
    m_loadedTextures.insert( { colorStr, textureView } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new texture view from file (%s)", colorStr.c_str() ), m_consoleChannel );
    return textureView;
}


TextureView2D* RenderContext::CreateRenderTarget( const std::string& targetID, const IntVec2& dimensions /*= IntVec2::NEGONE */ ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );
    IntVec2 textureDims = dimensions;

    if( dimensions == IntVec2::NEGONE ) {
        textureDims = GetBackBufferDimensions();
    }

    Texture2D* texture = Texture2D::MakeNewRenderTexture( m_d3dDevice, textureDims );
    TextureView2D* newTarget = new TextureView2D( m_d3dDevice, texture );

    delete texture;
    m_loadedTextures.insert( { targetID, newTarget } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new render target with ID (%s)", targetID.c_str() ), m_consoleChannel );
    return newTarget;
}


TextureView2D* RenderContext::CreateDepthTarget( const std::string& targetID, const IntVec2& dimensions /*= IntVec2::NEGONE */ ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );
    IntVec2 textureDims = dimensions;

    if( dimensions == IntVec2::NEGONE ) {
        textureDims = GetBackBufferDimensions();
    }

    Texture2D* texture = Texture2D::MakeNewDepthTexture( m_d3dDevice, textureDims );
    TextureView2D* newTarget = new TextureView2D( m_d3dDevice, texture );

    delete texture;
    m_loadedTextures.insert( { targetID, newTarget } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new depth target with ID (%s)", targetID.c_str() ), m_consoleChannel );
    return newTarget;
}


BitmapFont* RenderContext::CreateBitmapFont( const std::string& fontName ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );
    std::string imageFilePath = Stringf( "Data/Fonts/%s.png", fontName.c_str() );
    CreateTexture( imageFilePath.c_str() );

    BitmapFont* font = new BitmapFont( fontName, imageFilePath.c_str() );
    m_loadedFonts.insert( { fontName, font } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new font from file (%s)...", fontName.c_str() ), m_consoleChannel );
    return font;
}


Shader* RenderContext::CreateShader( const std::string& shaderFilePath ) {
    std::scoped_lock< std::recursive_mutex > writeLock( m_renderMutex );

    Shader* newShader = new Shader( m_d3dDevice, shaderFilePath );
    newShader->SetDepthMode( COMPARE_LESS_EQUAL, true );
    m_loadedShaders.insert( { shaderFilePath, newShader } );

    g_theDevConsole->PrintString( Stringf( "(Renderer) Loaded new shader from file (%s)...", shaderFilePath.c_str() ), m_consoleChannel );
    return newShader;
}


const char* RenderContext::DEFAULT_SHADER_UNLIT = R"(
//--------------------------------------------------------------------------------------
// Vertex Input - currently using a built in vertex index
struct VertexShaderInput {
    float3 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : UV;
};


struct VertexToFragment {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
};


//--------------------------------------------------------------------------------------
// Uniform Input
// ------
// Uniform Data is also externally provided data, but instead of changing
// per vertex call, it is constant for all vertices, hence the name "Constant Buffer"
// or "Uniform Buffer".  This is read-only memory; 
//
// I tend to use all cap naming here, as it is effectively a 
// constant from the shader's perspective. 
//
// register(b2) determines the buffer unit/slot to use.  In this case
// we'll say this data is coming from buffer slot 2. 
//
// Last - the layout needs to match the CPU side memory layout, with some 
// caveats.
// Objects MUST start on either a 4-byte, 8-byte, or 16-byte boundary (or multiple of 16).
// Members must be on their own byte alignment, or the next larges (so float2 is on 8-byte alignment
// float3 is on 16 (since there is no 12-byte alignment)
//--------------------------------------------------------------------------------------
cbuffer FrameBuffer : register(b1) {
    float FRAME_COUNT;
    float GAME_TIME;
    float COS_TIME;
    float SIN_TIME;
}


cbuffer CameraConstants : register(b2) {
    float4x4 VIEW;
    float4x4 PROJECTION;
};

cbuffer ModelConstants : register(b3) {
    float4x4 MODEL;
};

cbuffer DebugDrawConstants : register(b4) {
    float4 TINT;
};

//cbuffer LightConstants : register(b5)


Texture2D<float4> tAlbedo : register(t0); // Texutre used for albedo (color) information
SamplerState sAlbedo : register(s0);      // Sampler used for the Albedo texture


//--------------------------------------------------------------------------------------
// Static Constants - i.e. built into the shader
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex Shader
VertexToFragment VertexFunction( VertexShaderInput input ) {
    // Only outputs in the following ranges will display..
    // [-1, 1] on the x (left to right)
    // [-1, 1] on the y (bottom to top)
    // [ 0, 1] on the z 

    float4 localPos = float4( input.position, 1.f );
    float4 worldPos = mul( MODEL, localPos );
    float4 viewPos = mul( VIEW, worldPos );
    float4 clipPos = mul( PROJECTION, viewPos );

    VertexToFragment v2f = (VertexToFragment)0;
    v2f.position = clipPos;
    v2f.color = input.color;
    v2f.uv = input.uv;

    return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned is being drawn to the first bound color target.
float4 FragmentFunction( VertexToFragment input ) : SV_Target0 {
    float4 texColor = tAlbedo.Sample( sAlbedo, input.uv );
    float4 finalColor = texColor * input.color * TINT; // component wise multiply to "tint" the output

    return finalColor;
}
)";


const char* RenderContext::DEFAULT_SHADER_LIT = R"(
//--------------------------------------------------------------------------------------
// Static Constants - i.e. built into the shader
//--------------------------------------------------------------------------------------
#define MAX_LIGHTS (8)
#define GAMMA (2.2f)


//--------------------------------------------------------------------------------------
// Vertex Input - currently using a built in vertex index
struct VertexShaderInput {
    float3 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : UV;
    float3 tangent       : TANGENT;
    float3 bitangent     : BITANGENT;
    float3 normal        : NORMAL;
};


struct VertexToFragment {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POS;
};


struct LightDesc {
    float3 color;
    float intensity;

    float3 position;
    float isDirectional;
    
    float3 direction;
    //float padding;

    float3 diffuseAttenuation;
    //float padding;

    float3 specularAttenuation;
    //float padding;
};


struct VertexLighting {
    float3 diffuse;
    //float padding;

    float3 specular;
    //float padding;
};


//--------------------------------------------------------------------------------------
// Uniform Input
// ------
// Uniform Data is also externally provided data, but instead of changing
// per vertex call, it is constant for all vertices, hence the name "Constant Buffer"
// or "Uniform Buffer".  This is read-only memory; 
//
// I tend to use all cap naming here, as it is effectively a 
// constant from the shader's perspective. 
//
// register(b2) determines the buffer unit/slot to use.  In this case
// we'll say this data is coming from buffer slot 2. 
//
// Last - the layout needs to match the CPU side memory layout, with some 
// caveats.
// Objects MUST start on either a 4-byte, 8-byte, or 16-byte boundary (or multiple of 16).
// Members must be on their own byte alignment, or the next larges (so float2 is on 8-byte alignment
// float3 is on 16 (since there is no 12-byte alignment)
//--------------------------------------------------------------------------------------
cbuffer FrameBuffer : register(b1) {
    float FRAME_COUNT;
    float GAME_TIME;
    float COS_TIME;
    float SIN_TIME;
}


cbuffer CameraConstants : register(b2) {
    float4x4 VIEW;
    float4x4 PROJECTION;

    float3 CAMERA_POSITION;
    //float PADDING;
};


cbuffer ModelConstants : register(b3) {
    float4x4 MODEL;
};


//cbuffer DebugDrawConstants : register(b4)

cbuffer LightConstants : register(b5) {
    float4 AMBIENT;
    float4 EMISSIVE;

    float SPECULAR_FACTOR;
    float SPECULAR_POWER;
    float RENDER_MODE;
    float PADDING;
    
    LightDesc LIGHTS[MAX_LIGHTS];
};

SamplerState sAlbedo        : register(s0); // Sampler used for the textures
Texture2D<float4> tColor    : register(t0); // Texutre used for albedo (color) information
Texture2D<float4> tNormal   : register(t1); // Texutre used for normal information
Texture2D<float4> tSpecular : register(t2); // Texutre used for specular information
Texture2D<float4> tEmissive : register(t3); // Texutre used for emissive information

//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------
VertexLighting GetLighting( float3 eyePosition, float3 surfacePosition, float3 surfaceNormal ) {
    VertexLighting lighting;

    float3 surfaceToCamera = normalize(CAMERA_POSITION - surfacePosition);

    lighting.diffuse = (AMBIENT.xyz * AMBIENT.w); //float3 of color, float of intensity.. does not affect transparency
    lighting.specular = float3( 0.f, 0.f, 0.f );

    for( int lightIndex = 0; lightIndex < MAX_LIGHTS; lightIndex++ ) {
        LightDesc light = LIGHTS[lightIndex];

        // Get Direction
        //float3 directionalDir = normalize(light.direction);
        float3 directionalDir = light.direction;

        float3 lightToSurface = surfacePosition - light.position;
        float3 pointDir = normalize( lightToSurface );

        float3 lightDir = lerp( pointDir, directionalDir, light.isDirectional );


        // Get Distance
        float directionalDist = abs( dot( lightToSurface, lightDir ) );
        float pointDist = length( lightToSurface );
        float lightDist = lerp( pointDist, directionalDist, light.isDirectional );


        // Diffuse
        float3 dAtten = light.diffuseAttenuation;
        float attenuation = 1.f / (dAtten.x + (dAtten.y * lightDist) + (dAtten.z * lightDist * lightDist));
        attenuation = (isinf(attenuation) ? 0.f : attenuation);

        float dot3 = max( dot( -lightDir, surfaceNormal ), 0.f );

        float3 diffuseColor = light.color * light.intensity * attenuation * dot3;
        lighting.diffuse += diffuseColor;


        // Specular
        // Blinn-Phong 
        // dot( H, N );  -> H == half_vector, N == normal
        float3 halfVec = normalize( surfaceToCamera + -lightDir ); 
        float specCoefficient = max( dot( halfVec, surfaceNormal ), 0.0f ); // DO not saturate - spec can go higher;  
        specCoefficient = SPECULAR_FACTOR * pow( abs(specCoefficient), SPECULAR_POWER ); 

        float3 sAtten = light.specularAttenuation; 
        float sAttenuation = 1.f / (sAtten.x + (sAtten.y * lightDist) + (sAtten.z * lightDist * lightDist));
        sAttenuation = (isinf(sAttenuation) ? 0.f : sAttenuation);

        float3 specColor = light.color * light.intensity * sAttenuation * specCoefficient;
        lighting.specular += specColor; 
    }

    lighting.diffuse = saturate( lighting.diffuse );
    return lighting;
}


//--------------------------------------------------------------------------------------
// Vertex Shader
VertexToFragment VertexFunction( VertexShaderInput input ) {
    // Only outputs in the following ranges will display..
    // [-1, 1] on the x (left to right)
    // [-1, 1] on the y (bottom to top)
    // [ 0, 1] on the z 

    float4 localPos = float4( input.position, 1.f );
    float4 worldPos = mul( MODEL, localPos );
    float4 viewPos = mul( VIEW, worldPos );
    float4 clipPos = mul( PROJECTION, viewPos );

    VertexToFragment v2f = (VertexToFragment)0;
    v2f.position = clipPos;
    v2f.color = input.color;
    v2f.uv = input.uv;

    float4 localTangent   = float4( input.tangent, 0.f );
    float4 worldTangent   = mul( MODEL, localTangent );

    float4 localBitangent = float4( input.bitangent, 0.f );
    float4 worldBitangent = mul( MODEL, localBitangent );

    float4 localNormal    = float4( input.normal, 0.f );
    float4 worldNormal    = mul( MODEL, localNormal );

    v2f.tangent   = worldTangent.xyz;
    v2f.bitangent = worldBitangent.xyz;
    v2f.normal    = worldNormal.xyz;
    v2f.worldPos  = worldPos.xyz;

    return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned is being drawn to the first bound color target.
float4 FragmentFunction( VertexToFragment input ) : SV_Target0 {
    input.tangent = normalize( input.tangent );
    input.bitangent = normalize( input.bitangent );
    input.normal = normalize( input.normal );
    float4 texColor = tColor.Sample( sAlbedo, input.uv );

    float4 linearColor = pow( abs(texColor), GAMMA );
    float4 tintedColor = linearColor * input.color; // component wise multiply to "tint" the output

    // Sample Normal Map
    float3x3 surfaceToWorld = float3x3( normalize(input.tangent), normalize(input.bitangent), normalize(input.normal) );

    float4 normalColor = tNormal.Sample( sAlbedo, input.uv );
    float3 surfaceNormal = normalColor.xyz * float3( 2.f, 2.f, 1.f ) - float3( 1.f, 1.f, 0.f );
    float3 worldSurfaceNormal = mul( surfaceNormal, surfaceToWorld );

    // Adjust for Lighting
    VertexLighting lighting = GetLighting( CAMERA_POSITION, input.worldPos, worldSurfaceNormal );
    float4 litColor = tintedColor * float4( lighting.diffuse, 1.f) + float4( lighting.specular, 0.f);

    float4 emissiveColor = tEmissive.Sample( sAlbedo, input.uv ) * EMISSIVE;
    litColor += float4( emissiveColor.xyz * emissiveColor.w, 0.f );

    float4 finalLit = pow( abs(litColor), 1.f / GAMMA );


    // Render Tangents as Color
    float4 worldTangent = float4( input.tangent, 0.f );
    worldTangent.xyz += float3( 1.f, 1.f, 1.f );
    worldTangent *= 0.5f; // Range Map from -1 1 to 0 1

    worldTangent.w = 1.f;

    // Render Bitangents as Color
    float4 worldBitangent = float4( input.bitangent, 0.f );
    worldBitangent.xyz += float3( 1.f, 1.f, 1.f );
    worldBitangent *= 0.5f; // Range Map from -1 1 to 0 1

    worldBitangent.w = 1.f;

    // Render Normals as Color
    float4 worldNormal = float4( input.normal, 0.f );
    worldNormal.xyz += float3( 1.f, 1.f, 1.f );
    worldNormal *= 0.5f; // Range Map from -1 1 to 0 1

    worldNormal.w = 1.f;

    // Render Surface Normals as Color
    float4 worldSurfaceNormal4 = float4( worldSurfaceNormal, 0.f );
    worldSurfaceNormal4.xyz += float3( 1.f, 1.f, 1.f );
    worldSurfaceNormal4 *= 0.5f; // Range Map from -1 1 to 0 1

    worldSurfaceNormal4.w = 1.f;


    // Decide between Lighting, Normals, Surface Normals, Tangents, Bitangents
    float normalLerp = saturate( RENDER_MODE );
    float4 lightOrNormal = lerp( finalLit, worldNormal, normalLerp );

    float surfaceLerp = saturate( RENDER_MODE -1.f );
    float4 normalOrSurface = lerp( lightOrNormal, worldSurfaceNormal4, surfaceLerp );

    float4 tangentLerp = saturate( RENDER_MODE - 2.f );
    float4 normalOrTangent = lerp( normalOrSurface, worldTangent, tangentLerp );

    float4 bitangentLerp = saturate( RENDER_MODE - 3.f );
    float4 tangentOrBitangent = lerp( normalOrTangent, worldBitangent, bitangentLerp );

    return tangentOrBitangent;
}
)";


const char* RenderContext::DEFAULT_SHADER_INVALID = R"(
//--------------------------------------------------------------------------------------
// Vertex Input - currently using a built in vertex index
struct VertexShaderInput {
    float4 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : UV;
};


struct VertexToFragment {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
};


//--------------------------------------------------------------------------------------
// Uniform Input
// ------
// Uniform Data is also externally provided data, but instead of changing
// per vertex call, it is constant for all vertices, hence the name "Constant Buffer"
// or "Uniform Buffer".  This is read-only memory; 
//
// I tend to use all cap naming here, as it is effectively a 
// constant from the shader's perspective. 
//
// register(b2) determines the buffer unit/slot to use.  In this case
// we'll say this data is coming from buffer slot 2. 
//
// Last - the layout needs to match the CPU side memory layout, with some 
// caveats.
// Objects MUST start on either a 4-byte, 8-byte, or 16-byte boundary (or multiple of 16).
// Members must be on their own byte alignment, or the next larges (so float2 is on 8-byte alignment
// float3 is on 16 (since there is no 12-byte alignment)
//--------------------------------------------------------------------------------------
cbuffer FrameBuffer : register(b1) {
    float FRAME_COUNT;
    float GAME_TIME;
    float COS_TIME;
    float SIN_TIME;
}


cbuffer CameraConstants : register(b2) {
    float4x4 VIEW;
    float4x4 PROJECTION;
};

cbuffer ModelConstants : register(b3) {
    float4x4 MODEL;
};


Texture2D<float4> tAlbedo : register(t0); // Texutre used for albedo (color) information
SamplerState sAlbedo : register(s0);      // Sampler used for the Albedo texture


//--------------------------------------------------------------------------------------
// Static Constants - i.e. built into the shader
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex Shader
VertexToFragment VertexFunction( VertexShaderInput input ) {
    // Only outputs in the following ranges will display..
    // [-1, 1] on the x (left to right)
    // [-1, 1] on the y (bottom to top)
    // [ 0, 1] on the z 

    // (note: technically clip_space is a homogeneous coordinate
    //  so the above is not 100% accurate, but more on that later)
    float4 localPos = input.position;
    float4 worldPos = mul( MODEL, localPos );
    float4 viewPos = mul( VIEW, worldPos );
    float4 clipPos = mul( PROJECTION, viewPos );

    // For now, we'll just set w to 1, and forward.
    VertexToFragment v2f = (VertexToFragment)0;
    v2f.position = clipPos;
    v2f.color = float4( 1.f, 0.f, 1.f, 1.f ); // MAGENTA
    v2f.uv = float2( 0.f, 0.f );

    return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned is being drawn to the first bound color target.
float4 FragmentFunction( VertexToFragment input ) : SV_Target0 {
    float4 magenta =  float4( 1.f, 0.f, 1.f, 1.f );
    float4 yellow  =  float4( 1.f, 1.f, 0.f, 1.f );

    bool onCheckerX = (input.position.x % 100) < 50;
    bool onCheckerY = (input.position.y % 100) < 50;

    return ((onCheckerX && onCheckerY) || (!onCheckerX && !onCheckerY)) ? magenta : yellow;
}
)";
