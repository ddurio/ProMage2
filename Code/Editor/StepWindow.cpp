#if defined(_EDITOR)
#include "Editor/StepWindow.hpp"

#include "Editor/MapWindow.hpp"

#include "Engine/Core/ImGuiSystem.hpp"


StepWindow::StepWindow( const MapWindow& mapEditor, const Vec2& normDimensions /*= Vec2( 0.8f, 0.1f )*/, const Vec2& alignment /*= Vec2::ZERO */ ) :
    EditorWindow( normDimensions, alignment ),
    m_mapEditor( mapEditor ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "StepEditor";
}


StepWindow::~StepWindow() {

}


void StepWindow::UpdateChild( float deltaSeconds ) {
    Strings stepNames = m_mapEditor.GetStepNames();

    int numSteps = (int)stepNames.size();
    ImGui::BeginChild( "scrollingSteps", ImVec2( 0.f, 0.f ), false, ImGuiWindowFlags_HorizontalScrollbar );

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        const std::string& stepName = stepNames[stepIndex];

        if( ImGui::Button( stepName.c_str() ) ) {
            // Button click
            EventArgs args;
            args.SetValue( "stepIndex", stepIndex );

            g_theEventSystem->FireEvent( EVENT_EDITOR_STEP_INDEX, args );
        }

        ImGui::SameLine();
    }

    ImGui::EndChild();
}

#endif
