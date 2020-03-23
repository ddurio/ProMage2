#include "Engine/Vertex/VertexUtils.hpp"

#include "Engine/Math/MathUtils.hpp"


void AddVertsForDisc2D( std::vector<VertexPCU>& vertexArray, const Vec2& center, float radius, const Rgba& color, int numSides /*= 64 */ ) {
    Vec3 initialOuterVert = Vec3( 1, 0, 0 );
    Vec3 transformedOuterVert;
    float degreesPerTriangle = 360.f / numSides;

    float vertAngle = 0.f;
    
    for( int i = 0; i < numSides; i++ ) {
        // Triangle Vert1
        vertexArray.push_back( VertexPCU( Vec3( center.x, center.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );

        // Triangle Vert2
        transformedOuterVert = TransformPosition( initialOuterVert, radius, -vertAngle, center );
        vertexArray.push_back( VertexPCU( transformedOuterVert, color, Vec2( 0.f, 0.f ) ) );

        // Triangle Vert3
        vertAngle += degreesPerTriangle;
        transformedOuterVert = TransformPosition( initialOuterVert, radius, -vertAngle, center );
        vertexArray.push_back( VertexPCU( transformedOuterVert, color, Vec2( 0.f, 0.f ) ) );
    }
}


void AddVertsForLine2D( std::vector<VertexPCU>& vertexArray, const Vec2& start, const Vec2& end, float thickness, const Rgba& color ) {
    float halfThickness = 0.5f * thickness;
    Vec2 forward = (end - start);
    forward.SetLength( halfThickness );
    Vec2 left = (end - start);
    left.SetLength( halfThickness );
    left.Rotate90Degrees();

    Vec2 startLeft = start - forward + left;
    Vec2 startRight = start - forward - left;
    Vec2 endLeft = end + forward + left;
    Vec2 endRight = end + forward - left;

    vertexArray.push_back( VertexPCU( Vec3( startLeft.x, startLeft.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );
    vertexArray.push_back( VertexPCU( Vec3( endLeft.x, endLeft.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );
    vertexArray.push_back( VertexPCU( Vec3( startRight.x, startRight.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );

    vertexArray.push_back( VertexPCU( Vec3( startRight.x, startRight.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );
    vertexArray.push_back( VertexPCU( Vec3( endLeft.x, endLeft.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );
    vertexArray.push_back( VertexPCU( Vec3( endRight.x, endRight.y, 0.f ), color, Vec2( 0.f, 0.f ) ) );
}


void AddVertsForRing2D( std::vector<VertexPCU>& vertexArray, const Vec2& center, float radius, float thickness, const Rgba& color, int numSides /*= 64 */ ) {
    Vec3 identityVector = Vec3( 1, 0, 0 );
    Vec3 innerRight;
    Vec3 outerRight;
    Vec3 innerLeft;
    Vec3 outerLeft;
    float degreesPerTriangle = 360.f / numSides;
    float innerRadius = radius - (0.5f * thickness);
    float outerRadius = radius + (0.5f * thickness);
    
    float vertAngle = 0.f;

    for( int i = 0; i < numSides; i++ ) {
        // Triangle Vert1
        innerLeft = TransformPosition( identityVector, innerRadius, vertAngle, center );
        vertexArray.push_back( VertexPCU( innerLeft, color, Vec2( 0.f, 0.f ) ) );

        // Triangle Vert2
        outerLeft = TransformPosition( identityVector, outerRadius, vertAngle, center );
        vertexArray.push_back( VertexPCU( outerLeft, color, Vec2( 0.f, 0.f ) ) );

        // Triangle Vert3
        vertAngle -= degreesPerTriangle;
        innerRight = TransformPosition( identityVector, innerRadius, vertAngle, center );
        vertexArray.push_back( VertexPCU( innerRight, color, Vec2( 0.f, 0.f ) ) );

        // Triangle2 Verts1 & 2
        vertexArray.push_back( VertexPCU( innerRight, color, Vec2( 0.f, 0.f ) ) );
        vertexArray.push_back( VertexPCU( outerLeft, color, Vec2( 0.f, 0.f ) ) );

        // Triangle2 Vert3
        outerRight = TransformPosition( identityVector, outerRadius, vertAngle, center );
        vertexArray.push_back( VertexPCU( outerRight, color, Vec2( 0.f, 0.f ) ) );
    }
}


void AddVertsForAABB2D( std::vector<VertexPCU>& vertexArray, const AABB2& box, const Rgba& color, const Vec2& uvAtMins /*= Vec2( 0.f, 0.f )*/, const Vec2& uvAtMaxs /*= Vec2( 1.f, 1.f ) */ ) {
    Vec2 uvTL = Vec2( uvAtMins.x, uvAtMaxs.y );
    Vec2 uvBR = Vec2( uvAtMaxs.x, uvAtMins.y );

    vertexArray.push_back( VertexPCU( Vec3( box.mins.x, box.mins.y, 0.f ), color, uvAtMins ) );
    vertexArray.push_back( VertexPCU( Vec3( box.mins.x, box.maxs.y, 0.f ), color, uvTL ) );
    vertexArray.push_back( VertexPCU( Vec3( box.maxs.x, box.mins.y, 0.f ), color, uvBR ) );

    vertexArray.push_back( VertexPCU( Vec3( box.mins.x, box.maxs.y, 0.f ), color, uvTL ) );
    vertexArray.push_back( VertexPCU( Vec3( box.maxs.x, box.maxs.y, 0.f ), color, uvAtMaxs ) );
    vertexArray.push_back( VertexPCU( Vec3( box.maxs.x, box.mins.y, 0.f ), color, uvBR ) );
}


void TransformVertex( VertexPCU& position, float scaleXY, float rotationDegrees, const Vec2& translationXY ) {
    position.position = TransformPosition( position.position, scaleXY, rotationDegrees, translationXY );
}


void TransformVertexArray( int numVertices, VertexPCU* position, float scaleXY, float rotationDegrees, const Vec2& translationXY ) {
    for( int i = 0; i < numVertices; i++ ) {
        TransformVertex( position[i], scaleXY, rotationDegrees, translationXY );
    }
}


void TransformVertexArray( std::vector<VertexPCU>& vertexArray, float scaleXY, float rotationDegreesAboutZ, const Vec2& translationXY ) {
    int size = (int)vertexArray.size();
    VertexPCU* data = vertexArray.data();
    TransformVertexArray( size, data, scaleXY, rotationDegreesAboutZ, translationXY );
}
