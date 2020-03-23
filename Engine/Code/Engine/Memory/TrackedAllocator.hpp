#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Memory/Allocator.hpp"


class TrackedAllocator : public Allocator {
    public:
    void* Malloc( size_t numBytes ) override;
    void  Free( void* pointerToFree ) override;

    static TrackedAllocator* GetInstance();

    private:
};
