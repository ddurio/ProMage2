#include "Game/Actor.hpp" 

#include "Engine/Core/Timer.hpp"
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
#include "Game/Game.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
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
    std::string controllerLower = StringToLower( controllerType );

    if( controllerLower == "player" ) {
        m_controller = new PlayerController( this );
    }
}


Actor::~Actor() {
    CLEAR_POINTER( m_inventory );
    CLEAR_POINTER( m_animator );
    CLEAR_POINTER( m_portraitMesh );
    CLEAR_POINTER( m_statsManager );
}


void Actor::Startup() {
    m_animator = new Animator( this );

    Strings items = {
        "dress",
        "PlateChestM",
        "tunic",
        "PlateHelm",
        "Bandana",
        "LeatherLegsM",
        "RecurveBow",
        //"LongSword",
        "BootsM",
        "PlateShoulderM"
    };

    // DFS1FIXME: Weapon sprite sheets are messed up
    //            Portraits are all blank (no idle)
    //            Longsword is completely unusable (wrong UVs)
    int numItems = (int)items.size();

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        Item* item = m_inventory->SpawnNewItem( items[itemIndex] );
        m_inventory->EquipItem( item );
    }


    // Create meshes
    BuildMesh();
    BuildPortraitMesh();

    // Setup physics objects
    m_rigidBody = g_thePhysicsSystem->CreateNewRigidBody( 1.f, Rgba::GREEN );
    m_rigidBody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
    m_rigidBody->SetGameObject( this, &m_transform );
    m_rigidBody->AddCollider( Vec2::ZERO, m_physicsRadius );

}


void Actor::Shutdown() {

}


void Actor::Die() {
    m_isDead = true;
}


void Actor::Revive() {
    m_statsManager->Revive();
    m_isDead = false;
}


void Actor::Update( float deltaSeconds ) {
    UpdateFromController( deltaSeconds );
    m_animator->Update( deltaSeconds );

    //m_statsManager->TakeDamage( 10.f * deltaSeconds );

    // Update Position
    if( IsAlive() ) {
        float moveSpeed = m_statsManager->GetMoveSpeed();
        Vec2 frameMovement = m_moveDir * moveSpeed * deltaSeconds;

        m_transform.position += frameMovement;
        m_inventory->Update( deltaSeconds );
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


Vec2 Actor::GetMoveDir() const {
    return m_moveDir;
}


ActorController* Actor::GetController() const {
    return m_controller;
}


void Actor::UpdateFromController( float deltaSeconds ) {
    if( m_controller != nullptr && IsAlive() ) {
        m_controller->Update( deltaSeconds );
    }
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

