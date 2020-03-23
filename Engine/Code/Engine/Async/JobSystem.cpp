#include "Engine/Async/JobSystem.hpp"

#include "Engine/Async/Job.hpp"
#include "Engine/Async/JobQueue.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"


JobSystem::JobSystem( int numGenericThreads /*= -1*/, int numGameCategories /*= 0 */ ) :
    m_numGenericThreads( numGenericThreads ),
    m_numGameCategories( numGameCategories ) {
}


JobSystem::~JobSystem() {

}


void JobSystem::Startup() {
    // Create queues
    int numCategories = NUM_ENGINE_JOB_CATEGORIES + m_numGameCategories;
    m_categoryQueues.reserve( numCategories );

    for( int catIndex = 0; catIndex < numCategories; catIndex++ ) {
        JobQueue* queue = new JobQueue();
        m_categoryQueues.push_back( queue );
    }

    m_isRunning = true;
    int numThreads = m_numGenericThreads;

    // All cores except some number
    if( m_numGenericThreads < 0 ) {
        int numCores = std::thread::hardware_concurrency();
        numThreads = numCores + m_numGenericThreads; // (8 + -n) == (8 - n)
        numThreads = Max( numThreads, 1 );
    }

    // Spin up threads
    for( int threadIndex = 0; threadIndex < numThreads; threadIndex++ ) {
        m_genericThreads.emplace_back( &JobSystem::GenericThreadMain, this );
    }
}


void JobSystem::Shutdown() {
    m_isRunning = false;
    m_workSignal.notify_all();

    int numThreads = (int)m_genericThreads.size();

    for( int threadIndex = 0; threadIndex < numThreads; threadIndex++ ) {
        m_genericThreads[threadIndex].join();
    }

    m_genericThreads.clear();
}


void JobSystem::StartJob( Job* jobToStart ) {
    int remaining = --jobToStart->m_numPrerequisites;
    GUARANTEE_OR_DIE( remaining >= 0, "(JobSystem) ERROR - Remaining PreReqs should never be less than zero!" );

    if( remaining == 0 ) {
        int catIndex = jobToStart->m_category;
        m_categoryQueues[catIndex]->Enqueue( jobToStart );
        m_workSignal.notify_one();
    }
}


void JobSystem::FinishJob( Job* jobToFinish ) {
    int catIndex = jobToFinish->m_category;
    m_categoryQueues[catIndex]->EnqueueFinished( jobToFinish );
}


int JobSystem::ProcessCategory( int categoryName, int milliSeconds /*= -1 */ ) {
/*
    uint64_t startHPC = GetCurrentHPC();
    bool timeLimitHasElapsed = false;

    JobQueue* queue = m_categoryQueues[categoryName];
    Job* jobToProcess = nullptr;
    int numProcessed = 0;

    while( !timeLimitHasElapsed && queue->TryDequeue( &jobToProcess ) ) {
        jobToProcess->Execute();
        jobToProcess->Finish();
        numProcessed++;

        if( milliSeconds > 0 ) {
            uint64_t endHPC = GetCurrentHPC();
            double elapsedSeconds = ConvertHPCTOSeconds( endHPC - startHPC );
            timeLimitHasElapsed = (elapsedSeconds >= (double)(milliSeconds * 0.001f));
        }
    }

    return numProcessed;
*/

    uint64_t startHPC = GetCurrentHPC();
    bool timeLimitHasElapsed = false;
    int numProcessed = 0;

    while( !timeLimitHasElapsed ) {
        if( !ProcessOne( categoryName ) ) {
            break;
        }

        numProcessed++;

        if( milliSeconds > 0 ) {
            uint64_t endHPC = GetCurrentHPC();
            double elapsedSeconds = ConvertHPCTOSeconds( endHPC - startHPC );
            timeLimitHasElapsed = (elapsedSeconds >= (double)(milliSeconds * 0.001f));
        }
    }

    return numProcessed;
}


bool JobSystem::ProcessOne( int categoryName ) {
    JobQueue* queue = m_categoryQueues[categoryName];
    Job* jobToProcess = nullptr;

    if( queue->TryDequeue( &jobToProcess ) ) {
        jobToProcess->Execute();
        jobToProcess->Finish();

        return true;
    }

    return false;
}


int JobSystem::FinishCategory( int categoryName, int milliSeconds /*= -1 */ ) {
    uint64_t startHPC = GetCurrentHPC();
    bool timeLimitHasElapsed = false;
    int numFinished = 0;

    while( !timeLimitHasElapsed ) {
        if( !FinishOne( categoryName ) ) {
            break;
        }

        numFinished++;

        if( milliSeconds > 0 ) {
            uint64_t endHPC = GetCurrentHPC();
            double elapsedSeconds = ConvertHPCTOSeconds( endHPC - startHPC );
            timeLimitHasElapsed = (elapsedSeconds >= (double)(milliSeconds * 0.001f));
        }
    }

    return numFinished;
}


bool JobSystem::FinishOne( int categoryName ) {
    JobQueue* queue = m_categoryQueues[categoryName];
    Job* jobToFinish = nullptr;

    if( queue->TryDequeueFinished( &jobToFinish ) ) {
        EventArgs args;
        args.SetValue( JOB_FINISHED_KEY, jobToFinish );
        g_theEventSystem->FireEvent( jobToFinish->m_callbackEvent, args );

        if( jobToFinish->m_isAutoDestroyed ) {
            CLEAR_POINTER( jobToFinish );
        }

        return true;
    }

    return false;
}


void JobSystem::WaitForJob( Job* jobToWaitFor, std::vector< int > alternateCategories /*= std::vector< int >() */ ) {
    if( jobToWaitFor->m_isAutoDestroyed ) {
        ERROR_RECOVERABLE( "(JobSystem): Cannot wait for a job that auto destroys itself" );
        return;
    }

    int categoryIndex = 0;
    int numCategories = (int)alternateCategories.size() + 1;

    int jobCategory = jobToWaitFor->m_category;

    while( !jobToWaitFor->IsFinished() ) {
        if( !ProcessOne( jobCategory ) ) {
            // didn't find any of that category
            categoryIndex++;
            int altCatIndex = (categoryIndex % numCategories) - 1; // -1 to (numAltCategories - 1) inclusive

            if( altCatIndex < 0 ) {
                jobCategory = jobToWaitFor->m_category;
            } else {
                // Cycle through altCategories
                jobCategory = alternateCategories[altCatIndex];
            }
        }
    }
}


void JobSystem::GenericThreadMain() {
    std::mutex signalMutex;
    std::unique_lock signalLock( signalMutex ); // condition variable requires locked lock

    while( m_isRunning ) {
        m_workSignal.wait( signalLock, [&]() {
            bool hasWork = m_categoryQueues[JOB_CATEGORY_GENERIC]->HasWork();
            return hasWork || !m_isRunning;
        } );

        ProcessCategory( JOB_CATEGORY_GENERIC );
        std::this_thread::yield();
    }
}
