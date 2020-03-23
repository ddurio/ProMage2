#include "Engine/Memory/TrackedAllocator.hpp"

#include "Engine/Memory/Memory.hpp"


void* TrackedAllocator::Malloc( size_t numBytes ) {
    return TrackedMemory::Malloc( numBytes );
}


void TrackedAllocator::Free( void* pointerToFree ) {
    TrackedMemory::Free( pointerToFree );
}


TrackedAllocator* TrackedAllocator::GetInstance() {
    static TrackedAllocator* allocator = new TrackedAllocator();
    return allocator;
}
