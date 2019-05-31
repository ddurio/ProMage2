#include "Game/TopDownFollowCamera.hpp"

#include "Engine/Math/Matrix44.hpp"

#include "Game/Entity.hpp"


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
            m_position = targetPosition;
            Matrix44 model = Matrix44::MakeTranslation2D( m_position );
            SetModelMatrix( model );
        }
    }
}


void TopDownFollowCamera::SetFollowTarget( const Entity* targetToFollow ) {
    m_target = targetToFollow;
}
