#include "Editor/ImGuiUtils.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/MapGen/Map/TileDef.hpp"


bool RenderPercent( float& value, const std::string& label /*= ""*/, float defaultValue /*= 1.f */ ) {
    SetImGuiTextColor( value == defaultValue );
    std::string percentFormat = Stringf( "%.0f%%%%", value * 100.f );

    if( ImGui::SliderFloat( label.c_str(), &value, 0.f, 1.f, percentFormat.c_str() ) ) {
        if( value > 1.f ) {
            value /= 100.f;
        }

        return true;
    }

    return false;
}


bool RenderIntRange( IntRange& range, const std::string& label /*= ""*/, int minValue /*= 0*/, int maxValue /*= 10*/, const IntRange& defaultValue /*= IntRange::ONE */ ) {
    SetImGuiTextColor( range == defaultValue );
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

        return true;
    }

    return false;
}


bool RenderFloatRange( FloatRange& range, const std::string& label /*= ""*/, float minValue /*= 0.f*/, float maxValue /*= 10.f*/, const FloatRange& defaultValue /*= FloatRange::ONE */ ) {
    SetImGuiTextColor( range == defaultValue );
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

        return true;
    }

    return false;
}


bool RenderTileDropDown( const std::string& uniqueKey, std::string& currentType, const std::string& label /*= "Tile Type"*/, bool addNoneOption /*= true*/, const std::string& defaultValue /*= "" */ ) {
    std::string comboID = Stringf( "tile_%s", uniqueKey.c_str() );
    static const Strings tileTypes = TileDef::GetAllTypes();

    return RenderDropDown( comboID, currentType, tileTypes, label, addNoneOption, defaultValue );
}


bool RenderDropDown( const std::string& uniqueKey, std::string& currentType, const Strings& ddOptions, const std::string& label, bool addNoneOptions, const std::string& defaultValue ) {
    bool wasChanged = false;

    SetImGuiTextColor( false );
    std::string initialType = currentType;
    std::string comboID = Stringf( "dropdown_%s", uniqueKey.c_str() );
    ImGui::PushID( comboID.c_str() );

    if( ImGui::BeginCombo( "", initialType.c_str(), ImGuiComboFlags_None ) ) {
        if( addNoneOptions ) {
            ImGui::PushID( "empty" );

            if( ImGui::Selectable( "<NONE>", (initialType == "") ) ) {
                currentType = "";
                wasChanged = true;
            }

            if( initialType == "" ) {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }

        // Loop over all options
        for( int typeIndex = 0; typeIndex < ddOptions.size(); typeIndex++ ) {
            ImGui::PushID( typeIndex );

            const std::string& option = ddOptions[typeIndex];
            bool isSelected = StringICmp( initialType, option );

            if( ImGui::Selectable( option.c_str(), isSelected ) ) {
                currentType = option;
                wasChanged = true;
            }

            if( isSelected ) {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    ImGui::PopID();

    SetImGuiTextColor( currentType == defaultValue );
    ImGui::SameLine();
    ImGui::Text( label.c_str() );

    return wasChanged;
}


std::array< bool, 2> RenderTags( const std::string& uniqueKey, Strings& currentTags, bool missingHasChanged, const std::string& label /*= "" */ ) {
    std::string labelPref = Stringf( "%s%s", label.c_str(), (label == "") ? "" : " " );
    std::string hasTagLabel = Stringf( "%s Has Tag:", labelPref.c_str() );
    std::string missingTagLabel = Stringf( "%s Missing Tag:", labelPref.c_str() );

    std::string hasTagID = Stringf( "hasTag_%s", uniqueKey.c_str() );
    std::string missingTagID = Stringf( "missingTag_%s", uniqueKey.c_str() );
    std::array< bool, 2 > wasChanged = { false, false };

    int numHasTags = 0;
    int numMissingTags = 0;
    int numTags = (int)currentTags.size();

    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        const std::string& tag = currentTags[tagIndex];

        if( tag[0] == '!' ) {
            numMissingTags++;
        } else {
            numHasTags++;
        }

        if( numMissingTags && numHasTags ) {
            break;
        }
    }

    SetImGuiTextColor( numHasTags == 0 );
    ImGui::Text( hasTagLabel.c_str() );
    ImGui::SameLine();

    SetImGuiTextColor( false );
    bool focusLastAdd = false;

    ImGui::PushID( hasTagID.c_str() );
    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "" );
        focusLastAdd = true;
        wasChanged[0] = true;
    }

    std::vector< int > tagsToRemove;
    numTags = (int)currentTags.size();

    // Required existing tags
    for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
        ImGui::PushID( tagIndex );
        std::string& tag = currentTags[tagIndex];

        if( tag[0] != '!' ) {
            if( focusLastAdd && tagIndex == (numTags - 1) ) {
                ImGui::SetKeyboardFocusHere( 0 );
            }

            RenderChangeText( false ); // Always false, but keep the object aligned correctly
            SetImGuiTextColor( Rgba::WHITE );

            if( ImGui::InputText( "", &tag, ImGuiInputTextFlags_CharsNoBlank ) ) {
                wasChanged[0] = true;
            }

            ImGui::SameLine();

            if( ImGui::Button( "X" ) ) {
                tagsToRemove.push_back( tagIndex );
                wasChanged[0] = true;
            }
        }

        ImGui::PopID();
    }

    ImGui::PopID();

    // Missing Tags
    RenderChangeText( missingHasChanged );
    SetImGuiTextColor( numMissingTags == 0 );
    ImGui::Text( missingTagLabel.c_str() );
    ImGui::SameLine();

    SetImGuiTextColor( false );
    ImGui::PushID( missingTagID.c_str() );
    bool focusLastMissing = false;

    if( ImGui::Button( "+" ) ) {
        currentTags.push_back( "!" );
        focusLastMissing = true;
        wasChanged[1] = true;
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

            RenderChangeText( false ); // Always false, but keep the object aligned correctly
            SetImGuiTextColor( Rgba::WHITE );

            if( ImGui::InputText( "", &subTag, ImGuiInputTextFlags_CharsNoBlank ) ) {
                tag = Stringf( "!%s", subTag.c_str() );
                wasChanged[1] = true;
            }

            ImGui::SameLine();

            if( ImGui::Button( "X" ) ) {
                tagsToRemove.push_back( tagIndex );
                wasChanged[1] = true;
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

    return wasChanged;
}


bool RenderHeatMaps( const std::string& uniqueKey, std::map< std::string, FloatRange, StringCmpCaseI >& currentHeatMaps ) {
    SetImGuiTextColor( currentHeatMaps.empty() );
    ImGui::Text( "Heat Maps" );
    ImGui::SameLine();

    std::string plusButtonID = Stringf( "addHeatMap_Button0_%s", uniqueKey.c_str() );
    std::string addButtonID  = Stringf( "addHeatMap_Button1_%s", uniqueKey.c_str() );
    std::string popupID      = Stringf( "addHeatMap_Popup_%s", uniqueKey.c_str() );

    static std::string heatMapName = "";
    static int newHeatMap = -1;
    bool addHeatMap = false;
    bool wasChanged = false;

    // Add heat map popup
    SetImGuiTextColor( false );
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
                wasChanged = true;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        ImGui::PushID( addButtonID.c_str() );
        if( ImGui::Button( "Add" ) ) {
            if( heatMapName != "" ) {
                addHeatMap = true;
                wasChanged = true;
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
            wasChanged |= RenderFloatRange( heatIter->second, heatIter->first, -1.f, 1.f, FloatRange( 1.f, -1.f ) ); // invalid default.. should always be white
        } else {
            wasChanged |= RenderFloatRange( heatIter->second, heatIter->first, 0.f, 10.f, FloatRange( 1.f, -1.f ) );
        }

        ImGui::SameLine();
        ImGui::PushID( Stringf( "removeHeatMap_%s_%d", uniqueKey.c_str(), mapIndex ).c_str() );

        if( ImGui::Button( "X" ) ) {
            mapsToRemove.push_back( heatIter->first );
            wasChanged = true;
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

    return wasChanged;
}


void RenderChangeText( bool hasChanged ) {
    SetImGuiTextColor( Rgba::ORGANIC_YELLOW );

    if( hasChanged ) {
        ImGui::Text( "*" );

        if( ImGui::IsItemHovered() ) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );

            std::string changeStr = "Value has changed since map generation.";
            ImGui::TextUnformatted( changeStr.c_str() );

            std::string regenStr = "Please regenerate map to apply these changes.";
            ImGui::TextUnformatted( regenStr.c_str() );

            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    } else {
        ImGui::Text( " " );
    }

    ImGui::SameLine();
}


void SetImGuiTextColor( bool isDefaultValue ) {
    Rgba textColor = isDefaultValue ? Rgba::ORGANIC_GRAY : Rgba::WHITE;
    SetImGuiTextColor( textColor );
}


void SetImGuiTextColor( const Rgba& newColor ) {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = newColor.GetAsImGui();
}


bool RenderImageButton( const char* textureStr, const ImVec2& buttonSize, const Rgba& bgColor /*= Rgba::CLEAR_BLACK*/ ) {
    const TextureView2D* imageView = g_theRenderer->GetOrCreateTextureView2D( textureStr );
    void* shaderView = imageView->GetShaderView();

    bool wasPressed = ImGui::ImageButton( shaderView, buttonSize, Vec2::ZERO.GetAsImGui(), Vec2::ONE.GetAsImGui(), -1, bgColor.GetAsImGui() );
    return wasPressed;
}
