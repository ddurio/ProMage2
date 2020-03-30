#include "Engine/Math/Vec4.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/MathUtils.hpp"


const Vec4 Vec4::ZERO = Vec4( 0.f, 0.f, 0.f, 0.f );
const Vec4 Vec4::ONE  = Vec4( 1.f, 1.f, 1.f, 1.f );


Vec4::Vec4( float inX, float inY, float inZ, float inW ) :
    x(inX),
    y(inY),
    z(inZ),
    w(inW) {
}


Vec4::Vec4( const std::string& initialValues ) {
    SetFromText( initialValues );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator + ( const Vec4& vecToAdd ) const {
	float tempX = this->x + vecToAdd.x;
	float tempY = this->y + vecToAdd.y;
	float tempZ = this->z + vecToAdd.z;
	float tempW = this->w + vecToAdd.w;
	return Vec4( tempX, tempY, tempZ, tempW );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-( const Vec4& vecToSubtract ) const {
	float tempX = this->x - vecToSubtract.x;
	float tempY = this->y - vecToSubtract.y;
	float tempZ = this->z - vecToSubtract.z;
	float tempW = this->w - vecToSubtract.w;
	return Vec4( tempX, tempY, tempZ, tempW );
}


const Vec4 Vec4::operator-() const {
    return (Vec4::ZERO - *this);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*( float uniformScale ) const {
	float tempX = this->x * uniformScale;
	float tempY = this->y * uniformScale;
	float tempZ = this->z * uniformScale;
	float tempW = this->w * uniformScale;
	return Vec4( tempX, tempY, tempZ, tempW );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator/( float inverseScale ) const {
	float tempX = this->x / inverseScale;
	float tempY = this->y / inverseScale;
	float tempZ = this->z / inverseScale;
	float tempW = this->w / inverseScale;
	return Vec4( tempX, tempY, tempZ, tempW );
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator+=( const Vec4& vecToAdd ) {
	this->x += vecToAdd.x;
	this->y += vecToAdd.y;
	this->z += vecToAdd.z;
	this->w += vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator-=( const Vec4& vecToSubtract ) {
	this->x -= vecToSubtract.x;
	this->y -= vecToSubtract.y;
	this->z -= vecToSubtract.z;
	this->w -= vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator*=( const float uniformScale ) {
	this->x *= uniformScale;
	this->y *= uniformScale;
	this->z *= uniformScale;
	this->w *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator/=( const float uniformDivisor ) {
	this->x /= uniformDivisor;
	this->y /= uniformDivisor;
	this->z /= uniformDivisor;
	this->w /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator=( const Vec4& copyFrom ) {
	this->x = copyFrom.x;
	this->y = copyFrom.y;
	this->z = copyFrom.z;
	this->w = copyFrom.w;
}


//-----------------------------------------------------------------------------------------------
const Vec4 operator*( float uniformScale, const Vec4& vecToScale ) {
    return vecToScale * uniformScale;
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator==( const Vec4& compare ) const {
	return (
        (this->x == compare.x) &&
        (this->y == compare.y) &&
        (this->z == compare.z) &&
        (this->w == compare.w) 
    );
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator!=( const Vec4& compare ) const {
	return (
        (this->x != compare.x) ||
        (this->y != compare.y) ||
        (this->z != compare.z) ||
        (this->w != compare.w) 
    );
}


const std::string Vec4::GetAsString() const {
    return Stringf( "%.3f,%.3f,%.3f,%.3f", x, y, z, w );
}


void Vec4::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ',' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 4, Stringf( "Invalid initialization string (%s) for Vec4", valuesAsText.c_str() ) );

    x = (float)atof( splitString[0].c_str() );
    y = (float)atof( splitString[1].c_str() );
    z = (float)atof( splitString[2].c_str() );
    w = (float)atof( splitString[3].c_str() );
}


UNITTEST( "Basics", "Vec4", 0 ) {
    Vec4 vec4a;										            // default constructor
    Vec4 vec4b( 1.4f, -1.6f, 0.2f, -0.4f );				        // explicit constructor
    Vec4 vec4c = Vec4( -1.3f, 1.7f, 0.3f, -0.6f );      // explicit constructor (per C++ standard)
    Vec4 vec4d( vec4b );							            // copy constructor
    Vec4 vec4e = vec4c;								            // copy constructor (per C++ standard)
    Vec4 vec4bStr( "1.4,-1.6,0.2,-0.4" );                       // explicit string constructor
    Vec4 vec4cStr = Vec4( "-1.3f,1.7f,0.30f,-0.6f" );   // explicit string constructor (per C++ standard)

    UnitTest::VerifyResult( sizeof( Vec4 ) == 16,                               "sizeof(Vec4) was not 16 bytes",                                              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4b, 1.4f, -1.6f, 0.2f, -0.4f ),   "Vec4( x, y, z, w ) : explicit constructor failed to assign x, y, and/or z",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4d, 1.4f, -1.6f, 0.2f, -0.4f ),   "Vec4( Vec4 ) : copy constructor failed to copy x, y, and/or z",              theTest );
    UnitTest::VerifyResult( vec4bStr == vec4b,                                  "Vec4( string ) : string constructor failed to copy x, y, z, and/or w",       theTest );
    UnitTest::VerifyResult( vec4cStr == vec4c,                                  "Vec4( string ) : string constructor failed to copy x, y, z, and/or w",       theTest );

    // Static constants
    UnitTest::VerifyResult( Vec4::ZERO.x == 0.f, "Vec4::ZERO : x is non-zero", theTest );
    UnitTest::VerifyResult( Vec4::ZERO.y == 0.f, "Vec4::ZERO : y is non-zero", theTest );
    UnitTest::VerifyResult( Vec4::ZERO.z == 0.f, "Vec4::ZERO : z is non-zero", theTest );
    UnitTest::VerifyResult( Vec4::ZERO.w == 0.f, "Vec4::ZERO : w is non-zero", theTest );
    UnitTest::VerifyResult( Vec4::ONE.x == 1.f,  "Vec4::ONE : x is non-one",   theTest );
    UnitTest::VerifyResult( Vec4::ONE.y == 1.f,  "Vec4::ONE : y is non-one",   theTest );
    UnitTest::VerifyResult( Vec4::ONE.z == 1.f,  "Vec4::ONE : z is non-one",   theTest );
    UnitTest::VerifyResult( Vec4::ONE.w == 1.f,  "Vec4::ONE : w is non-one",   theTest );

    // Basic arithmetic
    Vec4 vec4f( 3.5f, -1.5f, -7.1f, 0.3f );	// explicit constructor from x,y,z,w
    Vec4 vec4g = Vec4( -0.5f, 1.0f, 0.1f, 3.1f );	// explicit constructor from x,y,z,w

    Vec4 vec4h = vec4f + vec4g;		// operator+ (Vec4 + Vec4)
    Vec4 vec4i = vec4f + vec4f;		// operator+ (Vec4 + Vec4), added with itself
    Vec4 vec4j = vec4f - vec4g;		// operator- (Vec4 - Vec4)
    Vec4 vec4k = vec4f - vec4f;		// operator- (Vec4 - Vec4), subtract from itself
    Vec4 vec4l = vec4f * 0.2f;		// operator* (Vec4 * float)
    Vec4 vec4m = 0.2f * vec4g;		// operator* (float * Vec4)
    Vec4 vec4n = vec4f / 0.5f;		// operator/ (Vec4 / float)

    UnitTest::VerifyResult( IsMostlyEqual( vec4h, 3.0f, -0.5f, -7.f, 3.4f ),    "Vec4::operator+ : did not add (vec4 + vec4) correctly",                    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4i, 7.0f, -3.0f, -14.2f, 0.6f ),  "Vec4::operator+ : did not add (vec4 + vec4) with itself correctly",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4j, 4.0f, -2.5f, -7.2f, -2.8f ),  "Vec4::operator- : did not subtract (vec4 - vec4) correctly",               theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4k, 0.0f, 0.0f, 0.0f, 0.0f ),     "Vec4::operator- : did not subtract (vec4 - vec4) from itself correctly",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4l, 0.7f, -0.3f, -1.42f, 0.06f ), "Vec4::operator* : did not scale (vec4 * float) correctly",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4m, -0.1f, 0.2f, 0.02f, 0.62f ),  "Vec4::operator* : did not scale (float * vec4) correctly",                 theTest ); UnitTest::VerifyResult( IsMostlyEqual( vec4n, 7.0f, -3.0f, -14.2f, 0.6f ),  "Vec4::operator/ : did not divide (vec4 / float) correctly",                theTest );

    Vec4 vec4o( 3.f, 4.f, 5.f, 6.f );	// explicit constructor
    Vec4 vec4p( 3.f, 4.f, 5.f, 6.f );	// explicit constructor
    Vec4 vec4q( 99.f, 4.f, 5.f, 6.f );	// explicit constructor
    Vec4 vec4r( 3.f, 99.f, 5.f, 6.f );	// explicit constructor
    Vec4 vec4s( 3.f, 4.f, 99.f, 6.f );	// explicit constructor
    Vec4 vec4t( 3.f, 4.f, 5.f, 99.f );	// explicit constructor

    UnitTest::VerifyResult( vec4o == vec4p,     "Vec4::operator== : failed to detect match",      theTest );
    UnitTest::VerifyResult( !(vec4o != vec4p),  "Vec4::operator!= : failed to reject match",      theTest );

    UnitTest::VerifyResult( vec4o != vec4q,     "Vec4::operator!= : failed to detect x-mismatch", theTest );
    UnitTest::VerifyResult( vec4o != vec4r,     "Vec4::operator!= : failed to detect y-mismatch", theTest );
    UnitTest::VerifyResult( vec4o != vec4s,     "Vec4::operator!= : failed to detect z-mismatch", theTest );
    UnitTest::VerifyResult( vec4o != vec4t,     "Vec4::operator!= : failed to detect w-mismatch", theTest );

    UnitTest::VerifyResult( !(vec4o == vec4q),  "Vec4::operator== : failed to reject x-mismatch", theTest );
    UnitTest::VerifyResult( !(vec4o == vec4r),  "Vec4::operator== : failed to reject y-mismatch", theTest );
    UnitTest::VerifyResult( !(vec4o == vec4s),  "Vec4::operator== : failed to reject z-mismatch", theTest );
    UnitTest::VerifyResult( !(vec4o == vec4t),  "Vec4::operator== : failed to reject w-mismatch", theTest );

    Vec4 vec4u( 3.f, 4.f, 5.f, 6.f );	// explicit constructor
    Vec4 vec4v;							// default constructor
    vec4v = vec4u;						// operator= (assignment operator)
    vec4u += Vec4( 1.f, 2.f, 3.f, 4.f );// operator+= (add-and-assign operator)
    Vec4 vec4w( 3.f, 4.f, 5.f, 6.f );	// 
    vec4w -= Vec4( 2.f, 1.f, 3.f, 4.f );// operator-= (subtract-and-assign operator)
    Vec4 vec4x( 3.f, 4.f, 5.f, 6.f );	// 
    Vec4 vec4aa( 5.f, 7.f, 9.f, 11.f );	// 
    Vec4 vec4bb( 1.f, 2.f, 3.f, 4.f );	// 
    Vec4 vec4cc( 8.f, 6.f, 4.f, 2.f );	// 
    vec4x += vec4aa;					// 
    vec4aa -= vec4bb;					// 
    vec4bb *= 3.f;						// operator*= (uniform scale by float)
    vec4cc /= 2.f;						// operator/= (uniform scale by 1/float)

    UnitTest::VerifyResult( IsMostlyEqual( vec4v, 3.f, 4.f, 5.f, 6.f ),     "Vec4::operator= : failed to copy correctly",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4u, 4.f, 6.f, 8.f, 10.f ),    "Vec4::operator+= : failed to add correctly",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4w, 1.f, 3.f, 2.f, 2.f ),     "Vec4::operator-= : failed to subtract correctly",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4x, 8.f, 11.f, 14.f, 17.f ),  "Vec4::operator+= : failed to add existing Vec4",       theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4aa, 4.f, 5.f, 6.f, 7.f ),    "Vec4::operator-= : failed to subtract existing Vec4",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4bb, 3.f, 6.f, 9.f, 12.f ),   "Vec4::operator*= : failed to scale correctly",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec4cc, 4.f, 3.f, 2.f, 1.f ),    "Vec4::operator/= : failed to divide correctly",        theTest );
}


UNITTEST( "Methods", "Vec4", 0 ) {
    Vec4 vec4a( 0.2f, -0.3f, 0.6f, -0.9f );
    Vec4 vec4b( -1.1f, 4.4f, -8.8f, 2.2f );
    Vec4 vec4c = Vec4( 0.360501766f, 0.00620349636f, 0.600000024f, 0.123456789f );

    // GetAsString
    std::string vec4aStr = vec4a.GetAsString();
    std::string vec4bStr = vec4b.GetAsString();
    std::string vec4cStr = vec4c.GetAsString();

    std::string vec4aCorrectStr = "0.200,-0.300,0.600,-0.900";
    std::string vec4bCorrectStr = "-1.100,4.400,-8.800,2.200";
    std::string vec4cCorrectStr = "0.361,0.006,0.600,0.123";

    UnitTest::VerifyResult( vec4aStr == vec4aCorrectStr, "Vec4::GetAsString failed for ( 0.2f, -0.3f, 0.6f, -0.9 )",       theTest );
    UnitTest::VerifyResult( vec4bStr == vec4bCorrectStr, "Vec4::GetAsString failed for ( -1.1f, 4.4f, -8.8f, 2.2 )",       theTest );
    UnitTest::VerifyResult( vec4cStr == vec4cCorrectStr, "Vec4::GetAsString failed for ( 0.360f, 0.006f, 0.600f, 0.123 )", theTest );
}
