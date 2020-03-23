#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/DevConsole/DevConsole.hpp"


class Callstack {
    public:
    static Callstack GetCallstack( int numSkippedFrames = 0 );

    void* GetHash() const;
    Strings GetAsStrings() const;
    void Print( const DevConsoleChannel& extraChannels = DevConsole::CHANNEL_UNDEFINED ) const;

    private:
    static const int s_maxNumFrames = 64;
    static bool s_initialized;

    void* m_pointerStack[s_maxNumFrames];
    int m_numFrames = 0;
    void* m_hashKey = nullptr;
};
