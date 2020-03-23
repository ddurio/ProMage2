#include "Engine/Async/JobQueue.hpp"

#include "Engine/Async/AsyncQueue.hpp"


void JobQueue::Enqueue( Job* pendingJob ) {
    m_pendingQueue.Enqueue( pendingJob );
}


bool JobQueue::TryDequeue( Job** outJob ) {
    return m_pendingQueue.Dequeue( outJob );
}


void JobQueue::Dequeue( Job** outJob ) {
    while( !m_pendingQueue.Dequeue( outJob ) ) {
        std::this_thread::yield();
    }
}


void JobQueue::EnqueueFinished( Job* finishedJob ) {
    m_finishedQueue.Enqueue( finishedJob );
}


bool JobQueue::TryDequeueFinished( Job** outJob ) {
    return m_finishedQueue.Dequeue( outJob );
}


bool JobQueue::HasWork() const {
    return !m_pendingQueue.IsEmpty();
}
