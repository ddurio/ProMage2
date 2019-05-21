#include "Game/MapGenStep_PerlinNoise.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Math/SmoothNoise.hpp"

#include "Game/Map.hpp"
#include "Game/Tile.hpp"


MapGenStep_PerlinNoise::MapGenStep_PerlinNoise( const XMLElement& element ) :
    MapGenStep( element ) {
    m_numOctaves        = ParseXMLAttribute( element, "octaves",     m_numOctaves );
    m_gridSize          = ParseXMLAttribute( element, "gridSize",    m_gridSize );
    m_octavePersistence = ParseXMLAttribute( element, "persistence", m_octavePersistence );
    m_octaveScale       = ParseXMLAttribute( element, "scale",       m_octaveScale );
    m_seed              = ParseXMLAttribute( element, "seed",        m_seed );
}


void MapGenStep_PerlinNoise::RunOnce( Map& map ) const {
    IntVec2 mapDimensions = map.GetMapDimensions();
    std::vector<int> tileIndexesToChange;

    int octaves       = g_RNG->GetRandomIntInRange( m_numOctaves );
    float persistence = g_RNG->GetRandomFloatInRange( m_octavePersistence );
    float scale       = g_RNG->GetRandomFloatInRange( m_octaveScale );
    int gridSize      = g_RNG->GetRandomIntInRange( m_gridSize );
    
    // For each tile (by X and Y)
    for( int tileY = 0; tileY < mapDimensions.y; tileY++ ) {
        for( int tileX = 0; tileX < mapDimensions.x; tileX++ ) {
            Tile& tile = GetTile( map, tileX, tileY );

            // Calculate Noise
            float centerX = (float)tileX + 0.5f;
            float centerY = (float)tileY + 0.5f;

            float tileNoise = Compute2dPerlinNoise( centerX, centerY, (float)gridSize, octaves, persistence, scale, true, m_seed );
            //float tileNoise = Compute2dPerlinNoise( centerX, centerY );

            tile.SetNoiseValue( tileNoise );
            //DebuggerPrintf( Stringf( "X: %d Y: %d Noise: %f\n", tileX, tileY, tileNoise ).c_str() );

            // if XML conditions met
            if( IsTileValid( tile ) ) {
                ChangeTile( map, tileX, tileY );
            }
        }
    }
}
