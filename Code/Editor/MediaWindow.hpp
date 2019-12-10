#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec2.hpp"


class MapWindow;
class Timer;


class MediaWindow : public EditorWindow {
    public:
    MediaWindow( const Vec2& normDimensions = Vec2( 0.8f, 0.1f ), const Vec2& alignment = Vec2::ZERO );
    ~MediaWindow();


    private:
    int m_sliderIndex = -1;

    // Media info
    Timer* m_stepTimer = nullptr;
    float m_secondsPerStep = 1.f;

    bool m_isPlaying = false;
    bool m_isLooping = false;


    bool UpdateChild( float deltaSeconds ) override;
    void UpdatePlaying( float deltaSeconds );

    void RenderMediaButtons();
    void RenderStepSlider();

    void ChangeStepIndex() const;
    bool HandleStepChange( EventArgs& args );
};
