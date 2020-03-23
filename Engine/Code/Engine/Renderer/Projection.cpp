#include "Engine/Renderer/Projection.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"


//------------------------------------------------------------------------
// These matrices are assuming a "Z-Forward" basis (0 is near, 1 is far)
Matrix44 MakeProjectionOrthographic( float minX, float maxX, float minY, float maxY, float nearZ /*= -100.0f*/, float farZ /*= 100.0f*/ ) {
    float invDiffX = 1.f / (maxX - minX);
    float invDiffY = 1.f / (maxY - minY);
    float invDiffZ = 1.f / (farZ - nearZ);

    // range maps x from (n, f) to (-1, 1)
    // == (x - n) / (f - n) * (1 - -1) + -1; Range map equation
    // == 2(x - n) / (f - n) - (f - n)/(f-n)
    // == (2x / (f - n)) + (-2n - f + n)/(f - n)
    // == (2x / (f - n)) - (f + n) / (f - n);

    // So @ x = n, (- f - n + 2n) / (f - n) = -(f - n)/(f - n) == -1, checks out
    // And @ x = f, (-f - n + 2f) / (f - n) = (f - n) / (f - n) = 1, checks out

    Matrix44 ortho = Matrix44();
    ortho[Ix] = 2.f * invDiffX;
    ortho[Jy] = 2.f * invDiffY;
    ortho[Kz] = 1.f * invDiffZ;

    ortho[Tx] = -(maxX + minX) * invDiffX;
    ortho[Ty] = -(maxY + minY) * invDiffY;
    ortho[Tz] = -(nearZ + 0.f) * invDiffZ;

    return ortho;
}

//------------------------------------------------------------------------
Matrix44 MakeProjectionPerspective( float fovDegrees, float aspect, float nearZ, float farZ ) {
    float size = 1.f / TanDegrees( fovDegrees / 2.0f );

    float width = size;
    float height = size;

    // I always grow the side that is large (so fov is for the minimum dimension)
    // This is a personal choice - most engines will just always prefer either width
    // or height (usually height); 
    if( aspect > 1.0f ) {
        width /= aspect;
    } else {
        height *= aspect;
    }

    float invDiffZ = 1.0f / (farZ - nearZ);

    Matrix44 perspective = Matrix44();

    perspective[Ix] = width;
    perspective[Jy] = height;
    perspective[Kz] = farZ * invDiffZ;
    perspective[Kw] = 1.f;
    perspective[Tz] = -nearZ * farZ * invDiffZ;
    perspective[Tw] = 0.f;

    return perspective;
}