#include "Game/ItemDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Item.hpp"
#include "Game/MapGen/Map/Map.hpp"
#include "Game/XMLUtils.hpp"


static bool g_progressionParsed = false;
static std::vector< int > g_qualityKeyFrames;
static std::vector< float* > g_qualityPercents;


static void ParseItemProgression() {
    XmlDocument doc;
    const XMLElement& root = ParseXMLRootElement( DATA_PROGRESSION, doc );

    const XMLElement* qualityEle = root.FirstChildElement( "ItemQuality" );
    const XMLElement* keyEle = qualityEle->FirstChildElement( "KeyFrame" );

    while( keyEle != nullptr ) {
        int floorIndex  = ParseXMLAttribute( *keyEle, "floor",     0 );
        float common    = ParseXMLAttribute( *keyEle, "common",    0.f );
        float uncommon  = ParseXMLAttribute( *keyEle, "uncommon",  0.f );
        float rare      = ParseXMLAttribute( *keyEle, "rare",      0.f );
        float epic      = ParseXMLAttribute( *keyEle, "epic",      0.f );
        float legendary = ParseXMLAttribute( *keyEle, "legendary", 0.f );

        float* frameValues = new float[5];
        frameValues[0] = common;
        frameValues[1] = uncommon;
        frameValues[2] = rare;
        frameValues[3] = epic;
        frameValues[4] = legendary;

        g_qualityKeyFrames.push_back( floorIndex );
        g_qualityPercents.push_back( frameValues );
        keyEle = keyEle->NextSiblingElement( "KeyFrame" );
    }
};


float ItemDef::s_slotBaseDefense[NUM_ITEM_SLOTS - 1] = {
    2.f, // Helm
    2.f, // Chest
    2.f, // Shoulder
    2.f, // Legs
    2.f  // Feet
};


void ItemDef::DefineObject( Item& theObject ) const {
    ItemSlot slot = theObject.GetItemSlot();
    RNG* rng = (theObject.m_itemRNG == nullptr) ? g_RNG : theObject.m_itemRNG;

    if( slot == ITEM_SLOT_NONE ) {
        return;
    }

    // Quality
    int numKeysFrames = (int)g_qualityKeyFrames.size();

    if( m_qualityOverride > 0.f ) {
        theObject.m_quality = m_qualityOverride;
    } else if( numKeysFrames < 2 ) {
        theObject.m_quality = 1.f;
    } else {
        Map* theMap = theObject.m_map;
        int floorIndex = theMap->GetCurrentFloor();
        int keyIndex = 1;

        while( (float)floorIndex > g_qualityKeyFrames[keyIndex] ) {
            if( keyIndex == numKeysFrames - 1 ) {
                break;
            }

            keyIndex++;
        }

        int floorMin = g_qualityKeyFrames[keyIndex - 1];
        int floorMax = g_qualityKeyFrames[keyIndex];
        float* percentMin = g_qualityPercents[keyIndex - 1];
        float* percentMax = g_qualityPercents[keyIndex];

        float scaledFloor = RangeMapFloat( (float)floorIndex, (float)floorMin, (float)floorMax, 0.f, 1.f );
        float scaledPercent[5] = {
            RangeMapFloat( scaledFloor, 0.f, 1.f, percentMin[0], percentMax[0] ),
            RangeMapFloat( scaledFloor, 0.f, 1.f, percentMin[1], percentMax[1] ),
            RangeMapFloat( scaledFloor, 0.f, 1.f, percentMin[2], percentMax[2] ),
            RangeMapFloat( scaledFloor, 0.f, 1.f, percentMin[3], percentMax[3] ),
            RangeMapFloat( scaledFloor, 0.f, 1.f, percentMin[4], percentMax[4] )
        };

        scaledPercent[0] = ClampFloat( scaledPercent[0], 0.f, 1.f );
        scaledPercent[1] = ClampFloat( scaledPercent[1], 0.f, 1.f );
        scaledPercent[2] = ClampFloat( scaledPercent[2], 0.f, 1.f );
        scaledPercent[3] = ClampFloat( scaledPercent[3], 0.f, 1.f );
        scaledPercent[4] = ClampFloat( scaledPercent[4], 0.f, 1.f );

        float requiredRoll = 0.f;
        float diceRoll = rng->GetRandomFloatZeroToOne();

        for( int qualityIndex = 0; qualityIndex < 5; qualityIndex++ ) {
            requiredRoll += scaledPercent[qualityIndex];

            if( diceRoll <= requiredRoll ) {
                theObject.m_quality = (float)qualityIndex + 1.f;
                break;
            }
        }
    }

    // Money Value
    theObject.m_moneyValue = 10 * (int)theObject.m_quality * (int)theObject.m_quality * ((int)m_proficiency + 2);

    // Defense
    if( slot != ITEM_SLOT_WEAPON && m_proficiency >= 0.f ) {
        theObject.m_defense = s_slotBaseDefense[slot] + theObject.m_quality + m_proficiency;
    }
}


ItemSlot ItemDef::GetItemSlot() const {
    return m_slot;
}


std::vector< Tags > ItemDef::GetItemSets() const {
    return m_itemSets;
}


std::string ItemDef::GetAttackAnim() const {
    return m_attackAnim;
}


float ItemDef::GetAttackRange() const {
    return m_attackRange;
}


float ItemDef::GetAttackDamage() const {
    return m_attackDamage;
}


float ItemDef::GetAttackConeWidth() const {
    return m_attackConeWidth;
}


float ItemDef::GetAttackConeDot() const {
    return m_attackConeDot;
}


bool ItemDef::IsConsumable() const {
    return m_isConsumable;
}


std::string ItemDef::GetConsumptionItemSet() const {
    return m_onConsumeItemSet;
}


IntRange ItemDef::GetDropFloors() const {
    return m_dropFloors;
}


std::string ItemDef::GetDropBias() const {
    return m_dropBias;
}

std::string ItemDef::GetSpriteSheet() const {
    return m_spriteSheet;
}


const SpriteDef ItemDef::GetPortrait() const {
    return m_portrait;
}


float ItemDef::GetSpritePPU() const {
    return m_spritePPU;
}


std::vector< const ItemDef* > ItemDef::GetAllDefinitions() {
    std::vector< const ItemDef* > allDefs;
    std::map< std::string, ItemDef* >::const_iterator itemDefIter = s_definitions.begin();

    while( itemDefIter != s_definitions.end() ) {
        allDefs.push_back( itemDefIter->second );
        itemDefIter++;
    }

    return allDefs;
}


ItemDef::ItemDef( const XMLElement& element ) {
    s_defClass = "ItemDef";

    // Parse Progression.xml
    if( !g_progressionParsed ) {
        ParseItemProgression();
        g_progressionParsed = true;
    }


    // Base Values
    m_defType                   = ParseXMLAttribute( element, "name",         "" );
    GUARANTEE_OR_DIE( m_defType != "", "(ItemDef) Missing required attribute 'name'" );

    m_baseValue   = ParseXMLAttribute( element, "value",       m_baseValue );
    m_slot        = ParseXMLAttribute( element, "slot",        m_slot );
    m_spriteSheet = ParseXMLAttribute( element, "spriteSheet", m_spriteSheet );
    GUARANTEE_OR_DIE( m_spriteSheet != "", "(ItemDef) Missing required attribute 'spriteSheet'" );


    const XMLElement* childEle = element.FirstChildElement();

    // Setup default portrait
    std::string defaultAnimName = Stringf( "%s.%s", ANIM_PAPER_DOLL_IDLE, "Down" );
    const SpriteAnimDef* defaultAnim = SpriteAnimDef::GetDefinition( defaultAnimName );
    SpriteDef defaultSprite = defaultAnim->GetSpriteDefAtTime( 0.f );

    AABB2 defaultUVs;
    defaultSprite.GetUVs( defaultUVs.mins, defaultUVs.maxs );

    const SpriteSheet& itemSheet = SpriteSheet::GetSpriteSheet( m_spriteSheet );
    std::string itemTexture = itemSheet.GetTexturePath();

    m_portrait = SpriteDef( defaultUVs.mins, defaultUVs.maxs, itemTexture );


    // Loop through child tags
    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "ItemSet" ) {
            std::string setName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( setName != "", "(ItemDef) ItemSet tag missing required attribute 'name'" );

            Tags itemTag;
            itemTag.SetTags( setName );
            m_itemSets.push_back( itemTag );

        } else if( tagName == "Portrait" ) {
            IntVec2 spriteCoords = ParseXMLAttribute( *childEle, "spriteCoords", IntVec2::NEGONE );

            if( spriteCoords != IntVec2::NEGONE ) {
                m_portrait = itemSheet.GetSpriteDef( spriteCoords );
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

                m_portrait = SpriteDef( portraitUVs.mins, portraitUVs.maxs, itemTexture );
            }

        } else if( tagName == "Attack" ) {
            m_attackAnim      = ParseXMLAttribute( *childEle, "anim",      m_attackAnim );
            m_attackRange     = ParseXMLAttribute( *childEle, "range",     m_attackRange );
            m_attackDamage    = ParseXMLAttribute( *childEle, "damage",    m_attackDamage );
            m_attackConeWidth = ParseXMLAttribute( *childEle, "coneWidth", m_attackConeWidth );
            m_attackConeDot   = CosDegrees( m_attackConeWidth * 0.5f );

        } else if( tagName == "Consumable" ) {
            m_isConsumable = true;
            m_onConsumeItemSet = ParseXMLAttribute( *childEle, "itemSet", m_onConsumeItemSet );
            
        } else if( tagName == "Drop" ) {
            m_dropFloors = ParseXMLAttribute( *childEle, "floor", IntRange::NEGONE );
            m_dropBias = ParseXMLAttribute( *childEle, "bias", m_dropBias );
            
        } else if( tagName == "Quality" ) {
            m_qualityOverride = ParseXMLAttribute( *childEle, "value", m_qualityOverride );
        }

        childEle = childEle->NextSiblingElement();
    }

    // Proficiency Factor
    if( m_slot == ITEM_SLOT_WEAPON ) {
        m_proficiency = 1.f; // Affects money value for weapons
    } else if( m_slot != ITEM_SLOT_NONE ) {
        // Affects defense and money value for armor
        int numTags = (int)m_itemSets.size();

        for( int tagIndex = 0; tagIndex < numTags; tagIndex++ ) {
            const Tags& tag = m_itemSets[tagIndex];

            if( tag.HasTags( "Light" ) ) {
                m_proficiency = Max( 0.f, m_proficiency );
            } else if( tag.HasTags( "Medium" ) ) {
                m_proficiency = Max( 1.f, m_proficiency );
            } else if( tag.HasTags( "Heavy" ) ) {
                m_proficiency = Max( 2.f, m_proficiency );
            }
        }
    }
}
