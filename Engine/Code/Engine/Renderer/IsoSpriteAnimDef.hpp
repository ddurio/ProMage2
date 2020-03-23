#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/SpriteAnimDef.hpp"


class SpriteAnimDef;


class IsoSpriteAnimDef {
    public:
    IsoSpriteAnimDef( const XMLElement& element );
    IsoSpriteAnimDef( SpriteAnimDef* anims[8] );

    static void Initialize( const std::string& filePath, const char* tagName );
    static const IsoSpriteAnimDef* GetDefinition( const std::string& animName );

    const SpriteDef& GetSpriteDefAtTimeAndDirection( float seconds, const Vec2& localDir ) const;
    Strings GetEventsInTimeRangeAndDirection( float timeStart, float timeEnd, const Vec2& localDir ) const;

    float GetDuration() const;

    private:
    static std::map< std::string, IsoSpriteAnimDef* > s_definitions;
    static std::vector<Vec2> s_directions;

    std::vector< const SpriteAnimDef* > m_anims;

    void InitializeDirections() const;
    const SpriteAnimDef* GetAnimFromDirection( const Vec2& localDir ) const;
};


const SpriteAnimDef* ParseXMLAttribute( const XMLElement& element, const char* attributeName, const SpriteAnimDef& defaultValue );
