#include "Game/ItemDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Tags.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Item.hpp"
#include "Game/XMLUtils.hpp"


template<>
Definition<Item>::Definition( const XMLElement& element ) {
    // Parse Values
    m_defType                   = ParseXMLAttribute( element, "name",         "" );
    ItemSlot itemSlot           = ParseXMLAttribute( element, "slot",         ITEM_SLOT_NONE );
    std::string spriteName      = ParseXMLAttribute( element, "spriteSheet",  "" );
    GUARANTEE_OR_DIE( spriteName != "", "(ItemDef) Missing required attribute 'spriteSheet'" );

    const XMLElement* childEle = element.FirstChildElement();
    std::vector< Tags > itemSets;

    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "ItemSet" ) {
            std::string setName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( setName != "", "(ItemDef) ItemSet tag missing required attribute 'name'" );

            Tags itemTag;
            itemTag.SetTags( setName );
            itemSets.push_back( itemTag );
        } else if( tagName == "Portrait" ) {
            std::string animName = ParseXMLAttribute( *childEle, "anim", "" );
            GUARANTEE_OR_DIE( animName != "", "(ItemDef) Portrait tag missing required attribute 'name'" );

            float portraitTime = ParseXMLAttribute( *childEle, "time", 0.f );

            m_properties.SetValue( "portraitAnim", animName );
            m_properties.SetValue( "portraitTime", portraitTime );
        }

        childEle = childEle->NextSiblingElement();
    }

    // Set Properties
    m_properties.SetValue( "slot",          itemSlot );
    m_properties.SetValue( "spriteSheet",   spriteName );
    m_properties.SetValue( "itemSets",      itemSets );


    g_theDevConsole->PrintString( Stringf( "(ItemDef) Loaded new ItemDef (%s)", m_defType.c_str() ) );

    m_defType = StringToLower( m_defType );
    s_definitions[m_defType] = this;
}


template<>
void Definition<Item>::Define( Item& theObject ) const {
    UNUSED( theObject );
    // Define item specific things here.. roll the dice, etc

    //theObject.m_strength     = g_RNG->GetRandomFloatInRange( strRange );
}


