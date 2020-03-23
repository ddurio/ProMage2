#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Async/AsyncQueue.hpp"
#include "Engine/Async/AsyncRingBuffer.hpp"
#include "Engine/Debug/Callstack.hpp"

#include "thread"
#include "shared_mutex"


class Callstack;


class LogSystem {
    public:
    struct LogPayload {
        char* content = nullptr;
        char* filter = nullptr;
        uint64_t hpc = 0;
    };


    LogSystem();
    ~LogSystem();

    void Startup();
    void Shutdown();

    void PrintString( const std::string& content, const std::string& filter = "" );
    void Flush();

    void EnableAllFilters();
    void DisableAllFilters();
    void EnableFilter( const std::string& filterToEnable );
    void DisableFilter( const std::string& filterToDisable );


    private:
    std::string m_filePath = "";
    std::thread m_logThread;

    bool m_logIsRunning = false;
    bool m_flushRequested = false;

    std::condition_variable m_workSignal;
    AsyncRingBuffer m_logPayloads;

    Strings m_filterList;
    bool m_filtersAllAllowed = true;
    mutable std::shared_mutex m_filterMutex;

    void ProcessLogThread();

    bool IsFilterAllowed( const std::string& filter ) const;
    void AddFilterToList( const std::string& filter );
    void RemoveFilterFromList( const std::string& filter );

    void StopLogThread();
};
