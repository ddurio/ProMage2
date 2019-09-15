#include "Game/MapGen/GenSteps/MGS_PerlinNoise.hpp"

#include "Engine/Math/RNG.hpp"
#include "Engine/Math/SmoothNoise.hpp"

#include "Game/MapGen/Map/Map.hpp"


MGS_PerlinNoise::MGS_PerlinNoise( const XMLElement& element ) :
    MapGenStep( element ) {
    m_gridSize          = ParseXMLAttribute( element, "gridSize",    m_gridSize );
    m_numOctaves        = ParseXMLAttribute( element, "octaves",     m_numOctaves );
    m_octavePersistence = ParseXMLAttribute( element, "persistence", m_octavePersistence );
    m_octaveScale       = ParseXMLAttribute( element, "scale",       m_octaveScale );
}


void MGS_PerlinNoise::RunOnce( Map& theMap ) const {
    IntVec2 theMapDimensions = theMap.GetMapDimensions();
    std::vector<int> tileIndexesToChange;

    int gridSize      = m_mapRNG->GetRandomIntInRange( m_gridSize );
    int octaves       = m_mapRNG->GetRandomIntInRange( m_numOctaves );
    float persistence = m_mapRNG->GetRandomFloatInRange( m_octavePersistence );
    float scale       = m_mapRNG->GetRandomFloatInRange( m_octaveScale );
    unsigned int seed = m_mapRNG->GetRandomSeed();
    
    // For each tile (by X and Y)
    for( int tileY = 0; tileY < theMapDimensions.y; tileY++ ) {
        for( int tileX = 0; tileX < theMapDimensions.x; tileX++ ) {
            Tile& tile = GetTile( theMap, tileX, tileY );

            // Calculate Noise
            float centerX = (float)tileX + 0.5f;
            float centerY = (float)tileY + 0.5f;

            float tileNoise = Compute2dPerlinNoise( centerX, centerY, (float)gridSize, octaves, persistence, scale, true, seed );
            tile.SetHeatMap( "Noise", tileNoise );

            if( IsTileValid( theMap, tile ) ) {
                ChangeTile( theMap, tileX, tileY );
            }
        }
    }
}
