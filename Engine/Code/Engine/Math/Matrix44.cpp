#include "Engine/Math/Matrix44.hpp"

#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform2D.hpp"


const Matrix44 Matrix44::IDENTITY = Matrix44();
const Matrix44 Matrix44::ZERO = Matrix44( Vec4::ZERO, Vec4::ZERO, Vec4::ZERO, Vec4::ZERO );


Matrix44::Matrix44() {
    // Set to identity
    m_values[Ix] = 1;
    m_values[Jy] = 1;
    m_values[Kz] = 1;
    m_values[Tw] = 1;
}


Matrix44::Matrix44( float* valuesBasisMajor ) {
    for( int index = 0; index < 16; index++ ) {
        m_values[index] = valuesBasisMajor[index];
    }
}


Matrix44::Matrix44( const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation /*= Vec2::ZERO*/ ) {
    m_values[Ix] = iBasis.x;
    m_values[Iy] = iBasis.y;

    m_values[Jx] = jBasis.x;
    m_values[Jy] = jBasis.y;

    m_values[Kz] = 1;

    m_values[Tx] = translation.x;
    m_values[Ty] = translation.y;
    m_values[Tw] = 1;
}


Matrix44::Matrix44( const Vec3& iBasis, const Vec3& jBasis, const Vec3& kBasis, const Vec3& translation /*= Vec3::ZERO*/ ) {
    m_values[Ix] = iBasis.x;
    m_values[Iy] = iBasis.y;
    m_values[Iz] = iBasis.z;

    m_values[Jx] = jBasis.x;
    m_values[Jy] = jBasis.y;
    m_values[Jz] = jBasis.z;

    m_values[Kx] = kBasis.x;
    m_values[Ky] = kBasis.y;
    m_values[Kz] = kBasis.z;

    m_values[Tx] = translation.x;
    m_values[Ty] = translation.y;
    m_values[Tz] = translation.z;
    m_values[Tw] = 1;
}


Matrix44::Matrix44( const Vec4& iHomogeneous, const Vec4& jHomogeneous, const Vec4& kHomogeneous, const Vec4& translationHomogeneous /*= Vec4(0.f, 0.f, 0.f, 1.f)*/ ) {
    m_values[Ix] = iHomogeneous.x;
    m_values[Iy] = iHomogeneous.y;
    m_values[Iz] = iHomogeneous.z;
    m_values[Iw] = iHomogeneous.w;

    m_values[Jx] = jHomogeneous.x;
    m_values[Jy] = jHomogeneous.y;
    m_values[Jz] = jHomogeneous.z;
    m_values[Jw] = jHomogeneous.w;

    m_values[Kx] = kHomogeneous.x;
    m_values[Ky] = kHomogeneous.y;
    m_values[Kz] = kHomogeneous.z;
    m_values[Kw] = kHomogeneous.w;

    m_values[Tx] = translationHomogeneous.x;
    m_values[Ty] = translationHomogeneous.y;
    m_values[Tz] = translationHomogeneous.z;
    m_values[Tw] = translationHomogeneous.w;
}


Matrix44::Matrix44( const std::string& transform ) {
    // Expects space delimited strings with simple basis changes like "-y z -x"
    Strings params = SplitStringOnDelimeter( transform, ' ' );

    Vec3 basis[3] = {
        Vec3( 1, 0, 0 ),
        Vec3( 0, 1, 0 ),
        Vec3( 0, 0, 1 )
    };

    for( int basisIndex = 0; basisIndex < 3; basisIndex++ ) {
        std::string& param = params[basisIndex];

        if( param == "x" ) {
            basis[basisIndex] = Vec3( 1, 0, 0 );
        } else if( param == "-x" ) {
            basis[basisIndex] = Vec3( -1, 0, 0 );
        } else if( param == "y" ) {
            basis[basisIndex] = Vec3( 0, 1, 0 );
        } else if( param == "-y" ) {
            basis[basisIndex] = Vec3( 0, -1, 0 );
        } else if( param == "z" ) {
            basis[basisIndex] = Vec3( 0, 0, 1 );
        } else if( param == "-z" ) {
            basis[basisIndex] = Vec3( 0, 0, -1 );
        }
    }

    *this = Matrix44( basis[0], basis[1], basis[2] );
}


float Matrix44::operator[]( int index ) const {
    return m_values[index];
}


float& Matrix44::operator[]( int index ) {
    return m_values[index];
}


bool Matrix44::operator==( const Matrix44& compareTo ) const {
    bool iEqual = (GetHomogeneousIBasis() == compareTo.GetHomogeneousIBasis());
    bool jEqual = (GetHomogeneousJBasis() == compareTo.GetHomogeneousJBasis());
    bool kEqual = (GetHomogeneousKBasis() == compareTo.GetHomogeneousKBasis());
    bool tEqual = (GetHomogeneousTBasis() == compareTo.GetHomogeneousTBasis());

    return (iEqual && jEqual, kEqual, tEqual);
}


bool Matrix44::operator!=( const Matrix44& compareTo ) const {
    return !(*this == compareTo);
}


Matrix44 Matrix44::MakeXRotationDegrees( float degrees ) {
    Matrix44 rotationMatrix;

    float c = CosDegrees( degrees );
    float s = SinDegrees( degrees );

    rotationMatrix[Jy] = c;
    rotationMatrix[Jz] = s;

    rotationMatrix[Ky] = -s;
    rotationMatrix[Kz] = c;

    return rotationMatrix;
}


Matrix44 Matrix44::MakeYRotationDegrees( float degrees ) {
    Matrix44 rotationMatrix;

    float c = CosDegrees( degrees );
    float s = SinDegrees( degrees );

    rotationMatrix[Ix] = c;
    rotationMatrix[Iz] = -s;

    rotationMatrix[Kx] = s;
    rotationMatrix[Kz] = c;

    return rotationMatrix;
}


Matrix44 Matrix44::MakeZRotationDegrees( float degrees ) {
    Matrix44 rotationMatrix;

    float c = CosDegrees( degrees );
    float s = SinDegrees( degrees );

    rotationMatrix[Ix] = c;
    rotationMatrix[Iy] = s;

    rotationMatrix[Jx] = -s;
    rotationMatrix[Jy] = c;

    return rotationMatrix;
}


Matrix44 Matrix44::MakeTransform( const Transform2D& transform ) {
    Matrix44 transformMatrix = MakeZRotationDegrees( transform.rotationDegrees );
    transformMatrix.SetTranslation( transform.position );

    return transformMatrix;
}


Matrix44 Matrix44::MakeTranslation2D( const Vec2& translationXY ) {
    Matrix44 translationMatrix;

    translationMatrix[Tx] = translationXY.x;
    translationMatrix[Ty] = translationXY.y;

    return translationMatrix;
}


Matrix44 Matrix44::MakeUniformScale2D( float uniformScaleXY ) {
    Matrix44 scaledMatrix;

    scaledMatrix[Ix] = uniformScaleXY;
    scaledMatrix[Jy] = uniformScaleXY;

    return scaledMatrix;
}


Matrix44 Matrix44::MakeRotationDegrees3D( const Vec3& rotationDegrees, RotationOrder order /*= ROTATION_ORDER_DEFAULT */ ) {
    Matrix44 rotationX = MakeXRotationDegrees( rotationDegrees.x );
    Matrix44 rotationY = MakeYRotationDegrees( rotationDegrees.y );
    Matrix44 rotationZ = MakeZRotationDegrees( rotationDegrees.z );
    Matrix44 rotation3D;

    switch( order ) {
        case(ROTATION_ORDER_XYZ): {
            rotation3D = rotationX;
            rotation3D = rotationY.TransformMatrix( rotation3D );
            rotation3D = rotationZ.TransformMatrix( rotation3D );
            break;
        } case(ROTATION_ORDER_ZXY): {
            rotation3D = rotationZ;
            rotation3D = rotationX.TransformMatrix( rotation3D );
            rotation3D = rotationY.TransformMatrix( rotation3D );
            break;
        } default: {
            ERROR_RECOVERABLE( "(Matrix44) Rotation order undefined" );
        }
    }

    return rotation3D;
}


Matrix44 Matrix44::MakeRotationBetweenVectors( const Vec3& fromA, const Vec3& toB ) {
    Vec3 fromANorm = fromA.GetNormalized();
    Vec3 toBNorm   = toB.GetNormalized();

    // Uses formula from wikipedia...
    // https://en.wikipedia.org/wiki/Rotation_matrix#Conversion_from_and_to_axis%E2%80%93angle
    Vec3 rotationAxis = CrossProduct( fromANorm, toBNorm );
    rotationAxis.Normalize(); // Needed?

    float c = DotProduct( fromANorm, toBNorm );
    float s = sqrt( 1 - (c * c) ); // Comes from a2 + b2 = c2 --> (a/c)2 + (b/c)2 = 1 --> s2 + c2 = 1

    Matrix44 rotation3D = IDENTITY;

    rotation3D[Ix] = c + (rotationAxis.x * rotationAxis.x * (1 - c));
    rotation3D[Iy] = (rotationAxis.x * rotationAxis.y * (1 - c)) + (rotationAxis.z * s);
    rotation3D[Iz] = (rotationAxis.x * rotationAxis.z * (1 - c)) - (rotationAxis.y * s);

    rotation3D[Jx] = (rotationAxis.x * rotationAxis.y * (1 - c)) - (rotationAxis.z * s);
    rotation3D[Jy] = c + (rotationAxis.y * rotationAxis.y * (1 - c));
    rotation3D[Jz] = (rotationAxis.y * rotationAxis.z * (1 - c)) - (rotationAxis.x * s);

    rotation3D[Kx] = (rotationAxis.x * rotationAxis.z * (1 - c)) - (rotationAxis.y * s);
    rotation3D[Ky] = (rotationAxis.y * rotationAxis.z * (1 - c)) - (rotationAxis.x * s);
    rotation3D[Kz] = c + (rotationAxis.z * rotationAxis.z * (1 - c));

    return rotation3D;
}


Matrix44 Matrix44::MakeTranslation3D( const Vec3& translation ) {
    Matrix44 translationMatrix;

    translationMatrix[Tx] = translation.x;
    translationMatrix[Ty] = translation.y;
    translationMatrix[Tz] = translation.z;

    return translationMatrix;
}


Matrix44 Matrix44::MakeLookAt( const Vec3& fromPos, const Vec3& toPos, const Vec3& upRef ) {
    Vec3 forward = (toPos - fromPos).GetNormalized();
    Vec3 right = CrossProduct( upRef, forward ).GetNormalized();
    Vec3 up = CrossProduct( forward, right );

    Matrix44 lookAt = Matrix44( right, up, forward, fromPos );
    return lookAt;
}


Vec3 Matrix44::GetIBasis() const {
    return Vec3( m_values[Ix], m_values[Iy], m_values[Iz] );
}


Vec3 Matrix44::GetJBasis() const {
    return Vec3( m_values[Jx], m_values[Jy], m_values[Jz] );
}


Vec3 Matrix44::GetKBasis() const {
    return Vec3( m_values[Kx], m_values[Ky], m_values[Kz] );
}


Vec4 Matrix44::GetHomogeneousIBasis() const {
    return Vec4( m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw] );
}


Vec4 Matrix44::GetHomogeneousJBasis() const {
    return Vec4( m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw] );
}


Vec4 Matrix44::GetHomogeneousKBasis() const {
    return Vec4( m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw] );
}


Vec4 Matrix44::GetHomogeneousTBasis() const {
    return Vec4( m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw] );
}


Vec3 Matrix44::GetTranslation() const {
    return Vec3( m_values[Tx], m_values[Ty], m_values[Tz] );
}


void Matrix44::SetTranslation( const Vec2& translation ) {
    m_values[Tx] = translation.x;
    m_values[Ty] = translation.y;
}


void Matrix44::SetTranslation( const Vec3& translation ) {
    m_values[Tx] = translation.x;
    m_values[Ty] = translation.y;
    m_values[Tz] = translation.z;
}


void Matrix44::InvertOrthoNormal() {
    Vec3 translation = GetTranslation();
    SetTranslation( Vec3::ZERO );

    Transpose();

    Vec3 newTranslation = TransformPosition3D( -translation );
    SetTranslation( newTranslation );
}


Matrix44 Matrix44::GetInverse() const {
    Matrix44 inverse;

    double inv[16];
    double det;
    double m[16];

    for( int i = 0; i < 16; ++i ) {
        m[i] = (double)m_values[i];
    }

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
        m[4] * m[11] * m[14] +
        m[8] * m[6] * m[15] -
        m[8] * m[7] * m[14] -
        m[12] * m[6] * m[11] +
        m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
        m[4] * m[11] * m[13] -
        m[8] * m[5] * m[15] +
        m[8] * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
        m[4] * m[10] * m[13] +
        m[8] * m[5] * m[14] -
        m[8] * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
        m[1] * m[11] * m[14] +
        m[9] * m[2] * m[15] -
        m[9] * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
        m[0] * m[11] * m[14] -
        m[8] * m[2] * m[15] +
        m[8] * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
        m[0] * m[11] * m[13] +
        m[8] * m[1] * m[15] -
        m[8] * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
        m[0] * m[10] * m[13] -
        m[8] * m[1] * m[14] +
        m[8] * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
        m[1] * m[7] * m[14] -
        m[5] * m[2] * m[15] +
        m[5] * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
        m[0] * m[7] * m[14] +
        m[4] * m[2] * m[15] -
        m[4] * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
        m[0] * m[7] * m[13] -
        m[4] * m[1] * m[15] +
        m[4] * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
        m[0] * m[6] * m[13] +
        m[4] * m[1] * m[14] -
        m[4] * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
        m[1] * m[7] * m[10] +
        m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] -
        m[9] * m[2] * m[7] +
        m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
        m[0] * m[7] * m[10] -
        m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] +
        m[8] * m[2] * m[7] -
        m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
        m[0] * m[7] * m[9] +
        m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -
        m[8] * m[1] * m[7] +
        m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
        m[0] * m[6] * m[9] -
        m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] +
        m[8] * m[1] * m[6] -
        m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    det = 1.0 / det;

    for( int i = 0; i < 16; i++ ) {
        inverse.m_values[i] = (float)(inv[i] * det);
    }

    return inverse;
}


void Matrix44::Invert() {
    *this = GetInverse();
}


void Matrix44::Transpose() {
    Vec4 iBasis = Vec4( m_values[Ix], m_values[Jx], m_values[Kx], m_values[Tx] );
    Vec4 jBasis = Vec4( m_values[Iy], m_values[Jy], m_values[Ky], m_values[Ty] );
    Vec4 kBasis = Vec4( m_values[Iz], m_values[Jz], m_values[Kz], m_values[Tz] );
    Vec4 tBasis = Vec4( m_values[Iw], m_values[Jw], m_values[Kw], m_values[Tw] );
    *this = Matrix44( iBasis, jBasis, kBasis, tBasis );
}


const Vec2 Matrix44::TransformVector2D( const Vec2& vectorQuantity ) const {
    float tempX = m_values[Ix] * vectorQuantity.x + m_values[Jx] * vectorQuantity.y;
    float tempY = m_values[Iy] * vectorQuantity.x + m_values[Jy] * vectorQuantity.y;

    return Vec2( tempX, tempY );
}


const Vec2 Matrix44::TransformPosition2D( const Vec2& position ) const {
    float tempX = m_values[Ix] * position.x + m_values[Jx] * position.y + m_values[Tx];
    float tempY = m_values[Iy] * position.x + m_values[Jy] * position.y + m_values[Ty];

    return Vec2( tempX, tempY );
}


const Vec3 Matrix44::TransformVector3D( const Vec3& vectorQuantity ) const {
    float tempX = m_values[Ix] * vectorQuantity.x + m_values[Jx] * vectorQuantity.y + m_values[Kx] * vectorQuantity.z;
    float tempY = m_values[Iy] * vectorQuantity.x + m_values[Jy] * vectorQuantity.y + m_values[Ky] * vectorQuantity.z;
    float tempZ = m_values[Iz] * vectorQuantity.x + m_values[Jz] * vectorQuantity.y + m_values[Kz] * vectorQuantity.z;

    return Vec3( tempX, tempY, tempZ );
}


const Vec3 Matrix44::TransformPosition3D( const Vec3& position ) const {
    float tempX = m_values[Ix] * position.x + m_values[Jx] * position.y + m_values[Kx] * position.z + m_values[Tx];
    float tempY = m_values[Iy] * position.x + m_values[Jy] * position.y + m_values[Ky] * position.z + m_values[Ty];
    float tempZ = m_values[Iz] * position.x + m_values[Jz] * position.y + m_values[Kz] * position.z + m_values[Tz];

    return Vec3( tempX, tempY, tempZ );
}


const Vec4 Matrix44::TransformHomogeneousPoint3D( const Vec4& homogeneousPoint ) const {
    float tempX = m_values[Ix] * homogeneousPoint.x + m_values[Jx] * homogeneousPoint.y + m_values[Kx] * homogeneousPoint.z + m_values[Tx] * homogeneousPoint.w;
    float tempY = m_values[Iy] * homogeneousPoint.x + m_values[Jy] * homogeneousPoint.y + m_values[Ky] * homogeneousPoint.z + m_values[Ty] * homogeneousPoint.w;
    float tempZ = m_values[Iz] * homogeneousPoint.x + m_values[Jz] * homogeneousPoint.y + m_values[Kz] * homogeneousPoint.z + m_values[Tz] * homogeneousPoint.w;
    float tempW = m_values[Iw] * homogeneousPoint.x + m_values[Jw] * homogeneousPoint.y + m_values[Kw] * homogeneousPoint.z + m_values[Tw] * homogeneousPoint.w;

    return Vec4( tempX, tempY, tempZ, tempW );
}


const Matrix44 Matrix44::TransformMatrix( const Matrix44& matrix ) const {
    Vec4 newBasisI = TransformHomogeneousPoint3D( matrix.GetHomogeneousIBasis() );
    Vec4 newBasisJ = TransformHomogeneousPoint3D( matrix.GetHomogeneousJBasis() );
    Vec4 newBasisK = TransformHomogeneousPoint3D( matrix.GetHomogeneousKBasis() );
    Vec4 newBasisT = TransformHomogeneousPoint3D( matrix.GetHomogeneousTBasis() );

    return Matrix44( newBasisI, newBasisJ, newBasisK, newBasisT );
}


UNITTEST( "Construction", "Matrix", 0 ) {
    float zeroValues[16] = { 0.f };
    float identityValues[16]            = { 1.f,  0.f, 0.f, 0.f,  0.f, 1.f, 0.f, 0.f,   0.f, 0.f,  1.f,  0.f,   0.f,  0.f,  0.f,  1.f  };
    float explicit2DValues[16]          = { 1.f,  2.f, 0.f, 0.f,  3.f, 4.f, 0.f, 0.f,   0.f, 0.f,  1.f,  0.f,   5.f,  6.f,  0.f,  1.f  };
    float explicit2DNoTransValues[16]   = { 1.f,  2.f, 0.f, 0.f,  3.f, 4.f, 0.f, 0.f,   0.f, 0.f,  1.f,  0.f,   0.f,  0.f,  0.f,  1.f  };
    float explicit3DValues[16]          = { 1.f,  2.f, 3.f, 0.f,  4.f, 5.f, 6.f, 0.f,   7.f, 8.f,  9.f,  0.f,   10.f, 11.f, 12.f, 1.f  };
    float explicit3DNoTransValues[16]   = { 1.f,  2.f, 3.f, 0.f,  4.f, 5.f, 6.f, 0.f,   7.f, 8.f,  9.f,  0.f,   0.f,  0.f,  0.f,  1.f  };
    float explicit4DValues[16]          = { 1.f,  2.f, 3.f, 4.f,  5.f, 6.f, 7.f, 8.f,   9.f, 10.f, 11.f, 12.f,  13.f, 14.f, 15.f, 16.f };
    float explicit4DNoTransValues[16]   = { 1.f,  2.f, 3.f, 4.f,  5.f, 6.f, 7.f, 8.f,   9.f, 10.f, 11.f, 12.f,  0.f,  0.f,  0.f,  1.f  };
    float strTransformValues[16]        = { 0.f, -1.f, 0.f, 0.f,  0.f, 0.f, 1.f, 0.f,  -1.f, 0.f,  0.f,  0.f,   0.f,  0.f,  0.f,  1.f  };

    Vec2 iBasis2D( 1.f, 2.f );
    Vec2 jBasis2D( 3.f, 4.f );
    Vec2 translation2D( 5.f, 6.f );

    Vec3 iBasis3D( 1.f, 2.f, 3.f );
    Vec3 jBasis3D( 4.f, 5.f, 6.f );
    Vec3 kBasis3D( 7.f, 8.f, 9.f );
    Vec3 translation3D( 10.f, 11.f, 12.f );

    Vec4 iBasis4D( 1.f, 2.f, 3.f, 4.f );
    Vec4 jBasis4D( 5.f, 6.f, 7.f, 8.f );
    Vec4 kBasis4D( 9.f, 10.f, 11.f, 12.f );
    Vec4 translation4D( 13.f, 14.f, 15.f, 16.f );

    UnitTest::VerifyResult( sizeof( Matrix44 ) == 64, "sizeof(Matrix) was wrong, should be 64 bytes (16 packed floats)", theTest );

    Matrix44 identity;
    UnitTest::VerifyResult( IsMostlyEqual( identity, identityValues ),           "Matrix should default-construct to Identity",     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( Matrix44::IDENTITY, identityValues ), "Matrix::IDENTITY does not match Identity values", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( Matrix44::ZERO, zeroValues ),         "Matrix::ZERO is not all zeroes",                  theTest );

    // Constructors
    Matrix44 explicitFloat16Array( explicit4DValues );
    Matrix44 explicit2DNoTrans( iBasis2D, jBasis2D );
    Matrix44 explicit2DTrans( iBasis2D, jBasis2D, translation2D );
    Matrix44 explicit3DNoTrans( iBasis3D, jBasis3D, kBasis3D );
    Matrix44 explicit3DTrans( iBasis3D, jBasis3D, kBasis3D, translation3D );
    Matrix44 explicit4DNoTrans( iBasis4D, jBasis4D, kBasis4D );
    Matrix44 explicit4DTrans( iBasis4D, jBasis4D, kBasis4D, translation4D );
    Matrix44 explicitStrIdentity( "x y z" );
    Matrix44 explicitStrTransform( "-y z -x" );

    UnitTest::VerifyResult( IsMostlyEqual( explicitFloat16Array, explicit4DValues ),     "Matrix explicit constructor from float* (float[16]) was incorrect",                               theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicit2DNoTrans, explicit2DNoTransValues ), "Matrix explicit constructor from (iBasis2D, jBasis2D) was incorrect",                             theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicit2DTrans, explicit2DValues ),          "Matrix explicit constructor from (iBasis2D, jBasis2D, translation2D) was incorrect",              theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicit3DNoTrans, explicit3DNoTransValues ), "Matrix explicit constructor from (iBasis3D, jBasis3D, kBasis3D) was incorrect",                   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicit3DTrans, explicit3DValues ),          "Matrix explicit constructor from (iBasis3D, jBasis3D, kBasis3D, translation3D) was incorrect",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicit4DNoTrans, explicit4DNoTransValues ), "Matrix explicit constructor from (iBasis4D, jBasis4D, kBasis4D) was incorrect",                   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicit4DTrans, explicit4DValues ),          "Matrix explicit constructor from (iBasis4D, jBasis4D, kBasis4D, translation4D) was incorrect",    theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicitStrIdentity, identityValues ),        "Matrix explicit constructor from str( x y z )",                                                   theTest );
    UnitTest::VerifyResult( IsMostlyEqual( explicitStrTransform, strTransformValues ),   "Matrix explicit constructor from str( -y z -x )",                                                 theTest );

    // [ ] operators read, and Ix enum
    UnitTest::VerifyResult( IsMostlyEqual( identity[Ix], identityValues[Ix] ), "Matrix Identity Ix does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Iy], identityValues[Iy] ), "Matrix Identity Iy does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Iz], identityValues[Iz] ), "Matrix Identity Iz does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Iw], identityValues[Iw] ), "Matrix Identity Iw does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Jx], identityValues[Jx] ), "Matrix Identity Jx does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Jy], identityValues[Jy] ), "Matrix Identity Jy does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Jz], identityValues[Jz] ), "Matrix Identity Jz does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Jw], identityValues[Jw] ), "Matrix Identity Jw does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Kx], identityValues[Kx] ), "Matrix Identity Kx does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Ky], identityValues[Ky] ), "Matrix Identity Ky does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Kz], identityValues[Kz] ), "Matrix Identity Kz does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Kw], identityValues[Kw] ), "Matrix Identity Kw does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Tx], identityValues[Tx] ), "Matrix Identity Tx does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Ty], identityValues[Ty] ), "Matrix Identity Ty does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Tz], identityValues[Tz] ), "Matrix Identity Tz does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( identity[Tw], identityValues[Tw] ), "Matrix Identity Tw does not match", theTest );

    // [ ] operator write
    Matrix44 bracketWrite = identity;
    bracketWrite[Iy] = 42.f;
    bracketWrite[Jz] = 24.f;
    bracketWrite[Kw] = 22.f;
    bracketWrite[Tx] = 2.f;

    UnitTest::VerifyResult( IsMostlyEqual( bracketWrite[Iy], 42.f ), "Matrix Identity Iy does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bracketWrite[Jz], 24.f ), "Matrix Identity Jz does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bracketWrite[Kw], 22.f ), "Matrix Identity Kw does not match", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( bracketWrite[Tx], 2.f ),  "Matrix Identity Tx does not match", theTest );
}


UNITTEST( "Static Make", "Matrix", 0 ) {
    float identityValues[16]        = { 1.f,  0.f,  0.f,  0.f,    0.f,  1.f,  0.f,  0.f,    0.f,  0.f,  1.f,  0.f,      0.f, 0.f, 0.f, 1.f };
    float translation2DValues[16]   = { 1.f,  0.f,  0.f,  0.f,    0.f,  1.f,  0.f,  0.f,    0.f,  0.f,  1.f,  0.f,      5.f, 9.f, 0.f, 1.f };
    float uniformScale2DValues[16]  = { 7.f,  0.f,  0.f,  0.f,    0.f,  7.f,  0.f,  0.f,    0.f,  0.f,  1.f,  0.f,      0.f, 0.f, 0.f, 1.f };
    float xRotation2DValues[16]     = { 1.f,  0.f,  0.f,  0.f,    0.f,  0.8f, 0.6f, 0.f,    0.f, -0.6f, 0.8f, 0.f,      0.f, 0.f, 0.f, 1.f };
    float yRotation2DValues[16]     = { 0.8f, 0.f, -0.6f, 0.f,    0.f,  1.f,  0.f,  0.f,    0.6f, 0.f,  0.8f, 0.f,      0.f, 0.f, 0.f, 1.f };
    float zRotation2DValues[16]     = { 0.8f, 0.6f, 0.f,  0.f,   -0.6f, 0.8f, 0.f,  0.f,    0.f,  0.f,  1.f,  0.f,      0.f, 0.f, 0.f, 1.f };

    Matrix44 noTranslation = Matrix44::MakeTranslation2D( Vec2( 0.f, 0.f ) );
    Matrix44 translation2D = Matrix44::MakeTranslation2D( Vec2( 5.f, 9.f ) );
    UnitTest::VerifyResult( IsMostlyEqual( noTranslation, identityValues ),      "Translation matrix should be identity for translation=(0,0)", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( translation2D, translation2DValues ), "Translation matrix was incorrect for translation=(5,9)",      theTest );

    Matrix44 noScale = Matrix44::MakeUniformScale2D( 1.f );
    Matrix44 uniformScale2D = Matrix44::MakeUniformScale2D( 7.f );
    UnitTest::VerifyResult( IsMostlyEqual( noScale, identityValues ),              "UniformScale2D matrix should be identity for scale=1.0",                     theTest );
    UnitTest::VerifyResult( IsMostlyEqual( uniformScale2D, uniformScale2DValues ), "UniformScale2D matrix incorrect for scale=7.0 (note: scaleZ should be 1.0)", theTest );

    Matrix44 xNoRotation = Matrix44::MakeXRotationDegrees( 0.f );
    Matrix44 xRotation2D = Matrix44::MakeXRotationDegrees( 36.8698976f );
    UnitTest::VerifyResult( IsMostlyEqual( xNoRotation, identityValues ),    "XRotationDegrees matrix should be identity for rotationDegrees=0.0", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( xRotation2D, xRotation2DValues ), "XRotationDegrees matrix incorrect for 36.8698976 degrees",           theTest );

    Matrix44 yNoRotation = Matrix44::MakeYRotationDegrees( 0.f );
    Matrix44 yRotation2D = Matrix44::MakeYRotationDegrees( 36.8698976f );
    UnitTest::VerifyResult( IsMostlyEqual( yNoRotation, identityValues ),    "YRotationDegrees matrix should be identity for rotationDegrees=0.0", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( yRotation2D, yRotation2DValues ), "YRotationDegrees matrix incorrect for 36.8698976 degrees",           theTest );

    Matrix44 zNoRotation = Matrix44::MakeZRotationDegrees( 0.f );
    Matrix44 zRotation2D = Matrix44::MakeZRotationDegrees( 36.8698976f );
    UnitTest::VerifyResult( IsMostlyEqual( zNoRotation, identityValues ),    "ZRotationDegrees matrix should be identity for rotationDegrees=0.0",                        theTest );
    UnitTest::VerifyResult( IsMostlyEqual( zRotation2D, zRotation2DValues ), "ZRotationDegrees matrix incorrect for 36.8698976 degrees - iBasis should be (.8,.6), etc.", theTest );
}


UNITTEST( "Transforms", "Matrix", 0 ) {
    float identityValues[16]       = { 1.f,  0.f, 0.f, 0.f,       0.f, 1.f,  0.f, 0.f,      0.f,  0.f,  1.f,  0.f,      0.f, 0.f, 0.f, 1.f     };
    float translation2DValues[16]  = { 1.f,  0.f, 0.f, 0.f,       0.f, 1.f,  0.f, 0.f,      0.f,  0.f,  1.f,  0.f,      5.f, 9.f, 0.f, 1.f     };
    float uniformScale2DValues[16] = { 7.f,  0.f, 0.f, 0.f,       0.f, 7.f,  0.f, 0.f,      0.f,  0.f,  1.f,  0.f,      0.f, 0.f, 0.f, 1.f     };
    float zRotation2DValues[16]    = { 0.8f, 0.6f, 0.f, 0.f,    -0.6f, 0.8f, 0.f, 0.f,      0.f,  0.f,  1.f,  0.f,      0.f, 0.f, 0.f, 1.f     };
    float complexMatrixValues[16]  = { 2.f,  3.f, 4.f, 5.f,       6.f, 7.f,  8.f, 9.f,      10.f, 11.f, 12.f, 13.f,     14.f, 15.f, 16.f, 17.f };

    Vec2 vec2D( 5.f, 2.f );					// assume z=0, w can be 0 or 1 depending on function being called
    Vec3 vec3D( 6.f, 3.f, 2.f );			// w can be 0 or 1 depending on function being called
    Vec4 position4D( 7.f, 5.f, 3.f, 1.f );	// w = 1 for positions
    Vec4 vector4D( 7.f, 5.f, 3.f, 0.f );	// w = 0 for directions

    Matrix44 identity;
    Vec2 v2a = identity.TransformVector2D( vec2D );
    Vec2 p2a = identity.TransformPosition2D( vec2D );
    Vec3 v3a = identity.TransformVector3D( vec3D );
    Vec3 p3a = identity.TransformPosition3D( vec3D );
    Vec4 v4a = identity.TransformHomogeneousPoint3D( vector4D );
    Vec4 p4a = identity.TransformHomogeneousPoint3D( position4D );
    UnitTest::VerifyResult( IsMostlyEqual( v2a, vec2D ),      "Identity matrix should not modify point when calling TransformVector2D()",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p2a, vec2D ),      "Identity matrix should not modify point when calling TransformPosition2D()",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( v3a, vec3D ),      "Identity matrix should not modify point when calling TransformVector3D()",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p3a, vec3D ),      "Identity matrix should not modify point when calling TransformPosition3D()",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( v4a, vector4D ),   "Identity matrix should not modify point when calling TransformHomogeneousPoint3D()", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p4a, position4D ), "Identity matrix should not modify point when calling TransformHomogeneousPoint3D()", theTest );

    Matrix44 complexMatrix( complexMatrixValues );
    Vec2 v2b = complexMatrix.TransformVector2D( vec2D );
    Vec2 p2b = complexMatrix.TransformPosition2D( vec2D );
    Vec3 v3b = complexMatrix.TransformVector3D( vec3D );
    Vec3 p3b = complexMatrix.TransformPosition3D( vec3D );
    Vec4 v4b = complexMatrix.TransformHomogeneousPoint3D( vector4D );
    Vec4 p4b = complexMatrix.TransformHomogeneousPoint3D( position4D );
    Vec2 v2bCorrect( 22.f, 29.f );
    Vec2 p2bCorrect( 36.f, 44.f );
    Vec3 v3bCorrect( 50.f, 61.f, 72.f );
    Vec3 p3bCorrect( 64.f, 76.f, 88.f );
    Vec4 v4bCorrect( 74.f, 89.f, 104.f, 119.f );
    Vec4 p4bCorrect( 88.f, 104.f, 120.f, 136.f );
    UnitTest::VerifyResult( IsMostlyEqual( v2b, v2bCorrect ), "incorrect result for complexMatrix.TransformVector2D()",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p2b, p2bCorrect ), "incorrect result for complexMatrix.TransformPosition2D()",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( v3b, v3bCorrect ), "incorrect result for complexMatrix.TransformVector3D()",           theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p3b, p3bCorrect ), "incorrect result for complexMatrix.TransformPosition3D()",         theTest );
    UnitTest::VerifyResult( IsMostlyEqual( v4b, v4bCorrect ), "incorrect result for complexMatrix.TransformHomogeneousPoint3D()", theTest );
    UnitTest::VerifyResult( IsMostlyEqual( p4b, p4bCorrect ), "incorrect result for complexMatrix.TransformHomogeneousPoint3D()", theTest );
}
