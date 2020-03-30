#define _USE_MATH_DEFINES
#include <cmath>
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Ray3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"


float ConvertRadiansToDegrees( float radians ) {
	return (radians * 180.f / (float)M_PI);
}


float ConvertDegreesToRadians( float degrees ) {
	return (degrees * (float)M_PI / 180.f);
}


float CosDegrees( float degrees ) {
	return cosf( ConvertDegreesToRadians( degrees ) );
}


float SinDegrees( float degrees ) {
	return sinf( ConvertDegreesToRadians( degrees ) );
}


float TanDegrees( float degrees ) {
    return tanf( ConvertDegreesToRadians( degrees ) );
}


float Atan2Degrees( float y, float x ) {
    return ConvertRadiansToDegrees( atan2f( y, x ) );
}


float GetDistanceXY( const Vec3& positionA, const Vec3& positionB ) {
    Vec3 displacement = positionB - positionA;
    return displacement.GetLengthXY();
}


float GetDistanceSquared( const Vec2& positionA, const Vec2& positionB ) {
    Vec2 displacement = positionB - positionA;
    return displacement.GetLengthSquared();
}


float GetDistanceSquared( const Vec3& positionA, const Vec3& positionB ) {
    Vec3 displacement = positionB - positionA;
    return displacement.GetLengthSquared();
}


float GetDistanceXYSquared( const Vec3& positionA, const Vec3& positionB ) {
    Vec3 displacement = positionB - positionA;
    return displacement.GetLengthXYSquared();
}


float GetDistance( const Vec2& positionA, const Vec2& positionB ) {
    Vec2 displacement = positionB - positionA;
    return displacement.GetLength();
}


float GetDistance( const Vec3& positionA, const Vec3& positionB ) {
    Vec3 displacement = positionB - positionA;
    return displacement.GetLength();
}


int GetManhattanDistance( const IntVec2& positionA, const IntVec2& positionB ) {
    IntVec2 displacement = positionB - positionA;
    int distance = abs( displacement.x ) + abs( displacement.y );
    
    return distance;
}


bool DoDiscsOverlap( const Vec2& centerA, float radiusA, const Vec2& centerB, float radiusB ) {
    float distanceSqr = GetDistanceSquared( centerA, centerB );
    float distanceRadii = radiusA + radiusB;
    return (distanceSqr < (distanceRadii * distanceRadii));
}


bool DoSpheresOverlap( const Vec3& centerA, float radiusA, const Vec3& centerB, float radiusB ) {
    float distanceSqr = GetDistanceSquared( centerA, centerB );
    float distanceRadii = radiusA + radiusB;
    return (distanceSqr < (distanceRadii * distanceRadii));
}


Vec2 TransformPosition( const Vec2& position, float uniformScale, float rotationDegrees, const Vec2& translation ) {
    // Scale
    Vec2 scaled = position * uniformScale;

    // Rotate
    Vec2 rotated = scaled.GetRotatedDegrees( rotationDegrees );

    // Translate
    Vec2 translated = rotated + translation;

    return translated;
}


Vec3 TransformPosition( const Vec3& position, float scaleXY, float rotationDegreesZ, const Vec2& translationXY ) {
    // Scale
    Vec3 scaled = Vec3( position.x * scaleXY, position.y * scaleXY, position.z );

    // Rotate
    Vec3 rotated = scaled.GetRotatedAboutZDegrees( rotationDegreesZ );

    // Translate
    Vec3 translated = rotated + Vec3( translationXY.x, translationXY.y, 0.0f );

    return translated;
}


Vec3 TransformPosition( const Vec3& position, float scaleXY, float rotationDegreesAboutZ, const Vec3& translation ) {
    Vec3 translated2D = TransformPosition( position, scaleXY, rotationDegreesAboutZ, Vec2(translation.x, translation.y) );
    return translated2D + Vec3( 0.f, 0.f, translation.z );
}


float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd ) {
    if( inStart == inEnd ) {
        // Avoid divide by zero failure
        return (outStart + outEnd) * 0.5f;
    }

    float inRange = inEnd - inStart;
    float outRange = outEnd - outStart;

    float inFraction = (inValue - inStart) / inRange;
    float outValue = (inFraction * outRange) + outStart;

    return outValue;
}


float Clamp( float inValue, float minValue, float maxValue ) {
    if( inValue < minValue ) {
        return minValue;
    } else if( inValue < maxValue ) {
        return inValue;
    } else {
        return maxValue;
    }
}


int Clamp( int inValue, int minValue, int maxValue ) {
    if( inValue < minValue ) {
        return minValue;
    } else if( inValue < maxValue ) {
        return inValue;
    } else {
        return maxValue;
    }
}


bool IsInRange( float inValue, const FloatRange& range ) {
    return (inValue >= range.min && inValue <= range.max);
}


bool IsInRange( int inValue, const IntRange& range ) {
    return (inValue >= range.min && inValue <= range.max);
}


int Min( int intA, int intB ) {
    return (intA <= intB ? intA : intB);
}


int Max( int intA, int intB ) {
    return (intA >= intB ? intA : intB);
}


float Min( float floatA, float floatB ) {
    return (floatA <= floatB ? floatA : floatB);
}


float Max( float floatA, float floatB ) {
    return (floatA >= floatB ? floatA : floatB);
}


int RoundToInt( float floatToRound ) {
    return (int)round( floatToRound );
}


int RoundToInt( double doubleToRound ) {
    return (int)round( doubleToRound );
}


int RoundUpToInt( float floatToCeil ) {
    return (int)ceil( floatToCeil );
}


int CountSetBits( unsigned int bitSet ) {
    int numSetBits = 0;

    while( bitSet != 0 ) {
        // Subtracting 1 flips all bits from the rightmost set bit onward
        // 10 (b'1010) - 1 = 9 (b'1001)
        // Bitwise AND  (b'1010 & b'1001 = b'1000) negates right most set bit
        // Simply count how many iterations that takes to get to zero
        numSetBits++;
        bitSet = bitSet & (bitSet - 1);
    }

    return numSetBits;
}


Vec3 GetCartesianFromSpherical( float radius, float degreesFromRight, float degreesFromUp ) {
    float x = radius * SinDegrees( degreesFromUp ) * CosDegrees( degreesFromRight );
    float y = radius * CosDegrees( degreesFromUp );
    float z = radius * SinDegrees( degreesFromUp ) * SinDegrees( degreesFromRight );

    return Vec3( x, y, z );
}


Vec2 GetClosestPointOnLine( const Vec2& point, const Vec2& linePointA, const Vec2& linePointB ) {
    Vec2 aToPoint = point - linePointA;
    Vec2 lineAToB = linePointB - linePointA;

    Vec2 projection = GetProjectedVector( aToPoint, lineAToB );
    return linePointA + projection;
}


Vec3 GetClosestPointOnLine( const Vec3& point, const Vec3& linePointA, const Vec3& linePointB ) {
    Vec3 aToPoint = point - linePointA;
    Vec3 lineAToB = linePointB - linePointA;

    Vec3 projection = GetProjectedVector( aToPoint, lineAToB );
    return linePointA + projection;
}


Vec2 GetClosestPointOnLineSegment( const Vec2& point, const Vec2& lineStart, const Vec2& lineEnd ) {
    Vec2 startToPointNorm = (point - lineStart).GetNormalized();
    Vec2 endToPointNorm = (point - lineEnd).GetNormalized();
    Vec2 lineNorm = (lineEnd - lineStart).GetNormalized();

    if( DotProduct( startToPointNorm, lineNorm ) < 0 ) { // Behind start of line segment
        return lineStart;
    } else if( DotProduct( endToPointNorm, lineNorm ) > 0 ) { // Past end of line segment
        return lineEnd;
    } else { // Within line segment
        return GetClosestPointOnLine( point, lineStart, lineEnd );
    }
}


Vec3 GetClosestPointOnLineSegment( const Vec3& point, const Vec3& lineStart, const Vec3& lineEnd ) {
    Vec3 startToPointNorm = (point - lineStart).GetNormalized();
    Vec3 endToPointNorm = (point - lineEnd).GetNormalized();
    Vec3 lineNorm = (lineEnd - lineStart).GetNormalized();

    if( DotProduct( startToPointNorm, lineNorm ) < 0 ) { // Behind start of line segment
        return lineStart;
    } else if( DotProduct( endToPointNorm, lineNorm ) > 0 ) { // Past end of line segment
        return lineEnd;
    } else { // Within line segment
        return GetClosestPointOnLine( point, lineStart, lineEnd );
    }
}


float GetAngularDisplacement( float startDegrees, float endDegrees ) {
    float displacementDegrees = endDegrees - startDegrees;

    while( displacementDegrees > 180.f ) {
        displacementDegrees -= 360.f;
    }

    while( displacementDegrees < -180.f ) {
        displacementDegrees += 360.f;
    }

    return displacementDegrees;
}


float GetTurnedTowards( float currentDegrees, float goalDegrees, float maxDeltaDegrees ) {
    float displacementDegrees = GetAngularDisplacement( currentDegrees, goalDegrees );
    float turnDegrees = Clamp( displacementDegrees, -maxDeltaDegrees, maxDeltaDegrees );
    return currentDegrees + turnDegrees;
}


bool DoesDiscOverlapAABB2( const Vec2& discCenter, float discRadius, const AABB2& aabb2 ) {
    Vec2 closestPoint = aabb2.GetClosestPointOnAABB2( discCenter );
    //Vec2 displacement = (discCenter - closestPoint);
    //float dispMagnitude = displacement.GetLength();
    float dispMagnitude = GetDistanceSquared( discCenter, closestPoint );
    return (dispMagnitude < discRadius * discRadius);
}


const Vec2 GetClosestPointOnDisc( const Vec2& point, const Vec2& discCenter, float discRadius ) {
    Vec2 displacement = point - discCenter;
    displacement.ClampLength( discRadius );
    return discCenter + displacement;
}


void PushDiscOutOfAABB2( Vec2& discCenter, float discRadius, const AABB2& aabb2 ) {
    Vec2 closestPoint = aabb2.GetClosestPointOnAABB2( discCenter );
    Vec2 displacement = (discCenter - closestPoint);
    float dispMagnitude = displacement.GetLength();

    if( dispMagnitude < discRadius ) {
        displacement.SetLength( discRadius - dispMagnitude );
        discCenter += displacement;
    }
}


void PushDiscOutOfDisc( Vec2& centerToPush, float radiusA, const Vec2& centerToPushOutOf, float radiusB ) {
    Vec2 displacement = (centerToPush - centerToPushOutOf);
    float dispMagnitude = displacement.GetLength();
    float combinedRadii = radiusA + radiusB;

    if( dispMagnitude < combinedRadii ) {
        float distanceToPushOut = combinedRadii - dispMagnitude;
        displacement.SetLength( distanceToPushOut );
        centerToPush += displacement;
    }
}


void PushDiscsOutOfEachOther( Vec2& centerA, float radiusA, Vec2& centerB, float radiusB ) {
    Vec2 displacement = (centerB - centerA);
    float dispMagnitude = displacement.GetLength();
    float combinedRadii = radiusA + radiusB;

    if( dispMagnitude < combinedRadii ) {
        float distanceToPushOut = combinedRadii - dispMagnitude;
        displacement.SetLength( 0.5f * distanceToPushOut );
        centerA -= displacement;
        centerB += displacement;
    }
}


float DotProduct( const Vec2& vecA, const Vec2& vecB ) {
    return (
        (vecA.x * vecB.x) +
        (vecA.y * vecB.y)
    );
}


float DotProduct( const Vec3& vecA, const Vec3& vecB ) {
    return (
        (vecA.x * vecB.x) +
        (vecA.y * vecB.y) +
        (vecA.z * vecB.z)
    );
}


float DotProduct( const Vec4& vecA, const Vec4& vecB ) {
    return (
        (vecA.x * vecB.x) +
        (vecA.y * vecB.y) +
        (vecA.z * vecB.z) +
        (vecA.w * vecB.w)
    );
}


float GetProjectedLength( const Vec2& projected, const Vec2& onto ) {
    Vec2 basisDirection = onto.GetNormalized();
    return DotProduct( projected, basisDirection );
}


const Vec2 GetProjectedVector( const Vec2& projected, const Vec2& onto ) {
    Vec2 basisDirection = onto.GetNormalized();
    float length = DotProduct( projected, basisDirection );

    return length * basisDirection;
}


const Vec2 GetReflectedVector( const Vec2& projected, const Vec2& onto ) {
    Vec2 normal = GetProjectedVector( projected, onto );
    Vec2 tangent = projected - normal;
    normal *= -1;

    return (normal + tangent);
}


const Vec3 GetProjectedVector( const Vec3& projected, const Vec3& onto ) {
    Vec3 basisDirection = onto.GetNormalized();
    float length = DotProduct( projected, basisDirection );

    return length * basisDirection;
}


const Vec3 GetReflectedVector( const Vec3& projected, const Vec3& onto ) {
    Vec3 normal = GetProjectedVector( projected, onto );
    Vec3 tangent = projected - normal;
    normal *= -1;

    return (normal + tangent);
}


float CrossProductLength( const Vec2& vecA, const Vec2& vecB ) {
    return (vecA.x * vecB.y) - (vecA.y * vecB.x); // Return just Z magnitude
}


Vec3 CrossProduct( const Vec3& vecA, const Vec3& vecB ) {
    float crossX = (vecA.y * vecB.z) - (vecA.z * vecB.y);
    float crossY = (vecA.z * vecB.x) - (vecA.x * vecB.z);
    float crossZ = (vecA.x * vecB.y) - (vecA.y * vecB.x);
    return Vec3( crossX, crossY, crossZ );
}


float SmoothStart2( float inZeroToOne ) {
    return inZeroToOne * inZeroToOne;
}


float SmoothStart3( float inZeroToOne ) {
    return inZeroToOne * inZeroToOne * inZeroToOne;
}


float SmoothStart4( float inZeroToOne ) {
    return inZeroToOne * inZeroToOne * inZeroToOne * inZeroToOne;
}


float SmoothStart5( float inZeroToOne ) {
    return inZeroToOne * inZeroToOne * inZeroToOne * inZeroToOne * inZeroToOne;
}


float SmoothStop2( float inZeroToOne ) {
    float oneMinusIn = 1 - inZeroToOne;
    return 1 - (oneMinusIn * oneMinusIn);
}


float SmoothStop3( float inZeroToOne ) {
    float oneMinusIn = 1 - inZeroToOne;
    return 1 - (oneMinusIn * oneMinusIn * oneMinusIn);
}


float SmoothStop4( float inZeroToOne ) {
    float oneMinusIn = 1 - inZeroToOne;
    return 1 - (oneMinusIn * oneMinusIn * oneMinusIn * oneMinusIn);
}


float SmoothStop5( float inZeroToOne ) {
    float oneMinusIn = 1 - inZeroToOne;
    return 1 - (oneMinusIn * oneMinusIn * oneMinusIn * oneMinusIn * oneMinusIn);
}


float SmoothStep3( float inZeroToOne ) {
    return inZeroToOne * inZeroToOne * (3 - (2 * inZeroToOne));
}


float SmoothStep5( float inZeroToOne ) {
    const float& t = inZeroToOne;
    return t * t * t * (t * ((t * 6) - 15) + 10);
}



bool IsMostlyEqual( int a, int b, int epsilon/*=0*/ ) {
    int diff = a - b;
    return (diff <= epsilon && diff >= -epsilon);
}


bool IsMostlyEqual( float a, float b, float epsilon/*=0.001f*/ ) {
    float diff = a - b;
    return (diff <= epsilon && diff >= -epsilon);
}


bool IsMostlyEqual( const Vec2& vec2, float x, float y ) {
    return IsMostlyEqual( vec2.x, x ) && IsMostlyEqual( vec2.y, y );
}


bool IsMostlyEqual( const Vec2& vec2a, const Vec2& vec2b ) {
    return IsMostlyEqual( vec2a.x, vec2b.x ) && IsMostlyEqual( vec2a.y, vec2b.y );
}


bool IsMostlyEqual( const Vec3& vec3, float x, float y, float z ) {
    return IsMostlyEqual( vec3.x, x ) && IsMostlyEqual( vec3.y, y ) && IsMostlyEqual( vec3.z, z );
}


bool IsMostlyEqual( const Vec3& vec3a, const Vec3& vec3b ) {
    return IsMostlyEqual( vec3a, vec3b.x, vec3b.y, vec3b.z );
}


bool IsMostlyEqual( const Vec4& vec4, float x, float y, float z, float w ) {
    return	IsMostlyEqual( vec4.x, x ) &&
        IsMostlyEqual( vec4.y, y ) &&
        IsMostlyEqual( vec4.z, z ) &&
        IsMostlyEqual( vec4.w, w );
}


bool IsMostlyEqual( const Vec4& vec4a, const Vec4& vec4b ) {
    return IsMostlyEqual( vec4a, vec4b.x, vec4b.y, vec4b.z, vec4b.w );
}


bool IsMostlyEqual( const Capsule2& cap2, const Vec2& start, const Vec2& end, float radius ) {
    return (IsMostlyEqual( cap2.start, start ) &&
        IsMostlyEqual( cap2.end, end ) &&
        IsMostlyEqual( cap2.radius, radius ));
}


bool IsMostlyEqual( const Capsule2& cap2a, const Capsule2& cap2b ) {
    return (IsMostlyEqual( cap2a.start, cap2b.start ) &&
        IsMostlyEqual( cap2a.end, cap2b.end ) &&
        IsMostlyEqual( cap2a.radius, cap2b.radius ));
}


bool IsMostlyEqual( const Capsule3& cap3, const Vec3& start, const Vec3& end, float radius ) {
    return (IsMostlyEqual( cap3.start, start ) &&
        IsMostlyEqual( cap3.end, end ) &&
        IsMostlyEqual( cap3.radius, radius ));
}


bool IsMostlyEqual( const Capsule3& cap3a, const Capsule3& cap3b ) {
    return (IsMostlyEqual( cap3a.start, cap3b.start ) &&
        IsMostlyEqual( cap3a.end, cap3b.end ) &&
        IsMostlyEqual( cap3a.radius, cap3b.radius ));
}


bool IsMostlyEqual( const Ray3& ray3a, const Vec3& start, const Vec3& dir ) {
    return IsMostlyEqual( ray3a.start, start )
        && IsMostlyEqual( ray3a.direction, dir );
}


bool IsMostlyEqual( const Ray3& ray3a, const Ray3& ray3b ) {
    return IsMostlyEqual( ray3a.start, ray3b.start )
        && IsMostlyEqual( ray3a.direction, ray3b.direction );
}


bool IsMostlyEqual( const Matrix44& matrix, const Matrix44& correctMatrix ) {
    bool isCorrectSize = sizeof( Matrix44 ) == 64;
    if( !isCorrectSize )
        return false;

    float* correctMatrixAsFloatArray = (float*)&correctMatrix;
    return IsMostlyEqual( matrix, correctMatrixAsFloatArray );
}


bool IsMostlyEqual( const Matrix44& matrix, float* sixteenCorrectMatrixValues ) {
    bool isCorrectSize = sizeof( Matrix44 ) == 64;
    if( !isCorrectSize )
        return false;

    float* matrixAsFloatArray = (float*)&matrix;
    return IsMostlyEqual( matrixAsFloatArray, sixteenCorrectMatrixValues, 16 );
}


bool IsMostlyEqual( float* floatArray1, float* floatArray2, int numArrayElements ) {
    for( int i = 0; i < numArrayElements; ++i ) {
        float value1 = floatArray1[i];
        float value2 = floatArray2[i];
        if( !IsMostlyEqual( value1, value2 ) )
            return false;
    }

    return true;
}


// Unit Tests ---------------------------------------------------------------------------------
UNITTEST( "Angles", "MathUtils", 0 ) {
    float radsFor30Degrees  = ConvertDegreesToRadians( 30.f );
    float radsForNegDegrees = ConvertDegreesToRadians( -713.61f );
    float degreesFor3Rads   = ConvertRadiansToDegrees( 3.f );
    float degreesForNegRads = ConvertRadiansToDegrees( -18.05f );

    float cos0 = CosDegrees( 0.f );		    float cos0Correct = 1.f;
    float cos45 = CosDegrees( 45.f );		float cos45Correct = 0.70710678118654752440084436210485f;
    float cos90 = CosDegrees( 90.f );		float cos90Correct = 0.f;
    float cos180 = CosDegrees( 180.f );		float cos180Correct = -1.f;
    float cos210 = CosDegrees( 210.f );		float cos210Correct = -0.86602540378443864676372317075294f;
    float cos270 = CosDegrees( 270.f );		float cos270Correct = 0.f;
    float cos370 = CosDegrees( 370.f );		float cos370Correct = 0.98480775301220805936674302458952f;
    float cosNeg10 = CosDegrees( -10.f );	float cosNeg10Correct = 0.98480775301220805936674302458952f;
    float cosNeg370 = CosDegrees( -370.f );	float cosNeg370Correct = 0.98480775301220805936674302458952f;

    float sin0 = SinDegrees( 0.f );		    float sin0Correct = 0.f;
    float sin45 = SinDegrees( 45.f );		float sin45Correct = 0.70710678118654752440084436210485f;
    float sin90 = SinDegrees( 90.f );		float sin90Correct = 1.f;
    float sin180 = SinDegrees( 180.f );		float sin180Correct = 0.f;
    float sin210 = SinDegrees( 210.f );		float sin210Correct = -0.5f;
    float sin270 = SinDegrees( 270.f );		float sin270Correct = -1.f;
    float sin370 = SinDegrees( 370.f );		float sin370Correct = 0.17364817766693034885171662676931f;
    float sinNeg10 = SinDegrees( -10.f );	float sinNeg10Correct = -0.17364817766693034885171662676931f;
    float sinNeg370 = SinDegrees( -370.f );	float sinNeg370Correct = -0.17364817766693034885171662676931f;

    bool isCosDegreesCorrect =
        IsMostlyEqual( cos0, cos0Correct ) &&
        IsMostlyEqual( cos45, cos45Correct ) &&
        IsMostlyEqual( cos90, cos90Correct ) &&
        IsMostlyEqual( cos180, cos180Correct ) &&
        IsMostlyEqual( cos210, cos210Correct ) &&
        IsMostlyEqual( cos270, cos270Correct ) &&
        IsMostlyEqual( cos370, cos370Correct ) &&
        IsMostlyEqual( cosNeg10, cosNeg10Correct ) &&
        IsMostlyEqual( cosNeg370, cosNeg370Correct );

    bool isSinDegreesCorrect =
        IsMostlyEqual( sin0, sin0Correct ) &&
        IsMostlyEqual( sin45, sin45Correct ) &&
        IsMostlyEqual( sin90, sin90Correct ) &&
        IsMostlyEqual( sin180, sin180Correct ) &&
        IsMostlyEqual( sin210, sin210Correct ) &&
        IsMostlyEqual( sin270, sin270Correct ) &&
        IsMostlyEqual( sin370, sin370Correct ) &&
        IsMostlyEqual( sinNeg10, sinNeg10Correct ) &&
        IsMostlyEqual( sinNeg370, sinNeg370Correct );

    UnitTest::VerifyResult( IsMostlyEqual( radsFor30Degrees, 0.523598776f ),  "ConvertDegreesToRadians(30 deg) failed / was incorrect",      theTest );
    UnitTest::VerifyResult( IsMostlyEqual( radsForNegDegrees, -12.4548435f ), "ConvertDegreesToRadians(-713.61 deg) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( degreesFor3Rads, 171.88733854f ),  "ConvertRadiansToDegrees(3 rad) failed / was incorrect",       theTest );
    UnitTest::VerifyResult( IsMostlyEqual( degreesForNegRads, -1034.18872f ), "ConvertRadiansToDegrees(-18.05 rad) failed / was incorrect",  theTest );
    UnitTest::VerifyResult( isCosDegreesCorrect,                              "CosDegrees() failed / was incorrect",                         theTest );
    UnitTest::VerifyResult( isSinDegreesCorrect,                              "SinDegrees() failed / was incorrect",                         theTest );

    float atan2DegreesTest1 = Atan2Degrees( 9.f, 9.f );
    float atan2DegreesTest2 = Atan2Degrees( 0.f, -1.f );
    float atan2DegreesTest3 = Atan2Degrees( -3.f, 4.f );
    float atan2DegreesTest4 = Atan2Degrees( -0.4f, -0.3f );
    bool isAtan2DegreesTest3Correct = IsMostlyEqual( atan2DegreesTest3, -36.8698959f ) || IsMostlyEqual( atan2DegreesTest3, 323.130104f );
    bool isAtan2DegreesTest4Correct = IsMostlyEqual( atan2DegreesTest4, -126.869896f ) || IsMostlyEqual( atan2DegreesTest4, 233.130104f );

    UnitTest::VerifyResult( IsMostlyEqual( atan2DegreesTest1, 45.f ), "Atan2Degrees(y=9,x=9) failed - should be 45 degrees",             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( atan2DegreesTest2, 180.f ), "Atan2Degrees(y=0,x=-1) failed - should be 180 degrees",          theTest );
    UnitTest::VerifyResult( isAtan2DegreesTest3Correct, "Atan2Degrees(y=-3,x=4) failed - should be about -36.9 (or +323.1) degrees",     theTest );
    UnitTest::VerifyResult( isAtan2DegreesTest4Correct, "Atan2Degrees(y=-.4,x=-.3) failed - should be about -126.9 (or +233.1) degrees", theTest );
}


UNITTEST( "Queries", "MathUtils", 0 ) {
    Vec2 posA( -3.1f, 4.1f );
    Vec2 posB( 4.9f, -1.9f ); // Points A&B are 10 apart (dx=8, dy=6)
    Vec2 posC( -0.1f, 0.3f );
    Vec2 posD( 0.4f, -0.9f ); // Points C&D are 1.3 apart (dx=.5, dy=1.2)

    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posA, posB ), 10.f ),         "GetDistance( vec2, vec2 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posB, posA ), 10.f ),         "GetDistance( vec2, vec2 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posC, posD ), 1.3f ),         "GetDistance( vec2, vec2 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posD, posC ), 1.3f ),         "GetDistance( vec2, vec2 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posA, posB ), 100.f ), "GetDistanceSquared( vec2, vec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posB, posA ), 100.f ), "GetDistanceSquared( vec2, vec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posC, posD ), 1.69f ), "GetDistanceSquared( vec2, vec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posD, posC ), 1.69f ), "GetDistanceSquared( vec2, vec2 ) failed / was incorrect", theTest );

    Vec3 posE( 6.0f, 7.3f, -8.6f );
    Vec3 posF( 6.2f, 7.0f, -8.0f ); // Points E&F are .7 apart (dx=.2, dy=.3, dz=.6)
    Vec3 posG( 0.1f, -0.2f, 0.5f );
    Vec3 posH( 0.0f, 0.2f, -0.3f ); // Points G&H are .9 apart (dx=.1, dy=.4, dz=.8)

    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posE, posF ), 0.7f ),           "GetDistance( vec3, vec3 ) failed / was incorrect",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posF, posE ), 0.7f ),           "GetDistance( vec3, vec3 ) failed / was incorrect",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posG, posH ), 0.9f ),           "GetDistance( vec3, vec3 ) failed / was incorrect",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistance( posH, posG ), 0.9f ),           "GetDistance( vec3, vec3 ) failed / was incorrect",          theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posE, posF ), 0.49f ),   "GetDistanceSquared( vec3, vec3 ) failed / was incorrect",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posF, posE ), 0.49f ),   "GetDistanceSquared( vec3, vec3 ) failed / was incorrect",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posG, posH ), 0.81f ),   "GetDistanceSquared( vec3, vec3 ) failed / was incorrect",   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceSquared( posH, posG ), 0.81f ),   "GetDistanceSquared( vec3, vec3 ) failed / was incorrect",   theTest );

    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXY( posE, posF ), 0.360555172f ), "GetDistanceXY( vec3, vec3 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXY( posF, posE ), 0.360555172f ), "GetDistanceXY( vec3, vec3 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXY( posG, posH ), 0.412310570f ), "GetDistanceXY( vec3, vec3 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXY( posH, posG ), 0.412310570f ), "GetDistanceXY( vec3, vec3 ) failed / was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXYSquared( posE, posF ), 0.13f ), "GetDistanceXYSquared( vec3, vec3 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXYSquared( posF, posE ), 0.13f ), "GetDistanceXYSquared( vec3, vec3 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXYSquared( posG, posH ), 0.17f ), "GetDistanceXYSquared( vec3, vec3 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( GetDistanceXYSquared( posH, posG ), 0.17f ), "GetDistanceXYSquared( vec3, vec3 ) failed / was incorrect", theTest );

    IntVec2 posI(  1,  2 );
    IntVec2 posJ(  2,  1 );
    IntVec2 posK( -2, -9 );
    IntVec2 posL( -9, -2 );
    IntVec2 posM( -2,  8 );

    UnitTest::VerifyResult( GetManhattanDistance( IntVec2::ZERO, posI ) ==  3, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( posI, IntVec2::ZERO ) ==  3, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( IntVec2::ZERO, posJ ) ==  3, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( posJ, IntVec2::ZERO ) ==  3, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( IntVec2::ZERO, posK ) == 11, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( posK, IntVec2::ZERO ) == 11, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( IntVec2::ZERO, posL ) == 11, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( posL, IntVec2::ZERO ) == 11, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( IntVec2::ZERO, posM ) == 10, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( posI, posM )          ==  9, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );
    UnitTest::VerifyResult( GetManhattanDistance( posK, posL )          == 14, "GetManhattanDistance( IntVec2, IntVec2 ) failed / was incorrect", theTest );

    Vec2 discACenter( 0.3f, 0.8f );
    Vec2 discBCenter( 3.3f, -3.2f ); // Discs A&B are 5 apart (dx=4, dy=3)

    // These three tests have combined radii > 5, therefore A&B should overlap here
    UnitTest::VerifyResult( DoDiscsOverlap( discACenter, 3.1f, discBCenter, 2.1f ), "DoDiscsOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoDiscsOverlap( discACenter, 1.1f, discBCenter, 4.1f ), "DoDiscsOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoDiscsOverlap( discBCenter, 0.1f, discACenter, 5.1f ), "DoDiscsOverlap() should have overlapped (but didn't)", theTest );

    // These three tests have combined radii < 5, therefore A&B should not overlap here
    UnitTest::VerifyResult( !DoDiscsOverlap( discACenter, 2.9f, discBCenter, 1.9f ), "DoDiscsOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoDiscsOverlap( discACenter, 0.9f, discBCenter, 3.9f ), "DoDiscsOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoDiscsOverlap( discBCenter, 0.1f, discACenter, 4.8f ), "DoDiscsOverlap() should not overlap (but did)", theTest );

    Vec2 discCCenter( 0.08f, -0.02f );
    Vec2 discDCenter( -0.04f, 0.03f ); // Discs C&D are .13 apart (dx=.12, dy=.05)

    // These three tests have combined radii > .13, therefore C&D should overlap here
    UnitTest::VerifyResult( DoDiscsOverlap( discCCenter, 0.09f, discDCenter, 0.05f ), "DoDiscsOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoDiscsOverlap( discCCenter, 0.01f, discDCenter, 0.13f ), "DoDiscsOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoDiscsOverlap( discDCenter, 0.12f, discCCenter, 0.02f ), "DoDiscsOverlap() should have overlapped (but didn't)", theTest );

    // These three tests have combined radii < .13, therefore C&D should not overlap here
    UnitTest::VerifyResult( !DoDiscsOverlap( discCCenter, 0.09f, discDCenter, 0.03f ), "DoDiscsOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoDiscsOverlap( discCCenter, 0.01f, discDCenter, 0.11f ), "DoDiscsOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoDiscsOverlap( discDCenter, 0.10f, discCCenter, 0.02f ), "DoDiscsOverlap() should not overlap (but did)", theTest );

    Vec3 sphereACenter( 1.5f, 50.7f, -3.5f );
    Vec3 sphereBCenter( -0.5f, 53.7f, 2.5f ); // Spheres A&B are 7 apart (dx=2, dy=3, dz=6)

    // These three tests have combined radii > 7, therefore A&B should overlap here
    UnitTest::VerifyResult( DoSpheresOverlap( sphereACenter, 5.6f, sphereBCenter, 1.5f ),   "DoSpheresOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoSpheresOverlap( sphereACenter, 1.6f, sphereBCenter, 5.5f ),   "DoSpheresOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoSpheresOverlap( sphereBCenter, 2.55f, sphereACenter, 4.55f ), "DoSpheresOverlap() should have overlapped (but didn't)", theTest );

    // These three tests have combined radii < 7, therefore A&B should not overlap here
    UnitTest::VerifyResult( !DoSpheresOverlap( sphereACenter, 1.5f, sphereBCenter, 5.4f ),   "DoSpheresOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoSpheresOverlap( sphereACenter, 5.5f, sphereBCenter, 1.4f ),   "DoSpheresOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoSpheresOverlap( sphereBCenter, 2.45f, sphereACenter, 4.45f ), "DoSpheresOverlap() should not overlap (but did)", theTest );

    Vec3 sphereCCenter( 2.45f, 0.1f, -0.2f );
    Vec3 sphereDCenter( 2.55f, -0.3f, 0.6f ); // Spheres C&D are .9 apart (dx=.1, dy=.4, dz=.8)

    // These three tests have combined radii > .9, therefore C&D should overlap here
    UnitTest::VerifyResult( DoSpheresOverlap( sphereCCenter, 0.1f, sphereDCenter, 0.9f ), "DoSpheresOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoSpheresOverlap( sphereCCenter, 0.6f, sphereDCenter, 0.4f ), "DoSpheresOverlap() should have overlapped (but didn't)", theTest );
    UnitTest::VerifyResult( DoSpheresOverlap( sphereDCenter, 0.5f, sphereCCenter, 0.5f ), "DoSpheresOverlap() should have overlapped (but didn't)", theTest );

    // These three tests have combined radii < .9, therefore C&D should not overlap here
    UnitTest::VerifyResult( !DoSpheresOverlap( sphereCCenter, 0.4f, sphereDCenter, 0.4f ), "DoSpheresOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoSpheresOverlap( sphereCCenter, 0.6f, sphereDCenter, 0.2f ), "DoSpheresOverlap() should not overlap (but did)", theTest );
    UnitTest::VerifyResult( !DoSpheresOverlap( sphereDCenter, 0.3f, sphereCCenter, 0.5f ), "DoSpheresOverlap() should not overlap (but did)", theTest );
}


UNITTEST( "Transforms", "MathUtils", 0 ) {
    /////////////////////////////////////////
    // Tests for TransformPosition2D
    /////////////////////////////////////////
    Vec2 origin2( 0.f, 0.f );
    Vec2 pos2a( 0.08f, 0.06f );
    Vec2 pos2b( -0.3f, 0.4f );
    Vec2 pos2c( -0.8f, -0.6f );
    Vec2 pos2d( 1.6f, -1.2f );

    // Test identity / do-nothing cases
    Vec2 identity2a   = TransformPosition( pos2a, 1.f, 0.f, origin2 );
    Vec2 identity2b   = TransformPosition( pos2b, 1.f, 360.f, origin2 );
    Vec2 identity2c   = TransformPosition( pos2c, 1.f, -720.f, origin2 );
    Vec2 stillOrigin2 = TransformPosition( origin2, 7.5f, 22.5f, origin2 );
    UnitTest::VerifyResult( IsMostlyEqual( identity2a, pos2a ),     "TransformPosition2D( scale=1, deg=0, trans=0,0 ) should not move the position",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity2b, pos2b ),     "TransformPosition2D( scale=1, deg=360, trans=0,0 ) should not move the position",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity2c, pos2c ),     "TransformPosition2D( scale=1, deg=-720, trans=0,0 ) should not move the position",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( stillOrigin2, origin2 ), "TransformPosition2D( scale=7.5, deg=22.5, trans=0,0 ) should not affect the origin(0,0)", theTest );

    // Test 2D uniform scaling only
    Vec2 scaled2b = TransformPosition( pos2b, -0.2f, 0.f, origin2 );
    Vec2 scaled2c = TransformPosition( pos2c, 1.1f, 0.f, origin2 );
    UnitTest::VerifyResult( IsMostlyEqual( scaled2b, 0.06f, -0.08f ),  "TransformPosition2D( scale only ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( scaled2c, -0.88f, -0.66f ), "TransformPosition2D( scale only ) was incorrect", theTest );

    // Test 2D rotation only
    Vec2 rotated2b = TransformPosition( pos2b, 1.f, 30.f, origin2 );
    Vec2 rotated2d = TransformPosition( pos2d, 1.f, -792.5f, origin2 );
    UnitTest::VerifyResult( IsMostlyEqual( rotated2b, -0.459808f,     0.196410f ),   "TransformPosition2D( rotation only ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( rotated2d, -0.663330197f, -1.88679433f ), "TransformPosition2D( rotation only ) was incorrect", theTest );

    // Test 2D translation only
    Vec2 translation2e( 101.101f, 202.202f );
    Vec2 translation2f( -303.303f, -404.404f );
    Vec2 translation2g( -123.456f, 789.123f );
    Vec2 translation2h( 135.791f, -246.802f );
    Vec2 translated2b = TransformPosition( pos2b, 1.f, 0.f, translation2e );
    Vec2 translated2d = TransformPosition( pos2d, 1.f, 0.f, translation2f );
    UnitTest::VerifyResult( IsMostlyEqual( translated2b, 100.801f, 202.602f ),         "TransformPosition2D( translation only ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( translated2d, -301.703003f, -405.604004f ), "TransformPosition2D( translation only ) was incorrect", theTest );

    // Test 2D combination transforms (scale, rotation, and/or translation)
    Vec2 scaledRotated2b     = TransformPosition( pos2b, 1.1f, 10.f, origin2 );
    Vec2 scaledRotated2d     = TransformPosition( pos2d, -11.f, -170.f, origin2 );
    Vec2 scaledTranslated2b  = TransformPosition( pos2b, 2.2f, 0.f, translation2g );
    Vec2 scaledTranslated2d  = TransformPosition( pos2d, -0.2f, 0.f, translation2h );
    Vec2 rotatedTranslated2b = TransformPosition( pos2b, 1.f, -15.f, translation2f );
    Vec2 rotatedTranslated2d = TransformPosition( pos2d, 1.f, 909.9f, translation2e );
    Vec2 fullyTransformed2b  = TransformPosition( pos2b, 2.f, 90.f, translation2h );
    Vec2 fullyTransformed2d  = TransformPosition( pos2d, -0.1f, -530.345f, translation2g );
    UnitTest::VerifyResult( IsMostlyEqual( scaledRotated2b, -0.401391804f, 0.376011491f ),    "TransformPosition2D( scale, rotate ) was incorrect",            theTest );
    UnitTest::VerifyResult( IsMostlyEqual( scaledRotated2d, 19.6247711f, -9.94325638f ),      "TransformPosition2D( scale, rotate ) was incorrect",            theTest );
    UnitTest::VerifyResult( IsMostlyEqual( scaledTranslated2b, -124.116005f, 790.002991f ),   "TransformPosition2D( scale, translate ) was incorrect",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( scaledTranslated2d, 135.470993f, -246.561996f ),   "TransformPosition2D( scale, translate ) was incorrect",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( rotatedTranslated2b, -303.489258f, -403.939972f ), "TransformPosition2D( rotate, translate ) was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( rotatedTranslated2d, 99.3185043f, 203.109039f ),   "TransformPosition2D( rotate, translate ) was incorrect",        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( fullyTransformed2b, 134.990997f, -247.402008f ),   "TransformPosition2D( scale, rotate, translate ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( fullyTransformed2d, -123.278141f, 789.031534f ),   "TransformPosition2D( scale, rotate, translate ) was incorrect", theTest );

    /////////////////////////////////////////
    // Tests for TransformPosition3DXY
    /////////////////////////////////////////
    Vec3 origin3( 0.f, 0.f, 0.f );
    Vec3 pos3a( 0.08f, 0.06f, 0.f );
    Vec3 pos3b( -0.3f, 0.4f, 1.1f );
    Vec3 pos3c( -0.8f, -0.6f, -0.5f );
    Vec3 pos3d( 1.6f, -1.2f, -0.1f );

    // Test identity / do-nothing cases
    Vec3 identity3a   = TransformPosition( pos3a, 1.f, 0.f, origin2 );
    Vec3 identity3b   = TransformPosition( pos3b, 1.f, 360.f, origin2 );
    Vec3 identity3c   = TransformPosition( pos3c, 1.f, -720.f, origin2 );
    Vec3 stillOrigin3 = TransformPosition( origin3, 7.5f, 22.5f, origin2 );
    UnitTest::VerifyResult( IsMostlyEqual( identity3a, pos3a ),     "TransformPosition3DXY( scale=1, deg=0, trans=0,0 ) should not move the position",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity3b, pos3b ),     "TransformPosition3DXY( scale=1, deg=360, trans=0,0 ) should not move the position",  theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity3c, pos3c ),     "TransformPosition3DXY( scale=1, deg=-720, trans=0,0 ) should not move the position", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( stillOrigin3, origin3 ), "TransformPosition3DXY( scaleXY, rotateXY ) should not affect the origin(0,0,0)",     theTest );

    // Test scaleXY only
    Vec3 scaled3b = TransformPosition( pos3b, -0.2f, 0.f, origin2 );
    Vec3 scaled3c = TransformPosition( pos3c, 1.1f, 0.f, origin2 );
    UnitTest::VerifyResult( IsMostlyEqual( scaled3b, 0.06f, -0.08f, 1.1f ),   "TransformPosition3DXY( scaleXY only ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( scaled3c, -0.88f, -0.66f, -0.5f ), "TransformPosition3DXY( scaleXY only ) was incorrect", theTest );

    // Test XY rotation only
    Vec3 rotated3b = TransformPosition( pos3b, 1.f, 30.f, origin2 );
    Vec3 rotated3d = TransformPosition( pos3d, 1.f, -792.5f, origin2 );
    UnitTest::VerifyResult( IsMostlyEqual( rotated3b, -0.459808f,     0.196410f,    1.1f ), "TransformPosition3DXY( rotateXY only ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( rotated3d, -0.663330197f, -1.88679433f, -0.1f ), "TransformPosition3DXY( rotateXY only ) was incorrect", theTest );

    // Test XY translation only
    Vec3 translated3b = TransformPosition( pos3b, 1.f, 0.f, translation2e );
    Vec3 translated3d = TransformPosition( pos3d, 1.f, 0.f, translation2f );
    UnitTest::VerifyResult( IsMostlyEqual( translated3b,  100.801f,     202.602f,     1.1f ), "TransformPosition3DXY( translateXY only ) was incorrect", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( translated3d, -301.703003f, -405.604004f, -0.1f ), "TransformPosition3DXY( translateXY only ) was incorrect", theTest );

    // Test combination transforms (scale, rotation, and/or translation: SR, ST, RT, SRT)
    Vec3 scaledRotated3b     = TransformPosition( pos3b, 1.1f, 10.f, origin2 );
    Vec3 scaledRotated3d     = TransformPosition( pos3d, -11.f, -170.f, origin2 );
    Vec3 scaledTranslated3b  = TransformPosition( pos3b, 2.2f, 0.f, translation2g );
    Vec3 scaledTranslated3d  = TransformPosition( pos3d, -0.2f, 0.f, translation2h );
    Vec3 rotatedTranslated3b = TransformPosition( pos3b, 1.f, -15.f, translation2f );
    Vec3 rotatedTranslated3d = TransformPosition( pos3d, 1.f, 909.9f, translation2e );
    Vec3 fullyTransformed3b  = TransformPosition( pos3b, 2.f, 90.f, translation2h );
    Vec3 fullyTransformed3d  = TransformPosition( pos3d, -0.1f, -530.345f, translation2g );

    bool isScaledRotated3bCorrect     = IsMostlyEqual( scaledRotated3b, -0.401391804f, 0.376011491f, 1.1f );
    bool isScaledRotated3dCorrect     = IsMostlyEqual( scaledRotated3d, 19.6247711f, -9.94325638f, -0.1f );
    bool isScaledTranslated3bCorrect  = IsMostlyEqual( scaledTranslated3b, -124.116005f, 790.002991f, 1.1f );
    bool isScaledTranslated3dCorrect  = IsMostlyEqual( scaledTranslated3d, 135.470993f, -246.561996f, -0.1f );
    bool isRotatedTranslated3bCorrect = IsMostlyEqual( rotatedTranslated3b, -303.489258f, -403.939972f, 1.1f );
    bool isRotatedTranslated3dCorrect = IsMostlyEqual( rotatedTranslated3d, 99.3185043f, 203.109039f, -0.1f );
    bool isFullyTransformed3bCorrect  = IsMostlyEqual( fullyTransformed3b, 134.990997f, -247.402008f, 1.1f );
    bool isFullyTransformed3dCorrect  = IsMostlyEqual( fullyTransformed3d, -123.278141f, 789.031534f, -0.1f );

    UnitTest::VerifyResult( isScaledRotated3bCorrect,     "TransformPosition3DXY( scaleXY, rotateXY ) was incorrect",              theTest );
    UnitTest::VerifyResult( isScaledRotated3dCorrect,     "TransformPosition3DXY( scaleXY, rotateXY ) was incorrect",              theTest );
    UnitTest::VerifyResult( isScaledTranslated3bCorrect,  "TransformPosition3DXY( scaleXY, translateXY ) was incorrect",           theTest );
    UnitTest::VerifyResult( isScaledTranslated3dCorrect,  "TransformPosition3DXY( scaleXY, translateXY ) was incorrect",           theTest );
    UnitTest::VerifyResult( isRotatedTranslated3bCorrect, "TransformPosition3DXY( rotateXY, translateXY ) was incorrect",          theTest );
    UnitTest::VerifyResult( isRotatedTranslated3dCorrect, "TransformPosition3DXY( rotateXY, translateXY ) was incorrect",          theTest );
    UnitTest::VerifyResult( isFullyTransformed3bCorrect,  "TransformPosition3DXY( scaleXY, rotateXY, translateXY ) was incorrect", theTest );
    UnitTest::VerifyResult( isFullyTransformed3dCorrect,  "TransformPosition3DXY( scaleXY, rotateXY, translateXY ) was incorrect", theTest );
}

