#include "Engine/Renderer/Camera.hpp"

#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Frustum.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Ray3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/Projection.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"


Camera::~Camera() {
    // RenderContext will take care of the textureViews
    CLEAR_POINTER( m_cameraUBO );
}


void Camera::SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight ) {
    SetOrthoView( Vec3( bottomLeft.x, bottomLeft.y, -100.f ), Vec3( topRight.x, topRight.y, 100.f ) );
}


void Camera::SetOrthoView( const Vec3& mins, const Vec3& maxs ) {
    // Set projection (FoV?)
    float viewHeight = maxs.y - mins.y;
    float viewWidth = maxs.x - mins.x;
    float aspect = viewWidth / viewHeight;

    Vec2 mins2D = Vec2( mins.x, mins.y );
    Vec2 maxs2D = Vec2( maxs.x, maxs.y );
    Vec2 center = (mins2D + maxs2D) * 0.5f;

    // Set view (translation)
    m_view = Matrix44::MakeTranslation2D( -center );
    m_model = Matrix44::MakeTranslation2D( center );

    SetOrthoProjection( viewHeight, mins.z, maxs.z, aspect );
}


void Camera::SetOrthoProjection( float height, float nearZ /*= -100.f*/, float farZ /*= 100.f*/, float aspect /*= (16.f / 9.f) */ ) {
    float width = aspect * height;
    float minX = -width * 0.5f;
    float maxX = width * 0.5f;

    float minY = -height * 0.5f;
    float maxY = height * 0.5f;

    Vec3 translation3D = m_model.GetTranslation();

    m_bottomLeft = Vec3( minX + translation3D.x, minY + translation3D.y, nearZ );
    m_topRight = Vec3( maxX + translation3D.x, maxY + translation3D.y, farZ );

    m_projection = MakeProjectionOrthographic( minX, maxX, minY, maxY, nearZ, farZ );
}


void Camera::SetPerspectiveProjection( float fovDegrees, float nearZ, float farZ, float aspect /*= (16.f / 9.f) */ ) {
    m_projection = MakeProjectionPerspective( fovDegrees, aspect, nearZ, farZ );
}


void Camera::SetModelMatrix( Matrix44 model ) {
    // Translation for botLeft topRight change
    Vec3 oldTranslation = m_model.GetTranslation();
    Vec3 newTranslation = model.GetTranslation();
    Vec3 displacement = newTranslation - oldTranslation;
    Translate( displacement );
}


Vec2 Camera::GetOrthoBottomLeft() const {
	return Vec2( m_bottomLeft.x, m_bottomLeft.y );
}


Vec2 Camera::GetOrthoTopRight() const {
	return Vec2( m_topRight.x, m_topRight.y );
}


Vec2 Camera::GetDimensions() const {
    float width = m_topRight.x - m_bottomLeft.x;
    float height = m_topRight.y - m_bottomLeft.y;
    return Vec2( width, height );
}


AABB2 Camera::GetBounds() const {
    Vec2 botLeft2D = GetOrthoBottomLeft();
    Vec2 topRight2D = GetOrthoTopRight();
    return AABB2( botLeft2D, topRight2D );
}


Vec2 Camera::GetCenter() const {
    float centerX;
    float centerY;
    GetCenter( centerX, centerY );

    return Vec2( centerX, centerY );
}


void Camera::GetCenter( float& centerX_out, float& centerY_out ) const {
    Vec2 dimensions = GetDimensions();

    centerX_out = m_bottomLeft.x + (0.5f * dimensions.x);
    centerY_out = m_bottomLeft.y + (0.5f * dimensions.y);
}


Vec3 Camera::GetPosition() const {
    return m_model.GetTranslation();
}


Matrix44 Camera::GetModelMatrix() const {
    return m_model;
}


Matrix44 Camera::GetViewMatrix() const {
    return m_view;
}


Matrix44 Camera::GetProjectionMatrix() const {
    return m_projection;
}


Vec2 Camera::ClientToNDCPoint( const IntVec2& clientPoint, const IntVec2& clientDimensions /*= IntVec2::ZERO */ ) const {
    IntVec2 clientDims = clientDimensions;

    if( clientDimensions == IntVec2::ZERO ) {
        GUARANTEE_OR_DIE( m_renderTarget != nullptr, "(Camera) Cannot calculate clientDimensions withot a RenderTarget" );

        clientDims = m_renderTarget->GetDimensions();
    }

    // Client to NDC
    Vec2 ndc;
    ndc.x = RangeMap( (float)clientPoint.x, 0.f, (float)clientDims.x, -1.f, 1.f );
    ndc.y = RangeMap( (float)clientPoint.y, (float)clientDims.y, 0.f, -1.f, 1.f );

    return ndc;
}


IntVec2 Camera::NDCToClientPoint( const Vec3& ndcPoint, const IntVec2& clientDimensions /*= IntVec2::ZERO */ ) const {
    IntVec2 clientDims = clientDimensions;

    if( clientDimensions == IntVec2::ZERO ) {
        GUARANTEE_OR_DIE( m_renderTarget != nullptr, "(Camera) Cannot calculate clientDimensions withot a RenderTarget" );

        clientDims = m_renderTarget->GetDimensions();
    }

    // Client to NDC
    IntVec2 client;
    client.x = (int)RangeMap( (float)ndcPoint.x, -1.f, 1.f, 0.f, (float)clientDims.x );
    client.y = (int)RangeMap( (float)ndcPoint.y, -1.f, 1.f, (float)clientDims.y, 0.f );

    return client;
}


Vec3 Camera::NDCToWorldPoint( const Vec2& ndcPoint, float ndcZ /*= 0.f */ ) const {
    // NDC to World
    Matrix44 ndcToWorld = m_projection.TransformMatrix( m_view ).GetInverse();

    Vec4 worldHomogeneous = ndcToWorld.TransformHomogeneousPoint3D( Vec4( ndcPoint.x, ndcPoint.y, ndcZ, 1.f ) );
    Vec4 worldPoint = worldHomogeneous / worldHomogeneous.w;

    return Vec3( worldPoint.x, worldPoint.y, worldPoint.z );
}


Vec3 Camera::NDCToWorldPoint( const Vec3& ndcPoint ) const {
    return NDCToWorldPoint( Vec2( ndcPoint.x, ndcPoint.y ), ndcPoint.z );
}


IntVec2 Camera::WorldToClientPoint( const Vec3& worldPoint, const IntVec2& clientDimensions /*= IntVec2::ZERO */ ) const {
    Matrix44 worldToNdc1 = m_projection.TransformMatrix( m_view );

    Vec4 clipPoint = worldToNdc1.TransformHomogeneousPoint3D( Vec4( worldPoint.x, worldPoint.y, worldPoint.z, 1.f ) );
    Vec3 ndcPoint = Vec3( clipPoint.x / clipPoint.w, clipPoint.y / clipPoint.w, clipPoint.z / clipPoint.w );
    return NDCToClientPoint( ndcPoint, clientDimensions );
}


Vec3 Camera::ClientToWorldPoint( const IntVec2& clientPoint, float ndcZ /*= 0.f*/, const IntVec2& clientDimensions /*= IntVec2::ZERO*/ ) const {
    Vec2 ndc = ClientToNDCPoint( clientPoint, clientDimensions );
    return NDCToWorldPoint( ndc, ndcZ );
}


Ray3 Camera::ClientToWorldRay( const IntVec2& clientPoint, const IntVec2& clientDimensions /*= IntVec2::ZERO */ ) const {
    Vec3 nearWorld = ClientToWorldPoint( clientPoint, 0.1f, clientDimensions );
    Vec3 farWorld = ClientToWorldPoint( clientPoint, 1.f, clientDimensions );

    Ray3 worldRay = Ray3( nearWorld, farWorld - nearWorld );
    return worldRay;
}


Vec2 Camera::GetMouseWorldPosition( const WindowContext* window ) const {
    IntVec2 mouseClient = window->GetMouseClientPosition();
    IntVec2 clientDims = window->GetClientDimensions();
    Vec3 mouseWorld = ClientToWorldPoint( mouseClient, 0.f, clientDims );

    return Vec2( mouseWorld.x, mouseWorld.y );
}


Frustum Camera::GetWorldFrustum() const {
    Vec3 ndcMins = Vec3( -1.f, -1.f, 0.f );
    Vec3 ndcMaxs = Vec3( 1.f, 1.f, 1.f );

    return GetWorldFrustumFromNDCRegion( ndcMins, ndcMaxs );
}


Frustum Camera::GetWorldFrustumFromClientRegion( const IntVec2& clientMins, const IntVec2& clientMaxs, const IntVec2& clientDimensions /*= IntVec2::ZERO */ ) const {
    Vec2 ndcMins2 = ClientToNDCPoint( clientMins, clientDimensions );
    Vec2 ndcMaxs2 = ClientToNDCPoint( clientMaxs, clientDimensions );

    Vec3 ndcMins3 = Vec3( ndcMins2, 0.f );
    Vec3 ndcMaxs3 = Vec3( ndcMaxs2, 1.f );

    return GetWorldFrustumFromNDCRegion( ndcMins3, ndcMaxs3 );
}


Frustum Camera::GetWorldFrustumFromNDCRegion( const Vec3& ndcPointA, const Vec3& ndcPointB ) const {
    float ndcMinX = Min( ndcPointA.x, ndcPointB.x );
    float ndcMinY = Min( ndcPointA.y, ndcPointB.y );
    float ndcMinZ = Min( ndcPointA.z, ndcPointB.z );

    float ndcMaxX = Max( ndcPointA.x, ndcPointB.x );
    float ndcMaxY = Max( ndcPointA.y, ndcPointB.y );
    float ndcMaxZ = Max( ndcPointA.z, ndcPointB.z );

    Vec3 ndcMins = Vec3( ndcMinX, ndcMinY, ndcMinZ );
    Vec3 ndcMaxs = Vec3( ndcMaxX, ndcMaxY, ndcMaxZ );

    // Front Corners - NDC
    Vec3 ndcLeftTopFront  = Vec3( ndcMins.x, ndcMaxs.y, ndcMins.z );
    Vec3 ndcRightTopFront = Vec3( ndcMaxs.x, ndcMaxs.y, ndcMins.z );
    Vec3 ndcLeftBotFront  = ndcMins;
    Vec3 ndcRightBotFront = Vec3( ndcMaxs.x, ndcMins.y, ndcMins.z );

    // Back Corners - NDC
    Vec3 ndcLeftTopBack   = Vec3( ndcMins.x, ndcMaxs.y, ndcMaxs.z );
    Vec3 ndcRightTopBack  = ndcMaxs;
    Vec3 ndcLeftBotBack   = Vec3( ndcMins.x, ndcMins.y, ndcMaxs.z );
    Vec3 ndcRightBotBack  = Vec3( ndcMaxs.x, ndcMins.y, ndcMaxs.z );

    // Front Corners - World
    Vec3 worldLeftTopFront  = NDCToWorldPoint( ndcLeftTopFront  );
    Vec3 worldRightTopFront = NDCToWorldPoint( ndcRightTopFront );
    Vec3 worldLeftBotFront  = NDCToWorldPoint( ndcLeftBotFront  );
    Vec3 worldRightBotFront = NDCToWorldPoint( ndcRightBotFront );
         
    // Back Corners - World
    Vec3 worldLeftTopBack   = NDCToWorldPoint( ndcLeftTopBack  );
    Vec3 worldRightTopBack  = NDCToWorldPoint( ndcRightTopBack );
    Vec3 worldLeftBotBack   = NDCToWorldPoint( ndcLeftBotBack  );
    Vec3 worldRightBotBack  = NDCToWorldPoint( ndcRightBotBack );

    // Create Frustum
    Frustum frustum;

    frustum.faces[FRUSTUM_LEFT] = Plane3( worldLeftBotFront, worldLeftTopFront, worldLeftBotBack );
    frustum.faces[FRUSTUM_RIGHT] = Plane3( worldRightBotFront, worldRightBotBack, worldRightTopFront );
    frustum.faces[FRUSTUM_TOP] = Plane3( worldLeftTopBack, worldRightTopFront, worldRightTopBack );
    frustum.faces[FRUSTUM_BOTTOM] = Plane3( worldLeftBotFront, worldRightBotBack, worldRightBotFront );
    frustum.faces[FRUSTUM_FRONT] = Plane3( worldRightTopFront, worldLeftBotFront, worldRightBotFront );
    frustum.faces[FRUSTUM_BACK] = Plane3( worldLeftBotBack, worldRightTopBack, worldRightBotBack );

    return frustum;
}

TextureView2D* Camera::GetRenderTarget() const {
    return m_renderTarget;
}


TextureView2D* Camera::GetDepthStencilView() const {
    return m_depthView;
}


const UniformBuffer* Camera::GetUBO() const {
    return m_cameraUBO;
}


void Camera::SetRenderTarget( TextureView2D* colorTarget ) {
    m_renderTarget = colorTarget;
}


void Camera::SetDepthStencilView( TextureView2D* depthView ) {
    m_depthView = depthView;
}


void Camera::Translate( const Vec2& translation2D ) {
    Vec3 translation3D = Vec3( translation2D, 0.f );
    Translate( translation3D );
}


void Camera::Translate( const Vec3& translation3D ) {
    Vec3 newBotLeft = m_bottomLeft + translation3D;
    Vec3 newTopRight = m_topRight + translation3D;
    SetOrthoView( newBotLeft, newTopRight );
}


void Camera::UpdateUBO( RenderContext* renderContext ) {
    if( m_cameraUBO == nullptr ) {
        m_cameraUBO = new UniformBuffer( renderContext );

        std::string bufferName = "CameraBuffer";
        RenderContext::SetD3DDebugName( (ID3D11DeviceChild*)(m_cameraUBO->GetBufferHandle()), bufferName );
    }

    CameraBuffer cpuData;
    cpuData.view = m_view;
    cpuData.projection = m_projection;
    cpuData.position = m_model.GetTranslation();

    m_cameraUBO->CopyCPUToGPU( &cpuData, sizeof( cpuData ) );
}
