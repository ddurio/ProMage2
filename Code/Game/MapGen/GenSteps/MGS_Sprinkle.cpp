#include "Game/MapGen/GenSteps/MGS_Sprinkle.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"


MGS_Sprinkle::MGS_Sprinkle( const XMLElement& element ) :
    MapGenStep( element ) {
    m_countRange = ParseXMLAttribute( element, "count", m_countRange );
}


void MGS_Sprinkle::RunOnce( Map& map ) const {
    int numSprinkles = m_mapRNG->GetRandomIntInRange( m_countRange );

    IntVec2 mapDimensions = map.GetMapDimensions();
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

            tileIndex = m_mapRNG->GetRandomIntLessThan( numTiles );
            tile = &(map.GetTile( tileIndex ));
            
            numAttempts++;
        } while( !IsTileValid( *tile ) );

        ChangeTile( map, tileIndex );
    }
}