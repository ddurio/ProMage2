#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor//EditorWindow.hpp"

#include "Engine/Math/Vec2.hpp"


class MapWindow;


class StepWindow : public EditorWindow {
    public:
    StepWindow( const MapWindow& mapEditor, const Vec2& normDimensions = Vec2( 0.8f, 0.1f ), const Vec2& alignment = Vec2::ZERO );
    ~StepWindow();


    private:
    const MapWindow& m_mapEditor;


    void UpdateChild( float deltaSeconds ) override;
};
