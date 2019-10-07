#if defined(_EDITOR)
#include "Editor/StepWindow.hpp"

#include "Editor/Editor.hpp"
#include "Editor/MapWindow.hpp"

#include "Engine/Core/ImGuiSystem.hpp"


StepWindow::StepWindow( const Vec2& normDimensions /*= Vec2( 0.8f, 0.1f )*/, const Vec2& alignment /*= Vec2::ZERO */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "StepEditor";
    g_theEventSystem->Subscribe( EVENT_EDITOR_CHANGE_STEP, this, &StepWindow::HandleStepChange );
}


StepWindow::~StepWindow() {
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_CHANGE_STEP, this, &StepWindow::HandleStepChange );
}


void StepWindow::UpdateChild( float deltaSeconds ) {
    UNUSED( deltaSeconds );
    RenderStepSlider();
}


void StepWindow::RenderStepSlider() {
    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    Strings stepNames = mapWindow->GetStepNames();

    int numSteps = (int)stepNames.size();

    if( m_sliderIndex < 0 || m_sliderIndex >= numSteps ) {
        m_sliderIndex = numSteps - 1;
    }

    std::string indexStr = Stringf( "%d", m_sliderIndex + 1 ); // displayed as 1 indexed values

    if( ImGui::SliderInt( "Step Index", &m_sliderIndex, 0, numSteps - 1, indexStr.c_str() ) ) {
        // Slider changed
        EventArgs args;
        args.SetValue( "stepIndex", m_sliderIndex );

        g_theEventSystem->FireEvent( EVENT_EDITOR_CHANGE_STEP, args );
    }
}


bool StepWindow::HandleStepChange( EventArgs& args ) {
    m_sliderIndex = args.GetValue( "stepIndex", m_sliderIndex );

    return false;
}

#endif
