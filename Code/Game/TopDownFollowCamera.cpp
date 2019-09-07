#include "Game/TopDownFollowCamera.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"

#include "Game/Entity.hpp"
#include "Game/MapGen/Map/Map.hpp"


TopDownFollowCamera::TopDownFollowCamera( const Entity* targetToFollow ) {
    SetFollowTarget( targetToFollow );
}


void TopDownFollowCamera::Startup() {

}


void TopDownFollowCamera::Shutdown() {

}


void TopDownFollowCamera::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    if( m_target != nullptr ) {
        Vec2 targetPosition = m_target->GetPosition();

        if( m_position != targetPosition ) {
            targetPosition = GetClampedPosition( targetPosition );

            m_position = targetPosition;
            Matrix44 model = Matrix44::MakeTranslation2D( m_position );
            SetModelMatrix( model );
        }
    }
}


void TopDownFollowCamera::SetFollowTarget( const Entity* targetToFollow ) {
    m_target = targetToFollow;
}


Vec2 TopDownFollowCamera::GetClampedPosition( const Vec2& targetPosition ) const {
    Map* theMap = m_target->GetMap();
    IntVec2 mapDims = theMap->GetMapDimensions();
    Vec2 cameraDims = GetDimensions();

    Vec2 halfCameraDims = cameraDims * 0.5f;

    Vec2 minPosition = halfCameraDims;
    Vec2 maxPosition = Vec2( mapDims ) - halfCameraDims;

    float clampedX = ClampFloat( targetPosition.x, minPosition.x, maxPosition.x );
    float clampedY = ClampFloat( targetPosition.y, minPosition.y, maxPosition.y );

    return Vec2( clampedX, clampedY );
}
