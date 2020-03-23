#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Plane3.hpp"


enum FrustumFace {
    FRUSTUM_LEFT,
    FRUSTUM_RIGHT,
    FRUSTUM_TOP,
    FRUSTUM_BOTTOM,
    FRUSTUM_FRONT,
    FRUSTUM_BACK,

    NUM_FRUSTUM_FACES
};


struct Frustum {
    public:
    Plane3 faces[NUM_FRUSTUM_FACES]; // Plane normals should face INTO the frustum

    bool IsPointInside( const Vec3& point ) const;
};
