#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Async/JobSystem.hpp"

#include "atomic"


class Job {
    friend class JobSystem;

    public:
    Job( JobSystem* jobSystem, int category, const std::string& callbackEvent = "" );
    virtual ~Job();

    void AddPrerequisite( Job* preReqJob );
    void ResetPrerequisites();

    void SetAutoDestroy( bool isAutoDestroyed );
    bool IsFinished() const;


    private:
    JobSystem* m_jobSystem = nullptr;
    std::string m_callbackEvent = "";
    int m_category = JOB_CATEGORY_GENERIC;

    std::atomic< int > m_numPrerequisites = 1;
    std::vector< Job* > m_childJobs;

    bool m_isAutoDestroyed = true;
    bool m_isFinished = false;


    virtual void Execute() = 0;
    void Finish();
};
