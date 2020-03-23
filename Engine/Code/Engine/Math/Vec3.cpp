#include "Engine/Math/Vec3.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"


const Vec3 Vec3::ZERO   = Vec3(  0.f,  0.f,  0.f );
const Vec3 Vec3::ONE    = Vec3(  1.f,  1.f,  1.f );

const Vec3 Vec3::LEFT   = Vec3(  -1.f, 0.f,  0.f );
const Vec3 Vec3::RIGHT  = Vec3(  1.f,  0.f,  0.f );
const Vec3 Vec3::UP     = Vec3(  0.f,  1.f,  0.f );
const Vec3 Vec3::DOWN   = Vec3(  0.f, -1.f,  0.f );

const Vec3 Vec3::SCREEN = Vec3(  0.f,  0.f, -1.f );

//-----------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec3& copy ) :
    x( copy.x ),
    y( copy.y ),
    z( copy.z ) {
}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3( float initialX, float initialY, float initialZ ) :
    x( initialX ),
    y( initialY ),
    z( initialZ ) {
}


Vec3::Vec3( const std::string& initialValues ) {
    SetFromText( initialValues );
}


Vec3::Vec3( const Vec2& initialXY, float initialZ ) :
    x( initialXY.x ),
    y( initialXY.y ),
    z( initialZ ) {
}


Vec3::Vec3( const IntVec2& initialXY, float initialZ ) :
    x( (float)initialXY.x ),
    y( (float)initialXY.y ),
    z( initialZ ) {
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + ( const Vec3& vecToAdd ) const {
	float tempX = this->x + vecToAdd.x;
	float tempY = this->y + vecToAdd.y;
	float tempZ = this->z + vecToAdd.z;
	return Vec3( tempX, tempY, tempZ );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-( const Vec3& vecToSubtract ) const {
	float tempX = this->x - vecToSubtract.x;
	float tempY = this->y - vecToSubtract.y;
	float tempZ = this->z - vecToSubtract.z;
	return Vec3( tempX, tempY, tempZ );
}


const Vec3 Vec3::operator-() const {
    return (Vec3::ZERO - *this);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( float uniformScale ) const {
	float tempX = this->x * uniformScale;
	float tempY = this->y * uniformScale;
	float tempZ = this->z * uniformScale;
	return Vec3( tempX, tempY, tempZ );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/( float inverseScale ) const {
	float tempX = this->x / inverseScale;
	float tempY = this->y / inverseScale;
	float tempZ = this->z / inverseScale;
	return Vec3( tempX, tempY, tempZ );
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=( const Vec3& vecToAdd ) {
	this->x += vecToAdd.x;
	this->y += vecToAdd.y;
	this->z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=( const Vec3& vecToSubtract ) {
	this->x -= vecToSubtract.x;
	this->y -= vecToSubtract.y;
	this->z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=( const float uniformScale ) {
	this->x *= uniformScale;
	this->y *= uniformScale;
	this->z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=( const float uniformDivisor ) {
	this->x /= uniformDivisor;
	this->y /= uniformDivisor;
	this->z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec3& copyFrom ) {
	this->x = copyFrom.x;
	this->y = copyFrom.y;
	this->z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*( float uniformScale, const Vec3& vecToScale ) {
    return vecToScale * uniformScale;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==( const Vec3& compare ) const {
	return ((this->x == compare.x) && (this->y == compare.y) && (this->z == compare.z));
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=( const Vec3& compare ) const {
	return ((this->x != compare.x) || (this->y != compare.y) || (this->z != compare.z));
}


const std::string Vec3::GetAsString() const {
    return Stringf( "%.3f,%.3f,%.3f", x, y, z );
}


float Vec3::GetLength() const {
	return sqrt( GetLengthSquared() );
}


float Vec3::GetLengthXY() const {
    return sqrt( GetLengthXYSquared() );
}


float Vec3::GetLengthSquared() const {
    return (x*x) + (y*y) + (z*z);
}


float Vec3::GetLengthXYSquared() const {
    return (x*x) + (y*y);
}


Vec3 Vec3::GetNormalized() const {
    if( *this == Vec3::ZERO ) {
        return Vec3::ZERO;
    }

    Vec3 tempVec = *this;
    tempVec.Normalize();
    return tempVec;
}


/*
float Vec3::GetAngleAboutXDegrees() const {
    return Atan2Degrees( z, y );
}


float Vec3::GetAngleAboutYDegrees() const {
    return Atan2Degrees( x, z );
}
*/


float Vec3::GetAngleAboutZDegrees() const {
    return Atan2Degrees( y, x );
}


float Vec3::GetAngleAboutZRadians() const {
    return atan2f( y, x );
}


const Vec3 Vec3::GetRotatedAboutZDegrees( float rotationDegrees ) const {
    float angleDegrees = GetAngleAboutZDegrees();
    angleDegrees += rotationDegrees;
    float tempX = GetLengthXY() * CosDegrees( angleDegrees );
    float tempY = GetLengthXY() * SinDegrees( angleDegrees );
    return Vec3( tempX, tempY, z );
}


const Vec3 Vec3::GetRotatedAboutZRadians( float rotationRadians ) const {
    float angleRadians = GetAngleAboutZRadians();
    angleRadians += rotationRadians;
    float tempX = GetLengthXY() * cos( angleRadians );
    float tempY = GetLengthXY() * sin( angleRadians );
    return Vec3( tempX, tempY, z );
}


void Vec3::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ',' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 3, Stringf( "Invalid initialization string (%s) for Vec3", valuesAsText.c_str() ) );

    x = (float)atof( splitString[0].c_str() );
    y = (float)atof( splitString[1].c_str() );
    z = (float)atof( splitString[2].c_str() );
}


void Vec3::ClampLength( float maxLength ) {
    float lengthSquared = GetLengthSquared();

    if( lengthSquared > (maxLength * maxLength) ) {
        SetLength( maxLength );
    }
}


void Vec3::SetLength( float newLength ) {
    float currentLength = GetLength();

    if( currentLength == 0.f ) {
        x = newLength;
        return;
    }

    *this *= newLength / currentLength;
}


void Vec3::Normalize() {
    if( *this == Vec3::ZERO ) {
        return;
    }

    SetLength( 1.f );
}


float Vec3::NormalizeGetPreviousLength() {
    float previousLength = GetLength();
    Normalize();
    return previousLength;
}


UNITTEST( "Basics", "Vec3", 0 ) {
    Vec3 vec3a;										    // default constructor
    Vec3 vec3b( 1.4f, -1.6f, 0.2f );				    // explicit constructor
    Vec3 vec3c = Vec3( -1.3f, 1.7f, 0.3f );	            // explicit constructor (per C++ standard)
    Vec3 vec3d( vec3b );							    // copy constructor
    Vec3 vec3e = vec3c;								    // copy constructor (per C++ standard)
    Vec3 vec3bVec2 = Vec3( Vec2( 1.4f, -1.6f ), 0.2f );
    Vec3 vec3cVec2 = Vec3( Vec2( -1.3f, 1.7f ), 0.3f );
    Vec3 vec3bStr( "1.4,-1.6,0.2" );                    // explicit string constructor
    Vec3 vec3cStr = Vec3( "-1.3f,1.7f,0.30f" );         // explicit string constructor (per C++ standard)
    Vec3 vec3IntVec2 = Vec3( IntVec2( 3, 4 ), 5.3f );         // explicit IntVec2 construct with float Z

    UnitTest::VerifyResult( sizeof( Vec3 ) == 12,                           "sizeof(Vec3) was not 12 bytes",                                                    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b, 1.4f, -1.6f, 0.2f ),      "Vec3( x, y, z ) : explicit constructor failed to assign x, y, and/or z",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3d, 1.4f, -1.6f, 0.2f ),      "Vec3( Vec3 ) : copy constructor failed to copy x, y, and/or z",                    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3bVec2, 1.4f, -1.6f, 0.2f ),  "Vec3( Vec2, z ) : Vec2, float constructor failed to assign x, y, and/or z",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3cVec2, -1.3f, 1.7f, 0.3f ),  "Vec3( Vec2, z ) : Vec2, float constructor failed to assign x, y, and/or z",        theTest );
    UnitTest::VerifyResult( vec3bStr == vec3b,                              "Vec3( string ) : string constructor failed to copy x, y, and/or z",                theTest );
    UnitTest::VerifyResult( vec3cStr == vec3c,                              "Vec3( string ) : string constructor failed to copy x, y, and/or z",                theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3IntVec2, 3.f, 4.f, 5.3f ),   "Vec3( IntVec2, z ) : IntVec2, float constructor failed to assign x, y, and/or z",  theTest );

    // Static constants
    UnitTest::VerifyResult( Vec3::ZERO.x == 0.f,    "Vec3::ZERO : x is non-zero",       theTest );
    UnitTest::VerifyResult( Vec3::ZERO.y == 0.f,    "Vec3::ZERO : y is non-zero",       theTest );
    UnitTest::VerifyResult( Vec3::ZERO.z == 0.f,    "Vec3::ZERO : z is non-zero",       theTest );
    UnitTest::VerifyResult( Vec3::ONE.x == 1.f,     "Vec3::ONE : x is non-one",         theTest );
    UnitTest::VerifyResult( Vec3::ONE.y == 1.f,     "Vec3::ONE : y is non-one",         theTest );
    UnitTest::VerifyResult( Vec3::ONE.z == 1.f,     "Vec3::ONE : z is non-one",         theTest );
    UnitTest::VerifyResult( Vec3::LEFT.x == -1.f,   "Vec3::LEFT : x is not neg one",    theTest );
    UnitTest::VerifyResult( Vec3::LEFT.y == 0.f,    "Vec3::LEFT : y is non-zero",       theTest );
    UnitTest::VerifyResult( Vec3::LEFT.z == 0.f,    "Vec3::LEFT : z is non-zero",       theTest );
    UnitTest::VerifyResult( Vec3::RIGHT.x == 1.f,   "Vec3::RIGHT : x is not one",       theTest );
    UnitTest::VerifyResult( Vec3::RIGHT.y == 0.f,   "Vec3::RIGHT : y is non-zero",      theTest );
    UnitTest::VerifyResult( Vec3::RIGHT.z == 0.f,   "Vec3::RIGHT : z is non-zero",      theTest );
    UnitTest::VerifyResult( Vec3::UP.x == 0.f,      "Vec3::UP : x is non-zero",         theTest );
    UnitTest::VerifyResult( Vec3::UP.y == 1.f,      "Vec3::UP : y is not one",          theTest );
    UnitTest::VerifyResult( Vec3::UP.z == 0.f,      "Vec3::UP : z is not one",          theTest );
    UnitTest::VerifyResult( Vec3::DOWN.x == 0.f,    "Vec3::DOWN : x is non-zero",       theTest );
    UnitTest::VerifyResult( Vec3::DOWN.y == -1.f,   "Vec3::DOWN : y is not neg one",    theTest );
    UnitTest::VerifyResult( Vec3::DOWN.z == 0.f,    "Vec3::DOWN : z is not neg one",    theTest );
    UnitTest::VerifyResult( Vec3::SCREEN.x == 0.f,  "Vec3::SCREEN : x is non-zero",     theTest );
    UnitTest::VerifyResult( Vec3::SCREEN.y == 0.f,  "Vec3::SCREEN : y is non-zero",     theTest );
    UnitTest::VerifyResult( Vec3::SCREEN.z == -1.f, "Vec3::SCREEN : z is not neg one",  theTest );

    // Basic arithmetic
    Vec3 vec3f( 3.5f, -1.5f, -7.1f );				// explicit constructor from x,y
    Vec3 vec3g = Vec3( -0.5f, 1.0f, 0.1f );	// explicit constructor from x,y

    Vec3 vec3h = vec3f + vec3g;		// operator+ (Vec3 + Vec3)
    Vec3 vec3i = vec3f + vec3f;		// operator+ (Vec3 + Vec3), added with itself
    Vec3 vec3j = vec3f - vec3g;		// operator- (Vec3 - Vec3)
    Vec3 vec3k = vec3f - vec3f;		// operator- (Vec3 - Vec3), subtract from itself
    Vec3 vec3l = vec3f * 0.2f;		// operator* (Vec3 * float)
    Vec3 vec3m = 0.2f * vec3g;		// operator* (float * Vec3)
    Vec3 vec3n = vec3f / 0.5f;		// operator/ (Vec3 / float)
    Vec3 vec3dd = -vec3n;            // operator- (negation)

    UnitTest::VerifyResult( IsMostlyEqual( vec3h,   3.0f, -0.5f, -7.f ),   "Vec3::operator+ : did not add (vec3 + vec3) correctly",                  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3i,   7.0f, -3.0f, -14.2f ), "Vec3::operator+ : did not add (vec3 + vec3) with itself correctly",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3j,   4.0f, -2.5f, -7.2f ),  "Vec3::operator- : did not subtract (vec3 - vec3) correctly",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3k,   0.0f,  0.0f,  0.0f ),  "Vec3::operator- : did not subtract (vec3 - vec3) from itself correctly", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3l,   0.7f, -0.3f, -1.42f ), "Vec3::operator* : did not scale (vec3 * float) correctly",               theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3m,  -0.1f,  0.2f,  0.02f ), "Vec3::operator* : did not scale (float * vec3) correctly",               theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3n,   7.0f, -3.0f, -14.2f ), "Vec3::operator/ : did not divide (vec3 / float) correctly",              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3dd, -7.0f,  3.0f,  14.2f ), "Vec3::operator- : did not negate (-vec3) correctly",                     theTest );

    Vec3 vec3o( 3.f, 4.f, 5.f );	// explicit constructor
    Vec3 vec3p( 3.f, 4.f, 5.f );	// explicit constructor
    Vec3 vec3q( 99.f, 4.f, 5.f );	// explicit constructor
    Vec3 vec3r( 3.f, 99.f, 5.f );	// explicit constructor
    Vec3 vec3s( 3.f, 4.f, 99.f );	// explicit constructor

    UnitTest::VerifyResult( vec3o == vec3p,    "Vec3::operator== : failed to detect match",      theTest );
    UnitTest::VerifyResult( !(vec3o != vec3p), "Vec3::operator!= : failed to reject match",      theTest );

    UnitTest::VerifyResult( vec3o != vec3q,    "Vec3::operator!= : failed to detect x-mismatch", theTest );
    UnitTest::VerifyResult( vec3o != vec3r,    "Vec3::operator!= : failed to detect y-mismatch", theTest );
    UnitTest::VerifyResult( vec3o != vec3s,    "Vec3::operator!= : failed to detect z-mismatch", theTest );

    UnitTest::VerifyResult( !(vec3o == vec3q), "Vec3::operator== : failed to reject x-mismatch", theTest );
    UnitTest::VerifyResult( !(vec3o == vec3r), "Vec3::operator== : failed to reject y-mismatch", theTest );
    UnitTest::VerifyResult( !(vec3o == vec3s), "Vec3::operator== : failed to reject z-mismatch", theTest );

    Vec3 vec3t( 3.f, 4.f, 5.f );		// explicit constructor
    Vec3 vec3u;							// default constructor
    vec3u = vec3t;						// operator= (assignment operator)
    vec3t += Vec3( 1.f, 2.f, 3.f );		// operator+= (add-and-assign operator)
    Vec3 vec3v( 3.f, 4.f, 5.f );		// 
    vec3v -= Vec3( 2.f, 1.f, 3.f );		// operator-= (subtract-and-assign operator)
    Vec3 vec3w( 3.f, 4.f, 5.f );		// 
    Vec3 vec3aa( 5.f, 7.f, 9.f );		// 
    Vec3 vec3bb( 1.f, 2.f, 3.f );		// 
    Vec3 vec3cc( 8.f, 6.f, 4.f );		// 
    vec3w += vec3aa;					// 
    vec3aa -= vec3bb;					// 
    vec3bb *= 3.f;						// operator*= (uniform scale by float)
    vec3cc /= 2.f;						// operator/= (uniform scale by 1/float)

    UnitTest::VerifyResult( IsMostlyEqual( vec3u, 3.f, 4.f, 5.f ),   "Vec3::operator= : failed to copy correctly",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3t, 4.f, 6.f, 8.f ),   "Vec3::operator+= : failed to add correctly",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3v, 1.f, 3.f, 2.f ),   "Vec3::operator-= : failed to subtract correctly",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3w, 8.f, 11.f, 14.f ), "Vec3::operator+= : failed to add existing Vec3",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3aa, 4.f, 5.f, 6.f ),  "Vec3::operator-= : failed to subtract existing Vec3", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3bb, 3.f, 6.f, 9.f ),  "Vec3::operator*= : failed to scale correctly",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3cc, 4.f, 3.f, 2.f ),  "Vec3::operator/= : failed to divide correctly",       theTest );
}


UNITTEST( "Methods", "Vec3", 0 ) {
    Vec3 vec3a( 0.2f, -0.3f, 0.6f ); // Length of 0.7
    Vec3 vec3b( -1.1f, 4.4f, -8.8f ); // Length of 9.9

    UnitTest::VerifyResult( IsMostlyEqual( vec3a.GetLength(), 0.7f ),                       "Vec3::GetLength() was incorrect",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b.GetLength(), 9.9f ),                       "Vec3::GetLength() was incorrect",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3a.GetLengthXY(), 0.360555f ),                "Vec3::GetLengthXY() was incorrect",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b.GetLengthXY(), 4.535416f ),                "Vec3::GetLengthXY() was incorrect",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3a.GetLengthSquared(), 0.49f ),               "Vec3::GetLengthSquared() was incorrect",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b.GetLengthSquared(), 98.01f ),              "Vec3::GetLengthSquared() was incorrect",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3a.GetLengthXYSquared(), 0.13f ),             "Vec3::GetLengthXYSquared() was incorrect",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b.GetLengthXYSquared(), 20.57f ),            "Vec3::GetLengthXYSquared() was incorrect",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3a.GetAngleAboutZRadians(), -0.982793748f ),  "Vec3::GetAngleAboutZRadians() was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b.GetAngleAboutZRadians(), 1.81577504f ),    "Vec3::GetAngleAboutZRadians() was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3a.GetAngleAboutZDegrees(), -56.309932f ),    "Vec3::GetAngleAboutZDegrees() was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3b.GetAngleAboutZDegrees(), 104.036240f ),    "Vec3::GetAngleAboutZDegrees() was incorrect", theTest );

    Vec3 vec3c = vec3a.GetRotatedAboutZRadians( 1.f );
    Vec3 vec3d = vec3b.GetRotatedAboutZRadians( -12.345f );
    Vec3 vec3e = vec3a.GetRotatedAboutZDegrees( 10.f );
    Vec3 vec3f = vec3b.GetRotatedAboutZDegrees( -1234.567f );

    // Make sure z is never affected by XY rotations
    UnitTest::VerifyResult( IsMostlyEqual( vec3c.z, vec3a.z ), "Vec3::GetRotatedAboutZRadians() should not affect vec3.z", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3f.z, vec3b.z ), "Vec3::GetRotatedAboutZDegrees() should not affect vec3.z", theTest );

    // Full rotational results check (changes on XY only, rotating "in 2D" about Z)
    Vec3 vec3cCorrect( 0.360501766f, 0.00620349636f, 0.600000024f );
    Vec3 vec3dCorrect( -2.03925061f, 4.05110550f, -8.80000019f );
    Vec3 vec3eCorrect( 0.249056026f, -0.260712683f, 0.600000024f );
    Vec3 vec3fCorrect( 2.88300228f, -3.50118518f, -8.80000019f );

    UnitTest::VerifyResult( IsMostlyEqual( vec3c, vec3cCorrect ), "Vec3::GetRotatedAboutZRadians() was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3d, vec3dCorrect ), "Vec3::GetRotatedAboutZRadians() was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3e, vec3eCorrect ), "Vec3::GetRotatedAboutZDegrees() was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3f, vec3fCorrect ), "Vec3::GetRotatedAboutZDegrees() was incorrect", theTest );

    // Normalize and SetLength
    // 2, 3, 6 = 7
    // 9, 12, 20 = 25
    Vec3 vec3g( 2.f, 3.f, 6.f );            // Length is 7.f
    Vec3 vec3h( -202.f, -303.f, -606.f );   // Length is 707.f
    Vec3 vec3i( -20.f, 30.f, -60.f );       // Length is 70.f
    Vec3 vec3j( 22.f, -33.f, 66.f );        // Length is 77.f
    Vec3 vec3k( 9.f, 12.f, -20.f );         // Length is 25.f
    Vec3 vec3l = -Vec3::RIGHT;      // (-1, 0, 0)
    Vec3 vec3m( 22.f, -33.f, 66.f );        // Length is 77.f
    Vec3 vec3n( 22.f, -33.f, 66.f );        // Length is 77.f

    Vec3 vec3gNormalized = vec3g.GetNormalized();
    Vec3 vec3iNormalized = vec3i.GetNormalized();
    Vec3 vec3zeroNormalized = Vec3::ZERO.GetNormalized();
    Vec3 vec3oneNormalized = Vec3::ONE.GetNormalized(); // sqrt(3) / 3

    vec3h.SetLength( 700.f );      // Reduced to -200, -300, -600
    vec3j.SetLength( 77.f );       // Unchanged
    vec3m.ClampLength( 90.f );     // Unchanged
    vec3n.ClampLength( 7.f );      // Reduced to 2, -3, 6
    vec3k.Normalize();             // Reduced to .36, .48, -.8

    float vec3gOldLength = vec3g.NormalizeGetPreviousLength(); // 7.f
    float vec3lOldLength = vec3l.NormalizeGetPreviousLength(); // 1.f
    float vec3gNewLength = vec3g.GetLength();             // 1.f
    float vec3lNewLength = vec3l.GetLength();             // 1.f

    float vec3gNormCorrectX = 0.28571429f;   float vec3gNormCorrectY = 0.42857143f;   float vec3gNormCorrectZ = 0.85714286f;
    float vec3iNormCorrectX = -0.28571429f;  float vec3iNormCorrectY = 0.42857143f;   float vec3iNormCorrectZ = -0.85714286f;
    float vec3zeroNormCorrectX = 0.f;        float vec3zeroNormCorrectY = 0.f;        float vec3zeroNormCorrectZ = 0.f;
    float vec3oneNormCorrectX = 0.57735027f; float vec3oneNormCorrectY = 0.57735027f; float vec3oneNormCorrectZ = 0.57735027f;
    float vec3hCorrectX = -200.f;            float vec3hCorrectY = -300.f;            float vec3hCorrectZ = -600.f;
    float vec3jCorrectX = 22.f;              float vec3jCorrectY = -33.f;             float vec3jCorrectZ = 66.f;
    float vec3kCorrectX = 0.36f;             float vec3kCorrectY = 0.48f;             float vec3kCorrectZ = -0.8f;
    float vec3gCorrectOldLength = 7.f;
    float vec3lCorrectOldLength = 1.f;
    float vec3gCorrectNewLength = 1.f;
    float vec3lCorrectNewLength = 1.f;
    float vec3mCorrectX = 22.f;              float vec3mCorrectY = -33.f;             float vec3mCorrectZ = 66.f;
    float vec3nCorrectX = 2.f;               float vec3nCorrectY = -3.f;              float vec3nCorrectZ = 6.f;

    UnitTest::VerifyResult( IsMostlyEqual( vec3gNormalized,     vec3gNormCorrectX,      vec3gNormCorrectY,    vec3gNormCorrectZ ),     "Vec3::GetNormalized failed on ( 2.f, 3.f, 6.f )",                           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3iNormalized,     vec3iNormCorrectX,      vec3iNormCorrectY,    vec3iNormCorrectZ ),     "Vec3::GetNormalized failed on ( -20.f, 30.f, -60.f )",                      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3zeroNormalized,  vec3zeroNormCorrectX,   vec3zeroNormCorrectY, vec3zeroNormCorrectZ ),   "Vec3::GetNormalized failed on Vec3::ZERO",                                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3oneNormalized,   vec3oneNormCorrectX,    vec3oneNormCorrectY,  vec3oneNormCorrectZ ),    "Vec3::GetNormalized failed on Vec3::ONE",                                  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3h,               vec3hCorrectX,          vec3hCorrectY,        vec3hCorrectZ ),          "Vec3::SetLength(700.f) failed on ( -202.f, -303.f, -606.f )",              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3j,               vec3jCorrectX,          vec3jCorrectY,        vec3jCorrectZ ),          "Vec3::SetLength(77.f) failed on ( 22.f, -33.f, 66.f )",                    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3k,               vec3kCorrectX,          vec3kCorrectY,        vec3kCorrectZ ),          "Vec3::Normalize failed on ( 9.f, 12.f, -20.f )",                           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3gOldLength,      vec3gCorrectOldLength ),                                                "Vec3::NormalizeAndGetLength failed getting length of ( 2.f, 3.f, 6.f )",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3lOldLength,      vec3lCorrectOldLength ),                                                "Vec3::NormalizeAndGetLength failed getting length of ( -1.f, 0.f, 0.f )",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3gNewLength,      vec3gCorrectNewLength ),                                                "Vec3::NormalizeAndGetLength failed normalizing ( 2.f, 3.f, 6.f )",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3lNewLength,      vec3lCorrectNewLength ),                                                "Vec3::NormalizeAndGetLength failed normalizing ( -1.f, 0.f, 0.f )",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3m,               vec3mCorrectX,          vec3mCorrectY,        vec3mCorrectZ ),          "Vec3::ClampLength(90.f) failed on ( 22.f, -33.f, 66.f )",                  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3n,               vec3nCorrectX,          vec3nCorrectY,        vec3nCorrectZ ),          "Vec3::ClampLength(7.f) failed on ( 22.f, -33.f, 66.f )",                   theTest );

    // GetAsString
    std::string vec3aStr = vec3a.GetAsString();
    std::string vec3bStr = vec3b.GetAsString();
    std::string vec3cStr = vec3c.GetAsString();

    std::string vec3aCorrectStr = "0.200,-0.300,0.600";
    std::string vec3bCorrectStr = "-1.100,4.400,-8.800";
    std::string vec3cCorrectStr = "0.361,0.006,0.600";

    UnitTest::VerifyResult( vec3aStr == vec3aCorrectStr, "Vec3::GetAsString failed for ( 0.2f, -0.3f, 0.6f )",      theTest );
    UnitTest::VerifyResult( vec3bStr == vec3bCorrectStr, "Vec3::GetAsString failed for ( -1.1f, 4.4f, -8.8f )",     theTest );
    UnitTest::VerifyResult( vec3cStr == vec3cCorrectStr, "Vec3::GetAsString failed for ( 0.360f, 0.006f, 0.600f )", theTest );
}
