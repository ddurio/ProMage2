#include "Engine/Memory/Memory.hpp"

#include "Engine/Async/AsyncQueue.hpp"
#include "Engine/Debug/Profiler.hpp"
#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/EngineBuildPreferences.hpp"

#include "chrono"
#include "queue"


namespace TrackedMemory {
    typedef std::map< void*, MemInfo, std::less< void* >, UntrackedMemory::Allocator< std::pair< void* const, MemInfo > > > TrackedMemoryMap;
    typedef std::priority_queue< MemSummary, std::vector< MemSummary >, CompareNumAllocs > OrderedSummaries;

    static TrackedMemoryMap* s_memMap = nullptr;
    static std::atomic<int> s_numTrackedAllocs = 0;
    static size_t s_numTrackedBytes = 0;
    static bool s_initialized = false;
    static DevConsoleChannel s_memChannel = DevConsole::CHANNEL_UNDEFINED;


    bool CompareNumAllocs::operator()( const MemSummary& summaryA, const MemSummary& summaryB ) {
        return (summaryA.numBytes > summaryB.numBytes);
    }
}


static std::mutex& GetMemoryLock() {
    static std::mutex memLock;
    return memLock;
}


static TrackedMemory::TrackedMemoryMap& GetMemoryMap() {
    using namespace TrackedMemory;

    if( s_memMap == nullptr ) {
        s_memMap = (TrackedMemoryMap*)UntrackedMemory::Malloc( sizeof( TrackedMemoryMap ) );
        new (s_memMap) TrackedMemoryMap();
    }

    return *TrackedMemory::s_memMap;
}


void* UntrackedMemory::Malloc( size_t numBytes ) {
    return ::malloc( numBytes );
}


void UntrackedMemory::Free( void* pointerToFree ) {
    ::free( pointerToFree );
}


void* TrackedMemory::Malloc( size_t numBytes ) {
    void* newPointer = UntrackedMemory::Malloc( numBytes );

#if defined( ENGINE_MEMORY_TRACKING )
#if ENGINE_MEMORY_TRACKING >= ENGINE_MEMORY_TRACKING_COUNT
    if( !s_initialized && g_theDevConsole != nullptr && g_theEventSystem != nullptr ) {
        s_initialized = true;
        s_memChannel = g_theDevConsole->AddChannel( "memTracking", Rgba::ORGANIC_BROWN, false );
        g_theEventSystem->Subscribe( "logLiveAllocs", &LogLiveAllocations );
    }

    s_numTrackedAllocs++;
#endif

#if ENGINE_MEMORY_TRACKING >= ENGINE_MEMORY_TRACKING_VERBOSE
    s_numTrackedBytes += numBytes;

    MemInfo info;
    info.trackedPointer = newPointer;
    info.numBytes = numBytes;
    info.callstack = Callstack::GetCallstack( 1 );

    GetMemoryLock().lock();
    TrackedMemoryMap& memMap = GetMemoryMap();
    memMap[newPointer] = info;
    GetMemoryLock().unlock();
#endif
#endif

    return newPointer;
}


void TrackedMemory::Free( void* pointerToFree ) {
#if defined( ENGINE_MEMORY_TRACKING )
#if ENGINE_MEMORY_TRACKING >= ENGINE_MEMORY_TRACKING_COUNT
    s_numTrackedAllocs--;
#endif

#if ENGINE_MEMORY_TRACKING >= ENGINE_MEMORY_TRACKING_VERBOSE
    GetMemoryLock().lock();
    TrackedMemoryMap& memMap = GetMemoryMap();
    s_numTrackedBytes -= memMap[pointerToFree].numBytes;
    memMap.erase( pointerToFree );
    GetMemoryLock().unlock();
#endif
#endif

    UntrackedMemory::Free( pointerToFree );
}


bool TrackedMemory::IsEnabled() {
    bool isEnabled = false;
#if defined( ENGINE_MEMORY_TRACKING )
#if ENGINE_MEMORY_TRACKING >= ENGINE_MEMORY_TRACKING_COUNT
    isEnabled = true;
#endif
#else
#endif
    return isEnabled;
}


std::string TrackedMemory::GetMode() {
    std::string modeStr = "MemTracking Disabled";
#if defined( ENGINE_MEMORY_TRACKING )
    modeStr = "MemTracking Enabled";
#if ENGINE_MEMORY_TRACKING == ENGINE_MEMORY_TRACKING_COUNT
    modeStr = "MemTracking NumAllocations";
#elif ENGINE_MEMORY_TRACKING == ENGINE_MEMORY_TRACKING_VERBOSE
    modeStr = "MemTracking Verbose";
#endif
#endif
    return modeStr;
}


int TrackedMemory::GetNumLiveAllocations() {
    return s_numTrackedAllocs;
}


int TrackedMemory::GetNumLiveBytes() {
/*
    size_t totalBytes = 0;
    TrackedMemoryMap& memMap = GetMemoryMap();
    TrackedMemoryMap::const_iterator mapIter = memMap.cbegin();

    while( mapIter != memMap.cend() ) {
        totalBytes += mapIter->second.numBytes;

        mapIter++;
    }

    return (int)totalBytes;
*/
    return (int)s_numTrackedBytes;
}


std::string TrackedMemory::GetByteString( int numBytes ) {
    if( numBytes < 1024 ) {
        return Stringf( "%d B", numBytes );
    }

    float kiloBytes = numBytes / 1024.f;

    if( kiloBytes < 1024.f ) {
        return Stringf( "%.3f KB", kiloBytes );
    }

    float megaBytes = kiloBytes / 1024.f;

    if( megaBytes < 1024.f ) {
        return Stringf( "%.3f MB", megaBytes );
    }

    float gigaByte = megaBytes / 1024.f;
    return Stringf( "%.3f GB", gigaByte );
}


std::string TrackedMemory::GetLiveByteString() {
    int numBytes = GetNumLiveBytes();
    return GetByteString( numBytes );
}


static TrackedMemory::OrderedSummaries GetAllocationSummaries() {
    TrackedMemory::TrackedMemoryMap memMapCopy = GetMemoryMap();
    TrackedMemory::TrackedMemoryMap::const_iterator memIter = memMapCopy.begin();
    std::map< void*, TrackedMemory::MemSummary > memSummaries;

    // Build the map (callstack hash ---> memSummary): this effectively groups by callstack
    while( memIter != memMapCopy.end() ) {
        const TrackedMemory::MemInfo& info = memIter->second;
        TrackedMemory::MemSummary& summary = memSummaries[info.callstack.GetHash()];

        summary.callstack = info.callstack;
        summary.numBytes += info.numBytes;
        summary.numAllocs++;

        memIter++;
    }

    // Get just the map values (summaries)
    std::map< void*, TrackedMemory::MemSummary >::const_iterator summaryIter = memSummaries.begin();
    TrackedMemory::OrderedSummaries outSummaries;

    while( summaryIter != memSummaries.end() ) {
        outSummaries.push( summaryIter->second );

        summaryIter++;
    }

    return outSummaries;
}


bool TrackedMemory::LogLiveAllocations( EventArgs& args ) {
    UNUSED( args );
    DebuggerPrintf( "\n=== Begin Memory Log ===\n\n" );

    // Details Per Callstack
    OrderedSummaries memSummaries = GetAllocationSummaries();

    while( !memSummaries.empty() ) {
        const MemSummary& summary = memSummaries.top();

        std::string byteStr = GetByteString( (int)summary.numBytes );
        std::string allocStr = Stringf( "\n- %s from %d allocation(s)", byteStr.c_str(), summary.numAllocs );
        DebuggerPrintf( allocStr.c_str() );
        summary.callstack.Print( s_memChannel );

        memSummaries.pop();
    }

    // Totals
    std::string liveByteStr = GetLiveByteString();
    int liveAllocs = GetNumLiveAllocations();
    std::string totalAllocStr = Stringf( "\n%s live from %d allocation(s)...\n", liveByteStr.c_str(), liveAllocs );

    DebuggerPrintf( totalAllocStr.c_str() );
    DebuggerPrintf( "=== End Memory Log ===\n\n" );

    return true;
}


void* operator new( size_t numBytes ) {
    return TrackedMemory::Malloc( numBytes );
}


void operator delete( void* pointerToDelete ) {
    TrackedMemory::Free( pointerToDelete );
}


// UnitTests -------------------------------
#if defined( ENGINE_MEMORY_TRACKING )
static void AllocTest( AsyncQueue<void*>& memQueue, std::atomic<int>& numRunningThreads ) {
    RNG testRNG;
    int itersPerThread = 10'000;
    int numBytesToAlloc = 128;

    for( int iterIndex = 0; iterIndex < itersPerThread; iterIndex++ ) {
        if( testRNG.PercentChance( 0.5f ) ) {
            char* ptr = (char*)TrackedMemory::Malloc( numBytesToAlloc );

            // just doing this to slow it down
            for( int charIndex = 0; charIndex < numBytesToAlloc; ++charIndex ) {
                ptr[charIndex] = (char)charIndex;
            }

            memQueue.Enqueue( ptr );
        } else {
            void* ptr;
            if( memQueue.Dequeue( &ptr ) ) {
                TrackedMemory::Free( ptr );
            }
        }
    }

    numRunningThreads--;
}


UNITTEST( "Threads", "MemTracking", 10 ) {
    int preAllocs = TrackedMemory::GetNumLiveAllocations();
    int preBytes = TrackedMemory::GetNumLiveBytes();

    {
        using namespace std::chrono_literals;
        AsyncQueue<void*> memQueue;

        unsigned int numCores = std::thread::hardware_concurrency();
        std::atomic<int> numRunningThreads = numCores;

        for( unsigned int i = 0; i < numCores; ++i ) {
            std::thread test_thread( AllocTest, std::ref( memQueue ), std::ref( numRunningThreads ) );
            test_thread.detach();
        }

        while( numRunningThreads.load() > 0 ) {
            std::this_thread::sleep_for( 100ms );
        }

        void* ptr;
        while( memQueue.Dequeue( &ptr ) ) {
            TrackedMemory::Free( ptr );
        }
    }

    int postAllocs = TrackedMemory::GetNumLiveAllocations();
    int postBytes = TrackedMemory::GetNumLiveBytes();

    UnitTest::VerifyResult( (preAllocs == postAllocs), "(TrackedMemory) Number of allocations changed", theTest );
    UnitTest::VerifyResult( (preBytes == postBytes), "(TrackedMemory) Number of bytes allocated changed", theTest );
}
#endif
