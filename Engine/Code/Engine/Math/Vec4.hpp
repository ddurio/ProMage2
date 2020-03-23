#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct Vec4 {
    public:
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;

    Vec4() {};
    explicit Vec4( float inX, float inY, float inZ, float inW );
    explicit Vec4( const std::string& initialValues );

    static const Vec4 ZERO;
    static const Vec4 ONE;

    // Operators
	const Vec4 operator+( const Vec4& vecToAdd ) const;			// Vec4 + Vec4
	const Vec4 operator-( const Vec4& vecToSubtract ) const;	// Vec4 - Vec4
    const Vec4 operator-() const;                               // -Vec4 (negation)
	const Vec4 operator*( float uniformScale ) const;			// Vec4 * float
	const Vec4 operator/( float inverseScale ) const;			// Vec4 / float
	void operator+=( const Vec4& vecToAdd );					// Vec4 += Vec4
	void operator-=( const Vec4& vecToSubtract );				// Vec4 -= Vec4
	void operator*=( const float uniformScale );				// Vec4 *= float
	void operator/=( const float uniformDivisor );				// Vec4 /= float
	void operator=( const Vec4& copyFrom );						// Vec4 = Vec4
	bool operator==( const Vec4& compare ) const;				// Vec4 == Vec4
	bool operator!=( const Vec4& compare ) const;				// Vec4 != Vec4

	friend const Vec4 operator*( float uniformScale, const Vec4& vecToScale );	// float * Vec4

    const std::string GetAsString() const;
    void SetFromText( const std::string& valuesAsText );
};
