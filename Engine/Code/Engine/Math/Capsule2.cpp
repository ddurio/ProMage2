#include "Engine/Math/Capsule2.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"


Capsule2::Capsule2( const Vec2& startIn, const Vec2& endIn, float radiusIn ) :
    start(startIn),
    end(endIn),
    radius(radiusIn) {
}


Capsule2::Capsule2( const std::string& textIn ) {
    SetFromText( textIn );
}


void Capsule2::operator=( const Capsule2& copyFrom ) {
    start = copyFrom.start;
    end = copyFrom.end;
    radius = copyFrom.radius;
}


bool Capsule2::operator==( const Capsule2& compareTo ) const {
    return (start == compareTo.start &&
            end == compareTo.end &&
            radius == compareTo.radius);
}


bool Capsule2::operator!=( const Capsule2& compareTo ) const {
    return (start != compareTo.start ||
            end != compareTo.end ||
            radius != compareTo.radius);
}


void Capsule2::SetFromText( const std::string& textIn ) {
    Strings params = SplitStringOnDelimeter( textIn, ' ' );
    int numParams = (int)params.size();
    GUARANTEE_OR_DIE( numParams == 3, Stringf( "Invalid Capsule2 setFromText value (%s)", textIn.c_str() ) );

    start = Vec2( params[0] );
    end = Vec2( params[1] );
    radius = (float)atof( params[2].c_str() );
}


std::string Capsule2::GetAsString() const {
    std::string startStr = start.GetAsString();
    std::string endStr = end.GetAsString();

    std::string capsuleStr = Stringf( "%s %s %.3f", startStr.c_str(), endStr.c_str(), radius );
    return capsuleStr;
}


float Capsule2::GetLength() const {
    Vec2 lineDisp = end - start;
    return lineDisp.GetLength();
}


Vec2 Capsule2::GetCenter() const {
    return (start + end) * 0.5f;
}


Vec2 Capsule2::GetRight() const {
    Vec2 lineDisp = end - start;
    Vec2 rightDir = lineDisp.GetRotatedMinus90Degrees();

    return rightDir.GetNormalized();
}


Vec2 Capsule2::GetClosestPoint( const Vec2& point ) {
    Vec2 closestOnLine = GetClosestPointOnLineSegment( point, start, end );
    Vec2 displacement = point - closestOnLine;
    displacement.ClampLength( radius );

    return closestOnLine + displacement;
}


Capsule2 Capsule2::GetTranslated( const Vec2& translation ) {
    Capsule2 translated = *this;

    translated.start += translation;
    translated.end   += translation;

    return translated;
}


AABB2 Capsule2::GetBoundingAABB() const {
    float xMin = Min( start.x, end.x ) - radius;
    float xMax = Max( start.x, end.x ) + radius;

    float yMin = Min( start.y, end.y ) - radius;
    float yMax = Max( start.y, end.y ) + radius;

    return AABB2( Vec2( xMin, yMin ), Vec2( xMax, yMax ) );
}


OBB2 Capsule2::GetBoundingOBB() const {
    Vec2 center = GetCenter();
    Vec2 halfExtents = Vec2( radius, GetLength() );
    Vec2 right = GetRight();

    return OBB2( center, halfExtents, right );
}


bool Capsule2::IsPointInside( const Vec2& point ) const {
    Vec2 closestPoint = GetClosestPointOnLineSegment( point, start, end );
    return ((closestPoint - point).GetLengthSquared() < (radius * radius));
}


void Capsule2::Translate( const Vec2& translation ) {
    *this = GetTranslated( translation );
}


UNITTEST( "Basics", "Capsule2", 0 ) {
    Vec2 vec2a( 1.4f, 0.2f );
    Vec2 vec2b( -1.3f, 0.3f );

    Capsule2 cap2a;										        // default constructor
    Capsule2 cap2b( vec2a, vec2b, 1.f );					    // copy constructor
    Capsule2 cap2c = Capsule2( vec2b, vec2a, 10.f );	        // explicit constructor (per C++ standard)
    Capsule2 cap2d( cap2b );								    // copy constructor (per C++ standard)
    Capsule2 cap2e = cap2c;								        // copy constructor (per C++ standard)
    Capsule2 cap2bStr( "1.4,0.2 -1.3,0.3 1" );                  // explicit string constructor
    Capsule2 cap2cStr = Capsule2( "-1.3f,0.30f 1.4,0.2 10" );   // explicit string constructor (per C++ standard)

    UnitTest::VerifyResult( sizeof( Capsule2 ) == 20,                               "sizeof(Cap2) was not 20 bytes",                                                theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2a,    Vec2::ZERO, Vec2::ZERO, 0.f ), "Cap2( start, end, radius ) : explicit constructor failed to assign a value",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2b,    vec2a,      vec2b, 1.f ),      "Cap2( start, end, radius ) : explicit constructor failed to assign a value",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2c,    vec2b,      vec2a, 10.f ),     "Cap2( start, end, radius ) : explicit constructor failed to assign a value",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2d,    cap2b ),                       "Cap2( Cap2 ) : copy constructor failed to copy a value",                       theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2e,    cap2c ),                       "Cap2 = Cap2 : copy constructor failed to copy a value",                        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2bStr, cap2b ),                       "Vec2( string ) : string constructor failed to copy a value",                   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2cStr, cap2c ),                       "Vec2( string ) : string constructor failed to copy a value",                   theTest );

    // Basic arithmetic
    Capsule2 cap2f = Capsule2( Vec2::ZERO, cap2b.end, cap2b.radius );
    Capsule2 cap2g = Capsule2( cap2b.start, Vec2::ZERO, cap2b.radius );
    Capsule2 cap2h = Capsule2( cap2b.start, cap2b.end, 0.f );

    UnitTest::VerifyResult( cap2b == cap2d,     "Vec2::operator== : failed to detect match",            theTest );
    UnitTest::VerifyResult( !(cap2b != cap2d),  "Vec2::operator!= : failed to reject match",            theTest );

    UnitTest::VerifyResult( cap2b != cap2f,     "Vec2::operator!= : failed to detect start mismatch",   theTest );
    UnitTest::VerifyResult( cap2b != cap2g,     "Vec2::operator!= : failed to detect end mismatch",     theTest );
    UnitTest::VerifyResult( cap2b != cap2h,     "Vec2::operator!= : failed to detect radius mismatch",  theTest );

    UnitTest::VerifyResult( !(cap2b == cap2f),  "Vec2::operator== : failed to reject start mismatch",   theTest );
    UnitTest::VerifyResult( !(cap2b == cap2g),  "Vec2::operator== : failed to reject end mismatch",     theTest );
    UnitTest::VerifyResult( !(cap2b == cap2h),  "Vec2::operator== : failed to reject radius mismatch",  theTest );

    cap2a = cap2g;								// operator= (assignment operator)

    UnitTest::VerifyResult( IsMostlyEqual( cap2a, cap2g ), "Cap2::operator= : failed to copy correctly", theTest );
}


UNITTEST( "Methods", "Capsule2", 0 ) {
    Vec2 vec2a( 0.2f, 0.6f );
    Vec2 vec2b( -1.1f, -8.8f );

    Capsule2 cap2a( vec2a, vec2b, 1.f );
    Capsule2 cap2b( vec2b, vec2a, 10.f );

    // GetAsString
    std::string vec2aStr = vec2a.GetAsString();
    std::string vec2bStr = vec2b.GetAsString();

    std::string vec2aCorrectStr = "0.200,0.600";
    std::string vec2bCorrectStr = "-1.100,-8.800";

    std::string cap2aStr = cap2a.GetAsString();
    std::string cap2bStr = cap2b.GetAsString();

    std::string cap2aCorrectStr = Stringf( "%s %s 1.000", vec2aCorrectStr.c_str(), vec2bCorrectStr.c_str() );
    std::string cap2bCorrectStr = Stringf( "%s %s 10.000", vec2bCorrectStr.c_str(), vec2aCorrectStr.c_str() );

    UnitTest::VerifyResult( cap2aStr == cap2aCorrectStr, "Capsule3::GetAsString failed for ( 0.2f, 0.6f ), ( -1.1f, -8.8f ), 1.f",  theTest );
    UnitTest::VerifyResult( cap2bStr == cap2bCorrectStr, "Capsule3::GetAsString failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f", theTest );

    // GetCenter
    Vec2 cap2aCenter = cap2a.GetCenter();
    Vec2 cap2bCenter = cap2b.GetCenter();

    Vec2 cap2aCorrectCenter( -0.45f, -4.1f );
    Vec2 cap2bCorrectCenter( -0.45f, -4.1f );

    UnitTest::VerifyResult( IsMostlyEqual( cap2aCenter, cap2aCorrectCenter ), "Capsule3::GetCenter failed for ( 0.2f, 0.6f ), ( -1.1f, -8.8f ), 1.f",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2bCenter, cap2bCorrectCenter ), "Capsule3::GetCenter failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f", theTest );

    // GetClosestPoint & Contains
    Vec2 vec2c( -20.f, -20.f );         // Off of start
    Vec2 vec2d( -2.f, -9.f );           // Inside of start
    Vec2 vec2e( 20.f, 20.f );           // Off of end
    Vec2 vec2f( 0.5f, 1.f );            // Inside of end
    Vec2 vec2g( 10.f, -10.f );          // Off of center
    Vec2 vec2h = Vec2::ZERO;    // Inside of center

    Vec2 vec2cClosest = cap2b.GetClosestPoint( vec2c );   // Off of start
    Vec2 vec2dClosest = cap2b.GetClosestPoint( vec2d );   // Inside of start
    Vec2 vec2eClosest = cap2b.GetClosestPoint( vec2e );   // Off of end
    Vec2 vec2fClosest = cap2b.GetClosestPoint( vec2f );   // Inside of end
    Vec2 vec2gClosest = cap2b.GetClosestPoint( vec2g );   // Off of center
    Vec2 vec2hClosest = cap2b.GetClosestPoint( vec2h );   // Inside of center

    Vec2 vec2cCorrect( -9.70291534f, -13.8980239f );    // Off of start
    Vec2 vec2dCorrect( -2.f, -9.f );                    // Inside of start
    Vec2 vec2eCorrect( 7.34243143f, 7.59813989f );      // Off of end
    Vec2 vec2fCorrect( 0.5f, 1.f );                     // Inside of end
    Vec2 vec2gCorrect( 8.85143327f, -9.84115566f );     // Off of center
    Vec2 vec2hCorrect = Vec2::ZERO;             // Inside of center

    UnitTest::VerifyResult( IsMostlyEqual( vec2cClosest, vec2cCorrect ), "Capsule3::GetClosestPoint failed for point off of start",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec2dClosest, vec2dCorrect ), "Capsule3::GetClosestPoint failed for point inside of start",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec2eClosest, vec2eCorrect ), "Capsule3::GetClosestPoint failed for point off of end",       theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec2fClosest, vec2fCorrect ), "Capsule3::GetClosestPoint failed for point inside of end",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec2gClosest, vec2gCorrect ), "Capsule3::GetClosestPoint failed for point off of center",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec2hClosest, vec2hCorrect ), "Capsule3::GetClosestPoint failed for point inside of center", theTest );

    bool vec2cContained = cap2b.IsPointInside( vec2c );
    bool vec2dContained = cap2b.IsPointInside( vec2d );
    bool vec2eContained = cap2b.IsPointInside( vec2e );
    bool vec2fContained = cap2b.IsPointInside( vec2f );
    bool vec2gContained = cap2b.IsPointInside( vec2g );
    bool vec2hContained = cap2b.IsPointInside( vec2h );

    UnitTest::VerifyResult( !vec2cContained, "Capsule3::Contains( -20.f, -20.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f",    theTest );
    UnitTest::VerifyResult( vec2dContained,  "Capsule3::Contains( -2.f, -9.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f",      theTest );
    UnitTest::VerifyResult( !vec2eContained, "Capsule3::Contains( 20.f, 20.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f",      theTest );
    UnitTest::VerifyResult( vec2fContained,  "Capsule3::Contains( 0.5f, 1.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f",       theTest );
    UnitTest::VerifyResult( !vec2gContained, "Capsule3::Contains( 0.f, -1.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f",       theTest );
    UnitTest::VerifyResult( vec2hContained,  "Capsule3::Contains( 0.f, 0.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f",        theTest );

    // Translate & GetTranslated
    Vec2 vec2i( 0.f, 2.f );
    Vec2 vec2j( 3.f, 1.f );

    Vec2 cap2aTransStart = vec2a + vec2i;
    Vec2 cap2aTransEnd = vec2b + vec2i;
    Vec2 cap2bTransStart = vec2b + vec2j;
    Vec2 cap2bTransEnd = vec2a + vec2j;

    cap2a.Translate( vec2i );
    Capsule2 cap2bTrans = cap2b.GetTranslated( vec2j );

    UnitTest::VerifyResult( IsMostlyEqual( cap2a,       cap2aTransStart, cap2aTransEnd, 1.f ),  "Capsule3::Translate( 0.f, 2.f ) failed for ( 0.2f, 0.6f ), ( -1.1f, -8.8f ), 1.f",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2b,       vec2b,           vec2a,         10.f ), "Capsule3::GetTranslated modified the existing capsule",                                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap2bTrans,  cap2bTransStart, cap2bTransEnd, 10.f ), "Capsule3::GetTranslated( 3.f, 1.f ) failed for ( -1.1f, -8.8f ), ( 0.2f, 0.6f ), 10.f", theTest );
}
