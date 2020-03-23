#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "mutex"
#include "queue"


template< typename T >
class AsyncQueue {
    public:
    void Enqueue( const T& theObject );
    bool Dequeue( T* outObject );

    bool IsEmpty() const;

    private:
    std::queue< T > m_queue;
    std::mutex m_mutex;
};


template< typename T >
void AsyncQueue<T>::Enqueue( const T& theObject ) {
    typename std::scoped_lock myLock( m_mutex );
    m_queue.push( theObject );
}


template< typename T >
bool AsyncQueue<T>::Dequeue( T* outObject ) {
    typename std::scoped_lock myLock( m_mutex );

    int numObjects = (int)m_queue.size();

    if( numObjects == 0 ) {
        return false;
    } 

    *outObject = m_queue.front();
    m_queue.pop();
    return true;
}


template< typename T >
bool AsyncQueue<T>::IsEmpty() const {
    return (m_queue.size() == 0);
}
