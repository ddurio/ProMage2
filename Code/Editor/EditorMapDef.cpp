#if defined(_EDITOR)
#include "Editor/EditorMapDef.hpp"

#include "Editor/EditorMapGenStep.hpp"
#include "Editor/ImGuiUtils.hpp"

#include "Engine/Async/JobSystem.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/Map/Map.hpp"


void EditorMapDef::DefineObject( std::vector< Map* >* mapSteps, bool useCustomSeed /*= false*/, unsigned int customSeed /*= 0 */ ) const {
    m_mapPerStep     = mapSteps;
    m_mapPerStep->resize( m_numSteps, nullptr );

    m_allJobsStarted = false;
    m_numJobsRunning = 0;

    LaunchJobs( useCustomSeed, customSeed );
    WaitForJobs();
}


void EditorMapDef::DefineObject( Map& theMap ) const {
    MapDef::DefineObject( theMap );
}


void EditorMapDef::RenderMapDefParams() {
    RenderIntRange( m_width, "Map Width", 3, 100 );
    RenderIntRange( m_height, "Map Height", 3, 100 );
    RenderTileDropDown( "emdFill", m_tileFillType, "Fill Type", false, "__HOPEFULLY_NOT_USED__" );
    RenderTileDropDown( "emdEdge", m_tileEdgeType, "Edge Type" );
}


MapGenStep* EditorMapDef::GetStep( int stepIndex ) const {
    int stepOffset = 1; // From FillAndEdge
    int numSteps = (int)m_mapGenSteps.size();
    int adjustedIndex = stepIndex - stepOffset;

    if( adjustedIndex < 0 || adjustedIndex >= numSteps ) {
        return nullptr;
    }

    return m_mapGenSteps[adjustedIndex];
}


Strings EditorMapDef::GetStepNames( int indexOffset /*= 0 */ ) const {
    Strings stepNames;
    int numSteps = (int)m_mapGenSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        const MapGenStep* genStep = m_mapGenSteps[stepIndex];
        std::string stepName = Stringf( "%d: %s", stepIndex + indexOffset, genStep->GetName().c_str() );
        stepNames.push_back( stepName );
    }

    return stepNames;
}


Strings EditorMapDef::GetMapTypes() const {
    std::map< std::string, EditorMapDef*, StringCmpCaseI >::const_iterator mapDefIter = s_definitions.begin();
    Strings mapTypes;

    while( mapDefIter != s_definitions.end() ) {
        mapTypes.push_back( mapDefIter->first );
        mapDefIter++;
    }

    return mapTypes;
}


void EditorMapDef::ReorderStepUp( int stepIndexToMove ) {
    int numSteps = (int)m_mapGenSteps.size();

    if( stepIndexToMove <= 0 || stepIndexToMove >= numSteps ) {
        return;
    }

    MapGenStep* stepToMoveDown = m_mapGenSteps[stepIndexToMove - 1];
    MapGenStep* stepToMoveUp   = m_mapGenSteps[stepIndexToMove];

    m_mapGenSteps[stepIndexToMove - 1] = stepToMoveUp;
    m_mapGenSteps[stepIndexToMove]     = stepToMoveDown;
}


void EditorMapDef::ReorderStepDown( int stepIndexToMove ) {
    int numSteps = (int)m_mapGenSteps.size();

    if( stepIndexToMove < 0 || stepIndexToMove >= (numSteps - 1) ) {
        return;
    }

    MapGenStep* stepToMoveDown = m_mapGenSteps[stepIndexToMove];
    MapGenStep* stepToMoveUp   = m_mapGenSteps[stepIndexToMove + 1];

    m_mapGenSteps[stepIndexToMove]     = stepToMoveUp;
    m_mapGenSteps[stepIndexToMove + 1] = stepToMoveDown;
}


void EditorMapDef::InsertStepBefore( int stepIndexToInsertBefore, MapGenStep* stepToInsert ) {
    if( stepIndexToInsertBefore < 0 ) {
        return;
    } else if( stepIndexToInsertBefore >= m_numSteps - (NUM_POST_STEPS + 1) ) {
        m_mapGenSteps.push_back( stepToInsert );
    } else {
        std::vector< MapGenStep* >::iterator stepIter = m_mapGenSteps.begin() + stepIndexToInsertBefore;
        m_mapGenSteps.insert( stepIter, stepToInsert );
    }

    std::string eventName = Stringf( "%s_%s", EVENT_EDITOR_MOTIF_CHANGED, m_defType.c_str() );
    g_theEventSystem->Subscribe( eventName, stepToInsert, &MapGenStep::RecalculateMotifVars );
    m_numSteps++;
}


void EditorMapDef::InsertStepAfter( int stepIndexToInsertAfter, MapGenStep* stepToInsert ) {
    InsertStepBefore( stepIndexToInsertAfter + 1, stepToInsert );
}


void EditorMapDef::DeleteStep( int stepIndexToDelete ) {
    int numSteps = (int)m_mapGenSteps.size();

    if( stepIndexToDelete < 0 || stepIndexToDelete >= numSteps ) {
        return;
    }

    std::vector< MapGenStep* >::iterator stepIter = m_mapGenSteps.begin() + stepIndexToDelete;

    std::string eventName = Stringf( "%s_%s", EVENT_EDITOR_MOTIF_CHANGED, m_defType.c_str() );
    g_theEventSystem->Unsubscribe( eventName, *stepIter, &MapGenStep::RecalculateMotifVars );

    delete *stepIter;
    m_mapGenSteps.erase( stepIter );
    m_numSteps--;
}


int EditorMapDef::SetupChangeTileMGS() {
    Strings attrNames;

    return MapGenStep::AddCustomResult( "changeTile", attrNames, REQUIRE_NONE );
}


bool EditorMapDef::SaveAllToXml( EventArgs& args ) {
    std::string filePath = args.GetValue( "filePath", DATA_MAP_DEFS );
    EditorMapDef::SaveToFile( filePath, "MapDefinition" );
    return true;
}


EditorMapDef* EditorMapDef::CreateNewMapDef( const std::string& mapType, const std::string& fillType ) {
    EditorMapDef* eMapDef = new EditorMapDef( mapType, fillType );
    AddDefinition( eMapDef );

    return eMapDef;
}


// PRIVATE -------------------------------------------
EditorMapDef::EditorMapDef( const XMLElement& element ) :
    MapDef( element ) {
    s_defClass = "EditorMapDef";

    int numGenSteps = (int)m_mapGenSteps.size();
    m_numSteps += numGenSteps;

    std::string eventName = Stringf( "%s_%s", EVENT_EDITOR_MOTIF_CHANGED, m_defType.c_str() );
    g_theEventSystem->Subscribe( eventName, (MapDef*)this, &EditorMapDef::RecalculateMotifVars );

    for( int stepIndex = 0; stepIndex < numGenSteps; stepIndex++ ) {
        MapGenStep* genStep = m_mapGenSteps[stepIndex];
        g_theEventSystem->Subscribe( eventName, genStep, &MapGenStep::RecalculateMotifVars );
    }
}


EditorMapDef::EditorMapDef( const std::string& mapType, const std::string& fillType ) :
    MapDef( mapType, fillType ) {

    std::string eventName = Stringf( "%s_%s", EVENT_EDITOR_MOTIF_CHANGED, m_defType.c_str() );
    g_theEventSystem->Subscribe( eventName, (MapDef*)this, &EditorMapDef::RecalculateMotifVars );
}


EditorMapDef::~EditorMapDef() {
    int numGenSteps = (int)m_mapGenSteps.size();
    g_theEventSystem->Unsubscribe( EVENT_EDITOR_MOTIF_CHANGED, (MapDef*)this, &MapDef::RecalculateMotifVars );

    for( int stepIndex = 0; stepIndex < numGenSteps; stepIndex++ ) {
        MapGenStep* genStep = m_mapGenSteps[stepIndex];
        g_theEventSystem->Unsubscribe( EVENT_EDITOR_MOTIF_CHANGED, genStep, &MapGenStep::RecalculateMotifVars );
    }
}


bool EditorMapDef::IsFinished() const {
    return (m_allJobsStarted && m_numJobsRunning == 0);
}


void EditorMapDef::LaunchJobs( bool useCustomSeed, unsigned int customSeed ) const {
    unsigned int mapSeed = (useCustomSeed) ? customSeed : g_RNG->GetRandomSeed();

    for( int stepIndex = 1; stepIndex <= m_numSteps; stepIndex++ ) {
        std::string mapName = Stringf( "%s: Step %d", m_defType.c_str(), stepIndex );
        RNG* mapRNG = new RNG( mapSeed );
        Map* theMap = new Map( mapName, mapRNG );

        SetMapDef( *theMap );
        g_theEventSystem->Subscribe( "changeTile", theMap, &Map::TrackModifiedTiles );

        EditorMapDefJob* eJob = new EditorMapDefJob( this, theMap, stepIndex );

        m_numJobsRunning++;
        g_theJobs->StartJob( eJob );
    }

    m_allJobsStarted = true;
}


void EditorMapDef::WaitForJobs() const {
    // Process all of the payloads
    while( m_numJobsRunning > 0 ) {
        g_theJobs->ProcessCategory( JOB_CATEGORY_GENERIC );
    }
}


void EditorMapDef::DefineFromEdgedTiles( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        Tile& tile = GetTile( theMap, tileIndex );

        if( tile.AddTypesFromNeighbors( theMap ) ) {
            EventArgs args;
            args.SetValue( "callingMap", &theMap );
            args.SetValue( "callingTile", &tile );

            theMap.TrackModifiedTiles( args );
        }
    }
}


void EditorMapDef::DefineFromWallTiles( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        Tile& tile = GetTile( theMap, tileIndex );
        std::string tileContext = tile.GetTileContext();

        if( StringICmp( tileContext, "southWall" ) ) {
            if( tile.ChooseWallFromNeighbor( theMap ) ) {
                EventArgs args;
                args.SetValue( "callingMap", &theMap );
                args.SetValue( "callingTile", &tile );

                theMap.TrackModifiedTiles( args );
            }
        }
    }
}


// EditorMapDefJob ---------------------------------------------
EditorMapDefJob::EditorMapDefJob( const EditorMapDef* eMapDef, Map* theMap, int numStepsToDo ) :
    Job( g_theJobs, JOB_CATEGORY_GENERIC ),
    m_eMapDef( eMapDef ),
    m_theMap( theMap ),
    m_numStepsToDo( numStepsToDo ) {
}


const EditorMapDef* EditorMapDefJob::GetMapDef() const {
    return m_eMapDef;
}


void EditorMapDefJob::Execute() {
    Map& theMap = *m_theMap;
    RNG* mapRNG = m_eMapDef->GetMapRNG( theMap );
    int mapWidth = mapRNG->GetRandomIntInRange( m_eMapDef->m_width );
    int mapHeight = mapRNG->GetRandomIntInRange( m_eMapDef->m_height );
    m_eMapDef->SetMapDimensions( theMap, IntVec2( mapWidth, mapHeight ) );

    m_eMapDef->DefineFillAndEdge( theMap );

    if( CompleteStep() ) {
        return;
    }

    // Run Each MapGenStep
    int numSteps = (int)m_eMapDef->m_mapGenSteps.size();
    bool payloadComplete = false;

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        m_eMapDef->m_mapGenSteps[stepIndex]->Run( theMap );

        if( CompleteStep() ) {
            payloadComplete = true;
            break;
        }
    }

    if( payloadComplete ) {
        return;
    }

    m_eMapDef->DefineFromEdgedTiles( theMap );

    if( CompleteStep() ) {
        return;
    }

    m_eMapDef->DefineFromWallTiles( theMap );

    if( CompleteStep() ) {
        return;
    }

    m_eMapDef->DefineTileColliders( theMap );

    if( CompleteStep() ) {
        return;
    } else {
        ERROR_RECOVERABLE( "(EditorMapDef) Worker never sent job back!" );
    }
}


bool EditorMapDefJob::CompleteStep() {
    m_numStepsDone++;

    GUARANTEE_RECOVERABLE( m_numStepsDone <= m_numStepsToDo, "(EditorMapDef) Too many steps completed!" );

    if( m_numStepsDone == m_numStepsToDo ) {
        m_theMap->StartupPostDefine();
        m_theMap->Shutdown();
        g_theEventSystem->Unsubscribe( "changeTile", m_theMap, &Map::TrackModifiedTiles );

        (*m_eMapDef->m_mapPerStep)[m_numStepsToDo - 1] = m_theMap;
        m_eMapDef->m_numJobsRunning--;
        return true;
    }

    m_theMap->ClearModifiedTiles();
    return false;
}


#endif
