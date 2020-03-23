#include "Engine/Math/IntVec3.hpp"

#include "Engine/Math/Vec3.hpp"


const IntVec3 IntVec3::ZERO = IntVec3( 0, 0, 0 );
const IntVec3 IntVec3::ONE = IntVec3( 1, 1, 1 );
const IntVec3 IntVec3::NEGONE = IntVec3( -1, -1, -1 );

IntVec3::IntVec3( const IntVec3& copyFrom ) {
    x = copyFrom.x;
    y = copyFrom.y;
    z = copyFrom.z;
}


IntVec3::IntVec3( int initialX, int initialY, int initialZ ) {
    x = initialX;
    y = initialY;
    z = initialZ;
}


IntVec3::IntVec3( const std::string& initialValues ) {
    SetFromText( initialValues );
}


const IntVec3 IntVec3::operator+( const IntVec3& vecToAdd ) const {
    int tempX = x + vecToAdd.x;
    int tempY = y + vecToAdd.y;
    int tempZ = z + vecToAdd.z;
    return(IntVec3( tempX, tempY, tempZ ));
}


const IntVec3 IntVec3::operator-( const IntVec3& vecToSubtract ) const {
    int tempX = x - vecToSubtract.x;
    int tempY = y - vecToSubtract.y;
    int tempZ = z - vecToSubtract.z;
    return(IntVec3( tempX, tempY, tempZ ));
}

const IntVec3 IntVec3::operator*( int uniformScale ) const {
    int tempX = x * uniformScale;
    int tempY = y * uniformScale;
    int tempZ = z * uniformScale;
    return(IntVec3( tempX, tempY, tempZ ));
}


const IntVec3 IntVec3::operator*( IntVec3 vecToMultiply ) const {
    return IntVec3( vecToMultiply.x * x, vecToMultiply.y * y, vecToMultiply.z * z );
}


const Vec3 IntVec3::operator*( const Vec3& vecToMultiply ) const {
    return Vec3( vecToMultiply.x * x, vecToMultiply.y * y, vecToMultiply.z * z );
}


const std::string IntVec3::GetAsString() const {
    return Stringf( "%d,%d,%d", x, y, z );
}


const IntVec3 IntVec3::operator/( int inverseScale ) const {
    int tempX = x / inverseScale;
    int tempY = y / inverseScale;
    int tempZ = z / inverseScale;
    return(IntVec3( tempX, tempY, tempZ ));
}


const Vec3 IntVec3::operator/( float uniformDivisor ) const {
    float tempX = (float)x / uniformDivisor;
    float tempY = (float)y / uniformDivisor;
    float tempZ = (float)z / uniformDivisor;
    return Vec3( tempX, tempY, tempZ );
}


void IntVec3::operator+=( const IntVec3& vecToAdd ) {
    x += vecToAdd.x;
    y += vecToAdd.y;
    z += vecToAdd.z;
}


void IntVec3::operator-=( const IntVec3& vecToSubtract ) {
    x -= vecToSubtract.x;
    y -= vecToSubtract.y;
    z -= vecToSubtract.z;
}


void IntVec3::operator*=( const int uniformScale ) {
    x *= uniformScale;
    y *= uniformScale;
    z *= uniformScale;
}


void IntVec3::operator/=( const int uniformDivisor ) {
    x /= uniformDivisor;
    y /= uniformDivisor;
    z /= uniformDivisor;
}


void IntVec3::operator=( const IntVec3& copyFrom ) {
    x = copyFrom.x;
    y = copyFrom.y;
    z = copyFrom.z;
}


bool IntVec3::operator==( const IntVec3& compare ) const {
    bool sameX = (x == compare.x);
    bool sameY = (y == compare.y);
    bool sameZ = (z == compare.z);
    return (sameX && sameY && sameZ);
}


bool IntVec3::operator!=( const IntVec3& compare ) const {
    bool sameX = (x == compare.x);
    bool sameY = (y == compare.y);
    bool sameZ = (z == compare.z);
    return !(sameX && sameY && sameZ);
}


const IntVec3 operator*( int uniformScale, const IntVec3& vecToScale ) {
    int tempX = vecToScale.x * uniformScale;
    int tempY = vecToScale.y * uniformScale;
    int tempZ = vecToScale.z * uniformScale;
	return IntVec3( tempX, tempY, tempZ );
}


void IntVec3::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ',' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 3, Stringf( "Invalid initialization string (%s) for IntVec3", valuesAsText.c_str() ) );

    x = atoi( splitString[0].c_str() );
    y = atoi( splitString[1].c_str() );
    z = atoi( splitString[2].c_str() );
}
