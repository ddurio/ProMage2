#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "thread"
#include "shared_mutex"


class Job;
class JobQueue;


enum JobCategory {
    JOB_CATEGORY_GENERIC = 0,
    JOB_CATEGORY_MAIN,
    JOB_CATEGORY_RENDER,

    NUM_ENGINE_JOB_CATEGORIES
};


constexpr char JOB_FINISHED_KEY[] = "finishedJob";


class JobSystem {
    public:
    JobSystem( int numGenericThreads = -1, int numGameCategories = 0 );
    ~JobSystem();

    void Startup();
    void Shutdown();

    template< typename ...ARGS >
    void StartJob( Job* jobToStartA, Job* jobToStartB, ARGS ...args );
    void StartJob( Job* jobToStart );

    void FinishJob( Job* jobToFinish );

    int ProcessCategory( int categoryName, int milliSeconds = -1 );
    bool ProcessOne( int categoryName );

    int FinishCategory( int categoryName, int milliSeconds = -1 );
    bool FinishOne( int categoryName );

    void WaitForJob( Job* jobToWaitFor, std::vector< int > alternateCategories = std::vector< int >() );


    private:
    bool m_isRunning = false;
    int m_numGenericThreads = -1;
    int m_numGameCategories = 0;

    std::vector< JobQueue* > m_categoryQueues;
    std::vector< std::thread > m_genericThreads;

    std::condition_variable m_workSignal;


    void GenericThreadMain();
};


template< typename ...ARGS >
void JobSystem::StartJob( Job* jobToStartA, Job* jobToStartB, ARGS... args ) {
    StartJob( jobToStartA );
    StartJob( jobToStartB, args... );
}
