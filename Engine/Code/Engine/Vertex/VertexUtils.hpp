#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Vertex/VertexPCU.hpp"

#include "vector"

typedef std::vector<VertexPCU> VertexList;

void AddVertsForDisc2D( std::vector<VertexPCU>& vertexArray, const Vec2& center, float radius, const Rgba& color, int numSides = 64 );
void AddVertsForLine2D( std::vector<VertexPCU>& vertexArray, const Vec2& start, const Vec2& end, float thickness, const Rgba& color );
void AddVertsForRing2D( std::vector<VertexPCU>& vertexArray, const Vec2& center, float radius, float thickness, const Rgba& color, int numSides = 64 );
void AddVertsForAABB2D( std::vector<VertexPCU>& vertexArray, const AABB2& box, const Rgba& color, const Vec2& uvAtMins = Vec2( 0.f, 1.f ), const Vec2& uvAtMaxs = Vec2( 1.f, 0.f ) );

void TransformVertex( VertexPCU& position, float scaleXY, float rotationDegreesAboutZ, const Vec2& translationXY );
void TransformVertexArray( int numVertices, VertexPCU* position, float scaleXY, float rotationDegreesAboutZ, const Vec2& translationXY );
void TransformVertexArray( std::vector<VertexPCU>& vertexArray, float scaleXY, float rotationDegreesAboutZ, const Vec2& translationXY );
