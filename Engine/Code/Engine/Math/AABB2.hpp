#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"


struct AABB2 {
    public:
    Vec2 mins;
    Vec2 maxs;

	~AABB2() {}												// destructor: do nothing (for speed)
	AABB2() {}												// default constructor: do nothing (for speed)
	AABB2( const AABB2& copyFrom );							// copy constructor (from another aabb2)
	explicit AABB2( Vec2 initialMins, Vec2 initialMaxs );	// explicit constructor (from mins, maxs)
    explicit AABB2( float minX, float minY, float maxX, float maxY );
    explicit AABB2( const std::string& valuesAsText );

    static const AABB2 ZEROTOONE;
    static const AABB2 UVZEROTOONE;

	// Operators
	const AABB2 operator+( const AABB2& aabb2ToAdd ) const;		// aabb2 + aabb2
    const AABB2 operator-( const AABB2& aabb2ToSubtract ) const;// aabb2 - aabb2
	const AABB2 operator*( float uniformScale ) const;			// aabb2 * float
	const AABB2 operator/( float inverseScale ) const;			// aabb2 / float
    const AABB2 operator+( const Vec2& vecToAdd ) const;        // aabb2 + Vec2
    const AABB2 operator-( const Vec2& vecToSubtract ) const;   // aabb2 - Vec2
	void operator+=( const AABB2& aabb2ToAdd );					// aabb2 += aabb2
	void operator-=( const AABB2& aabb2ToSubtract );			// aabb2 -= aabb2
    void operator+=( const Vec2& vecToAdd );                     // aabb2 += Vec2
    void operator-=( const Vec2& vecToSubtract );                // aabb2 -= Vec2
	void operator*=( const float uniformScale );				// aabb2 *= float
	void operator/=( const float uniformDivisor );				// aabb2 /= float
	void operator=( const AABB2& copyFrom );					// aabb2 = aabb2
	bool operator==( const AABB2& compare ) const;				// aabb2 == aabb2
	bool operator!=( const AABB2& compare ) const;				// aabb2 != aabb2

	friend const AABB2 operator*( float uniformScale, const AABB2& aabb2ToScale ); // float * aabb2
    friend const AABB2 operator+( const Vec2& vecToAdd, const AABB2& aabb2ToAdd ); // vec2 + aabb2

    bool IsPointInside( const Vec2& point ) const;
    std::string GetAsString() const;
    const Vec2 GetClosestPointOnAABB2( const Vec2& point ) const;
    const Vec2 GetClosestPointOnAABB2Edge( const Vec2& point ) const;

    const Vec2 GetDimensions() const;
    const Vec2 GetCenter() const;
    float GetAspectRatio() const;

    const AABB2 GetPaddedAABB2( float padding ) const;
    const AABB2 GetTranslated( const Vec2& translation ) const;

    void GrowToIncludePoint( const Vec2& pointToInclude );
    //void Resize( const Vec2& newDimensions, const Vec2& pivotAnchorAlignment = ALIGN_CENTER );
    void Translate( const Vec2& translation );

    void AlignWithinBox( const AABB2& box, const Vec2& alignment );
    AABB2 GetBoxWithin( const Vec2& boxDimensions, const Vec2& alignment ) const;
    Vec2 GetPointWithin( const Vec2& alignment ) const;

    Vec2 GetPointAlignment( const Vec2& point ) const;

    /*
    AABB2 GetBoxAtLeft( float fractionOfWidth, float additionalWidth = 0.f ) const;
    AABB2 GetBoxAtRight( float fractionOfWidth, float additionalWidth = 0.f ) const;
    AABB2 GetBoxAtBottom( float fractionOfHeight, float additionalHeight = 0.f ) const;
    AABB2 GetBoxAtTop( float fractionOfHeight, float additionalHeight = 0.f ) const;

    AABB2 CarveBoxOffLeft( float fractionOfWidth, float additionalWidth = 0.f );
    AABB2 CarveBoxOffRight( float fractionOfWidth, float additionalWidth = 0.f );
    */
    AABB2 CarveBoxOffBottom( float fractionOfHeight, float additionalHeight = 0.f );
    AABB2 CarveBoxOffTop( float fractionOfHeight, float additionalHeight = 0.f );

    /*
    Vec2 GetUVForPoint( const Vec2& point ) const;
    Vec2 GetPointForUV( const Vec2& normalizedLocalPosition ) const;
    */

    void SetFromText( const std::string& valuesAsText );
};