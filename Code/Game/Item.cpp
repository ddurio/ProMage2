#include "Game/Item.hpp"

#include "Engine/Core/Tags.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Game.hpp"
#include "Game/ItemDef.hpp"


Item::Item( Map* theMap, std::string itemType ) :
    Entity( theMap ) {
    m_itemDef = Definition<Item>::GetDefinition( itemType );
    GUARANTEE_OR_DIE( m_itemDef != nullptr, Stringf( "(Item) Failed to find itemDef of name %s", itemType.c_str() ) );
    m_itemDef->Define( *this );
}


Item::Item( Map* theMap, const Definition<Item>* itemDef ) :
    Entity( theMap ),
    m_itemDef(itemDef) {
    GUARANTEE_OR_DIE( m_itemDef != nullptr, "(Item) Cannot construct from itemDef of nullptr" );
    m_itemDef->Define( *this );
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
}


void Item::OnCollisionEntity( Entity* collidingEntity ) {
    UNUSED( collidingEntity );
    // Determine if the itemDef needs you to collide??
}


void Item::OnCollisionTile( Tile* collidingTile ) {
    UNUSED( collidingTile );
    // Determine if the itemDef needs you to collide??
}


ItemSlot Item::GetItemSlot() const {
    ItemSlot slot = m_itemDef->GetProperty( "slot", ITEM_SLOT_NONE );
    return slot;
}


std::vector< Tags > Item::GetItemSets() const {
    std::vector< Tags > emptySet;
    return m_itemDef->GetProperty( "itemSets", emptySet );
}


std::string Item::GetSprites() const {
    return m_itemDef->GetProperty( "spriteSheet", std::string("") );
}


void Item::SetWorldPosition( const Vec2& worldPosition ) {
    m_transform.position = worldPosition;
}


void Item::BuildMesh( const Rgba& tint /*= Rgba::WHITE */ ) {
    TextureView2D* texture = g_theRenderer->GetOrCreateTextureView2D( m_itemDef->GetProperty( "texturePath", std::string("") ) );

    IntVec2 textureDimensions = texture->GetDimensions();
    // DFS1FIXME: Set correct UVs!
    AABB2 uvs = m_itemDef->GetProperty( "spriteUVs", AABB2::ZEROTOONE );

    Vec2 uvDimensions = uvs.GetDimensions();
    Vec2 spriteDimensions = textureDimensions * uvDimensions;
    Vec2 spriteWorldDimensions = spriteDimensions / m_itemDef->GetProperty( "spritePPU", 1.f );

    float halfWidth  = spriteWorldDimensions.x / 2.f;
    float halfHeight = spriteWorldDimensions.y / 2.f;
    m_localBounds = OBB2( Vec2::ZERO, Vec2(halfWidth, halfHeight), Vec2::RIGHT );

    Entity::BuildMesh( tint );
}
