#include "Engine/Renderer/SpriteAnimDef.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


std::map< std::string, SpriteAnimDef* > SpriteAnimDef::s_definitions;


SpriteAnimDef::SpriteAnimDef( const XMLElement& element ) :
    m_spriteSheet( GetSpriteSheetFromXML(element) ) {
    std::string name = ParseXMLAttribute( element, "name", "UNKNOWN" );
    GUARANTEE_OR_DIE( name != "UNKNOWN", "(SpriteAnimDef) Missing required XML attribute name." );

    // Start Index
    IntVec2 startCoords = ParseXMLAttribute( element, "startCoords", IntVec2::NEGONE );

    if( startCoords == IntVec2::NEGONE ) {
        m_startSpriteIndex = ParseXMLAttribute( element, "startIndex", m_startSpriteIndex );
    } else {
        m_startSpriteIndex = m_spriteSheet.GetSpriteIndexFromCoords( startCoords );
    }

    GUARANTEE_OR_DIE( m_startSpriteIndex >= 0, Stringf( "(SpriteAnimDef) Invalid start index %d", m_startSpriteIndex ) );

    // End Index
    IntVec2 endCoords = ParseXMLAttribute( element, "endCoords", IntVec2::NEGONE );

    if( endCoords == IntVec2::NEGONE ) {
        m_endSpriteIndex = ParseXMLAttribute( element, "endIndex", m_endSpriteIndex );
    } else {
        m_endSpriteIndex = m_spriteSheet.GetSpriteIndexFromCoords( endCoords );
    }

    GUARANTEE_OR_DIE( m_endSpriteIndex >= 0, Stringf( "(SpriteAnimDef) Invalid end index %d", m_endSpriteIndex ) );

    // Anim Duration
    m_durationSeconds = ParseXMLAttribute( element, "duration", m_durationSeconds );
    GUARANTEE_OR_DIE( m_durationSeconds > 0.f, Stringf( "(SpriteAnimDef) Invalid duration %f", m_durationSeconds ) );

    // Playback
    m_playbackType = ParseXMLAttribute( element, "playbackType", m_playbackType );
    GUARANTEE_OR_DIE( m_playbackType != SPRITE_ANIM_PLAYBACK_INVALID, "(SpriteAnimDef) Missing required XML attribute 'playbackType'." );

    // Events
    const XMLElement* childEle = element.FirstChildElement( "Event" );

    while( childEle != nullptr ) {
        std::string eventStr = ParseXMLAttribute( *childEle, "command", "" );
        GUARANTEE_OR_DIE( eventStr != "", "(SpriteAnimDef) Event tag missing required XML attribute 'command'." );

        float eventTime = ParseXMLAttribute( *childEle, "time", -1.f );
        int frameTime = ParseXMLAttribute( *childEle, "frame", -1 );
        GUARANTEE_OR_DIE( eventTime >= 0.f || frameTime >= 0, "(SpriteAnimDef) Event tag missing required XML attribute 'time' OR 'frame'." );

        if( eventTime < 0 ) {
            eventTime = GetTimeFromFrame( frameTime );
        }

        m_eventStrings.push_back( eventStr );
        m_eventTimes.push_back( eventTime );

        childEle = childEle->NextSiblingElement( "Event" );
    }




    g_theDevConsole->PrintString( Stringf( "(SpriteAnimDef) Loaded new SpriteAnimDef (%s)", name.c_str() ) );
    std::string lowerName = StringToLower( name );
    s_definitions[lowerName] = this;
}


SpriteAnimDef::SpriteAnimDef( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType /*= SPRITE_ANIM_PLAYBACK_LOOP */, const std::string& sheetName /*= ""*/ ) :
    m_spriteSheet( sheet ),
    m_startSpriteIndex( startSpriteIndex ),
    m_endSpriteIndex( endSpriteIndex ),
    m_durationSeconds( durationSeconds ),
    m_playbackType( playbackType ) {
    GUARANTEE_OR_DIE( (m_endSpriteIndex - m_startSpriteIndex) >= 0, "Invalid Start/End Sprite Index" );

    if( sheetName != "" ) {
        std::string lowerName = StringToLower( sheetName );
        s_definitions[lowerName] = this;
    }
}


void SpriteAnimDef::Initialize( const std::string& filePath, const char* tagName ) {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( filePath, document );

    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        new SpriteAnimDef( *element ); // Upon construction, adds self to static registry
    }
}


const SpriteAnimDef* SpriteAnimDef::GetDefinition( const std::string& animName ) {
    std::string lowerName = StringToLower( animName );
    auto animIter = s_definitions.find( lowerName );

    if( animIter != s_definitions.end() ) {
        return animIter->second;
    } else {
        ERROR_AND_DIE( Stringf( "(SpriteAnimDef) Unknown SpriteAnimDef name (%s)", animName.c_str() ) );
    }
}


float SpriteAnimDef::GetTimeFromFrame( int frameIndex ) const {
    int numFrames = m_endSpriteIndex - m_startSpriteIndex + 1;
    float animTime = (float)frameIndex / (float)numFrames;

    return animTime;
}


const SpriteDef& SpriteAnimDef::GetSpriteDefAtTime( float seconds ) const {
    int numSprites = m_endSpriteIndex - m_startSpriteIndex + 1;
    float frameDurationSeconds = m_durationSeconds / numSprites;
    int frameIndex = (int)(seconds / frameDurationSeconds);
    int spriteIndex = -1;

    if( m_playbackType == SPRITE_ANIM_PLAYBACK_ONCE ) {
        frameIndex = Clamp( frameIndex, 0, numSprites - 1 );
        spriteIndex = m_startSpriteIndex + frameIndex;
    } else if( m_playbackType == SPRITE_ANIM_PLAYBACK_LOOP ) {
        frameIndex = frameIndex % numSprites;
        spriteIndex = m_startSpriteIndex + frameIndex;
    } else if( m_playbackType == SPRITE_ANIM_PLAYBACK_PINGPONG ) {
        int numCycleFrames = (numSprites - 2) + numSprites; // Not counting start & end sprites, double it
        frameIndex = frameIndex % numCycleFrames;

        if( frameIndex < numSprites - 1 ) {
            spriteIndex = m_startSpriteIndex + frameIndex;
        } else {
            int reverseIndex = numCycleFrames - frameIndex;
            spriteIndex = m_startSpriteIndex + reverseIndex;
        }
    }

    return m_spriteSheet.GetSpriteDef( spriteIndex );
}


const SpriteSheet& SpriteAnimDef::GetSpriteSheet() const {
    return m_spriteSheet;
}


Strings SpriteAnimDef::GetEventsInTimeRange( float timeStart, float timeEnd ) const {
    float timeStartNorm = ApplyLoopMode( timeStart );
    float timeEndNorm = ApplyLoopMode( timeEnd );

    int numEvents = (int)m_eventTimes.size();
    Strings eventCommands;

    for( int eventIndex = 0; eventIndex < numEvents; eventIndex++ ) {
        float eventTime = m_eventTimes[eventIndex];

        if( (eventTime  >= timeStartNorm && eventTime < timeEndNorm ) ||                                        // Normal between time range
            (timeStartNorm > timeEndNorm && eventTime >= timeStartNorm && eventTime <= m_durationSeconds) ||    // Loop mode: at the end of anim
            (timeStartNorm > timeEndNorm && eventTime >= 0.f && eventTime < timeEndNorm) ) {                    // Loop mode: at the beginning of anim
            // SDSTFIXME: This doesn't work for ping pong
            const std::string& command = m_eventStrings[eventIndex];
            eventCommands.push_back( command );
        }
    }

    return eventCommands;
}


float SpriteAnimDef::GetDuration() const {
    return m_durationSeconds;
}


const SpriteSheet& SpriteAnimDef::GetSpriteSheetFromXML( const XMLElement& element ) {
    std::string spriteSheetName = ParseXMLAttribute( element, "spriteSheet", "UNKNOWN" );
    GUARANTEE_OR_DIE( spriteSheetName != "UNKNOWN", "(SpriteAnimDef) Missing required XML attribute spriteSheet." );
    return SpriteSheet::GetSpriteSheet( spriteSheetName );
}


float SpriteAnimDef::ApplyLoopMode( float time ) const {
    float normTime = time;

    if( m_playbackType == SPRITE_ANIM_PLAYBACK_LOOP ) {
        normTime = fmodf( time, m_durationSeconds );
    } else if( m_playbackType == SPRITE_ANIM_PLAYBACK_ONCE ) {
        normTime = Clamp( time, 0.f, m_durationSeconds );
    } else if( m_playbackType == SPRITE_ANIM_PLAYBACK_PINGPONG ) {
        normTime = fmodf( time, m_durationSeconds * 2.f );
    }

    return normTime;
}


SpriteAnimPlaybackType ParseXMLAttribute( const XMLElement& element, const char* attributeName, SpriteAnimPlaybackType defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );
    attributeValue = StringToLower( attributeValue );

    if( attributeValue == "loop" ) {
        return SPRITE_ANIM_PLAYBACK_LOOP;
    } else if( attributeValue == "once") {
        return SPRITE_ANIM_PLAYBACK_ONCE;
    } else if( attributeValue == "pingpong") {
        return SPRITE_ANIM_PLAYBACK_PINGPONG;
    }

    return defaultValue;
}
