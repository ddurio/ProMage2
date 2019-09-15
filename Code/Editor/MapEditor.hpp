#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Game/MapGen/Map/Map.hpp"


class Camera;


class MapEditor : public EditorWindow {
    public:
    MapEditor( const Vec2& normDimensions = Vec2( 0.8f, 0.9f ), const Vec2& alignment = Vec2( 0.f, 1.f ) );
    ~MapEditor();


    private:
    std::vector< Map* > m_mapPerStep;
    int m_stepIndex = -1;

    Camera* m_mapCamera = nullptr;
    std::string m_mapViewName = "mapEditor/mapView";


    void UpdateChild( float deltaSeconds ) override;
};
