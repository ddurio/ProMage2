#if defined(_EDITOR)
#include "Editor/MediaWindow.hpp"

#include "Editor/Editor.hpp"
#include "Editor/ImGuiUtils.hpp"
#include "Editor/MapWindow.hpp"

#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Engine/Core/ImGuiSystem.hpp"


MediaWindow::MediaWindow( const Vec2& normDimensions /*= Vec2( 0.8f, 0.1f )*/, const Vec2& alignment /*= Vec2::ZERO */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "StepEditor";
    m_extraFlags = ImGuiWindowFlags_NoTitleBar;
    g_theEventSystem->Subscribe( EVENT_EDITOR_CHANGE_STEP, this, &MediaWindow::HandleStepChange );

    const Clock* editorClock = g_theEditor->GetEditorClock();
    m_stepTimer = new Timer( editorClock );
    m_stepTimer->SetDuration( m_secondsPerStep );
}


MediaWindow::~MediaWindow() {
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_CHANGE_STEP, this, &MediaWindow::HandleStepChange );
}


bool MediaWindow::UpdateChild( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    ImGui::SetWindowFontScale( 2.f );

    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    std::string stepName = mapWindow->GetStepName();
    ImGui::Text( stepName.c_str() );

    UpdatePlaying( deltaSeconds );
    RenderMediaButtons();
    RenderStepSlider();

    return false;
}


void MediaWindow::UpdatePlaying( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    if( m_isPlaying && m_stepTimer->Decrement() ) {
        MapWindow* mapWindow = g_theEditor->GetMapWindow();
        int numSteps = mapWindow->GetNumSteps();

        if( m_isLooping ) {
            m_sliderIndex = (m_sliderIndex + 1) % numSteps;
        } else if( m_sliderIndex + 1 == numSteps ) {
            m_isPlaying = false;
            return;
        } else {
            m_sliderIndex++;
        }

        ChangeStepIndex();
    }
}


void MediaWindow::RenderMediaButtons() {
    ImVec2 buttonSize = ImVec2( 25.f, 25.f );

    if( RenderImageButton( TEXTURE_EDITOR_MEDIA_START, buttonSize ) ) {
        m_isPlaying = false;
        m_sliderIndex = 0;
    }

    ImGui::SameLine();
    const char* playPause = m_isPlaying ? TEXTURE_EDITOR_MEDIA_PAUSE : TEXTURE_EDITOR_MEDIA_PLAY;

    if( RenderImageButton( playPause, buttonSize ) ) {
        m_isPlaying = !m_isPlaying;

        if( m_isPlaying ) {
            m_sliderIndex++;
            m_stepTimer->Restart();
        }
    }

    ImGui::SameLine();
    Rgba bgColor = m_isLooping ? Rgba::ORGANIC_GRAY : Rgba::CLEAR_BLACK;

    if( RenderImageButton( TEXTURE_EDITOR_MEDIA_LOOP, buttonSize, bgColor ) ) {
        m_isLooping = !m_isLooping;
    }

    ImGui::SameLine();

    if( RenderImageButton( TEXTURE_EDITOR_MEDIA_END, buttonSize ) ) {
        m_isPlaying = false;

        MapWindow* mapWindow = g_theEditor->GetMapWindow();
        Strings stepNames = mapWindow->GetStepNames();

        m_sliderIndex = (int)stepNames.size();
    }

    ImGui::SameLine();
}


void MediaWindow::RenderStepSlider() {
    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    Strings stepNames = mapWindow->GetStepNames();

    int numSteps = (int)stepNames.size();

    if( m_sliderIndex < 0 || m_sliderIndex >= numSteps ) {
        m_sliderIndex = numSteps - 1;
    }

    std::string indexStr = Stringf( "%d", m_sliderIndex + 1 ); // displayed as 1 indexed values

    if( ImGui::SliderInt( "Step Index", &m_sliderIndex, 0, numSteps - 1, indexStr.c_str() ) ) {
        // Slider manually changed
        m_isPlaying = false;
    }

    if( m_sliderIndex != mapWindow->GetStepIndex() ) {
        // Someone updated stepIndex (manually, via Event call, etc...)
        ChangeStepIndex();
    }
}


void MediaWindow::ChangeStepIndex() const {
    EventArgs args;
    args.SetValue( "stepIndex", m_sliderIndex );

    g_theEventSystem->FireEvent( EVENT_EDITOR_CHANGE_STEP, args );
}


bool MediaWindow::HandleStepChange( EventArgs& args ) {
    m_sliderIndex = args.GetValue( "stepIndex", m_sliderIndex );

    return false;
}

#endif
