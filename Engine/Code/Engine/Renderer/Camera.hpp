#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"


struct CameraBuffer {
    Matrix44 view;
    Matrix44 projection;
    Vec3 position;
    float padding0;
};

class RenderContext;
class TextureView2D;
class UniformBuffer;
class WindowContext;

struct AABB2;
struct Frustum;
struct Ray3;


class Camera {
	public:
    Camera() {}
    Camera( const Camera& copy ) = delete; 
    virtual ~Camera();

	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
    void SetOrthoView( const Vec3& mins, const Vec3& maxs );
    void SetOrthoProjection( float height, float nearZ = -100.f, float farZ = 100.f, float aspect = (16.f / 9.f) );
    void SetPerspectiveProjection( float fovDegrees, float nearZ, float farZ, float aspect = (16.f / 9.f) );
    void SetModelMatrix( Matrix44 model );

	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
    Vec2 GetDimensions() const;
    AABB2 GetBounds() const;

    Vec2 GetCenter() const;
    void GetCenter( float& centerX_out, float& centerY_out ) const;
    Vec3 GetPosition() const;
    Matrix44 GetModelMatrix() const;
    Matrix44 GetViewMatrix() const;
    Matrix44 GetProjectionMatrix() const;

    Vec2 ClientToNDCPoint( const IntVec2& clientPoint, const IntVec2& clientDimensions = IntVec2::ZERO ) const;
    IntVec2 NDCToClientPoint( const Vec3& ndcPoint, const IntVec2& clientDimensions = IntVec2::ZERO ) const;
    Vec3 NDCToWorldPoint( const Vec2& ndcPoint, float ndcZ = 0.f ) const;
    Vec3 NDCToWorldPoint( const Vec3& ndcPoint ) const;
    IntVec2 WorldToClientPoint( const Vec3& worldPoint, const IntVec2& clientDimensions = IntVec2::ZERO ) const;
    Vec3 ClientToWorldPoint( const IntVec2& clientPoint, float ndcZ = 0.f, const IntVec2& clientDimensions = IntVec2::ZERO ) const;
    Ray3 ClientToWorldRay( const IntVec2& clientPoint, const IntVec2& clientDimensions = IntVec2::ZERO ) const;

    Vec2 GetMouseWorldPosition( const WindowContext* window ) const;

    Frustum GetWorldFrustum() const;
    Frustum GetWorldFrustumFromClientRegion( const IntVec2& clientMins, const IntVec2& clientMaxs, const IntVec2& clientDimensions = IntVec2::ZERO ) const;
    Frustum GetWorldFrustumFromNDCRegion( const Vec3& ndcMins, const Vec3& ndcMaxs ) const;

    TextureView2D* GetRenderTarget() const;
    TextureView2D* GetDepthStencilView() const;
    const UniformBuffer* GetUBO() const;

    void SetRenderTarget( TextureView2D* colorTarget );
    void SetDepthStencilView( TextureView2D* depthView );
    void Translate( const Vec2& translation );
    void Translate( const Vec3& translation );
    void UpdateUBO( RenderContext* renderContext );

	private:
    Vec3 m_bottomLeft = Vec3::ZERO;
    Vec3 m_topRight = Vec3::ONE;

    Matrix44 m_projection = Matrix44();
    Matrix44 m_model = Matrix44();
    Matrix44 m_view = Matrix44();

    TextureView2D* m_renderTarget = nullptr;
    TextureView2D* m_depthView = nullptr;
    UniformBuffer* m_cameraUBO = nullptr;
};
