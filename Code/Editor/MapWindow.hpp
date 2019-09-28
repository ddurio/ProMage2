#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Game/MapGen/Map/Map.hpp"


class Camera;


class MapWindow : public EditorWindow {
    public:
    MapWindow( const Vec2& normDimensions = Vec2( 0.8f, 0.9f ), const Vec2& alignment = Vec2( 0.f, 1.f ) );
    ~MapWindow();

    void Startup();
    void Shutdown();

    int GetStepIndex() const;
    std::string GetMapType() const;
    Strings GetStepNames() const;


    private:
    std::vector< Map* > m_mapPerStep;
    int m_stepIndex = -1;

    Camera* m_mapCamera = nullptr;
    const std::string m_mapViewName  = "mapEditor/mapView";

    unsigned int m_highlightColor = 0;

    // Map sizes
    bool m_sizeIsCalculated = false;
    AABB2 m_mapBounds = AABB2::ZEROTOONE;
    float m_pixelsPerTile = -1;


    void UpdateChild( float deltaSeconds ) override;

    void RenderMap( float deltaSeconds );
    void RenderTileChangeHighlight();
    void RenderTileChangeTooltip();

    bool GenerateMaps( EventArgs& args );
    bool SetVisibleMapStep( EventArgs& args );

    void CalculateMapSizes();
    Strings GetTileChanges( const IntVec2& tileCoord ) const;
};
