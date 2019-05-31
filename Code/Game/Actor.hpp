#pragma once
#include "Game/GameCommon.hpp"

#include "Game/Entity.hpp"
#include "Game/ActorDef.hpp"


class Inventory;
class Tile;


class Actor : public Entity {
    //friend class ActorDef;
    friend class Definition<Actor>;

    public:
    explicit Actor( Map* theMap, std::string actorType, int playerID = -1 );
    explicit Actor( Map* theMap, const Definition<Actor>* actorDef, int playerID = -1 );

    void Startup();
    void Shutdown();

    void Die();

    void Update( float deltaSeconds );
    void Render() const;

    void OnCollisionEntity( Entity* collidingEntity );
    void OnCollisionTile( Tile* collidingTile );

    void SetWorldPosition( const Vec2& worldPosition );

    private:
    const Definition<Actor>* m_actorDef = nullptr;
    Inventory* m_inventory = nullptr;
    bool m_isPlayerControlled = false;
    int m_playerID = -1;

    float m_movementFraction = 0.f;

    float m_strength = 0;
    float m_intelligence = 0;
    float m_agility = 0;

    void UpdateFromController( float deltaSeconds );
    void BuildMesh( const Rgba& tint = Rgba::WHITE ) override;
};
