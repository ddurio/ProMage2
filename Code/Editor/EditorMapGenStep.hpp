#pragma once
#include "Editor/EditorCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"

struct FloatRange;
struct IntRange;


class EditorMapGenStep {
    public:
    EditorMapGenStep() {};
    ~EditorMapGenStep() {};

    static void RenderStepParms( MapGenStep* genStep, const std::string& stepName );


    private:
    // Conditions
    static void RenderConditions( MapGenStep* genStep, const std::string& stepName );
    static void RenderConditions_BaseClass( MapGenStep* genStep );
    static void RenderConditions_CellularAutomata( MapGenStep* genStep );
    static void RenderConditions_DistanceField( MapGenStep* genStep );
    static void RenderConditions_FromImage( MapGenStep* genStep );
    static void RenderConditions_PerlinNoise( MapGenStep* genStep );
    static void RenderConditions_RoomsAndPaths( MapGenStep* genStep );
    static void RenderConditions_Sprinkle( MapGenStep* genStep );

    // Results
    static void RenderResults( MapGenStep* genStep, const std::string& stepName );
    static void RenderResults_BaseClass( MapGenStep* genStep );
    static void RenderResults_CellularAutomata( MapGenStep* genStep );
    static void RenderResults_DistanceField( MapGenStep* genStep );
    static void RenderResults_FromImage( MapGenStep* genStep );
    static void RenderResults_PerlinNoise( MapGenStep* genStep );
    static void RenderResults_RoomsAndPaths( MapGenStep* genStep );
    static void RenderResults_Sprinkle( MapGenStep* genStep );

    // Helpers
    static void RenderEventList( const std::string& label, std::vector< MapGenStep::CustomEvent >& allEvents, std::vector< MapGenStep::CustomEvent >& currentEvents );

    
    private:
    static Strings GetEventNames( const std::vector< MapGenStep::CustomEvent >& eventList );
};
