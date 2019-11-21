#include "Editor/HelpWindow.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "ThirdParty/DearImGui/imgui_internal.h"


HelpWindow::HelpWindow() :
    EditorWindow( Vec2( 1.f, 0.98f ), Vec2::ZERO ) {
    m_windowName = "Step Type Descriptions";
    m_useCloseButton = true;
}


HelpWindow::~HelpWindow() {

}


bool HelpWindow::IsOpen() const {
    return m_windowIsOpen;
}


bool HelpWindow::UpdateChild( float deltaSeconds ) {
    UNUSED( deltaSeconds );
    ImGui::Columns( 3, nullptr, false, ImGuiColumnsFlags_NoResize ); // ThesisFIXME: Potentially dangerous.. modified imGui to expose this flag

    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
    Vec2 contentDims = Vec2( contentMax ) - Vec2( contentMin );
    Vec2 imageDims = 0.9f * contentDims;

    ImGui::SetColumnWidth( 0, 0.05f * contentDims.x );
    ImGui::SetColumnWidth( 1, imageDims.x );
    ImGui::SetColumnWidth( 2, 0.05f * contentDims.x );

    // Ignore first column
    ImGui::NextColumn();

    // Middle column with all the stuff
    const std::string& imageName = m_typeNames[m_typeIndex];
    TextureView2D* helpImage = g_theRenderer->GetOrCreateTextureView2D( imageName );
    void* shaderResource = helpImage->GetShaderView();

    ImGui::Image( shaderResource, imageDims.GetAsImGui() );

    // Controls
    TextureView2D* leftArrow = g_theRenderer->GetOrCreateTextureView2D( TEXTURE_EDITOR_MEDIA_LEFT );
    TextureView2D* rightArrow = g_theRenderer->GetOrCreateTextureView2D( TEXTURE_EDITOR_MEDIA_RIGHT );

    void* leftResource  = leftArrow->GetShaderView();
    void* rightResource = rightArrow->GetShaderView();

    if( ImGui::ImageButton( leftResource, ImVec2( 50.f, 50.f ) ) ) {
        if( m_typeIndex == 0 ) {
            m_typeIndex = (int)m_typeNames.size();
        }

        m_typeIndex--;
    }

    ImGui::SameLine();

    if( ImGui::ImageButton( rightResource, ImVec2( 50.f, 50.f ) ) ) {
        m_typeIndex++;
        m_typeIndex = m_typeIndex % m_typeNames.size();
    }

    ImVec2 buttonSize = ImGui::GetItemRectSize(); // Requested 50, but could be different because of padding
    ImGui::SameLine();

    if( ImGui::Button( "Close", buttonSize ) ) {
        m_windowIsOpen = false;
    }


    ImGui::NextColumn();

    // Ignore last column
    ImGui::NextColumn();

    ImGui::Columns( 1 );
    return false;
}
