#include "Game/ItemDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Tags.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Item.hpp"
#include "Game/XMLUtils.hpp"


template<>
Definition<Item>::Definition( const XMLElement& element ) {
    // Parse Values
    m_defType                   = ParseXMLAttribute( element, "name",         "" );
    int moneyValue              = ParseXMLAttribute( element, "value",        100 );
    ItemSlot itemSlot           = ParseXMLAttribute( element, "slot",         ITEM_SLOT_NONE );
    std::string spriteName      = ParseXMLAttribute( element, "spriteSheet",  "" );
    GUARANTEE_OR_DIE( spriteName != "", "(ItemDef) Missing required attribute 'spriteSheet'" );


    const XMLElement* childEle = element.FirstChildElement();
    std::vector< Tags > itemSets;
    std::string attackAnim = ANIM_PAPER_DOLL_IDLE;

    float portraitTime = 0.f;
    std::string portraitAnim = Stringf( "%s.%s", ANIM_PAPER_DOLL_IDLE, "Down" );

    // Loop through child tags
    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "ItemSet" ) {
            std::string setName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( setName != "", "(ItemDef) ItemSet tag missing required attribute 'name'" );

            Tags itemTag;
            itemTag.SetTags( setName );
            itemSets.push_back( itemTag );
        } else if( tagName == "Portrait" ) {
            portraitAnim  = ParseXMLAttribute( *childEle, "anim", portraitAnim );
            float time  = ParseXMLAttribute( *childEle, "time", -1.f );
            int frame = ParseXMLAttribute( *childEle, "frame", -1 );

            if( time >= 0.f ) {
                portraitTime = time;
            } else if( frame >= 0 ) {
                const SpriteAnimDef* anim = SpriteAnimDef::GetDefinition( portraitAnim );
                portraitTime = anim->GetTimeFromFrame( frame );
            }
        } else if( tagName == "Attack" ) {
            std::string animShortName = ParseXMLAttribute( *childEle, "anim", "" );
            GUARANTEE_OR_DIE( animShortName != "", "(ItemDef) Attack tag missing required attribute 'anim'" );
        }

        childEle = childEle->NextSiblingElement();
    }

    // Set Properties
    m_properties.SetValue( "slot",          itemSlot );
    m_properties.SetValue( "value",         moneyValue );
    m_properties.SetValue( "spriteSheet",   spriteName );
    m_properties.SetValue( "itemSets",      itemSets );

    m_properties.SetValue( "attackAnim",    attackAnim );
    m_properties.SetValue( "portraitAnim",  portraitAnim );
    m_properties.SetValue( "portraitTime",  portraitTime );


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
