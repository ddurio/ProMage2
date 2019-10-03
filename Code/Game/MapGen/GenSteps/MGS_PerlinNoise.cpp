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


void MGS_PerlinNoise::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetName( "PerlinNoise" );
    MapGenStep::SaveToXml( document, element );

    if( m_gridSize != IntRange( 10, 30 ) ) {
        element.SetAttribute( "gridSize", m_gridSize.GetAsString().c_str() );
    }

    if( m_numOctaves != IntRange( 1, 3 ) ) {
        element.SetAttribute( "octaves", m_numOctaves.GetAsString().c_str() );
    }

    if( m_octavePersistence != FloatRange( 0.4f, 0.6f ) ) {
        element.SetAttribute( "persistence", m_octavePersistence.GetAsString().c_str() );
    }

    if( m_octaveScale != FloatRange( 1.5f, 2.5f ) ) {
        element.SetAttribute( "scale", m_octaveScale.GetAsString().c_str() );
    }
}


void MGS_PerlinNoise::RunOnce( Map& theMap ) const {
    RNG* mapRNG = theMap.GetMapRNG();

    IntVec2 theMapDimensions = theMap.GetMapDimensions();
    std::vector<int> tileIndexesToChange;

    int gridSize      = mapRNG->GetRandomIntInRange( m_gridSize );
    int octaves       = mapRNG->GetRandomIntInRange( m_numOctaves );
    float persistence = mapRNG->GetRandomFloatInRange( m_octavePersistence );
    float scale       = mapRNG->GetRandomFloatInRange( m_octaveScale );
    unsigned int seed = mapRNG->GetRandomSeed();
    
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
