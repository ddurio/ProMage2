#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <cmath>


struct Vec2;

//-----------------------------------------------------------------------------------------------
struct IntVec2 {
    public:
    int x = 0;
    int y = 0;

    // Construction/Destruction
    IntVec2() {}												// default constructor: do nothing (for speed)
    IntVec2( const IntVec2& copyFrom );							// copy constructor (from another vec2)
    explicit IntVec2( int initialX, int initialY );		// explicit constructor (from x, y)
    explicit IntVec2( const char* initialValues );
    ~IntVec2() {}												// destructor: do nothing (for speed)

    static const IntVec2 ZERO;
    static const IntVec2 ONE;
    static const IntVec2 NEGONE;

    static const IntVec2 LEFT;
    static const IntVec2 RIGHT;
    static const IntVec2 UP;
    static const IntVec2 DOWN;

    // Operators
    const IntVec2 operator+( const IntVec2& vecToAdd ) const;		// IntVec2 +  IntVec2
    const IntVec2 operator-( const IntVec2& vecToSubtract ) const;	// IntVec2 -  IntVec2
    const IntVec2 operator*( int uniformScale ) const;			    // IntVec2 *  int
    const IntVec2 operator/( int inverseScale ) const;			    // IntVec2 /  int
    const IntVec2 operator*( const IntVec2& vecToMultiply ) const;  // IntVec2 *  IntVec2
    const Vec2 operator*( const Vec2& vecToMultiply ) const;        // IntVec2 *  Vec2
    const Vec2 operator*( float uniforScale ) const;                // IntVec2 *  float
    const Vec2 operator/( float uniformDivisor ) const;             // IntVec2 /  float
    void operator+=( const IntVec2& vecToAdd );					    // IntVec2 += IntVec2
    void operator-=( const IntVec2& vecToSubtract );				// IntVec2 -= IntVec2
    void operator*=( const int uniformScale );				        // IntVec2 *= int
    void operator/=( const int uniformDivisor );				    // IntVec2 /= int
    void operator=( const IntVec2& copyFrom );						// IntVec2 =  IntVec2
    bool operator==( const IntVec2& compare ) const;				// IntVec2 == IntVec2
    bool operator!=( const IntVec2& compare ) const;				// IntVec2 != IntVec2

    friend const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale );	// int * IntVec2
    friend const Vec2 operator*( float uniformScale, const IntVec2& vecToScale );   // float * IntVec2

    const std::string GetAsString() const;
    const IntVec2 GetRotated90Degrees() const;
    const IntVec2 GetRotatedMinus90Degrees() const;

    void SetFromText( const char* valuesAsText );
    void Rotate90Degrees();
    void RotateMinus90Degrees();
};
