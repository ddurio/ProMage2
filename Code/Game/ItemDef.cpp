#include "Game/ItemDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Tags.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
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
    std::string sheetName      = ParseXMLAttribute( element, "spriteSheet",  "" );
    GUARANTEE_OR_DIE( sheetName != "", "(ItemDef) Missing required attribute 'spriteSheet'" );


    const XMLElement* childEle = element.FirstChildElement();
    std::vector< Tags > itemSets;

    std::string attackAnim = ANIM_PAPER_DOLL_IDLE;
    float attackRange = -1.f;
    float attackDamage = 0.f;
    float attackConeWidth = 0.f;
    float attackConeDot = 1.f;

    // Setup default portrait
    std::string defaultAnimName = Stringf( "%s.%s", ANIM_PAPER_DOLL_IDLE, "Down" );
    const SpriteAnimDef* defaultAnim = SpriteAnimDef::GetDefinition( defaultAnimName );
    SpriteDef defaultSprite = defaultAnim->GetSpriteDefAtTime( 0.f );

    AABB2 defaultUVs;
    defaultSprite.GetUVs( defaultUVs.mins, defaultUVs.maxs );

    const SpriteSheet& itemSheet = SpriteSheet::GetSpriteSheet( sheetName );
    std::string itemTexture = itemSheet.GetTexturePath();

    SpriteDef portrait = SpriteDef( defaultUVs.mins, defaultUVs.maxs, itemTexture );


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
            IntVec2 spriteCoords = ParseXMLAttribute( *childEle, "spriteCoords", IntVec2::NEGONE );

            if( spriteCoords != IntVec2::NEGONE ) {
                portrait = itemSheet.GetSpriteDef( spriteCoords );
            } else {
                std::string portraitAnim = ParseXMLAttribute( *childEle, "anim", "" );
                const SpriteAnimDef* anim = SpriteAnimDef::GetDefinition( portraitAnim );
                GUARANTEE_OR_DIE( anim != nullptr, Stringf( "(ItemDef) Portrait tag invalid attribute value: 'anim' = '%s'", portraitAnim.c_str() ) );

                float time = ParseXMLAttribute( *childEle, "time", -1.f );
                int frame = ParseXMLAttribute( *childEle, "frame", -1 );
                GUARANTEE_OR_DIE( time >= 0.f || frame >= 0, "(ItemDef) Portrait tag missing required attribute 'time' OR 'frame'" );

                if( time < 0.f ) {
                    time = anim->GetTimeFromFrame( frame );
                }

                SpriteDef sprite = anim->GetSpriteDefAtTime( time ); // Has the wrong texturePath.. replace with item texture
                AABB2 portraitUVs;
                sprite.GetUVs( portraitUVs.mins, portraitUVs.maxs );

                portrait = SpriteDef( portraitUVs.mins, portraitUVs.maxs, itemTexture );
            }
        } else if( tagName == "Attack" ) {
            attackAnim   = ParseXMLAttribute( *childEle, "anim",            attackAnim );
            attackRange  = ParseXMLAttribute( *childEle, "range",           attackRange );
            attackDamage = ParseXMLAttribute( *childEle, "damage",          attackDamage );
            attackConeWidth = ParseXMLAttribute( *childEle, "coneWidth",    attackConeWidth );
            attackConeDot = CosDegrees( attackConeWidth * 0.5f );
        } else if( tagName == "Consumable" ) {
            m_properties.SetValue( "isConsumable", true );

            std::string addItemSet = ParseXMLAttribute( *childEle, "itemSet", std::string() );
            m_properties.SetValue( "onConsumeItemSet", addItemSet );
        }

        childEle = childEle->NextSiblingElement();
    }

    // Set Properties
    m_properties.SetValue( "slot",              itemSlot        );
    m_properties.SetValue( "value",             moneyValue      );
    m_properties.SetValue( "spriteSheet",       sheetName      );
    m_properties.SetValue( "itemSets",          itemSets        );

    m_properties.SetValue( "attackAnim",        attackAnim      );
    m_properties.SetValue( "attackRange",       attackRange     );
    m_properties.SetValue( "attackDamage",      attackDamage    );
    m_properties.SetValue( "attackConeWidth",   attackConeWidth );
    m_properties.SetValue( "attackConeDot",     attackConeDot   );

    m_properties.SetValue( "portrait",          portrait        );


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
