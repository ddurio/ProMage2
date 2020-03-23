#if defined(_EDITOR)
#include "Editor/XmlWindow.hpp"

#include "Editor/ImGuiUtils.hpp"

#include "Editor/Editor.hpp"
#include "Editor/EditorMapDef.hpp"
#include "Editor/EditorMapGenStep.hpp"
#include "Editor/MapWindow.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"

#include "ThirdParty/DearImGui/imgui_internal.h"


XmlWindow::XmlWindow( const Vec2& normDimensions /*= Vec2( 0.2f, 1.f )*/, const Vec2& alignment /*= Vec2::ONE */ ) :
    EditorWindow( normDimensions, alignment ) {
    m_windowName = "XML Editor";

    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    m_mapType = mapWindow->GetMapType();

    g_theEventSystem->Subscribe( EVENT_EDITOR_GENERATE_MAP, &EditorMapGenStep::ResetChangedParams );
    g_theEventSystem->Subscribe( EVENT_EDITOR_GENERATE_MAP, this, &XmlWindow::ChangeMapType );

    Strings customStepNames = MGS_CustomDef::GetAllDefinitionTypes();
    m_stepTypes.insert( m_stepTypes.end(), customStepNames.begin(), customStepNames.end() );
}


XmlWindow::~XmlWindow() {
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_GENERATE_MAP, &EditorMapGenStep::ResetChangedParams );
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_GENERATE_MAP, this, &XmlWindow::ChangeMapType );
}


bool XmlWindow::ShouldHighlightTiles() const {
    return m_highlightChanges;
}


void XmlWindow::TriggerMapGen( const std::string& mapType, int stepIndex, bool useCurrentSeed /*= false*/ ) {
    MapWindow* mapWindow = g_theEditor->GetMapWindow();
    EventArgs args;

    if( mapWindow->GetMapType() == mapType ) {
        args.SetValue( "stepIndex", stepIndex );
    }

    args.SetValue( "mapType", mapType );
    args.SetValue( "useCustomSeed", useCurrentSeed );
    args.SetValue( "customSeed", mapWindow->GetMapSeed() );

    g_theEventSystem->FireEvent( EVENT_EDITOR_GENERATE_MAP, args );
}


bool XmlWindow::UpdateChild( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    MapWindow* mapWindow = g_theEditor->GetMapWindow();

    int currentIndex = mapWindow->GetStepIndex();
    EditorMapDef* eMapDef = (EditorMapDef*)EditorMapDef::GetDefinition( m_mapType ); // Forced to cast 'const'-ness away

    bool regenTriggered = RenderRegenSettings( eMapDef, currentIndex );

    if( regenTriggered ) {
        return true;
    }

    RenderGenSteps( eMapDef );
    return false;
}


bool XmlWindow::RenderRegenSettings( EditorMapDef* eMapDef, int stepIndex  ) {
    bool regenTriggered = false;

    if( ImGui::CollapsingHeader( "General Options", ImGuiTreeNodeFlags_DefaultOpen ) ) {
        ImGui::TreePush( "General Options" );

        Strings mapTypes = eMapDef->GetMapTypes();
        bool typeChanged = RenderDropDown( "emdMapTypes", m_mapType, mapTypes, "Map Type", false, "__HOPEFULLY_NOT_A_MAP_TYPE__" );
        ImGui::Columns( 2, nullptr, true, ImGuiColumnsFlags_NoResize ); // ThesisFIXME: Potentially dangerous.. modified imGui to expose this flag

        // Current Map Button
        if( typeChanged || ImGui::Button( "Regenerate Current Map", ImVec2( -1, 0 ) ) ) {
            TriggerMapGen( m_mapType, stepIndex, true );
            regenTriggered = true;
        }

        ImGui::NextColumn();

        // New Map Button
        if( ImGui::Button( "Generate New Map", ImVec2( -1, 0 ) ) ) {
            TriggerMapGen( m_mapType, stepIndex, false );
            regenTriggered = true;
        }

        ImGui::Columns( 1 );

        // Map Seed
        if( !regenTriggered ) {
            MapWindow* mapWindow = g_theEditor->GetMapWindow();
            ImGui::Text( "Current Map Seed: %u", mapWindow->GetMapSeed() );
        }

        ImGui::Checkbox( "Highlight Modified Tiles", &m_highlightChanges );
        ImGui::TreePop();
    }

    return regenTriggered;
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
        bool headerOpen = ImGui::CollapsingHeader( stepName.c_str(), ImGuiTreeNodeFlags_None );
        bool regenTriggered = RenderContextMenu( eMapDef, stepName, stepIndex, numSteps );

        if( !regenTriggered && headerOpen ) {
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
    }
}


bool XmlWindow::RenderContextMenu( EditorMapDef* eMapDef, const std::string& guiID, int stepIndex, int numSteps ) {
    bool regenTriggered = false;

    const MapWindow* mapWindow = g_theEditor->GetMapWindow();
    int selectedIndex = mapWindow->GetStepIndex();

    bool isSelected = (stepIndex == selectedIndex);
    bool shortcutCopy = g_theEditor->IsCopyShortcutPressed() && isSelected;
    bool shortcutPaste = g_theEditor->IsPasteShortcutPressed() && isSelected;
    bool shortcutAny = shortcutCopy || shortcutPaste;

    if( shortcutAny ) {
        ImGui::OpenPopup( guiID.c_str() );
    }

    if( ImGui::BeginPopupContextItem( guiID.c_str() ) || shortcutAny ) {
        int firstXmlStepIndex = EditorMapDef::NUM_PRE_STEPS;
        int lastXmlStepIndex = numSteps - EditorMapDef::NUM_POST_STEPS - 1;
        bool isXmlStep = (stepIndex >= firstXmlStepIndex)
                      && (stepIndex <= lastXmlStepIndex);

        // Copy
        if( ImGui::MenuItem( "Copy", "Ctrl + C", nullptr, isXmlStep ) || (shortcutCopy && isXmlStep) ) {
            m_stepToCopy = eMapDef->GetStep( stepIndex );

            if( shortcutCopy ) {
                g_theEditor->m_cPressed = false;
                ImGui::CloseCurrentPopup();
            }
        }

        // Paste
        bool insertAfterEnabled = (stepIndex >= firstXmlStepIndex - 1)
                               && (stepIndex <= lastXmlStepIndex);
        bool pasteEnabled = insertAfterEnabled && (m_stepToCopy != nullptr);

        if( ImGui::MenuItem( "Paste", "Ctrl + V", nullptr, pasteEnabled ) || (shortcutPaste && isXmlStep) ) {
            if( shortcutPaste ) {
                g_theEditor->m_vPressed = false;
            }

            MapGenStep* newCopiedStep = MapGenStep::CreateMapGenStep( m_stepToCopy );
            eMapDef->InsertStepAfter( newCopiedStep, stepIndex );

            TriggerMapGen( m_mapType, stepIndex + 1, true );
            regenTriggered = true;
        }

        ImGui::Separator();

        // Move Up
        bool moveUpEnabled = (stepIndex >= firstXmlStepIndex + 1)
                          && (stepIndex <= lastXmlStepIndex);

        if( !regenTriggered && ImGui::MenuItem( "Move Up", "", nullptr, moveUpEnabled ) ) {
            eMapDef->ReorderStepUp( stepIndex );

            TriggerMapGen( m_mapType, stepIndex - 1, true );
            regenTriggered = true;
        }

        // Move Down
        bool moveDownEnabled = (stepIndex >= firstXmlStepIndex)
                            && (stepIndex <= lastXmlStepIndex - 1);

        if( !regenTriggered && ImGui::MenuItem( "Move Down", "", nullptr, moveDownEnabled ) ) {
            eMapDef->ReorderStepDown( stepIndex );

            TriggerMapGen( m_mapType, stepIndex + 1, true );
            regenTriggered = true;
        }

        ImGui::Separator();

        // Insert Before
        bool insertBeforeEnabled = (stepIndex >= firstXmlStepIndex)
                                && (stepIndex <= lastXmlStepIndex + 1);

        if( !regenTriggered && ImGui::BeginMenu( "Insert Before", insertBeforeEnabled ) ) {
            regenTriggered = RenderNewStepMenu( eMapDef, stepIndex, true );
            ImGui::EndMenu();
        }

        // Insert After
        //bool insertAfterEnabled: Created above for "Paste" option

        if( !regenTriggered && ImGui::BeginMenu( "Insert After", insertAfterEnabled ) ) {
            regenTriggered = RenderNewStepMenu( eMapDef, stepIndex, false );
            ImGui::EndMenu();
        }

        ImGui::Separator();
        SetImGuiTextColor( Rgba::ORGANIC_RED );

        // Delete
        if( !regenTriggered && ImGui::MenuItem( "Delete", "", nullptr, isXmlStep ) ) {
            eMapDef->DeleteStep( stepIndex );
            
            int selectedStep = mapWindow->GetStepIndex();

            if( selectedStep > stepIndex ) {
                selectedStep--;
            }

            TriggerMapGen( m_mapType, selectedStep, true );
            regenTriggered = true;
        }

        SetImGuiTextColor( Rgba::WHITE );
        ImGui::EndPopup();
    }

    return regenTriggered;
}


bool XmlWindow::RenderNewStepMenu( EditorMapDef* eMapDef, int stepIndex, bool insertBefore ) {
    std::string mapMotif = eMapDef->GetMotif();
    int numTypes = (int)m_stepTypes.size();

    for( int typeIndex = 0; typeIndex < numTypes; typeIndex++ ) {
        const std::string& stepType = m_stepTypes[typeIndex];

        if( ImGui::MenuItem( stepType.c_str() ) ) {
            MapGenStep* newStep = MapGenStep::CreateMapGenStep( stepType, { mapMotif } );
            int newSelectedIndex = stepIndex;

            if( insertBefore ) {
                eMapDef->InsertStepBefore( newStep, stepIndex );
            } else {
                eMapDef->InsertStepAfter( newStep, stepIndex );
                newSelectedIndex++;
            }

            TriggerMapGen( m_mapType, newSelectedIndex, true );
            return true;
        }
    }

    return false;
}


bool XmlWindow::ChangeMapType( EventArgs& args ) {
    m_mapType = args.GetValue( "mapType", m_mapType );
    return false;
}

#endif
