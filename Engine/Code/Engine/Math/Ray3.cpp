#include "Engine/Math/Ray3.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/Vec2.hpp"


Ray3::Ray3( const Vec3& startIn /*= Vec3::ZERO*/, const Vec3& directionIn /*= -Vec3::SCREEN */ ) :
    start( startIn ),
    direction( directionIn.GetNormalized() ) {
}


Ray3::Ray3( const std::string& valuesAsText ) {
    SetFromText( valuesAsText );
}


Ray3 Ray3::MakeFromPoints( const Vec3& start, const Vec3& end ) {
    Vec3 dir = end - start;
    return Ray3( start, dir );
}


void Ray3::SetFromText( const std::string& valuesAsText ) {
    // Expects "startX,startY,startZ dirX,dirY,dirZ"
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ' ' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 2, Stringf( "(Ray3) Invalid initialization string (%s)", valuesAsText.c_str() ) );

    start = Vec3( splitString[0] );
    direction = Vec3( splitString[1] ).GetNormalized();
}


std::string Ray3::GetAsString() const {
    std::string startStr = start.GetAsString();
    std::string dirStr = direction.GetAsString();
    std::string rayStr = Stringf( "%s %s", startStr.c_str(), dirStr.c_str() );

    return rayStr;
}


Vec3 Ray3::GetPoint( float distanceAlong ) const {
    return start + (direction * distanceAlong);
}


int Ray3::Raycast( const Vec3& sphereCenter, float sphereRadius, float* outIntersections ) const {
    float radiusSqr = sphereRadius * sphereRadius;
    Vec3 rayStartToCenter = sphereCenter - start;

    if( direction == Vec3::ZERO ) { // A ray without direction can only impact at it's start
        float distFromSphereSqr = rayStartToCenter.GetLengthSquared();

        if( distFromSphereSqr == radiusSqr ) {
            outIntersections[0] = 0.f;
            return 1;
        }

        return 0;
    }

    // Otherwise redoing closest point logic to maintain dotProduct result
    float rayClosestDist = DotProduct( rayStartToCenter, direction );
    Vec3 closestPoint = start + (rayClosestDist * direction);

    float distFromSphereSqr = (closestPoint - sphereCenter).GetLengthSquared();

    if( distFromSphereSqr > radiusSqr ) { // No intersection
        return 0;
    } else if( distFromSphereSqr == radiusSqr ) { // Precisely one intersection
        outIntersections[0] = rayClosestDist;
        return 1;
    }

    float offsetToRadius = sqrt( radiusSqr + distFromSphereSqr );

    outIntersections[0] = rayClosestDist - offsetToRadius;
    outIntersections[1] = rayClosestDist + offsetToRadius;
    return 2;
}


int Ray3::Raycast( const Capsule3& capsule, float* outIntersections ) const {
    if( direction == Vec3::ZERO ) {
        if( capsule.IsPointInside( start ) ) {
            outIntersections[0] = 0.f;
            return 1;
        }

        return 0;
    }

    // Check bounding sphere first
    float capLength = (capsule.end - capsule.start).GetLength();
    float radius = capLength + capsule.radius;
    Vec3 capCenter = capsule.GetCenter();

    float boundsInter[2] = { 0.f };
    int numBoundsInter = Raycast( capCenter, radius, boundsInter );

    if( numBoundsInter == 0 ) {
        return 0;
    }

    // General check passed -- do specifics
    float cylinderHits[2] = { 0.f };
    int numCylinderHits = Raycast( capsule.start, capsule.end, capsule.radius, cylinderHits );


    float startHits[2] = { 0.f };
    int numStartHits = Raycast( capsule.start, capsule.radius, startHits );

    float endHits[2] = { 0.f };
    int numEndHits = Raycast( capsule.end, capsule.radius, endHits );

    float lowestHit = std::numeric_limits<float>::infinity();
    float highestHit = -std::numeric_limits<float>::infinity();

    if( numCylinderHits > 0 ) {
        lowestHit = Min( lowestHit, cylinderHits[0] );
        highestHit = Max( highestHit, cylinderHits[0] );
    
        if( numCylinderHits > 1 ) {
            lowestHit = Min( lowestHit, cylinderHits[1] );
            highestHit = Max( highestHit, cylinderHits[1] );
        }
    }
    
    if( numStartHits > 0 ) {
        lowestHit = Min( lowestHit, startHits[0] );
        highestHit = Max( highestHit, startHits[0] );
    
        if( numStartHits > 1 ) {
            lowestHit = Min( lowestHit, startHits[1] );
            highestHit = Max( highestHit, startHits[1] );
        }
    }
    
    if( numEndHits > 0 ) {
        lowestHit = Min( lowestHit, endHits[0] );
        highestHit = Max( highestHit, endHits[0] );
    
        if( numEndHits > 1 ) {
            lowestHit = Min( lowestHit, endHits[1] );
            highestHit = Max( highestHit, endHits[1] );
        }
    }

    int numHits = Min( numCylinderHits + numStartHits + numEndHits, 2 );

    if( numHits > 0 ) {
        outIntersections[0] = lowestHit;
    }

    if( numHits > 1 ) {
        outIntersections[1] = highestHit;
    }

    return numHits;
}


int Ray3::Raycast( const Plane3& plane, float* outIntersections ) const {
    float dotDirections = DotProduct( direction, plane.normal );

    if( abs(dotDirections) < 0.001f  ) { // Perpendicular or direction == ZERO
        if( abs(plane.GetDistanceFromPlane( start )) <= 0.001f ) {
            outIntersections[0] = 0.f;

            if( direction == Vec3::ZERO ) {
                return 1;
            }

            outIntersections[1] = 1.f;
            return 2; // Really infinite...
        }

        return 0;
    }

    float pDotN = DotProduct( start, plane.normal );
    outIntersections[0] = (plane.distance - pDotN) / dotDirections;
    return 1;
}


int Ray3::Raycast( const Vec3& cylinderStart, const Vec3& cylinderEnd, float radius, float* outIntersections ) const {
    Vec3 cylDirection = (cylinderEnd - cylinderStart).GetNormalized();
    Vec3 rayToCylinder = (cylinderStart - start);

    float dotDirections = DotProduct( direction, cylDirection );

    if( abs(dotDirections) >= 0.999f ) {
        // They are essentially parallel
        return 0;
    }

    Vec3 subA = (dotDirections * cylDirection) - direction;
    Vec3 subB = (DotProduct( -rayToCylinder, cylDirection ) * cylDirection ) + cylinderStart - start;

    float aDotA = DotProduct( subA, subA );
    float aDotB = DotProduct( subA, subB );
    float bDotB = DotProduct( subB, subB );

    float quadA = aDotA;
    float quadB = 2.f * aDotB;
    float quadC = bDotB - (radius * radius);

    float underRoot = (quadB * quadB) - (4.f * quadA * quadC);
    float allOver = 2.f * quadA;

    if( underRoot < 0.f ) {
        return 0;
    }

    int numHits = 0;
    float inters[2] = { 0.f };
    inters[0] = (-quadB - sqrt( underRoot )) / allOver;

    Vec3 hitPos = GetPoint( inters[0] );
    Vec3 startToHit = hitPos - cylinderStart;
    Vec3 endToHit = hitPos - cylinderEnd;

    float startDot = DotProduct( startToHit, cylDirection );
    float endDot = DotProduct( endToHit, cylDirection );

    if( startDot * endDot < 0.f ) {
        numHits++;
        outIntersections[0] = inters[0];
    }

    if( underRoot < 0.001f ) {
        // Essentially zero, only one root?
        return numHits;
    }

    inters[1] = (-quadB + sqrt( underRoot )) / allOver;

    hitPos = GetPoint( inters[1] );
    startToHit = hitPos - cylinderStart;
    endToHit = hitPos - cylinderEnd;

    startDot = DotProduct( startToHit, cylDirection );
    endDot = DotProduct( endToHit, cylDirection );

    if( startDot * endDot < 0.f ) {
        outIntersections[numHits] = inters[numHits];
        numHits++;
    }

    return numHits;
}


UNITTEST( "Basics", "Ray3", 0 ) {
    Ray3 ray3a;									    // default constructor
    Ray3 ray3b( Vec3::ONE );				    // explicit constructor
    Ray3 ray3c = Ray3( Vec3::LEFT, Vec3::ONE );	    // explicit constructor (per C++ standard)
    Ray3 ray3d( ray3b );							    // copy constructor
    Ray3 ray3e = ray3c;								// copy constructor (per C++ standard)
    Ray3 ray3bStr( "1,1,1 0,0,1" );                    // explicit string constructor
    std::string cStr = Stringf( "-1.f,0.f,0.f %f,%f,%f", fSQRT_3_OVER_3, fSQRT_3_OVER_3, fSQRT_3_OVER_3 );
    Ray3 ray3cStr = Ray3( cStr );                 // explicit string constructor (per C++ standard)

    UnitTest::VerifyResult( sizeof( Ray3 ) == 24,                               "sizeof(Ray3) was not 24 bytes",                                            theTest );
    UnitTest::VerifyResult( IsMostlyEqual( ray3b, Vec3::ONE, -Vec3::SCREEN ),   "Ray3( Vec3 ) : explicit constructor failed to assign start or direction",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( ray3d, ray3b ),                      "Ray3( Ray3 ) : copy constructor failed to copy start or direction",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( ray3bStr, ray3b ),                   "Ray3( string ) : string constructor failed to assign start or direction",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( ray3cStr, ray3c ),                   "Ray3( string ) : string constructor failed to assign start or direction",  theTest );
}


UNITTEST( "Methods", "Ray3", 0 ) {
    Vec3 vec3a = Vec3( -2.f, 2.f, 0.f );
    Vec3 vec3b = Vec3( -1.f, 1.f, 0.f );
    Vec3 vec3c = Vec3( 0.f, 0.f, 0.f );
    Vec3 vec3d = Vec3( 1.f, 0.f, 0.f );
    Ray3 ray3a = Ray3( vec3a, vec3d );
    Ray3 ray3b = Ray3( vec3b, vec3d );
    Ray3 ray3c = Ray3( vec3c, vec3d );

    Vec2 inter3a = Vec2::ZERO;
    Vec2 inter3b = Vec2::ZERO;
    Vec2 inter3c = Vec2::ZERO;

    // Versus Sphere
    int numInter3a = ray3a.Raycast( Vec3::ZERO, 1.f, (float*)&inter3a );
    int numInter3b = ray3b.Raycast( Vec3::ZERO, 1.f, (float*)&inter3b );
    int numInter3c = ray3c.Raycast( Vec3::ZERO, 1.f, (float*)&inter3c );

    UnitTest::VerifyResult( numInter3a == 0,                             "Ray3::RaycastSphere found incorrect number of intersections",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( inter3a, Vec2::ZERO),         "Ray3::RaycastSphere modified the intersections despite no intersections", theTest );
    UnitTest::VerifyResult( numInter3b == 1,                             "Ray3::RaycastSphere found incorrect number of intersections",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( inter3b, Vec2::RIGHT ),       "Ray3::RaycastSphere found incorrect intersection (expected 1)",           theTest );
    UnitTest::VerifyResult( numInter3c == 2,                             "Ray3::RaycastSphere found incorrect number of intersections",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( inter3c, Vec2( -1.f, 1.f ) ), "Ray3::RaycastSphere found incorrect intersections(expected 2)",           theTest );

    // Versus Capsule
    //Ray3 ray3d = Ray3( Vec3( -0.5f, 2.f, 0.f ), Vec3( 0.5f, -1.f, 0.f ) );
    //Capsule3 cap3a( Vec3( 0.f, 0.5f, 0.f ), Vec3( 0.f, 1.5f, 0.f ), 0.25f );
    //float interCap3a[2] = { 0.f };
    //int numInterCap3a = ray3d.Raycast( cap3a, interCap3a );  // FIXME: This isn't verified

    // GetAsString
    std::string ray3aStr = ray3a.GetAsString();
    std::string ray3bStr = ray3b.GetAsString();
    std::string ray3cStr = ray3c.GetAsString();

    std::string ray3aCorrectStr = "-2.000,2.000,0.000 1.000,0.000,0.000";
    std::string ray3bCorrectStr = "-1.000,1.000,0.000 1.000,0.000,0.000";
    std::string ray3cCorrectStr = "0.000,0.000,0.000 1.000,0.000,0.000";

    UnitTest::VerifyResult( ray3aStr == ray3aCorrectStr, "Ray3::GetAsString failed for (-2.000,2.000,0.000) (1.000,0.000,0.000)", theTest );
    UnitTest::VerifyResult( ray3bStr == ray3bCorrectStr, "Ray3::GetAsString failed for (-2.000,1.000,0.000) (1.000,0.000,0.000)", theTest );
    UnitTest::VerifyResult( ray3cStr == ray3cCorrectStr, "Ray3::GetAsString failed for (-2.000,0.000,0.000) (1.000,0.000,0.000)", theTest );
}

