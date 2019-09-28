#pragma once
#include "Editor/EditorCommon.hpp"


class MapGenStep;

struct IntRange;


class EditorMapGenStep {
    public:
    EditorMapGenStep() {};
    ~EditorMapGenStep() {};

    static void RenderStepParms( MapGenStep* genStep );


    private:
    static void RenderConditions( MapGenStep* genStep );
    static void RenderConditions_BaseClass( MapGenStep* genStep );
    static void RenderConditions_CellularAutomata( MapGenStep* genStep );
    static void RenderConditions_DistanceField( MapGenStep* genStep );
    static void RenderConditions_FromImage( MapGenStep* genStep );
    static void RenderConditions_PerlinNoise( MapGenStep* genStep );
    static void RenderConditions_RoomsAndPaths( MapGenStep* genStep );
    static void RenderConditions_Sprinkle( MapGenStep* genStep );

    static void RenderResults( MapGenStep* genStep );
    static void RenderResults_BaseClass( MapGenStep* genStep );
    static void RenderResults_CellularAutomata( MapGenStep* genStep );
    static void RenderResults_DistanceField( MapGenStep* genStep );
    static void RenderResults_FromImage( MapGenStep* genStep );
    static void RenderResults_PerlinNoise( MapGenStep* genStep );
    static void RenderResults_RoomsAndPaths( MapGenStep* genStep );
    static void RenderResults_Sprinkle( MapGenStep* genStep );

    static void RenderIntRange( IntRange& range );
    static void RenderTileDropDown( std::string& currentType, const std::string& label = "Tile Type" );
    static void RenderTags( Strings& currentTags );
};
