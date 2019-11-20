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

    ImGui::SetColumnWidth( 0, 0.05f * contentDims.x );
    ImGui::SetColumnWidth( 1, 0.9f  * contentDims.x );
    ImGui::SetColumnWidth( 2, 0.05f * contentDims.x );

    // put a left button
    ImGui::NextColumn();

    const std::string& imageName = m_typeNames[m_typeIndex];
    TextureView2D* helpImage = g_theRenderer->GetOrCreateTextureView2D( imageName );
    void* shaderResource = helpImage->GetShaderView();

    Vec2 imageDims = 0.9f * contentDims;

    ImGui::Image( shaderResource, imageDims.GetAsImGui() );
    ImGui::NextColumn();

    // put a right button
    ImGui::NextColumn();

    ImGui::Columns( 1 );
    return false;
}
