#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Engine/Core/WindowContext.hpp"

#include "Game/MapGen/Map/Map.hpp"


class Camera;


class MapWindow : public EditorWindow {
    public:
    MapWindow( const Vec2& normDimensions = Vec2( 0.8f, 0.9f ), const Vec2& alignment = Vec2( 0.f, 1.f ) );
    ~MapWindow();

    void Startup();
    void Shutdown();

    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    int GetStepIndex() const;
    unsigned int GetMapSeed() const;
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
    float m_minPixelsPerTile = -1;
    AABB2 m_imageBounds = AABB2::ZEROTOONE;

    // Zoom info
    float m_currentZoom = 0.f;
    float m_maxZoom = 1.f;
    float m_minZoomT = 0.f;
    float m_zoomIncrement = 0.1f;
    float m_zoomPixelsPerTile = 0.f;

    // Drag info
    Vec2 m_cameraPosition = Vec2::ZERO;
    bool m_isDragging = false;
    bool m_isZooming = true;


    void UpdateChild( float deltaSeconds ) override;
    void UpdateMapCamera( float deltaSeconds );
    void UpdateZoom( float deltaSeconds );

    Vec2 GetClampedDisplacement( const Vec2& worldDisp ) const;
    Vec2 GetMouseWorldPosition() const;

    void RenderMap( float deltaSeconds );
    void RenderTileChangeHighlight();
    void RenderTileChangeTooltip();

    bool GenerateMaps( EventArgs& args );
    bool SetVisibleMapStep( EventArgs& args );

    void CalculateMaxZoom();
    void CalculateZoomRange();
    void CalculateZoomIncrement();
    void CalculateMapSizes();

    // Tile Changes
    Strings GetTileChanges( const IntVec2& tileCoord ) const;
    void GetTileTypeChanges( Strings& changeList, const Tile& currentTile, const Tile& prevTile ) const;
    void GetTagChanges( Strings& changeList, Strings currentTags, Strings prevTags ) const;
    void GetHeatMapChanges( Strings& changeList, const Metadata* currentMetadata, const Metadata* prevMetadata ) const;
    void GetActorChanges( Strings& changeList, const Map* currentMap, const Map* prevMap, const Tile& currentTile ) const;
    void GetItemChanges( Strings& changeList, const Map* currentMap, const Map* prevMap, const Tile& currentTile ) const;
    void GetTileRenderChanges( Strings& changeList, const Tile& currentTile, const Tile& prevTile ) const;
};
