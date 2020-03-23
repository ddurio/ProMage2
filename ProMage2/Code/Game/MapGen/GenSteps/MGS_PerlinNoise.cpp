#include "Game/MapGen/GenSteps/MGS_PerlinNoise.hpp"

#include "Engine/Math/RNG.hpp"
#include "Engine/Math/SmoothNoise.hpp"

#include "Game/MapGen/Map/Map.hpp"


MGS_PerlinNoise::MGS_PerlinNoise( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    m_gridSize          = ParseXMLAttribute( element, "gridSize",    m_motifVars,  m_motifHierarchy,  m_gridSize );
    m_numOctaves        = ParseXMLAttribute( element, "octaves",     m_motifVars,  m_motifHierarchy,  m_numOctaves );
    m_octavePersistence = ParseXMLAttribute( element, "persistence", m_motifVars,  m_motifHierarchy,  m_octavePersistence );
    m_octaveScale       = ParseXMLAttribute( element, "scale",       m_motifVars,  m_motifHierarchy,  m_octaveScale );
}


MGS_PerlinNoise::MGS_PerlinNoise( const Strings& motifHierarchy ) :
    MapGenStep( motifHierarchy ) {
}


void MGS_PerlinNoise::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetName( "PerlinNoise" );
    MapGenStep::SaveToXml( document, element );

    std::string varName = "gridSize";

    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_gridSize != IntRange( 10, 30 ) ) {
        element.SetAttribute( varName.c_str(), m_gridSize.GetAsString().c_str() );
    }

    varName = "octaves";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_numOctaves != IntRange( 1, 3 ) ) {
        element.SetAttribute( varName.c_str(), m_numOctaves.GetAsString().c_str() );
    }

    varName = "persistence";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_octavePersistence != FloatRange( 0.4f, 0.6f ) ) {
        element.SetAttribute( varName.c_str(), m_octavePersistence.GetAsString().c_str() );
    }

    varName = "scale";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_octaveScale != FloatRange( 1.5f, 2.5f ) ) {
        element.SetAttribute( varName.c_str(), m_octaveScale.GetAsString().c_str() );
    }
}


bool MGS_PerlinNoise::RecalculateMotifVars( EventArgs& args ) {
    MapGenStep::RecalculateMotifVars( args );

    std::string attrName = args.GetValue( MAPGEN_ARG_ATTR_NAME, "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, MAPGEN_ARG_RECALC_ALL );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "gridSize" ) ) {
        varName = m_motifVars.GetValue( "gridSize", "" );
        m_gridSize = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_gridSize );
    }
    
    if( calcAllVars || StringICmp( attrName, "octaves" ) ) {
        varName = m_motifVars.GetValue( "octaves", "" );
        m_numOctaves = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_numOctaves );
    }
    
    if( calcAllVars || StringICmp( attrName, "persistence" ) ) {
        varName = m_motifVars.GetValue( "persistence", "" );
        m_octavePersistence = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_octavePersistence );
    }
    
    if( calcAllVars || StringICmp( attrName, "scale" ) ) {
        varName = m_motifVars.GetValue( "scale", "" );
        m_octaveScale = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_octaveScale );
    }

    return false;
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
