#if defined(_EDITOR)
#include "Editor/XmlWindow.hpp"

#include "Editor/Editor.hpp"
#include "Editor/EditorMapDef.hpp"
#include "Editor/EditorMapGenStep.hpp"
#include "Editor/MapWindow.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ImGuiSystem.hpp"


XmlWindow::XmlWindow( const Vec2& normDimensions /*= Vec2( 0.2f, 1.f )*/, const Vec2& alignment /*= Vec( 0.8f, 0.f ) */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "XmlEditor";
}


XmlWindow::~XmlWindow() {

}


bool XmlWindow::ShouldHighlightTiles() const {
    return m_highlightChanges;
}


void XmlWindow::UpdateChild( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    // General settings
    if( ImGui::CollapsingHeader( "General Options", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        if( ImGui::Button( "Regenerate Map", ImVec2( -1, 0 ) ) ) {
            EventArgs args;
            g_theEventSystem->FireEvent( EVENT_EDITOR_GENERATE_MAP, args );
        }

        ImGui::Checkbox( "Highlight Modified Tiles", &m_highlightChanges );
    }

    // Step params
    MapWindow* mapWindow = g_theEditor->GetMapWindow();

    std::string mapType = mapWindow->GetMapType();
    const EditorMapDef* eMapDef = EditorMapDef::GetDefinition( mapType );

    int stepIndex = mapWindow->GetStepIndex();
    MapGenStep* genStep = eMapDef->GetStep( stepIndex );

    EditorMapGenStep::RenderStepParms( genStep );
}

#endif
