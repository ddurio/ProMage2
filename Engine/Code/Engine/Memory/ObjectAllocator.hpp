#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Memory/BlockAllocator.hpp"


template < typename ObjType >
class ObjectAllocator : private BlockAllocator {
    public:
    ObjectAllocator( void* buffer, size_t bufferSize );
    ObjectAllocator( Allocator* baseAllocator, size_t blocksPerChunk );
    ~ObjectAllocator() {};

    template < typename ...ARGS >
    ObjType* Malloc( ARGS&&... args );
    void  Free( ObjType* pointerToFree );


    private:
};


template < typename ObjType >
ObjectAllocator<ObjType>::ObjectAllocator( void* buffer, size_t bufferSize ) :
    BlockAllocator( buffer, bufferSize, sizeof( ObjType ), alignof( ObjType ) ) {
};


template < typename ObjType >
ObjectAllocator<ObjType>::ObjectAllocator( Allocator* baseAllocator, size_t blocksPerChunk ) :
    BlockAllocator( baseAllocator, blocksPerChunk, sizeof( ObjType ), alignof( ObjType ) ) {
};


template < typename ObjType >
template < typename ...ARGS >
ObjType* ObjectAllocator<ObjType>::Malloc( ARGS&&... args ) {
    void* newMemory = BlockAllocator::Malloc();

    if( newMemory == nullptr ) {
        return nullptr;
    }

    ObjType* newObj = new (newMemory) ObjType( std::forward( args )... );
    return newObj;
}


template < typename ObjType >
void ObjectAllocator<ObjType>::Free( ObjType* pointerToFree ) {
    BlockAllocator::Free( pointerToFree );
}
