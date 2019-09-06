#include "Game/MapGen/GenSteps/MapGenStep_Sprinkle.hpp"

#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Tile.hpp"


MapGenStep_Sprinkle::MapGenStep_Sprinkle( const XMLElement& element ) :
    MapGenStep( element ) {
    m_count = ParseXMLAttribute( element, "count", m_count );
}


void MapGenStep_Sprinkle::RunOnce( Map& map ) const {
    int numSprinkles = m_mapRNG->GetRandomIntInRange( m_count );

    IntVec2 mapDimensions = map.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int sprinkleIndex = 0; sprinkleIndex < numSprinkles; sprinkleIndex++ ) {
        int tileIndex = -1;
        const Tile* tile = nullptr;

        // ThesisFIXME: This produces an infinite loop if no tiles available
        do {
            tileIndex = m_mapRNG->GetRandomIntLessThan( numTiles );
            tile = &(map.GetTile( tileIndex ));
        } while( !IsTileValid( *tile ) );

        ChangeTile( map, tileIndex );
    }
}

