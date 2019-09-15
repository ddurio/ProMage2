#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor//EditorWindow.hpp"

#include "Engine/Math/Vec2.hpp"


class StepEditor : public EditorWindow {
    public:
    StepEditor( const Vec2& normDimensions = Vec2( 0.8f, 0.1f ), const Vec2& alignment = Vec2::ZERO );
    ~StepEditor();


    private:
    void UpdateChild( float deltaSeconds ) override;
};