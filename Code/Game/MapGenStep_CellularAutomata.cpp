#include "Game/MapGenStep_CellularAutomata.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/Map.hpp"
#include "Game/Metadata.hpp"
#include "Game/Tile.hpp"


MapGenStep_CellularAutomata::MapGenStep_CellularAutomata( const XMLElement& element ) :
    MapGenStep(element) {
    m_ifNeighborType    = ParseXMLAttribute( element, "ifNeighborType",    m_ifNeighborType );
    m_ifNeighborHasTags = ParseXMLAttribute( element, "ifNeighborHasTags", m_ifNeighborHasTags );
    m_ifNumNeighbors    = ParseXMLAttribute( element, "ifNumNeighbors",    m_ifNumNeighbors );

    m_radius            = ParseXMLAttribute( element, "radius",            m_radius );
    m_chancePerTile     = ParseXMLAttribute( element, "chancePerTile",     m_chancePerTile );
}


void MapGenStep_CellularAutomata::RunOnce( Map& map ) const {
    IntVec2 mapDimensions = map.GetMapDimensions();
    std::vector<int> tileIndexesToChange;
    
    // For each tile (by X and Y)
    for( int tileY = 0; tileY < mapDimensions.y; tileY++ ) {
        for( int tileX = 0; tileX < mapDimensions.x; tileX++ ) {
            const Tile& tile = map.GetTileFromTileCoords( tileX, tileY );
            int numMatchingNeighbors = 0;

            // ifType matches or ifType not set in XML
            if( IsTileValid( tile ) ) {
                // For each neighbor within a radius (by X and Y)
                for( int neighborY = tileY - m_radius; neighborY <= tileY + m_radius; neighborY++ ) {
                    // If yCoords are valid
                    if( neighborY >= 0 && neighborY < mapDimensions.y ) {
                        for( int neighborX = tileX - m_radius; neighborX <= tileX + m_radius; neighborX++ ) {
                            // If xCoords are valid and don't match tileX & Y
                            if( neighborX >= 0 && neighborX < mapDimensions.x &&
                                !(neighborX == tileX && neighborY == tileY ) ) {
                                const Tile& neighbor = map.GetTileFromTileCoords( neighborX, neighborY );

                                if( IsNeighborTileValid( neighbor ) ) {
                                    numMatchingNeighbors++;
                                }
                            }
                        }
                    }
                }

                if( m_ifNumNeighbors.IsIntInRange( numMatchingNeighbors ) ) {
                    tileIndexesToChange.push_back( map.GetTileIndexFromTileCoords( tileX, tileY ) );
                }
            }
        }
    }

    int numTilesToChange = (int)tileIndexesToChange.size();
    for( int tileIndex = 0; tileIndex < numTilesToChange; tileIndex++ ) {
        if( m_mapRNG->PercentChance(m_chancePerTile) ) {
            ChangeTile( map, tileIndexesToChange[tileIndex] );
        }
    }
}


bool MapGenStep_CellularAutomata::IsNeighborTileValid( const Tile& neighbor ) const {
    bool isValid = true;

    std::string tileType     = neighbor.GetTileType();
    const Metadata* metadata = neighbor.GetMetadata();
    const Tags& tileTags     = metadata->m_tagData;

    if( m_ifNeighborType != "" && tileType != m_ifNeighborType ) {
        isValid = false;
    }

    if( m_ifNeighborHasTags != "" && !tileTags.HasTags(m_ifNeighborHasTags)) {
        isValid = false;
    }

    return isValid;
}
