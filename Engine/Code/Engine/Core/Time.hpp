#pragma once
#include "Engine/Core/EngineCommon.hpp"


uint64_t ConvertSecondsToHPC( double seconds );
double ConvertHPCTOSeconds( uint64_t hpc );
double GetCurrentTimeSeconds();
double GetCurrentSystemTimeSeconds();
uint64_t GetCurrentHPC();
uint64_t GetOffsetHPC( double offsetSeconds );
std::string GetProfileTimeString( uint64_t numHPC );
std::string GetProfileTimeString( double durationSeconds );
int GetIndexOverTime( int numIndices, float indexDurationSeconds );


class Clock {
    public:
    Clock();
    Clock( Clock* parent );
    ~Clock();

    float GetDeltaTime() const;
    double GetTotalTime() const;
    int GetFrameNumber() const;

    void SetParent( Clock* parent );
    void SetFrameLimit( float limit );
    void SetTimeDilation( float dilation );
    void SpeedUp( float percentMore );
    void SlowDown( float percentLess );

    void Pause();
    void Unpause();

    void Tick();
    void Tick( float deltaSeconds );
    void ForceTick( float deltaSeconds );

    bool IsPaused() const;

    static Clock s_master;

    private:
    // State
    Clock* m_parent = nullptr;
    std::vector<Clock*> m_children;

    double m_totalSeconds = 0.f;
    double m_lastFrameSeconds = 0.f; // Only used by root clocks
    float m_deltaSeconds = 0.f;
    float m_deltaSecondsForced = std::numeric_limits<float>::quiet_NaN();
    int m_frameCount = 0;
    int m_pauseCount = 0;

    // Configuration
    float m_frameLimit = std::numeric_limits<float>::infinity();
    float m_dilation = 1.f;

    void AddChild( Clock* childToAdd );
    void RemoveChild( Clock* childToRemove );
};
