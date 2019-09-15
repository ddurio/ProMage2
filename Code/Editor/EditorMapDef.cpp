#if defined(_EDITOR)
#include "Editor/EditorMapDef.hpp"

#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/Map/Map.hpp"


void EditorMapDef::DefineObject( std::vector< Map* >* mapSteps ) const {
    m_mapPerStep     = mapSteps;
    m_mapPerStep->resize( m_numSteps, nullptr );

    m_allJobsStarted = false;
    m_numJobsRunning = 0;

    SpinUpThreads();
    LaunchJobs();
    ProcessMainPayloads();
    SpinDownThreads();
}


void EditorMapDef::DefineObject( Map& theMap ) const {
    MapDef::DefineObject( theMap );
}


// PRIVATE -------------------------------------------
EditorMapDef::EditorMapDef( const XMLElement& element ) :
    MapDef( element ) {
    s_defClass = "EditorMapDef";
    m_numSteps += (int)m_mapGenSteps.size();
}


bool EditorMapDef::IsFinished() const {
    return (m_allJobsStarted && m_numJobsRunning == 0);
}


bool EditorMapDef::FinishStep( AsyncPayload& payload ) const {
    payload.numStepsDone++;

    GUARANTEE_RECOVERABLE( payload.numStepsDone <= payload.numStepsToDo, "(EditorMapDef) Too many steps completed!" );

    if( payload.numStepsDone == payload.numStepsToDo ) {
        payload.theMap->StartupPostDefine();
        payload.theMap->Shutdown();
        m_mainPayloads.Enqueue( payload );

        return true;
    }

    return false;
}


void EditorMapDef::SpinUpThreads() const {
    int numCores = std::thread::hardware_concurrency();

    for( int coreIndex = 0; coreIndex < numCores; coreIndex++ ) {
        m_threads.emplace_back( &EditorMapDef::ProcessWorkerPayloads, this );
    }
}


void EditorMapDef::LaunchJobs() const {
    unsigned int mapSeed = g_RNG->GetRandomSeed();

    for( int stepIndex = 1; stepIndex <= m_numSteps; stepIndex++ ) {
        std::string mapName = Stringf( "%s: Step %d", m_defType.c_str(), stepIndex );
        RNG* mapRNG = new RNG( mapSeed );
        Map* theMap = new Map( mapName, mapRNG );
        SetMapDef( *theMap );

        AsyncPayload payload;
        payload.theMap = theMap;
        payload.numStepsToDo = stepIndex;

        m_numJobsRunning++;
        m_workerPayloads.Enqueue( payload );
    }

    m_allJobsStarted = true;
}


void EditorMapDef::ProcessWorkerPayloads() const {
    AsyncPayload payload;

    while( !IsFinished() ) {
        while( m_workerPayloads.Dequeue( &payload ) ) {

            Map& theMap = *payload.theMap;
            RNG* mapRNG = GetMapRNG( theMap );
            int mapWidth = mapRNG->GetRandomIntInRange( m_width );
            int mapHeight = mapRNG->GetRandomIntInRange( m_height );
            SetMapDimensions( theMap, IntVec2( mapWidth, mapHeight ) );

            DefineFillAndEdge( theMap );

            if( FinishStep( payload ) ) {
                break;
            }

            // Run Each MapGenStep
            int numSteps = (int)m_mapGenSteps.size();
            bool payloadComplete = false;

            for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
                m_mapGenSteps[stepIndex]->Run( theMap );

                if( FinishStep( payload ) ) {
                    payloadComplete = true;
                    break;
                }
            }

            if( payloadComplete ) {
                break;
            }

            DefineFromContextTiles( theMap );

            if( FinishStep( payload ) ) {
                break;
            }

            DefineTileColliders( theMap );

            if( FinishStep( payload ) ) {
                break;
            } else {
                ERROR_RECOVERABLE( "(EditorMapDef) Worker never sent job back!" );
            }
        }

        std::this_thread::sleep_for( std::chrono::seconds( 0 ) );
    }
}


void EditorMapDef::ProcessMainPayloads() const {
    // Process all of the payloads
    AsyncPayload payload;

    while( m_numJobsRunning > 0 ) {
        while( m_mainPayloads.Dequeue( &payload ) ) {
            m_numJobsRunning--;
            (*m_mapPerStep)[payload.numStepsToDo - 1] = payload.theMap;
        }
    }
}


void EditorMapDef::SpinDownThreads() const {
    // Kill the threads
    int numThreads = (int)m_threads.size();

    for( int threadIndex = 0; threadIndex < numThreads; threadIndex++ ) {
        m_threads[threadIndex].join();
    }

    m_threads.clear();
}

#endif
