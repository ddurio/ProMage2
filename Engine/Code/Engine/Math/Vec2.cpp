#include "Engine/Math/Vec2.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"


const Vec2 Vec2::ZERO   = Vec2(  0.f );
const Vec2 Vec2::ONE    = Vec2(  1.f );
const Vec2 Vec2::NEGONE = Vec2( -1.f );

const Vec2 Vec2::LEFT   = Vec2( -1.f,  0.f );
const Vec2 Vec2::RIGHT  = Vec2(  1.f,  0.f );
const Vec2 Vec2::UP     = Vec2(  0.f,  1.f );
const Vec2 Vec2::DOWN   = Vec2(  0.f, -1.f );


Vec2::Vec2( const Vec2& copy ) :
    x( copy.x ),
    y( copy.y ) {
}


Vec2::Vec2( const ImVec2& copyFrom ) :
    x( copyFrom.x ),
    y( copyFrom.y ) {
}


Vec2::Vec2( float initialXY ) :
    x( initialXY ),
    y( initialXY ) {
}


Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY ) {
}


Vec2::Vec2( const float* floatArr ) :
    x( floatArr[0] ),
    y( floatArr[1] ) {
}


Vec2::Vec2( const IntVec2& castFrom ) :
    x( (float)castFrom.x ),
    y( (float)castFrom.y ) {
}


Vec2::Vec2( const std::string& initialValues ) {
    SetFromText( initialValues );
}


const Vec2 Vec2::MakeFromPolarDegrees( float angleDegrees, float radius /*= 1.f */ ) {
    float tempX = radius * CosDegrees( angleDegrees );
    float tempY = radius * SinDegrees( angleDegrees );
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::MakeFromPolarRadians( float angleRadians, float radius /*= 1.f */ ) {
    float tempX = radius * cos( angleRadians );
    float tempY = radius * sin( angleRadians );
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator+( const Vec2& vecToAdd ) const {
	float tempX = x + vecToAdd.x;
	float tempY = y + vecToAdd.y;
	return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const {
	float tempX = x - vecToSubtract.x;
	float tempY = y - vecToSubtract.y;
	return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator-() const {
    return (Vec2::ZERO - *this);
}


const Vec2 Vec2::operator*( float uniformScale ) const {
	float tempX = x * uniformScale;
	float tempY = y * uniformScale;
	return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const {
    float tempX = x * vecToMultiply.x;
    float tempY = y * vecToMultiply.y;
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator*( const IntVec2& vecToMultiply ) const {
    float tempX = x * (float)vecToMultiply.x;
    float tempY = y * (float)vecToMultiply.y;
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator/( float inverseScale ) const {
	float tempX = x / inverseScale;
	float tempY = y / inverseScale;
	return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator/( const Vec2& inverseScale ) const {
    float tempX = x / inverseScale.x;
    float tempY = y / inverseScale.y;
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::operator/( const IntVec2& inverseScale ) {
    float tempX = x / (float)inverseScale.x;
    float tempY = y / (float)inverseScale.y;
    return Vec2( tempX, tempY );
}


void Vec2::operator+=( const Vec2& vecToAdd ) {
	x += vecToAdd.x;
	y += vecToAdd.y;
}


void Vec2::operator-=( const Vec2& vecToSubtract ) {
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


void Vec2::operator*=( const float uniformScale ) {
	x *= uniformScale;
	y *= uniformScale;
}


void Vec2::operator/=( const float uniformDivisor ) {
	x /= uniformDivisor;
	y /= uniformDivisor;
}


void Vec2::operator=( const Vec2& copyFrom ) {
	x = copyFrom.x;
	y = copyFrom.y;
}


bool Vec2::operator==( const Vec2& compare ) const {
	return ((x == compare.x) && (y == compare.y));
}


bool Vec2::operator!=( const Vec2& compare ) const {
	return ((x != compare.x) || (y != compare.y));
}


const Vec2 operator*( float uniformScale, const Vec2& vecToScale ) {
	return Vec2( uniformScale * vecToScale.x, uniformScale * vecToScale.y );
}


// ImGui Operators ----------------------------------------------------------------
const Vec2 Vec2::operator-( const ImVec2& imVectoSub ) const {
    float tempX = x - imVectoSub.x;
    float tempY = y - imVectoSub.y;
    return Vec2( tempX, tempY );
}


void Vec2::operator-=( const ImVec2& imVecToSub ) {
    x -= imVecToSub.x;
    y -= imVecToSub.y;
}


const Vec2 operator-( const ImVec2& imVec, const Vec2& vecToSub ) {
    float tempX = imVec.x - vecToSub.x;
    float tempY = imVec.y - vecToSub.y;
    return Vec2( tempX, tempY );
}


const std::string Vec2::GetAsString() const {
    return Stringf( "%0.3f,%0.3f", x, y );
}


ImVec2 Vec2::GetAsImGui() const {
    return ImVec2( x, y );
}


float Vec2::GetLength() const {
    return sqrt( GetLengthSquared() );
}


float Vec2::GetLengthSquared() const {
    return (x*x) + (y*y);
}


float Vec2::GetAngleDegrees() const {
    return Atan2Degrees( y, x );
}


float Vec2::GetAngleRadians() const {
    return atan2f( y, x );
}


const Vec2 Vec2::GetRotated90Degrees() const {
    return Vec2( -y, x );
}


const Vec2 Vec2::GetRotatedMinus90Degrees() const {
    return Vec2( y, -x );
}


const Vec2 Vec2::GetRotatedDegrees( float angleDegrees ) const {
    float theta = GetAngleDegrees();
    theta += angleDegrees;
    float tempX = GetLength() * CosDegrees( theta );
    float tempY = GetLength() * SinDegrees( theta );
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::GetRotatedRadians( float angleRadians ) const {
    float theta = GetAngleRadians();
    theta += angleRadians;
    float tempX = GetLength() * cos( theta );
    float tempY = GetLength() * sin( theta );
    return Vec2( tempX, tempY );
}


const Vec2 Vec2::GetClamped( float maxLength ) const {
    float theta = GetAngleRadians();
    float radius = GetLength();
    radius = (radius > maxLength) ? maxLength : radius;
    return MakeFromPolarRadians( theta, radius );
}


const Vec2 Vec2::GetNormalized() const {
    if( x == 0.f && y == 0.f ) {
        return Vec2::ZERO;
    }

    float theta = GetAngleRadians();
    return MakeFromPolarRadians( theta, 1.f );
}


void Vec2::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ',' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 2, Stringf( "Invalid initialization string (%s) for Vec2", valuesAsText.c_str() ) );

    x = (float)atof( splitString[0].c_str() );
    y = (float)atof( splitString[1].c_str() );
}


void Vec2::ClampLength( float maxLength ) {
    float lengthSquared = GetLengthSquared();

    if( lengthSquared > (maxLength * maxLength) ) {
        SetLength( maxLength );
    }
}


void Vec2::SetLength( float newLength ) {
    Vec2 tempVec = MakeFromPolarRadians( GetAngleRadians(), newLength );
    *this = tempVec;
}


void Vec2::SetAngleDegrees( float newAngleDegrees ) {
    float length = GetLength();
    Vec2 tempVec = MakeFromPolarDegrees( newAngleDegrees, length );
    x = tempVec.x;
    y = tempVec.y;
}


void Vec2::SetAngleRadians( float newAngleRadians ) {
    float length = GetLength();
    Vec2 tempVec = MakeFromPolarRadians( newAngleRadians, length );
    x = tempVec.x;
    y = tempVec.y;
}


void Vec2::SetPolarDegrees( float newAngleDegrees, float newLength ) {
    Vec2 tempVec = MakeFromPolarDegrees( newAngleDegrees, newLength );
    x = tempVec.x;
    y = tempVec.y;
}


void Vec2::SetPolarRadians( float newangleRadians, float newLength ) {
    Vec2 tempVec = MakeFromPolarRadians( newangleRadians, newLength );
    x = tempVec.x;
    y = tempVec.y;
}


void Vec2::RotateDegrees( float angleDegrees ) {
    Vec2 tempVec = GetRotatedDegrees( angleDegrees );
    x = tempVec.x;
    y = tempVec.y;
}


void Vec2::RotateRadians( float angleRadians ) {
    Vec2 tempVec = GetRotatedRadians( angleRadians );
    x = tempVec.x;
    y = tempVec.y;
}


void Vec2::Rotate90Degrees() {
    RotateDegrees( 90.f );
}


void Vec2::RotateMinus90Degrees() {
    RotateDegrees( -90.f );
}


void Vec2::Normalize() {
    Vec2 tempVec = GetNormalized();
    *this = tempVec;
}


float Vec2::NormalizeGetPreviousLength() {
    float previousLength = GetLength();
    Normalize();
    return previousLength;
}


// Unit Tests ------------------------------------------------
UNITTEST( "Constructors", "Vec2", 0 ) {
    Vec2 v1;								// default constructor
    Vec2 v2( 1.4f, -1.6f );					// explicit constructor
    Vec2 v3 = Vec2( -1.3f, 1.7f );	        // explicit constructor (per C++ standard)
    Vec2 v4( v2 );							// copy constructor
    Vec2 v5 = v3;							// copy constructor (per C++ standard)
    Vec2 v6( "-2.1,0" );                    // explicit string constructor
    Vec2 v7 = Vec2( "6.f,-7.1f" );          // explicit string constructor (per C++ standard)

    UnitTest::VerifyResult( sizeof( Vec2 ) == 8,    "sizeof(Vec2) was not 8 bytes",                                     theTest );
    UnitTest::VerifyResult( v2.x == 1.4f,           "Vec2( x, y ) : explicit constructor failed to assign x",           theTest );
    UnitTest::VerifyResult( v2.y == -1.6f,          "Vec2( x, y ) : explicit constructor failed to assign y",           theTest );
    UnitTest::VerifyResult( v4.x == 1.4f,           "Vec2( vec2 ) : copy constructor failed to copy x",                 theTest );
    UnitTest::VerifyResult( v4.y == -1.6f,          "Vec2( vec2 ) : copy constructor failed to copy y",                 theTest );
    UnitTest::VerifyResult( v6.x == -2.1f,          "Vec2( string ) : explicit string constructor failed to assign x",  theTest );
    UnitTest::VerifyResult( v6.y == 0.f,            "Vec2( string ) : explicit string constructor failed to assign y",  theTest );
    UnitTest::VerifyResult( v7.x == 6.f,            "Vec2( string ) : string constructor failed to assign x.f",         theTest );
    UnitTest::VerifyResult( v7.y == -7.1f,          "Vec2( string ) : string constructor failed to assign y.f",         theTest );

    // Static constants
    UnitTest::VerifyResult( Vec2::ZERO.x == 0.f,    "Vec2::ZERO : x is non-zero",       theTest );
    UnitTest::VerifyResult( Vec2::ZERO.y == 0.f,    "Vec2::ZERO : y is non-zero",       theTest );
    UnitTest::VerifyResult( Vec2::ONE.x == 1.f,     "Vec2::ONE : x is non-one",         theTest );
    UnitTest::VerifyResult( Vec2::ONE.y == 1.f,     "Vec2::ONE : y is non-one",         theTest );
    UnitTest::VerifyResult( Vec2::LEFT.x == -1.f,   "Vec2::LEFT : x is not neg one",    theTest );
    UnitTest::VerifyResult( Vec2::LEFT.y == 0.f,    "Vec2::LEFT : y is non-zero",       theTest );
    UnitTest::VerifyResult( Vec2::RIGHT.x == 1.f,   "Vec2::RIGHT : x is not one",       theTest );
    UnitTest::VerifyResult( Vec2::RIGHT.y == 0.f,   "Vec2::RIGHT : y is non-zero",      theTest );
    UnitTest::VerifyResult( Vec2::UP.x == 0.f,      "Vec2::UP : x is non-zero",         theTest );
    UnitTest::VerifyResult( Vec2::UP.y == 1.f,      "Vec2::UP : y is not one",          theTest );
    UnitTest::VerifyResult( Vec2::DOWN.x == 0.f,    "Vec2::DOWN : x is non-zero",       theTest );
    UnitTest::VerifyResult( Vec2::DOWN.y == -1.f,   "Vec2::DOWN : y is not neg one",    theTest );
}


UNITTEST( "Temporary Operators", "Vec2", 0 ) {
    Vec2 v1( 3.5f, -1.5f );					// explicit constructor from x,y
    Vec2 v2 = Vec2( -0.5f, 1.0f );	// explicit constructor from x,y

    Vec2 v3 = v1 + v2;						// operator+ (vec2 + vec2)
    Vec2 v4 = v1 + v1;						// operator+ (vec2 + vec2), added with itself
    Vec2 v5 = v1 - v2;						// operator- (vec2 - vec2)
    Vec2 v6 = v1 - v1;						// operator- (vec2 - vec2), subtract from itself
    Vec2 v7 = v1 * 2.0f;					// operator* (vec2 * float)
    Vec2 v8 = 2.0f * v1;					// operator* (float * vec2)
    Vec2 v9 = v1 / 2.0f;					// operator/ (vec2 / float)
    Vec2 v10 = v1 * v2;                     // operator* (vec2 * vec2)

    UnitTest::VerifyResult( v3.x  == 3.0f,   "Vec2::operator+ : did not add x in (vec2 + vec2) correctly",                  theTest );
    UnitTest::VerifyResult( v4.x  == 7.0f,   "Vec2::operator+ : did not add x in (vec2 + vec2) with itself correctly",      theTest );
    UnitTest::VerifyResult( v5.x  == 4.0f,   "Vec2::operator- : did not subtract x in (vec2 - vec2) correctly",             theTest );
    UnitTest::VerifyResult( v6.x  == 0.0f,   "Vec2::operator- : did not subtract x in (vec2 - vec2) from itself correctly", theTest );
    UnitTest::VerifyResult( v7.x  == 7.0f,   "Vec2::operator* : did not scale x in (vec2 * float) correctly",               theTest );
    UnitTest::VerifyResult( v8.x  == 7.0f,   "Vec2::operator* : did not scale x in (float * vec2) correctly",               theTest );
    UnitTest::VerifyResult( v9.x  == 1.75f,  "Vec2::operator/ : did not divide x in (vec2 / float) correctly",              theTest );
    UnitTest::VerifyResult( v10.x == -1.75f, "Vec2::operator* : did not scale x in (vec2 * vec2) correctly",                theTest );

    UnitTest::VerifyResult( v3.y  == -0.5f,  "Vec2::operator+ : did not add y in (vec2 + vec2) correctly",                  theTest );
    UnitTest::VerifyResult( v4.y  == -3.0f,  "Vec2::operator+ : did not add y in (vec2 + vec2) with itself correctly",      theTest );
    UnitTest::VerifyResult( v5.y  == -2.5f,  "Vec2::operator- : did not subtract y in (vec2 - vec2) correctly",             theTest );
    UnitTest::VerifyResult( v6.y  ==  0.0f,  "Vec2::operator- : did not subtract y in (vec2 - vec2) from itself correctly", theTest );
    UnitTest::VerifyResult( v7.y  == -3.0f,  "Vec2::operator* : did not scale y in (vec2 * float) correctly",               theTest );
    UnitTest::VerifyResult( v8.y  == -3.0f,  "Vec2::operator* : did not scale y in (float * vec2) correctly",               theTest );
    UnitTest::VerifyResult( v9.y  == -0.75f, "Vec2::operator/ : did not divide y in (vec2 / float) correctly",              theTest );
    UnitTest::VerifyResult( v10.y == -1.5f,  "Vec2::operator* : did not scale y in (vec2 * vec2) correctly",                theTest );
}


UNITTEST( "Comparison Operators", "Vec2", 0 ) {
    Vec2 v1( 3.f, 4.f );					// explicit constructor
    Vec2 v2( 3.f, 4.f );					// explicit constructor
    Vec2 v3( 3.f, 99.f );					// explicit constructor
    Vec2 v4( 99.f, 4.f );					// explicit constructor
    Vec2 v5( 3.f, 3.f );					// explicit constructor
    Vec2 v6( 4.f, 4.f );					// explicit constructor
    Vec2 v7( 4.f, 3.f );					// explicit constructor

    UnitTest::VerifyResult( v1 == v2, "Vec2::operator== : failed to detect match",                      theTest );
    UnitTest::VerifyResult( !(v1 != v2), "Vec2::operator!= : failed to reject match",                   theTest );

    UnitTest::VerifyResult( v1 != v3, "Vec2::operator!= : failed to detect y-mismatch",                 theTest );
    UnitTest::VerifyResult( v1 != v4, "Vec2::operator!= : failed to detect x-mismatch",                 theTest );
    UnitTest::VerifyResult( v1 != v5, "Vec2::operator!= : failed to detect y-mismatch coincidence",     theTest );
    UnitTest::VerifyResult( v1 != v6, "Vec2::operator!= : failed to detect x-mismatch coincidence",     theTest );
    UnitTest::VerifyResult( v1 != v7, "Vec2::operator!= : failed to detect x/y mixup",                  theTest );

    UnitTest::VerifyResult( !(v1 == v3), "Vec2::operator== : failed to reject y-mismatch",              theTest );
    UnitTest::VerifyResult( !(v1 == v4), "Vec2::operator== : failed to reject x-mismatch",              theTest );
    UnitTest::VerifyResult( !(v1 == v5), "Vec2::operator== : failed to reject y-mismatch coincidence",  theTest );
    UnitTest::VerifyResult( !(v1 == v6), "Vec2::operator== : failed to reject x-mismatch coincidence",  theTest );
    UnitTest::VerifyResult( !(v1 == v7), "Vec2::operator== : failed to reject x/y mixup",               theTest );
}


UNITTEST( "Assignment Operators", "Vec2", 0 ) {
    Vec2 v1( 3.f, 4.f );					// explicit constructor
    Vec2 v2;								// default constructor
    v2 = v1;										// operator= (assignment operator)
    v1 += Vec2( 1.f, 2.f );					// operator+= (add-and-assign operator)
    Vec2 v3( 3.f, 4.f );					// 
    v3 -= Vec2( 2.f, 1.f );					// operator-= (subtract-and-assign operator)
    Vec2 v4( 3.f, 4.f );					// 
    Vec2 v5( 5.f, 7.f );					// 
    Vec2 v6( 1.f, 2.f );					// 
    Vec2 v7( 8.f, 6.f );					// 
    v4 += v5;										// 
    v5 -= v6;										// 
    v6 *= 3.f;										// operator*= (uniform scale by float)
    v7 /= 2.f;										// operator/= (uniform scale by 1/float)

    UnitTest::VerifyResult( v2.x == 3.f, "Vec2::operator= : failed to copy x",                      theTest );
    UnitTest::VerifyResult( v2.y == 4.f, "Vec2::operator= : failed to copy y",                      theTest );
    UnitTest::VerifyResult( v1.x == 4.f, "Vec2::operator+= : failed to add x",                      theTest );
    UnitTest::VerifyResult( v1.y == 6.f, "Vec2::operator+= : failed to add y",                      theTest );
    UnitTest::VerifyResult( v3.x == 1.f, "Vec2::operator-= : failed to subtract x",                 theTest );
    UnitTest::VerifyResult( v3.y == 3.f, "Vec2::operator-= : failed to subtract y",                 theTest );
    UnitTest::VerifyResult( v4.x == 8.f, "Vec2::operator+= : failed to add existing vec2's x",      theTest );
    UnitTest::VerifyResult( v4.y == 11.f, "Vec2::operator+= : failed to add existing vec2's y",     theTest );
    UnitTest::VerifyResult( v5.x == 4.f, "Vec2::operator-= : failed to subtract existing vec2's x", theTest );
    UnitTest::VerifyResult( v5.y == 5.f, "Vec2::operator-= : failed to subtract existing vec2's y", theTest );
    UnitTest::VerifyResult( v6.x == 3.f, "Vec2::operator*= : failed to scale x",                    theTest );
    UnitTest::VerifyResult( v6.y == 6.f, "Vec2::operator*= : failed to scale y",                    theTest );
    UnitTest::VerifyResult( v7.x == 4.f, "Vec2::operator/= : failed to divide x",                   theTest );
    UnitTest::VerifyResult( v7.y == 3.f, "Vec2::operator/= : failed to divide y",                   theTest );
}


UNITTEST( "Methods", "Vec2", 0 ) {
        // Vec2::MakeFromPolar static methods
    Vec2 a( 4.f, 3.f );
    Vec2 b( 8.f, -6.f );
    Vec2 c( -0.06f, -0.08f );
    Vec2 d( 0.f, 1.f );
    Vec2 e = Vec2::MakeFromPolarRadians( 5.1f, 9.3f );
    Vec2 f = Vec2::MakeFromPolarDegrees( 160.3f, 2.7f );
    Vec2 g = Vec2::MakeFromPolarRadians( 3.9f );
    Vec2 h = Vec2::MakeFromPolarDegrees( 245.7f );

    float eCorrectX = -2.5419704709925023961033282004531f;
    float eCorrectY = 0.9101571977423216172547009490633f;
    float fCorrectX = 3.5151930072307192388813526422395f;
    float fCorrectY = -8.6100765456479103615991601021672f;
    float gCorrectX = -0.72593230420014012937233048461435f;
    float gCorrectY = -0.68776615918397381809088812537869f;
    float hCorrectX = -0.41151435860510877405343473217572f;
    float hCorrectY = -0.91140327663544524821332779863426f;

    std::string aStr = a.GetAsString();
    std::string bStr = b.GetAsString();
    std::string cStr = c.GetAsString();
    std::string dStr = d.GetAsString();

    std::string aCorrectStr = "4.000,3.000";
    std::string bCorrectStr = "8.000,-6.000";
    std::string cCorrectStr = "-0.060,-0.080";
    std::string dCorrectStr = "0.000,1.000";

    UnitTest::VerifyResult( IsMostlyEqual( e, fCorrectX, fCorrectY ), "Vec2::MakeFromPolarRadians() failed for radians=5.1, length=9.3",            theTest );
    UnitTest::VerifyResult( IsMostlyEqual( f, eCorrectX, eCorrectY ), "Vec2::MakeFromPolarDegrees() failed for degrees=160.3, length=2.7",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( g, gCorrectX, gCorrectY ), "Vec2::MakeFromPolarRadians() failed for radians=3.9, length=default(1)",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( h, hCorrectX, hCorrectY ), "Vec2::MakeFromPolarDegrees() failed for degrees=245.7, length=default(1)",   theTest );

    UnitTest::VerifyResult( aStr == aCorrectStr, "Vec2::GetAsString failed for (4.f, 3.f)",     theTest );
    UnitTest::VerifyResult( bStr == bCorrectStr, "Vec2::GetAsString failed for (8.f, -6.f)",    theTest );
    UnitTest::VerifyResult( cStr == cCorrectStr, "Vec2::GetAsString failed for (-.06f, -.08f)", theTest );
    UnitTest::VerifyResult( dStr == dCorrectStr, "Vec2::GetAsString failed for (0.f, 1.f)",     theTest );

    // Vec2::GetLength methods
    float aLength = a.GetLength();
    float bLength = b.GetLength();
    float cLength = c.GetLength();
    float aLengthSquared = a.GetLengthSquared();
    float bLengthSquared = b.GetLengthSquared();
    float cLengthSquared = c.GetLengthSquared();
    float dLengthSquared = d.GetLengthSquared();

    float aLengthCorrect = 5.f;
    float bLengthCorrect = 10.f;
    float cLengthCorrect = 0.1f;
    float aLengthSquaredCorrect = 25.f;
    float bLengthSquaredCorrect = 100.f;
    float cLengthSquaredCorrect = 0.01f;
    float dLengthSquaredCorrect = 1.f;

    UnitTest::VerifyResult( IsMostlyEqual( aLength, aLengthCorrect ), "Vec2::GetLength() is wrong for positive x,y",                        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bLength, bLengthCorrect ), "Vec2::GetLength() is wrong for positive x, negative y",              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cLength, cLengthCorrect ), "Vec2::GetLength() is wrong for negative x,y",                        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aLengthSquared, aLengthSquaredCorrect ), "Vec2::GetLengthSquared() is wrong for positive x,y",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bLengthSquared, bLengthSquaredCorrect ), "Vec2::GetLengthSquared() is wrong for mixed x,y",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cLengthSquared, cLengthSquaredCorrect ), "Vec2::GetLengthSquared() is wrong for negative x,y",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( dLengthSquared, dLengthSquaredCorrect ), "Vec2::GetLengthSquared() is wrong for vec2(0,1)",      theTest );

    // Vec2::GetAngle methods
    float aRadians = a.GetAngleRadians();
    float cRadians = c.GetAngleRadians();
    float dRadians = d.GetAngleRadians();
    float aDegrees = a.GetAngleDegrees();
    float cDegrees = c.GetAngleDegrees();
    float dDegrees = d.GetAngleDegrees();

    float aRadiansCorrect = 0.64350110879328438680280922871732f;
    float cRadiansCorrect = -2.214297435588181006034130920357f;
    float dRadiansCorrect = 1.5707963267948966192313216916398f;
    float aDegreesCorrect = 36.869897645844021296855612559093f;
    float cDegreesCorrect = -126.86989764584402129685561255909f;
    float dDegreesCorrect = 90.f;

    UnitTest::VerifyResult( IsMostlyEqual( aRadians, aRadiansCorrect ), "Vec2::GetAngleRadians() is wrong for positive x,y",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cRadians, cRadiansCorrect ), "Vec2::GetAngleRadians() is wrong for negative x,y",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( dRadians, dRadiansCorrect ), "Vec2::GetAngleRadians() is wrong for x=0 (vertical)",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aDegrees, aDegreesCorrect ), "Vec2::GetAngleDegrees() is wrong for positive x,y",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cDegrees, cDegreesCorrect ), "Vec2::GetAngleDegrees() is wrong for negative x,y",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( dDegrees, dDegreesCorrect ), "Vec2::GetAngleDegrees() is wrong for x=0 (vertical)",  theTest );

    // Vec2::GetRotated methods
    Vec2 aRotated90     = a.GetRotated90Degrees();
    Vec2 cRotated90     = c.GetRotated90Degrees();
    Vec2 bRotatedNeg90  = b.GetRotatedMinus90Degrees();
    Vec2 cRotatedNeg90  = c.GetRotatedMinus90Degrees();
    Vec2 aRotated1Rad   = a.GetRotatedRadians( 1.f );
    Vec2 bRotated9Rad   = b.GetRotatedRadians( 9.f );
    Vec2 cRotatedNegRad = c.GetRotatedRadians( -7.31f );
    Vec2 aRotated30     = a.GetRotatedDegrees( 30.f );
    Vec2 bRotatedNeg205 = b.GetRotatedDegrees( -205.f );
    Vec2 cRotated555    = c.GetRotatedDegrees( 555.555f );

    float aRot90CorrectX        = -3.f;			    float aRot90CorrectY        = 4.f;
    float cRot90CorrectX        = 0.08f;		    float cRot90CorrectY        = -0.06f;
    float bRotNeg90CorrectX     = -6.f;			    float bRotNeg90CorrectY     = -8.f;
    float cRotNeg90CorrectX     = -0.08f;		    float cRotNeg90CorrectY     = 0.06f;
    float aRot1RadCorrectX      = -0.36320373f;	    float aRot1RadCorrectY      = 4.9867908f;
    float bRot9RadCorrectX      = -4.81633118f;	    float bRot9RadCorrectY      = 8.7637294f;
    float cRotNegRadCorrectX    = -0.09950520f;	    float cRotNegRadCorrectY    = 0.00993552806f;
    float aRot30CorrectX        = 1.9641016f;	    float aRot30CorrectY        = 4.5980762f;
    float bRotNeg205CorrectX    = -4.7147527f;	    float bRotNeg205CorrectY    = 8.8187928f;
    float cRot555CorrectX       = 0.036349393f;	    float cRot555CorrectY       = 0.09315966f;

    UnitTest::VerifyResult( IsMostlyEqual( aRotated90,      aRot90CorrectX,     aRot90CorrectY ),       "Vec2::GetRotated90Degrees() was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cRotated90,      cRot90CorrectX,     cRot90CorrectY ),       "Vec2::GetRotated90Degrees() was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bRotatedNeg90,   bRotNeg90CorrectX,  bRotNeg90CorrectY ),    "Vec2::GetRotatedMinus90Degrees() was incorrect",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cRotatedNeg90,   cRotNeg90CorrectX,  cRotNeg90CorrectY ),    "Vec2::GetRotatedMinus90Degrees() was incorrect",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aRotated1Rad,    aRot1RadCorrectX,   aRot1RadCorrectY ),     "Vec2::GetRotatedRadians(1) was incorrect",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bRotated9Rad,    bRot9RadCorrectX,   bRot9RadCorrectY ),     "Vec2::GetRotatedRadians(9) was incorrect",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cRotatedNegRad,  cRotNegRadCorrectX, cRotNegRadCorrectY ),   "Vec2::GetRotatedRadians(-7.31) was incorrect",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aRotated30,      aRot30CorrectX,     aRot30CorrectY ),       "Vec2::GetRotatedDegrees(30) was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bRotatedNeg205,  bRotNeg205CorrectX, bRotNeg205CorrectY ),   "Vec2::GetRotatedDegrees(-205) was incorrect",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cRotated555,     cRot555CorrectX,    cRot555CorrectY ),      "Vec2::GetRotatedDegrees(555) was incorrect",       theTest );

    // Vec2::Polar methods
    Vec2 j( 30.f, -40.f );
    Vec2 k( -8.f, 6.f );
    Vec2 l( 1000.f, 0.f );
    Vec2 m( -2.0f, -1.5f );
    Vec2 n( -3.1f, -4.3f );
    Vec2 o( 200.f, 100.f );
    Vec2 p( 1.3f, 2.1f );
    Vec2 q( -2.3f, 3.1f );
    Vec2 r( 3.3f, -4.1f );
    Vec2 s( -4.3f, -5.1f );
    Vec2 t( 7.3f, -8.1f );
    Vec2 u( -8.3f, -9.1f );
    Vec2 v( 5.3f, 6.1f );
    Vec2 w( -6.3f, 7.1f );
    j.SetAngleRadians( 2.4980915447965088516598341545622f );
    k.SetAngleRadians( -666.86989764584402129685561255909f );
    l.SetAngleDegrees( 36.869897645844021296855612559093f );
    m.SetAngleDegrees( -233.13010235415597870314438744091f );
    n.SetPolarRadians( -8.5f, 10.f );
    o.SetPolarDegrees( 30.f, 100.f );
    p.Rotate90Degrees();
    q.RotateMinus90Degrees();
    r.Rotate90Degrees();
    s.RotateMinus90Degrees();
    t.RotateRadians( 3.1415926535897932384626433832795f / 4.f );
    u.RotateRadians( -10.7f );
    v.RotateDegrees( 30.f );
    w.RotateDegrees( -501.3f );

    float JCorrectX = -40.f;		float JCorrectY = 30.f;
    float KCorrectX = 6.58287268f;	float KCorrectY = -7.52766812f;
    float LCorrectX = 800.f;		float LCorrectY = 600.f;
    float MCorrectX = -1.5f;		float MCorrectY = 2.0f;
    float NCorrectX = -6.0201190f;	float NCorrectY = -7.98487112f;
    float OCorrectX = 86.6025403f;	float OCorrectY = 50.f;
    float pCorrectX = -2.1f;		float pCorrectY = 1.3f;
    float qCorrectX = 3.1f;			float qCorrectY = 2.3f;
    float rCorrectX = 4.1f;			float rCorrectY = 3.3f;
    float sCorrectX = -5.1f;		float sCorrectY = 4.3f;
    float TCorrectX = 10.8894444f;	float TCorrectY = -0.56568542495f;
    float UCorrectX = 11.1230797f;	float UCorrectY = -5.2893381714f;
    float VCorrectX = 1.53993464f;	float VCorrectY = 7.9327549631f;
    float WCorrectX = 9.35593443f;	float WCorrectY = -1.6020271572f;

    UnitTest::VerifyResult( IsMostlyEqual( j, JCorrectX, JCorrectY ), "Vec2::SetAngleRadians(2.498092) was incorrect",              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( k, KCorrectX, KCorrectY ), "Vec2::SetAngleRadians(-666.869898) was incorrect",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( l, LCorrectX, LCorrectY ), "Vec2::SetAngleDegrees(36.869898) was incorrect",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( m, MCorrectX, MCorrectY ), "Vec2::SetAngleDegrees(-233.130102) was incorrect",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( n, NCorrectX, NCorrectY ), "Vec2::SetPolarRadians(radians=-8.5,len=10) was incorrect",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( o, OCorrectX, OCorrectY ), "Vec2::SetPolarDegrees(degrees=30,len=100) was incorrect",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p, pCorrectX, pCorrectY ), "Vec2::Rotate90Degrees() was incorrect",                      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( q, qCorrectX, qCorrectY ), "Vec2::RotateMinus90Degrees() was incorrect",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( r, rCorrectX, rCorrectY ), "Vec2::Rotate90Degrees() was incorrect",                      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( s, sCorrectX, sCorrectY ), "Vec2::RotateMinus90Degrees() was incorrect",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( t, TCorrectX, TCorrectY ), "Vec2::RotateRadians(pi/4) was incorrect",                    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( u, UCorrectX, UCorrectY ), "Vec2::RotateRadians(-10.7) was incorrect",                   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( v, VCorrectX, VCorrectY ), "Vec2::RotateDegrees(30) was incorrect",                      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( w, WCorrectX, WCorrectY ), "Vec2::RotateDegrees(-501.3) was incorrect",                  theTest );

    // Vec2::Normalize methods
    Vec2 aa( 0.8f, 0.6f );		// Length is 1.0
    Vec2 bb( -606.f, -808.f );	// Length is 1010
    Vec2 cc( -6.f, 8.f );		// Length is 10
    Vec2 dd( 66.f, -88.f );		// Length is 110
    Vec2 aaNotClamped   = aa.GetClamped( 1.1f );
    Vec2 aaClamped      = aa.GetClamped( 0.1f );
    Vec2 bbNotClamped   = bb.GetClamped( 1020.f );
    Vec2 bbClamped      = bb.GetClamped( 1000.f );
    Vec2 aNormalized    = a.GetNormalized();
    Vec2 cNormalized    = c.GetNormalized();
    aa.SetLength( 20.f );
    bb.SetLength( 10.f );
    cc.ClampLength( 11.f ); // Should do nothing to cc, still (-6,8)
    dd.ClampLength( 100.f ); // Should clamp from 110 to 100, now (60,-80)
    c.Normalize();
    float aOldLength = a.NormalizeGetPreviousLength();
    float bOldLength = b.NormalizeGetPreviousLength();
    float aNewLength = a.GetLength();
    float bNewLength = b.GetLength();
    float cNewLength = c.GetLength();

    float aaNoClampCorrectX = 0.8f;		float aaNoClampCorrectY = 0.6f;
    float aaClampedCorrectX = 0.08f;	float aaClampedCorrectY = 0.06f;
    float bbNoClampCorrectX = -606.f;	float bbNoClampCorrectY = -808.f;
    float bbClampedCorrectX = -600.f;	float bbClampedCorrectY = -800.f;
    float ccNoClampCorrectX = -6.f;		float ccNoClampCorrectY = 8.f;
    float ddClampedCorrectX = 60.f;		float ddClampedCorrectY = -80.f;
    float aNormCorrectX = 0.8f;			float aNormCorrectY = 0.6f;
    float cNormCorrectX = -0.6f;		float cNormCorrectY = -0.8f;
    float aCorrectX = 0.8f;				float aCorrectY = 0.6f;
    float cCorrectX = -0.6f;			float cCorrectY = -0.8f;
    float aOldLenCorrect = 5.f;
    float bOldLenCorrect = 10.f;
    float aNewLenCorrect = 1.f;
    float bNewLenCorrect = 1.f;
    float cNewLenCorrect = 1.f;

    UnitTest::VerifyResult( IsMostlyEqual( aaNotClamped,  aaNoClampCorrectX, aaNoClampCorrectY ), "Vec2::GetClamped() clamped when it shouldn't have",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aaClamped,     aaClampedCorrectX, aaClampedCorrectY ), "Vec2::GetClamped() did not clamp correctly",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bbNotClamped,  bbNoClampCorrectX, bbNoClampCorrectY ), "Vec2::GetClamped() clamped when it shouldn't have",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bbClamped,     bbClampedCorrectX, bbClampedCorrectY ), "Vec2::GetClamped() did not clamp correctly",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cc,            ccNoClampCorrectX, ccNoClampCorrectY ), "Vec2::GetClamped() clamped when it shouldn't have",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( dd,            ddClampedCorrectX, ddClampedCorrectY ), "Vec2::GetClamped() did not clamp correctly",                 theTest );

    UnitTest::VerifyResult( IsMostlyEqual( aNormalized,   aNormCorrectX,      aNormCorrectY ), "Vec2::GetNormalized() was incorrect",                           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cNormalized,   cNormCorrectX,      cNormCorrectY ), "Vec2::GetNormalized() was incorrect",                           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( c,             cCorrectX,          cCorrectY ),     "Vec2::Normalize() did not normalize correctly",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( a,             aCorrectX,          aCorrectY ),     "Vec2::NormalizeAndGetLength() did not normalize correctly",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aOldLength,    aOldLenCorrect ),                    "Vec2::NormalizeAndGetLength() did not return previous length",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bOldLength,    bOldLenCorrect ),                    "Vec2::NormalizeAndGetLength() did not return previous length",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( aNewLength,    aNewLenCorrect ),                    "Vec2::NormalizeAndGetLength() failed to make vec2 length=1",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bNewLength,    bNewLenCorrect ),                    "Vec2::NormalizeAndGetLength() failed to make vec2 length=1",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cNewLength,    cNewLenCorrect ),                    "Vec2::Normalize() failed to make vec2 length=1",                theTest );
}
