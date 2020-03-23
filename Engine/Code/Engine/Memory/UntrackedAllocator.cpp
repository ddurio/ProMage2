#include "Engine/Memory/UntrackedAllocator.hpp"

#include "Engine/Memory/Memory.hpp"


void* UntrackedAllocator::Malloc( size_t numBytes ) {
    return UntrackedMemory::Malloc( numBytes );
}


void UntrackedAllocator::Free( void* pointerToFree ) {
    UntrackedMemory::Free( pointerToFree );
}


UntrackedAllocator* UntrackedAllocator::GetInstance() {
    static UntrackedAllocator* allocator = new UntrackedAllocator();
    return allocator;
}
