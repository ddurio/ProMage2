#include "Engine/Core/Time.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Engine/Math/MathUtils.hpp"


double g_secondsPerHPC;


double InitializeTime( LARGE_INTEGER& out_initialTime ) {
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}


uint64_t ConvertSecondsToHPC( double seconds ) {
    return RoundToInt( seconds / g_secondsPerHPC );
}


double ConvertHPCTOSeconds( uint64_t numHPC ) {
    return (double)numHPC * g_secondsPerHPC;
}


double GetCurrentTimeSeconds() {
	static LARGE_INTEGER initialTime;
    static double secondsPerHPC = InitializeTime( initialTime );
    g_secondsPerHPC = secondsPerHPC;

    uint64_t currentHPC = GetCurrentHPC();
	LONGLONG elapsedHPCSinceInitialTime = ((LARGE_INTEGER*)&currentHPC)->QuadPart - initialTime.QuadPart;

	//double currentSeconds = static_cast< double >( elapsedHPCSinceInitialTime ) * g_secondsPerHPC;
    double currentSeconds = ConvertHPCTOSeconds( elapsedHPCSinceInitialTime );
	return currentSeconds;
}


double GetCurrentSystemTimeSeconds() {
    uint64_t currentHPC = GetCurrentHPC();
    return ConvertHPCTOSeconds( currentHPC );
}


uint64_t GetCurrentHPC() {
    uint64_t hpc;
    QueryPerformanceCounter( (LARGE_INTEGER*)&hpc );

    return hpc;
}


uint64_t GetOffsetHPC( double offsetSeconds ) {
    uint64_t currentHPC = GetCurrentHPC();
    uint64_t numOffsetHPC = ConvertSecondsToHPC( offsetSeconds );

    uint64_t offsetHPC = currentHPC + numOffsetHPC;
    return offsetHPC;
}


std::string GetProfileTimeString( uint64_t numHPC ) {
    double durationSeconds = numHPC * g_secondsPerHPC;
    return GetProfileTimeString( durationSeconds );
}


std::string GetProfileTimeString( double durationSeconds ) {
    if( durationSeconds < 0.001 ) { // Less than ms.. display as micro sec
        double durationMicroSeconds = durationSeconds * 1000000;
        std::string microSecondsStr = Stringf( "%0.3fus", durationMicroSeconds );

        return microSecondsStr;
    } else if( durationSeconds < 1 ) { // Less than seconds.. display as milli sec
        double durationMilliSeconds = durationSeconds * 1000;
        std::string milliSecondsStr = Stringf( "%0.3fms", durationMilliSeconds );

        return milliSecondsStr;
    } else { // Just display as seconds
        std::string secondsStr = Stringf( "%0.3fsec", durationSeconds );

        return secondsStr;
    }
}


int GetIndexOverTime( int numIndices, float indexDurationSeconds ) {
    return (int)(fmod( GetCurrentTimeSeconds() / indexDurationSeconds, numIndices ));
}


Clock Clock::s_master = Clock( nullptr );


Clock::Clock() {
    SetParent( &s_master );
}


Clock::Clock( Clock* parent ) {
    SetParent( parent );
}


Clock::~Clock() {
    // Re-parent children
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        Clock* child = m_children[childIndex];

        if( child != nullptr ) {
            m_children[childIndex] = nullptr;
            child->SetParent( m_parent ); // Could be nullptr
        }
    }

    if( m_parent != nullptr ) {
        m_parent->RemoveChild( this );
    }
}


float Clock::GetDeltaTime() const {
    return m_deltaSeconds;
}


double Clock::GetTotalTime() const {
    return m_totalSeconds;
}


int Clock::GetFrameNumber() const {
    return m_frameCount;
}


void Clock::SetParent( Clock* parent ) {
    if( m_parent != nullptr ) { // Already had a parent, remove me as their child
        m_parent->RemoveChild( this );
        m_parent = nullptr;
    }

    m_parent = parent;

    if( m_parent != nullptr ) {
        m_parent->AddChild( this );
    } else {
        // Assume root clocks want to start at system time
        m_totalSeconds = GetCurrentTimeSeconds();
    }
}


void Clock::SetFrameLimit( float limit ) {
    m_frameLimit = limit;
}


void Clock::SetTimeDilation( float dilation ) {
    m_dilation = dilation;
}


void Clock::SpeedUp( float percentMore ) {
    m_dilation += percentMore;
}


void Clock::SlowDown( float percentLess ) {
    m_dilation -= percentLess;
}


void Clock::Pause() {
    m_pauseCount++;
}


void Clock::Unpause() {
    m_pauseCount = Max( --m_pauseCount, 0 );
}


void Clock::Tick() {
    double currentTime = GetCurrentTimeSeconds();
    float deltaSeconds = (float)(currentTime - m_lastFrameSeconds);
    m_lastFrameSeconds = currentTime;
    Tick( deltaSeconds );
}


void Clock::Tick( float deltaSeconds ) {
    deltaSeconds *= m_dilation;

    if( IsPaused() ) {
        deltaSeconds = 0.f;
    }

    if( deltaSeconds > m_frameLimit ) {
        deltaSeconds = deltaSeconds;
    }

    deltaSeconds = Clamp( deltaSeconds, -m_frameLimit, m_frameLimit );

    if( !std::isnan(m_deltaSecondsForced) ) {
        deltaSeconds = m_deltaSecondsForced;
        m_deltaSecondsForced = std::numeric_limits<float>::quiet_NaN();
    }

    m_frameCount++;
    m_totalSeconds += deltaSeconds;
    m_deltaSeconds = deltaSeconds;

    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        Clock* child = m_children[childIndex];

        if( child != nullptr ) {
            child->Tick( m_deltaSeconds );
        }
    }
}


void Clock::ForceTick( float deltaSeconds ) {
    m_deltaSecondsForced = deltaSeconds;
}


bool Clock::IsPaused() const {
    return (m_pauseCount > 0);
}


void Clock::AddChild( Clock* childToAdd ) {
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        Clock* child = m_children[childIndex];

        if( child == nullptr ) {
            m_children[childIndex] = childToAdd;
            return;
        }
    }

    m_children.push_back( childToAdd );
}


void Clock::RemoveChild( Clock* childToRemove ) {
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        Clock* child = m_children[childIndex];

        if( child == childToRemove ) {
            m_children[childIndex] = nullptr;
            return;
        }
    }
}
