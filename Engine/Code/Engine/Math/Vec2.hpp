#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <cmath>


struct ImVec2;
struct IntVec2;

//-----------------------------------------------------------------------------------------------
struct Vec2 {
	public:
	float x = 0.f;
	float y = 0.f;

	// Construction/Destruction
	Vec2() {}												// default constructor: do nothing (for speed)
	Vec2( const Vec2& copyFrom );							// copy constructor (from another vec2)
    explicit Vec2( float initialXY );                       // explicit constructor (from single value)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)
    explicit Vec2( const float* floatArr );
    explicit Vec2( const IntVec2& castFrom );               // explicit constructor (from IntVec2)
    explicit Vec2( const std::string& initialValues );
	~Vec2() {}												// destructor: do nothing (for speed)

    static const Vec2 ZERO;
    static const Vec2 ONE;
    static const Vec2 NEGONE;

    static const Vec2 LEFT;
    static const Vec2 RIGHT;
    static const Vec2 UP;
    static const Vec2 DOWN;

    static const Vec2 MakeFromPolarDegrees( float angleDegrees, float radius = 1.f );
    static const Vec2 MakeFromPolarRadians( float angleRadians, float radius = 1.f );

	// Operators
	const Vec2 operator+( const Vec2& vecToAdd ) const;			// Vec2 +  Vec2
	const Vec2 operator-( const Vec2& vecToSubtract ) const;	// Vec2 -  Vec2
    const Vec2 operator-() const;                               // -Vec2 (negation)
	const Vec2 operator*( float uniformScale ) const;			// Vec2 *  float
    const Vec2 operator*( const Vec2& vecToMultiply ) const;    // Vec2 *  Vec2
    const Vec2 operator*( const IntVec2& vecToMultiply ) const; // Vec2 *  IntVec2
	const Vec2 operator/( float inverseScale ) const;			// Vec2 /  float
    const Vec2 operator/( const Vec2& inverseScale ) const;     // Vec2 / Vec2
    const Vec2 operator/( const IntVec2& inverseScale );        // Vec2 / IntVec2
	void operator+=( const Vec2& vecToAdd );					// Vec2 += Vec2
	void operator-=( const Vec2& vecToSubtract );				// Vec2 -= Vec2
	void operator*=( const float uniformScale );				// Vec2 *= float
	void operator/=( const float uniformDivisor );				// Vec2 /= float
	void operator=( const Vec2& copyFrom );						// Vec2 =  Vec2
	bool operator==( const Vec2& compare ) const;				// Vec2 == Vec2
	bool operator!=( const Vec2& compare ) const;				// Vec2 != Vec2

	friend const Vec2 operator*( float uniformScale, const Vec2& vecToScale );	// float * Vec2

    // ImGui Operators
    Vec2( const ImVec2& copyFrom );
    const Vec2 operator-( const ImVec2& imVectoSub ) const;
    void operator-=( const ImVec2& imVecToSub );

    friend const Vec2 operator-( const ImVec2& imVec, const Vec2& vecToSub );   // ImVec2 - Vec2

    // Methods
    const std::string GetAsString() const;
    ImVec2 GetAsImGui() const;

	float GetLength() const;
    float GetLengthSquared() const;
    float GetAngleDegrees() const;
    float GetAngleRadians() const;
    const Vec2 GetRotated90Degrees() const;
    const Vec2 GetRotatedMinus90Degrees() const;
    const Vec2 GetRotatedDegrees( float angleDegrees ) const;
    const Vec2 GetRotatedRadians( float angleRadians ) const;
    const Vec2 GetClamped( float maxLength ) const;
    const Vec2 GetNormalized() const;

    void SetFromText( const std::string& valuesAsText );
    void ClampLength( float maxLength );
    void SetLength( float newLength );
    void SetAngleDegrees( float newAngleDegrees );
    void SetAngleRadians( float newAngleRadians );
    void SetPolarDegrees( float newAngleDegrees, float newLength );
    void SetPolarRadians( float newangleRadians, float newLength );
    void RotateDegrees( float angleDegrees );
    void RotateRadians( float angleRadians );
    void Rotate90Degrees();
    void RotateMinus90Degrees();
    void Normalize();
    float NormalizeGetPreviousLength();
};
