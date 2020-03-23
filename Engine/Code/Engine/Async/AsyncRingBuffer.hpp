#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "mutex"


// Intended for Multiple Producer Single Consumer use case
class AsyncRingBuffer {
    public:
    AsyncRingBuffer( int numBytes );
    ~AsyncRingBuffer();

    void* LockWrite( int numBytes );
    void* TryLockWrite( int numBytes );
    void UnlockWrite( void* dataPtr );

    void* TryLockRead( int& out_numBytes );
    void UnlockRead( void* dataPtr );

    void Write( const void* dataPtr, int numBytes );
    bool Read( const void*& out_dataPtr, int& out_numBytes );

    bool IsEmpty() const;


    private:
    struct Header {
        unsigned int numBytes : 31;
        unsigned int isReady : 1;
    };

    unsigned char* m_bufferStart = nullptr;
    int m_bufferSize = 0;

    int m_readOffset = 0;
    int m_writeOffset = 0;

    std::mutex m_mutex;


    bool HasEnoughSpace( int numBytes ) const;
};
