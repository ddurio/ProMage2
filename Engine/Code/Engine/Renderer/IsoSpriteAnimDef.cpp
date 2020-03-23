#include "Engine/Renderer/IsoSpriteAnimDef.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


std::map< std::string, IsoSpriteAnimDef* > IsoSpriteAnimDef::s_definitions;
std::vector<Vec2> IsoSpriteAnimDef::s_directions;


IsoSpriteAnimDef::IsoSpriteAnimDef( const XMLElement& element ) {
    if( (int)s_directions.size() != 8 ) {
        InitializeDirections();
    }

    std::string name = ParseXMLAttribute( element, "name", "UNKNOWN" );
    GUARANTEE_OR_DIE( name != "UNKNOWN", "(IsoSpriteAnimDef) Missing required XML attribute name." );

    m_anims.resize( 8 );
    
    SpriteSheet invalidSheet = SpriteSheet( "", IntVec2::ONE );
    SpriteAnimDef invalidAnim = SpriteAnimDef( invalidSheet, 0, 0, 0 );

    m_anims[0] = ParseXMLAttribute( element, "animRight",     invalidAnim );
    m_anims[1] = ParseXMLAttribute( element, "animUpRight",   invalidAnim );
    m_anims[2] = ParseXMLAttribute( element, "animUp",        invalidAnim );
    m_anims[3] = ParseXMLAttribute( element, "animUpLeft",    invalidAnim );
    m_anims[4] = ParseXMLAttribute( element, "animLeft",      invalidAnim );
    m_anims[5] = ParseXMLAttribute( element, "animDownLeft",  invalidAnim );
    m_anims[6] = ParseXMLAttribute( element, "animDown",      invalidAnim );
    m_anims[7] = ParseXMLAttribute( element, "animDownRight", invalidAnim );

    g_theDevConsole->PrintString( Stringf( "(IsoSpriteAnimDef) Loaded new IsoSpriteAnimDef (%s)", name.c_str() ) );
    std::string lowerName = StringToLower( name );
    s_definitions[lowerName] = this;
}


IsoSpriteAnimDef::IsoSpriteAnimDef( SpriteAnimDef* anims[8] ) {
    if( (int)s_directions.size() != 8 ) {
        InitializeDirections();
    }

    // Empty by default, add all 8 SpriteAnims
    for( int animIndex = 0; animIndex < 8; animIndex++ ) {
        m_anims.push_back( anims[animIndex] );
    }
}


void IsoSpriteAnimDef::Initialize( const std::string& filePath, const char* tagName ) {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( filePath, document );

    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        new IsoSpriteAnimDef( *element ); // Upon construction, adds self to static registry
    }
}


const IsoSpriteAnimDef* IsoSpriteAnimDef::GetDefinition( const std::string& animName ) {
    std::string lowerName = StringToLower( animName );
    auto animIter = s_definitions.find( lowerName );

    if( animIter != s_definitions.end() ) {
        return animIter->second;
    } else {
        ERROR_AND_DIE( Stringf( "(IsoSpriteAnimDef) Unknown IsoSpriteAnimDef name (%s)", animName.c_str() ) );
    }
}


const SpriteDef& IsoSpriteAnimDef::GetSpriteDefAtTimeAndDirection( float seconds, const Vec2& localDir ) const {
    const SpriteAnimDef* anim = GetAnimFromDirection( localDir );
    return anim->GetSpriteDefAtTime( seconds );
}


Strings IsoSpriteAnimDef::GetEventsInTimeRangeAndDirection( float timeStart, float timeEnd, const Vec2& localDir ) const {
    const SpriteAnimDef* anim = GetAnimFromDirection( localDir );
    return anim->GetEventsInTimeRange( timeStart, timeEnd );
}


float IsoSpriteAnimDef::GetDuration() const {
    const SpriteAnimDef* anim = m_anims[0];
    return anim->GetDuration();
}


void IsoSpriteAnimDef::InitializeDirections() const {
    s_directions.clear();

    s_directions.push_back( Vec2::RIGHT );
    s_directions.push_back( Vec2( 1.f, 1.f ).GetNormalized() );
    s_directions.push_back( Vec2::UP );
    s_directions.push_back( Vec2( -1.f, 1.f ).GetNormalized() );
    s_directions.push_back( Vec2::LEFT );
    s_directions.push_back( Vec2( -1.f, -1.f ).GetNormalized() );
    s_directions.push_back( Vec2::DOWN );
    s_directions.push_back( Vec2( -1.f, 1.f ).GetNormalized() );
}


const SpriteAnimDef* IsoSpriteAnimDef::GetAnimFromDirection( const Vec2& localDir ) const {
    int bestIndex = 0;
    float bestValue = DotProduct( localDir, s_directions[0] );

    for( int dirIndex = 1; dirIndex < 8; dirIndex++ ) {
        float value = DotProduct( localDir, s_directions[dirIndex] );

        if( value > bestValue ) {
            bestIndex = dirIndex;
            bestValue = value;
        }
    }

    return m_anims[bestIndex];
}


const SpriteAnimDef* ParseXMLAttribute( const XMLElement& element, const char* attributeName, const SpriteAnimDef& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );
    attributeValue = StringToLower( attributeValue );

    GUARANTEE_OR_DIE( attributeValue != "", Stringf( "(IsoSpriteAnim) Missing requested attribute (%s)", attributeName ) );

    UNUSED( defaultValue ); // ERROR_AND_DIE in getDef takes care of default case
    return SpriteAnimDef::GetDefinition( attributeValue );
}
