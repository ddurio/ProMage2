#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Time.hpp"


class Timer {
    public:
    Timer();
    Timer( const Clock* clock );
    ~Timer() {};

    void Start( float duration );
    void SetDuration( float newDuration );
    void Restart();

    float GetDuration() const;
    float GetElapsedTime() const;
    float GetRemainingTime() const;
    float GetNormalizedElapsedTime() const;

    bool HasFinished() const;
    int GetNumFinished() const;

    bool Decrement( int numDecrement = 1 );
    int DecrementAll();

    private:
    const Clock* m_clock = nullptr;
    double m_startTime = 0;
    float m_duration = 0.f;
};
