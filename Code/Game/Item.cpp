#include "Game/Item.hpp"

#include "Engine/Core/Tags.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Game.hpp"
#include "Game/Inventory.hpp"
#include "Game/ItemDef.hpp"


Item::Item( Map* theMap, std::string itemType, RNG* itemRNG /*= nullptr */ ) :
    Entity( theMap ),
    m_itemRNG( itemRNG ) {
    m_itemDef = ItemDef::GetDefinition( itemType );
    GUARANTEE_OR_DIE( m_itemDef != nullptr, Stringf( "(Item) Failed to find itemDef of name %s", itemType.c_str() ) );
    m_itemDef->DefineObject( *this );

    // Item Sprite Material
    m_material = g_theRenderer->GetOrCreateMaterial( "ItemSprite" );

    Shader* shader = g_theRenderer->GetOrCreateShader( "Data/Shaders/Item.hlsl" );
    shader->CreateInputLayout<VertexPCU>();
    shader->SetDepthMode( COMPARE_ALWAYS, false );
    g_theRenderer->BindShader( shader );

    m_material->SetShader( shader );
}


void Item::Startup() {
    m_cosmeticRadius = 0.4f; //FIXME: Hard coded value, should be calculated / defined from XML?
    // DFS1FIXME: set physicsRadius?
    BuildMesh();
}


void Item::Shutdown() {

}


void Item::Die() {
    m_isDead = true;
}


void Item::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );
    // Check decay?
    // Update position?

    //UpdateEntityVerts();
}


void Item::Render() const {
    // Add verts for render?
    // DFS1FIXME: Fix items rendering
    /*
    if( g_theGame->IsDebugDrawingOn() ) {
        g_theRenderer->BindTexture( nullptr );
        g_theRenderer->DrawVertexArray( m_debugCosmeticVerts );
    }

    g_theRenderer->BindTexture( m_entityTexture );
    g_theRenderer->DrawVertexArray( m_entityVerts );

    if( g_theGame->IsDebugDrawingOn() ) {
        g_theRenderer->BindTexture( nullptr );
        g_theRenderer->DrawVertexArray( m_debugCosmeticVerts );
    }
    */

    SpriteDef portrait = GetPortrait();
    m_material->SetTexture( portrait.GetTexturePath() );

    g_theRenderer->BindMaterial( m_material );
    g_theRenderer->DrawMesh( m_mesh, Matrix44::MakeTranslation2D( m_transform.position ) );
}


void Item::OnCollisionEntity( Entity* collidingEntity ) {
    UNUSED( collidingEntity );
    // Determine if the itemDef needs you to collide??
}


void Item::OnCollisionTile( Tile* collidingTile ) {
    UNUSED( collidingTile );
    // Determine if the itemDef needs you to collide??
}


const SpriteDef Item::GetPortrait() const {
    return m_itemDef->GetPortrait();
}


ItemSlot Item::GetItemSlot() const {
    ItemSlot slot = m_itemDef->GetItemSlot();
    return slot;
}


std::string Item::GetItemSlotText() const {
    ItemSlot slot = GetItemSlot();

    switch( slot ) {
        case(ITEM_SLOT_CHEST):      { return "Chest";       }
        case(ITEM_SLOT_NONE):       { return "None";        }
        case(ITEM_SLOT_HELM):       { return "Helm";        }
        case(ITEM_SLOT_SHOULDER):   { return "Shoulder";    }
        case(ITEM_SLOT_LEGS):       { return "Legs";        }
        case(ITEM_SLOT_FEET):       { return "Feet";        }
        case(ITEM_SLOT_WEAPON):     { return "Weapon";      }
    }

    return "Unknown";
}


std::vector< Tags > Item::GetItemSets() const {
    return m_itemDef->GetItemSets();
}


std::string Item::GetSprites() const {
    return m_itemDef->GetSpriteSheet();
}


std::string Item::GetSpriteTexture() const {
    std::string sheetName = GetSprites();
    const SpriteSheet sheet = SpriteSheet::GetSpriteSheet( sheetName );

    return sheet.GetTexturePath();
}


std::string Item::GetItemType() const {
    return m_itemDef->GetDefintionType();
}


int Item::GetValue() const {
    return m_moneyValue;
}


std::string Item::GetAttackAnim() const {
    return m_itemDef->GetAttackAnim();
}


float Item::GetAttackRange() const {
    return m_itemDef->GetAttackRange();
}


float Item::GetAttackDamage() const {
    float baseDamage = m_itemDef->GetAttackDamage();
    float scaledDamage = baseDamage * m_quality;

    return scaledDamage;
}


float Item::GetAttackConeWidth() const {
    return m_itemDef->GetAttackConeWidth();
}


float Item::GetAttackConeDotProduct() const {
    return m_itemDef->GetAttackConeDot();
}


WeaponInfo Item::GetWeaponInfo() const {
    WeaponInfo info;

    if( GetItemSlot() != ITEM_SLOT_WEAPON ) {
        return info;
    }


    // Get attack type
    const std::vector< Tags >& itemSets = GetItemSets();
    int numItemSets = (int)itemSets.size();

    for( int setIndex = 0; setIndex < numItemSets; setIndex++ ) {
        const Tags& itemSet = itemSets[setIndex];

        if( itemSet.HasTags( "melee" ) ) {
            info.type = ATTACK_MELEE;
            break;
        } else if( itemSet.HasTags( "ranged" ) ) {
            info.type = ATTACK_RANGED;
            break;
        }
    }

    // Get range/damage/coneWidth
    info.range = GetAttackRange();
    info.damage = GetAttackDamage();
    info.coneDotProduct = GetAttackConeDotProduct();

    return info;
}


float Item::GetDefense() const {
    return m_defense;
}


Rgba Item::GetQualityColor() const {
    if( m_quality >= 5.f ) {
        return Rgba::ORGANIC_ORANGE;
    } else if( m_quality >= 4.f ) {
        return Rgba::ORGANIC_PURPLE;
    } else if( m_quality >= 3.f ) {
        return Rgba::ORGANIC_BLUE;
    } else if( m_quality >= 2.f ) {
        return Rgba::ORGANIC_GREEN;
    } else {
        return Rgba::DIM_WHITE;
    }
}


std::string Item::GetQualityDescription() const {
    std::string qualityStr = "";

    if( m_quality >= 5.f ) {
        qualityStr = "Legendary";
    } else if( m_quality >= 4.f ) {
        qualityStr = "Epic";
    } else if( m_quality >= 3.f ) {
        qualityStr = "Rare";
    } else if( m_quality >= 2.f ) {
        qualityStr = "Uncommon";
    } else {
        qualityStr = "Common";
    }

    std::string description = Stringf( "Quality: %s", qualityStr.c_str() );
    return description;
}


bool Item::IsConsumable() const {
    return m_itemDef->IsConsumable();
}


std::string Item::GetConsumptionDescription() const {
    if( !IsConsumable() ) {
        return "";
    }

    std::string setsToAddStr = m_itemDef->GetConsumptionItemSet();
    Strings setsToAddArray = SplitStringOnDelimeter( setsToAddStr, ',', false );
    std::string formattedSets = JoinStrings( setsToAddArray, " AND " );
    std::string description = Stringf( "Provides: %s", formattedSets.c_str() );

    return description;
}


void Item::Consume( Actor* consumer ) const {
    if( !IsConsumable() ) {
        return;
    }

    std::string itemSetToAdd = m_itemDef->GetConsumptionItemSet();
    
    Inventory* consumerInv = consumer->GetInventory();
    consumerInv->AddItemSets( itemSetToAdd );
}


void Item::BuildMesh( const Rgba& tint /*= Rgba::WHITE */ ) {
    // UV Dims
    SpriteDef portrait = GetPortrait();
    portrait.GetUVs( m_spriteUVs.mins, m_spriteUVs.maxs );

    Vec2 uvDimensions = m_spriteUVs.GetDimensions();
    uvDimensions.y *= -1.f;

    // Texture Dims
    std::string texturePath = portrait.GetTexturePath();
    TextureView2D* texture = g_theRenderer->GetOrCreateTextureView2D( texturePath );
    IntVec2 textureDimensions = texture->GetDimensions();

    // World Dims
    Vec2 spriteDimensions = textureDimensions * uvDimensions;
    Vec2 spriteWorldDimensions = spriteDimensions / m_itemDef->GetSpritePPU();

    // Local Dims
    float halfWidth  = spriteWorldDimensions.x / 2.f;
    float halfHeight = spriteWorldDimensions.y / 2.f;
    m_localBounds = OBB2( Vec2::ZERO, Vec2(halfWidth, halfHeight), Vec2::RIGHT );

    m_physicsRadius = 0.4f;

    Entity::BuildMesh( tint );
}
