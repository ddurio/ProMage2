#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <cmath>


struct Vec3;

//-----------------------------------------------------------------------------------------------
struct IntVec3 {
    public:
    int x = 0;
    int y = 0;
    int z = 0;

    // Construction/Destruction
    IntVec3() {}												// default constructor: do nothing (for speed)
    IntVec3( const IntVec3& copyFrom );							// copy constructor (from another vec2)
    explicit IntVec3( int initialX, int initialY, int initialZ );		// explicit constructor (from x, y)
    explicit IntVec3( const std::string& initialValues );
    ~IntVec3() {}												// destructor: do nothing (for speed)

    static const IntVec3 ZERO;
    static const IntVec3 ONE;
    static const IntVec3 NEGONE;

    // Operators
    const IntVec3 operator+( const IntVec3& vecToAdd ) const;		// IntVec3 +  IntVec3
    const IntVec3 operator-( const IntVec3& vecToSubtract ) const;	// IntVec3 -  IntVec3
    const IntVec3 operator*( int uniformScale ) const;			    // IntVec3 *  int
    const IntVec3 operator/( int inverseScale ) const;			    // IntVec3 /  int
    const IntVec3 operator*( IntVec3 vecToMultiply ) const;         // IntVec3 *  IntVec3
    const Vec3 operator*( const Vec3& vecToMultiply ) const;        // IntVec3 *  Vec3
    const Vec3 operator/( float uniformDivisor ) const;             // IntVec3 /  float
    void operator+=( const IntVec3& vecToAdd );					    // IntVec3 += IntVec3
    void operator-=( const IntVec3& vecToSubtract );				// IntVec3 -= IntVec3
    void operator*=( const int uniformScale );				        // IntVec3 *= int
    void operator/=( const int uniformDivisor );				    // IntVec3 /= int
    void operator=( const IntVec3& copyFrom );						// IntVec3 =  IntVec3
    bool operator==( const IntVec3& compare ) const;				// IntVec3 == IntVec3
    bool operator!=( const IntVec3& compare ) const;				// IntVec3 != IntVec3

    friend const IntVec3 operator*( int uniformScale, const IntVec3& vecToScale );	// int * vec2

    const std::string GetAsString() const;

    void SetFromText( const std::string& valuesAsText );
};
