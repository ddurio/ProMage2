#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec2.hpp"


class EditorMapDef;
class MapGenStep;


class XmlWindow : public EditorWindow {
    public:
    XmlWindow( const Vec2& normDimensions = Vec2( 0.2f, 1.f ), const Vec2& alignment = Vec2::ONE );
    ~XmlWindow();

    bool ShouldHighlightTiles() const;

    void TriggerMapGen( const std::string& mapType, int stepIndex, bool useCustomSeed = false );


    private:
    bool m_highlightChanges = true;
    std::vector< bool > m_stepHeaderOpen;
    const MapGenStep* m_stepToCopy = nullptr;

    std::string m_mapType = "";
    std::string m_newStepType = "";
    Strings m_stepTypes = {
        "CellularAutomata",
        "DistanceField",
        "FromImage",
        "PerlinNoise",
        "RoomsAndPaths",
        "Sprinkle",
        // ...
        // plus custom step types added at run time
        // ...
    };



    bool UpdateChild( float deltaSeconds ) override;

    bool RenderRegenSettings( EditorMapDef* eMapDef, int stepIndex );
    void RenderGenSteps( EditorMapDef* eMapDef );
    bool RenderContextMenu( EditorMapDef* eMapDef, const std::string& guiID, int stepIndex, int numSteps );
    bool RenderNewStepMenu( EditorMapDef* eMapDef, int stepIndex, bool insertBefore );

    bool ChangeMapType( EventArgs& args );
};
