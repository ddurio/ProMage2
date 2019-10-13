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


void EditorMapGenStep::RenderStepParms( MapGenStep* genStep, const std::string& stepName ) {
    if( genStep == nullptr ) {
        ImGui::Text( "Internal step... No modifiable values" );
        return;
    }

    RenderConditions( genStep, stepName );
    RenderResults( genStep, stepName );
}


void EditorMapGenStep::RenderConditions( MapGenStep* genStep, const std::string& stepName ) {
    SetImGuiTextColor( false );

    if( ImGui::CollapsingHeader( "Conditions", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        std::string stepCondStr = Stringf( "%s_Conditions", stepName.c_str() );
        ImGui::TreePush( stepCondStr.c_str() );

        RenderConditions_BaseClass( genStep );
        ImGui::Separator();

        std::string stepType = genStep->GetName();

        if( StringICmp( stepType, "Sprinkle" ) ) {
            RenderConditions_Sprinkle( genStep );
        } else if( StringICmp( stepType, "CellularAutomata" ) ) {
            RenderConditions_CellularAutomata( genStep );
        } else if( StringICmp( stepType, "DistanceField" ) ) {
            RenderConditions_DistanceField( genStep );
        } else if( StringICmp( stepType, "PerlinNoise" ) ) {
            RenderConditions_PerlinNoise( genStep );
        } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
            RenderConditions_RoomsAndPaths( genStep );
        } else if( StringICmp( stepType, "FromImage" ) ) {
            RenderConditions_FromImage( genStep );
        } else {
            ERROR_RECOVERABLE( Stringf( "(EditorMapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }

        ImGui::TreePop();
    }
}


void EditorMapGenStep::RenderConditions_BaseClass( MapGenStep* genStep ) {
    RenderPercent( genStep->m_chanceToRun, "Chance to Run" );
    RenderIntRange( genStep->m_numIterations, "Iterations" );
    ImGui::Separator();

    RenderTileDropDown( "baseCond", genStep->m_ifIsType );
    RenderTags( "baseCond", genStep->m_ifHasTags, "Tile" );
    RenderHeatMaps( "conditions", genStep->m_ifHeatMap );
    RenderEventList( "Conditions", genStep->s_customConditions, genStep->m_customConditions );
}


void EditorMapGenStep::RenderConditions_CellularAutomata( MapGenStep* genStep ) {
    MGS_CellularAutomata* caStep = (MGS_CellularAutomata*)genStep;

    RenderPercent( caStep->m_chancePerTile, "Chance per Tile" );
    RenderIntRange( caStep->m_radius, "Tile Radius", 1 );
    ImGui::Separator();

    RenderTileDropDown( "neighborCond", caStep->m_ifNeighborType, "Neighbor Type" );
    RenderTags( "caCond", caStep->m_ifNeighborHasTags, "Neighbor" );

    int tileWidth = (2 * caStep->m_radius.max) + 1;
    int maxNeighbors = (tileWidth * tileWidth) - 1;
    RenderIntRange( caStep->m_ifNumNeighbors, "Num Neighbors", 0, maxNeighbors, IntRange( 1, 8 ) );
}


void EditorMapGenStep::RenderConditions_DistanceField( MapGenStep* genStep ) {
    MGS_DistanceField* dfStep = (MGS_DistanceField*)genStep;

    SetImGuiTextColor( false );
    static const Strings movementTypes = MGS_DistanceField::GetMovementTypes();
    std::string initialType = dfStep->m_movementType;

    if( ImGui::BeginCombo( "Movement Type", initialType.c_str(), ImGuiComboFlags_None ) ) {
        for( int typeIndex = 0; typeIndex < movementTypes.size(); typeIndex++ ) {
            ImGui::PushID( typeIndex );

            const std::string& defType = movementTypes[typeIndex];
            bool isSelected = StringICmp( initialType, defType );

            if( ImGui::Selectable( defType.c_str(), isSelected ) ) {
                dfStep->m_movementType = defType;
            }

            if( isSelected ) {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    SetImGuiTextColor( dfStep->m_maxDistance == dfStep->INVALID_DISTANCE );
    ImGui::InputInt( "Max Distance", &dfStep->m_maxDistance );
}


void EditorMapGenStep::RenderConditions_FromImage( MapGenStep* genStep ) {
    MGS_FromImage* imageStep = (MGS_FromImage*)genStep;

    SetImGuiTextColor( false );
    ImGui::Text( imageStep->m_imageFilePath.c_str() );
    ImGui::SameLine();
    
    if( ImGui::Button( "Open File" ) ) {
        Strings filter = {
            "JPG",  "*.jpg",
            "PNG",  "*.png"
        };

        imageStep->m_imageFilePath = g_theWindow->OpenFileDialog( "Data/Images", filter, "MGS_FromImage: Open File" );
    }

    RenderFloatRange( imageStep->m_alignX, "X Alignment", 0.f, 1.f, FloatRange::ZEROTOONE );
    RenderFloatRange( imageStep->m_alignY, "Y Alignment", 0.f, 1.f, FloatRange::ZEROTOONE );
    RenderIntRange( imageStep->m_numRotations, "Rotations", 0, 3, IntRange::ZERO );
}


void EditorMapGenStep::RenderConditions_PerlinNoise( MapGenStep* genStep ) {
    MGS_PerlinNoise* noiseStep = (MGS_PerlinNoise*)genStep;

    RenderIntRange( noiseStep->m_gridSize, "Grid Size", 1, 50, IntRange( 10, 30 ) );
    RenderIntRange( noiseStep->m_numOctaves, "Octaves", 1, 10, IntRange( 1, 3 ) );
    RenderFloatRange( noiseStep->m_octavePersistence, "Persistence", 0.f, 1.f, FloatRange( 0.4f, 0.6f ) );
    RenderFloatRange( noiseStep->m_octaveScale, "Scale", 0.f, 5.f, FloatRange( 1.5f, 2.5f ) );
}


void EditorMapGenStep::RenderConditions_RoomsAndPaths( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderConditions_Sprinkle( MapGenStep* genStep ) {
    MGS_Sprinkle* sprinkleStep = (MGS_Sprinkle*)genStep;
    RenderIntRange( sprinkleStep->m_countRange, "Sprinkles", 1 );
}


void EditorMapGenStep::RenderResults( MapGenStep* genStep, const std::string& stepName ) {
    SetImGuiTextColor( false );

    if( ImGui::CollapsingHeader( "Results", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        std::string stepResultStr = Stringf( "%s_Results", stepName.c_str() );
        ImGui::TreePush( stepResultStr.c_str() );

        RenderResults_BaseClass( genStep );
        ImGui::Separator();

        std::string stepType = genStep->GetName();

        if( StringICmp( stepType, "Sprinkle" ) ) {
            RenderResults_Sprinkle( genStep );
        } else if( StringICmp( stepType, "FromImage" ) ) {
            RenderResults_FromImage( genStep );
        } else if( StringICmp( stepType, "CellularAutomata" ) ) {
            RenderResults_CellularAutomata( genStep );
        } else if( StringICmp( stepType, "DistanceField" ) ) {
            RenderResults_DistanceField( genStep );
        } else if( StringICmp( stepType, "PerlinNoise" ) ) {
            RenderResults_PerlinNoise( genStep );
        } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
            RenderResults_RoomsAndPaths( genStep );
        } else {
            ERROR_RECOVERABLE( Stringf( "(EditorMapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }

        ImGui::TreePop();
    }
}


void EditorMapGenStep::RenderResults_BaseClass( MapGenStep* genStep ) {
    RenderTileDropDown( "baseResult", genStep->m_setType );
    RenderTags( "baseResult", genStep->m_setTags, "Tile" );
    RenderHeatMaps( "results", genStep->m_setHeatMap );
    RenderEventList( "Results", genStep->s_customResults, genStep->m_customResults );
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
    MGS_RoomsAndPaths* rnpStep = (MGS_RoomsAndPaths*)genStep;

    // Rooms
    RenderIntRange( rnpStep->m_numRooms, "Rooms", 1, 50, IntRange::ZERO );
    RenderIntRange( rnpStep->m_roomWidth, "Width in Tiles", 1, 30, IntRange::ZERO );
    RenderIntRange( rnpStep->m_roomHeight, "Height in Tiles", 1, 30, IntRange::ZERO );
    RenderTileDropDown( "rnpRoomFloor", rnpStep->m_roomFloor, "Room Floor Tiles" );
    RenderTileDropDown( "rnpRoomWall", rnpStep->m_roomWall, "Room Wall Tiles" );
    RenderIntRange( rnpStep->m_numOverlaps, "Allowed Overlaps", 0, 10, IntRange::ZERO );
    ImGui::Separator();

    // Paths
    SetImGuiTextColor( rnpStep->m_pathLoop == true );
    ImGui::Checkbox( "Make Paths Loop", &rnpStep->m_pathLoop );
    RenderTileDropDown( "rnPPath", rnpStep->m_pathFloor, "Path Tiles" );
    RenderIntRange( rnpStep->m_numExtraPaths, "Extra Paths", 0, 10, IntRange::ZERO );
    RenderFloatRange( rnpStep->m_pathStraightChance, "Path Straightness", 0.f, 1.f, FloatRange::ZERO );
}


void EditorMapGenStep::RenderResults_Sprinkle( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderEventList( const std::string& label, std::vector< MapGenStep::CustomEvent >& allEvents, std::vector< MapGenStep::CustomEvent >& currentEvents ) {
    std::string fullLabel = Stringf( "Other %s", label.c_str() );
    std::string addButtonID = Stringf( "customEvent_Button_%s", label.c_str() );
    std::string popupID = Stringf( "customEvent_Popup_%s", label.c_str() );

    Strings allNames = GetEventNames( allEvents );

    // Double check something exist
    if( allNames.empty() ) {
        return;
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
        }

        int numAttrs = (int)event.attrNames.size();

        if( event.attrValues.size() != numAttrs ) {
            event.attrValues.resize( numAttrs, "" );
        }

        for( int attrIndex = 0; attrIndex < numAttrs; attrIndex++ ) {
            std::string& name = event.attrNames[attrIndex];
            std::string& value = event.attrValues[attrIndex];

            ImGui::InputText( name.c_str(), &value, ImGuiInputTextFlags_CharsNoBlank );
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
