#include "Game/Actor.hpp" 

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Animator.hpp"
#include "Game/EnemyController.hpp"
#include "Game/Game.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/MerchantController.hpp"
#include "Game/PlayerController.hpp"
#include "Game/StatsManager.hpp"
#include "Game/Tile.hpp"


Actor::Actor( Map* theMap, const std::string& actorType, const std::string& controllerType ) :
    Entity( theMap, ENTITY_TYPE_ACTOR ) {

    m_actorDef = Definition<Actor>::GetDefinition( actorType );
    GUARANTEE_OR_DIE( m_actorDef != nullptr, Stringf( "(Actor) Failed to find actorDef of name %s", actorType.c_str() ) );
    m_actorDef->Define( *this );

    // Actor PaperDoll Material
    m_material = g_theRenderer->GetOrCreateMaterial( MAT_NAME_PAPER_DOLL );

    Shader* shader = g_theRenderer->GetOrCreateShader( "Data/Shaders/PaperDoll2.hlsl" );
    shader->CreateInputLayout<VertexPCU>();
    shader->SetDepthMode( COMPARE_ALWAYS, false );
    g_theRenderer->BindShader( shader );

    m_material->SetShader( shader );

    // Player Controller
    if( StringICmp(controllerType, "Player" ) ) {
        m_controller = new PlayerController( this );
    } else if( StringICmp(controllerType, "Merchant" ) ) {
        m_controller = new MerchantController( this );
    } else if( StringICmp(controllerType, "Enemy" ) ) {
        m_controller = new EnemyController( this );
    }
}


Actor::~Actor() {
    CLEAR_POINTER( m_inventory );
    CLEAR_POINTER( m_animator );
    CLEAR_POINTER( m_portraitMesh );
    CLEAR_POINTER( m_statsManager );
}


bool Actor::CompareActorDrawOrder( const Actor* const& actorA, const Actor* const& actorB ) {
    if( actorA == nullptr || actorB == nullptr ) {
        return false;
    }

    return (actorA->GetPosition().y > actorB->GetPosition().y);
}


void Actor::Startup() {
    m_animator = new Animator( this );

    // Clothe people
    std::vector< std::string > starterItems = {
        "Dress",
        "Tunic",
        "LightArmorSkillbook",
    };

    int numItems = (int)starterItems.size();

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        const std::string& itemType = starterItems[itemIndex];
        Item* item = m_inventory->SpawnNewItem( itemType, Vec2::ZERO, m_map->GetMapRNG() );
        m_inventory->EquipItem( item );
    }

    // Create meshes
    BuildMesh();
    BuildPortraitMesh();

    // Setup physics objects
    m_rigidBody = g_thePhysicsSystem->CreateNewRigidBody( 1.f, Rgba::GREEN );

    if( IsMovable() ) {
        m_rigidBody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
    }

    m_rigidBody->SetGameObject( this, &m_transform );
    m_rigidBody->AddCollider( Vec2::ZERO, m_physicsRadius );
}


void Actor::Shutdown() {

}


void Actor::Die() {
    m_isDead = true;
}


void Actor::TakeDamage( float damageToTake ) {
    if( damageToTake > 0 ) {
        float defense = m_inventory->GetDefense();
        float damageAfterDefense = Max( damageToTake - defense, 0.f );

        m_statsManager->TakeDamage( damageAfterDefense );
    } else {
        m_statsManager->TakeDamage( damageToTake );
    }
}


void Actor::Revive() {
    m_statsManager->Revive();
    m_isDead = false;
}


void Actor::Update( float deltaSeconds ) {
    UpdateFromController( deltaSeconds );
    m_animator->Update( deltaSeconds );

    // Update Position
    if( IsAlive() ) {
        //TakeDamage( 10.f * deltaSeconds );

        float moveSpeed = m_statsManager->GetMoveSpeed();
        Vec2 frameMovement = m_moveDir * moveSpeed * deltaSeconds;

        m_transform.position += frameMovement;
        m_inventory->Update( deltaSeconds );

        // Show health bar
        UpdateHealthBar();
    } else if( m_deathTimer != nullptr && m_deathTimer->HasFinshed() ) {
        if( m_controller != nullptr ) {
            std::string deathEvent = m_controller->GetDeathEvent();

            EventArgs args;
            args.SetValue( "actor", this );

            g_theEventSystem->FireEvent( deathEvent, args );
        }
    }

    m_inventory->UpdatePaperDoll( m_paperDollSprites );
    BuildMesh();
}


void Actor::Render() const {
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


void Actor::RenderPortrait() const {
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
    g_theRenderer->DrawMesh( m_portraitMesh, Matrix44::IDENTITY );
}


bool Actor::InteractWithActor( Actor* instigator ) {
    if( m_controller == nullptr ) {
        return false;
    }

    return m_controller->InteractWithActor( instigator );
}


void Actor::StartAttack( bool isAttacking /*= true*/ ) {
    m_isAttacking = isAttacking;
}


void Actor::Attack() {
    // Get weapon parameters
    WeaponInfo weaponInfo = GetWeaponInfo();

    if( weaponInfo.type == ATTACK_MELEE ) {
        Vec2 facing = m_animator->GetCurrentFacing();

        Actor* attackTarget = m_map->GetActorInCone( GetPosition(), facing, weaponInfo.coneDotProduct, weaponInfo.range, this );

        if( attackTarget != nullptr ) {
            attackTarget->TakeDamage( weaponInfo.damage );
        }
    }
}


std::string Actor::GetActorType() const {
    return m_actorDef->GetDefintionType();
}


Vec2 Actor::GetMoveDir() const {
    return m_moveDir;
}


ActorController* Actor::GetController() const {
    return m_controller;
}


Inventory* Actor::GetInventory() const {
    return m_inventory;
}


bool Actor::IsAttacking() const {
    return m_isAttacking;
}


float Actor::GetAttackRange() const {
    const Item* weapon = m_inventory->GetItemInSlot( ITEM_SLOT_WEAPON );
    return weapon->GetAttackRange();
}


float Actor::GetAttackDamage() const {
    const Item* weapon = m_inventory->GetItemInSlot( ITEM_SLOT_WEAPON );
    return weapon->GetAttackDamage();
}


WeaponInfo Actor::GetWeaponInfo() const {
    const Item* weapon = m_inventory->GetItemInSlot( ITEM_SLOT_WEAPON );
    return weapon->GetWeaponInfo();
}


void Actor::UpdateFromController( float deltaSeconds ) {
    if( m_controller != nullptr && IsAlive() ) {
        m_controller->Update( deltaSeconds );
    }
}


void Actor::UpdateHealthBar() const {
    // Only show if damaged
    float percentHealth = GetPercentHealth();

    if( percentHealth >= 1.f ) {
        return;
    }

    // Setup Window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGuiStyle& healthStyle = ImGui::GetStyle();
    ImGuiStyle origHealthStyle = healthStyle;
    healthStyle.WindowMinSize = ImVec2( 1.f, 1.f );
    healthStyle.WindowPadding = ImVec2( 1.f, 1.f );

    // Window size & pos
    Camera* gameCamera       = g_theGame->GetGameCamera();
    IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
    Vec2    actorPos         = GetPosition();
    IntVec2 clientPos        = gameCamera->WorldToClientPoint( Vec3( actorPos, 0.f ), clientDimensions );

    Vec2 windowSize = Vec2( 100.f, 10.f );
    Vec2 windowOrigin = Vec2( clientPos ) - (0.5f * windowSize) - (100.f * Vec2::UP);

    ImGui::SetNextWindowPos( ImVec2( windowOrigin.x, windowOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( windowSize.x, windowSize.y ), ImGuiCond_Always );

    std::string windowName = Stringf( "HealthBar_%d", m_entityIndex );
    ImGui::Begin( windowName.c_str(), nullptr, windowFlags );

    // Window Style
    ImGui::SetWindowFontScale( 0.001f );

    healthStyle.Colors[ImGuiCol_PlotHistogram] = ImVec4( Rgba::RED.r, Rgba::RED.g, Rgba::RED.b, Rgba::RED.a );

    // Window content
    ImGui::ProgressBar( percentHealth, ImVec2( -1.f, -1.f ), "" );

    // Shutdown Window
    healthStyle = origHealthStyle;
    ImGui::End();
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
    m_physicsRadius = halfHeight * 0.5f;
    
    m_localBounds = OBB2( Vec2( 0.f, m_physicsRadius ), Vec2(halfWidth, halfHeight), Vec2::RIGHT );
    Entity::BuildMesh( tint );
}


void Actor::BuildPortraitMesh( const Rgba& tint /*= Rgba::WHITE */ ) {
    const SpriteDef sprite = m_animator->GetPortraitSpriteDef();
    sprite.GetUVs( m_spriteUVs.mins, m_spriteUVs.maxs );

    Camera* uiCamera = g_theGame->GetUICamera();
    AABB2 cameraBounds = uiCamera->GetBounds();


    CPUMesh builder;
    builder.SetColor( tint );
    builder.AddQuad( OBB2( cameraBounds ), m_spriteUVs );

    CLEAR_POINTER( m_portraitMesh );
    m_portraitMesh = new GPUMesh( g_theRenderer );
    m_portraitMesh->CopyVertsFromCPUMesh( &builder );
}
