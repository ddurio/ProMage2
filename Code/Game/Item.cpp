#include "Game/Item.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Game.hpp"
#include "Game/ItemDef.hpp"


Item::Item( Map* theMap, std::string itemType ) :
    Entity( theMap ) {
    m_itemDef = ItemDef::GetItemDef( itemType );
    m_itemDef->Define( *this );
}


Item::Item( Map* theMap, const ItemDef* itemDef ) :
    Entity( theMap ),
    m_itemDef(itemDef) {
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
    return m_itemDef->GetItemSlot();
}


void Item::SetWorldPosition( const Vec2& worldPosition ) {
    m_transform.position = worldPosition;

    // DFS1FIXME: Test code for PLAYABLE
    switch( m_itemDef->GetItemSlot() ) {
        case( ITEM_SLOT_MAIN_HAND ): {
            m_transform.position += Vec2( -1.f, 0.f );
            break;
        } case( ITEM_SLOT_OFF_HAND ): {
            m_transform.position += Vec2( 1.f, 0.f );
            break;
        }
    }
}


void Item::BuildMesh( const Rgba& tint /*= Rgba::WHITE */ ) {
    TextureView2D* texture = g_theRenderer->GetOrCreateTextureView2D( m_itemDef->GetTexturePath() );

    IntVec2 textureDimensions = texture->GetDimensions();
    // DFS1FIXME: Set correct UVs!
    AABB2 uvs = m_itemDef->GetUVs();

    Vec2 uvDimensions = uvs.GetDimensions();
    Vec2 spriteDimensions = textureDimensions * uvDimensions;
    Vec2 spriteWorldDimensions = spriteDimensions / (float)m_itemDef->GetPPU();

    float halfWidth  = spriteWorldDimensions.x / 2.f;
    float halfHeight = spriteWorldDimensions.y / 2.f;
    m_localBounds = OBB2( Vec2::ZERO, Vec2(halfWidth, halfHeight), Vec2::RIGHT );

    Entity::BuildMesh( tint );
}
