#include "Engine/Async/Job.hpp"

#include "Engine/Async/JobSystem.hpp"


Job::Job( JobSystem* jobSystem, int category, const std::string& callbackEvent /*= "" */ ) :
    m_jobSystem( jobSystem ),
    m_category( category ),
    m_callbackEvent( callbackEvent ) {
}


Job::~Job() {

}


// All prerequisites must be added before starting a job chain
void Job::AddPrerequisite( Job* preReqJob ) {
    m_numPrerequisites++;
    preReqJob->m_childJobs.push_back( this );
}


void Job::ResetPrerequisites() {
    m_numPrerequisites = 1;
    m_childJobs.clear();
}


void Job::SetAutoDestroy( bool isAutoDestroyed ) {
    m_isAutoDestroyed = isAutoDestroyed;
}


bool Job::IsFinished() const {
    return m_isFinished;
}


void Job::Finish() {
    m_isFinished = true;
    int numChildren = (int)m_childJobs.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        m_jobSystem->StartJob( m_childJobs[childIndex] );
    }

    if( m_callbackEvent != "" ) {
        m_jobSystem->FinishJob( this );
    } else if( m_isAutoDestroyed ) {
        delete this;
    }
}
