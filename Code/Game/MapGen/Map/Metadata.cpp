#include "Game/MapGen/Map/Metadata.hpp"


bool Metadata::IsHeatMapSet( const std::string& heatMapName ) const {
    std::map< std::string, float, StringCmpCaseI >::const_iterator heatMapIter = m_heatMaps.find( heatMapName );

    return (heatMapIter != m_heatMaps.end());
}
