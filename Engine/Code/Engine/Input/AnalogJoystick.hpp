#pragma once
#include "Engine/Math/Vec2.hpp"


class AnalogJoystick {
    public:
    explicit AnalogJoystick( float innerDeadzone, float outerDeadzone );
    ~AnalogJoystick();
    const Vec2 GetRawPosition() const;
    const Vec2 GetPosition() const;
    const float GetAngleDegrees() const;
    const float GetMagnitude() const;

    float GetInnerDeadzoneFraction() const;
    float GetOuterDeadzoneFraction() const;

    void UpdateInput( Vec2 rawIn );
    void Reset();

    private:
    Vec2 m_rawPosition = Vec2( 0.f, 0.f );
    Vec2 m_correctedPosition = Vec2( 0.f, 0.f );
    float m_correctedAngleDegrees = 0.f;
    float m_correctedMagnitude = 0.f;
    const float m_innerDeadzoneFraction = 0.f;
    const float m_outerDeadzoneFraction = 1.f;
};
