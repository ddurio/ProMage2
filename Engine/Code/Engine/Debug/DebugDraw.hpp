#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Debug/DebugDrawTypes.hpp"

#include <list>


class BitmapFont;
class Camera;
class CPUMesh;
class GPUMesh;
class RenderContext;
class Shader;
class TextureView2D;
class UniformBuffer;

struct AABB2;
struct OBB3;
struct Vec2;
struct Vec3;


class DebugDraw {
    public:
    DebugDraw() {};
    ~DebugDraw() {};

    void Startup( RenderContext* renderer, float orthoScreenHeight = 10.f );
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Update( float deltaSeconds );
    void RenderWorld( Camera* gameCamera ) const;
    void RenderScreen() const;

    void DrawDebugPoint( const Vec2& position, float duration, float size = DebugDrawOptions::s_defaultPointSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugPoint( const Vec3& position, float duration, float size = DebugDrawOptions::s_defaultPointSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH, FillMode fillMode = FILL_MODE_SOLID );
    void DrawDebugPoint( const Vec2& alignment, const Vec2& offset, float duration, float size = DebugDrawOptions::s_defaultPointSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawSpace coordinateSpace = DRAW_SPACE_SCREEN );
    void DrawDebugPoint( const DebugDrawOptions& options );

    void DrawDebugLine( const Vec2& start, const Vec2& end, float duration, float size = DebugDrawOptions::s_defaultLineSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugLine( const Vec3& start, const Vec3& end, float duration, float size = DebugDrawOptions::s_defaultLineSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugLine( const Vec2& alignmentStart, const Vec2& offsetStart, const Vec2& alignmentEnd, const Vec2& offsetEnd, float duration, float size = DebugDrawOptions::s_defaultLineSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE );
    void DrawDebugLine( DebugDrawOptions options );

    void DrawDebugArrow( const Vec2& start, const Vec2& end, float duration, float size = DebugDrawOptions::s_defaultLineSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugArrow( const Vec3& start, const Vec3& end, float duration, float size = DebugDrawOptions::s_defaultLineSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugArrow( const Vec2& alignmentStart, const Vec2& offsetStart, const Vec2& alignmentEnd, const Vec2& offsetEnd, float duration, float size = DebugDrawOptions::s_defaultLineSize, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE );
    void DrawDebugArrow( DebugDrawOptions options );

    void DrawDebugBasis( const Matrix44& basisMatrix, float duration, float size = DebugDrawOptions::s_defaultLineSize, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );

    void DrawDebugQuad( const AABB2& bounds, float duration, const Vec2& pivotAlignment = ALIGN_CENTER, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, std::string texturePath = "", DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugQuad( const Vec2& position, const Vec2& dimensions, float duration, const Vec2& pivotAlignment = ALIGN_CENTER, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, std::string texturePath = "", DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugBillboardedQuad( const Vec3& position, const Vec2& dimensions, float duration, const Vec2& pivotAlignment = ALIGN_CENTER, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, std::string texturePath = "", DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugQuad( const Vec2& alignment, const Vec2& offset, const Vec2& dimensions, float duration, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, std::string texturePath = "" );
    void DrawDebugQuad( const DebugDrawOptions& options );

    void DrawDebugBox( const OBB3& box, float duration, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, std::string texturePath = "", DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugBox( DebugDrawOptions& options );

    void DrawDebugText( const Vec2& position, const Vec2& dimensions, std::string text, float duration, const Vec2& textAlignment = ALIGN_CENTER, BitmapFont* font = nullptr, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugBillboardedText( const Vec3& position, const Vec2& dimensions, std::string text, float duration, const Vec2& pivotAlignment = ALIGN_CENTER, BitmapFont* font = nullptr, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, DrawDepthMode depthMode = DRAW_DEPTH_MODE_DEPTH );
    void DrawDebugText( const Vec2& alignment, const Vec2& offset, const Vec2& dimensions, std::string text, float duration, BitmapFont* font = nullptr, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE );
    void DrawDebugText( const DebugDrawOptions& options, BitmapFont* font, std::string text, const Vec2& alignment = ALIGN_CENTER );

    void DrawDebugMessage( std::string text, float duration, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE, BitmapFont* font = nullptr );

    void DrawDebugPiP( const TextureView2D* renderTarget, const Vec2& alignment, const Vec2& offset, const Vec2& dimensions, float duration, const Rgba& colorStart = Rgba::WHITE, const Rgba& colorEnd = Rgba::WHITE );


    static bool Command_DebugDrawToggle( EventArgs& args );
    static bool Command_DebugDrawClear( EventArgs& args );

    private:

    struct DebugDrawMesh {
        public:
        GPUMesh* mesh = nullptr;
        DebugDrawOptions options;

        DebugDrawMesh( GPUMesh* meshIn, const DebugDrawOptions& optionsIn );
        ~DebugDrawMesh();
    };

    std::list<DebugDrawMesh> m_worldMeshes;
    std::list<DebugDrawMesh> m_screenMeshes;
    std::list<DebugDrawMesh> m_messageMeshes;
    bool m_isEnabled = true;
    static const int m_numMessagesPerScreen = 20;

    RenderContext* m_renderer = nullptr;
    Camera* m_screenCamera = nullptr;
    Shader* m_unlitShader = nullptr;

    UniformBuffer* m_drawUBO = nullptr;

    void SetTint( const Rgba& tint ) const;
    void AddNewMesh( const CPUMesh* builder, const DebugDrawOptions& options );
    void RenderMeshesByMode( const std::list<DebugDrawMesh>& meshList, DrawDepthMode depthMode, Camera* gameCamera = nullptr, float opacityFactor = 1.f ) const;
    void ClearMeshes();
    Matrix44 GetMessageTranslationMatrix( int messageIndex ) const;
};
