#include "Game/Actor.hpp" 
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Animator.hpp"
#include "Game/Game.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Tile.hpp"


Actor::Actor( Map* theMap, std::string actorType, int playerID /*= -1*/ ) :
    Entity( theMap, ENTITY_TYPE_ACTOR ) {
    m_actorDef = Definition<Actor>::GetDefinition( actorType );
    GUARANTEE_OR_DIE( m_actorDef != nullptr, Stringf( "(Actor) Failed to find actorDef of name %s", actorType.c_str() ) );
    m_actorDef->Define( *this );

    // Actor PaperDoll Material
    m_material = g_theRenderer->GetOrCreateMaterial( "PaperDoll" );

    Shader* shader = g_theRenderer->GetOrCreateShader( "Data/Shaders/PaperDoll.hlsl" );
    shader->CreateInputLayout<VertexPCU>();
    shader->SetDepthMode( COMPARE_ALWAYS, false );
    g_theRenderer->BindShader( shader );

    m_material->SetShader( shader );

    // Player Controller
    if( playerID >= 0 ) {
        m_controller = new PlayerController( this, playerID );
    }
}


// Missing code from above, but is this function necessary?
//Actor::Actor( Map* theMap, const Definition<Actor>* actorDef, int playerID /*= -1*/ ) :
/*
    Entity( theMap, ENTITY_TYPE_ACTOR ),
    m_actorDef(actorDef) {
    m_actorDef->Define( *this );

    if( playerID > 0 ) {
        m_controller = new PlayerController( this, playerID );
    }
}
*/


Actor::~Actor() {
    CLEAR_POINTER( m_inventory );
    CLEAR_POINTER( m_animator );
}


void Actor::Startup() {
    m_animator = new Animator( this );

    Item* dress = m_inventory->SpawnNewItem( "dress" );
    m_inventory->EquipItem( dress );

    Item* tunic = m_inventory->SpawnNewItem( "tunic" );
    m_inventory->EquipItem( tunic );

    Item* boots = m_inventory->SpawnNewItem( "boots" );
    m_inventory->EquipItem( boots );

    BuildMesh();
}


void Actor::Shutdown() {

}


void Actor::Die() {
    m_isDead = true;
}


void Actor::Update( float deltaSeconds ) {
    UpdateFromController( deltaSeconds );
    m_animator->Update( deltaSeconds );

    // Update Position
    Vec2 frameMovement = m_moveDir * m_moveSpeed * deltaSeconds;

    m_transform.position += frameMovement;
    // DFS1FIXME: This shouldn't be needed anymore right?
    //m_inventory->UpdateItemPositions( m_transform.position );
    m_inventory->Update( deltaSeconds );

    m_inventory->UpdatePaperDoll( m_paperDollSprites );
    BuildMesh();
}


void Actor::Render() const {
    /* DFS1FIXME: Add physics debug drawing back in
    if( g_theGame->IsDebugDrawingOn() ) {
        g_theRenderer->BindTexture( nullptr );
        g_theRenderer->DrawVertexArray( m_debugCosmeticVerts );
    }

    g_theRenderer->BindTexture( m_entityTexture );
    g_theRenderer->DrawVertexArray( m_entityVerts );

    m_inventory->Render();

    if( g_theGame->IsDebugDrawingOn() ) {
        g_theRenderer->BindTexture( nullptr );
        g_theRenderer->DrawVertexArray( m_debugPhysicsVerts );
    }
    */


    for( int slotIndex = 0; slotIndex < NUM_PAPER_DOLL_SLOTS; slotIndex++ ) {
        PaperDollSlot slot = (PaperDollSlot)slotIndex;

        std::string sheetName = m_paperDollSprites[slot];
        std::string textureName = "CLEAR_BLACK";

        if( sheetName != "" ) {
            const SpriteSheet sheet = SpriteSheet::GetSpriteSheet( sheetName );
            textureName = sheet.GetTexturePath();
        }

        m_material->SetTexture( textureName, slot );
    }

    g_theRenderer->BindMaterial( m_material );
    g_theRenderer->DrawMesh( m_mesh, Matrix44::MakeTranslation2D( m_transform.position ) );
}


void Actor::OnCollisionEntity( Entity* collidingEntity ) {
    UNUSED( collidingEntity );
}


void Actor::OnCollisionTile( Tile* collidingTile ) {
    if( !( (m_canWalk && collidingTile->AllowsWalking()  ) ||
           (m_canFly  && collidingTile->AllowsFlying()   ) ||
           (m_canSwim && collidingTile->AllowsSwimming() ) ) ) {
        PushDiscOutOfAABB2( m_transform.position, m_physicsRadius, collidingTile->GetWorldBounds() );
    }
}


Vec2 Actor::GetMoveDir() const {
    return m_moveDir;
}


void Actor::UpdateFromController( float deltaSeconds ) {
    if( m_controller != nullptr ) {
        m_controller->Update( deltaSeconds );
    }


    /* DFS1FIXME: Update controller key mappings
    UNUSED( deltaSeconds );

    if( m_playerID < 0 ) {
        return;
    }

    const XboxController& controller = g_theInput->GetController( m_playerID );

    if( m_isDead ) {
        const KeyButtonState yButton = controller.GetKeyButtonState( XBOX_BUTTON_ID_Y );

        if( yButton.WasJustPressed() ) {
            //Respawn();
        } else {
            return;
        }
    }

    // PlayerTank Movement
    const AnalogJoystick& leftStick = controller.GetLeftJoystick();
    float leftMagnitude = leftStick.GetMagnitude();
    m_movementFraction = leftMagnitude;

    if( leftMagnitude > 0.f ) {
        //m_orientationDegrees = leftStick.GetAngleDegrees();
    }

    const KeyButtonState aButton = controller.GetKeyButtonState( XBOX_BUTTON_ID_A );
    const KeyButtonState bButton = controller.GetKeyButtonState( XBOX_BUTTON_ID_B );
    const KeyButtonState xButton = controller.GetKeyButtonState( XBOX_BUTTON_ID_X );
    const KeyButtonState yButton = controller.GetKeyButtonState( XBOX_BUTTON_ID_Y );

    if( aButton.WasJustPressed() ) {
        // Pick up item?
        Inventory* mapInventory = m_map->GetMapInventory();
        Item* itemToPickUp = mapInventory->GetItemAtPosition( m_transform.position );

        if( itemToPickUp != nullptr ) {
            mapInventory->RemoveItemFromInventory( itemToPickUp );
            m_inventory->AddItemToInventory( itemToPickUp );
        }
    }

    if( bButton.WasJustPressed() ) {
        // Drop item?
        Item* itemToDrop = m_inventory->GetItemInSlot( 0 );

        if( itemToDrop != nullptr ) {
            m_inventory->RemoveItemFromInventory( itemToDrop );
            m_map->GetMapInventory()->AddItemToInventory( itemToDrop );
            itemToDrop->SetWorldPosition( m_transform.position );
        } else {
            itemToDrop = m_inventory->GetItemInSlot( 1 );

            if( itemToDrop != nullptr ) {
                m_inventory->RemoveItemFromInventory( itemToDrop );
                m_map->GetMapInventory()->AddItemToInventory( itemToDrop );
                itemToDrop->SetWorldPosition( m_transform.position );
            }
        }
    }

    if( xButton.WasJustPressed() ) {
        // Equip item?
        Item* itemToEquip = m_inventory->GetItemInSlot( 0 );

        if( itemToEquip != nullptr ) {
            m_inventory->EquipItem( itemToEquip );
        } else {
            itemToEquip = m_inventory->GetItemInSlot( 1 );

            if( itemToEquip != nullptr ) {
                m_inventory->EquipItem( itemToEquip );
            }
        }

    }


    if( yButton.WasJustPressed() ) {
        // Unequip item??
        // DFS1FIXME: Updated from mainHand/offHand to Weapon
        Item* itemToUnequip = m_inventory->GetItemInSlot( ITEM_SLOT_MAIN_HAND );

        if( itemToUnequip != nullptr ) {
            m_inventory->UnequipItem( itemToUnequip );
            return;
        } else {
            itemToUnequip = m_inventory->GetItemInSlot( ITEM_SLOT_OFF_HAND );

            if( itemToUnequip != nullptr ) {
                m_inventory->UnequipItem( itemToUnequip );
            }
        }
    }
    */
}


void Actor::BuildMesh( const Rgba& tint /*= Rgba::WHITE */ ) {
    // UV Dims
    const SpriteDef sprite = m_animator->GetCurrentSpriteDef();
    sprite.GetUVs( m_spriteUVs.mins, m_spriteUVs.maxs );

    Vec2 uvDimensions = m_spriteUVs.GetDimensions();
    uvDimensions.y *= -1.f;

    // Texture Dims
    std::string texturePath = sprite.GetTexturePath();
    TextureView2D* texture = g_theRenderer->GetOrCreateTextureView2D( texturePath );
    IntVec2 textureDimensions = texture->GetDimensions();

    // World Dims
    Vec2 spriteDimensions = textureDimensions * uvDimensions;
    Vec2 spriteWorldDimensions = spriteDimensions / m_actorDef->GetProperty( "spritePPU", 40.f );

    // Local Dims
    float halfWidth  = spriteWorldDimensions.x / 2.f;
    float halfHeight = spriteWorldDimensions.y / 2.f;
    m_localBounds = OBB2( Vec2::ZERO, Vec2(halfWidth, halfHeight), Vec2::RIGHT );

    m_physicsRadius = 0.4f;

    Entity::BuildMesh( tint );
}

