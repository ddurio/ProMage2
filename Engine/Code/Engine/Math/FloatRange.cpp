#include "Engine/Math/FloatRange.hpp"

#include "Engine/Math/IntRange.hpp"


const FloatRange FloatRange::ZERO      = FloatRange(  0.f,  0.f );
const FloatRange FloatRange::ZEROTOONE = FloatRange(  0.f,  1.f );
const FloatRange FloatRange::ONE       = FloatRange(  1.f,  1.f );
const FloatRange FloatRange::NEGONE    = FloatRange( -1.f, -1.f );


FloatRange::FloatRange( float initialMinAndMax ) : 
    min(initialMinAndMax),
    max(initialMinAndMax) {
}


FloatRange::FloatRange( float initialMin, float initialMax ) :
    min(initialMin),
    max(initialMax) {
}


FloatRange::FloatRange( const IntRange& initialRange ) :
    min( (float)initialRange.min ),
    max( (float)initialRange.max ) {
}


FloatRange::FloatRange( const std::string& initialRange ) {
    SetFromText( initialRange );
}


const std::string FloatRange::GetAsString() const {
    if( min == max ) {
        return Stringf( "%f", min );
    } else {
        return Stringf( "%f~%f", min, max );
    }
}


void FloatRange::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, '~' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 1 || numValues == 2, Stringf( "(FloatRange) Invalid initialization string (%s)", valuesAsText.c_str() ) );

    min = (float)atof( splitString[0].c_str() );

    if( numValues == 1) {
        max = min;
    } else {
        max = (float)atof( splitString[1].c_str() );
    }
}


const FloatRange FloatRange::operator+( const FloatRange& rangeToAdd ) const {
    float tempMin = min + rangeToAdd.min;
    float tempMax = max + rangeToAdd.max;
    return FloatRange( tempMin, tempMax );
}


const FloatRange FloatRange::operator-( const FloatRange& rangeToSubtract ) const {
    float tempMin = min - rangeToSubtract.min;
    float tempMax = max - rangeToSubtract.max;
    return FloatRange( tempMin, tempMax );
}


const FloatRange FloatRange::operator*( float uniformScale ) const {
    float tempMin = min * uniformScale;
    float tempMax = max * uniformScale;
    return FloatRange( tempMin, tempMax );
}


const FloatRange FloatRange::operator/( float inverseScale ) const {
    float tempMin = min / inverseScale;
    float tempMax = max / inverseScale;
    return FloatRange( tempMin, tempMax );
}



void FloatRange::operator*=( const float uniformScale ) {
    min *= uniformScale;
    max *= uniformScale;
}


void FloatRange::operator/=( const float uniformDivisor ) {
    min /= uniformDivisor;
    max /= uniformDivisor;
}


void FloatRange::operator=( const FloatRange& copyFrom ) {
    min = copyFrom.min;
    max = copyFrom.max;
}


bool FloatRange::operator==( const FloatRange& compare ) const {
    return (min == compare.min && max == compare.max);
}


bool FloatRange::operator!=( const FloatRange& compare ) const {
    return (min != compare.min || max != compare.max);
}


const FloatRange operator*( float uniformScale, const FloatRange& rangeToScale ) {
    return rangeToScale * uniformScale;
}
