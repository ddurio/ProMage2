#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Memory/Allocator.hpp"


class UntrackedAllocator : public Allocator {
    public:
    void* Malloc( size_t numBytes ) override;
    void  Free( void* pointerToFree ) override;

    static UntrackedAllocator* GetInstance();

    private:
};
