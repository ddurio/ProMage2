#include "Engine/Async/AsyncRingBuffer.hpp"

#include "Engine/Memory/Memory.hpp"


AsyncRingBuffer::AsyncRingBuffer( int numBytes ) {
    m_bufferStart = (unsigned char*)UntrackedMemory::Malloc( numBytes );
    m_bufferSize = numBytes;
}


AsyncRingBuffer::~AsyncRingBuffer() {
    if( m_bufferStart != nullptr ) {
        UntrackedMemory::Free( m_bufferStart );
        m_bufferStart = nullptr;
        m_bufferSize = 0;
    }
}


void* AsyncRingBuffer::LockWrite( int numBytes ) {
    void* writePtr = TryLockWrite( numBytes );

    while( writePtr == nullptr ) {
        writePtr = TryLockWrite( numBytes );
    }

    return writePtr;
}


void* AsyncRingBuffer::TryLockWrite( int requestedSize ) {
    GUARANTEE_OR_DIE( (requestedSize > 0) && (requestedSize < (1 << 30)), Stringf( "(AsyncRingBuffer): Cannot allocate space of size %d", requestedSize ) );
    int headerSize = sizeof( Header );
    int totalSize = headerSize + requestedSize; // Accounts for header for current call
    int extraSize = totalSize + headerSize; // Accounts for extra skip header by next call if needed

    std::scoped_lock localLock( m_mutex );

    if( !HasEnoughSpace( extraSize ) ) {
        return nullptr;
    }

    int newWriteOffset = m_writeOffset + totalSize;

    if( newWriteOffset > m_bufferSize ) { // Need to wrap
        // Write skip header at end
        Header* skipHeader = (Header*)(m_bufferStart + m_writeOffset);
        skipHeader->numBytes = 0;
        skipHeader->isReady = 1;

        // Wrap write offset to start
        m_writeOffset = 0;

        if( !HasEnoughSpace( extraSize ) ) {
            return nullptr;
        }
    }

    // Write offset is a valid memory location now
    Header* headerPtr = (Header*)(m_bufferStart + m_writeOffset);
    headerPtr->numBytes = requestedSize;
    headerPtr->isReady = 0;

    m_writeOffset += totalSize;
    void* dataPtr = headerPtr + 1;

    return dataPtr;
}


void AsyncRingBuffer::UnlockWrite( void* dataPtr ) {
    Header* headerPtr = ((Header*)dataPtr) - 1;
    headerPtr->isReady = 1;
}


void* AsyncRingBuffer::TryLockRead( int& out_numBytes ) {
    std::scoped_lock localLock( m_mutex );

    while( true ) {
        if( m_readOffset == m_writeOffset ) {
            return nullptr;
        }

        Header* headerPtr = (Header*)(m_bufferStart + m_readOffset);

        if( headerPtr->isReady ) {
            if( headerPtr->numBytes == 0 ) { // Found a skip header
                m_readOffset = 0; // Run through while loop again
            } else {
                out_numBytes = headerPtr->numBytes;
                void* dataPtr = headerPtr + 1;

                return dataPtr;
            }
        } else {
            return nullptr;
        }
    }

}


void AsyncRingBuffer::UnlockRead( void* dataPtr ) {
    Header* headerPtr = ((Header*)dataPtr) - 1;
    GUARANTEE_OR_DIE( (void*)headerPtr == (m_bufferStart + m_readOffset), "(AsyncRingBuffer): Attempting to unlock read not aligned with buffer's read head." );

    int totalSize = sizeof( Header ) + headerPtr->numBytes;
    m_readOffset += totalSize;
}


void AsyncRingBuffer::Write( const void* dataPtr, int numBytes ) {
    char* bufferPtr = (char*)LockWrite( numBytes );
    memcpy( bufferPtr, (const char*)dataPtr, numBytes );
    UnlockWrite( bufferPtr );
}


bool AsyncRingBuffer::Read( const void*& out_dataPtr, int& out_numBytes ) {
    int numBytes = 0;
    void* dataPtr = TryLockRead( numBytes );

    if( dataPtr != nullptr ) {
        out_dataPtr = dataPtr;
        out_numBytes = numBytes;
        return true;
    }

    return false;
}


bool AsyncRingBuffer::IsEmpty() const {
    return (m_readOffset == m_writeOffset);
}


bool AsyncRingBuffer::HasEnoughSpace( int numBytes ) const {
    int remaining = 0;

    if( m_writeOffset >= m_readOffset ) {
        // Normal case: free space is everything from start up to read head AND everything after write head
        remaining = m_readOffset;
        remaining += (m_bufferSize - m_writeOffset);
    } else {
        // Wrapped case: everything from write head to read head
        remaining = m_readOffset - m_writeOffset;
    }

    return (numBytes < remaining);
}
