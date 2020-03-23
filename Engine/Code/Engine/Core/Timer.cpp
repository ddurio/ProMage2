#include "Engine/Core/Timer.hpp"

#include "Engine/Core/Time.hpp"


Timer::Timer() :
    Timer( &Clock::s_master ) {
}


Timer::Timer( const Clock* clock ) :
    m_clock( clock ) {
}


void Timer::Start( float duration ) {
    SetDuration( duration );
    Restart();
}


void Timer::SetDuration( float newDuration ) {
    m_duration = newDuration;
}


void Timer::Restart() {
    m_startTime = m_clock->GetTotalTime();
}


float Timer::GetDuration() const {
    return m_duration;
}


float Timer::GetElapsedTime() const {
    double currentTime = m_clock->GetTotalTime();
    float elapsedTime = (float)(currentTime - m_startTime);

    return elapsedTime;
}


float Timer::GetRemainingTime() const {
    float elapsedTime = GetElapsedTime();
    float remainingTime = (m_duration - elapsedTime);

    return remainingTime;
}


float Timer::GetNormalizedElapsedTime() const {
    if( m_duration == 0.f ) {
        return 0.f;
    }

    float elapsedTime = GetElapsedTime();
    float normElapsedTime = (elapsedTime / m_duration);

    return normElapsedTime;
}


bool Timer::HasFinished() const {
    if( this == nullptr ) {
        return false;
    }

    float normElapsedTime = GetNormalizedElapsedTime();
    bool hasFinished = (normElapsedTime > 1.f);

    return hasFinished;
}


int Timer::GetNumFinished() const {
    float normElapsedTime = GetNormalizedElapsedTime();
    int numFinished = (int)normElapsedTime; // Truncates to int

    return numFinished;
}


bool Timer::Decrement( int numDecrement /*= 1 */ ) {
    if( HasFinished() ) {
        m_startTime += m_duration * (float)numDecrement;
        return true;
    }

    return false;
}


int Timer::DecrementAll() {
    if( m_duration <= 0.f ) {
        return 0;
    }

    int numFinished = 0;

    while( HasFinished() ) {
        m_startTime += m_duration;
        numFinished++;
    }

    return numFinished;
}
