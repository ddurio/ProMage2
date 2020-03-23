#pragma once
#include "Engine/Core/EngineCommon.hpp"


class Allocator {
    public:
    virtual void* Malloc( size_t numBytes ) = 0;
    virtual void  Free( void* pointerToFree ) = 0;

    template< typename T, typename ...ARGS >
    T* New( ARGS&& ...args );

    template< typename T >
    void Delete( T* );


    private:
};


void* operator new( size_t numBytes, Allocator& allocator );
void  operator delete( void* pointerToFree, Allocator& allocator );


template< typename T, typename ...ARGS >
T* Allocator::New( ARGS&& ...args ) {
    void* memory = this->Malloc( sizeof( T ) );

    if( memory != nullptr ) {
        new (memory) T( args... );
    }

    return (T*)memory;
}


template< typename T >
void Allocator::Delete( T* pointerToFree ) {
    if( pointerToFree != nullptr ) {
        pointerToFree->~T();
        this->Free( pointerToFree );
    }
}
