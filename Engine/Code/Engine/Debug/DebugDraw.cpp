#include "Engine/Debug/DebugDraw.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"
#include "Engine/Utils/NamedStrings.hpp"


DebugDraw* g_theDebugger = new DebugDraw();


void DebugDraw::Startup( RenderContext* renderer, float orthoScreenHeight /*= 10.f*/ ) {
    m_renderer = renderer;

    m_screenCamera = new Camera();
    m_screenCamera->SetOrthoProjection( orthoScreenHeight );

    m_unlitShader = m_renderer->GetOrCreateShader( SHADER_UNLIT );

    m_drawUBO = new UniformBuffer( m_renderer );

    // Setup console commands
    g_theEventSystem->SubscribeEventCallbackFunction( "DebugDrawToggle", Command_DebugDrawToggle );
    g_theEventSystem->SubscribeEventCallbackFunction( "DebugDrawClear", Command_DebugDrawClear );
}


void DebugDraw::Shutdown() {
    ClearMeshes();

    CLEAR_POINTER( m_screenCamera );
    CLEAR_POINTER( m_drawUBO );
}


void DebugDraw::BeginFrame() {
    SetTint( Rgba::WHITE );
}


void DebugDraw::EndFrame() {
    // Loop through meshes remove duration < 0
    std::list<DebugDrawMesh>::iterator worldIter = m_worldMeshes.begin();

    while( worldIter != m_worldMeshes.end() ) {
        if( worldIter->options.remainingSeconds <= 0.f ) {
            worldIter = m_worldMeshes.erase( worldIter );
        } else {
            worldIter++;
        }
    }


    std::list<DebugDrawMesh>::iterator screenIter = m_screenMeshes.begin();

    while( screenIter != m_screenMeshes.end() ) {
        if( screenIter->options.remainingSeconds <= 0.f ) {
            screenIter = m_screenMeshes.erase( screenIter );
        } else {
            screenIter++;
        }
    }


    std::list<DebugDrawMesh>::iterator messageIter = m_messageMeshes.begin();
    int messageIndex = 0;

    while( messageIter != m_messageMeshes.end() ) {
        if( messageIter->options.remainingSeconds <= 0.f ) {
            messageIter = m_messageMeshes.erase( messageIter );
        } else {
            DebugDrawOptions& options = messageIter->options;
            options.modelMatrix = GetMessageTranslationMatrix( messageIndex );
            messageIter++;
            messageIndex++;
        }
    }
}


void DebugDraw::Update( float deltaSeconds ) {
    // Loop through meshes, subtract dletaSecons from duration
    std::list<DebugDrawMesh>::iterator worldIter = m_worldMeshes.begin();

    while( worldIter != m_worldMeshes.end() ) {
        worldIter->options.remainingSeconds -= deltaSeconds;
        worldIter++;
    }


    std::list<DebugDrawMesh>::iterator screenIter = m_screenMeshes.begin();

    while( screenIter != m_screenMeshes.end() ) {
        screenIter->options.remainingSeconds -= deltaSeconds;
        screenIter++;
    }


    std::list<DebugDrawMesh>::iterator messageIter = m_messageMeshes.begin();

    while( messageIter != m_messageMeshes.end() ) {
        messageIter->options.remainingSeconds -= deltaSeconds;
        messageIter++;
    }
}


void DebugDraw::RenderWorld( Camera* gameCamera ) const {
    if( !m_isEnabled ) {
        return;
    }

     // Loop through meshes, render the meshes
    m_renderer->BeginCamera( gameCamera );
    std::list<DebugDrawMesh>::const_iterator worldIter = m_worldMeshes.begin();

    // Draw Depth Meshes
    m_unlitShader->SetDepthMode( COMPARE_LESS_EQUAL, true );
    m_renderer->BindShader( m_unlitShader );
    RenderMeshesByMode( m_worldMeshes, DRAW_DEPTH_MODE_DEPTH, gameCamera );

    // Draw XRAY Meshes
    m_unlitShader->SetDepthMode( COMPARE_GREATER, false );
    m_renderer->BindShader( m_unlitShader );
    RenderMeshesByMode( m_worldMeshes, DRAW_DEPTH_MOD_XRAY, gameCamera, 0.25f );

    m_unlitShader->SetDepthMode( COMPARE_LESS_EQUAL, true );
    m_renderer->BindShader( m_unlitShader );
    RenderMeshesByMode( m_worldMeshes, DRAW_DEPTH_MOD_XRAY, gameCamera, 1.f );

    // Draw Always Meshes
    m_unlitShader->SetDepthMode( COMPARE_ALWAYS, false );
    m_renderer->BindShader( m_unlitShader );
    RenderMeshesByMode( m_worldMeshes, DRAW_DEPTH_MODE_ALWAYS, gameCamera );

    SetTint( Rgba::WHITE );
}


void DebugDraw::RenderScreen() const {
    if( !m_isEnabled ) {
        return;
    }

     // Loop through meshes, render the meshes
    m_renderer->BeginCamera( m_screenCamera );
    std::list<DebugDrawMesh>::const_iterator screenIter = m_screenMeshes.begin();

    m_unlitShader->SetDepthMode( COMPARE_ALWAYS, false );
    m_renderer->BindShader( m_unlitShader );
    RenderMeshesByMode( m_screenMeshes, DRAW_DEPTH_MODE_ALWAYS );
    RenderMeshesByMode( m_messageMeshes, DRAW_DEPTH_MODE_ALWAYS );

    SetTint( Rgba::WHITE );
}


void DebugDraw::DrawDebugPoint( const Vec2& position, float duration, float size /*= DebugDrawOptions::s_defaultPointSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    Vec3 position3 = Vec3( position, 0.f );
    DrawDebugPoint( position3, duration, size, colorStart, colorEnd, depthMode );
}


void DebugDraw::DrawDebugPoint( const Vec3& position, float duration, float size /*= DebugDrawOptions::s_defaultPointSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH*/, FillMode fillMode /*= FILL_MODE_SOLID*/ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;
    options.fillMode = fillMode;
    options.modelMatrix = Matrix44::MakeTranslation3D( position );
    options.size = size;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugPoint( options );
}


void DebugDraw::DrawDebugPoint( const Vec2& alignment, const Vec2& offset, float duration, float size /*= DebugDrawOptions::s_defaultPointSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, DrawSpace coordinateSpace /*= DRAW_SPACE_SCREEN */ ) {
    DebugDrawOptions options;
    options.depthMode = DRAW_DEPTH_MODE_ALWAYS;
    options.coordinateSpace = coordinateSpace;

    // Position
    AABB2 cameraBounds = m_screenCamera->GetBounds();
    Vec2 position = cameraBounds.GetPointWithin( alignment );
    options.modelMatrix = Matrix44::MakeTranslation2D( position + offset );

    options.size = size;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugPoint( options );
}


void DebugDraw::DrawDebugPoint( const DebugDrawOptions& options ) {
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE ); // Color handled through UBO to facilitate color Lerp
    builder.AddUVSphere( Vec3::ZERO, options.size );

    AddNewMesh( &builder, options );
}


void DebugDraw::DrawDebugLine( const Vec2& start, const Vec2& end, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    Vec3 start3 = Vec3( start, 0.f );
    Vec3 end3   = Vec3( end, 0.f );

    DrawDebugLine( start3, end3, duration, size, colorStart, colorEnd, depthMode );
}


void DebugDraw::DrawDebugLine( const Vec3& start, const Vec3& end, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH*/ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;

    Vec3 direction = end - start;
    options.modelMatrix = Matrix44::MakeLookAt( start, end, Vec3::UP );

    options.length = direction.GetLength();
    options.size = size;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugLine( options );
}

void DebugDraw::DrawDebugLine( const Vec2& alignmentStart, const Vec2& offsetStart, const Vec2& alignmentEnd, const Vec2& offsetEnd, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */ ) {
    DebugDrawOptions options;

    // Position
    options.coordinateSpace = DRAW_SPACE_SCREEN;
    options.depthMode = DRAW_DEPTH_MODE_ALWAYS;

    AABB2 cameraBounds = m_screenCamera->GetBounds();
    Vec2 positionStart = cameraBounds.GetPointWithin( alignmentStart );
    positionStart += offsetStart;

    Vec2 positionEnd = cameraBounds.GetPointWithin( alignmentEnd );
    positionEnd += offsetEnd;

    Vec2 direction = positionEnd - positionStart;
    options.modelMatrix = Matrix44::MakeLookAt( Vec3( positionStart, 0.f ), Vec3( positionEnd, 0.f ), Vec3::UP );

    options.length = direction.GetLength();
    options.size = size;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugLine( options );
}


void DebugDraw::DrawDebugLine( DebugDrawOptions options ) {
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE ); // Color handled through UBO to facilitate color Lerp

    // Add line
    Vec3 lineStart = options.modelMatrix.TransformPosition3D( Vec3::ZERO );
    Vec3 lineEnd = options.modelMatrix.TransformPosition3D( Vec3( 0.f, 0.f, options.length ) );
    builder.AddLine( lineStart, lineEnd, options.size );
    options.modelMatrix = Matrix44::IDENTITY;

    AddNewMesh( &builder, options );
}


void DebugDraw::DrawDebugArrow( const Vec2& start, const Vec2& end, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    Vec3 start3 = Vec3( start, 0.f );
    Vec3 end3 = Vec3( end, 0.f );
    DrawDebugArrow( start3, end3, duration, size, colorStart, colorEnd, depthMode );
}


void DebugDraw::DrawDebugArrow( const Vec3& start, const Vec3& end, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH*/ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;

    Vec3 direction = end - start;
    options.modelMatrix = Matrix44::MakeLookAt( start, end, Vec3::UP );

    options.length = direction.GetLength();
    options.size = size;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugArrow( options );
}


void DebugDraw::DrawDebugArrow( const Vec2& alignmentStart, const Vec2& offsetStart, const Vec2& alignmentEnd, const Vec2& offsetEnd, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */ ) {
    DebugDrawOptions options;

    // Position
    options.coordinateSpace = DRAW_SPACE_SCREEN;
    options.depthMode = DRAW_DEPTH_MODE_ALWAYS;

    AABB2 cameraBounds = m_screenCamera->GetBounds();
    Vec2 positionStart = cameraBounds.GetPointWithin( alignmentStart );
    positionStart += offsetStart;

    Vec2 positionEnd = cameraBounds.GetPointWithin( alignmentEnd );
    positionEnd += offsetEnd;

    Vec2 direction = positionEnd - positionStart;
    options.modelMatrix = Matrix44::MakeLookAt( Vec3( positionStart, 0.f ), Vec3( positionEnd, 0.f ), Vec3::UP );

    options.length = direction.GetLength();
    options.size = size;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugArrow( options );
}


void DebugDraw::DrawDebugArrow( DebugDrawOptions options ) {
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE ); // Color handled through UBO to facilitate color Lerp

    // Add line
    Vec3 lineStart = options.modelMatrix.TransformPosition3D( Vec3::ZERO );
    float coneStartDistance = (0.8f * options.length);
    float lineEndDistance = coneStartDistance - (0.5f * options.size);
    Vec3 lineEnd = options.modelMatrix.TransformPosition3D( Vec3( 0.f, 0.f, lineEndDistance ) );
    builder.AddLine( lineStart, lineEnd, options.size );

    // Add arrow at end
    Vec3 coneStart = options.modelMatrix.TransformPosition3D( Vec3( 0.f, 0.f, coneStartDistance ) );
    builder.AddCone( coneStart, options.length - coneStartDistance, 1.25f * options.size, (lineEnd - lineStart) );
    options.modelMatrix = Matrix44::IDENTITY;

    AddNewMesh( &builder, options );
}


void DebugDraw::DrawDebugBasis( const Matrix44& basisMatrix, float duration, float size /*= DebugDrawOptions::s_defaultLineSize*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    Vec3 origin = basisMatrix.GetTranslation();
    Vec3 iBasis = basisMatrix.GetIBasis();
    Vec3 jBasis = basisMatrix.GetJBasis();
    Vec3 kBasis = basisMatrix.GetKBasis();

    DrawDebugPoint( origin, duration, size, Rgba::WHITE, Rgba::WHITE, depthMode );
    DrawDebugArrow( origin, origin + iBasis, duration, size, Rgba::RED,   Rgba::RED,   depthMode );
    DrawDebugArrow( origin, origin + jBasis, duration, size, Rgba::GREEN, Rgba::GREEN, depthMode );
    DrawDebugArrow( origin, origin + kBasis, duration, size, Rgba::BLUE,  Rgba::BLUE,  depthMode );
}


void DebugDraw::DrawDebugQuad( const AABB2& bounds, float duration, const Vec2& pivotAlignment /*= ALIGN_CENTER*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, std::string texturePath /*= ""*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    DrawDebugQuad( bounds.GetCenter(), bounds.GetDimensions(), duration, pivotAlignment, colorStart, colorEnd, texturePath, depthMode );
}


void DebugDraw::DrawDebugQuad( const Vec2& position, const Vec2& dimensions, float duration, const Vec2& pivotAlignment /*= ALIGN_CENTER*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, std::string texturePath /*= ""*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    Vec3 position3 = Vec3( position, 0.f );
    DrawDebugBillboardedQuad( position3, dimensions, duration, pivotAlignment, colorStart, colorEnd, texturePath, depthMode );
}


void DebugDraw::DrawDebugBillboardedQuad( const Vec3& center, const Vec2& dimensions, float duration, const Vec2& pivotAlignment /*= ALIGN_CENTER*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, std::string texturePath /*= "" */, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH*/ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;
    options.isBillboarded = true;

    float halfWidth = dimensions.x * 0.5f;
    float halfHeight = dimensions.y * 0.5f;

    options.length = halfWidth;
    options.size = halfHeight;

    AABB2 quad = AABB2( Vec2( -halfWidth, -halfHeight ), Vec2( halfWidth, halfHeight ) );
    Vec2 offset = quad.GetPointWithin( pivotAlignment );
    Vec3 position = center - Vec3( offset.x, offset.y, 0.f );
    options.modelMatrix.SetTranslation( position );

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    //Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugQuad( options );
}


void DebugDraw::DrawDebugQuad( const Vec2& alignment, const Vec2& offset, const Vec2& dimensions, float duration, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, std::string texturePath /*= "" */ ) {
    DebugDrawOptions options;

    // Position
    options.coordinateSpace = DRAW_SPACE_SCREEN;
    options.depthMode = DRAW_DEPTH_MODE_ALWAYS;

    float halfWidth = dimensions.x * 0.5f;
    float halfHeight = dimensions.y * 0.5f;

    options.length = halfWidth;
    options.size = halfHeight;

    AABB2 cameraBounds = m_screenCamera->GetBounds();
    AABB2 quad = cameraBounds.GetBoxWithin( dimensions, alignment );
    Vec2 center = quad.GetCenter();
    center += offset;

    options.modelMatrix.SetTranslation( Vec3( center.x, center.y, 0.f ) );

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;
    options.texturePath = texturePath;

    //Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugQuad( options );
}


void DebugDraw::DrawDebugQuad( const DebugDrawOptions& options ) {
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE ); // Color handled through UBO to facilitate color Lerp

    AABB2 quad = AABB2( Vec2( -options.length, -options.size ), Vec2( options.length, options.size ) );
    builder.AddQuad( quad );

    AddNewMesh( &builder, options );
}


void DebugDraw::DrawDebugBox( const OBB3& box, float duration, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, std::string texturePath /*= ""*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;

    Vec3 forward = CrossProduct( box.right, box.up );
    options.modelMatrix = Matrix44( box.right, box.up, forward, box.center );

    options.length = box.halfExtents.x;
    options.size = box.halfExtents.y;
    options.depth = box.halfExtents.z;

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;
    options.texturePath = texturePath;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    DrawDebugBox( options );
}


void DebugDraw::DrawDebugBox( DebugDrawOptions& options ) {
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE ); // Color handled through UBO to facilitate color Lerp

    OBB3 box = OBB3( options.modelMatrix.GetTranslation(), Vec3( options.length, options.size, options.depth ), options.modelMatrix.GetIBasis(), options.modelMatrix.GetJBasis() );
    builder.AddBox( box );
    options.modelMatrix = Matrix44::IDENTITY;

    AddNewMesh( &builder, options );
}



void DebugDraw::DrawDebugText( const Vec2& position, const Vec2& dimensions, std::string text, float duration, const Vec2& textAlignment /*= ALIGN_CENTER*/, BitmapFont* font /*= nullptr*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE*/, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH */ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;

    float halfWidth = dimensions.x * 0.5f;
    float halfHeight = dimensions.y * 0.5f;

    options.length = halfWidth;
    options.size = halfHeight;

    AABB2 quad = AABB2( Vec2( -halfWidth, -halfHeight ), Vec2( halfWidth, halfHeight ) );
    options.modelMatrix.SetTranslation( Vec3( position, 0.f ) );

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    // Font
    if( font == nullptr ) {
        std::string fontName = g_theGameConfigBlackboard.GetValue( "defaultFont", "" );

        if( fontName == "" ) {
            ERROR_RECOVERABLE( "ERROR: No font found for DebugDraw" );
            return;
        }

        font = m_renderer->GetOrCreateBitmapFont( fontName.c_str() );
    }

    options.texturePath = font->GetTexturePath();


    DrawDebugText( options, font, text, textAlignment );
}


void DebugDraw::DrawDebugBillboardedText( const Vec3& position, const Vec2& dimensions, std::string text, float duration, const Vec2& textAlignment /*= ALIGN_CENTER*/, BitmapFont* font /*= nullptr*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */, DrawDepthMode depthMode /*= DRAW_DEPTH_MODE_DEPTH*/ ) {
    DebugDrawOptions options;

    // Position
    options.depthMode = depthMode;
    options.isBillboarded = true;

    float halfWidth = dimensions.x * 0.5f;
    float halfHeight = dimensions.y * 0.5f;

    options.length = halfWidth;
    options.size = halfHeight;

    AABB2 quad = AABB2( Vec2( -halfWidth, -halfHeight ), Vec2( halfWidth, halfHeight ) );
    options.modelMatrix.SetTranslation( position );

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    // Font
    if( font == nullptr ) {
        std::string fontName = g_theGameConfigBlackboard.GetValue( "defaultFont", "" );

        if( fontName == "" ) {
            ERROR_RECOVERABLE( "ERROR: No font found for DebugDraw" );
            return;
        }

        font = m_renderer->GetOrCreateBitmapFont( fontName.c_str() );
    }

    options.texturePath = font->GetTexturePath();


    DrawDebugText( options, font, text, textAlignment );
}


void DebugDraw::DrawDebugText( const Vec2& alignment, const Vec2& offset, const Vec2& dimensions, std::string text, float duration, BitmapFont* font /*= nullptr*/, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */ ) {
    DebugDrawOptions options;

    // Position
    options.coordinateSpace = DRAW_SPACE_SCREEN;
    options.depthMode = DRAW_DEPTH_MODE_ALWAYS;

    float halfWidth = dimensions.x * 0.5f;
    float halfHeight = dimensions.y * 0.5f;

    options.length = halfWidth;
    options.size = halfHeight;

    AABB2 cameraBounds = m_screenCamera->GetBounds();
    AABB2 quad = cameraBounds.GetBoxWithin( dimensions, alignment );
    Vec2 center = quad.GetCenter();
    center += offset;

    options.modelMatrix.SetTranslation( Vec3( center.x, center.y, 0.f ) );

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    // Font
    if( font == nullptr ) {
        std::string fontName = g_theGameConfigBlackboard.GetValue( "defaultFont", "" );

        if( fontName == "" ) {
            ERROR_RECOVERABLE( "ERROR: No font found for DebugDraw" );
            return;
        }

        font = m_renderer->GetOrCreateBitmapFont( fontName.c_str() );
    }

    options.texturePath = font->GetTexturePath();


    DrawDebugText( options, font, text, alignment );
}


void DebugDraw::DrawDebugText( const DebugDrawOptions& options, BitmapFont* font, std::string text, const Vec2& alignment /*= ALIGN_CENTER*/ ) {
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE );

    AABB2 quad = AABB2( Vec2( -options.length, -options.size ), Vec2( options.length, options.size ) );
    VertexList textVerts;
    font->AddVertsForTextInBox2D( textVerts, quad, 2.f * options.size, text, Rgba::WHITE, 1.f, alignment, TEXT_DRAW_SHRINK_TO_FIT );

    builder.AddVertexArray( textVerts );
    AddNewMesh( &builder, options );
}


void DebugDraw::DrawDebugMessage( std::string text, float duration, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */, BitmapFont* font /*= nullptr*/ ) {
    DebugDrawOptions options;

    // Position
    options.coordinateSpace = DRAW_SPACE_SCREEN;
    options.depthMode = DRAW_DEPTH_MODE_ALWAYS;

    int messageIndex = (int)m_messageMeshes.size();
    options.modelMatrix = GetMessageTranslationMatrix( messageIndex );

    // Color
    options.colorStart = colorStart;
    options.colorEnd = colorEnd;

    // Time
    options.durationSeconds = duration;
    options.remainingSeconds = duration;

    // Font
    if( font == nullptr ) {
        std::string fontName = g_theGameConfigBlackboard.GetValue( "defaultFont", "" );

        if( fontName == "" ) {
            ERROR_RECOVERABLE( "ERROR: No font found for DebugDraw" );
            return;
        }

        font = m_renderer->GetOrCreateBitmapFont( fontName.c_str() );
    }

    options.texturePath = font->GetTexturePath();

    // Build Mesh
    CPUMesh builder;
    builder.SetColor( Rgba::WHITE );

    VertexList textVerts;

    float cellHeight = m_screenCamera->GetDimensions().y / (float)m_numMessagesPerScreen;
    font->AddVertsForText2D( textVerts, Vec2::ZERO, cellHeight, text );
    builder.AddVertexArray( textVerts );

    // AddNewMesh does not support messages because it looks just like a screen space mesh
    GPUMesh* gpuMesh = new GPUMesh( m_renderer );
    gpuMesh->CopyVertsFromCPUMesh( &builder );
    m_messageMeshes.emplace_back( gpuMesh, options );
}


void DebugDraw::DrawDebugPiP( const TextureView2D* renderTarget, const Vec2& alignment, const Vec2& offset, const Vec2& dimensions, float duration, const Rgba& colorStart /*= Rgba::WHITE*/, const Rgba& colorEnd /*= Rgba::WHITE */ ) {
    std::string renderTargetID = m_renderer->GetTextureID( renderTarget );
    DrawDebugQuad( alignment, offset, dimensions, duration, colorStart, colorEnd, renderTargetID );
}


bool DebugDraw::Command_DebugDrawToggle( EventArgs& args ) {
    g_theDebugger->m_isEnabled = args.GetValue( "enabled", !g_theDebugger->m_isEnabled );
    std::string msg = Stringf( "    - (DebugDraw): %s", (g_theDebugger->m_isEnabled ? "Enabled" : "Disabled") );
    g_theDevConsole->PrintString( msg, DevConsole::CHANNEL_INFO );
    return false;
}


bool DebugDraw::Command_DebugDrawClear( EventArgs& args ) {
    UNUSED( args );
    g_theDebugger->ClearMeshes();
    return false;
}


void DebugDraw::SetTint( const Rgba& tint ) const {
    m_drawUBO->CopyCPUToGPU( &tint, sizeof( Rgba ) );
    m_renderer->BindUBO( m_drawUBO, UBO_SLOT_DEBUG_DRAW );
}


void DebugDraw::AddNewMesh( const CPUMesh* builder, const DebugDrawOptions& options ) {
    GPUMesh* gpuMesh = new GPUMesh( m_renderer );

    Shader* oldShader = m_renderer->GetCurrentShader();
    m_renderer->BindShader( m_unlitShader );

    gpuMesh->CopyVertsFromCPUMesh( builder );
    m_renderer->BindShader( oldShader );

    if( options.coordinateSpace == DRAW_SPACE_CAMERA || options.coordinateSpace == DRAW_SPACE_WORLD ) {
        m_worldMeshes.emplace_back( gpuMesh, options );
    } else if( options.coordinateSpace == DRAW_SPACE_SCREEN ) {
        m_screenMeshes.emplace_back( gpuMesh, options );
    }
}


void DebugDraw::RenderMeshesByMode( const std::list<DebugDrawMesh>& meshList, DrawDepthMode depthMode, Camera* gameCamera /*= nullptr*/, float opacityFactor /*= 1.f*/ ) const {
    std::list<DebugDrawMesh>::const_iterator meshesIter = meshList.begin();

    while( meshesIter != meshList.end() ) {
        DebugDrawOptions options = meshesIter->options;

        if( options.depthMode == depthMode ) {
            // Calculate color blend
            float blendFraction = Clamp( 1 - (options.durationSeconds - options.remainingSeconds) / options.durationSeconds, 0.f, 1.f );
            Rgba currentColor = options.colorStart.GetBlended( options.colorEnd, blendFraction );
            currentColor.a *= opacityFactor;
            SetTint( currentColor );

            // Calculate model matrix
            Matrix44 modelMatrix;

            if( options.isBillboarded && gameCamera != nullptr ) {
                modelMatrix = gameCamera->GetModelMatrix();
                modelMatrix.SetTranslation( options.modelMatrix.GetTranslation() );
            } else {
                modelMatrix = options.modelMatrix;
            }

            if( options.fillMode == FILL_MODE_WIREFRAME ) { // Bind Wireframe
                m_unlitShader->SetFillMode( FILL_MODE_WIREFRAME );
                m_renderer->BindShader( m_unlitShader );
            }

            m_renderer->BindTexture( options.texturePath );
            m_renderer->DrawMesh( meshesIter->mesh, modelMatrix );

            if( options.fillMode == FILL_MODE_WIREFRAME ) { // Put it back to Solid
                m_unlitShader->SetFillMode( FILL_MODE_SOLID );
                m_renderer->BindShader( m_unlitShader );
            }
        }

        meshesIter++;
    }
}


void DebugDraw::ClearMeshes() {
    // Loop through meshes, erase them all to free GPUMesh memory
    std::list<DebugDrawMesh>::iterator worldIter = m_worldMeshes.begin();

    while( worldIter != m_worldMeshes.end() ) {
        worldIter = m_worldMeshes.erase( worldIter );
    }


    std::list<DebugDrawMesh>::iterator screenIter = m_screenMeshes.begin();

    while( screenIter != m_screenMeshes.end() ) {
        screenIter = m_screenMeshes.erase( screenIter );
    }


    std::list<DebugDrawMesh>::iterator messageIter = m_messageMeshes.begin();

    while( messageIter != m_messageMeshes.end() ) {
        messageIter = m_messageMeshes.erase( messageIter );
    }
}


Matrix44 DebugDraw::GetMessageTranslationMatrix( int messageIndex ) const {
    float vAlignment = 1 - ((float)(messageIndex + 1) / (float)m_numMessagesPerScreen);
    AABB2 cameraBounds = m_screenCamera->GetBounds();
    Vec2 position = cameraBounds.GetPointWithin( Vec2( 0.f, vAlignment ) );

    Matrix44 translation;
    translation.SetTranslation( Vec3( position.x, position.y, 0.f ) );

    return translation;
}


DebugDraw::DebugDrawMesh::DebugDrawMesh( GPUMesh* meshIn, const DebugDrawOptions& optionsIn ) :
    mesh(meshIn),
    options(optionsIn) {
}


DebugDraw::DebugDrawMesh::~DebugDrawMesh() {
    delete mesh;
    mesh = nullptr;
}
