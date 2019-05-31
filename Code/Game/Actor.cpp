#include "Game/Actor.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"


Actor::Actor( Map* theMap, std::string actorType, int playerID /*= -1*/ ) :
    Entity( theMap, ENTITY_TYPE_ACTOR ) {
    //m_actorDef = ActorDef::GetActorDef( actorType );
    m_actorDef = Definition<Actor>::GetDefinition( actorType );
    m_actorDef->Define( *this );

    if( playerID >= 0 ) {
        m_isPlayerControlled = true;
        m_playerID = playerID;
    }
}


Actor::Actor( Map* theMap, const Definition<Actor>* actorDef, int playerID /*= -1*/ ) :
    Entity( theMap, ENTITY_TYPE_ACTOR ),
    m_actorDef(actorDef) {
    m_actorDef->Define( *this );

    if( playerID > 0 ) {
        m_isPlayerControlled = true;
        m_playerID = playerID;
    }
}


void Actor::Startup() {
    BuildMesh();
}


void Actor::Shutdown() {

}


void Actor::Die() {
    m_isDead = true;
}


void Actor::Update( float deltaSeconds ) {
    if( m_isPlayerControlled ) {
        UpdateFromController( deltaSeconds );

        // Update Position
        float thrustSpeed = 0.f;
        if( m_movementFraction > 0.f ) {
            thrustSpeed = 1.3f * m_movementFraction * deltaSeconds;
        }

        m_transform.position += thrustSpeed * GetForwardVector();
        m_inventory->UpdateItemPositions( m_transform.position );
        m_inventory->Update( deltaSeconds );
    }
}


void Actor::Render() const {
    /* DFS1FIXME: Need to render actors correctly
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

    //g_theRenderer->BindTexture( m_actorDef->GetTexturePath() );
    std::string texturePath = m_actorDef->GetProperty( "texturePath", std::string("") );
    g_theRenderer->BindTexture( texturePath );
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


void Actor::SetWorldPosition( const Vec2& worldPosition ) {
    m_transform.position = worldPosition;
}


void Actor::UpdateFromController( float deltaSeconds ) {
    // DFS1FIXME: Update controller key mappings
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
}


void Actor::BuildMesh( const Rgba& tint /*= Rgba::WHITE */ ) {
    std::string texturePath = m_actorDef->GetProperty( "texturePath", std::string() );
    TextureView2D* texture = g_theRenderer->GetOrCreateTextureView2D( texturePath );

    IntVec2 textureDimensions = texture->GetDimensions();
    // DFS1FIXME: Set correct UVs!
    AABB2 uvs = m_actorDef->GetProperty( "spriteUVs", AABB2::ZEROTOONE );

    Vec2 uvDimensions = uvs.GetDimensions();
    Vec2 spriteDimensions = textureDimensions * uvDimensions;
    Vec2 spriteWorldDimensions = spriteDimensions / m_actorDef->GetProperty( "spritePPU", 1.f );

    float halfWidth  = spriteWorldDimensions.x / 2.f;
    float halfHeight = spriteWorldDimensions.y / 2.f;
    m_localBounds = OBB2( Vec2::ZERO, Vec2(halfWidth, halfHeight), Vec2::RIGHT );

    m_physicsRadius = 0.4f;

    Entity::BuildMesh( tint );
}

