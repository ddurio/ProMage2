#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Debug/Callstack.hpp"

#include "atomic"
#include "mutex"


namespace UntrackedMemory {
    void* Malloc( size_t numBytes );
    void  Free( void* pointerToFree );

    template< typename T >
    struct Allocator {
        using value_type = T;

        Allocator() = default;

        template <class U>
        Allocator( const Allocator<U>& ) {}

        T* allocate( std::size_t numObjects ) {
            if( numObjects <= std::numeric_limits< std::size_t >::max() / sizeof( T ) ) {
                void* newPointer = UntrackedMemory::Malloc( numObjects * sizeof( T ) );

                if( newPointer != nullptr ) {
                    return static_cast<T*>(newPointer);
                }
            }

            throw std::bad_alloc();
        }

        void deallocate( T* pointerToFree, std::size_t n ) {
            UNUSED( n );
            UntrackedMemory::Free( pointerToFree );
        }
    };

    template <typename T, typename U>
    inline bool operator == ( const Allocator<T>&, const Allocator<U>& ) {
        return true;
    }

    template <typename T, typename U>
    inline bool operator != ( const Allocator<T>& a, const Allocator<U>& b ) {
        return !(a == b);
    }
}


namespace TrackedMemory {
#define ENGINE_MEMORY_TRACKING_COUNT      0
#define ENGINE_MEMORY_TRACKING_VERBOSE    1

    struct MemInfo {
        void* trackedPointer = nullptr;
        size_t numBytes = 0;
        Callstack callstack;
    };


    struct MemSummary {
        Callstack callstack;
        size_t numBytes = 0;
        int numAllocs = 0;
    };


    struct CompareNumAllocs {
        bool operator()( const MemSummary& summaryA, const MemSummary& summaryB );
    };


    void* Malloc( size_t numBytes );
    void  Free( void* pointerToFree );

    bool IsEnabled();
    std::string GetMode();
    int GetNumLiveAllocations();
    int GetNumLiveBytes();
    std::string GetByteString( int numBytes );
    std::string GetLiveByteString();
    bool LogLiveAllocations( EventArgs& args );
};
