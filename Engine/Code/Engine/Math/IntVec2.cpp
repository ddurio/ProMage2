#include "Engine/Math/IntVec2.hpp"

#include "Engine/Math/Vec2.hpp"


const IntVec2 IntVec2::ZERO     = IntVec2(  0,  0 );
const IntVec2 IntVec2::ONE      = IntVec2(  1,  1 );
const IntVec2 IntVec2::NEGONE   = IntVec2( -1, -1 );

const IntVec2 IntVec2::LEFT     = IntVec2( -1,  0 );
const IntVec2 IntVec2::RIGHT    = IntVec2(  1,  0 );
const IntVec2 IntVec2::UP       = IntVec2(  0,  1 );
const IntVec2 IntVec2::DOWN     = IntVec2(  0, -1 );


IntVec2::IntVec2( const IntVec2& copyFrom ) {
    x = copyFrom.x;
    y = copyFrom.y;
}


IntVec2::IntVec2( int initialX, int initialY ) {
    x = initialX;
    y = initialY;
}


IntVec2::IntVec2( const char* initialValues ) {
    SetFromText( initialValues );
}


const IntVec2 IntVec2::operator+( const IntVec2& vecToAdd ) const {
    int tempX = x + vecToAdd.x;
    int tempY = y + vecToAdd.y;
    return(IntVec2( tempX, tempY ));
}


const IntVec2 IntVec2::operator-( const IntVec2& vecToSubtract ) const {
    int tempX = x - vecToSubtract.x;
    int tempY = y - vecToSubtract.y;
    return(IntVec2( tempX, tempY ));
}

const IntVec2 IntVec2::operator*( int uniformScale ) const {
    int tempX = x * uniformScale;
    int tempY = y * uniformScale;
    return(IntVec2( tempX, tempY ));
}


const IntVec2 IntVec2::operator/( int inverseScale ) const {
    int tempX = x / inverseScale;
    int tempY = y / inverseScale;
    return(IntVec2( tempX, tempY ));
}


const IntVec2 IntVec2::operator*( const IntVec2& vecToMultiply ) const {
    return IntVec2( vecToMultiply.x * x, vecToMultiply.y * y );
}


const Vec2 IntVec2::operator*( const Vec2& vecToMultiply ) const {
    return Vec2( vecToMultiply.x * x, vecToMultiply.y * y );
}


const Vec2 IntVec2::operator*( float uniforScale ) const {
    float tempX = (float)x * uniforScale;
    float tempY = (float)y * uniforScale;
    return Vec2( tempX, tempY );
}


const Vec2 IntVec2::operator/( float uniformDivisor ) const {
    float tempX = (float)x / uniformDivisor;
    float tempY = (float)y / uniformDivisor;
    return Vec2( tempX, tempY );
}


void IntVec2::operator+=( const IntVec2& vecToAdd ) {
    x += vecToAdd.x;
    y += vecToAdd.y;
}


void IntVec2::operator-=( const IntVec2& vecToSubtract ) {
    x -= vecToSubtract.x;
    y -= vecToSubtract.y;
}


void IntVec2::operator*=( const int uniformScale ) {
    x *= uniformScale;
    y *= uniformScale;
}


void IntVec2::operator/=( const int uniformDivisor ) {
    x /= uniformDivisor;
    y /= uniformDivisor;
}


void IntVec2::operator=( const IntVec2& copyFrom ) {
    x = copyFrom.x;
    y = copyFrom.y;
}


bool IntVec2::operator==( const IntVec2& compare ) const {
    bool sameX = (x == compare.x);
    bool sameY = (y == compare.y);
    return (sameX && sameY);
}


bool IntVec2::operator!=( const IntVec2& compare ) const {
    bool sameX = (x == compare.x);
    bool sameY = (y == compare.y);
    return !(sameX && sameY);
}


const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale ) {
    int tempX = vecToScale.x * uniformScale;
    int tempY = vecToScale.y * uniformScale;
	return IntVec2( tempX, tempY );
}


const Vec2 operator*( float uniformScale, const IntVec2& vecToScale ) {
    float tempX = uniformScale * (float)vecToScale.x;
    float tempY = uniformScale * (float)vecToScale.y;
    return Vec2( tempX, tempY );
}


const std::string IntVec2::GetAsString() const {
    return Stringf( "%d,%d", x, y );
}


const IntVec2 IntVec2::GetRotated90Degrees() const {
    return IntVec2( -y, x );
}


const IntVec2 IntVec2::GetRotatedMinus90Degrees() const {
    return IntVec2( y, -x );
}


void IntVec2::SetFromText( const char* valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ',' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 2, Stringf( "Invalid initialization string (%s) for IntVec2", valuesAsText ) );

    x = atoi( splitString[0].c_str() );
    y = atoi( splitString[1].c_str() );
}


void IntVec2::Rotate90Degrees() {
    IntVec2 tempIntVec = GetRotated90Degrees();
    x = tempIntVec.x;
    y = tempIntVec.y;
}


void IntVec2::RotateMinus90Degrees() {
    IntVec2 tempIntVec = GetRotatedMinus90Degrees();
    x = tempIntVec.x;
    y = tempIntVec.y;
}

