#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/EventSystem.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"

struct FloatRange;
struct IntRange;


class EditorMapGenStep {
    public:
    EditorMapGenStep() {};
    ~EditorMapGenStep() {};

    virtual void DontImplement() = 0; // Safety net to insure EMGS is not instantiated.. use only as static class/namespace

    static void RenderStepParams( MapGenStep* genStep, const std::string& stepName );
    static bool ResetChangedParams( EventArgs& args );

    static bool IsChanged( MapGenStep* genStep );

    static void FindXmlMotifVariables( const XMLElement& element, MapGenStep* genStep );


    private:
    static std::map< MapGenStep*, std::vector< bool > > s_conditionChangelist;
    static std::map< MapGenStep*, std::vector< bool > > s_resultChangelist;


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
    static Strings GetEventNames( const std::vector< MapGenStep::CustomEvent >& eventList );
    static bool RenderEventList( const std::string& label, std::vector< MapGenStep::CustomEvent >& allEvents, std::vector< MapGenStep::CustomEvent >& currentEvents );

    static bool IsChangedConditions( MapGenStep* genStep );
    static bool IsChangedResults( MapGenStep* genStep );

    // Find Xml Variables
    static void FindXmlMotifVariables_BaseClass( const XMLElement& element, MapGenStep* genStep );
    static void FindXmlMotifVariables_CellularAutomata( const XMLElement& element, MapGenStep* genStep );
    static void FindXmlMotifVariables_DistanceField( const XMLElement& element, MapGenStep* genStep );
    static void FindXmlMotifVariables_FromImage( const XMLElement& element, MapGenStep* genStep );
    static void FindXmlMotifVariables_PerlinNoise( const XMLElement& element, MapGenStep* genStep );
    static void FindXmlMotifVariables_RoomsAndPaths( const XMLElement& element, MapGenStep* genStep );
    static void FindXmlMotifVariables_Sprinkle( const XMLElement& element, MapGenStep* genStep );
};
