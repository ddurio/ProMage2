#include "Game/MapGenStep.hpp"

#include "Engine/Core/Tags.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/Inventory.hpp"
#include "Game/Map.hpp"
#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Game/MapGenStep_DistanceField.hpp"
#include "Game/MapGenStep_FromImage.hpp"
#include "Game/MapGenStep_PerlinNoise.hpp"
#include "Game/MapGenStep_RoomsAndPaths.hpp"
#include "Game/MapGenStep_Sprinkle.hpp"
#include "Game/Metadata.hpp"
#include "Game/Tile.hpp"

MapGenStep::MapGenStep( const XMLElement& element ) {
    m_stepType = element.Name();

    // Conditions
    m_ifIsType         = ParseXMLAttribute( element, "ifIsType",         m_ifIsType );
    m_ifHasTags        = ParseXMLAttribute( element, "ifHasTags",        m_ifHasTags );
    m_ifDistanceField  = ParseXMLAttribute( element, "ifDistanceField",  m_ifDistanceField );
    m_ifNoise          = ParseXMLAttribute( element, "ifNoise",          m_ifNoise );

    // Results
    m_changeToType     = ParseXMLAttribute( element, "changeToType",     m_changeToType );
    m_setTags          = ParseXMLAttribute( element, "setTags",          m_setTags );
    m_setDistanceField = ParseXMLAttribute( element, "setDistanceField", m_setDistanceField );
    m_spawnActorType   = ParseXMLAttribute( element, "spawnActor",       m_spawnActorType );
    m_actorController  = ParseXMLAttribute( element, "controller",       m_actorController );
    m_spawnItemType    = ParseXMLAttribute( element, "spawnItem",        m_spawnItemType );

    m_chanceToRun      = ParseXMLAttribute( element, "chanceToRun",      m_chanceToRun );
    m_numIterations    = ParseXMLAttribute( element, "numIterations",    m_numIterations );
}


MapGenStep* MapGenStep::CreateMapGenStep( const XMLElement& element ) {
    std::string stepType = element.Name();
    MapGenStep* step = nullptr;

    if( stepType == "Sprinkle" ) {
        step = new MapGenStep_Sprinkle( element );
    } else if( stepType == "FromImage") {
        step = new MapGenStep_FromImage( element );
    } else if( stepType == "CellularAutomata" ) {
        step = new MapGenStep_CellularAutomata( element );
    } else if( stepType == "DistanceField" ) {
        step = new MapGenStep_DistanceField( element );
    } else if( stepType == "PerlinNoise" ) {
        step = new MapGenStep_PerlinNoise( element );
    } else if( stepType == "RoomsAndPaths" ) {
        step = new MapGenStep_RoomsAndPaths( element );
    }

    return step;
}


void MapGenStep::Run( Map& map ) const {
    m_mapRNG = map.m_mapRNG;
    int iterations = m_mapRNG->GetRandomIntInRange( m_numIterations );

    for( int iterationIndex = 0; iterationIndex < iterations; iterationIndex++ ) {
        if( m_mapRNG->PercentChance( m_chanceToRun ) ) {
            RunOnce( map );
        }
    }
}


bool MapGenStep::IsTileValid( const Tile& tile ) const {
    bool isValid = true;

    std::string tileType     = tile.GetTileType();
    const Metadata* metadata = tile.GetMetadata();
    const Tags& tileTags     = metadata->m_tagData;
    float distance           = tile.GetDistanceField();
    float noise              = tile.GetNoiseValue();

    if( m_ifIsType != "" && tileType != m_ifIsType ) {
        isValid = false;
    }

    if( m_ifHasTags != "" && !tileTags.HasTags(m_ifHasTags)) {
        isValid = false;
    }

    if( m_ifDistanceField != FloatRange::NEGONE && !IsFloatInRange(distance, m_ifDistanceField) ) {
        isValid = false;
    }

    if( m_ifNoise != FloatRange::NEGONE && !IsFloatInRange( noise, m_ifNoise ) ) {
        isValid = false;
    }

    return isValid;
}


void MapGenStep::ChangeTile( Map& map, int tileIndex ) const {
    Tile& tile = map.m_tiles[tileIndex];

    if( m_changeToType != "" ) {
        tile.SetTileType( m_changeToType );
    }

    if( m_setTags != "" ) {
        Metadata* tileMetadata = tile.GetMetadata();
        Tags& tileTags = tileMetadata->m_tagData;

        tileTags.SetTags( m_setTags );
    }

    if( m_setDistanceField != FloatRange::NEGONE ) {
        float distance = m_mapRNG->GetRandomFloatInRange( m_setDistanceField );
        tile.SetDistanceField( distance );
    }

    if( m_spawnActorType != "" ) {
        IntVec2 tileCoords = tile.GetTileCoords();
        Vec2 worldCoords = Vec2( (float)tileCoords.x + 0.5f, (float)tileCoords.y + 0.5f );

        map.SpawnNewActor( m_spawnActorType, m_actorController, worldCoords );
    }

    if( m_spawnItemType != "" ) {
        IntVec2 tileCoords = tile.GetTileCoords();
        Vec2 worldCoords = Vec2( (float)tileCoords.x + 0.5f, (float)tileCoords.y + 0.5f );
        Inventory* mapInventory = map.GetMapInventory();
        mapInventory->SpawnNewItem( m_spawnItemType, worldCoords );
    }
}


void MapGenStep::ChangeTile( Map& map, int tileX, int tileY ) const {
    int tileIndex = map.GetTileIndexFromTileCoords( tileX, tileY );
    ChangeTile( map, tileIndex );
}


Tile& MapGenStep::GetTile( Map& map, int tileIndex ) const {
    return map.m_tiles[tileIndex];
}


Tile& MapGenStep::GetTile( Map& map, int tileX, int tileY ) const {
    int tileIndex = map.GetTileIndexFromTileCoords( tileX, tileY );
    return GetTile( map, tileIndex );
}
