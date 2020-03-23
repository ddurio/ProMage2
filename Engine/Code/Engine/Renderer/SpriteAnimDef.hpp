#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Utils/XMLUtils.hpp"


class SpriteDef;
class SpriteSheet;

enum SpriteAnimPlaybackType {
    SPRITE_ANIM_PLAYBACK_INVALID = -1,

    SPRITE_ANIM_PLAYBACK_ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4...
    SPRITE_ANIM_PLAYBACK_LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4...
    SPRITE_ANIM_PLAYBACK_PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2...

    NUM_SPRITE_ANIM_PLAYBACK_TYPES
};


class SpriteAnimDef {
    public:
    SpriteAnimDef( const XMLElement& element );
    SpriteAnimDef( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex,
        float durationSeconds, SpriteAnimPlaybackType playbackType = SPRITE_ANIM_PLAYBACK_LOOP, const std::string& sheetName = "" );

    static void Initialize( const std::string& filePath, const char* tagName );
    static const SpriteAnimDef* GetDefinition( const std::string& animName );

    float GetTimeFromFrame( int frameNum ) const;
    const SpriteDef& GetSpriteDefAtTime( float seconds ) const;
    const SpriteSheet& GetSpriteSheet() const;
    Strings GetEventsInTimeRange( float timeStart, float timeEnd ) const;
    float GetDuration() const;


    private:
    static std::map< std::string, SpriteAnimDef* > s_definitions;

    const SpriteSheet&		m_spriteSheet;
    int				        m_startSpriteIndex = -1;
    int				        m_endSpriteIndex = -1;
    float				    m_durationSeconds = 1.f;
    SpriteAnimPlaybackType	m_playbackType = SPRITE_ANIM_PLAYBACK_INVALID;
    Strings                 m_eventStrings;
    std::vector< float >    m_eventTimes;


    const SpriteSheet& GetSpriteSheetFromXML( const XMLElement& element );
    float ApplyLoopMode( float time ) const;
};


SpriteAnimPlaybackType ParseXMLAttribute( const XMLElement& element, const char* attributeName, SpriteAnimPlaybackType defaultValue );
