#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

// Used for VertexBuffer Layouts
enum AttributeType {
    ATTR_TYPE_NULL,
    ATTR_TYPE_FLOAT,
    ATTR_TYPE_VEC2,
    ATTR_TYPE_VEC3,
    ATTR_TYPE_RGBA,
};


struct BufferAttribute {
    public:
    std::string semanticName = "";
    AttributeType type = ATTR_TYPE_NULL;
    unsigned int offset = 0;

    BufferAttribute() {};
    explicit BufferAttribute( std::string name, AttributeType typeIn, unsigned int offsetIn ) : semanticName( name ), type( typeIn ), offset( offsetIn ) {};
    bool isNull() const { return (type == ATTR_TYPE_NULL); };
};


struct VertexMaster {
    public:
    Vec3 position = Vec3::ZERO;
    Rgba color = Rgba::WHITE;
    Vec2 uvTexCoords = Vec2::ZERO;

    Vec3 tangent = Vec3::RIGHT;
    Vec3 bitangent = -Vec3::SCREEN;
    Vec3 normal = Vec3::UP;

    VertexMaster() {};
    explicit VertexMaster( const Vec3& positionIn, const Rgba& colorIn, const Vec2& uvTexCoordsIn, const Vec3& tangentIn, const Vec3& bitangentIn, const Vec3& normalIn );

    bool operator==( const VertexMaster& vertToCompare ) const;
};


typedef void( *CopyFromMasterCallback )(void* outArray, const std::vector<VertexMaster>& inList);

