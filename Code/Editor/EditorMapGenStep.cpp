#include "Editor/EditorMapGenStep.hpp"

#include "Editor/ImGuiUtils.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Game/MapGen/GenSteps/MGS_CellularAutomata.hpp"
#include "Game/MapGen/GenSteps/MGS_DistanceField.hpp"
#include "Game/MapGen/GenSteps/MGS_FromImage.hpp"
#include "Game/MapGen/GenSteps/MGS_PerlinNoise.hpp"
#include "Game/MapGen/GenSteps/MGS_RoomsAndPaths.hpp"
#include "Game/MapGen/GenSteps/MGS_Sprinkle.hpp"
#include "Game/MapGen/Map/TileDef.hpp"


std::map< MapGenStep*, std::vector< bool > > EditorMapGenStep::s_conditionChangelist;   // PRIVATE
std::map< MapGenStep*, std::vector< bool > > EditorMapGenStep::s_resultChangelist;      // PRIVATE


void EditorMapGenStep::RenderStepParams( MapGenStep* genStep, const std::string& stepName ) {
    if( genStep == nullptr ) {
        ImGui::Text( "Internal step... No modifiable values" );
        return;
    }

    RenderConditions( genStep, stepName );
    RenderResults( genStep, stepName );
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


void EditorMapGenStep::FindXmlMotifVariables( const XMLElement& element, MapGenStep* genStep ) {
    // First do base class
    FindXmlMotifVariables_BaseClass( element, genStep );

    // Then do child class
    std::string stepType = element.Name();

    if( StringICmp( stepType, "CellularAutomata" ) ) {
        FindXmlMotifVariables_CellularAutomata( element, genStep );
    } else if( StringICmp( stepType, "DistanceField" ) ) {
        FindXmlMotifVariables_DistanceField( element, genStep );
    } else if( StringICmp( stepType, "FromImage" ) ) {
        FindXmlMotifVariables_FromImage( element, genStep );
    } else if( StringICmp( stepType, "PerlinNoise" ) ) {
        FindXmlMotifVariables_PerlinNoise( element, genStep );
    } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
        FindXmlMotifVariables_RoomsAndPaths( element, genStep );
    } else if( StringICmp( stepType, "Sprinkle" ) ) {
        FindXmlMotifVariables_Sprinkle( element, genStep );
    } else {
        ERROR_RECOVERABLE( Stringf( "(MapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
    }
}


// PRIVATE ----------------------------------------------------------------------
void EditorMapGenStep::RenderConditions( MapGenStep* genStep, const std::string& stepName ) {
    bool condChanged = IsChangedConditions( genStep );
    std::string headerStr = Stringf( "Conditions%s", condChanged ? " *" : "" );
    SetImGuiTextColor( condChanged ? Rgba::ORGANIC_YELLOW : Rgba::WHITE );

    if( ImGui::CollapsingHeader( headerStr.c_str(), ImGuiTreeNodeFlags_DefaultOpen ) ) {
        std::string stepCondStr = Stringf( "%s_Conditions", stepName.c_str() );
        ImGui::TreePush( stepCondStr.c_str() );

        RenderConditions_BaseClass( genStep );
        ImGui::Separator();

        std::string stepType = genStep->GetName();

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
        } else {
            ERROR_RECOVERABLE( Stringf( "(EditorMapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }

        ImGui::TreePop();
    }
}


void EditorMapGenStep::RenderConditions_BaseClass( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.empty() ) {
        paramsChanged.resize( 7, false );
    }

    std::vector< bool > localChanges;
    localChanges.resize( 7, false );

    NamedStrings& stepVars = genStep->m_motifVars;
    std::string uniqueKey = "baseCond";

    RenderChangeText( paramsChanged[0] );
    localChanges[0] = RenderPercentOrVar( uniqueKey, stepVars, "chanceToRun", genStep->m_motifHeirarchy, genStep->m_chanceToRun, "Chance To Run" );

    RenderChangeText( paramsChanged[1] );
    localChanges[1] = RenderIntRange( genStep->m_numIterations, "Iterations" );
    ImGui::Separator();

    RenderChangeText( paramsChanged[2] );
    localChanges[2] = RenderTileDropDown( uniqueKey, genStep->m_ifIsType );

    RenderChangeText( paramsChanged[3] );
    std::array< bool, 2 > change3and4 = RenderTags( uniqueKey, genStep->m_ifHasTags, paramsChanged[4], "Tile" );

    RenderChangeText( paramsChanged[5] );
    localChanges[5] = RenderHeatMaps( "conditions", genStep->m_ifHeatMap );

    RenderChangeText( paramsChanged[6] );
    localChanges[6] = RenderEventList( "Conditions", genStep->s_customConditions, genStep->m_customConditions );

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

    if( paramsChanged.size() <= 7 ) {
        paramsChanged.resize( 7 + 6, false );
    }

    MGS_CellularAutomata* caStep = (MGS_CellularAutomata*)genStep;

    RenderChangeText( paramsChanged[7] );
    bool change7 = RenderPercent( caStep->m_chancePerTile, "Chance per Tile" );

    RenderChangeText( paramsChanged[8] );
    bool change8 = RenderIntRange( caStep->m_radius, "Tile Radius", 1 );
    ImGui::Separator();

    RenderChangeText( paramsChanged[9] );
    bool change9 = RenderTileDropDown( "neighborCond", caStep->m_ifNeighborType, "Neighbor Type" );

    RenderChangeText( paramsChanged[10] );
    std::array< bool, 2 > change10and11 = RenderTags( "caCond", caStep->m_ifNeighborHasTags, paramsChanged[11], "Neighbor" ); // FIXME: hard coded changed value

    int tileWidth = (2 * caStep->m_radius.max) + 1;
    int maxNeighbors = (tileWidth * tileWidth) - 1;

    RenderChangeText( paramsChanged[12] );
    bool change12 = RenderIntRange( caStep->m_ifNumNeighbors, "Num Neighbors", 0, maxNeighbors, IntRange( 1, 8 ) );

    paramsChanged[7]  = paramsChanged[7]  || change7;
    paramsChanged[8]  = paramsChanged[8]  || change8;
    paramsChanged[9]  = paramsChanged[9]  || change9;
    paramsChanged[10] = paramsChanged[10] || change10and11[0];
    paramsChanged[11] = paramsChanged[11] || change10and11[1];
    paramsChanged[12] = paramsChanged[12] || change12;
}


void EditorMapGenStep::RenderConditions_DistanceField( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= 7 ) {
        paramsChanged.resize( 7 + 2, false );
    }

    MGS_DistanceField* dfStep = (MGS_DistanceField*)genStep;

    static const Strings movementTypes = MGS_DistanceField::GetMovementTypes();
    std::string initialType = dfStep->m_movementType;

    RenderChangeText( paramsChanged[7] );
    bool change7 = false;
    SetImGuiTextColor( false );

    if( ImGui::BeginCombo( "Movement Type", initialType.c_str(), ImGuiComboFlags_None ) ) {
        for( int typeIndex = 0; typeIndex < movementTypes.size(); typeIndex++ ) {
            ImGui::PushID( typeIndex );

            const std::string& defType = movementTypes[typeIndex];
            bool isSelected = StringICmp( initialType, defType );

            if( ImGui::Selectable( defType.c_str(), isSelected ) ) {
                dfStep->m_movementType = defType;

                if( !isSelected ) {
                    change7 = true;
                }
            }

            if( isSelected ) {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    RenderChangeText( paramsChanged[8] );
    SetImGuiTextColor( dfStep->m_maxDistance == dfStep->INVALID_DISTANCE );
    bool change8 = ImGui::InputInt( "Max Distance", &dfStep->m_maxDistance );

    paramsChanged[7] = paramsChanged[7] || change7;
    paramsChanged[8] = paramsChanged[8] || change8;
}


void EditorMapGenStep::RenderConditions_FromImage( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= 7 ) {
        paramsChanged.resize( 7 + 4, false );
    }

    MGS_FromImage* imageStep = (MGS_FromImage*)genStep;

    RenderChangeText( paramsChanged[7] );
    SetImGuiTextColor( false );
    ImGui::Text( imageStep->m_imageFilePath.c_str() );
    ImGui::SameLine();
    bool change7 = false;
    
    if( ImGui::Button( "Open File" ) ) {
        Strings filter = {
            "JPG",  "*.jpg",
            "PNG",  "*.png"
        };

        std::string newFilePath = g_theWindow->OpenFileDialog( "Data/Images", filter, "MGS_FromImage: Open File" );

        if( imageStep->m_imageFilePath != newFilePath ) {
            change7 = true;
        }

        imageStep->m_imageFilePath = newFilePath;
    }

    RenderChangeText( paramsChanged[8] );
    bool change8  = RenderFloatRange( imageStep->m_alignX, "X Alignment", 0.f, 1.f, FloatRange::ZEROTOONE );

    RenderChangeText( paramsChanged[9] );
    bool change9  = RenderFloatRange( imageStep->m_alignY, "Y Alignment", 0.f, 1.f, FloatRange::ZEROTOONE );

    RenderChangeText( paramsChanged[10] );
    bool change10 = RenderIntRange( imageStep->m_numRotations, "Rotations", 0, 3, IntRange::ZERO );

    paramsChanged[7]  = paramsChanged[7]  || change7;
    paramsChanged[8]  = paramsChanged[8]  || change8;
    paramsChanged[9]  = paramsChanged[9]  || change9;
    paramsChanged[10] = paramsChanged[10] || change10;
}


void EditorMapGenStep::RenderConditions_PerlinNoise( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= 7 ) {
        paramsChanged.resize( 7 + 4, false );
    }

    MGS_PerlinNoise* noiseStep = (MGS_PerlinNoise*)genStep;

    RenderChangeText( paramsChanged[7] );
    bool change7  = RenderIntRange( noiseStep->m_gridSize, "Grid Size", 1, 50, IntRange( 10, 30 ) );

    RenderChangeText( paramsChanged[8] );
    bool change8  = RenderIntRange( noiseStep->m_numOctaves, "Octaves", 1, 10, IntRange( 1, 3 ) );

    RenderChangeText( paramsChanged[9] );
    bool change9  = RenderFloatRange( noiseStep->m_octavePersistence, "Persistence", 0.f, 1.f, FloatRange( 0.4f, 0.6f ) );

    RenderChangeText( paramsChanged[10] );
    bool change10 = RenderFloatRange( noiseStep->m_octaveScale, "Scale", 0.f, 5.f, FloatRange( 1.5f, 2.5f ) );

    paramsChanged[7]  = paramsChanged[7]  || change7;
    paramsChanged[8]  = paramsChanged[8]  || change8;
    paramsChanged[9]  = paramsChanged[9]  || change9;
    paramsChanged[10] = paramsChanged[10] || change10;
}


void EditorMapGenStep::RenderConditions_RoomsAndPaths( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderConditions_Sprinkle( MapGenStep* genStep ) {
    std::vector< bool >& paramsChanged = s_conditionChangelist[genStep];

    if( paramsChanged.size() <= 7 ) {
        paramsChanged.resize( 7 + 1, false );
    }

    MGS_Sprinkle* sprinkleStep = (MGS_Sprinkle*)genStep;

    RenderChangeText( paramsChanged[7] );
    bool change7 = RenderIntRange( sprinkleStep->m_countRange, "Sprinkles", 1 );

    paramsChanged[7] = paramsChanged[7] || change7;
}


void EditorMapGenStep::RenderResults( MapGenStep* genStep, const std::string& stepName ) {
    bool resultsChanged = IsChangedResults( genStep );
    std::string headerStr = Stringf( "Results%s", resultsChanged ? " *" : "" );
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
        paramsChanged.resize( 5, false );
    }

    RenderChangeText( paramsChanged[0] );
    bool change0 = RenderTileDropDown( "baseResult", genStep->m_setType );

    RenderChangeText( paramsChanged[1] );
    std::array< bool, 2 > change1and2 = RenderTags( "baseResult", genStep->m_setTags, paramsChanged[2], "Tile" ); // FIXME: hard coded changed value

    RenderChangeText( paramsChanged[3] );
    bool change3 = RenderHeatMaps( "results", genStep->m_setHeatMap );

    RenderChangeText( paramsChanged[4] );
    bool change4 = RenderEventList( "Results", genStep->s_customResults, genStep->m_customResults );

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

    if( paramsChanged.size() <= 5 ) {
        paramsChanged.resize( 5 + 10, false );
    }

    MGS_RoomsAndPaths* rnpStep = (MGS_RoomsAndPaths*)genStep;

    // Rooms
    RenderChangeText( paramsChanged[5] );
    bool change5  = RenderIntRange( rnpStep->m_numRooms, "Rooms", 1, 50, IntRange::ZERO );

    RenderChangeText( paramsChanged[6] );
    bool change6  = RenderIntRange( rnpStep->m_roomWidth, "Width in Tiles", 1, 30, IntRange::ZERO );

    RenderChangeText( paramsChanged[7] );
    bool change7  = RenderIntRange( rnpStep->m_roomHeight, "Height in Tiles", 1, 30, IntRange::ZERO );

    RenderChangeText( paramsChanged[8] );
    bool change8  = RenderTileDropDown( "rnpRoomFloor", rnpStep->m_roomFloor, "Room Floor Tiles" );

    RenderChangeText( paramsChanged[9] );
    bool change9  = RenderTileDropDown( "rnpRoomWall", rnpStep->m_roomWall, "Room Wall Tiles" );

    RenderChangeText( paramsChanged[10] );
    bool change10 = RenderIntRange( rnpStep->m_numOverlaps, "Allowed Overlaps", 0, 10, IntRange::ZERO );
    ImGui::Separator();

    // Paths
    RenderChangeText( paramsChanged[11] );
    SetImGuiTextColor( rnpStep->m_pathLoop == true );
    bool change11 = ImGui::Checkbox( "Make Paths Loop", &rnpStep->m_pathLoop );

    RenderChangeText( paramsChanged[12] );
    bool change12 = RenderTileDropDown( "rnPPath", rnpStep->m_pathFloor, "Path Tiles" );

    RenderChangeText( paramsChanged[13] );
    bool change13 = RenderIntRange( rnpStep->m_numExtraPaths, "Extra Paths", 0, 10, IntRange::ZERO );

    RenderChangeText( paramsChanged[14] );
    bool change14 = RenderFloatRange( rnpStep->m_pathStraightChance, "Path Straightness", 0.f, 1.f, FloatRange::ZERO );

    paramsChanged[5]  = paramsChanged[5]  || change5;
    paramsChanged[6]  = paramsChanged[6]  || change6;
    paramsChanged[7]  = paramsChanged[7]  || change7;
    paramsChanged[8]  = paramsChanged[8]  || change8;
    paramsChanged[9]  = paramsChanged[9]  || change9;
    paramsChanged[10] = paramsChanged[10] || change10;
    paramsChanged[11] = paramsChanged[11] ^  change11; // XOR for checkbox
    paramsChanged[12] = paramsChanged[12] || change12;
    paramsChanged[13] = paramsChanged[13] || change13;
    paramsChanged[14] = paramsChanged[14] || change14;
}


void EditorMapGenStep::RenderResults_Sprinkle( MapGenStep* genStep ) {
    UNUSED( genStep );
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


bool EditorMapGenStep::RenderEventList( const std::string& label, std::vector< MapGenStep::CustomEvent >& allEvents, std::vector< MapGenStep::CustomEvent >& currentEvents ) {
    std::string fullLabel = Stringf( "Other %s", label.c_str() );
    std::string addButtonID = Stringf( "customEvent_Button_%s", label.c_str() );
    std::string popupID = Stringf( "customEvent_Popup_%s", label.c_str() );

    Strings allNames = GetEventNames( allEvents );

    // Double check something exist
    if( allNames.empty() ) {
        return false;
    }

    Strings currentNames = GetEventNames( currentEvents );
    Strings unselectedNames;
    Strings::iterator nameIter = allNames.begin();

    while( nameIter != allNames.end() ) {
        if( !EngineCommon::VectorContains( currentNames, *nameIter ) &&
            !StringICmp( *nameIter, "ChangeTile" ) ) {

            unselectedNames.push_back( *nameIter );
        }

        nameIter++;
    }

    // Start drawing
    std::string newName = "";
    bool wasChanged = false;

    SetImGuiTextColor( currentEvents.size() <= 1 );
    ImGui::Text( fullLabel.c_str() );
    ImGui::PushID( addButtonID.c_str() );

    if( !unselectedNames.empty() ) {
        ImGui::SameLine();
        SetImGuiTextColor( false );

        if( ImGui::Button( "+" ) ) {
            ImGui::OpenPopup( popupID.c_str() );
        }
    }

    // Add event pop up
    if( ImGui::BeginPopup( popupID.c_str() ) ) {
        ImGui::Text( label.c_str() );
        ImGui::Separator();

        int numNames = (int)unselectedNames.size();

        for( int nameIndex = 0; nameIndex < numNames; nameIndex++ ) {
            bool unused = false;

            if( ImGui::Selectable( unselectedNames[nameIndex].c_str(), &unused ) ) {
                newName = unselectedNames[nameIndex];
                wasChanged = true;
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();

    // Add new name if necessary
    if( newName != "" ) {
        int numEvents = (int)allEvents.size();

        for( int eventIndex = 0; eventIndex < numEvents; eventIndex++ ) {
            if( StringICmp( allEvents[eventIndex].name, newName ) ) {
                currentEvents.push_back( allEvents[eventIndex] );
                break;
            }
        }
    }

    // Draw current events
    std::vector< int > eventsToRemove;
    int numEvents = (int)currentEvents.size();

    for( int eventIndex = 0; eventIndex < numEvents; eventIndex++ ) {
        MapGenStep::CustomEvent& event = currentEvents[eventIndex];

        if( StringICmp( event.name, "ChangeTile" ) ) {
            // Used by editor.. no need to show this
            continue;
        }

        if( eventIndex > 0 ) {
            ImGui::Separator();
        }

        ImGui::Text( event.name.c_str() );
        ImGui::SameLine();
        ImGui::PushID( eventIndex );
        
        if( ImGui::Button( "X" ) ) {
            eventsToRemove.push_back( eventIndex );
            wasChanged = true;
        }

        int numAttrs = (int)event.attrNames.size();

        if( event.attrValues.size() != numAttrs ) {
            event.attrValues.resize( numAttrs, "" );
        }

        for( int attrIndex = 0; attrIndex < numAttrs; attrIndex++ ) {
            std::string& name = event.attrNames[attrIndex];
            std::string& value = event.attrValues[attrIndex];

            if( ImGui::InputText( name.c_str(), &value, ImGuiInputTextFlags_CharsNoBlank ) ) {
                wasChanged = true;
            }
        }

        ImGui::PopID();
    }

    // Remove events if necessary
    if( !eventsToRemove.empty() ) {
        std::vector< MapGenStep::CustomEvent >::iterator currentIter = currentEvents.begin();
        int eventIndex = 0;

        while( currentIter != currentEvents.end() ) {
            if( EngineCommon::VectorContains( eventsToRemove, eventIndex ) ) {
                currentIter = currentEvents.erase( currentIter );
            } else {
                currentIter++;
            }

            eventIndex++;
        }
    }

    return wasChanged;
}


bool EditorMapGenStep::IsChangedConditions( MapGenStep* genStep ) {
    const std::vector< bool >& conditions = s_conditionChangelist[genStep];
    int numParms = (int)conditions.size();

    for( int paramIndex = 0; paramIndex < numParms; paramIndex++ ) {
        if( conditions[paramIndex] ) {
            return true;
        }
    }

    return false;
}


bool EditorMapGenStep::IsChangedResults( MapGenStep* genStep ) {
    const std::vector< bool >& results = s_resultChangelist[genStep];
    int numParms = (int)results.size();

    for( int paramIndex = 0; paramIndex < numParms; paramIndex++ ) {
        if( results[paramIndex] ) {
            return true;
        }
    }

    return false;
}


void EditorMapGenStep::FindXmlMotifVariables_BaseClass( const XMLElement& element, MapGenStep* genStep ) {
    NamedStrings& stepVars = genStep->m_motifVars;

    GetXMLMotifVariable( element, "chanceToRun",    stepVars );
    GetXMLMotifVariable( element, "numIterations",  stepVars );
    GetXMLMotifVariable( element, "ifIsType",       stepVars );
    GetXMLMotifVariable( element, "ifHasTags",      stepVars ); // ThesisFIXME: this is wrong for tags
    int numCondEvents = (int)genStep->m_customConditions.size();

    for( int condIndex = 0; condIndex < numCondEvents; condIndex++ ) {
        const MapGenStep::CustomEvent&  cEvent = genStep->m_customConditions[condIndex];
        int numNames = (int)cEvent.attrNames.size();

        for( int nameIndex = 0; nameIndex < numNames; nameIndex++ ) {
            const std::string& attrName = cEvent.attrNames[nameIndex];
            GetXMLMotifVariable( element, attrName.c_str(), stepVars );
        }
    }

    HeatMaps::const_iterator heatIter = genStep->m_ifHeatMap.begin();

    for( heatIter; heatIter != genStep->m_ifHeatMap.end(); heatIter++ ) {
        std::string attrName = Stringf( "ifHeatMap%s", heatIter->first.c_str() );
        GetXMLMotifVariable( element, attrName.c_str(), stepVars );
    }

    GetXMLMotifVariable( element, "setType", stepVars );
    GetXMLMotifVariable( element, "setTags", stepVars ); // ThesisFIXME: this is wrong for tags
    int numResultEvents = (int)genStep->m_customResults.size();

    for( int resultIndex = 0; resultIndex < numResultEvents; resultIndex++ ) {
        const MapGenStep::CustomEvent&  cEvent = genStep->m_customResults[resultIndex];
        int numNames = (int)cEvent.attrNames.size();

        for( int nameIndex = 0; nameIndex < numNames; nameIndex++ ) {
            const std::string& attrName = cEvent.attrNames[nameIndex];
            GetXMLMotifVariable( element, attrName.c_str(), stepVars );
        }
    }

    heatIter = genStep->m_setHeatMap.begin();

    for( heatIter; heatIter != genStep->m_setHeatMap.end(); heatIter++ ) {
        std::string attrName = Stringf( "setHeatMap%s", heatIter->first.c_str() );
        GetXMLMotifVariable( element, attrName.c_str(), stepVars );
    }
}


void EditorMapGenStep::FindXmlMotifVariables_CellularAutomata( const XMLElement& element, MapGenStep* genStep ) {

}


void EditorMapGenStep::FindXmlMotifVariables_DistanceField( const XMLElement& element, MapGenStep* genStep ) {

}


void EditorMapGenStep::FindXmlMotifVariables_FromImage( const XMLElement& element, MapGenStep* genStep ) {

}


void EditorMapGenStep::FindXmlMotifVariables_PerlinNoise( const XMLElement& element, MapGenStep* genStep ) {

}


void EditorMapGenStep::FindXmlMotifVariables_RoomsAndPaths( const XMLElement& element, MapGenStep* genStep ) {

}


void EditorMapGenStep::FindXmlMotifVariables_Sprinkle( const XMLElement& element, MapGenStep* genStep ) {

}

