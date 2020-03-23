#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"


AnalogJoystick::AnalogJoystick( float innerDeadzone, float outerDeadzone)
    : m_innerDeadzoneFraction( innerDeadzone ),
      m_outerDeadzoneFraction( outerDeadzone ) {

}


AnalogJoystick::~AnalogJoystick() {

}


const Vec2 AnalogJoystick::GetRawPosition() const {
    return m_rawPosition;
}


const Vec2 AnalogJoystick::GetPosition() const {
    return m_correctedPosition;
}


const float AnalogJoystick::GetAngleDegrees() const {
    return m_correctedAngleDegrees;
}


const float AnalogJoystick::GetMagnitude() const {
    return m_correctedMagnitude;
}


float AnalogJoystick::GetInnerDeadzoneFraction() const {
    return m_innerDeadzoneFraction;
}


float AnalogJoystick::GetOuterDeadzoneFraction() const {
    return m_outerDeadzoneFraction;
}


void AnalogJoystick::UpdateInput( Vec2 rawIn ) {
    m_rawPosition = rawIn;

    Vec2 tempXY = m_rawPosition;
    tempXY.x = RangeMap( m_rawPosition.x, -32768, 32767, -1.f, 1.f );
    tempXY.y = RangeMap( m_rawPosition.y, -32768, 32767, -1.f, 1.f );

    if( tempXY.GetLength() < m_innerDeadzoneFraction ) {
        tempXY = Vec2( 0.f, 0.f );
    } else if( tempXY.GetLength() > m_outerDeadzoneFraction ) {
        tempXY.Normalize();
    } else {
        float correctedLength = RangeMap( tempXY.GetLength(), m_innerDeadzoneFraction, m_outerDeadzoneFraction, 0.f, 1.f );
        tempXY.SetLength( correctedLength );
    }

    m_correctedPosition = tempXY;
    m_correctedAngleDegrees = m_correctedPosition.GetAngleDegrees();
    m_correctedMagnitude = m_correctedPosition.GetLength();
}


void AnalogJoystick::Reset() {
    m_rawPosition = Vec2( 0.f, 0.f );
    m_correctedPosition = Vec2( 0.f, 0.f );
}
