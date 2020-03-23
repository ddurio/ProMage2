#include "Engine/Math/IntRange.hpp"


const IntRange IntRange::ZERO   = IntRange( 0, 0 );
const IntRange IntRange::ONE    = IntRange( 1, 1 );
const IntRange IntRange::NEGONE = IntRange( -1, -1 );


IntRange::IntRange( int initialMin, int initialMax ) :
    min(initialMin),
    max(initialMax) {
}


IntRange::IntRange( const std::string& initialRange ) {
    SetFromText( initialRange );
}


bool IntRange::operator==( const IntRange& compare ) const {
    bool minEqual = (min == compare.min);
    bool maxEqual = (max == compare.max);

    return (minEqual && maxEqual);
}


bool IntRange::operator!=( const IntRange& compare ) const {
    bool minEqual = (min == compare.min);
    bool maxEqual = (max == compare.max);

    return (!minEqual || !maxEqual);
}

const std::string IntRange::GetAsString() const {
    if( min == max ) {
        return Stringf( "%i", min );
    } else {
        return Stringf( "%i~%i", min, max );
    }
}


void IntRange::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, '~' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 1 || numValues == 2, Stringf( "Invalid initialization string (%s) for IntRange", valuesAsText.c_str() ) );

    min = atoi( splitString[0].c_str() );

    if( numValues == 1 ) {
        max = min;
    } else {
        max = atoi( splitString[1].c_str() );
    }
}


bool IntRange::IsIntInRange( int inValue ) const {
    return (min <= inValue) && (inValue <= max);
}

