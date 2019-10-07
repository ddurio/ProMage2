#if defined(_EDITOR)
#include "Editor/MapWindow.hpp"

#include "Editor/Editor.hpp"
#include "Editor/EditorMapDef.hpp"
#include "Editor/XmlWindow.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture2D.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Inventory.hpp"
#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/Map/Metadata.hpp"


MapWindow::MapWindow( const Vec2& normDimensions /*= Vec2( 0.8f, 0.9f )*/, const Vec2& alignment /*= Vec2( 0.f, 1.f ) */ ) :
    EditorWindow( normDimensions, alignment ) {
    m_windowName = "MapEditor";
    g_theEventSystem->Subscribe( EVENT_EDITOR_CHANGE_STEP, this, &MapWindow::SetVisibleMapStep );
    g_theEventSystem->Subscribe( EVENT_EDITOR_GENERATE_MAP, this, &MapWindow::GenerateMaps );

    Rgba tileHighlight = Rgba::RED;
    tileHighlight.a = .3f;
    unsigned char packedColor[4];
    tileHighlight.GetAsBytes( packedColor );
    m_highlightColor = *(ImU32*)packedColor;

    Startup();
}


MapWindow::~MapWindow() {
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_CHANGE_STEP, this, &MapWindow::SetVisibleMapStep );
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_GENERATE_MAP, this, &MapWindow::GenerateMaps );

    Shutdown();
}


void MapWindow::Startup() {
    EventArgs args;
    GenerateMaps( args );
}


void MapWindow::Shutdown() {
    m_stepIndex = -1;
    CLEAR_POINTER( m_mapCamera );
    EngineCommon::ClearVector( m_mapPerStep );
}


int MapWindow::GetStepIndex() const {
    return m_stepIndex;
}


std::string MapWindow::GetMapType() const {
    Map* theMap = m_mapPerStep[m_stepIndex];
    return theMap->GetMapType();
}


Strings MapWindow::GetStepNames() const {
    Strings stepNames;
    stepNames.push_back( "1: Fill And Edge" );

    // Gen Steps
    std::string defType = m_mapPerStep[0]->GetMapType();
    const EditorMapDef* mapDef = EditorMapDef::GetDefinition( defType );
    Strings genStepNames = mapDef->GetStepNames( 2 ); // One step and make it not zero indexed

    stepNames.insert( stepNames.end(), genStepNames.begin(), genStepNames.end() );

    // Edged
    int stepIndex = (int)stepNames.size();
    std::string edgedName = Stringf( "%d: Edged Tiles", stepIndex + 1 );
    stepNames.push_back( edgedName );

    // Colliders
    std::string colliderName = Stringf( "%d: Phys. Colliders", stepIndex + 2 );
    stepNames.push_back( colliderName );

    return stepNames;
}


void MapWindow::UpdateChild( float deltaSeconds ) {
    CalculateMapSizes();
    RenderMap( deltaSeconds );
    RenderTileChangeHighlight();
    RenderTileChangeTooltip();
}


void MapWindow::RenderMap( float deltaSeconds ) {
    Map*& theMap = m_mapPerStep[m_stepIndex];
    m_windowName = theMap->GetMapName();

    // Update and draw to textureView
    theMap->Update( deltaSeconds );
    g_theRenderer->BeginCamera( m_mapCamera );
    theMap->Render();
    g_theRenderer->EndCamera( m_mapCamera );

    void* mapView = m_mapCamera->GetRenderTarget()->GetShaderView();

    // Actually render the map
    ImGui::Image( mapView, m_mapBounds.GetDimensions().GetAsImGui() ); // Map render
    //ImGui::GetForegroundDrawList()->AddRect( contentBounds.mins.GetAsImGui(), contentBounds.maxs.GetAsImGui(), 0xFFFF'FF00 );
    ImGui::GetForegroundDrawList()->AddRect( m_mapBounds.mins.GetAsImGui(), m_mapBounds.maxs.GetAsImGui(), 0xFFFF'FFFF );
}


void MapWindow::RenderTileChangeHighlight() {
    XmlWindow* xmlWindow = g_theEditor->GetXmlWindow();

    if( !xmlWindow->ShouldHighlightTiles() ) {
        return;
    }

    Map* theMap = m_mapPerStep[m_stepIndex];

    Vec2 mapOrigin = m_mapBounds.mins;
    IntVec2 mapSizeTiles = theMap->GetMapDimensions();

    std::vector< IntVec2 > modifiedTiles = theMap->GetModifiedTiles();
    int numModifies = (int)modifiedTiles.size();

    for( int modifyIndex = 0; modifyIndex < numModifies; modifyIndex++ ) {
        IntVec2 invertedTileCoord = modifiedTiles[modifyIndex];
        invertedTileCoord.y = (mapSizeTiles.y - 1) - invertedTileCoord.y;

        Vec2 tileMin = mapOrigin + (m_pixelsPerTile * invertedTileCoord);
        Vec2 tileMax = tileMin + Vec2( m_pixelsPerTile );

        ImGui::GetForegroundDrawList()->AddRectFilled( tileMin.GetAsImGui(), tileMax.GetAsImGui(), m_highlightColor );
    }
}


void MapWindow::RenderTileChangeTooltip() {
    Map* theMap = m_mapPerStep[m_stepIndex];

    Vec2 mapOrigin = m_mapBounds.mins;
    IntVec2 mapSizeTiles = theMap->GetMapDimensions();
    std::vector< IntVec2 > modifiedTiles = theMap->GetModifiedTiles();

    if( ImGui::IsItemHovered() ) {
        ImVec2 cursorPos = ImGui::GetMousePos();

        if( m_mapBounds.IsPointInside( cursorPos ) ) {
            Vec2 cursorOffset = cursorPos - mapOrigin;

            Vec2 cursorTileFloat = cursorOffset / m_pixelsPerTile;
            IntVec2 cursorInvertedTileCoord = IntVec2( (int)cursorTileFloat.x, (int)cursorTileFloat.y );
            IntVec2 cursorTileCoord = IntVec2( cursorInvertedTileCoord.x, (mapSizeTiles.y - 1) - cursorInvertedTileCoord.y );
            
            Vec2 tileMin = mapOrigin + (m_pixelsPerTile * cursorInvertedTileCoord);
            Vec2 tileMax = tileMin + Vec2( m_pixelsPerTile );

            ImGui::GetForegroundDrawList()->AddRect( tileMin.GetAsImGui(), tileMax.GetAsImGui(), 0xFFFF'FFFF );

            // Actual tooltip if it was changed
            if( EngineCommon::VectorContains( modifiedTiles, cursorTileCoord ) ) {
                Vec2 tooltipDims = Vec2( 0.2f * m_windowDimensions.x, 0.1f * m_windowDimensions.y );
                ImGuiWindowFlags tooltipFlags = ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoTitleBar;

                g_theEditor->CreateWindow( tooltipDims, Vec2( 0.703f, 0.115f ), "modifiedTooltip", tooltipFlags );

                Strings tileChanges = GetTileChanges( cursorTileCoord );
                Strings::const_iterator changeIter = tileChanges.begin();

                while( changeIter != tileChanges.end() ) {
                    ImGui::Text( changeIter->c_str() );

                    changeIter++;
                }


                ImGui::End();
            }
        }
    }
}


bool MapWindow::GenerateMaps( EventArgs& args ) {
    if( m_mapPerStep.size() > 0 ) { // Already generated maps
        Shutdown();
    }

    //std::string mapType = args.GetValue( "mapType", "Cavern" );
    std::string mapType = args.GetValue( "mapType", "Island" );
    //std::string mapType = args.GetValue( "mapType", "DD1" );

    const EditorMapDef* eMapDef = EditorMapDef::GetDefinition( mapType );
    eMapDef->DefineObject( &m_mapPerStep );

    m_stepIndex = (int)m_mapPerStep.size() - 1;
    m_sizeIsCalculated = false;

    // Setup camera
    IntVec2 mapDims = m_mapPerStep[0]->GetMapDimensions();
    float mapAspect = (float)mapDims.x / (float)mapDims.y;

    m_mapCamera = new Camera();
    m_mapCamera->SetOrthoProjection( (float)mapDims.y, -100.f, 100.f, mapAspect );

    Vec2 halfDims = mapDims * 0.5f;
    m_mapCamera->Translate( halfDims );

    // Setup render target
    TextureView2D* mapView = g_theRenderer->GetOrCreateRenderTarget( m_mapViewName );
    m_mapCamera->SetRenderTarget( mapView );

    return false;
}


bool MapWindow::SetVisibleMapStep( EventArgs& args ) {
    int newIndex = args.GetValue( "stepIndex", -1 );
    int numSteps = (int)m_mapPerStep.size();

    if( newIndex < 0 || newIndex >= numSteps ) {
        std::string warningMsg = Stringf( "(MapEditor) Attempting to set stepIndex (%d) with %d steps", newIndex, numSteps );
        g_theDevConsole->PrintString( warningMsg, DevConsole::CHANNEL_WARNING ); // TheisFIXME: Add this to the editor channel when it exists
        return false;
    }

    m_stepIndex = newIndex;
    return false;
}


void MapWindow::CalculateMapSizes() {
    if( m_sizeIsCalculated ) {
        return;
    }

    Map* theMap = m_mapPerStep[m_stepIndex];

    // Calculate Pixel Sizes
    Vec2 contentMin = ImGui::GetWindowContentRegionMin();
    Vec2 contentMax = ImGui::GetWindowContentRegionMax();
    Vec2 contentSize = contentMax - contentMin;

    IntVec2 mapSizeTiles = theMap->GetMapDimensions();

    Vec2 pixelsPerTile2D = contentSize / mapSizeTiles;
    m_pixelsPerTile = Min( pixelsPerTile2D.x, pixelsPerTile2D.y ); // Tile size in pixels
    Vec2 mapSizePixels = m_pixelsPerTile * mapSizeTiles;

    // Get aligned position
    AABB2 contentBounds = AABB2( contentMin, contentMax );
    m_mapBounds = contentBounds.GetBoxWithin( mapSizePixels, ALIGN_BOTTOM_LEFT ); // Y is inverted.. actually means top left
}


Strings MapWindow::GetTileChanges( const IntVec2& tileCoord ) const {
    Strings tileChanges;

    // Setup data
    const Map* currentMap = m_mapPerStep[m_stepIndex];
    const Map* prevMap    = m_mapPerStep[m_stepIndex - 1];

    const Tile& currentTile = currentMap->GetTile( tileCoord );
    const Tile& prevTile    = prevMap->GetTile( tileCoord );

    const Metadata* currentData = currentTile.GetMetadata();
    const Metadata* prevData    = prevTile.GetMetadata();

    Strings currentTags = currentData->m_tagData.GetTags();
    Strings prevTags    = prevData->m_tagData.GetTags();

    // Find Changes
    GetTileTypeChanges( tileChanges, currentTile, prevTile );
    GetTagChanges( tileChanges, currentTags, prevTags );
    GetHeatMapChanges( tileChanges, currentData, prevData );
    GetActorChanges( tileChanges, currentMap, prevMap, currentTile );
    GetItemChanges( tileChanges, currentMap, prevMap, currentTile );

    return tileChanges;
}


void MapWindow::GetTileTypeChanges( Strings& changeList, const Tile& currentTile, const Tile& prevTile ) const {
    if( prevTile.GetTileType() != currentTile.GetTileType() ) {
        std::string typeChange = Stringf( "Type: %s --> %s", prevTile.GetTileType().c_str(), currentTile.GetTileType().c_str() );
        changeList.push_back( typeChange );
    }
}


void MapWindow::GetTagChanges( Strings& changeList, Strings currentTags, Strings prevTags ) const {
    Strings::iterator currentIter = currentTags.begin();

    while( currentIter != currentTags.end() ) {
        Strings::iterator prevIter = prevTags.begin();
        bool foundTag = false;

        while( prevIter != prevTags.end() ) {
            if( StringICmp( *currentIter, *prevIter ) ) {
                currentIter = currentTags.erase( currentIter );
                prevIter = prevTags.erase( prevIter );

                foundTag = true;
                break;
            }
        }

        if( !foundTag ) {
            currentIter++;
        }
    }

    if( !currentTags.empty() ) {
        std::string pluralStr = (currentTags.size() > 1) ? "s" : "";
        std::string newTagStr = JoinStrings( currentTags, ", " );
        std::string addedTagsChange = Stringf( "Tag%s Added: %s", pluralStr.c_str(), newTagStr.c_str() );
        changeList.push_back( addedTagsChange );
    }

    if( !prevTags.empty() ) {
        std::string pluralStr = (prevTags.size() > 1) ? "s" : "";
        std::string removedTags = JoinStrings( prevTags, ", " );
        std::string removedTagsChange = Stringf( "Tag%s Removed: %s", pluralStr.c_str(), removedTags.c_str() );
        changeList.push_back( removedTagsChange );
    }
}


void MapWindow::GetHeatMapChanges( Strings& changeList, const Metadata* currentMetadata, const Metadata* prevMetadata ) const {
    std::map< std::string, float, StringCmpCaseI > currentHeat = currentMetadata->m_heatMaps;
    std::map< std::string, float, StringCmpCaseI > prevHeat = prevMetadata->m_heatMaps;
    std::map< std::string, float, StringCmpCaseI >::iterator prevIter = prevHeat.begin();

    while( prevIter != prevHeat.end() ) {
        // Impossible to "unset" a value in the heat map.. so must still exist
        std::string mapName = prevIter->first;
        float currentValue = currentHeat[mapName];
        float prevValue = prevIter->second;

        if( prevValue != currentValue ) { // Changed
            std::string heatChange = Stringf( "Heat Map (%s): %.1f --> %.1f", mapName.c_str(), prevValue, currentValue );
            changeList.push_back( heatChange );
        }

        currentHeat.erase( prevIter->first );
        prevIter = prevHeat.erase( prevIter );
    }

    std::map< std::string, float, StringCmpCaseI >::iterator newHeatIter = currentHeat.begin();

    while( newHeatIter != currentHeat.end() ) {
        std::string heatChange = Stringf( "Heat Map (%s): -.- --> %.1f", newHeatIter->first.c_str(), newHeatIter->second );
        changeList.push_back( heatChange );

        newHeatIter++;
    }
}


void MapWindow::GetActorChanges( Strings& changeList, const Map* currentMap, const Map* prevMap, const Tile& currentTile ) const {
    std::string actorType = "";
    std::string controller = "";

    std::string mapType = currentMap->GetMapType();
    const EditorMapDef* mapDef = EditorMapDef::GetDefinition( mapType );
    const MapGenStep* currentStep = mapDef->GetStep( m_stepIndex );
    const std::vector< MapGenStep::CustomEvent > results = currentStep->GetCustomResults();

    // Find actor type from custom events
    int numResults = (int)results.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        const MapGenStep::CustomEvent& result = results[resultIndex];

        if( StringICmp( result.name, "SpawnActor" ) ) {
            if( StringICmp( result.attrNames[0], "SpawnActor" ) ) {
                actorType = result.attrValues[0];
                controller = result.attrValues[1];
            } else {
                actorType = result.attrValues[1];
                controller = result.attrValues[0];
            }

            break;
        }
    }

    // See if actor was spawned
    if( actorType != "" ) {
        Vec2 tileCenter = currentTile.GetWorldBounds().GetCenter();
        Actor* currentActor = currentMap->GetActorInRange( actorType, tileCenter, 0.2f );
        Actor* prevActor = prevMap->GetActorInRange( actorType, tileCenter, 0.2f );

        if( currentActor != nullptr && currentActor != prevActor ) {
            std::string actorChange = Stringf( "Spawned: (%s) Actor", actorType.c_str() );
            changeList.push_back( actorChange );

            if( controller != "" ) {
                std::string controllerChange = Stringf( "   with (%s) Controller", controller.c_str() );
                changeList.push_back( controllerChange );
            }
        }
    }
}


void MapWindow::GetItemChanges( Strings& changeList, const Map* currentMap, const Map* prevMap, const Tile& currentTile ) const {
    std::string itemType = "";

    std::string mapType = currentMap->GetMapType();
    const EditorMapDef* mapDef = EditorMapDef::GetDefinition( mapType );
    const MapGenStep* currentStep = mapDef->GetStep( m_stepIndex );
    const std::vector< MapGenStep::CustomEvent > results = currentStep->GetCustomResults();

    // Find item type from custom events
    int numResults = (int)results.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        const MapGenStep::CustomEvent& result = results[resultIndex];

        if( StringICmp( result.name, "SpawnItem" ) ) {
            itemType = result.attrValues[0];
            break;
        }
    }

    // See if item was spawned
    if( itemType != "" ) {
        Vec2 tileCenter = currentTile.GetWorldBounds().GetCenter();
        Item* currentItem = currentMap->GetMapInventory()->GetClosestItemInRange( tileCenter, 0.2f );
        Item* prevItem = prevMap->GetMapInventory()->GetClosestItemInRange( tileCenter, 0.2f );

        if( currentItem != nullptr && currentItem != prevItem ) {
            std::string actorChange = Stringf( "Spawned: (%s) Item", itemType.c_str() );
            changeList.push_back( actorChange );
        }
    }
}

#endif
