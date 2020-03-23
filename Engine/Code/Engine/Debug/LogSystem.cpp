#include "Engine/Debug/LogSystem.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Debug/Profiler.hpp"
#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/NamedStrings.hpp"

#include "fstream"


LogSystem* g_theLogger = nullptr;


LogSystem::LogSystem() :
    m_logPayloads( AsyncRingBuffer( 1'000'000 ) ) {
}


LogSystem::~LogSystem() {

}


void LogSystem::Startup() {
    if( this == nullptr ) {
        return;
    }

    m_filePath = g_theGameConfigBlackboard.GetValue( "logFilePath", "Data/Logs/Guardian.log" );

    m_logIsRunning = true;
    m_logThread = std::thread( &LogSystem::ProcessLogThread, this );
}


void LogSystem::Shutdown() {
    if( this == nullptr ) {
        return;
    }

    StopLogThread();
    m_workSignal.notify_one();
    m_logThread.join();
}


void LogSystem::PrintString( const std::string& content, const std::string& filter /*= ""*/ ) {
    PROFILE_FUNCTION();

    if( this == nullptr || !IsFilterAllowed( filter ) ) {
        return;
    }

    LogPayload payload;

    // Get sizes
    int payloadBytes = sizeof( LogPayload );
    int filterBytes = (int)filter.size() + 1; // Need to include null terminator
    int contentBytes = (int)content.size() + 1;
    int totalSize = payloadBytes + filterBytes + contentBytes;

    // Setup pointers
    void* writePtr = m_logPayloads.LockWrite( totalSize );

    LogPayload* payloadPtr = (LogPayload*)writePtr;
    payload.filter = (char*)writePtr + payloadBytes;
    payload.content = payload.filter + filterBytes;
    payload.hpc = GetCurrentHPC();

    // Fill data
    *payloadPtr = payload;
    strcpy_s( payload.filter, filterBytes, filter.c_str() );
    strcpy_s( payload.content, contentBytes, content.c_str() );

    // Unlock ringBuffer
    m_logPayloads.UnlockWrite( payloadPtr );
    m_workSignal.notify_one();
}


void LogSystem::Flush() {
    if( this == nullptr ) {
        return;
    }

    m_flushRequested = true;
    m_workSignal.notify_one();

    while( m_flushRequested ) {
        std::this_thread::yield();
    }
}


void LogSystem::EnableAllFilters() {
    if( this == nullptr ) {
        return;
    }

    m_filtersAllAllowed = true;
    m_filterList.clear();
}


void LogSystem::DisableAllFilters() {
    if( this == nullptr ) {
        return;
    }

    m_filtersAllAllowed = false;
    m_filterList.clear();
}


void LogSystem::EnableFilter( const std::string& filterToEnable ) {
    if( this == nullptr ) {
        return;
    }

    if( m_filtersAllAllowed ) {
        RemoveFilterFromList( filterToEnable );
    } else {
        AddFilterToList( filterToEnable );
    }
}


void LogSystem::DisableFilter( const std::string& filterToDisable ) {
    if( this == nullptr ) {
        return;
    }

    if( m_filtersAllAllowed ) {
        AddFilterToList( filterToDisable );
    } else {
        RemoveFilterFromList( filterToDisable );
    }
}


void LogSystem::ProcessLogThread() {
    // Open log file
    std::ofstream fileHandle;

    if( !OpenFileForWrite( m_filePath.c_str(), fileHandle ) ) {
        return;
    }

    // Setup necessary variables
    const void* payloadPtr = nullptr;
    int readBytes = 0;

    std::mutex signalMutex;
    std::unique_lock signalLock( signalMutex ); // condition variable requires locked lock

    // Start looping
    while( m_logIsRunning ) {
        m_workSignal.wait( signalLock, [&]() {
            bool hasWork = !m_logPayloads.IsEmpty();
            bool hasStopped = !m_logIsRunning;
            return hasWork || hasStopped || m_flushRequested;
        } );
        PROFILE_FUNCTION();

        while( m_logPayloads.Read( payloadPtr, readBytes ) ) {
            LogPayload& payload = *(LogPayload*)payloadPtr;

            std::string filterStr = (StringICmp( payload.filter, "" )) ? "" : Stringf( " (%s)", payload.filter );
            std::string logContent = Stringf( "%u%s: %s\n", payload.hpc, filterStr.c_str(), payload.content );
            WriteToFile( fileHandle, logContent );

            m_logPayloads.UnlockRead( (void*)payloadPtr );
        }

        if( m_flushRequested ) {
            m_flushRequested = false;
            fileHandle.flush();
        }
    }
}


bool LogSystem::IsFilterAllowed( const std::string& filter ) const {
    std::shared_lock< std::shared_mutex > localLock( m_filterMutex );

    if( m_filtersAllAllowed ) {
        return !EngineCommon::VectorContains( m_filterList, filter );
    } else {
        return EngineCommon::VectorContains( m_filterList, filter );
    }
}


void LogSystem::AddFilterToList( const std::string& filter ) {
    std::unique_lock< std::shared_mutex > localLock( m_filterMutex );
    m_filterList.push_back( filter );
}


void LogSystem::RemoveFilterFromList( const std::string& filter ) {
    std::unique_lock< std::shared_mutex > localLock( m_filterMutex );
    Strings::const_iterator filterIter = m_filterList.begin();

    while( filterIter != m_filterList.end() ) {
        if( StringICmp( filter, *filterIter ) ) {
            m_filterList.erase( filterIter );
            return;
        }

        filterIter++;
    }
}


void LogSystem::StopLogThread() {
    m_logIsRunning = false;
}


