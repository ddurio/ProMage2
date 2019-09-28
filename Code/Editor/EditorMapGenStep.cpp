#include "Editor/EditorMapGenStep.hpp"

#include "Engine/Core/ImGuiSystem.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
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
        std::string stepType = genStep->GetName();

        if( StringICmp( stepType, "Sprinkle" ) ) {
            RenderConditions_Sprinkle( genStep );
        } else if( StringICmp( stepType, "FromImage" ) ) {
            RenderConditions_FromImage( genStep );
        } else if( StringICmp( stepType, "CellularAutomata" ) ) {
            RenderConditions_CellularAutomata( genStep );
        } else if( StringICmp( stepType, "DistanceField" ) ) {
            RenderConditions_DistanceField( genStep );
        } else if( StringICmp( stepType, "PerlinNoise" ) ) {
            RenderConditions_PerlinNoise( genStep );
        } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
            RenderConditions_RoomsAndPaths( genStep );
        } else {
            ERROR_RECOVERABLE( Stringf( "(EditorMapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }
    }
}


void EditorMapGenStep::RenderConditions_BaseClass( MapGenStep* genStep ) {
    // Chance to run
    std::string percentFormat = Stringf( "%.0f%%%%", genStep->m_chanceToRun * 100.f );
    if( ImGui::SliderFloat( "Chance to Run", &(genStep->m_chanceToRun), 0.f, 1.f, percentFormat.c_str() ) ) {
        if( genStep->m_chanceToRun > 1.f ) {
            genStep->m_chanceToRun /= 100.f;
        }
    }

    RenderIntRange( genStep->m_numIterations );
    RenderTileDropDown( genStep->m_ifIsType );
    RenderTags( genStep->m_ifHasTags );
}


void EditorMapGenStep::RenderConditions_CellularAutomata( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderConditions_DistanceField( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderConditions_FromImage( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderConditions_PerlinNoise( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderConditions_RoomsAndPaths( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderConditions_Sprinkle( MapGenStep* genStep ) {

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
}


void EditorMapGenStep::RenderResults_CellularAutomata( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderResults_DistanceField( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderResults_FromImage( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderResults_PerlinNoise( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderResults_RoomsAndPaths( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderResults_Sprinkle( MapGenStep* genStep ) {

}


void EditorMapGenStep::RenderIntRange( IntRange& range ) {
    IntRange initialIters = range;

    if( ImGui::SliderInt2( "Min/Max Iterations", (int*)&(range), 0, 10 ) ) {
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


void EditorMapGenStep::RenderTags( Strings& currentTags ) {
    ImGui::Text( "Has Tag:" );
    ImGui::SameLine();

    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "" );
    }

    std::vector< int > tagsToRemove;
    int numTags = (int)currentTags.size();

    // Required existing tags
    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        ImGui::PushID( tagIndex );
        std::string& tag = currentTags[tagIndex];

        if( tag[0] != '!' ) {
            ImGui::InputText( "", &tag, ImGuiInputTextFlags_CharsNoBlank );
            ImGui::SameLine();

            if( ImGui::Button( "X" ) ) {
                tagsToRemove.push_back( tagIndex );
            }
        }

        ImGui::PopID();
    }

    // Missing Tags
    ImGui::Text( "Missing Tag:" );
    ImGui::SameLine();
    ImGui::PushID( 1 );

    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "!" );
    }

    ImGui::PopID();
    numTags = (int)currentTags.size();

    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        ImGui::PushID( tagIndex );
        std::string& tag = currentTags[tagIndex];

        if( tag[0] == '!' ) {
            std::string subTag = tag.substr( 1 );

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

