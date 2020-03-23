#include "Engine/Memory/Allocator.hpp"


void* operator new(size_t numBytes, Allocator& allocator) {
    return allocator.Malloc( numBytes );
}


void operator delete(void* pointerToFree, Allocator& allocator) {
    allocator.Free( pointerToFree );
}
