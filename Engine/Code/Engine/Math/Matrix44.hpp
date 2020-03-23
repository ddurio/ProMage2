#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"


struct Transform2D;

enum RotationOrder {
    ROTATION_ORDER_XYZ,     // Unreal / Squirrel Default
    ROTATION_ORDER_ZXY,     // Unity  / Forseth  Default

    ROTATION_ORDER_DEFAULT = ROTATION_ORDER_ZXY
};

enum {
    Ix, Iy, Iz, Iw,    Jx, Jy, Jz, Jw,    Kx, Ky, Kz, Kw,    Tx, Ty, Tz, Tw
};


struct Matrix44 {
    public:
    float m_values[16] = { 0 };

    Matrix44();
    Matrix44( float* valuesBasisMajor );
    Matrix44( const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation = Vec2::ZERO );
    Matrix44( const Vec3& iBasis, const Vec3& jBasis, const Vec3& kBasis, const Vec3& translation = Vec3::ZERO );
    Matrix44( const Vec4& iHomogeneous, const Vec4& jHomogeneous, const Vec4& kHomogeneous, const Vec4& translationHomogeneous = Vec4(0.f, 0.f, 0.f, 1.f) );
    Matrix44( const std::string& transform );
    ~Matrix44() {};

    float  operator[]( int index ) const; // Getter
    float& operator[]( int index );       // Setter
    bool   operator==( const Matrix44& compareTo ) const;
    bool   operator!=( const Matrix44& compareTo ) const;

    static const Matrix44 IDENTITY;
    static const Matrix44 ZERO;


    // 1D Constructors
    static Matrix44 MakeXRotationDegrees( float degrees );
    static Matrix44 MakeYRotationDegrees( float degrees );
    static Matrix44 MakeZRotationDegrees( float degrees );

    // 2D Constructors
    static Matrix44 MakeTransform( const Transform2D& transform );
    static Matrix44 MakeTranslation2D( const Vec2& translationXY );
    static Matrix44 MakeUniformScale2D( float uniformScaleXY );


    // 3D Constructors
    static Matrix44 MakeRotationDegrees3D( const Vec3& rotationDegrees, RotationOrder order = ROTATION_ORDER_DEFAULT );
    static Matrix44 MakeRotationBetweenVectors( const Vec3& fromA, const Vec3& toB );
    static Matrix44 MakeTranslation3D( const Vec3& translation );

    static Matrix44 MakeLookAt( const Vec3& fromPos, const Vec3& toPos, const Vec3& upRef );


    // Helper Functions
    Vec3 GetIBasis() const;
    Vec3 GetJBasis() const;
    Vec3 GetKBasis() const;
    Vec4 GetHomogeneousIBasis() const;
    Vec4 GetHomogeneousJBasis() const;
    Vec4 GetHomogeneousKBasis() const;
    Vec4 GetHomogeneousTBasis() const;
    Vec3 GetTranslation() const;
    void SetTranslation( const Vec2& translation );
    void SetTranslation( const Vec3& translation );

    void InvertOrthoNormal();
    Matrix44 GetInverse() const;
    void Invert();
    void Transpose();


    // 2D Transforms
    const Vec2 TransformVector2D( const Vec2& vectorQuantity ) const;
    const Vec2 TransformPosition2D( const Vec2& position ) const;

    // 3D Transforms
    const Vec3 TransformVector3D( const Vec3& vectorQuantity ) const;
    const Vec3 TransformPosition3D( const Vec3& position ) const;
    const Vec4 TransformHomogeneousPoint3D( const Vec4& homogeneousPoint ) const;
    const Matrix44 TransformMatrix( const Matrix44& matrix ) const;
};
