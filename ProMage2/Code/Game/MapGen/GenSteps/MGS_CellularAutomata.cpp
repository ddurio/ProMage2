#include "Game/MapGen/GenSteps/MGS_CellularAutomata.hpp"

#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Metadata.hpp"


MGS_CellularAutomata::MGS_CellularAutomata( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    std::string radius  = ParseXMLAttribute( element, "radius",            m_motifVars,     m_motifHierarchy,    "1" );
    m_radius            = ParseCustomIntRange( radius );

    m_chancePerTile     = ParseXMLAttribute( element, "chancePerTile",     m_motifVars,     m_motifHierarchy,    m_chancePerTile );
    m_ifNeighborType    = ParseXMLAttribute( element, "ifNeighborType",    m_motifVars,     m_motifHierarchy,    m_ifNeighborType );
    m_ifNumNeighbors    = ParseXMLAttribute( element, "ifNumNeighbors",    m_motifVars,     m_motifHierarchy,    m_ifNumNeighbors );

    std::string neighborTagCSV = ParseXMLAttribute( element, "ifNeighborHasTags", m_motifVars, m_motifHierarchy, "" );
    m_ifNeighborHasTags = SplitStringOnDelimeter( neighborTagCSV, ',', false );
}


MGS_CellularAutomata::MGS_CellularAutomata( const Strings& motifHierarchy ) :
    MapGenStep( motifHierarchy ) {
}


void MGS_CellularAutomata::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetName( "CellularAutomata" );
    MapGenStep::SaveToXml( document, element );

    std::string varName = "radius";

    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_radius != IntRange::ONE ) {
        std::string radiusStr = SaveCustomIntRange( m_radius );
        element.SetAttribute( varName.c_str(), radiusStr.c_str() );
    }

    varName = "chancePerTile";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_chancePerTile != 1.f ) {
        element.SetAttribute( varName.c_str(), m_chancePerTile );
    }

    varName = "ifNeighborType";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_ifNeighborType != "" ) {
        element.SetAttribute( varName.c_str(), m_ifNeighborType.c_str() );
    }

    varName = "ifNumNeighbors";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_ifNumNeighbors != IntRange( 1, 8 ) ) {
        element.SetAttribute( varName.c_str(), m_ifNumNeighbors.GetAsString().c_str() );
    }

    varName = "ifNeighborHasTags";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( !m_ifNeighborHasTags.empty() ) {
        std::string ifTagsCSV = JoinStrings( m_ifNeighborHasTags, "," );
        element.SetAttribute( varName.c_str(), ifTagsCSV.c_str() );
    }
}


bool MGS_CellularAutomata::RecalculateMotifVars( EventArgs& args ) {
    MapGenStep::RecalculateMotifVars( args );

    std::string attrName = args.GetValue( MAPGEN_ARG_ATTR_NAME, "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, MAPGEN_ARG_RECALC_ALL );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "radius" ) ) {
        varName = m_motifVars.GetValue( "radius", "" );
        m_radius = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_radius );
    }
    
    if( calcAllVars || StringICmp( attrName, "chancePerTile" ) ) {
        varName = m_motifVars.GetValue( "chancePerTile", "" );
        m_chancePerTile = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_chancePerTile );
    }
    
    if( calcAllVars || StringICmp( attrName, "ifNeighborType" ) ) {
        varName = m_motifVars.GetValue( "ifNeighborType", "" );
        m_ifNeighborType = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_ifNeighborType );
    }
    
    if( calcAllVars || StringICmp( attrName, "ifNumNeighbors" ) ) {
        varName = m_motifVars.GetValue( "ifNumNeighbor", "" );
        m_ifNumNeighbors = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_ifNumNeighbors );
    }
    
    if( calcAllVars || StringICmp( attrName, "ifNeighborTags" ) ) {
        varName = m_motifVars.GetValue( "ifNeighborTags", "" );
        std::string joinedTags = JoinStrings( m_ifNeighborHasTags );
        std::string neighborTagCSV = MotifDef::GetVariableValue( m_motifHierarchy, varName, joinedTags );
        m_ifNeighborHasTags = SplitStringOnDelimeter( neighborTagCSV, ',', false );
    }

    return false;
}


void MGS_CellularAutomata::RunOnce( Map& theMap ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    std::vector<int> tileIndexesToChange;

    std::vector< TileValidity > neighborValidities;
    neighborValidities.resize( mapDimensions.x * mapDimensions.y, TileValidity::UNKNOWN );

    // For each tile (by X and Y)
    for( int tileY = 0; tileY < mapDimensions.y; tileY++ ) {
        for( int tileX = 0; tileX < mapDimensions.x; tileX++ ) {
            // Check home (center) tile
            int tileIndex = theMap.GetTileIndex( tileX, tileY );
            const Tile& homeTile = theMap.GetTile( tileIndex );

            if( !IsTileValid( theMap, homeTile ) ) {
                continue;
            }

            // Check neighbors
            int numMatchingNeighbors = CountMatchingNeighbors( tileX, tileY, theMap, neighborValidities );

            if( m_ifNumNeighbors.IsIntInRange( numMatchingNeighbors ) ) {
                tileIndexesToChange.push_back( tileIndex );
            }
        }
    }

    // Actually change the tiles
    int numTilesToChange = (int)tileIndexesToChange.size();

    for( int changeIndex = 0; changeIndex < numTilesToChange; changeIndex++ ) {
        if( mapRNG->PercentChance( m_chancePerTile ) ) {
            ChangeTile( theMap, tileIndexesToChange[changeIndex] );
        }
    }
}


MGS_CellularAutomata::TileValidity MGS_CellularAutomata::IsNeighborTileValid( const Tile& neighbor ) const {
    TileValidity isValid = VALID;

    std::string tileType     = neighbor.GetTileType();
    const Metadata* metadata = neighbor.GetMetadata();
    const Tags& tileTags     = metadata->m_tagData;

    if( m_ifNeighborType != "" && tileType != m_ifNeighborType ) {
        isValid = INVALID;
    }
    
    if( !m_ifNeighborHasTags.empty() && !tileTags.HasTags(m_ifNeighborHasTags)) {
        isValid = INVALID;
    }

    return isValid;
}


int MGS_CellularAutomata::CountMatchingNeighbors( int homeTileX, int homeTileY, Map& theMap, std::vector< TileValidity >& tileValidities ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numMatchingNeighbors = 0;

    // For each neighbor within a radius (by X and Y)
    for( int neighborY = homeTileY - m_radius.max; neighborY <= homeTileY + m_radius.max; neighborY++ ) {
        if( neighborY < 0 || neighborY >= mapDimensions.y ) {
            continue;
        }

        for( int neighborX = homeTileX - m_radius.max; neighborX <= homeTileX + m_radius.max; neighborX++ ) {
            if( neighborX < 0 || neighborX >= mapDimensions.x ) {
                continue;
            }

            // Inside minimum radius check
            bool xRadiusInvalid = (abs( homeTileX - neighborX ) < m_radius.min);
            bool yRadiusInvalid = (abs( homeTileY - neighborY ) < m_radius.min);

            if( xRadiusInvalid && yRadiusInvalid ) {
                continue;
            }

            // Finally check this neighbor
            int tileIndex = theMap.GetTileIndex( neighborX, neighborY );

            const Tile& neighborTile = theMap.GetTile( tileIndex );
            TileValidity& validity = tileValidities[tileIndex];

            if( validity == TileValidity::UNKNOWN ) {
                validity = IsNeighborTileValid( neighborTile );
            }

            if( validity == TileValidity::VALID ) {
                numMatchingNeighbors++;
            }
        }
    }

    return numMatchingNeighbors;
}


IntRange MGS_CellularAutomata::ParseCustomIntRange( const std::string& rangeAsStr ) const {
    std::vector<std::string> splitString = SplitStringOnDelimeter( rangeAsStr, '~' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 1 || numValues == 2, Stringf( "Invalid initialization string (%s) for IntRange", rangeAsStr.c_str() ) );

    int minRange = 0;
    int maxRange = 0;

    if( numValues == 1 ) {  // radius="3"   --becomes--> IntRange( 1, 3 )
        minRange = 1;
        maxRange = atoi( splitString[0].c_str() );
    } else {                // radius="3~3" --becomes--> IntRange( 3, 3 )
        minRange = atoi( splitString[0].c_str() );
        maxRange = atoi( splitString[1].c_str() );
    }

    return IntRange( minRange, maxRange );
}


std::string MGS_CellularAutomata::SaveCustomIntRange( const IntRange& range ) const {
    if( range.min == 1 ) {
        return Stringf( "%d", range.max );
    } else {
        return Stringf( "%d~%d", range.min, range.max );
    }
}
