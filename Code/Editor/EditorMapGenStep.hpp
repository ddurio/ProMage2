#pragma once
#include "Editor/EditorCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"

struct FloatRange;
struct IntRange;


class EditorMapGenStep {
    public:
    EditorMapGenStep() {};
    ~EditorMapGenStep() {};

    static void RenderStepParms( MapGenStep* genStep );


    private:
    // Conditions
    static void RenderConditions( MapGenStep* genStep );
    static void RenderConditions_BaseClass( MapGenStep* genStep );
    static void RenderConditions_CellularAutomata( MapGenStep* genStep );
    static void RenderConditions_DistanceField( MapGenStep* genStep );
    static void RenderConditions_FromImage( MapGenStep* genStep );
    static void RenderConditions_PerlinNoise( MapGenStep* genStep );
    static void RenderConditions_RoomsAndPaths( MapGenStep* genStep );
    static void RenderConditions_Sprinkle( MapGenStep* genStep );

    // Results
    static void RenderResults( MapGenStep* genStep );
    static void RenderResults_BaseClass( MapGenStep* genStep );
    static void RenderResults_CellularAutomata( MapGenStep* genStep );
    static void RenderResults_DistanceField( MapGenStep* genStep );
    static void RenderResults_FromImage( MapGenStep* genStep );
    static void RenderResults_PerlinNoise( MapGenStep* genStep );
    static void RenderResults_RoomsAndPaths( MapGenStep* genStep );
    static void RenderResults_Sprinkle( MapGenStep* genStep );

    // Helpers
    static void RenderPercent( float& value, const std::string& label = "" );
    static void RenderIntRange( IntRange& range, const std::string& label = "", int minValue = 0, int maxValue = 10 );
    static void RenderFloatRange( FloatRange& range, const std::string& label = "", float minValue = 0.f, float maxValue = 10.f );
    static void RenderTileDropDown( std::string& currentType, const std::string& label = "Tile Type" );
    static void RenderTags( const std::string& uniqueKey, Strings& currentTags, const std::string& label = "" );
    static void RenderHeatMaps( const std::string& uniqueKey, std::map< std::string, FloatRange, StringCmpCaseI >& currentHeatMaps );
    static void RenderEventList( const std::string& label, std::vector< MapGenStep::CustomEvent >& allEvents, std::vector< MapGenStep::CustomEvent >& currentEvents );

    
    private:
    static Strings GetEventNames( const std::vector< MapGenStep::CustomEvent >& eventList );
};
