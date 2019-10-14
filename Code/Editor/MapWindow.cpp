#if defined(_EDITOR)
#include "Editor/MapWindow.hpp"

#include "Editor/Editor.hpp"
#include "Editor/EditorMapDef.hpp"
#include "Editor/XmlWindow.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
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
    m_extraFlags |= ImGuiWindowFlags_NoMouseInputs;

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


bool MapWindow::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    if( event == MOUSE_EVENT_SCROLL ) {
        if( scrollAmount > 0.f ) {
            //m_currentZoom += m_zoomIncrement;
            m_currentZoom += 0.1f;
        } else {
            //m_currentZoom -= m_zoomIncrement;
            m_currentZoom -= 0.1f;
        }

        m_currentZoom = Clamp( m_currentZoom, 0.f, 1.f );
        //m_currentZoom = Max( m_currentZoom, 0.f );
        return true;
    } else if( event == MOUSE_EVENT_RBUTTON_UP ) {
        m_isDragging = false;
        g_theWindow->UnlockMouse();
    } else if( event == MOUSE_EVENT_RBUTTON_DOWN ) {
        m_isDragging = true;
        g_theWindow->LockMouse();
    }

    return false;
}


int MapWindow::GetStepIndex() const {
    return m_stepIndex;
}


unsigned int MapWindow::GetMapSeed() const {
    Map* theMap = m_mapPerStep[m_stepIndex];
    RNG* mapRNG = theMap->GetMapRNG();
    unsigned int mapSeed = mapRNG->GetSeed();

    return mapSeed;
}


std::string MapWindow::GetMapType() const {
    Map* theMap = m_mapPerStep[m_stepIndex];
    return theMap->GetMapType();
}


Strings MapWindow::GetStepNames() const {
    Strings stepNames;
    stepNames.push_back( "1: Map Initialization" );

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


void MapWindow::UpdateMapCamera( float deltaSeconds ) {
    UNUSED( deltaSeconds );
    Map*& theMap = m_mapPerStep[m_stepIndex];

    // Window sizes
    Vec2 contentMin = ImGui::GetWindowContentRegionMin();
    Vec2 contentMax = ImGui::GetWindowContentRegionMax();

    AABB2 contentBounds = AABB2( contentMin, contentMax );
    Vec2 contentDims = contentBounds.GetDimensions();

    // Apply zoom
    //float minZoomFactor = SmoothStop2( m_minZoomT );
    float zoomFactor = SmoothStart5( m_currentZoom );
    zoomFactor = RangeMap( m_currentZoom, 0.f, 1.f, 0.f, m_maxZoom );
    float zoomPixelsPerTile = m_minPixelsPerTile * (1.f + zoomFactor);

    IntVec2 mapDims = theMap->GetMapDimensions();
    Vec2 mapSizePixels = zoomPixelsPerTile * mapDims;

    mapSizePixels.x = Min( mapSizePixels.x, contentDims.x );
    mapSizePixels.y = Min( mapSizePixels.y, contentDims.y );

    // Get aligned position
    m_mapBounds = contentBounds.GetBoxWithin( mapSizePixels, ALIGN_BOTTOM_LEFT ); // Y is inverted.. actually means top left

    float heightTiles = mapSizePixels.y / zoomPixelsPerTile;
    m_mapCamera->SetOrthoProjection( heightTiles, -100.f, 100.f, m_mapBounds.GetAspectRatio() );

    // Apply drag
    if( m_isDragging ) {
        Vec2 worldDelta = GetClampedMouseDelta( zoomPixelsPerTile );
        m_mapCamera->Translate( worldDelta );
    }
}


Vec2 MapWindow::GetClampedMouseDelta( float pixelsPerTile ) const {
    IntVec2 mouseDelta = g_theWindow->GetMouseClientDisplacement();
    mouseDelta.y = -mouseDelta.y; // Invert client y
    Vec2 worldDelta = mouseDelta / pixelsPerTile;
    worldDelta = -worldDelta;


    Map* theMap = m_mapPerStep[m_stepIndex];
    IntVec2 mapDims = theMap->GetMapDimensions();
    Vec2 cameraDims = m_mapCamera->GetDimensions();
    Vec2 cameraPos  = m_mapCamera->GetCenter();

    Vec2 halfCameraDims = cameraDims * 0.5f;

    Vec2 minDelta = halfCameraDims - cameraPos;
    Vec2 maxDelta = Vec2( mapDims ) - halfCameraDims - cameraPos;

    float clampedX = Clamp( worldDelta.x, minDelta.x, maxDelta.x );
    float clampedY = Clamp( worldDelta.y, minDelta.y, maxDelta.y );

    return Vec2( clampedX, clampedY );
}


void MapWindow::RenderMap( float deltaSeconds ) {
    Map*& theMap = m_mapPerStep[m_stepIndex];
    m_windowName = theMap->GetMapName();

    // Update and draw to textureView
    UpdateMapCamera( deltaSeconds );
    theMap->Update( deltaSeconds );
    g_theRenderer->BeginCamera( m_mapCamera );
    g_theRenderer->ClearRenderTarget( Rgba::BLACK );
    theMap->Render();

    XmlWindow* xmlWindow = g_theEditor->GetXmlWindow();
    bool showModified = xmlWindow->ShouldHighlightTiles();
    int numSteps = (int)m_mapPerStep.size();

    if( showModified && m_stepIndex == (numSteps - 1) ) {
        g_thePhysicsSystem->RenderDebug();
    }

    g_theRenderer->EndCamera( m_mapCamera );

    // Render map to Editor
    void* mapView = m_mapCamera->GetRenderTarget()->GetShaderView();

    ImGui::Image( mapView, m_mapBounds.GetDimensions().GetAsImGui() ); // Map render
    ImGui::GetWindowDrawList()->AddRect( m_mapBounds.mins.GetAsImGui(), m_mapBounds.maxs.GetAsImGui(), 0xFFFF'FFFF );
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

        Vec2 tileMin = mapOrigin + (m_minPixelsPerTile * invertedTileCoord);
        Vec2 tileMax = tileMin + Vec2( m_minPixelsPerTile );

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

            Vec2 cursorTileFloat = cursorOffset / m_minPixelsPerTile;
            IntVec2 cursorInvertedTileCoord = IntVec2( (int)cursorTileFloat.x, (int)cursorTileFloat.y );
            IntVec2 cursorTileCoord = IntVec2( cursorInvertedTileCoord.x, (mapSizeTiles.y - 1) - cursorInvertedTileCoord.y );
            
            Vec2 tileMin = mapOrigin + (m_minPixelsPerTile * cursorInvertedTileCoord);
            Vec2 tileMax = tileMin + Vec2( m_minPixelsPerTile );

            ImGui::GetForegroundDrawList()->AddRect( tileMin.GetAsImGui(), tileMax.GetAsImGui(), 0xFFFF'FFFF );

            // Actual tooltip if it was changed
            if( EngineCommon::VectorContains( modifiedTiles, cursorTileCoord ) ) {
                Vec2 tooltipDims = Vec2( 0.2f * m_windowDimensions.x, 0.1f * m_windowDimensions.y );
                ImGuiWindowFlags tooltipFlags = ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoTitleBar;

                g_theGui->CreateStaticWindow( tooltipDims, Vec2( 0.647f, 0.115f ), "modifiedTooltip", tooltipFlags );

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
    if( m_mapPerStep.size() > 0 ) { // Previously generated maps.. delete them
        Shutdown();
    }

    //std::string mapType = args.GetValue( "mapType", "Cavern" );
    std::string mapType = args.GetValue( "mapType", "Island" );
    //std::string mapType = args.GetValue( "mapType", "DD1" );

    bool useCustomSeed = args.GetValue( "useCustomSeed", false );
    unsigned int customSeed = args.GetValue( "customSeed", 0 );

    const EditorMapDef* eMapDef = EditorMapDef::GetDefinition( mapType );
    eMapDef->DefineObject( &m_mapPerStep, useCustomSeed, customSeed );

    m_stepIndex = (int)m_mapPerStep.size() - 1;
    m_stepIndex = args.GetValue( "stepIndex", m_stepIndex );

    m_sizeIsCalculated = false;

    // Setup camera
    IntVec2 mapDims = m_mapPerStep[0]->GetMapDimensions();
    float mapAspect = (float)mapDims.x / (float)mapDims.y;

    m_mapCamera = new Camera();
    m_mapCamera->SetOrthoProjection( (float)mapDims.y, -100.f, 100.f, mapAspect );

    Vec2 halfDims = mapDims * 0.5f;
    m_mapCamera->Translate( halfDims );

    m_currentZoom = 0.f;

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


// Expects to be called with m_minPixelsPerTile set at ZERO ZOOM
void MapWindow::CalculateMaxZoom() {
    // Calculate Pixel Sizes
    Vec2 contentMin = ImGui::GetWindowContentRegionMin();
    Vec2 contentMax = ImGui::GetWindowContentRegionMax();
    Vec2 contentSize = contentMax - contentMin;

    Vec2 maxPixelsPerTile2D = contentSize / 10.f;
    float maxPixelsPerTile = Min( maxPixelsPerTile2D.x, maxPixelsPerTile2D.y );

    // Calculate zoom factor
    m_maxZoom = maxPixelsPerTile / m_minPixelsPerTile;

    CalculateZoomRange();
}


void MapWindow::CalculateZoomRange() {
    Map* theMap = m_mapPerStep[m_stepIndex];

    IntVec2 mapSizeTiles2D = theMap->GetMapDimensions();
    int maxMapSizeTiles = Max( mapSizeTiles2D.x, mapSizeTiles2D.y );

    int numZooms = maxMapSizeTiles / 10;
    numZooms = Clamp( numZooms, 0, 10 );

    m_minZoomT = 1 - ((float)numZooms * 0.1f);
    //m_currentZoom = m_minZoomT;

    CalculateZoomIncrement();
}


void MapWindow::CalculateZoomIncrement() {
    Map* theMap = m_mapPerStep[m_stepIndex];

    IntVec2 mapSizeTiles2D = theMap->GetMapDimensions();
    int maxMapSizeTiles = Max( mapSizeTiles2D.x, mapSizeTiles2D.y );

    int numZooms = maxMapSizeTiles / 10;
    numZooms = Clamp( numZooms, 0, 10 );

    m_zoomIncrement = 1.f / numZooms;
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
    m_minPixelsPerTile = Min( pixelsPerTile2D.x, pixelsPerTile2D.y ); // Tile size in pixels

    CalculateMaxZoom();
    m_sizeIsCalculated = true;
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
    GetTileRenderChanges( tileChanges, currentTile, prevTile );

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

            prevIter++;
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

    if( currentStep == nullptr ) {
        return;
    }

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

    if( currentStep == nullptr ) {
        return;
    }

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


void MapWindow::GetTileRenderChanges( Strings& changeList, const Tile& currentTile, const Tile& prevTile ) const {
    Strings currentTypes = currentTile.GetRenderTypes();
    Strings prevTypes = prevTile.GetRenderTypes();

    Strings::iterator currentIter = currentTypes.begin();

    while( currentIter != currentTypes.end() ) {
        Strings::iterator prevIter = prevTypes.begin();
        bool foundTag = false;

        while( prevIter != prevTypes.end() ) {
            if( StringICmp( *currentIter, *prevIter ) ) {
                currentIter = currentTypes.erase( currentIter );
                prevIter = prevTypes.erase( prevIter );
                foundTag = true;

                break;
            }

            prevIter++;
        }

        if( !foundTag ) {
            currentIter++;
        }
    }

    // Added render type
    if( !currentTypes.empty() ) {
        std::string pluralStr = (currentTypes.size() > 1) ? "s" : "";
        std::string newTagStr = JoinStrings( currentTypes, ", " );
        std::string addedRenderChange = Stringf( "Render Type%s Added: %s", pluralStr.c_str(), newTagStr.c_str() );
        changeList.push_back( addedRenderChange );
    }

    // Removed render type
    if( !prevTypes.empty() ) {
        std::string pluralStr = (prevTypes.size() > 1) ? "s" : "";
        std::string removedTypes = JoinStrings( prevTypes, ", " );
        std::string removedRenderChange = Stringf( "Render Type%s Removed: %s", pluralStr.c_str(), removedTypes.c_str() );
        changeList.push_back( removedRenderChange );
    }
}

#endif
