#if defined(_EDITOR)
#include "Editor/XmlWindow.hpp"

#include "Editor/ImGuiUtils.hpp"

#include "Editor/Editor.hpp"
#include "Editor/EditorMapDef.hpp"
#include "Editor/EditorMapGenStep.hpp"
#include "Editor/MapWindow.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ImGuiSystem.hpp"

#include "ThirdParty/DearImGui/imgui_internal.h"


XmlWindow::XmlWindow( const Vec2& normDimensions /*= Vec2( 0.2f, 1.f )*/, const Vec2& alignment /*= Vec2::ONE */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "XmlEditor";

    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    m_mapType = mapWindow->GetMapType();

    g_theEventSystem->Subscribe( EVENT_EDITOR_GENERATE_MAP, &EditorMapGenStep::ResetChangedParams );
}


XmlWindow::~XmlWindow() {
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_GENERATE_MAP, &EditorMapGenStep::ResetChangedParams );
}


bool XmlWindow::ShouldHighlightTiles() const {
    return m_highlightChanges;
}


void XmlWindow::UpdateChild( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    MapWindow* mapWindow = g_theEditor->GetMapWindow();

    int currentIndex = mapWindow->GetStepIndex();
    std::string mapType = mapWindow->GetMapType();
    EditorMapDef* eMapDef = (EditorMapDef*)EditorMapDef::GetDefinition( mapType ); // Forced to cast 'const'-ness away

    RenderRegenSettings( eMapDef, currentIndex );
    RenderGenSteps( eMapDef );
}


void XmlWindow::RenderRegenSettings( EditorMapDef* eMapDef, int stepIndex  ) {
    if( ImGui::CollapsingHeader( "General Options", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        ImGui::TreePush( "General Options" );

        Strings mapTypes = eMapDef->GetMapTypes();
        RenderDropDown( "emdMapTypes", m_mapType, mapTypes, "Map Type", false, "__HOPEFULLY_NOT_A_MAP_TYPE__" );
        ImGui::Columns( 2, nullptr, true, ImGuiColumnsFlags_NoResize ); // ThesisFIXME: Potentially dangerous.. modified imGui to expose this flag

        if( ImGui::Button( "Regenerate Current Map", ImVec2( -1, 0 ) ) ) {
            MapWindow* mapWindow = g_theEditor->GetMapWindow();
            EventArgs args;

            if( mapWindow->GetMapType() == m_mapType ) {
                args.SetValue( "stepIndex", stepIndex );
            }

            args.SetValue( "mapType", m_mapType );
            args.SetValue( "useCustomSeed", true );
            args.SetValue( "customSeed", mapWindow->GetMapSeed() );

            g_theEventSystem->FireEvent( EVENT_EDITOR_GENERATE_MAP, args );
        }

        ImGui::NextColumn();

        if( ImGui::Button( "Generate New Map", ImVec2( -1, 0 ) ) ) {
            EventArgs args;
            args.SetValue( "mapType", m_mapType );

            MapWindow* mapWindow = g_theEditor->GetMapWindow();
            std::string currentMapType = mapWindow->GetMapType();

            if( currentMapType == m_mapType ) { // Only keep current step if it's the same map type
                args.SetValue( "stepIndex", stepIndex );
            }

            g_theEventSystem->FireEvent( EVENT_EDITOR_GENERATE_MAP, args );
        }

        ImGui::Columns( 1 );
        ImGui::Checkbox( "Highlight Modified Tiles", &m_highlightChanges );

        ImGui::TreePop();
    }
}


void XmlWindow::RenderGenSteps( EditorMapDef* eMapDef ) {
    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    int currentIndex = mapWindow->GetStepIndex();

    // Step params
    Strings stepNames = mapWindow->GetStepNames();

    int numSteps = (int)stepNames.size();
    m_stepHeaderOpen.resize( numSteps, false );

    // Draw headers
    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = eMapDef->GetStep( stepIndex );
        std::string stepName = stepNames[stepIndex];
        SetImGuiTextColor( Rgba::WHITE );

        if( stepIndex != 0 && EditorMapGenStep::IsChanged( genStep ) ) {
            stepName = Stringf( "%s%s", stepName.c_str(), " *" );
            SetImGuiTextColor( Rgba::ORGANIC_YELLOW );
        }

        ImGui::SetNextTreeNodeOpen( (stepIndex == currentIndex) );
        ImGui::PushID( stepName.c_str() );

        if( ImGui::CollapsingHeader( stepName.c_str(), ImGuiTreeNodeFlags_None ) ) {
            ImGui::TreePush( stepName.c_str() );

            if( stepIndex == 0 ) {
                eMapDef->RenderMapDefParams();
            } else {
                EditorMapGenStep::RenderStepParams( genStep, stepName );
            }

            ImGui::TreePop();

            if( stepIndex != currentIndex ) {
                // User opened a new tab.. change map
                m_stepHeaderOpen[currentIndex] = false;

                EventArgs args;
                args.SetValue( "stepIndex", stepIndex );

                g_theEventSystem->FireEvent( EVENT_EDITOR_CHANGE_STEP, args );
            }
        }

        ImGui::PopID();
        RenderContextMenu( eMapDef, stepName, stepIndex, numSteps );
    }
}


void XmlWindow::RenderContextMenu( EditorMapDef* eMapDef, const std::string& guiID, int stepIndex, int numSteps ) {
    if( ImGui::BeginPopupContextItem( guiID.c_str() ) ) {
        if( stepIndex <= 1 ) {
            ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
            SetImGuiTextColor( true );
        }

        // Move Up
        if( ImGui::MenuItem( "Move Up" ) ) {
            eMapDef->ReorderStepUp( stepIndex - 1 );
        }

        if( stepIndex <= 1 ) {
            ImGui::PopItemFlag();
            SetImGuiTextColor( false );
        } else if( stepIndex >= (numSteps - 3) ) {
            ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
            SetImGuiTextColor( true );
        } else {
            SetImGuiTextColor( false );
        }

        // Move Down
        if( ImGui::MenuItem( "Move Down" ) ) {
            eMapDef->ReorderStepDown( stepIndex - 1 );
        }

        if( stepIndex >= (numSteps - 3) ) {
            ImGui::PopItemFlag();
            SetImGuiTextColor( false );
        }

        ImGui::Separator();

        // Insert Before
        if( ImGui::MenuItem( "Insert Before" ) ) {

        }

        // Insert After
        if( ImGui::MenuItem( "Insert After" ) ) {

        }

        ImGui::Separator();
        SetImGuiTextColor( Rgba::ORGANIC_RED );

        // Delete
        if( ImGui::MenuItem( "Delete" ) ) {
            eMapDef->DeleteStep( stepIndex - 1 );
        }

        ImGui::EndPopup();
    }
}

#endif
