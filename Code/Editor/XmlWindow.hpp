#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Engine/Math/Vec2.hpp"


class EditorMapDef;


class XmlWindow : public EditorWindow {
    public:
    XmlWindow( const Vec2& normDimensions = Vec2( 0.2f, 1.f ), const Vec2& alignment = Vec2::ONE );
    ~XmlWindow();

    bool ShouldHighlightTiles() const;


    private:
    bool m_highlightChanges = true;
    std::vector< bool > m_stepHeaderOpen;

    std::string m_mapType = "";


    void UpdateChild( float deltaSeconds ) override;

    void RenderRegenSettings( EditorMapDef* eMapDef, int stepIndex );
};
