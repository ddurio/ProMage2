#include "Game/MapGenStep_Sprinkle.hpp"

#include "Engine/Math/RNG.hpp"

#include "Game/Map.hpp"
#include "Game/Tile.hpp"


MapGenStep_Sprinkle::MapGenStep_Sprinkle( const XMLElement& element ) :
    MapGenStep( element ) {
    m_count = ParseXMLAttribute( element, "count", m_count );
}


void MapGenStep_Sprinkle::RunOnce( Map& map ) const {
    int numSprinkles = g_RNG->GetRandomIntInRange( m_count );

    IntVec2 mapDimensions = map.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int sprinkleIndex = 0; sprinkleIndex < numSprinkles; sprinkleIndex++ ) {
        int tileIndex = -1;
        const Tile* tile = nullptr;

        do {
            tileIndex = g_RNG->GetRandomIntLessThan( numTiles );
            tile = &(map.GetTileFromTileIndex( tileIndex ));
        } while( m_ifIsType != "" && m_ifIsType != tile->GetTileType() );

        ChangeTile( map, tileIndex );
    }
}

