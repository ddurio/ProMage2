#include "Editor/EditorMapGenStep.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
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
    RenderTileDropDown( genStep->m_ifIsType );
    RenderTags( genStep->m_ifHasTags, "Tile" );
    RenderHeatMaps( genStep->m_ifHeatMap );
}


void EditorMapGenStep::RenderConditions_CellularAutomata( MapGenStep* genStep ) {
    MGS_CellularAutomata* caStep = (MGS_CellularAutomata*)genStep;

    RenderPercent( caStep->m_chancePerTile, "Chance per Tile" );
    RenderIntRange( caStep->m_radius, "Tile Radius", 1 );
    ImGui::Separator();
    RenderTileDropDown( caStep->m_ifNeighborType, "Neighbor Type" );
    RenderTags( caStep->m_ifNeighborHasTags, "Neighbor" );

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

}


void EditorMapGenStep::RenderConditions_RoomsAndPaths( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderConditions_Sprinkle( MapGenStep* genStep ) {
    MGS_Sprinkle* sprinkleStep = (MGS_Sprinkle*)genStep;
    RenderIntRange( sprinkleStep->m_countRange, "Sprinkles", 1 );
}


void EditorMapGenStep::RenderResults( MapGenStep* genStep ) {
    if( ImGui::CollapsingHeader( "Results", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        RenderResults_BaseClass( genStep );
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
    RenderTileDropDown( genStep->m_setType );
    RenderTags( genStep->m_setTags );
    RenderHeatMaps( genStep->m_setHeatMap );
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

}


void EditorMapGenStep::RenderResults_RoomsAndPaths( MapGenStep* genStep ) {

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


void EditorMapGenStep::RenderTileDropDown( std::string& currentType, const std::string& label /*= "Tile Type" */ ) {
    static const Strings tileTypes = TileDef::GetAllTypes();
    std::string initialType = currentType;

    if( ImGui::BeginCombo( label.c_str(), initialType.c_str(), ImGuiComboFlags_None ) ) {
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
}


void EditorMapGenStep::RenderTags( Strings& currentTags, const std::string& label /*= "" */ ) {
    std::string labelPref = Stringf( "%s%s", label.c_str(), (label == "") ? "" : " " );
    std::string hasTagLabel = Stringf( "%sHas Tag:", labelPref.c_str() );
    std::string missingTagLabel = Stringf( "%sMissing Tag:", labelPref.c_str() );

    ImGui::Text( hasTagLabel.c_str() );
    ImGui::SameLine();
    bool focusLastAdd = false;

    ImGui::PushID( "addHasTag" );
    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "" );
        focusLastAdd = true;
    }
    ImGui::PopID();

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

    // Missing Tags
    ImGui::Text( missingTagLabel.c_str() );
    ImGui::SameLine();
    ImGui::PushID( "addMissingTag" );
    bool focusLastMissing = false;

    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "!" );
        focusLastMissing = true;
    }

    ImGui::PopID();
    numTags = (int)currentTags.size();

    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        ImGui::PushID( tagIndex );
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

    // Remove tags if necessary
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


void EditorMapGenStep::RenderHeatMaps( std::map< std::string, FloatRange, StringCmpCaseI >& currentHeatMaps ) {
    ImGui::Text( "Heat Maps" );
    ImGui::SameLine();

    static std::string heatMapName = "";
    bool addHeatMap = false;

    // Add heat map popup
    ImGui::PushID( "addNewHeatMap" );
    if( ImGui::Button( "+" ) ) {
        heatMapName = "";
        ImGui::OpenPopup( "addHeatMap" );
    }

    if( ImGui::BeginPopup( "addHeatMap" ) ) {
        ImGui::Text( "New Heat Map" );
        ImGui::Separator();

        if( heatMapName == "" ) {
            // Focus prevents button from being pressed
            ImGui::SetKeyboardFocusHere();
        }

        if( ImGui::InputText( "Name", &heatMapName, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue )
            && heatMapName != "" ) {

            addHeatMap = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if( ImGui::Button( "Add" ) ) {
            addHeatMap = true;
            ImGui::CloseCurrentPopup();
        }

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
        RenderFloatRange( heatIter->second, heatIter->first );
        ImGui::SameLine();
        ImGui::PushID( mapIndex );

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
