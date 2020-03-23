#include "Engine/Core/ImGuiSystem.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "dxguid.lib" )

#include "Engine/Core/WindowContext.hpp"
#include "Engine/Debug/Profiler.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"


ImGuiSystem* g_theGui = new ImGuiSystem();


ImGuiSystem::ImGuiSystem( RenderContext* renderer /*= nullptr*/, WindowContext* window /*= nullptr */ ) {
    Setup( renderer, window );
}


ImGuiSystem::~ImGuiSystem() {

}


void ImGuiSystem::Startup( RenderContext* renderer /*= nullptr*/, WindowContext* window /*= nullptr */ ) {
    Setup( renderer, window );
    CheckSetup();

    m_renderer->SetupImGuiInternals( this );

    // Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup IO
    ImGuiIO& io = ImGui::GetIO();
    UNUSED( io );

    // Render device / context
    ImGui_ImplWin32_Init( m_window->GetWindowHandle() );
    ImGui_ImplDX11_Init( m_d3dDevice, m_d3dContext );
    ImGui::StyleColorsDark();
}


void ImGuiSystem::Shutdown() {
    m_isSetup = false;
    m_renderer = nullptr;
    m_window = nullptr;

    m_d3dContext = nullptr;
    m_d3dDevice = nullptr;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


void ImGuiSystem::BeginFrame() {
    PROFILE_FUNCTION();
    CheckSetup();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
}


void ImGuiSystem::EndFrame() {

}


void ImGuiSystem::Render() const {
    CheckSetup();

    ImGui::Render();
    ImDrawData* data = ImGui::GetDrawData();

    ID3D11RenderTargetView* rtv = m_renderer->GetCurrentRenderTarget()->GetRenderView();
    m_d3dContext->OMSetRenderTargets( 1, &rtv, NULL );

    ImGui_ImplDX11_RenderDrawData( data );
}


Vec2 ImGuiSystem::CreateGuiWindow( const Vec2& normDimensions, const Vec2& alignment, const char* name, ImGuiWindowFlags windowFlags, bool* windowIsOpen /*= nullptr */ ) const {
    // Setup Window
    AABB2 clientBounds = m_window->GetClientBoundsInverted();
    Vec2 clientDimensions = clientBounds.GetDimensions();

    Vec2 windowSize = normDimensions * clientDimensions;
    AABB2 windowBounds = clientBounds.GetBoxWithin( windowSize, alignment );
    Vec2 windowOrigin = Vec2( windowBounds.mins.x, -windowBounds.maxs.y );

    ImGui::SetNextWindowPos( ImVec2( windowOrigin.x, windowOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( windowSize.x, windowSize.y ), ImGuiCond_Always );
    ImGui::Begin( name, windowIsOpen, windowFlags );

    return windowSize;
}


Vec2 ImGuiSystem::CreateStaticWindow( const Vec2& normDimensions, const Vec2& alignment, const char* name /*= ""*/, ImGuiWindowFlags extraFlags /*= ImGuiWindowFlags_None*/, bool* windowIsOpen /*= nullptr */ ) const {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoSavedSettings
        | extraFlags;

    return CreateGuiWindow( normDimensions, alignment, name, windowFlags, windowIsOpen );
}


bool ImGuiSystem::IsSetup() const {
    if( this == nullptr || !m_isSetup ) {
        return false;
    }

    return true;
}


void ImGuiSystem::Setup( RenderContext* renderer, WindowContext* window ) {
    if( m_renderer == nullptr ) {
        m_renderer = renderer;
    }

    if( m_window == nullptr ) {
        m_window = window;
    }

    if( m_renderer != nullptr && m_window != nullptr ) {
        m_isSetup = true;
    }
}


void ImGuiSystem::CheckSetup() const {
    if( !m_isSetup ) {
        ERROR_AND_DIE( "(ImGuiSystem): RenderContext and WindowContext must both be set" );
    }
}

