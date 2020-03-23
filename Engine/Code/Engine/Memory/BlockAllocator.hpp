#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Memory/Allocator.hpp"

#include "mutex"


class BlockAllocator : public Allocator {
    public:
    BlockAllocator( void* buffer, size_t bufferSize, size_t blockSize, size_t alignment );
    BlockAllocator( Allocator* baseAllocator, size_t blocksPerChunk, size_t blockSize, size_t alignment );
    virtual ~BlockAllocator();

    void* Malloc();
    void  Free( void* pointerToFree ) override;


    private:
    struct ListNode {
        ListNode* next = nullptr;
    };


    Allocator* m_baseAllocator = nullptr;

    size_t m_bufferSize = 0;
    size_t m_blockSize = 0;
    size_t m_blocksPerChunk = 0;

    size_t m_alignment = 0; // Means nothing for now

    ListNode* m_freeBlocks = nullptr;
    ListNode* m_chunks = nullptr;

    std::recursive_mutex m_blockLock;
    std::mutex m_chunkLock;


    void* Malloc( size_t numBytes ) override;

    void  PushFreeBlock( void* freeBlock );
    void* PopFreeBlock();

    bool AddNewChunk();
    void SubdivideChunk( void* chunkStart );
};
