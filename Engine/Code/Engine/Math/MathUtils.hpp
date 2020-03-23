#pragma once
#include "Engine/Core/EngineCommon.hpp"

struct AABB2;
struct Capsule2;
struct Capsule3;
struct FloatRange;
struct IntRange;
struct IntVec2;
struct Matrix44;
struct Ray3;
struct Vec2;
struct Vec3;
struct Vec4;

constexpr float fSQRT_3_OVER_3 = 0.57735026918962576450914878050196f;

float ConvertRadiansToDegrees( float radians );
float ConvertDegreesToRadians( float degrees );
float CosDegrees( float degrees );
float SinDegrees( float degrees );
float TanDegrees( float degrees );
float Atan2Degrees( float y, float x );

float GetDistanceXY( const Vec3& positionA, const Vec3& positionB );
float GetDistanceSquared( const Vec2& positionA, const Vec2& positionB );
float GetDistanceSquared( const Vec3& positionA, const Vec3& positionB );
float GetDistanceXYSquared( const Vec3& positionA, const Vec3& positionB );

float GetDistance( const Vec2& positionA, const Vec2& positionB );
float GetDistance( const Vec3& positionA, const Vec3& positionB );

int GetManhattanDistance( const IntVec2& positionA, const IntVec2& positionB );

bool DoDiscsOverlap( const Vec2& centerA, float radiusA, const Vec2& centerB, float radiusB );
bool DoSpheresOverlap( const Vec3& centerA, float radiusA, const Vec3& centerB, float radiusB );

Vec2 TransformPosition( const Vec2& position, float uniformScale, float rotationDegrees, const Vec2& translation );
Vec3 TransformPosition( const Vec3& position, float scaleXY, float rotationDegreesAboutZ, const Vec2& translationXY );
Vec3 TransformPosition( const Vec3& position, float scaleXY, float rotationDegreesAboutZ, const Vec3& translation );

float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd );

float Clamp( float inValue, float minValue, float maxValue );
int Clamp( int inValue, int minValue, int maxValue );

bool IsInRange( float inValue, const FloatRange& range );
bool IsInRange( int inValue, const IntRange& range );

int Min( int intA, int intB );
int Max( int intA, int intB );

float Min( float floatA, float floatB );
float Max( float floatA, float floatB );

int RoundToInt( float floatToRound );
int RoundToInt( double doubleToRound );

int RoundUpToInt( float floatToCeil );

int CountSetBits( unsigned int bitSet );

Vec3 GetCartesianFromSpherical( float radius, float degreesFromRight, float degreesFromUp );

Vec2 GetClosestPointOnLine( const Vec2& point, const Vec2& linePointA, const Vec2& linePointB );
Vec3 GetClosestPointOnLine( const Vec3& point, const Vec3& linePointA, const Vec3& linePointB );
Vec2 GetClosestPointOnLineSegment( const Vec2& point, const Vec2& lineStart, const Vec2& lineEnd );
Vec3 GetClosestPointOnLineSegment( const Vec3& point, const Vec3& lineStart, const Vec3& lineEnd );


// Disc Physics
float GetAngularDisplacement( float startDegrees, float endDegrees );
float GetTurnedTowards( float currentDegrees, float goalDegrees, float maxDeltaDegrees );

bool DoesDiscOverlapAABB2( const Vec2& discCenter, float discRadius, const AABB2& aabb2 );
const Vec2 GetClosestPointOnDisc( const Vec2& point, const Vec2& discCenter, float discRadius );

void PushDiscOutOfAABB2( Vec2& discCenter, float discRadius, const AABB2& aabb2 );
void PushDiscOutOfDisc( Vec2& discCenterToPush, float discRadiusA, const Vec2& discCenterToPushOutOf, float discRadiusB );
void PushDiscsOutOfEachOther( Vec2& centerA, float radiusA, Vec2& centerB, float radiusB );


// Dot Products
float DotProduct( const Vec2& vecA, const Vec2& vecB );
float DotProduct( const Vec3& vecA, const Vec3& vecB );
float DotProduct( const Vec4& vecA, const Vec4& vecB );

float GetProjectedLength( const Vec2& projected, const Vec2& onto );
const Vec2 GetProjectedVector( const Vec2& projected, const Vec2& onto );
const Vec2 GetReflectedVector( const Vec2& projected, const Vec2& onto );

const Vec3 GetProjectedVector( const Vec3& projected, const Vec3& onto );
const Vec3 GetReflectedVector( const Vec3& projected, const Vec3& onto );


// Cross Product
float CrossProductLength( const Vec2& vecA, const Vec2& vecB );
Vec3 CrossProduct( const Vec3& vecA, const Vec3& vecB );


// Easing Functions
float SmoothStart2( float inZeroToOne );
float SmoothStart3( float inZeroToOne );
float SmoothStart4( float inZeroToOne );
float SmoothStart5( float inZeroToOne );

float SmoothStop2( float inZeroToOne );
float SmoothStop3( float inZeroToOne );
float SmoothStop4( float inZeroToOne );
float SmoothStop5( float inZeroToOne );

float SmoothStep3( float inZeroToOne );
float SmoothStep5( float inZeroToOne );


// UnitTest 'close enough' Functions
bool IsMostlyEqual( int a, int b, int epsilon = 0 );
bool IsMostlyEqual( float a, float b, float epsilon = 0.001f );

bool IsMostlyEqual( const Vec2& vec2, float x, float y );
bool IsMostlyEqual( const Vec2& vec2a, const Vec2& vec2b );
bool IsMostlyEqual( const Vec3& vec3, float x, float y, float z );
bool IsMostlyEqual( const Vec3& vec3a, const Vec3& vec3b );
bool IsMostlyEqual( const Vec4& vec4, float x, float y, float z, float w );
bool IsMostlyEqual( const Vec4& vec4a, const Vec4& vec4b );

bool IsMostlyEqual( const Matrix44& matrix, const Matrix44& correctMatrix );
bool IsMostlyEqual( const Matrix44& matrix, float* sixteenCorrectMatrixValues );
bool IsMostlyEqual( float* floatArray1, float* floatArray2, int numArrayElements );

bool IsMostlyEqual( const Capsule2& cap2,  const Vec2& start, const Vec2& end, float radius );
bool IsMostlyEqual( const Capsule2& cap2a, const Capsule2& cap2b );
bool IsMostlyEqual( const Capsule3& cap3,  const Vec3& start, const Vec3& end, float radius );
bool IsMostlyEqual( const Capsule3& cap3a, const Capsule3& cap3b );

bool IsMostlyEqual( const Ray3& ray3a, const Vec3& start, const Vec3& dir );
bool IsMostlyEqual( const Ray3& ray3a, const Ray3& ray3b );
