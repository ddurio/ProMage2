#include "Game/MapGen/GenSteps/MapGenStep.hpp"

#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/GenSteps/MGS_CellularAutomata.hpp"
#include "Game/MapGen/GenSteps/MGS_DistanceField.hpp"
#include "Game/MapGen/GenSteps/MGS_FromImage.hpp"
#include "Game/MapGen/GenSteps/MGS_PerlinNoise.hpp"
#include "Game/MapGen/GenSteps/MGS_RoomsAndPaths.hpp"
#include "Game/MapGen/GenSteps/MGS_Sprinkle.hpp"
#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Metadata.hpp"
#include "Game/MapGen/Map/Tile.hpp"

#include "regex"


std::vector< MapGenStep::CustomEvent > MapGenStep::s_customConditions;
std::vector< MapGenStep::CustomEvent > MapGenStep::s_customResults;


MapGenStep::MapGenStep( const XMLElement& element ) {
    if( s_mgsChannel == DevConsole::CHANNEL_UNDEFINED ) {
        s_mgsChannel = g_theDevConsole->AddChannelByName( "MapGen" );
    }

    m_stepType = element.Name();
    m_chanceToRun               = ParseXMLAttribute( element, "chanceToRun",      m_chanceToRun );
    m_numIterations             = ParseXMLAttribute( element, "numIterations",    m_numIterations );

    // Conditions
    m_ifIsType                  = ParseXMLAttribute( element, "ifIsType",         m_ifIsType );
    std::string ifHasTagsCSV    = ParseXMLAttribute( element, "ifHasTags",        "" );
    m_ifHasTags = SplitStringOnDelimeter( ifHasTagsCSV, ',', false );

    int numCustomConditions = (int)s_customConditions.size();

    for( int conditionIndex = 0; conditionIndex < numCustomConditions; conditionIndex++ ) {
        const CustomEvent& event = s_customConditions[conditionIndex];
        Strings values = event.ParseCustomEvent( element );
        int numNames = (int)event.attrNames.size();
        int numValues = (int)values.size();

        if( numValues == numNames ) {
            m_customConditions.emplace_back( event, values );
        }
    }

    // Results
    m_setType               = ParseXMLAttribute( element, "setType", m_setType );
    std::string setTagsCSV  = ParseXMLAttribute( element, "setTags", "" );
    m_setTags = SplitStringOnDelimeter( setTagsCSV, ',', false );

    int numCustomResults = (int)s_customResults.size();

    for( int resultIndex = 0; resultIndex < numCustomResults; resultIndex++ ) {
        const CustomEvent& event = s_customResults[resultIndex];
        Strings values = event.ParseCustomEvent( element );

        int numNames = (int)event.attrNames.size();
        int numValues = (int)values.size();

        if( numValues == numNames ) {
            m_customResults.emplace_back( event, values );
        }
    }

    // Heat Maps (expected format:  ifHeatMapDefault="2~5" or setHeatMapMyHeatMapName="4~10")
    const XMLAttribute* attribute = element.FirstAttribute();
    std::regex ifHeatMapRegex( "ifHeatMap(.+)", std::regex_constants::icase );
    std::regex setHeatMapRegex( "setHeatMap(.+)", std::regex_constants::icase );

    for( attribute; attribute != nullptr; attribute = attribute->Next() ) {
        std::string attrName = attribute->Name();
        std::string attrValue = attribute->Value();

        if( attrValue == "" ) {
            continue;
        }

        std::smatch heatMapMatch;

        if( regex_search( attrName, heatMapMatch, ifHeatMapRegex ) ) {
            std::string heatMapName = heatMapMatch[1];
            m_ifHeatMap[heatMapName] = FloatRange( attrValue );
        } else if( regex_search( attrName, heatMapMatch, setHeatMapRegex ) ) {
            std::string heatMapName = heatMapMatch[1];
            m_setHeatMap[heatMapName] = FloatRange( attrValue );
        }
    }
}


int MapGenStep::AddCustomCondition( const std::string& eventName, const Strings& attrNames, bool requireAllAttr /*= true */ ) {
    CustomEvent newCondition;
    newCondition.name = eventName;
    newCondition.attrNames = attrNames;
    newCondition.requireAll = requireAllAttr;

    int numConditions = (int)s_customConditions.size();

    for( int conditionIndex = 0; conditionIndex < numConditions; conditionIndex++ ) {
        const CustomEvent& oldEvent = s_customConditions[conditionIndex];

        if( !oldEvent.isEnabled ) {
            s_customConditions[conditionIndex] = newCondition;
            return conditionIndex;
        }
    }

    s_customConditions.push_back( newCondition );
    return numConditions;
}


int MapGenStep::AddCustomResult( const std::string& eventName, const Strings& attrNames, bool requireAllAttr /*= true */ ) {
    CustomEvent newResult;
    newResult.name = eventName;
    newResult.attrNames = attrNames;
    newResult.requireAll = requireAllAttr;

    int numResults = (int)s_customResults.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        const CustomEvent& oldEvent = s_customResults[resultIndex];

        if( !oldEvent.isEnabled ) {
            s_customResults[resultIndex] = newResult;
            return resultIndex;
        }
    }

    s_customResults.push_back( newResult );
    return numResults;
}


void MapGenStep::RemoveCustomCondition( int conditionIndex ) {
    int numConditions = (int)s_customConditions.size();

    if( conditionIndex >= 0 && conditionIndex < numConditions ) {
        s_customConditions[conditionIndex].isEnabled = false;
    }
}


void MapGenStep::RemoveCustomResult( int resultIndex ) {
    int numResults = (int)s_customResults.size();

    if( resultIndex >= 0 && resultIndex < numResults ) {
        s_customResults[resultIndex].isEnabled = false;
    }
}


MapGenStep* MapGenStep::CreateMapGenStep( const XMLElement& element ) {
    std::string stepType = element.Name();
    MapGenStep* step = nullptr;

    if( stepType == "Sprinkle" ) {
        step = new MGS_Sprinkle( element );
    } else if( stepType == "FromImage") {
        step = new MGS_FromImage( element );
    } else if( stepType == "CellularAutomata" ) {
        step = new MGS_CellularAutomata( element );
    } else if( stepType == "DistanceField" ) {
        step = new MGS_DistanceField( element );
    } else if( stepType == "PerlinNoise" ) {
        step = new MGS_PerlinNoise( element );
    } else if( stepType == "RoomsAndPaths" ) {
        step = new MGS_RoomsAndPaths( element );
    }

    return step;
}


void MapGenStep::Run( Map& map ) const {
    m_mapRNG = map.m_mapRNG;
    int numIterations = m_mapRNG->GetRandomIntInRange( m_numIterations );

    for( int iterationIndex = 0; iterationIndex < numIterations; iterationIndex++ ) {
        float iterationChance = m_mapRNG->GetRandomFloatInRange( m_chanceToRun );

        if( m_mapRNG->PercentChance( iterationChance ) ) {
            RunOnce( map );
        }
    }
}


// PROTECTED ---------------------------------------
bool MapGenStep::IsTileValid( Map& theMap, const Tile& tile ) const {
    std::string tileType     = tile.GetTileType();
    const Metadata* metadata = tile.GetMetadata();
    const Tags& tileTags     = metadata->m_tagData;

    // Tile Type
    if( m_ifIsType != "" && tileType != m_ifIsType ) {
        return false;
    }

    // Tags
    if( !m_ifHasTags.empty() && !tileTags.HasTags( m_ifHasTags )) {
        return false;
    }

    // Heat Maps
    std::map< std::string, FloatRange, StringCmpCaseI >::const_iterator heatMapIter = m_ifHeatMap.begin();

    for( heatMapIter; heatMapIter != m_ifHeatMap.end(); heatMapIter++ ) {
        const std::string& heatMapName = heatMapIter->first;
        const FloatRange& heatMapRange = heatMapIter->second;
        float heatMapValue;

        if( !tile.GetHeatMap( heatMapName, heatMapValue ) ||
            !IsFloatInRange( heatMapValue, heatMapRange ) ) {
            return false;
        }
    }

    // Custom Conditions
    int numCustomConditions = (int)m_customConditions.size();

    for( int conditionIndex = 0; conditionIndex < numCustomConditions; conditionIndex++ ) {
        const CustomEvent& event = m_customConditions[conditionIndex];

        if( !event.isEnabled ) {
            continue;
        }

        EventArgs args = event.CreateEventArgs();
        args.SetValue( "callingTile", &tile );
        args.SetValue( "callingMap", &theMap );

        g_theEventSystem->FireEvent( event.name, args );

        if( !args.GetValue( "isValid", false ) ) {
            return false;
        }
    }

    return true;
}


void MapGenStep::ChangeTile( Map& theMap, int tileIndex ) const {
    Tile& tile = theMap.m_tiles[tileIndex];

    // Tile Type
    if( m_setType != "" ) {
        tile.SetTileType( m_setType );
    }

    // Tags
    if( !m_setTags.empty() ) {
        Metadata* tileMetadata = tile.GetMetadata();
        Tags& tileTags = tileMetadata->m_tagData;

        tileTags.SetTags( m_setTags );
    }

    // Heat Maps
    std::map< std::string, FloatRange, StringCmpCaseI >::const_iterator heatMapIter = m_setHeatMap.begin();

    for( heatMapIter; heatMapIter != m_setHeatMap.end(); heatMapIter++ ) {
        const std::string& heatMapName = heatMapIter->first;
        const FloatRange& heatMapRange = heatMapIter->second;

        float heatMapValue = m_mapRNG->GetRandomFloatInRange( heatMapRange );
        tile.SetHeatMap( heatMapName, heatMapValue );
    }

    // Custom Results
    int numCustomResults = (int)m_customResults.size();

    for( int resultIndex = 0; resultIndex < numCustomResults; resultIndex++ ) {
        const CustomEvent& event = m_customResults[resultIndex];

        if( !event.isEnabled ) {
            continue;
        }

        EventArgs args = event.CreateEventArgs();
        args.SetValue( "callingTile", &tile );
        args.SetValue( "callingMap", &theMap );

        g_theEventSystem->FireEvent( event.name, args );
    }
}


void MapGenStep::ChangeTile( Map& theMap, int tileX, int tileY ) const {
    int tileIndex = theMap.GetTileIndex( tileX, tileY );
    ChangeTile( theMap, tileIndex );
}


// PROTECTED -------------------------------------------------
DevConsoleChannel MapGenStep::s_mgsChannel = DevConsole::CHANNEL_UNDEFINED;


Tile& MapGenStep::GetTile( Map& map, int tileIndex ) const {
    return map.m_tiles[tileIndex];
}


Tile& MapGenStep::GetTile( Map& map, int tileX, int tileY ) const {
    int tileIndex = map.GetTileIndex( tileX, tileY );
    return GetTile( map, tileIndex );
}


// PRIVATE -------------------------------------------------
MapGenStep::CustomEvent::CustomEvent( const CustomEvent& event, const Strings& parsedValues ) :
    name( event.name ),
    requireAll( event.requireAll ),
    attrNames( event.attrNames ),
    attrValues( parsedValues ) {
}


Strings MapGenStep::CustomEvent::ParseCustomEvent( const XMLElement& element ) const {
    int numAttrs = (int)attrNames.size();
    int numParsed = 0;

    Strings parsedValues;
    parsedValues.resize( numAttrs );

    for( int nameIndex = 0; nameIndex < numAttrs; nameIndex++ ) {
        const std::string& attrName = attrNames[nameIndex];
        parsedValues[nameIndex] = ParseXMLAttribute( element, attrName.c_str(), "" );

        if( parsedValues[nameIndex] != "" ) {
            numParsed++;
        }
    }

    if( numParsed == 0 || (requireAll && numParsed != numAttrs) ) {
        return Strings();
    } else {
        return parsedValues;
    }
}



EventArgs MapGenStep::CustomEvent::CreateEventArgs() const {
    EventArgs args;
    int numNames = (int)attrNames.size();
    int numValues = (int)attrValues.size();
    GUARANTEE_OR_DIE( numNames == numValues, "(MapGenStep) Number of custom event attribute names and events must match!" );

    for( int attrIndex = 0; attrIndex < numNames; attrIndex++ ) {
        const std::string& attrName = attrNames[attrIndex];
        const std::string& attrValue = attrValues[attrIndex];

        args.SetValue( attrName, attrValue );
    }

    return args;
}
