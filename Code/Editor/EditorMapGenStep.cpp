#include "Editor/EditorMapGenStep.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Game/MapGen/GenSteps/MGS_CellularAutomata.hpp"
#include "Game/MapGen/GenSteps/MGS_DistanceField.hpp"
#include "Game/MapGen/GenSteps/MGS_FromImage.hpp"
#include "Game/MapGen/GenSteps/MGS_PerlinNoise.hpp"
#include "Game/MapGen/GenSteps/MGS_RoomsAndPaths.hpp"
#include "Game/MapGen/GenSteps/MGS_Sprinkle.hpp"
#include "Game/MapGen/Map/TileDef.hpp"


void EditorMapGenStep::RenderStepParms( MapGenStep* genStep ) {
    if( genStep == nullptr ) {
        return;
    }

    RenderConditions( genStep );
    RenderResults( genStep );
}


void EditorMapGenStep::RenderConditions( MapGenStep* genStep ) {
    if( ImGui::CollapsingHeader( "Conditions", ImGuiTreeNodeFlags_DefaultOpen ) ) {
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
    RenderIntRange( caStep->m_ifNumNeighbors, "Num Neighbors", 0, maxNeighbors );
}


void EditorMapGenStep::RenderConditions_DistanceField( MapGenStep* genStep ) {
    MGS_DistanceField* dfStep = (MGS_DistanceField*)genStep;

    static const Strings movementTypes = MGS_DistanceField::GetMovementTypes();
    std::string initialType = dfStep->m_movementType;

    if( ImGui::BeginCombo( "Movement Type", initialType.c_str(), ImGuiComboFlags_None ) ) {
        for( int typeIndex = 0; typeIndex < movementTypes.size(); typeIndex++ ) {
            ImGui::PushID( typeIndex ); // ThesisFIXME: typeIndex isn't unique

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
}


void EditorMapGenStep::RenderConditions_FromImage( MapGenStep* genStep ) {
    MGS_FromImage* imageStep = (MGS_FromImage*)genStep;

    ImGui::Text( imageStep->m_imageFilePath.c_str() );
    ImGui::SameLine();
    
    if( ImGui::Button( "Open File" ) ) {
        Strings filter = {
            "JPG",  "*.jpg",
            "PNG",  "*.png"
        };

        imageStep->m_imageFilePath = g_theWindow->OpenFileDialog( "Data/Images", filter, "MGS_FromImage: Open File" );
    }

    RenderFloatRange( imageStep->m_alignX, "X Alignment", 0.f, 1.f );
    RenderFloatRange( imageStep->m_alignY, "Y Alignment", 0.f, 1.f );
    RenderIntRange( imageStep->m_numRotations, "Rotations", 0, 3 );
}


void EditorMapGenStep::RenderConditions_PerlinNoise( MapGenStep* genStep ) {
    MGS_PerlinNoise* noiseStep = (MGS_PerlinNoise*)genStep;

    RenderIntRange( noiseStep->m_gridSize, "Grid Size", 1, 50 );
    RenderIntRange( noiseStep->m_numOctaves, "Octaves", 1 );
    RenderFloatRange( noiseStep->m_octavePersistence, "Persistence", 0.f, 1.f );
    RenderFloatRange( noiseStep->m_octaveScale, "Scale", 0.f, 5.f );
}


void EditorMapGenStep::RenderConditions_RoomsAndPaths( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderConditions_Sprinkle( MapGenStep* genStep ) {
    MGS_Sprinkle* sprinkleStep = (MGS_Sprinkle*)genStep;
    RenderIntRange( sprinkleStep->m_countRange, "Sprinkles", 1 );
}


void EditorMapGenStep::RenderResults( MapGenStep* genStep ) {
    if( ImGui::CollapsingHeader( "Results", ImGuiTreeNodeFlags_DefaultOpen ) ) {
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
    RenderIntRange( rnpStep->m_numRooms, "Rooms", 1, 50 );
    RenderIntRange( rnpStep->m_roomWidth, "Width in Tiles", 1, 30 );
    RenderIntRange( rnpStep->m_roomHeight, "Height in Tiles", 1, 30 );
    RenderTileDropDown( "rnpRoomFloor", rnpStep->m_roomFloor, "Room Floor Tiles" );
    RenderTileDropDown( "rnpRoomWall", rnpStep->m_roomWall, "Room Wall Tiles" );
    RenderIntRange( rnpStep->m_numOverlaps, "Allowed Overlaps" );
    ImGui::Separator();

    // Paths
    ImGui::Checkbox( "Make Paths Loop", &rnpStep->m_pathLoop );
    RenderTileDropDown( "rnPPath", rnpStep->m_pathFloor, "Path Tiles" );
    RenderIntRange( rnpStep->m_numExtraPaths, "Extra Paths" );
    RenderFloatRange( rnpStep->m_pathStraightChance, "Path Straightness", 0.f, 1.f );
}


void EditorMapGenStep::RenderResults_Sprinkle( MapGenStep* genStep ) {
    UNUSED( genStep );
}


void EditorMapGenStep::RenderPercent( float& value, const std::string& label /*= ""*/ ) {
    std::string percentFormat = Stringf( "%.0f%%%%", value * 100.f );

    if( ImGui::SliderFloat( label.c_str(), &value, 0.f, 1.f, percentFormat.c_str() ) ) {
        if( value > 1.f ) {
            value /= 100.f;
        }
    }
}


void EditorMapGenStep::RenderIntRange( IntRange& range, const std::string& label /*= ""*/, int minValue /*= 0*/, int maxValue /*= 10 */ ) {
    IntRange initialIters = range;

    if( ImGui::SliderInt2( label.c_str(), (int*)&(range), minValue, maxValue ) ) {
        bool minChanged = (initialIters.min != range.min);

        if( range.min > range.max ) {
            if( minChanged ) {
                range.max = range.min;
            } else {
                range.min = range.max;
            }
        }
    }
}


void EditorMapGenStep::RenderFloatRange( FloatRange& range, const std::string& label /*= ""*/, float minValue /*= 0.f*/, float maxValue /*= 10.f */ ) {
    FloatRange initialIters = range;

    if( ImGui::SliderFloat2( label.c_str(), (float*)&(range), minValue, maxValue ) ) {
        bool minChanged = (initialIters.min != range.min);

        if( range.min > range.max ) {
            if( minChanged ) {
                range.max = range.min;
            } else {
                range.min = range.max;
            }
        }
    }
}


void EditorMapGenStep::RenderTileDropDown( const std::string& uniqueKey, std::string& currentType, const std::string& label /*= "Tile Type" */ ) {
    std::string comboID = Stringf( "tileDD_%s", uniqueKey.c_str() );

    static const Strings tileTypes = TileDef::GetAllTypes();
    std::string initialType = currentType;
    ImGui::PushID( comboID.c_str() );

    if( ImGui::BeginCombo( label.c_str(), initialType.c_str(), ImGuiComboFlags_None ) ) {
        ImGui::PushID( "empty" );

        if( ImGui::Selectable( "<NONE>", (initialType == "") ) ) {
            currentType = "";
        }

        if( initialType == "" ) {
            ImGui::SetItemDefaultFocus();
        }

        ImGui::PopID();

        for( int typeIndex = 0; typeIndex < tileTypes.size(); typeIndex++ ) {
            ImGui::PushID( typeIndex );

            const std::string& defType = tileTypes[typeIndex];
            bool isSelected = StringICmp( initialType, defType );

            if( ImGui::Selectable( defType.c_str(), isSelected ) ) {
                currentType = defType;
            }

            if( isSelected ) {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    ImGui::PopID();
}


void EditorMapGenStep::RenderTags( const std::string& uniqueKey, Strings& currentTags, const std::string& label /*= "" */ ) {
    std::string labelPref = Stringf( "%s%s", label.c_str(), (label == "") ? "" : " " );
    std::string hasTagLabel = Stringf( "%s Has Tag:", labelPref.c_str() );
    std::string missingTagLabel = Stringf( "%s Missing Tag:", labelPref.c_str() );

    std::string hasTagID = Stringf( "hasTag_%s", uniqueKey.c_str() );
    std::string missingTagID = Stringf( "missingTag_%s", uniqueKey.c_str() );

    ImGui::Text( hasTagLabel.c_str() );
    ImGui::SameLine();
    bool focusLastAdd = false;

    ImGui::PushID( hasTagID.c_str() );
    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "" );
        focusLastAdd = true;
    }

    std::vector< int > tagsToRemove;
    int numTags = (int)currentTags.size();

    // Required existing tags
    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        ImGui::PushID( tagIndex );
        std::string& tag = currentTags[tagIndex];

        if( tag[0] != '!' ) {
            if( focusLastAdd && tagIndex == (numTags - 1) ) {
                ImGui::SetKeyboardFocusHere( 0 );
            }

            ImGui::InputText( "", &tag, ImGuiInputTextFlags_CharsNoBlank );
            ImGui::SameLine();

            if( ImGui::Button( "X" ) ) {
                tagsToRemove.push_back( tagIndex );
            }
        }

        ImGui::PopID();
    }

    ImGui::PopID();

    // Missing Tags
    ImGui::Text( missingTagLabel.c_str() );
    ImGui::SameLine();
    ImGui::PushID( missingTagID.c_str() );
    bool focusLastMissing = false;

    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "!" );
        focusLastMissing = true;
    }

    numTags = (int)currentTags.size();

    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        ImGui::PushID( Stringf( "%s_%d", missingTagLabel.c_str(), tagIndex ).c_str() );
        std::string& tag = currentTags[tagIndex];

        if( tag[0] == '!' ) {
            std::string subTag = tag.substr( 1 );

            if( focusLastMissing && tagIndex == (numTags - 1) ) {
                ImGui::SetKeyboardFocusHere( 0 );
            }

            if( ImGui::InputText( "", &subTag, ImGuiInputTextFlags_CharsNoBlank ) ) {
                tag = Stringf( "!%s", subTag.c_str() );
            }

            ImGui::SameLine();

            if( ImGui::Button( "X" ) ) {
                tagsToRemove.push_back( tagIndex );
            }
        }

        ImGui::PopID();
    }

    ImGui::PopID();

    // Remove tags if necessary
    if( !tagsToRemove.empty() ) {
        Strings::iterator tagIter = currentTags.begin();
        int tagIndex = 0;

        while( tagIter != currentTags.end() ) {
            if( EngineCommon::VectorContains( tagsToRemove, tagIndex ) ) {
                tagIter = currentTags.erase( tagIter );
            } else {
                tagIter++;
            }

            tagIndex++;
        }
    }
}


void EditorMapGenStep::RenderHeatMaps( const std::string& uniqueKey, std::map< std::string, FloatRange, StringCmpCaseI >& currentHeatMaps ) {
    ImGui::Text( "Heat Maps" );
    ImGui::SameLine();

    std::string plusButtonID = Stringf( "addHeatMap_Button0_%s", uniqueKey.c_str() );
    std::string addButtonID  = Stringf( "addHeatMap_Button1_%s", uniqueKey.c_str() );
    std::string popupID      = Stringf( "addHeatMap_Popup_%s", uniqueKey.c_str() );

    static std::string heatMapName = "";
    static int newHeatMap = -1;
    bool addHeatMap = false;

    // Add heat map popup
    ImGui::PushID( plusButtonID.c_str() );
    if( ImGui::Button( "+" ) ) {
        heatMapName = "";
        newHeatMap = 2;
        ImGui::OpenPopup( popupID.c_str() );
    }

    if( ImGui::BeginPopup( popupID.c_str() ) ) {
        ImGui::Text( "New Heat Map" );
        ImGui::Separator();

        --newHeatMap;
        if( newHeatMap == 0 ) {
            // Counter required because of ImGui bug #343 -- SKFH doesn't work on first frame of popup existing
            // https://github.com/ocornut/imgui/issues/343
            ImGui::SetKeyboardFocusHere();
        }

        if( ImGui::InputText( "Name", &heatMapName, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue ) ) {
            if( heatMapName != "" ) {
                addHeatMap = true;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        ImGui::PushID( addButtonID.c_str() );
        if( ImGui::Button( "Add" ) ) {
            if( heatMapName != "" ) {
                addHeatMap = true;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::PopID();
        ImGui::EndPopup();
    }

    ImGui::PopID();

    // Add it to the map
    if( addHeatMap ) {
        std::map< std::string, FloatRange, StringCmpCaseI >::iterator heatIter = currentHeatMaps.find( heatMapName );

        if( heatIter == currentHeatMaps.end() ) {
            currentHeatMaps[heatMapName] = FloatRange::ZERO;
        }
    }

    // Show the current heat maps
    std::map< std::string, FloatRange, StringCmpCaseI >::iterator heatIter = currentHeatMaps.begin();
    Strings mapsToRemove;
    int mapIndex = 0;

    while( heatIter != currentHeatMaps.end() ) {
        if( StringICmp( heatIter->first, "Noise" ) ) {
            RenderFloatRange( heatIter->second, heatIter->first, -1.f, 1.f );
        } else {
            RenderFloatRange( heatIter->second, heatIter->first );
        }

        ImGui::SameLine();
        ImGui::PushID( Stringf( "removeHeatMap_%s_%d", uniqueKey.c_str(), mapIndex ).c_str() );

        if( ImGui::Button( "X" ) ) {
            mapsToRemove.push_back( heatIter->first );
        }

        ImGui::PopID();

        mapIndex++;
        heatIter++;
    }

    // Remove maps if necessary
    int numToRemove = (int)mapsToRemove.size();

    for( int removeIndex = 0; removeIndex < numToRemove; removeIndex++ ) {
        const std::string& nameToRemove = mapsToRemove[removeIndex];
        currentHeatMaps.erase( nameToRemove );
    }
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

    ImGui::Text( fullLabel.c_str() );
    ImGui::PushID( addButtonID.c_str() );

    if( !unselectedNames.empty() ) {
        ImGui::SameLine();

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

            ImGui::InputText( name.c_str(), &value );
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
