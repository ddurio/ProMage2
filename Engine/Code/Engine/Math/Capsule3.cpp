#include "Engine/Math/Capsule3.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB3.hpp"


Capsule3::Capsule3( const Vec3& startIn, const Vec3& endIn, float radiusIn ) :
    start(startIn),
    end(endIn),
    radius(radiusIn) {
}


Capsule3::Capsule3( const std::string& textIn ) {
    SetFromText( textIn );
}


void Capsule3::operator=( const Capsule3& copyFrom ) {
    start = copyFrom.start;
    end = copyFrom.end;
    radius = copyFrom.radius;
}


bool Capsule3::operator==( const Capsule3& compareTo ) const {
    return (start == compareTo.start &&
            end == compareTo.end &&
            radius == compareTo.radius);
}


bool Capsule3::operator!=( const Capsule3& compareTo ) const {
    return (start != compareTo.start ||
            end != compareTo.end ||
            radius != compareTo.radius);
}


void Capsule3::SetFromText( const std::string& textIn ) {
    Strings params = SplitStringOnDelimeter( textIn, ' ' );
    int numParams = (int)params.size();
    GUARANTEE_OR_DIE( numParams == 3, Stringf( "Invalid Capsule3 setFromText value (%s)", textIn.c_str() ) );

    start = Vec3( params[0] );
    end = Vec3( params[1] );
    radius = (float)atof( params[2].c_str() );
}


std::string Capsule3::GetAsString() const {
    std::string startStr = start.GetAsString();
    std::string endStr = end.GetAsString();

    std::string capsuleStr = Stringf( "%s %s %.3f", startStr.c_str(), endStr.c_str(), radius );
    return capsuleStr;
}


Vec3 Capsule3::GetCenter() const {
    return (start + end) * 0.5f;
}


Vec3 Capsule3::GetClosestPoint( const Vec3& point ) {
    Vec3 closestOnLine = GetClosestPointOnLineSegment( point, start, end );
    Vec3 displacement = point - closestOnLine;
    displacement.ClampLength( radius );

    return closestOnLine + displacement;
}


Capsule3 Capsule3::GetTranslated( const Vec3& translation ) {
    Capsule3 translated = *this;

    translated.start += translation;
    translated.end   += translation;

    return translated;
}


/*
AABB3 Capsule3::GetBoundingAABB() const {
    float xMin = Min( start.x, end.x ) - radius;
    float xMax = Max( start.x, end.x ) + radius;

    float yMin = Min( start.y, end.y ) - radius;
    float yMax = Max( start.y, end.y ) + radius;

    return AABB3( Vec3( xMin, yMin ), Vec3( xMax, yMax ) );
}
*/


bool Capsule3::IsPointInside( const Vec3& point ) const {
    Vec3 closestPoint = GetClosestPointOnLineSegment( point, start, end );
    return ((closestPoint - point).GetLengthSquared() < (radius * radius));
}


void Capsule3::Translate( const Vec3& translation ) {
    *this = GetTranslated( translation );
}


UNITTEST( "Basics", "Capsule3", 0 ) {
    Vec3 vec3a( 1.4f, -1.6f, 0.2f );
    Vec3 vec3b( -1.3f, 1.7f, 0.3f );

    Capsule3 cap3a;										    // default constructor
    Capsule3 cap3b( vec3a, vec3b, 1.f );					    // copy constructor
    Capsule3 cap3c = Capsule3( vec3b, vec3a, 10.f );	    // explicit constructor (per C++ standard)
    Capsule3 cap3d( cap3b );								    // copy constructor (per C++ standard)
    Capsule3 cap3e = cap3c;								    // copy constructor (per C++ standard)
    Capsule3 cap3bStr( "1.4,-1.6,0.2 -1.3,1.7,0.3 1" );                    // explicit string constructor
    Capsule3 cap3cStr = Capsule3( "-1.3f,1.7f,0.30f 1.4,-1.6,0.2 10" ); // explicit string constructor (per C++ standard)

    UnitTest::VerifyResult( sizeof( Capsule3 ) == 28,                               "sizeof(Cap3) was not 28 bytes",                                              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3a, Vec3::ZERO, Vec3::ZERO, 0.f ),    "Cap3( start, end, radius ) : explicit constructor failed to assign a value", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3b, vec3a, vec3b, 1.f ),              "Cap3( start, end, radius ) : explicit constructor failed to assign a value", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3c, vec3b, vec3a, 10.f ),             "Cap3( start, end, radius ) : explicit constructor failed to assign a value", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3d, cap3b ),                          "Cap3( Cap3 ) : copy constructor failed to copy a value",                     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3e, cap3c ),                          "Cap3 = Cap3 : copy constructor failed to copy a value",                      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3bStr, cap3b ),                       "Vec3( string ) : string constructor failed to copy a value",                 theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3cStr, cap3c ),                       "Vec3( string ) : string constructor failed to copy a value",                 theTest );

    // Basic arithmetic
    Capsule3 cap3f = Capsule3( Vec3::ZERO, cap3b.end, cap3b.radius );
    Capsule3 cap3g = Capsule3( cap3b.start, Vec3::ZERO, cap3b.radius );
    Capsule3 cap3h = Capsule3( cap3b.start, cap3b.end, 0.f );

    UnitTest::VerifyResult( cap3b == cap3d,     "Vec3::operator== : failed to detect match",            theTest );
    UnitTest::VerifyResult( !(cap3b != cap3d),  "Vec3::operator!= : failed to reject match",            theTest );

    UnitTest::VerifyResult( cap3b != cap3f,     "Vec3::operator!= : failed to detect start mismatch",   theTest );
    UnitTest::VerifyResult( cap3b != cap3g,     "Vec3::operator!= : failed to detect end mismatch",     theTest );
    UnitTest::VerifyResult( cap3b != cap3h,     "Vec3::operator!= : failed to detect radius mismatch",  theTest );

    UnitTest::VerifyResult( !(cap3b == cap3f),  "Vec3::operator== : failed to reject start mismatch",   theTest );
    UnitTest::VerifyResult( !(cap3b == cap3g),  "Vec3::operator== : failed to reject end mismatch",     theTest );
    UnitTest::VerifyResult( !(cap3b == cap3h),  "Vec3::operator== : failed to reject radius mismatch",  theTest );

    cap3a = cap3g;								// operator= (assignment operator)

    UnitTest::VerifyResult( IsMostlyEqual( cap3a, cap3g ), "Cap3::operator= : failed to copy correctly", theTest );
}


UNITTEST( "Methods", "Capsule3", 0 ) {
    Vec3 vec3a( 0.2f, -0.3f, 0.6f );
    Vec3 vec3b( -1.1f, 4.4f, -8.8f );

    Capsule3 cap3a( vec3a, vec3b, 1.f );
    Capsule3 cap3b( vec3b, vec3a, 10.f );

    // GetAsString
    std::string vec3aStr = vec3a.GetAsString();
    std::string vec3bStr = vec3b.GetAsString();

    std::string vec3aCorrectStr = "0.200,-0.300,0.600";
    std::string vec3bCorrectStr = "-1.100,4.400,-8.800";

    std::string cap3aStr = cap3a.GetAsString();
    std::string cap3bStr = cap3b.GetAsString();

    std::string cap3aCorrectStr = Stringf( "%s %s 1.000", vec3aCorrectStr.c_str(), vec3bCorrectStr.c_str() );
    std::string cap3bCorrectStr = Stringf( "%s %s 10.000", vec3bCorrectStr.c_str(), vec3aCorrectStr.c_str() );

    UnitTest::VerifyResult( cap3aStr == cap3aCorrectStr, "Capsule3::GetAsString failed for ( 0.2f, -0.3f, 0.6f ), ( -1.1f, 4.4f, -8.8f ), 1.f",  theTest );
    UnitTest::VerifyResult( cap3bStr == cap3bCorrectStr, "Capsule3::GetAsString failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f", theTest );

    // GetCenter
    Vec3 cap3aCenter = cap3a.GetCenter();
    Vec3 cap3bCenter = cap3b.GetCenter();

    Vec3 cap3aCorrectCenter( -0.45f, 2.05f, -4.1f );
    Vec3 cap3bCorrectCenter( -0.45f, 2.05f, -4.1f );

    UnitTest::VerifyResult( IsMostlyEqual( cap3aCenter, cap3aCorrectCenter ), "Capsule3::GetCenter failed for ( 0.2f, -0.3f, 0.6f ), ( -1.1f, 4.4f, -8.8f ), 1.f",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3bCenter, cap3bCorrectCenter ), "Capsule3::GetCenter failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f", theTest );

    // GetClosestPoint & Contains
    Vec3 vec3c( -20.f, 0.f, -20.f );    // Off of start
    Vec3 vec3d( -2.f, 4.4f, -9.f );     // Inside of start
    Vec3 vec3e( 20.f, 0.f, 20.f );      // Off of end
    Vec3 vec3f( 0.5f, -0.5f, 1.f );     // Inside of end
    Vec3 vec3g( 0.f, 20.f, -1.f );      // Off of center
    Vec3 vec3h = Vec3::ZERO;    // Inside of center

    Vec3 vec3cClosest = cap3b.GetClosestPoint( vec3c );   // Off of start
    Vec3 vec3dClosest = cap3b.GetClosestPoint( vec3d );   // Inside of start
    Vec3 vec3eClosest = cap3b.GetClosestPoint( vec3e );   // Off of end
    Vec3 vec3fClosest = cap3b.GetClosestPoint( vec3f );   // Inside of end
    Vec3 vec3gClosest = cap3b.GetClosestPoint( vec3g );   // Off of center
    Vec3 vec3hClosest = cap3b.GetClosestPoint( vec3h );   // Inside of center

    Vec3 vec3cCorrect( -9.53539881f, 2.43620345f, -13.79875985f );    // Off of start
    Vec3 vec3dCorrect( -2.f, 4.4f, -9.f );     // Inside of start
    Vec3 vec3eCorrect( 7.34243143f, -0.19178134f, 7.59813989f );      // Off of end
    Vec3 vec3fCorrect( 0.5f, -0.5f, 1.f );     // Inside of end
    Vec3 vec3gCorrect( -0.46345462f, 13.30116482f, -4.28532280f );      // Off of center
    Vec3 vec3hCorrect = Vec3::ZERO;    // Inside of center

    UnitTest::VerifyResult( IsMostlyEqual( vec3cClosest, vec3cCorrect ), "Capsule3::GetClosestPoint failed for point off of start",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3dClosest, vec3dCorrect ), "Capsule3::GetClosestPoint failed for point inside of start",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3eClosest, vec3eCorrect ), "Capsule3::GetClosestPoint failed for point off of end",       theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3fClosest, vec3fCorrect ), "Capsule3::GetClosestPoint failed for point inside of end",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3gClosest, vec3gCorrect ), "Capsule3::GetClosestPoint failed for point off of center",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( vec3hClosest, vec3hCorrect ), "Capsule3::GetClosestPoint failed for point inside of center", theTest );

    bool vec3cContained = cap3b.IsPointInside( vec3c );
    bool vec3dContained = cap3b.IsPointInside( vec3d );
    bool vec3eContained = cap3b.IsPointInside( vec3e );
    bool vec3fContained = cap3b.IsPointInside( vec3f );
    bool vec3gContained = cap3b.IsPointInside( vec3g );
    bool vec3hContained = cap3b.IsPointInside( vec3h );

    UnitTest::VerifyResult( !vec3cContained, "Capsule3::Contains( -20.f, 0.f, -20.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f", theTest );
    UnitTest::VerifyResult( vec3dContained, "Capsule3::Contains( -2.f, 4.4f, -9.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f",   theTest );
    UnitTest::VerifyResult( !vec3eContained, "Capsule3::Contains( 20.f, 0.f, 20.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f",   theTest );
    UnitTest::VerifyResult( vec3fContained, "Capsule3::Contains( 0.5f, -0.5f, 1.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f",   theTest );
    UnitTest::VerifyResult( !vec3gContained, "Capsule3::Contains( 0.f, 20.f, -1.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f",   theTest );
    UnitTest::VerifyResult( vec3hContained, "Capsule3::Contains( 0.f, 0.f, 0.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f",      theTest );

    // Translate & GetTranslated
    Vec3 vec3i( 0.f, 1.f, 2.f );
    Vec3 vec3j( 3.f, 2.f, 1.f );

    Vec3 cap3aTransStart = vec3a + vec3i;
    Vec3 cap3aTransEnd = vec3b + vec3i;
    Vec3 cap3bTransStart = vec3b + vec3j;
    Vec3 cap3bTransEnd = vec3a + vec3j;

    cap3a.Translate( vec3i );
    Capsule3 cap3bTrans = cap3b.GetTranslated( vec3j );

    UnitTest::VerifyResult( IsMostlyEqual( cap3a,      cap3aTransStart, cap3aTransEnd, 1.f ),  "Capsule3::Translate( 0.f, 1.f, 2.f ) failed for ( 0.2f, -0.3f, 0.6f ), ( -1.1f, 4.4f, -8.8f ), 1.f",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3b,      vec3b,           vec3a,         10.f ), "Capsule3::GetTranslated modified the existing capsule",                                                     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( cap3bTrans, cap3bTransStart, cap3bTransEnd, 10.f ), "Capsule3::GetTranslated( 3.f, 2.f, 1.f ) failed for ( -1.1f, 4.4f, -8.8f ), ( 0.2f, -0.3f, 0.6f ), 10.f",   theTest );
}
