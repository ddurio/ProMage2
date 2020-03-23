#if defined(_EDITOR)
#include "Editor/EditorMapGenStep.hpp"

#include "Editor/ImGuiUtils.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Game/MapGen/GenSteps/MGS_CellularAutomata.hpp"
#include "Game/MapGen/GenSteps/MGS_Custom.hpp"
#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"
#include "Game/MapGen/GenSteps/MGS_DistanceField.hpp"
#include "Game/MapGen/GenSteps/MGS_FromImage.hpp"
#include "Game/MapGen/GenSteps/MGS_PerlinNoise.hpp"
#include "Game/MapGen/GenSteps/MGS_RoomsAndPaths.hpp"
#include "Game/MapGen/GenSteps/MGS_Sprinkle.hpp"
#include "Game/MapGen/Map/TileDef.hpp"

#include "ThirdParty/DearImGui/imgui_internal.h"


std::map< MapGenStep*, std::vector< bool > > EditorMapGenStep::s_conditionChangelist;   // PRIVATE
std::map< MapGenStep*, std::vector< bool > > EditorMapGenStep::s_resultChangelist;      // PRIVATE
std::map< MapGenStep*, std::vector< bool > > EditorMapGenStep::s_customChangelist;      // PRIVATE


void EditorMapGenStep::RenderStepParams( MapGenStep* genStep, const std::string& stepName ) {
    if( genStep == nullptr ) {
        ImGui::Text( "Internal step... No modifiable values" );
        return;
    }
    
    std::string stepType = genStep->GetName();
    Strings customStepTypes = MGS_CustomDef::GetAllDefinitionTypes();
    bool isCustomStep = EngineCommon::VectorContains( customStepTypes, stepType );

    // Motif 
    Strings motifNames = MotifDef::GetAllDefinitionTypes();
    int motifIndex = (isCustomStep) ? 1 : 0;

    if( RenderDropDown( "eMapDefMotif", genStep->m_motifHierarchy[motifIndex], motifNames, "Motif", true, "" ) ) {
        if( isCustomStep ) {
            genStep->UpdateParentMotifs( { genStep->m_motifHierarchy[2] } );
        } else {
            TriggerMotifRecalculation( MAPGEN_ARG_RECALC_ALL ); // ThesisFIXME: Changed values will not be marked as different by Editor (maybe regen..?)
        }
    }

    // Conditions
    RenderConditions( genStep, stepName, isCustomStep );

    // Results
    if( isCustomStep ) {
        RenderChildSteps( genStep, stepName );
    } else {
        RenderResults( genStep, stepName );
    }
}


bool EditorMapGenStep::ResetChangedParams( EventArgs& args ) {
    UNUSED( args );
    s_conditionChangelist.clear();
    s_resultChangelist.clear();
    return false;
}


bool EditorMapGenStep::IsChanged( MapGenStep* genStep ) {
    bool condChanged = IsChangedConditions( genStep );
    bool resultChanged = IsChangedResults( genStep );

    return condChanged || resultChanged;
}


Strings EditorMapGenStep::GetEventNames( const std::vector< MapGenStep::CustomEvent >& eventList ) {
    Strings names;
    int numEvents = (int)eventList.size();

    for( int eventIndex = 0; eventIndex < numEvents; eventIndex++ ) {
        const MapGenStep::CustomEvent& event = eventList[eventIndex];
        names.push_back( event.name );
    }

    return names;
}


// PRIVATE ----------------------------------------------------------------------
void EditorMapGenStep::RenderConditions( MapGenStep* genStep, const std::string& stepName, bool isCustomStep ) {
    bool condChanged = IsChangedConditions( genStep );
    std::string headerStr = Stringf( "Conditions%s", condChanged ? " *" : "" );
    SetImGuiTextColor( condChanged ? Rgba::ORGANIC_YELLOW : Rgba::WHITE );

    if( ImGui::CollapsingHeader( headerStr.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) ) {
        std::string stepCondStr = Stringf( "%s_Conditions", stepName.c_str() );
        ImGui::TreePush( stepCondStr.c_str() );

        std::string stepType = genStep->GetName();

        RenderConditions_BaseClass( genStep, isCustomStep );
        ImGui::Separator();

        if( StringICmp( stepType, "CellularAutomata" ) ) {
            RenderConditions_CellularAutomata( genStep );
        } else if( StringICmp( stepType, "DistanceField" ) ) {
            RenderConditions_DistanceField( genStep );
        } else if( StringICmp( stepType, "FromImage" ) ) {
            RenderConditions_FromImage( genStep );
        } else if( StringICmp( stepType, "PerlinNoise" ) ) {
            RenderConditions_PerlinNoise( genStep );
        } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
            RenderConditions_RoomsAndPaths( genStep );
        } else if( StringICmp( stepType, "Sprinkle" ) ) {
            RenderConditions_Sprinkle( genStep );
        } else if( isCustomStep ) {
            RenderConditions_Custom( genStep );
        } else {
            ERROR_RECOVERABLE( Stringf( "(EditorMapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }

        ImGui::TreePop();
    }
}


void EditorMapGenStep::RenderConditions_BaseClass( MapGenStep* genStep, bool isCustomStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.empty() ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS, false );
    }

    std::vector< bool > localChanges;
    localChanges.resize( NUM_BASE_CONDITIONS, false );

    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "baseCond";

    RenderChangeText( paramsChanged[0] );
    localChanges[0] = RenderPercentOrVar( uniqueKey, stepVars, "chanceToRun", genStep->m_motifHierarchy, genStep->m_chanceToRun, "Chance To Run" );

    RenderChangeText( paramsChanged[1] );
    localChanges[1] = RenderIntRangeOrVar( uniqueKey, stepVars, "numIterations", genStep->m_motifHierarchy, genStep->m_numIterations, "Iterations" );
    std::array< bool, 2 > change3and4 = { false, false };

    if( !isCustomStep ) {
        ImGui::Separator();

        RenderChangeText( paramsChanged[2] );
        localChanges[2] = RenderTileDropDownOrVar( uniqueKey, stepVars, "ifIsType", genStep->m_motifHierarchy, genStep->m_ifIsType );

        RenderChangeText( paramsChanged[3] );
        change3and4 = RenderTagsOrVar( uniqueKey, stepVars, "ifHasTags", genStep->m_motifHierarchy, genStep->m_ifHasTags, paramsChanged[4] );

        RenderChangeText( paramsChanged[5] );
        localChanges[5] = RenderHeatMapsOrVar( uniqueKey, stepVars, "ifHeatMap", genStep->m_motifHierarchy, genStep->m_ifHeatMap );

        RenderChangeText( paramsChanged[6] );
        localChanges[6] = RenderEventListOrVar( uniqueKey, stepVars, genStep->m_motifHierarchy, genStep->m_customConditions, genStep->s_customConditions, "Conditions" );
    }

    paramsChanged[0] = paramsChanged[0] || localChanges[0];
    paramsChanged[1] = paramsChanged[1] || localChanges[1];
    paramsChanged[2] = paramsChanged[2] || localChanges[2];
    paramsChanged[3] = paramsChanged[3] || change3and4[0];
    paramsChanged[4] = paramsChanged[4] || change3and4[1];
    paramsChanged[5] = paramsChanged[5] || localChanges[5];
    paramsChanged[6] = paramsChanged[6] || localChanges[6];
}


void EditorMapGenStep::RenderConditions_CellularAutomata( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= NUM_BASE_CONDITIONS ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS + 6, false );
    }

    MGS_CellularAutomata* caStep = (MGS_CellularAutomata*)genStep;
    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "caCond";

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS] );
    bool change0 = RenderPercentOrVar( uniqueKey, stepVars, "chancePerTile", genStep->m_motifHierarchy, caStep->m_chancePerTile, "Chance per Tile" );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 1] );
    bool change1 = RenderIntRangeOrVar( uniqueKey, stepVars, "radius", genStep->m_motifHierarchy, caStep->m_radius, "Tile Radius", 1 );
    ImGui::Separator();

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 2] );
    bool change2 = RenderTileDropDownOrVar( uniqueKey, stepVars, "ifNeighborType", genStep->m_motifHierarchy, caStep->m_ifNeighborType, "Neighbor Type" );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 3] );
    std::array< bool, 2 > change3and4 = RenderTagsOrVar( uniqueKey, stepVars, "ifNeighborHasTags", genStep->m_motifHierarchy, caStep->m_ifNeighborHasTags, paramsChanged[11], "Neighbor Has", "Neighbor Missing" ); // FIXME: hard coded changed value

    int tileWidth = (2 * caStep->m_radius.max) + 1;
    int maxNeighbors = (tileWidth * tileWidth) - 1;

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 5] );
    bool change5 = RenderIntRangeOrVar( uniqueKey, stepVars, "ifNumNeighbors", genStep->m_motifHierarchy, caStep->m_ifNumNeighbors, "Num Neighbors", 0, maxNeighbors, IntRange( 1, 8 ) );

    paramsChanged[NUM_BASE_CONDITIONS    ] = paramsChanged[NUM_BASE_CONDITIONS]     || change0;
    paramsChanged[NUM_BASE_CONDITIONS + 1] = paramsChanged[NUM_BASE_CONDITIONS + 1] || change1;
    paramsChanged[NUM_BASE_CONDITIONS + 2] = paramsChanged[NUM_BASE_CONDITIONS + 2] || change2;
    paramsChanged[NUM_BASE_CONDITIONS + 3] = paramsChanged[NUM_BASE_CONDITIONS + 3] || change3and4[0];
    paramsChanged[NUM_BASE_CONDITIONS + 4] = paramsChanged[NUM_BASE_CONDITIONS + 4] || change3and4[1];
    paramsChanged[NUM_BASE_CONDITIONS + 5] = paramsChanged[NUM_BASE_CONDITIONS + 5] || change5;
}


void EditorMapGenStep::RenderConditions_DistanceField( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= NUM_BASE_CONDITIONS ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS + 2, false );
    }

    MGS_DistanceField* dfStep = (MGS_DistanceField*)genStep;
    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "dfCond";

    static const Strings movementTypes = MGS_DistanceField::GetMovementTypes();
    std::string initialType = dfStep->m_movementType;

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS] );
    bool change0 = RenderDropDownOrVar( uniqueKey, stepVars, "movementType", genStep->m_motifHierarchy, dfStep->m_movementType, movementTypes, "Movement Type", false, "__HOPEFULLLY_NOT_A_MOVEMENT_TYPE__" );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 1] );
    bool change1 = RenderIntOrVar( uniqueKey, stepVars, "maxDistance", genStep->m_motifHierarchy, dfStep->m_maxDistance, "Max Distance", (int)dfStep->INVALID_DISTANCE );

    paramsChanged[NUM_BASE_CONDITIONS    ] = paramsChanged[NUM_BASE_CONDITIONS]     || change0;
    paramsChanged[NUM_BASE_CONDITIONS + 1] = paramsChanged[NUM_BASE_CONDITIONS + 1] || change1;
}


void EditorMapGenStep::RenderConditions_FromImage( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= NUM_BASE_CONDITIONS ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS + 4, false );
    }

    MGS_FromImage* imageStep = (MGS_FromImage*)genStep;
    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "imageCond";

    Strings filter = {
        "JPG",  "*.jpg",
        "PNG",  "*.png"
    };

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS] );
    bool change0 = RenderFilePathOrVar( uniqueKey, stepVars, "imageFilePath", genStep->m_motifHierarchy, imageStep->m_imageFilePath, filter );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 1] );
    bool change1  = RenderFloatRangeOrVar( uniqueKey, stepVars, "alignX", genStep->m_motifHierarchy, imageStep->m_alignX, "X Alignment", 0.f, 1.f, FloatRange::ZEROTOONE );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 2] );
    bool change2  = RenderFloatRangeOrVar( uniqueKey, stepVars, "alignY", genStep->m_motifHierarchy, imageStep->m_alignY, "Y Alignment", 0.f, 1.f, FloatRange::ZEROTOONE );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 3] );
    bool change3 = RenderIntRangeOrVar( uniqueKey, stepVars, "numRotations", genStep->m_motifHierarchy, imageStep->m_numRotations, "Rotations", 0, 3, IntRange::ZERO );

    paramsChanged[NUM_BASE_CONDITIONS    ] = paramsChanged[NUM_BASE_CONDITIONS]     || change0;
    paramsChanged[NUM_BASE_CONDITIONS + 1] = paramsChanged[NUM_BASE_CONDITIONS + 1] || change1;
    paramsChanged[NUM_BASE_CONDITIONS + 2] = paramsChanged[NUM_BASE_CONDITIONS + 2] || change2;
    paramsChanged[NUM_BASE_CONDITIONS + 3] = paramsChanged[NUM_BASE_CONDITIONS + 3] || change3;
}


void EditorMapGenStep::RenderConditions_PerlinNoise( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= NUM_BASE_CONDITIONS ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS + 4, false );
    }

    MGS_PerlinNoise* noiseStep = (MGS_PerlinNoise*)genStep;
    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "noiseCond";

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS] );
    bool change0  = RenderIntRangeOrVar( uniqueKey, stepVars, "gridSize", genStep->m_motifHierarchy, noiseStep->m_gridSize, "Grid Size", 1, 50, IntRange( 10, 30 ) );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 1] );
    bool change1  = RenderIntRangeOrVar( uniqueKey, stepVars, "octaves", genStep->m_motifHierarchy, noiseStep->m_numOctaves, "Octaves", 1, 10, IntRange( 1, 3 ) );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 2] );
    bool change2  = RenderFloatRangeOrVar( uniqueKey, stepVars, "persistence", genStep->m_motifHierarchy, noiseStep->m_octavePersistence, "Persistence", 0.f, 1.f, FloatRange( 0.4f, 0.6f ) );

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + 3] );
    bool change3 = RenderFloatRangeOrVar( uniqueKey, stepVars, "scale", genStep->m_motifHierarchy, noiseStep->m_octaveScale, "Scale", 0.f, 5.f, FloatRange( 1.5f, 2.5f ) );

    paramsChanged[NUM_BASE_CONDITIONS    ] = paramsChanged[NUM_BASE_CONDITIONS]     || change0;
    paramsChanged[NUM_BASE_CONDITIONS + 1] = paramsChanged[NUM_BASE_CONDITIONS + 1] || change1;
    paramsChanged[NUM_BASE_CONDITIONS + 2] = paramsChanged[NUM_BASE_CONDITIONS + 2] || change2;
    paramsChanged[NUM_BASE_CONDITIONS + 3] = paramsChanged[NUM_BASE_CONDITIONS + 3] || change3;
}


void EditorMapGenStep::RenderConditions_RoomsAndPaths( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderConditions_Sprinkle( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= NUM_BASE_CONDITIONS ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS + 1, false );
    }

    MGS_Sprinkle* sprinkleStep = (MGS_Sprinkle*)genStep;
    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "sprinkleCond";

    RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS] );
    bool change0 = RenderIntRangeOrVar( uniqueKey, stepVars, "count", genStep->m_motifHierarchy, sprinkleStep->m_countRange, "Sprinkles", 1 );

    paramsChanged[NUM_BASE_CONDITIONS] = paramsChanged[NUM_BASE_CONDITIONS] || change0;
}


void EditorMapGenStep::RenderConditions_Custom( MapGenStep* genStep ) {
    MGS_Custom* cStep = (MGS_Custom*)genStep;

    const MGS_CustomDef* cStepDef = MGS_CustomDef::GetDefinition( cStep->m_defType );
    std::string defMotifName = cStepDef->GetMotif();
    Strings motifVarNames = MotifDef::GetVariableNames( { defMotifName } );

    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];
    int numVars = (int)motifVarNames.size();

    if( paramsChanged.size() <= NUM_BASE_CONDITIONS ) {
        paramsChanged.resize( NUM_BASE_CONDITIONS + numVars, false );
    }

    // Start rendering parameters
    NamedProperties& motifVars = cStep->m_customXmlMotif->GetVariableValues();
    Strings motifHierarchy = genStep->m_motifHierarchy;
    motifHierarchy.push_back( defMotifName );

    std::string uniqueKey = "customCond";

    for( int varIndex = 0; varIndex < numVars; varIndex++ ) {
        std::string varName = motifVarNames[varIndex];
        std::string varType = MotifDef::GetVariableType( { defMotifName }, varName, "string" );

        RenderChangeText( paramsChanged[NUM_BASE_CONDITIONS + varIndex] );
        bool wasChanged = false;
        std::string valueAsStr = "";

        ImGui::PushID( varName.c_str() );

        if( StringICmp( varType, "String" ) ) {
            valueAsStr = MotifDef::GetVariableValue( motifHierarchy, varName, "" );
            wasChanged = ImGui::InputText( varName.c_str(), &valueAsStr );

        } else if( StringICmp( varType, "Percent" ) ) {
            float currentValue = MotifDef::GetVariableValue( motifHierarchy, varName, 0.f );
            float defaultValue = MotifDef::GetVariableValue( { defMotifName }, varName, std::numeric_limits<float>::infinity() );
            wasChanged = RenderPercent( currentValue, varName, defaultValue );
            valueAsStr = Stringf( "%.3f", currentValue );

        } else if( StringICmp( varType, "FloatRange" ) ) {
            FloatRange currentValue = MotifDef::GetVariableValue( motifHierarchy, varName, FloatRange::NEGONE );
            FloatRange defaultValue = MotifDef::GetVariableValue( { defMotifName }, varName, FloatRange( -1.f, 1.f ) );
            wasChanged = RenderFloatRange( currentValue, varName, 0.f, 10.f, defaultValue );
            valueAsStr = currentValue.GetAsString();

        } else if( StringICmp( varType, "IntRange" ) ) {
            IntRange currentValue = MotifDef::GetVariableValue( motifHierarchy, varName, IntRange::NEGONE );
            IntRange defaultValue = MotifDef::GetVariableValue( { defMotifName }, varName, IntRange( 1, -1 ) );
            wasChanged = RenderIntRange( currentValue, varName, 0, 10, defaultValue );
            valueAsStr = currentValue.GetAsString();

        } else if( StringICmp( varType, "Bool" ) ) {
            bool currentValue = MotifDef::GetVariableValue( motifHierarchy, varName, false );
            bool defaultValue = MotifDef::GetVariableValue( { defMotifName }, varName, false );
            wasChanged = RenderCheckbox( currentValue, varName, defaultValue );
            valueAsStr = currentValue ? "True" : "False";

        } else if( StringICmp( varType, "Tile" ) ) {
            std::string currentValue = MotifDef::GetVariableValue( motifHierarchy, varName, "" );
            std::string defaultValue = MotifDef::GetVariableValue( { defMotifName }, varName, "" );
            wasChanged = RenderTileDropDown( uniqueKey, currentValue, varName, false, defaultValue );
            valueAsStr = currentValue;
        }

        ImGui::PopID();

        if( wasChanged ) {
            motifVars.SetValue( varName, valueAsStr );

            EventArgs args;
            args.SetValue( MAPGEN_ARG_ATTR_NAME, MAPGEN_ARG_RECALC_ALL );
            genStep->RecalculateMotifVars( args );
        }

        paramsChanged[NUM_BASE_CONDITIONS] = paramsChanged[NUM_BASE_CONDITIONS] || wasChanged;
    }
}


void EditorMapGenStep::RenderResults( MapGenStep* genStep, const std::string& stepName ) {
    bool resultsChanged = IsChangedResults( genStep );
    std::string headerStr = Stringf( "Modifications%s", resultsChanged ? " *" : "" );
    SetImGuiTextColor( resultsChanged ? Rgba::ORGANIC_YELLOW : Rgba::WHITE );

    if( ImGui::CollapsingHeader( headerStr.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) ) {
        std::string stepResultStr = Stringf( "%s_Results", stepName.c_str() );
        ImGui::TreePush( stepResultStr.c_str() );

        RenderResults_BaseClass( genStep );
        ImGui::Separator();

        std::string stepType = genStep->GetName();

        if( StringICmp( stepType, "CellularAutomata" ) ) {
            RenderResults_CellularAutomata( genStep );
        } else if( StringICmp( stepType, "DistanceField" ) ) {
            RenderResults_DistanceField( genStep );
        } else if( StringICmp( stepType, "FromImage" ) ) {
            RenderResults_FromImage( genStep );
        } else if( StringICmp( stepType, "PerlinNoise" ) ) {
            RenderResults_PerlinNoise( genStep );
        } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
            RenderResults_RoomsAndPaths( genStep );
        } else if( StringICmp( stepType, "Sprinkle" ) ) {
            RenderResults_Sprinkle( genStep );
        } else {
            ERROR_RECOVERABLE( Stringf( "(EditorMapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }

        ImGui::TreePop();
    }
}


void EditorMapGenStep::RenderResults_BaseClass( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_resultChangelist[genStep];

    if( paramsChanged.empty() ) {
        paramsChanged.resize( NUM_BASE_RESULTS, false );
    }

    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "baseResult";

    RenderChangeText( paramsChanged[0] );
    bool change0 = RenderTileDropDownOrVar( uniqueKey, stepVars, "setType", genStep->m_motifHierarchy, genStep->m_setType );

    RenderChangeText( paramsChanged[1] );
    std::array< bool, 2 > change1and2 = RenderTagsOrVar( uniqueKey, stepVars, "setTags", genStep->m_motifHierarchy, genStep->m_setTags, paramsChanged[2], "Add", "Remove" ); // FIXME: hard coded changed value

    RenderChangeText( paramsChanged[3] );
    bool change3 = RenderHeatMapsOrVar( uniqueKey, stepVars, "setHeatMap", genStep->m_motifHierarchy, genStep->m_setHeatMap );

    RenderChangeText( paramsChanged[4] );
    bool change4 = RenderEventListOrVar( uniqueKey, stepVars, genStep->m_motifHierarchy, genStep->m_customResults, genStep->s_customResults, "Results" );

    paramsChanged[0] = paramsChanged[0] || change0;
    paramsChanged[1] = paramsChanged[1] || change1and2[0];
    paramsChanged[2] = paramsChanged[2] || change1and2[1];
    paramsChanged[3] = paramsChanged[3] || change3;
    paramsChanged[4] = paramsChanged[4] || change4;
}


void EditorMapGenStep::RenderResults_CellularAutomata( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderResults_DistanceField( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderResults_FromImage( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderResults_PerlinNoise( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderResults_RoomsAndPaths( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_resultChangelist[genStep];

    if( paramsChanged.size() <= NUM_BASE_RESULTS ) {
        paramsChanged.resize( NUM_BASE_RESULTS + 10, false );
    }

    MGS_RoomsAndPaths* rnpStep = (MGS_RoomsAndPaths*)genStep;
    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "rnpResult";

    // Rooms
    RenderChangeText( paramsChanged[NUM_BASE_RESULTS] );
    bool change0  = RenderIntRangeOrVar( uniqueKey, stepVars, "count", genStep->m_motifHierarchy, rnpStep->m_numRooms, "Rooms", 1, 50, IntRange::ZERO );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 1] );
    bool change1 = RenderIntRangeOrVar( uniqueKey, stepVars, "width", genStep->m_motifHierarchy, rnpStep->m_roomWidth, "Width in Tiles", 1, 30, IntRange::ZERO );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 2] );
    bool change2 = RenderIntRangeOrVar( uniqueKey, stepVars, "height", genStep->m_motifHierarchy, rnpStep->m_roomHeight, "Height in Tiles", 1, 30, IntRange::ZERO );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 3] );
    bool change3 = RenderTileDropDownOrVar( uniqueKey, stepVars, "roomFloor", genStep->m_motifHierarchy, rnpStep->m_roomFloor, "Room Floor Tiles" );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 4] );
    bool change4 = RenderTileDropDownOrVar( uniqueKey, stepVars, "wall", genStep->m_motifHierarchy, rnpStep->m_roomWall, "Room Wall Tiles" );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 5] );
    bool change5 = RenderIntRangeOrVar( uniqueKey, stepVars, "numOverlaps", genStep->m_motifHierarchy, rnpStep->m_numOverlaps, "Allowed Overlaps", 0, 10, IntRange::ZERO );
    ImGui::Separator();

    // Paths
    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 6] );
    bool change6 = RenderCheckboxOrVar( uniqueKey, stepVars, "loop", genStep->m_motifHierarchy, rnpStep->m_pathLoop, "Make Paths Loop" );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 7] );
    bool change7 = RenderTileDropDownOrVar( uniqueKey, stepVars, "pathFloor", genStep->m_motifHierarchy, rnpStep->m_pathFloor, "Path Tiles" );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 8] );
    bool change8 = RenderIntRangeOrVar( uniqueKey, stepVars, "extraCount", genStep->m_motifHierarchy, rnpStep->m_numExtraPaths, "Extra Paths", 0, 10, IntRange::ZERO );

    RenderChangeText( paramsChanged[NUM_BASE_RESULTS + 9] );
    bool change9 = RenderFloatRangeOrVar( uniqueKey, stepVars, "straightChance", genStep->m_motifHierarchy, rnpStep->m_pathStraightChance, "Path Straightness", 0.f, 1.f, FloatRange::ZERO );

    paramsChanged[NUM_BASE_RESULTS    ]  = paramsChanged[NUM_BASE_RESULTS    ]  || change0;
    paramsChanged[NUM_BASE_RESULTS + 1]  = paramsChanged[NUM_BASE_RESULTS + 1]  || change1;
    paramsChanged[NUM_BASE_RESULTS + 2]  = paramsChanged[NUM_BASE_RESULTS + 2]  || change2;
    paramsChanged[NUM_BASE_RESULTS + 3]  = paramsChanged[NUM_BASE_RESULTS + 3]  || change3;
    paramsChanged[NUM_BASE_RESULTS + 4]  = paramsChanged[NUM_BASE_RESULTS + 4]  || change4;
    paramsChanged[NUM_BASE_RESULTS + 5]  = paramsChanged[NUM_BASE_RESULTS + 5]  || change5;
    paramsChanged[NUM_BASE_RESULTS + 6]  = paramsChanged[NUM_BASE_RESULTS + 6]  ^  change6;
    paramsChanged[NUM_BASE_RESULTS + 7]  = paramsChanged[NUM_BASE_RESULTS + 7]  || change7;
    paramsChanged[NUM_BASE_RESULTS + 8]  = paramsChanged[NUM_BASE_RESULTS + 8]  || change8;
    paramsChanged[NUM_BASE_RESULTS + 9]  = paramsChanged[NUM_BASE_RESULTS + 9]  || change9;
}


void EditorMapGenStep::RenderResults_Sprinkle( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderChildSteps( MapGenStep* genStep, const std::string& stepName ) {
    MGS_Custom* cStep = (MGS_Custom*)genStep;
    bool customChanged = IsChangedCustom( cStep );

    std::string headerStr = Stringf( "Child Steps%s", customChanged ? " *" : "" );
    SetImGuiTextColor( customChanged ? Rgba::ORGANIC_YELLOW : Rgba::WHITE );

    if( ImGui::CollapsingHeader( headerStr.c_str() ) ) {
        std::string stepCustomStr = Stringf( "%s_Custom", stepName.c_str() );
        ImGui::TreePush( stepCustomStr.c_str() );

        // Child steps
        int numChildSteps = (int)cStep->m_genSteps.size();

        for( int stepIndex = 0; stepIndex < numChildSteps; stepIndex++ ) {
            MapGenStep* childStep = cStep->m_genSteps[stepIndex];
            std::string childName = Stringf( "%d. %s [Read-Only]", stepIndex + 1, childStep->GetName().c_str() );

            if( ImGui::CollapsingHeader( childName.c_str() ) ) {
                std::string stepChildStr = Stringf( "%s_%s_Child", stepCustomStr.c_str(), childName.c_str() );
                ImGui::TreePush( stepChildStr.c_str() );
                ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true ); // FIXME: This is using imgui_internal.h

                RenderStepParams( childStep, childName );

                ImGui::PopItemFlag(); // FIXME: This is using imgui_internal.h
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
}


bool EditorMapGenStep::IsChangedConditions( MapGenStep* genStep ) {
    const std::vector< bool >& conditions = s_conditionChangelist[genStep];
    bool isChanged = EngineCommon::VectorContains( conditions, true );
    return isChanged;
}


bool EditorMapGenStep::IsChangedResults( MapGenStep* genStep ) {
    const std::vector< bool >& results = s_resultChangelist[genStep];
    bool isChanged = EngineCommon::VectorContains( results, true );
    return isChanged;
}


bool EditorMapGenStep::IsChangedCustom( MGS_Custom* customStep ) {
    int numChildren = (int)customStep->m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numChildren; stepIndex++ ) {
        MapGenStep* childStep = customStep->m_genSteps[stepIndex];

        if( IsChanged( childStep ) ) {
            return true;
        }
    }

    return false;
}

#endif
