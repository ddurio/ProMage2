#include "Game/MapGen/GenSteps/MGS_Sprinkle.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"


MGS_Sprinkle::MGS_Sprinkle( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    m_countRange = ParseXMLAttribute( element, "count", m_motifVars, m_motifHierarchy, m_countRange );
}


MGS_Sprinkle::MGS_Sprinkle( const Strings& motifHierarchy ) :
    MapGenStep( motifHierarchy ) {
}


void MGS_Sprinkle::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetName( "Sprinkle" );
    MapGenStep::SaveToXml( document, element );

    std::string varName = "count";

    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_countRange != IntRange::ONE ) {
        element.SetAttribute( varName.c_str(), m_countRange.GetAsString().c_str() );
    }
}


bool MGS_Sprinkle::RecalculateMotifVars( EventArgs& args ) {
    MapGenStep::RecalculateMotifVars( args );

    std::string attrName = args.GetValue( MAPGEN_ARG_ATTR_NAME, "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, MAPGEN_ARG_RECALC_ALL );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "count" ) ) {
        varName = m_motifVars.GetValue( "count", "" );
        m_countRange = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_countRange );
    }

    return false;
}


void MGS_Sprinkle::RunOnce( Map& theMap ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    int numSprinkles = mapRNG->GetRandomIntInRange( m_countRange );

    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int sprinkleIndex = 0; sprinkleIndex < numSprinkles; sprinkleIndex++ ) {
        int tileIndex = -1;
        const Tile* tile = nullptr;
        int numAttempts = 0;

        do {
            if( numAttempts > numTiles ) {
                std::string warningMsg = Stringf( "(MGS_Sprinkle) WARNING: %d attempts failed to find a matching tile", numTiles );
                g_theDevConsole->PrintString( warningMsg, s_mgsChannel | DevConsole::CHANNEL_WARNING );
                return;
            }

            tileIndex = mapRNG->GetRandomIntLessThan( numTiles );
            tile = &(theMap.GetTile( tileIndex ));
            
            numAttempts++;
        } while( !IsTileValid( theMap, *tile ) );

        ChangeTile( theMap, tileIndex );
    }
}
