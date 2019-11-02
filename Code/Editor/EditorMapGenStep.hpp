#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/EventSystem.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"

struct FloatRange;
struct IntRange;
class MGS_Custom;


class EditorMapGenStep {
    public:
    EditorMapGenStep() {};
    ~EditorMapGenStep() {};

    virtual void DontImplement() = 0; // Safety net to insure EMGS is not instantiated.. use only as static class/namespace

    static void RenderStepParams( MapGenStep* genStep, const std::string& stepName );
    static bool ResetChangedParams( EventArgs& args );

    static bool IsChanged( MapGenStep* genStep );


    private:
    static std::map< MapGenStep*, std::vector< bool > > s_conditionChangelist;
    static std::map< MapGenStep*, std::vector< bool > > s_resultChangelist;
    static std::map< MapGenStep*, std::vector< bool > > s_customChangelist;


    // Conditions
    static void RenderConditions( MapGenStep* genStep, const std::string& stepName, bool isCustomStep );
    static void RenderConditions_BaseClass( MapGenStep* genStep, bool isCustomStep );
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

    // Custom Step
    static void RenderChildSteps( MapGenStep* genStep, const std::string& stepName );

    // Helpers
    static Strings GetEventNames( const std::vector< MapGenStep::CustomEvent >& eventList );
    static bool RenderEventList( const std::string& label, std::vector< MapGenStep::CustomEvent >& allEvents, std::vector< MapGenStep::CustomEvent >& currentEvents );

    static bool IsChangedConditions( MapGenStep* genStep );
    static bool IsChangedResults( MapGenStep* genStep );
    static bool IsChangedCustom( MGS_Custom* genStep );
};
