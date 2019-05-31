#include "Game/ActorDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"

/*
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"
#include "Game/XMLUtils.hpp"


std::map<std::string, ActorDef*> ActorDef::s_actorDefs;

ActorDef::ActorDef( const XMLElement& element ) {
    m_actorType    = ParseXMLAttribute( element, "name",         m_actorType );
    m_spriteCoords = ParseXMLAttribute( element, "spriteCoords", m_spriteCoords );

    m_canSee       = ParseXMLAttribute( element, "canSee",  m_canSee );
    m_canWalk      = ParseXMLAttribute( element, "canWalk", m_canWalk );
    m_canFly       = ParseXMLAttribute( element, "canFly",  m_canFly );
    m_canSwim      = ParseXMLAttribute( element, "canSwim", m_canSwim );

    m_strength     = ParseXMLAttribute( element, "strength",     m_strength );
    m_intelligence = ParseXMLAttribute( element, "intelligence", m_intelligence );
    m_agility      = ParseXMLAttribute( element, "agility",      m_agility );

    g_theDevConsole->PrintString( Stringf( "(ActorDef) Loaded new ActorDef (%s)", m_actorType.c_str() ) );

    s_actorDefs[m_actorType] = this;
}


void ActorDef::InitializeActorDefs() {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( DATA_ACTOR_DEFS, document );

    std::string imageFilePath   = ParseXMLAttribute( root, "imageFilePath", "" );
    IntVec2     imageLayout     = ParseXMLAttribute( root, "gridLayout",    IntVec2::ZERO );
    int         imagePPU        = ParseXMLAttribute( root, "ppu",           1 );

    SpriteSheet* sprites = new SpriteSheet( imageFilePath, imageLayout );

    const char* tagName = "ActorDefinition";
    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        ActorDef* actorDef = new ActorDef( *element ); // Upon construction, adds self to static registry
        actorDef->m_spriteSheet = sprites;
        actorDef->m_spritePPU = imagePPU;

        Vec2 uvMins = Vec2::ZERO;
        Vec2 uvMaxs = Vec2::ONE;
        sprites->GetSpriteDef( actorDef->m_spriteCoords ).GetUVs(uvMins, uvMaxs);
        actorDef->m_spriteUVs = AABB2( uvMins, uvMaxs );
    }
}


void ActorDef::DestroyActorDefs() {
    auto actorDefIter = s_actorDefs.begin();
    SpriteSheet* sprites = actorDefIter->second->m_spriteSheet;
    delete sprites;

    for( actorDefIter; actorDefIter != s_actorDefs.end(); actorDefIter++ ) {
        ActorDef* actorDef = actorDefIter->second;
        delete actorDef;
        actorDefIter->second = nullptr;
    }
}


const ActorDef* ActorDef::GetActorDef( std::string actorType ) {
     auto actorDefIter = s_actorDefs.find( actorType );

    if( actorDefIter != s_actorDefs.end() ) {
        return actorDefIter->second;
    } else {
        return nullptr;
    }
}


const std::string& ActorDef::GetActorType() const {
    return m_actorType;
}


std::string ActorDef::GetTexturePath() const {
    return m_spriteSheet->GetTexturePath();
}


int ActorDef::GetPPU() const {
    return m_spritePPU;
}


const AABB2& ActorDef::GetUVs() const {
    return m_spriteUVs;
}


void ActorDef::Define( Actor& actor ) const {
    actor.m_inventory = new Inventory( actor.m_map );

    actor.m_strength     = g_RNG->GetRandomFloatInRange( m_strength );
    actor.m_intelligence = g_RNG->GetRandomFloatInRange( m_intelligence );
    actor.m_agility      = g_RNG->GetRandomFloatInRange( m_agility );
}
*/


//template<>
//std::map< std::string, Definition<Actor>* > Definition<Actor>::s_definitions;


template<>
Definition<Actor>::Definition( const XMLElement& element ) {
    // Parse Values
    m_defType                   = ParseXMLAttribute( element, "name",         m_defType );
    IntVec2 spriteCoords        = ParseXMLAttribute( element, "spriteCoords", IntVec2::ZERO );

    bool canSee                 = ParseXMLAttribute( element, "canSee",  true );
    bool canWalk                = ParseXMLAttribute( element, "canWalk", true );
    bool canFly                 = ParseXMLAttribute( element, "canFly",  false );
    bool canSwim                = ParseXMLAttribute( element, "canSwim", false );

    FloatRange strength         = ParseXMLAttribute( element, "strength",     FloatRange::ZERO );
    FloatRange intelligence     = ParseXMLAttribute( element, "intelligence", FloatRange::ZERO );
    FloatRange agility          = ParseXMLAttribute( element, "agility",      FloatRange::ZERO );

    const XMLElement* spriteEle = element.FirstChildElement( "Sprite" );

    std::string imageFilePath   = ParseXMLAttribute( *spriteEle, "imageFilePath", "" );
    IntVec2     imageLayout     = ParseXMLAttribute( *spriteEle, "gridLayout",    IntVec2::ZERO );
    float       imagePPU        = ParseXMLAttribute( *spriteEle, "ppu",           1.f );

    SpriteSheet* sprites = new SpriteSheet( imageFilePath, imageLayout );

    AABB2 spriteUVs = AABB2::ZEROTOONE;
    sprites->GetSpriteDef( spriteCoords ).GetUVs( spriteUVs.mins, spriteUVs.maxs );


    // Set Properties
    m_properties.SetValue( "spriteCoords",  spriteCoords );
    m_properties.SetValue( "canSee",        canSee );
    m_properties.SetValue( "canWalk",       canWalk );
    m_properties.SetValue( "canFly",        canFly );
    m_properties.SetValue( "canSwim",       canSwim );
    m_properties.SetValue( "strength",      strength );
    m_properties.SetValue( "intelligence",  intelligence );
    m_properties.SetValue( "agility",       agility );
    m_properties.SetValue( "spriteSheet",   imageFilePath );
    m_properties.SetValue( "spritePPU",     imagePPU );
    m_properties.SetValue( "spriteUVs",     spriteUVs );


    g_theDevConsole->PrintString( Stringf( "(ActorDef) Loaded new ActorDef (%s)", m_defType.c_str() ) );

    m_defType = StringToLower( m_defType );
    s_definitions[m_defType] = this;
}


template<>
void Definition<Actor>::Define( Actor& theObject ) const {
    theObject.m_inventory = new Inventory( theObject.m_map );

    const FloatRange strRange = m_properties.GetValue( "strength",     FloatRange( 0.f, 0.f ) );
    const FloatRange intRange = m_properties.GetValue( "intelligence", FloatRange::ZERO );
    const FloatRange agiRange = m_properties.GetValue( "agility",      FloatRange::ZERO );

    theObject.m_strength     = g_RNG->GetRandomFloatInRange( strRange );
    theObject.m_intelligence = g_RNG->GetRandomFloatInRange( intRange );
    theObject.m_agility      = g_RNG->GetRandomFloatInRange( agiRange );
}


