#include "Engine/Math/Frustum.hpp"


bool Frustum::IsPointInside( const Vec3& point ) const {
    for( int faceIndex = 0; faceIndex < NUM_FRUSTUM_FACES; faceIndex++ ) {
        const Plane3& face = faces[faceIndex];
        float distAway = face.GetDistanceFromPlane( point );

        if( distAway < 0 ) {
            return false;
        }
    }

    return true;
}
