#include "Engine/Debug/Profiler.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Debug/ProfilerReport.hpp"
#include "Engine/Debug/UnitTests.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Memory/TrackedAllocator.hpp"


ProfilerSystem* g_theProfiler = new ProfilerSystem();


// Tree Node ---------------------------------------------------------
void ProfilerNode::AddChild( ProfilerNode* newChild ) {
    newChild->parent = this;

    if( firstChild == nullptr && lastChild == nullptr ) {
        firstChild = newChild;
        lastChild  = newChild;
        return;
    }

    lastChild->nextSibling = newChild;
    newChild->prevSibling  = lastChild;
    lastChild = newChild;
}


ProfilerNode* ProfilerNode::FindNode( const char* labelToFind ) {
    if( strcmp( label, labelToFind ) == 0 ) {
        return this;
    }

    if( firstChild != nullptr ) {
        ProfilerNode* child = firstChild->FindNode( labelToFind );

        if( child != nullptr ) {
            return child;
        }
    }

    if( nextSibling != nullptr ) {
        return nextSibling->FindNode( labelToFind );
    }

    return nullptr;
}


// Local object from macro --------------------------------------------
ScopeProfiler::ScopeProfiler( const char* scopeName, bool logToOutput /*= false */ ) :
    m_logToOutput( logToOutput ) {
    m_nodeToLog = g_theProfiler->ProfilePush( scopeName );
}


ScopeProfiler::~ScopeProfiler() {
    if( m_logToOutput && m_nodeToLog != nullptr ) {
        unsigned long long numHPC = GetCurrentHPC() - m_nodeToLog->startHPC;
        DebuggerPrintf( "\n(Profiler) %s took %s..\n\n", m_nodeToLog->label, GetProfileTimeString( numHPC ).c_str() );
    }

    g_theProfiler->ProfilePop();
}


// Profiler --------------------------------------------------------
ProfilerSystem::ProfilerSystem() {
    m_allocator = new ObjectAllocator<ProfilerNode>( TrackedAllocator::GetInstance(), 1'000'000 );

    m_reportColumnFlags = REPORT_COLUMN_NUM_CALLS
                        | REPORT_COLUMN_PERCENT_TOTAL
                        | REPORT_COLUMN_PERCENT_SELF
                        | REPORT_COLUMN_SECONDS_TOTAL
                        | REPORT_COLUMN_SECONDS_SELF;

    g_theEventSystem->Subscribe( "profilerSetMaxHistory",   this, &ProfilerSystem::SetMaxHistorySeconds );
    g_theEventSystem->Subscribe( "profilerToggle",          this, &ProfilerSystem::Toggle );
    g_theEventSystem->Subscribe( "profilerReport",          this, &ProfilerSystem::GenerateReport );
    g_theEventSystem->Subscribe( "profilerPause",           this, &ProfilerSystem::Pause );
    g_theEventSystem->Subscribe( "profilerResume",          this, &ProfilerSystem::Resume );
}


void ProfilerSystem::Startup() {

}


void ProfilerSystem::Shutdown() {

}


void ProfilerSystem::Render() {
    if( !m_isRendering ) {
        return;
    }

    g_theGui->CreateStaticWindow( Vec2::ONE, ALIGN_CENTER, "Profiler" );

    RenderFrameTimes();
    ImGui::SameLine();

    if( m_isRunning ) {
        if( ImGui::Button( "Pause" ) ) {
            g_theEventSystem->FireEvent( "profilerPause" );
        }
    } else {
        if( ImGui::Button( "Resume" ) ) {
            g_theEventSystem->FireEvent( "profilerResume" );
        }
    }

    bool reportOpen = ImGui::Button( "Generate Report" );
    RenderColumnCheckboxes();

    if( ImGui::RadioButton( "Tree View", m_reportViewTree ) ) {
        m_reportViewTree = true;

        EventArgs args;
        args.SetValue( "framePtr", m_reportBasis );
        g_theEventSystem->FireEvent( "profilerReport", args );
    }

    ImGui::SameLine();

    if( ImGui::RadioButton( "Flat View", !m_reportViewTree ) ) {
        m_reportViewTree = false;

        EventArgs args;
        args.SetValue( "framePtr", m_reportBasis );
        g_theEventSystem->FireEvent( "profilerReport", args );
    }



    if( reportOpen ) {
        g_theEventSystem->FireEvent( "profilerReport" );
    }

    if( m_report != nullptr ) {
        RenderColumnHeaders();
        m_report->Render( m_reportColumnFlags );
    }

    ImGui::End();
}


void ProfilerSystem::BeginFrame( const char* label /*= "BeginFrame" */ ) {
    GUARANTEE_OR_DIE( t_activeNode == nullptr, "(Profiler) ERROR -- Profile tree started before start of frame" );
    ProfilePush( label );
}


void ProfilerSystem::EndFrame() {
    ProfilePop();
    GUARANTEE_OR_DIE( t_depthCount == 0, "(Profiler) ERROR -- Profile tree not completed before end of frame" );

    // Free up old trees
    if( m_isRunning ) {
        std::scoped_lock< std::shared_mutex > localLock( m_historyMutex );
        uint64_t offsetHPC = GetOffsetHPC( -m_historySeconds );

        int numTrees = (int)m_historyTrees.size();

        for( int treeIndex = 0; treeIndex < numTrees; treeIndex++ ) {
            ProfilerNode* root = m_historyTrees.front();

            if( offsetHPC < root->endHPC ) {
                break;
            }

            ReleaseTree( root );
            m_historyTrees.pop();
        }

        ProfilerNode* newRoot = m_historyTrees.front();
        newRoot->prevSibling = nullptr;
    }
}


// PRIVATE ---------------------------------------
thread_local ProfilerNode*      ProfilerSystem::t_activeNode    = nullptr;
thread_local int                ProfilerSystem::t_depthCount    = 0;


ProfilerNode* ProfilerSystem::ProfilePush( const char* label ) {
    t_depthCount++;

    if( !m_isRunning ) {
        if( m_isResuming && t_depthCount == 1 ) {
            m_isResuming = false;
            m_isRunning = true;
        } else {
            return nullptr;
        }
    }

    ProfilerNode* newNode = AllocNode();

    newNode->threadID = std::this_thread::get_id();
    newNode->startHPC = GetCurrentHPC();
    memcpy( newNode->label, label, Min( (int)strlen( label ), 32 ) );
    newNode->label[31] = '\0'; // Guarantee null terminated

    if( t_activeNode != nullptr ) {
        t_activeNode->AddChild( newNode );
    }

    t_activeNode = newNode;

    return newNode;
}


void ProfilerSystem::ProfilePop() {
    t_depthCount--;

    if( t_activeNode == nullptr ) {
        // Too many pops called
        return;
    }

    if( !m_isRunning ) {
        return;
    }

    t_activeNode->endHPC = GetCurrentHPC();

    // Check for completed frame
    if( t_activeNode->parent == nullptr ) {
        std::scoped_lock< std::shared_mutex > localLock( m_historyMutex );

        if( !m_historyTrees.empty() ) {
            t_activeNode->prevSibling = m_historyTrees.back();
        }

        m_historyTrees.push( t_activeNode );

        // Check if ready to pause
        if( m_isPausing ) {
            m_isPausing = false;
            m_isRunning = false;
        }
    }

    t_activeNode = t_activeNode->parent;
}


double ProfilerSystem::GetAverageFrameSeconds() {
    std::thread::id thisID = std::this_thread::get_id();
    uint64_t numHPC = 0;
    int numFrames = 0;

    std::scoped_lock< std::shared_mutex > localLock( m_historyMutex );

    if( m_historyTrees.size() == 0 ) {
        return 0.;
    }

    ProfilerNode* node = m_historyTrees.back();

    while( node != nullptr ) {
        if( node->threadID == thisID ) {
            numFrames++;
            numHPC += (node->endHPC - node->startHPC);
        }

        node = node->prevSibling;
    }

    if( numFrames == 0 ) {
        return 0.;
    }

    double durationTotal = ConvertHPCTOSeconds( numHPC );
    double avgFrameTime = durationTotal / (double)numFrames;

    return avgFrameTime;
}


float ProfilerSystem::GetProfileTime( const char* label, std::thread::id threadID /*= std::this_thread::get_id() */ ) {
    ProfilerNode* frameTree = GetHistory( threadID );

    if( frameTree == nullptr ) {
        return 0.f;
    }

    ProfilerNode* scopeNode = frameTree->FindNode( label );

    if( scopeNode == nullptr ) {
        return 0.f;
    }

    uint64_t elapsedHPC = scopeNode->endHPC - scopeNode->startHPC;
    float elapsedSeconds = (float)ConvertHPCTOSeconds( elapsedHPC );

    return elapsedSeconds;
}


bool ProfilerSystem::IsTakingInput() const {
    return m_isRendering;
}


ProfilerSystem::~ProfilerSystem() {
    g_theEventSystem->Unsubscribe( "profilerSetMaxHistory", this, &ProfilerSystem::SetMaxHistorySeconds );
    g_theEventSystem->Unsubscribe( "profilerToggle",        this, &ProfilerSystem::Toggle );
    g_theEventSystem->Unsubscribe( "profilerReport",        this, &ProfilerSystem::GenerateReport );
    g_theEventSystem->Unsubscribe( "profilerPause",         this, &ProfilerSystem::Pause );
    g_theEventSystem->Unsubscribe( "profilerResume",        this, &ProfilerSystem::Resume );
}


ProfilerNode* ProfilerSystem::AllocNode() const {
    ProfilerNode* newNode = m_allocator->Malloc();
    newNode->refCount = 1;

    return newNode;
}


void ProfilerSystem::FreeNode( ProfilerNode* nodeToFree ) const {
    m_allocator->Free( nodeToFree );
}


void ProfilerSystem::ReleaseTree( ProfilerNode* treeToFree ) const {
    unsigned int remaingRefs = ::InterlockedDecrement( &(treeToFree->refCount) );

    if( remaingRefs == 0 ) {
        FreeTree( treeToFree );
    }
}


void ProfilerSystem::FreeTree( ProfilerNode* root ) const {
    // Children
    if( root->firstChild != nullptr ) {
        FreeTree( root->firstChild );
    }

    // Siblings
    if( root->nextSibling != nullptr ) {
        FreeTree( root->nextSibling );
    }

    // Self
    FreeNode( root );
}


std::vector< float > ProfilerSystem::GetFrameTimes() const {
    std::thread::id thisID = std::this_thread::get_id();

    int numFrames = (int)m_historyTrees.size();
    std::vector< float > frameTimes;
    frameTimes.reserve( numFrames );

    std::queue< ProfilerNode* > historyCopy = m_historyTrees;

    while( !historyCopy.empty() ) {
        ProfilerNode* node = historyCopy.front();

        if( node->threadID == thisID ) {
            uint64_t numHPC = node->endHPC - node->startHPC;
            double duration = ConvertHPCTOSeconds( numHPC );
            frameTimes.push_back( (float)duration );
        }

        historyCopy.pop();
    }

    return frameTimes;
}


ProfilerNode* ProfilerSystem::GetFrameTree( int index ) const {
    int numFrames = (int)m_historyTrees.size();

    if( index < 0 || index >= numFrames ) {
        return nullptr;
    }

    std::thread::id thisID = std::this_thread::get_id();
    std::queue< ProfilerNode* > historyCopy = m_historyTrees;

    while( !historyCopy.empty() ) {
        ProfilerNode* node = historyCopy.front();

        if( node->threadID == thisID ) {
            if( index == 0 ) {
                ::InterlockedIncrement( &node->refCount );
                return node;
            }

            index--;
        }

        historyCopy.pop();
    }

    return nullptr;
}


ProfilerNode* ProfilerSystem::GetHistory( std::thread::id threadID, int history /*= 0 */ ) {
    std::scoped_lock< std::shared_mutex > localLock( m_historyMutex );

    if( m_historyTrees.size() == 0 ) {
        return nullptr;
    }

    ProfilerNode* node = m_historyTrees.back();

    while( node != nullptr ) {
        if( node->threadID == threadID ) {
            if( history <= 0 ) {
                break;
            }

            history--;
        }

        node = node->prevSibling;
    }

    if( node != nullptr ) {
        ::InterlockedIncrement( &(node->refCount) );
    }

    return node;
}


void ProfilerSystem::RenderFrameTimes() const {
    std::vector< float > frameTimes = GetFrameTimes();
    int numFrames = (int)frameTimes.size();
    int frameHovered = ImGui::PlotHistogram( "Frame Times", frameTimes.data(), numFrames );

    if( frameHovered >= 0 ) {
        ImGui::SetTooltip( "%d: %s", frameHovered, GetProfileTimeString( frameTimes[frameHovered] ).c_str() );

        if( ImGui::IsItemClicked() ) {
            g_theEventSystem->FireEvent( "profilerPause" );

            EventArgs args;
            ProfilerNode* node = GetFrameTree( frameHovered );
            args.SetValue( "framePtr", node );

            g_theEventSystem->FireEvent( "profilerReport", args );
        }
    }
}


void ProfilerSystem::RenderColumnCheckboxes() const {
    const Strings columnLabels = {
        "Num Calls",
        "Total%",
        "Total Time",
        "Avg Total Time",
        "Max Total Time",
        "Self%",
        "Self Time",
        "Avg Self Time",
        "Max Self Time"
    };

    const std::vector< ProfilerReportColumn > columnFlags = {
        REPORT_COLUMN_NUM_CALLS,
        REPORT_COLUMN_PERCENT_TOTAL,
        REPORT_COLUMN_SECONDS_TOTAL,
        REPORT_COLUMN_AVERAGE_TOTAL_SECONDS,
        REPORT_COLUMN_MAX_SECONDS_TOTAL,
        REPORT_COLUMN_PERCENT_SELF,
        REPORT_COLUMN_SECONDS_SELF,
        REPORT_COLUMN_AVERAGE_SELF_SECONDS,
        REPORT_COLUMN_MAX_SECONDS_SELF
    };

    int numBoxes = (int)columnLabels.size();

    for( int boxIndex = 0; boxIndex < numBoxes; boxIndex++ ) {
        ImGui::SameLine();

        const std::string& label = columnLabels[boxIndex];
        const ProfilerReportColumn& flag = columnFlags[boxIndex];

        ImGui::CheckboxFlags( label.c_str(), (unsigned int*)&m_reportColumnFlags, flag );
    }
}


void ProfilerSystem::RenderColumnHeaders() {
    int numColumns = CountSetBits( m_reportColumnFlags );
    ImGui::Columns( numColumns + 1, "ProfilerReport", true );

    ImGui::Text( "Label" );
    ImGui::NextColumn();

    // Num Calls
    if( (m_reportColumnFlags & REPORT_COLUMN_NUM_CALLS) == REPORT_COLUMN_NUM_CALLS ) {
        RenderColumnHeader( "Num Calls" );
    }

    // Percent Total
    if( (m_reportColumnFlags & REPORT_COLUMN_PERCENT_TOTAL) == REPORT_COLUMN_PERCENT_TOTAL ) {
        RenderColumnHeader( "Total%" );
    }

    // Seconds Total
    if( (m_reportColumnFlags & REPORT_COLUMN_SECONDS_TOTAL) == REPORT_COLUMN_SECONDS_TOTAL ) {
        RenderColumnHeader( "Total Time" );
    }

    // Avg Total
    if( (m_reportColumnFlags & REPORT_COLUMN_AVERAGE_TOTAL_SECONDS) == REPORT_COLUMN_AVERAGE_TOTAL_SECONDS ) {
        RenderColumnHeader( "Avg Total Time" );
    }

    // Max Total
    if( (m_reportColumnFlags & REPORT_COLUMN_MAX_SECONDS_TOTAL) == REPORT_COLUMN_MAX_SECONDS_TOTAL ) {
        RenderColumnHeader( "Max Total Time" );
    }

    // Percent Self
    if( (m_reportColumnFlags & REPORT_COLUMN_PERCENT_SELF) == REPORT_COLUMN_PERCENT_SELF ) {
        RenderColumnHeader( "Self%" );
    }

    // Seconds Self
    if( (m_reportColumnFlags & REPORT_COLUMN_SECONDS_SELF) == REPORT_COLUMN_SECONDS_SELF ) {
        RenderColumnHeader( "Self Time" );
    }

    // Avg Self
    if( (m_reportColumnFlags & REPORT_COLUMN_AVERAGE_SELF_SECONDS) == REPORT_COLUMN_AVERAGE_SELF_SECONDS ) {
        RenderColumnHeader( "Avg Self Time" );
    }

    // Max Self
    if( (m_reportColumnFlags & REPORT_COLUMN_MAX_SECONDS_SELF) == REPORT_COLUMN_MAX_SECONDS_SELF ) {
        RenderColumnHeader( "Max Self Time" );
    }
}


void ProfilerSystem::RenderColumnHeader( const char* label ) {
    ImGui::PushID( label );

    if( ImGui::Button( label ) ) {
        if( StringICmp( m_reportSortBy, label ) ) {
            m_reportOrderDesc = !m_reportOrderDesc;
        } else {
            m_reportSortBy = label;
            m_reportOrderDesc = true;
        }

        m_report->Sort( m_reportSortBy, m_reportOrderDesc );
    }

    ImGui::PopID();
    ImGui::NextColumn();
}


// Event Functions --------------------------------------------------
bool ProfilerSystem::SetMaxHistorySeconds( EventArgs& args ) {
    m_historySeconds = args.GetValue( "seconds", m_historySeconds );
    return true;
}


bool ProfilerSystem::Toggle( EventArgs& args ) {
    UNUSED( args );

    m_isRendering = !m_isRendering;
    g_theDevConsole->SetMode( DEV_CONSOLE_OFF );

    return true;
}


bool ProfilerSystem::GenerateReport( EventArgs& args ) {
    ProfilerNode* frameTree = args.GetValue( "framePtr",    (ProfilerNode*)nullptr );
    int frameIndex          = args.GetValue( "frameIndex",  0 );
    bool viewAsTree         = args.GetValue( "view",        m_reportViewTree );
    std::string sortBy      = args.GetValue( "sortBy",      m_reportSortBy );
    bool orderDesc          = args.GetValue( "order",       m_reportOrderDesc );

    if( frameTree == nullptr ) {
        frameTree = GetHistory( std::this_thread::get_id(), frameIndex );
    }

    ProfilerReportNode* reportRoot = new ProfilerReportNode();
    reportRoot->label = "ROOT";

    if( viewAsTree ) {
        reportRoot->AppendChildTreeView( frameTree );
    } else {
        reportRoot->AppendChildFlatView( frameTree );
    }


    if( m_reportBasis != nullptr ) {
        ReleaseTree( frameTree );
    }

    CLEAR_POINTER( m_report );
    m_report = reportRoot;
    m_reportBasis = frameTree;

    m_reportViewTree = viewAsTree;
    m_reportSortBy = sortBy;
    m_reportOrderDesc = orderDesc;
    m_report->Sort( m_reportSortBy, m_reportOrderDesc );

    return false;
}


bool ProfilerSystem::Pause( EventArgs& args ) {
    UNUSED( args );
    m_isPausing = true;

    return true;
}


bool ProfilerSystem::Resume( EventArgs& args ) {
    UNUSED( args );
    m_isResuming = true;

    return true;
}


//UNITTEST( )
