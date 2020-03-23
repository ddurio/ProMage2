#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Async/AsyncQueue.hpp"


class Job;


class JobQueue {
    public:
    JobQueue() {};
    ~JobQueue() {};

    void Enqueue( Job* pendingJob );
    bool TryDequeue( Job** outJob );
    void Dequeue( Job** outJob );

    void EnqueueFinished( Job* finishedJob );
    bool TryDequeueFinished( Job** outJob );

    bool HasWork() const;


    private:
    AsyncQueue< Job* > m_pendingQueue;
    AsyncQueue< Job* > m_finishedQueue;
};
