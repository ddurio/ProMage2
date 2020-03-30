#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Memory/ObjectAllocator.hpp"

#include "queue"
#include "shared_mutex"
#include "thread"


// EXPAND solves problem with VS compiler and __VA__ARGS__ not getting processed as multiple args
// PROFILE_SCOPE_X simply chooses the third argument:
//      when VA_ARGS is one argument  --> arg list becomes ( va_arg1, profile_scope_2, PROFILE_SCOPE_1 ) -->  chooses correct one arg macro
//      when VA_ARGS is two arguments --> arg list becomes ( va_arg1, va_arg2, PROFILE_SCOPE_2, ... )    -->  chooses correct two arg macro

#define _PROFILE_SCOPE_1( scopeName ) ScopeProfiler MACRO_COMBINE( ____scopeLog, __LINE__ ) ## ( scopeName )
#define _PROFILE_SCOPE_2( scopeName, logToOutput ) ScopeProfiler MACRO_COMBINE( ____scopeLog, __LINE__ ) ## ( scopeName, logToOutput )

#define _PROFILE_SCOPE_X( arg1, arg2, arg3, ... ) arg3
#define _EXPAND( x ) x

// Creates a variable of type Profiler named ____scopeLog<LINE_NUMBER>
// Upon construction stores an HPC, upon destruction (out of scope) prints the duration
#define PROFILE_SCOPE( ... ) _EXPAND( _PROFILE_SCOPE_X( __VA_ARGS__, _PROFILE_SCOPE_2, _PROFILE_SCOPE_1 ) ## (__VA_ARGS__) )
#define PROFILE_FUNCTION() ScopeProfiler MACRO_COMBINE( ____scopeLog, __LINE__ ) ## ( __FUNCTION__ )



struct ProfilerReportNode;
enum ProfilerReportColumn : unsigned int;


struct ProfilerNode {
    // Family Tree
    ProfilerNode* parent        = nullptr;
    ProfilerNode* prevSibling   = nullptr;
    ProfilerNode* nextSibling   = nullptr;
    ProfilerNode* firstChild    = nullptr;
    ProfilerNode* lastChild     = nullptr;

    // Head Specific
    std::thread::id threadID;
    unsigned int refCount = 0;

    // Timing Data
    char label[32];
    uint64_t startHPC   = 0;
    uint64_t endHPC     = 0;

    // Memory Data
    // ???


    void AddChild( ProfilerNode* newChild );
    ProfilerNode* FindNode( const char* labelToFind );
};


class ScopeProfiler {
    public:
    ScopeProfiler( const char* scopeName, bool logToOutput = false );
    ~ScopeProfiler();

    private:
    ProfilerNode* m_nodeToLog = nullptr;
    bool m_logToOutput = false;
};


class ProfilerSystem {
    public:
    ProfilerSystem();

    void Startup();
    void Shutdown();

    void Render();

    void BeginFrame( const char* label = "Frame" );
    void EndFrame();

    ProfilerNode* ProfilePush( const char* label );
    void ProfilePop();

    float GetProfileTime( const char* label, std::thread::id threadID = std::this_thread::get_id() );


    private:
    static thread_local ProfilerNode* t_activeNode;
    static thread_local int t_depthCount;

    ObjectAllocator<ProfilerNode>* m_allocator = nullptr;
    std::queue< ProfilerNode* > m_historyTrees;
    std::shared_mutex m_historyMutex;
    float m_historySeconds = 10.f;

    bool m_isRendering = false;
    bool m_isRunning = true;
    bool m_isPausing = false;
    bool m_isResuming = false;

    ProfilerNode* m_reportBasis = nullptr;
    ProfilerReportNode* m_report = nullptr;
    ProfilerReportColumn m_reportColumnFlags = (ProfilerReportColumn)0U;
    bool m_reportViewTree = true;
    std::string m_reportSortBy = "Total Time";
    bool m_reportOrderDesc = true;


    ~ProfilerSystem(); // Should not be deleted by game or engine

    ProfilerNode* AllocNode() const;
    void FreeNode( ProfilerNode* nodeToFree ) const;

    void ReleaseTree( ProfilerNode* treeToFree ) const;
    void FreeTree( ProfilerNode* treeToFree ) const;

    std::vector< float > GetFrameTimes() const;
    ProfilerNode* GetFrameTree( int index ) const;
    ProfilerNode* GetHistory( std::thread::id threadID, int history = 0 );

    void RenderFrameTimes() const;
    void RenderColumnCheckboxes() const;
    void RenderColumnHeaders();
    void RenderColumnHeader( const char* label );

    // Event Functions -------------------------
    bool SetMaxHistorySeconds( EventArgs& args );
    bool Toggle( EventArgs& args );
    bool GenerateReport( EventArgs& args );
    bool Pause( EventArgs& args );
    bool Resume( EventArgs& args );
};
