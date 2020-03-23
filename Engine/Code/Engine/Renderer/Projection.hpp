#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct Matrix44;


Matrix44 MakeProjectionOrthographic( float minX, float maxX, float minY, float maxY, float const nearZ = -100.0f, float const farZ = 100.0f );
Matrix44 MakeProjectionPerspective( float fovDegrees, float aspect, float nz, float fz );
