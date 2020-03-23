#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct IntRange;


struct FloatRange {
    public:
    float min = 0.f;
    float max = 0.f;

    explicit FloatRange() {};
    explicit FloatRange( float initialMinAndMax );
    explicit FloatRange( float initialMin, float initialMax );
    explicit FloatRange( const IntRange& initialRange );
    explicit FloatRange( const std::string& initialRange );
    ~FloatRange() {};

    static const FloatRange ZERO;
    static const FloatRange ZEROTOONE;
    static const FloatRange ONE;
    static const FloatRange NEGONE;

    const std::string GetAsString() const;
    void SetFromText( const std::string& valuesAsText );

    // Operators
    const FloatRange operator+( const FloatRange& rangeToAdd ) const;		// FloatRange +  FloatRange
    const FloatRange operator-( const FloatRange& rangeToSubtract ) const;	// FloatRange -  FloatRange
    const FloatRange operator*( float uniformScale ) const;			        // FloatRange *  int
    const FloatRange operator/( float inverseScale ) const;			        // FloatRange /  int
    void operator*=( const float uniformScale );		                    // FloatRange *= float
    void operator/=( const float uniformDivisor );		                    // FloatRange /= float
    void operator=(  const FloatRange& copyFrom );		                    // FloatRange =  FloatRange
    bool operator==( const FloatRange& compare ) const;	                    // FloatRange == FloatRange
    bool operator!=( const FloatRange& compare ) const;	                    // FloatRange != FloatRange

    friend const FloatRange operator*( float uniformScale, const FloatRange& rangeToScale );	// float * range2
};
