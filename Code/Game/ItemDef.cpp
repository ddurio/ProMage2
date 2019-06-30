#include "Game/ItemDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Tags.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/XMLUtils.hpp"


template<>
Definition<Item>::Definition( const XMLElement& element ) {
    // Base Values
    m_defType                   = ParseXMLAttribute( element, "name",         "" );
    GUARANTEE_OR_DIE( m_defType != "", "(ItemDef) Missing required attribute 'name'" );

    int moneyValue              = ParseXMLAttribute( element, "value",        100 );
    ItemSlot itemSlot           = ParseXMLAttribute( element, "slot",         ITEM_SLOT_NONE );
    std::string sheetName       = ParseXMLAttribute( element, "spriteSheet",  "" );
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

    // Proficiency Factor
    float proficiency = -1.f;
    int numTags = (int)itemSets.size();

    if( itemSlot == ITEM_SLOT_WEAPON ) {
        proficiency = 1.f; // Affects money value for weapons
    } else if( itemSlot != ITEM_SLOT_NONE ) {
        // Affects defense and money value for armor
        for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
            const Tags& tag = itemSets[tagIndex];

            if( tag.HasTags( "Light" ) ) {
                proficiency = Max( 0.f, proficiency );
            } else if( tag.HasTags( "Medium" ) ) {
                proficiency = Max( 1.f, proficiency );
            } else if( tag.HasTags( "Heavy" ) ) {
                proficiency = Max( 2.f, proficiency );
            }
        }
    }

    // Set Properties
    m_properties.SetValue( "slot",              itemSlot        );
    m_properties.SetValue( "baseValue",         moneyValue      );
    m_properties.SetValue( "spriteSheet",       sheetName       );
    m_properties.SetValue( "itemSets",          itemSets        );
    m_properties.SetValue( "proficiency",       proficiency     );

    m_properties.SetValue( "attackAnim",        attackAnim      );
    m_properties.SetValue( "attackRange",       attackRange     );
    m_properties.SetValue( "attackDamage",      attackDamage    );
    m_properties.SetValue( "attackConeWidth",   attackConeWidth );
    m_properties.SetValue( "attackConeDot",     attackConeDot   );

    m_properties.SetValue( "portrait",          portrait        );


    g_theDevConsole->PrintString( Stringf( "(ItemDef) Loaded new ItemDef (%s)", m_defType.c_str() ) );
    s_definitions[m_defType] = this;
}


float s_qualityChances[4] = {
    0.995f,
    0.95f,
    0.9f,
    0.8f
};


float s_slotBaseDefense[NUM_ITEM_SLOTS - 1] = {
    2.f, // Helm
    4.f, // Chest
    0.f, // Shoulder
    3.f, // Legs
    1.f  // Feet
};

template<>
void Definition<Item>::Define( Item& theObject ) const {
    ItemSlot slot = theObject.GetItemSlot();

    if( slot == ITEM_SLOT_NONE ) {
        return;
    }

    RNG* mapRNG = theObject.m_map->GetMapRNG();

    // Quality
    float qualityRoll = mapRNG->GetRandomFloatZeroToOne();

    if( qualityRoll >= s_qualityChances[0] ) {
        theObject.m_quality = 5.f; // Legendary
    } else if( qualityRoll >= s_qualityChances[1] ) {
        theObject.m_quality = 4.f; // Epic
    } else if( qualityRoll >= s_qualityChances[2] ) {
        theObject.m_quality = 3.f; // Rare
    } else if( qualityRoll >= s_qualityChances[3] ) {
        theObject.m_quality = 2.f; // Uncommon
    } else {
        theObject.m_quality = 1.f; // Common
    }

    // Money Value
    float proficiency = GetProperty( "proficiency", 0.f );
    theObject.m_moneyValue = 10 * (int)theObject.m_quality * (int)theObject.m_quality * ((int)proficiency + 2);

    // Defense
    if( slot != ITEM_SLOT_WEAPON ) {

        if( proficiency >= 0.f ) {
            theObject.m_defense = s_slotBaseDefense[slot] + theObject.m_quality + (2.f * proficiency);
        }
    }
}
