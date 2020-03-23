#include "Engine/Memory/BlockAllocator.hpp"


BlockAllocator::BlockAllocator( void* buffer, size_t bufferSize, size_t blockSize, size_t alignment ) :
    m_bufferSize( bufferSize ),
    m_blockSize( blockSize ),
    m_alignment( alignment ) {
    m_blocksPerChunk = m_bufferSize / m_blockSize;
    SubdivideChunk( buffer );
}


BlockAllocator::BlockAllocator( Allocator* baseAllocator, size_t blocksPerChunk, size_t blockSize, size_t alignment ) :
    m_baseAllocator( baseAllocator ),
    m_blocksPerChunk( blocksPerChunk ),
    m_blockSize( blockSize ),
    m_alignment( alignment ) {

    AddNewChunk();
}


BlockAllocator::~BlockAllocator() {
    if( m_baseAllocator != nullptr ) {
        std::scoped_lock localLock( m_chunkLock );

        while( m_chunks != nullptr ) {
            ListNode* chunk = m_chunks;
            m_chunks = chunk->next;

            m_baseAllocator->Free( chunk );
        }
    }
}


void* BlockAllocator::Malloc() {
    void* newBlock = PopFreeBlock();

    while( newBlock == nullptr ) {
        if( !AddNewChunk() ) {
            return nullptr;
        }

        newBlock = PopFreeBlock();
    }

    return newBlock;
}


void BlockAllocator::Free( void* pointerToFree ) {
    PushFreeBlock( pointerToFree );
}


// PRIVATE -------------------------------------------
void* BlockAllocator::Malloc( size_t numBytes ) {
    if( numBytes > m_blockSize ) {
        return nullptr;
    }

    return Malloc();
}


void BlockAllocator::PushFreeBlock( void* freeBlock ) {
    ListNode* node = (ListNode*)freeBlock;
    std::scoped_lock< std::recursive_mutex > localLock( m_blockLock );
    node->next = m_freeBlocks;
    m_freeBlocks = node;
}


void* BlockAllocator::PopFreeBlock() {
    std::scoped_lock< std::recursive_mutex > localLock( m_blockLock );
    void* freeBlock = m_freeBlocks;

    if( m_freeBlocks != nullptr ) {
        m_freeBlocks = ((ListNode*)freeBlock)->next;
    }

    return freeBlock;
}


bool BlockAllocator::AddNewChunk() {
    if( m_baseAllocator == nullptr ) {
        return false;
    }

    if( m_chunkLock.try_lock() ) {
        size_t chunkSize = (m_blocksPerChunk * m_blockSize) + sizeof( ListNode );
        ListNode* newChunk = (ListNode*)(m_baseAllocator->Malloc( chunkSize ));

        if( newChunk == nullptr ) {
            return false;
        }

        newChunk->next = m_chunks;
        m_chunks = newChunk;

        SubdivideChunk( m_chunks + 1 );
        m_chunkLock.unlock();
    }

    return true;
}


void BlockAllocator::SubdivideChunk( void* chunkStart ) {
    unsigned char* buffer = (unsigned char*)chunkStart;
    ListNode* firstBlock = (ListNode*)chunkStart;

    ListNode* blockHead = nullptr;

    for( int blockIndex = 0; blockIndex < m_blocksPerChunk; blockIndex++ ) {
        ListNode* node = (ListNode*)buffer;
        buffer += m_blockSize;

        node->next = blockHead;
        blockHead = node;
    }

    std::scoped_lock< std::recursive_mutex > localLock( m_blockLock );
    firstBlock->next = m_freeBlocks;
    m_freeBlocks = blockHead;
}
