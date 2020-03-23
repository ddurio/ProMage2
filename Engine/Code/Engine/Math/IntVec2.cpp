#include "Engine/Math/IntVec2.hpp"

#include "Engine/Debug/UnitTests.hpp"
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


UNITTEST( "Constructors", "IntVec2", 0 ) {
    IntVec2 v1;								// default constructor
    IntVec2 v2( 1, -1 );					// explicit constructor
    IntVec2 v3  = IntVec2( -4, 2 );	        // explicit constructor (per C++ standard)
    IntVec2 v4( v2 );							// copy constructor
    IntVec2 v5 = v3;							// copy constructor (per C++ standard)
    IntVec2 v6( "-2,0" );                    // explicit string constructor
    IntVec2 v7  = IntVec2( "6,-7" );          // explicit string constructor (per C++ standard)

    UnitTest::VerifyResult( sizeof( Vec2 )  ==  8,   "sizeof(IntVec2) was not 8 bytes",                                     theTest );
    UnitTest::VerifyResult( v2.x            ==  1,   "IntVec2( x, y ) : explicit constructor failed to assign x",           theTest );
    UnitTest::VerifyResult( v2.y            == -1,   "IntVec2( x, y ) : explicit constructor failed to assign y",           theTest );
    UnitTest::VerifyResult( v3.x            == -4,   "IntVec2( x, y ) : explicit constructor failed to assign x",           theTest );
    UnitTest::VerifyResult( v3.y            ==  2,   "IntVec2( x, y ) : explicit constructor failed to assign y",           theTest );
    UnitTest::VerifyResult( v4.x            ==  1,   "IntVec2( intVec2 ) : copy constructor failed to copy x",              theTest );
    UnitTest::VerifyResult( v4.y            == -1,   "IntVec2( intVec2 ) : copy constructor failed to copy y",              theTest );
    UnitTest::VerifyResult( v5.x            == -4,   "IntVec2::operator= failed to assign x",                               theTest );
    UnitTest::VerifyResult( v5.y            ==  2,   "IntVec2::operator= failed to assign y",                               theTest );
    UnitTest::VerifyResult( v6.x            == -2,   "IntVec2( string ) : explicit string constructor failed to assign x",  theTest );
    UnitTest::VerifyResult( v6.y            ==  0,   "IntVec2( string ) : explicit string constructor failed to assign y",  theTest );
    UnitTest::VerifyResult( v7.x            ==  6,   "IntVec2( string ) : string constructor failed to assign x.f",         theTest );
    UnitTest::VerifyResult( v7.y            == -7,   "IntVec2( string ) : string constructor failed to assign y.f",         theTest );

    // Static constants
    UnitTest::VerifyResult( IntVec2::ZERO.x    ==  0,    "IntVec2::ZERO : x is non-zero",       theTest );
    UnitTest::VerifyResult( IntVec2::ZERO.y    ==  0,    "IntVec2::ZERO : y is non-zero",       theTest );
    UnitTest::VerifyResult( IntVec2::ONE.x     ==  1,    "IntVec2::ONE : x is non-one",         theTest );
    UnitTest::VerifyResult( IntVec2::ONE.y     ==  1,    "IntVec2::ONE : y is non-one",         theTest );
    UnitTest::VerifyResult( IntVec2::NEGONE.x  == -1,    "IntVec2::ONE : x is non-one",         theTest );
    UnitTest::VerifyResult( IntVec2::NEGONE.y  == -1,    "IntVec2::ONE : y is non-one",         theTest );
    UnitTest::VerifyResult( IntVec2::LEFT.x    == -1,    "IntVec2::LEFT : x is not neg one",    theTest );
    UnitTest::VerifyResult( IntVec2::LEFT.y    ==  0,    "IntVec2::LEFT : y is non-zero",       theTest );
    UnitTest::VerifyResult( IntVec2::RIGHT.x   ==  1,    "IntVec2::RIGHT : x is not one",       theTest );
    UnitTest::VerifyResult( IntVec2::RIGHT.y   ==  0,    "IntVec2::RIGHT : y is non-zero",      theTest );
    UnitTest::VerifyResult( IntVec2::UP.x      ==  0,    "IntVec2::UP : x is non-zero",         theTest );
    UnitTest::VerifyResult( IntVec2::UP.y      ==  1,    "IntVec2::UP : y is not one",          theTest );
    UnitTest::VerifyResult( IntVec2::DOWN.x    ==  0,    "IntVec2::DOWN : x is non-zero",       theTest );
    UnitTest::VerifyResult( IntVec2::DOWN.y    == -1,    "IntVec2::DOWN : y is not neg one",    theTest );
}


UNITTEST( "Temporary Operators", "IntVec2", 0 ) {
    IntVec2 v1( 3, -1 );					    // explicit constructor from x,y
    IntVec2 v2 = IntVec2( -5, 1 );	            // explicit constructor from x,y

    IntVec2 v3  = v1 + v2;						// operator+ (intVec2 + intVec2)
    IntVec2 v4  = v1 + v1;						// operator+ (intVec2 + intVec2), added with itself
    IntVec2 v5  = v1 - v2;						// operator- (intVec2 - intVec2)
    IntVec2 v6  = v1 - v1;						// operator- (intVec2 - intVec2), subtract from itself
    IntVec2 v7  = v1 * 2;					    // operator* (intVec2 * float)
    IntVec2 v8  = 2 * v1;					    // operator* (float * intVec2)
    IntVec2 v9  = v1 / 2;					    // operator/ (intVec2 / int)
    IntVec2 v10 = v1 * v2;                      // operator* (intVec2 * intVec2)
    Vec2    v11 = v1 * Vec2::NEGONE;            // operator* (intVec2 * intVec2)
    Vec2    v12 = v1 * 2.f;                     // operator/ (intVec2 * float)
    Vec2    v13 = v1 / 2.f;                     // operator/ (intVec2 / float)

    UnitTest::VerifyResult( v3.x  == -2,        "IntVec2::operator+ : did not add x in (intVec2 + intVec2) correctly",                  theTest );
    UnitTest::VerifyResult( v3.y  ==  0,        "IntVec2::operator+ : did not add y in (intVec2 + intVec2) correctly",                  theTest );
    UnitTest::VerifyResult( v4.x  ==  6,        "IntVec2::operator+ : did not add x in (intVec2 + intVec2) with itself correctly",      theTest );
    UnitTest::VerifyResult( v4.y  == -2,        "IntVec2::operator+ : did not add y in (intVec2 + intVec2) with itself correctly",      theTest );
    UnitTest::VerifyResult( v5.x  ==  8,        "IntVec2::operator- : did not subtract x in (intVec2 - intVec2) correctly",             theTest );
    UnitTest::VerifyResult( v5.y  == -2,        "IntVec2::operator- : did not subtract y in (intVec2 - intVec2) correctly",             theTest );
    UnitTest::VerifyResult( v6.x  ==  0,        "IntVec2::operator- : did not subtract x in (intVec2 - intVec2) from itself correctly", theTest );
    UnitTest::VerifyResult( v6.y  ==  0,        "IntVec2::operator- : did not subtract y in (intVec2 - intVec2) from itself correctly", theTest );
    UnitTest::VerifyResult( v7.x  ==  6,        "IntVec2::operator* : did not scale x in (intVec2 * float) correctly",                  theTest );
    UnitTest::VerifyResult( v7.y  == -2,        "IntVec2::operator* : did not scale y in (intVec2 * float) correctly",                  theTest );
    UnitTest::VerifyResult( v8.x  ==  6,        "IntVec2::operator* : did not scale x in (float * intVec2) correctly",                  theTest );
    UnitTest::VerifyResult( v8.y  == -2,        "IntVec2::operator* : did not scale y in (float * intVec2) correctly",                  theTest );
    UnitTest::VerifyResult( v9.x  ==  1,        "IntVec2::operator/ : did not divide x in (intVec2 / int) correctly",                   theTest );
    UnitTest::VerifyResult( v9.y  ==  0,        "IntVec2::operator/ : did not divide y in (intVec2 / int) correctly",                   theTest );
    UnitTest::VerifyResult( v10.x == -15,       "IntVec2::operator* : did not scale x in (intVec2 * intVec2) correctly",                theTest );
    UnitTest::VerifyResult( v10.y == -1,        "IntVec2::operator* : did not scale y in (intVec2 * intVec2) correctly",                theTest );
    UnitTest::VerifyResult( v11.x == -3.f,      "IntVec2::operator* : did not scale x in (intVec2 * vec2) correctly",                   theTest );
    UnitTest::VerifyResult( v11.y ==  1.f,      "IntVec2::operator* : did not scale y in (intVec2 * vec2) correctly",                   theTest );
    UnitTest::VerifyResult( v12.x ==  6.f,      "IntVec2::operator/ : did not scale x in (intVec2 * float) correctly",                  theTest );
    UnitTest::VerifyResult( v12.y == -2.f,      "IntVec2::operator/ : did not scale y in (intVec2 * float) correctly",                  theTest );
    UnitTest::VerifyResult( v13.x ==  1.5f,     "IntVec2::operator/ : did not scale x in (intVec2 / float) correctly",                  theTest );
    UnitTest::VerifyResult( v13.y == -0.5f,     "IntVec2::operator/ : did not scale y in (intVec2 / float) correctly",                  theTest );
}


UNITTEST( "Comparison Operators", "IntVec2", 0 ) {
    IntVec2 v1( 3, 4 );					// explicit constructor
    IntVec2 v2( 3, 4 );					// explicit constructor
    IntVec2 v3( 3, 99 );					// explicit constructor
    IntVec2 v4( 99, 4 );					// explicit constructor
    IntVec2 v5( 3, 3 );					// explicit constructor
    IntVec2 v6( 4, 4 );					// explicit constructor
    IntVec2 v7( 4, 3 );					// explicit constructor

    UnitTest::VerifyResult( v1 == v2,       "IntVec2::operator== : failed to detect match",                   theTest );
    UnitTest::VerifyResult( !(v1 != v2),    "IntVec2::operator!= : failed to reject match",                   theTest );

    UnitTest::VerifyResult( v1 != v3,       "IntVec2::operator!= : failed to detect y-mismatch",              theTest );
    UnitTest::VerifyResult( v1 != v4,       "IntVec2::operator!= : failed to detect x-mismatch",              theTest );
    UnitTest::VerifyResult( v1 != v5,       "IntVec2::operator!= : failed to detect y-mismatch coincidence",  theTest );
    UnitTest::VerifyResult( v1 != v6,       "IntVec2::operator!= : failed to detect x-mismatch coincidence",  theTest );
    UnitTest::VerifyResult( v1 != v7,       "IntVec2::operator!= : failed to detect x/y mixup",               theTest );

    UnitTest::VerifyResult( !(v1 == v3),    "IntVec2::operator== : failed to reject y-mismatch",              theTest );
    UnitTest::VerifyResult( !(v1 == v4),    "IntVec2::operator== : failed to reject x-mismatch",              theTest );
    UnitTest::VerifyResult( !(v1 == v5),    "IntVec2::operator== : failed to reject y-mismatch coincidence",  theTest );
    UnitTest::VerifyResult( !(v1 == v6),    "IntVec2::operator== : failed to reject x-mismatch coincidence",  theTest );
    UnitTest::VerifyResult( !(v1 == v7),    "IntVec2::operator== : failed to reject x/y mixup",               theTest );
}


UNITTEST( "Assignment Operators", "IntVec2", 0 ) {
    IntVec2 v1( 3, 4 );					// explicit constructor
    IntVec2 v2;								// default constructor
    v2 = v1;										// operator= (assignment operator)
    v1 += IntVec2( 1, 2 );					// operator+= (add-and-assign operator)
    IntVec2 v3( 3, 4 );					// 
    v3 -= IntVec2( 2, 1 );					// operator-= (subtract-and-assign operator)
    IntVec2 v4( 3, 4 );					// 
    IntVec2 v5( 5, 7 );					// 
    IntVec2 v6( 1, 2 );					// 
    IntVec2 v7( 8, 6 );					// 
    v4 += v5;										// 
    v5 -= v6;										// 
    v6 *= 3;										// operator*= (uniform scale by float)
    v7 /= 2;										// operator/= (uniform scale by 1/float)

    UnitTest::VerifyResult( v2.x == 3,  "IntVec2::operator= : failed to copy x",                      theTest );
    UnitTest::VerifyResult( v2.y == 4,  "IntVec2::operator= : failed to copy y",                      theTest );
    UnitTest::VerifyResult( v1.x == 4,  "IntVec2::operator+= : failed to add x",                      theTest );
    UnitTest::VerifyResult( v1.y == 6,  "IntVec2::operator+= : failed to add y",                      theTest );
    UnitTest::VerifyResult( v3.x == 1,  "IntVec2::operator-= : failed to subtract x",                 theTest );
    UnitTest::VerifyResult( v3.y == 3,  "IntVec2::operator-= : failed to subtract y",                 theTest );
    UnitTest::VerifyResult( v4.x == 8,  "IntVec2::operator+= : failed to add existing vec2's x",      theTest );
    UnitTest::VerifyResult( v4.y == 11, "IntVec2::operator+= : failed to add existing vec2's y",      theTest );
    UnitTest::VerifyResult( v5.x == 4,  "IntVec2::operator-= : failed to subtract existing vec2's x", theTest );
    UnitTest::VerifyResult( v5.y == 5,  "IntVec2::operator-= : failed to subtract existing vec2's y", theTest );
    UnitTest::VerifyResult( v6.x == 3,  "IntVec2::operator*= : failed to scale x",                    theTest );
    UnitTest::VerifyResult( v6.y == 6,  "IntVec2::operator*= : failed to scale y",                    theTest );
    UnitTest::VerifyResult( v7.x == 4,  "IntVec2::operator/= : failed to divide x",                   theTest );
    UnitTest::VerifyResult( v7.y == 3,  "IntVec2::operator/= : failed to divide y",                   theTest );
}


UNITTEST( "Methods", "IntVec2", 0 ) {
    // IntVec2::GetAsString methods
    IntVec2 a(  4,  3 );
    IntVec2 b(  8, -6 );
    IntVec2 c( -6, -8 );
    IntVec2 d(  0,  1 );

    std::string aStr = a.GetAsString();
    std::string bStr = b.GetAsString();
    std::string cStr = c.GetAsString();
    std::string dStr = d.GetAsString();

    std::string aCorrectStr = "4,3";
    std::string bCorrectStr = "8,-6";
    std::string cCorrectStr = "-6,-8";
    std::string dCorrectStr = "0,1";

    UnitTest::VerifyResult( aStr == aCorrectStr, "IntVec2::GetAsString failed for (4, 3)",   theTest );
    UnitTest::VerifyResult( bStr == bCorrectStr, "IntVec2::GetAsString failed for (8, -6)",  theTest );
    UnitTest::VerifyResult( cStr == cCorrectStr, "IntVec2::GetAsString failed for (-6, -8)", theTest );
    UnitTest::VerifyResult( dStr == dCorrectStr, "IntVec2::GetAsString failed for (0, 1)",   theTest );

    // IntVec2::GetRotated methods
    IntVec2 aRotated90     = a.GetRotated90Degrees();
    IntVec2 cRotated90     = c.GetRotated90Degrees();
    IntVec2 bRotatedNeg90  = b.GetRotatedMinus90Degrees();
    IntVec2 cRotatedNeg90  = c.GetRotatedMinus90Degrees();

    float aRot90CorrectX        = -3;		    float aRot90CorrectY        =  4;
    float cRot90CorrectX        =  8;		    float cRot90CorrectY        = -6;
    float bRotNeg90CorrectX     = -6;		    float bRotNeg90CorrectY     = -8;
    float cRotNeg90CorrectX     = -8;		    float cRotNeg90CorrectY     =  6;

    UnitTest::VerifyResult( aRotated90.x    == aRot90CorrectX,      "Vec2::GetRotated90Degrees().x was incorrect",        theTest );
    UnitTest::VerifyResult( aRotated90.y    == aRot90CorrectY,      "Vec2::GetRotated90Degrees().y was incorrect",        theTest );
    UnitTest::VerifyResult( cRotated90.x    == cRot90CorrectX,      "Vec2::GetRotated90Degrees().x was incorrect",        theTest );
    UnitTest::VerifyResult( cRotated90.y    == cRot90CorrectY,      "Vec2::GetRotated90Degrees().y was incorrect",        theTest );
    UnitTest::VerifyResult( bRotatedNeg90.x == bRotNeg90CorrectX,   "Vec2::GetRotatedMinus90Degrees().x was incorrect",   theTest );
    UnitTest::VerifyResult( bRotatedNeg90.y == bRotNeg90CorrectY,   "Vec2::GetRotatedMinus90Degrees().y was incorrect",   theTest );
    UnitTest::VerifyResult( cRotatedNeg90.x == cRotNeg90CorrectX,   "Vec2::GetRotatedMinus90Degrees().x was incorrect",   theTest );
    UnitTest::VerifyResult( cRotatedNeg90.y == cRotNeg90CorrectY,   "Vec2::GetRotatedMinus90Degrees().y was incorrect",   theTest );
}



