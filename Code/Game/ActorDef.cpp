#include "Game/ActorDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"


template<>
Definition<Actor>::Definition( const XMLElement& element ) {
    // Parse Values
    m_defType                   = ParseXMLAttribute( element, "name",         m_defType );

    bool canSee                 = ParseXMLAttribute( element, "canSee",  true );
    bool canWalk                = ParseXMLAttribute( element, "canWalk", true );
    bool canFly                 = ParseXMLAttribute( element, "canFly",  false );
    bool canSwim                = ParseXMLAttribute( element, "canSwim", false );

    FloatRange strength         = ParseXMLAttribute( element, "strength",     FloatRange::ZERO );
    FloatRange intelligence     = ParseXMLAttribute( element, "intelligence", FloatRange::ZERO );
    FloatRange agility          = ParseXMLAttribute( element, "agility",      FloatRange::ZERO );

    const XMLElement* childEle = element.FirstChildElement();

    Strings     bodyOptions;
    Strings     bodyItemSets;
    Strings     earOptions;
    Strings     hairOptions;
    std::string itemSetsCSV;

    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "Body" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Body tag missing required attribute 'spriteSheet'" );
            bodyOptions.push_back( spriteName );

            std::string setName = ParseXMLAttribute( *childEle, "itemSet", "" );
            bodyItemSets.push_back( setName );
        } else if( tagName == "Ears" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Ears tag missing required attribute 'spriteSheet'" );
            earOptions.push_back( spriteName );
        } else if( tagName == "Hair" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Hair tag missing required attribute 'spriteSheet''" );
            hairOptions.push_back( spriteName );
        } else if( tagName == "ItemSet" ) {
            std::string setName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( setName != "", "(ActorDef) ItemSet tag missing required attribute 'name'" );
            itemSetsCSV = Stringf( "%s,%s", itemSetsCSV.c_str(), setName.c_str() );
        }

        childEle = childEle->NextSiblingElement();
    }


    // Set Properties
    m_properties.SetValue( "canSee",        canSee );
    m_properties.SetValue( "canWalk",       canWalk );
    m_properties.SetValue( "canFly",        canFly );
    m_properties.SetValue( "canSwim",       canSwim );
    m_properties.SetValue( "strength",      strength );
    m_properties.SetValue( "intelligence",  intelligence );
    m_properties.SetValue( "agility",       agility );
    m_properties.SetValue( "bodySprites",   bodyOptions );
    m_properties.SetValue( "bodyItemSets",  bodyItemSets );
    m_properties.SetValue( "earSprites",    earOptions );
    m_properties.SetValue( "hairSprites",   hairOptions );
    m_properties.SetValue( "validItemSets", itemSetsCSV );


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


    // Body appearance
    Strings bodyOptions;
    Strings bodyItemSets;
    Strings earOptions;
    Strings hairOptions;
    bodyOptions  = m_properties.GetValue( "bodySprites",  bodyOptions );
    bodyItemSets = m_properties.GetValue( "bodyItemSets", bodyItemSets );
    earOptions   = m_properties.GetValue( "earSprites",   earOptions );
    hairOptions  = m_properties.GetValue( "hairSprites",  hairOptions );

    int numBody = (int)bodyOptions.size();
    int numEars = (int)earOptions.size();
    int numHair = (int)hairOptions.size();

    int bodyIndex = g_RNG->GetRandomIntLessThan( numBody );
    int earIndex  = g_RNG->GetRandomIntLessThan( numEars );
    int hairIndex = g_RNG->GetRandomIntLessThan( numHair );

    theObject.m_paperDollSprites[PAPER_DOLL_BODY] = bodyOptions[bodyIndex];
    theObject.m_paperDollSprites[PAPER_DOLL_EARS] = earOptions[earIndex];
    theObject.m_paperDollSprites[PAPER_DOLL_HAIR] = hairOptions[hairIndex];


    // Item Sets
    std::string itemSetsCSV = "";
    itemSetsCSV = m_properties.GetValue( "validItemSets", itemSetsCSV );

    std::string bodyItemSet = bodyItemSets[bodyIndex];

    if( bodyItemSet != "" ) {
        itemSetsCSV = Stringf( "%s,%s", itemSetsCSV.c_str(), bodyItemSet.c_str() );
    }

    theObject.m_inventory->AddItemSets( itemSetsCSV );
}
