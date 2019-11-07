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
#include "Game/MapGen/GenSteps/MGS_Custom.hpp"
#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"
#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Metadata.hpp"
#include "Game/MapGen/Map/Tile.hpp"

#include "regex"


std::vector< MapGenStep::CustomEvent > MapGenStep::s_customConditions;
std::vector< MapGenStep::CustomEvent > MapGenStep::s_customResults;


MapGenStep::MapGenStep( const XMLElement& element, const Strings& motifHierarchy ) {
    if( s_mgsChannel == DevConsole::CHANNEL_UNDEFINED ) {
        s_mgsChannel = g_theDevConsole->AddChannel( "MapGen", Rgba::ORGANIC_GREEN );
    }

    m_stepType                  = element.Name();
    std::string stepMotif       = ParseXMLAttribute( element, "motif", "" );
    AddChildMotifs( { stepMotif } );
    AddParentMotifs( motifHierarchy );

    m_chanceToRun               = ParseXMLAttribute( element, "chanceToRun",      m_motifVars,  m_motifHierarchy,   m_chanceToRun );
    m_numIterations             = ParseXMLAttribute( element, "numIterations",    m_motifVars,  m_motifHierarchy,   m_numIterations );

    // Conditions
    m_ifIsType                  = ParseXMLAttribute( element, "ifIsType",         m_motifVars,  m_motifHierarchy,   m_ifIsType );
    std::string ifHasTagsCSV    = ParseXMLAttribute( element, "ifHasTags",        m_motifVars,  m_motifHierarchy,   "" );
    m_ifHasTags                 = SplitStringOnDelimeter( ifHasTagsCSV, ',', false );

    int numCustomConditions = (int)s_customConditions.size();

    for( int conditionIndex = 0; conditionIndex < numCustomConditions; conditionIndex++ ) {
        const CustomEvent& sEvent = s_customConditions[conditionIndex];
        Strings values = sEvent.ParseCustomEvent( element, this );

        int numNames = (int)sEvent.attrNames.size();
        int numValues = (int)values.size();

        if( numValues == numNames ) {
            m_customConditions.emplace_back( sEvent, values );
        }
    }

    // Results
    m_setType               = ParseXMLAttribute( element, "setType", m_motifVars,  m_motifHierarchy,    m_setType );
    std::string setTagsCSV  = ParseXMLAttribute( element, "setTags", m_motifVars,  m_motifHierarchy,    "" );
    m_setTags = SplitStringOnDelimeter( setTagsCSV, ',', false );

    int numCustomResults = (int)s_customResults.size();

    for( int resultIndex = 0; resultIndex < numCustomResults; resultIndex++ ) {
        const CustomEvent& event = s_customResults[resultIndex];
        Strings values = event.ParseCustomEvent( element, this );

        int numNames = (int)event.attrNames.size();
        int numValues = (int)values.size();

        if( numValues == numNames ) {
            m_customResults.emplace_back( event, values );
        }
    }

    // Heat Maps (expected format:  ifHeatMapDefault="2~5" or setHeatMapMyHeatMapName="4~10")
    std::regex ifHeatMapRegex( "ifHeatMap(.+)", std::regex_constants::icase );
    std::regex setHeatMapRegex( "setHeatMap(.+)", std::regex_constants::icase );
    const XMLAttribute* attribute = element.FirstAttribute();

    for( attribute; attribute != nullptr; attribute = attribute->Next() ) {
        std::string attrName = attribute->Name();
        std::string attrValue = attribute->Value();

        if( attrValue == "" ) {
            continue;
        }

        std::smatch heatMapMatch;

        if( regex_search( attrName, heatMapMatch, ifHeatMapRegex ) ) {
            GetXMLMotifVariable( element, attrName.c_str(), m_motifVars );
            std::string heatMapName = heatMapMatch[1];
            m_ifHeatMap[heatMapName] = FloatRange( attrValue );

        } else if( regex_search( attrName, heatMapMatch, setHeatMapRegex ) ) {
            GetXMLMotifVariable( element, attrName.c_str(), m_motifVars );
            std::string heatMapName = heatMapMatch[1];
            m_setHeatMap[heatMapName] = FloatRange( attrValue );
        }
    }
}


// Custom Event ----------------------------------------------------------------------
MapGenStep::CustomEvent::CustomEvent( const CustomEvent& event, const Strings& parsedValues ) :
    name( event.name ),
    requirement( event.requirement ),
    attrNames( event.attrNames ),
    attrValues( parsedValues ) {
}


Strings MapGenStep::CustomEvent::ParseCustomEvent( const XMLElement& element, MapGenStep* genStep ) const {
    int numAttrs = (int)attrNames.size();
    int numParsed = 0;

    Strings parsedValues;
    parsedValues.resize( numAttrs );

    for( int nameIndex = 0; nameIndex < numAttrs; nameIndex++ ) {
        const std::string& attrName = attrNames[nameIndex];
        parsedValues[nameIndex] = ParseXMLAttribute( element, attrName.c_str(), genStep->m_motifVars, genStep->m_motifHierarchy, "" );

        if( parsedValues[nameIndex] != "" ) {
            numParsed++;
        }
    }

    switch( requirement ) {
        case( REQUIRE_ALL ): {
            return (numParsed == numAttrs) ? parsedValues : Strings();
        } case( REQUIRE_ONE ): {
            return (numParsed > 0) ? parsedValues : Strings();
        } case( REQUIRE_NONE ): {
            return parsedValues;
        }
    }

    return Strings();
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


// Static --------------------------------------------------------------------
int MapGenStep::AddCustomCondition( const std::string& eventName, const Strings& attrNames, CustomAttrRequirement requirement /*= REQUIRE_ALL */ ) {
    CustomEvent newCondition;
    newCondition.name = eventName;
    newCondition.attrNames = attrNames;
    newCondition.requirement = requirement;

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


int MapGenStep::AddCustomResult( const std::string& eventName, const Strings& attrNames, CustomAttrRequirement requirement /*= REQUIRE_ALL */ ) {
    CustomEvent newResult;
    newResult.name = eventName;
    newResult.attrNames = attrNames;
    newResult.requirement = requirement;

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


MapGenStep* MapGenStep::CreateMapGenStep( const XMLElement& element, const Strings& motifHierarchy ) {
    std::string stepType = element.Name();
    MapGenStep* step = nullptr;

    if( StringICmp( stepType, "CellularAutomata" ) ) {
        step = new MGS_CellularAutomata( element, motifHierarchy );
    } else if( StringICmp( stepType, "DistanceField" ) ) {
        step = new MGS_DistanceField( element, motifHierarchy );
    } else if( StringICmp( stepType, "FromImage" ) ) {
        step = new MGS_FromImage( element, motifHierarchy );
    } else if( StringICmp( stepType, "PerlinNoise" ) ) {
        step = new MGS_PerlinNoise( element, motifHierarchy );
    } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
        step = new MGS_RoomsAndPaths( element, motifHierarchy );
    } else if( StringICmp( stepType, "Sprinkle" ) ) {
        step = new MGS_Sprinkle( element, motifHierarchy );
    } else {
        const MGS_CustomDef* virtualDef = MGS_CustomDef::GetDefinition( stepType );

        if( virtualDef != nullptr ) {
            step = new MGS_Custom( element, motifHierarchy );
        } else {
            ERROR_RECOVERABLE( Stringf( "(MapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
        }
    }

    return step;
}


MapGenStep* MapGenStep::CreateMapGenStep( const MapGenStep* stepToCopy ) {
    std::string stepType = stepToCopy->m_stepType;
    MapGenStep* newStep = nullptr;

    if( StringICmp( stepType, "CellularAutomata" ) ) {
        MGS_CellularAutomata* caStepToCopy = (MGS_CellularAutomata*)stepToCopy;
        newStep = new MGS_CellularAutomata( *caStepToCopy );

    } else if( StringICmp( stepType, "DistanceField" ) ) {
        MGS_DistanceField* dfStepToCopy = (MGS_DistanceField*)stepToCopy;
        newStep = new MGS_DistanceField( *dfStepToCopy );

    } else if( StringICmp( stepType, "FromImage" ) ) {
        MGS_FromImage* fiStepToCopy = (MGS_FromImage*)stepToCopy;
        newStep = new MGS_FromImage( *fiStepToCopy );

    } else if( StringICmp( stepType, "PerlinNoise" ) ) {
        MGS_PerlinNoise* pnStepToCopy = (MGS_PerlinNoise*)stepToCopy;
        newStep = new MGS_PerlinNoise( *pnStepToCopy );

    } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
        MGS_RoomsAndPaths* rnpStepToCopy = (MGS_RoomsAndPaths*)stepToCopy;
        newStep = new MGS_RoomsAndPaths( *rnpStepToCopy );

    } else if( StringICmp( stepType, "Sprinkle" ) ) {
        MGS_Sprinkle* sStepToCopy = (MGS_Sprinkle*)stepToCopy;
        newStep = new MGS_Sprinkle( *sStepToCopy );

    } else {
        ERROR_RECOVERABLE( Stringf( "(MapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
    }

    return newStep;
}


MapGenStep* MapGenStep::CreateMapGenStep( const std::string& stepType ) {
    MapGenStep* newStep = nullptr;

    if( StringICmp( stepType, "CellularAutomata" ) ) {
        newStep = new MGS_CellularAutomata();
    } else if( StringICmp( stepType, "DistanceField" ) ) {
        newStep = new MGS_DistanceField();
    } else if( StringICmp( stepType, "FromImage" ) ) {
        newStep = new MGS_FromImage();
    } else if( StringICmp( stepType, "PerlinNoise" ) ) {
        newStep = new MGS_PerlinNoise();
    } else if( StringICmp( stepType, "RoomsAndPaths" ) ) {
        newStep = new MGS_RoomsAndPaths();
    } else if( StringICmp( stepType, "Sprinkle" ) ) {
        newStep = new MGS_Sprinkle();
    } else {
        ERROR_RECOVERABLE( Stringf( "(MapGenStep): Unrecognized step type '%s'", stepType.c_str() ) );
    }

    newStep->m_stepType = stepType;
    return newStep;
}


// Public ----------------------------------------------------
void MapGenStep::Run( Map& theMap ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    int numIterations = mapRNG->GetRandomIntInRange( m_numIterations );

    for( int iterationIndex = 0; iterationIndex < numIterations; iterationIndex++ ) {
        if( mapRNG->PercentChance( m_chanceToRun ) ) {
            RunOnce( theMap );
        }
    }
}


std::vector< MapGenStep::CustomEvent > MapGenStep::GetCustomResults() const {
    return m_customResults;
}


std::string MapGenStep::GetName() const {
    return m_stepType;
}


const Strings& MapGenStep::GetMotifs() const {
    return m_motifHierarchy;
}


void MapGenStep::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    UNUSED( document );

    // Conditions
    if( m_chanceToRun != 1.f ) {
        element.SetAttribute( "chanceToRun", m_chanceToRun );
    }

    if( m_numIterations != IntRange::ONE ) {
        element.SetAttribute( "numIterations", m_numIterations.GetAsString().c_str() );
    }

    if( m_ifIsType != "" ) {
        element.SetAttribute( "ifIsType", m_ifIsType.c_str() );
    }

    if( !m_ifHasTags.empty() ) {
        std::string ifHasTagsCSV = JoinStrings( m_ifHasTags, "," );
        element.SetAttribute( "ifHasTags", ifHasTagsCSV.c_str() );
    }

    std::map< std::string, FloatRange, StringCmpCaseI >::const_iterator ifHeatIter = m_ifHeatMap.begin();

    while( ifHeatIter != m_ifHeatMap.end() ) {
        std::string attrName = Stringf( "ifHeatMap%s", ifHeatIter->first.c_str() );
        element.SetAttribute( attrName.c_str(), ifHeatIter->second.GetAsString().c_str() );

        ifHeatIter++;
    }

    int numConditions = (int)m_customConditions.size();

    for( int condIndex = 0; condIndex < numConditions; condIndex++ ) {
        const CustomEvent& condition = m_customConditions[condIndex];

        if( condition.isEnabled ) {
            int numAttr = (int)condition.attrNames.size();

            for( int attrIndex = 0; attrIndex < numAttr; attrIndex++ ) {
                std::string name = condition.attrNames[attrIndex];
                std::string value = condition.attrValues[attrIndex];
                element.SetAttribute( name.c_str(), value.c_str() );
            }
        }
    }

    // Results
    if( m_setType != "" ) {
        element.SetAttribute( "setType", m_setType.c_str() );
    }

    if( !m_setTags.empty() ) {
        std::string setTagsCSV = JoinStrings( m_setTags, "," );
        element.SetAttribute( "setTags", setTagsCSV.c_str() );
    }

    std::map< std::string, FloatRange, StringCmpCaseI >::const_iterator setHeatIter = m_setHeatMap.begin();

    while( setHeatIter != m_setHeatMap.end() ) {
        std::string attrName = Stringf( "setHeatMap%s", setHeatIter->first.c_str() );
        element.SetAttribute( attrName.c_str(), setHeatIter->second.GetAsString().c_str() );

        setHeatIter++;
    }

    int numResults = (int)m_customResults.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        const CustomEvent& result = m_customResults[resultIndex];

        if( result.isEnabled ) {
            int numAttr = (int)result.attrNames.size();

            for( int attrIndex = 0; attrIndex < numAttr; attrIndex++ ) {
                std::string name = result.attrNames[attrIndex];
                std::string value = result.attrValues[attrIndex];
                element.SetAttribute( name.c_str(), value.c_str() );
            }
        }
    }
}


bool MapGenStep::RecalculateMotifVars( EventArgs& args ) {
    std::string attrName = args.GetValue( "attrName", "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, "All" );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "chanceToRun" ) ) {
        varName = m_motifVars.GetValue( "chanceToRun", "" );
        m_chanceToRun = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_chanceToRun );
    }

    if( calcAllVars || StringICmp( attrName, "numIterations" ) ) {
        varName = m_motifVars.GetValue( "numIterations", "" );
        m_numIterations = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_numIterations );
    }
    
    if( calcAllVars || StringICmp( attrName, "ifIsType" ) ) {
        varName = m_motifVars.GetValue( "ifIsType", "" );
        m_ifIsType = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_ifIsType );
    }
    
    if( calcAllVars || StringICmp( attrName, "ifHasTags" ) ) {
        varName = m_motifVars.GetValue( "ifHasTags", "" );
        std::string joinedTags = JoinStrings( m_ifHasTags );
        std::string tagCSV = MotifDef::GetVariableValue( m_motifHierarchy, varName, joinedTags );
        m_ifHasTags = SplitStringOnDelimeter( tagCSV, ',', false );
    }

    int numCondEvents = (int)m_customConditions.size();

    for( int condIndex = 0; condIndex < numCondEvents; condIndex++ ) {
        CustomEvent&  cEvent = m_customConditions[condIndex];
        int numNames = (int)cEvent.attrNames.size();

        for( int nameIndex = 0; nameIndex < numNames; nameIndex++ ) {
            const std::string& eventAttrName = cEvent.attrNames[nameIndex];
            std::string& eventAttrValue = cEvent.attrValues[nameIndex];

            if( calcAllVars || StringICmp( attrName, eventAttrName ) ) {
                varName = m_motifVars.GetValue( eventAttrName, "" );
                eventAttrValue = MotifDef::GetVariableValue( m_motifHierarchy, varName, eventAttrValue );
            }
        }
    }

    HeatMaps::iterator heatIter = m_ifHeatMap.begin();

    for( heatIter; heatIter != m_ifHeatMap.end(); heatIter++ ) {
        std::string heatAttrName = Stringf( "ifHeatMap%s", heatIter->first.c_str() );

        if( calcAllVars || StringICmp( attrName, heatAttrName ) ) {
            varName = m_motifVars.GetValue( heatAttrName, "" );
            heatIter->second = MotifDef::GetVariableValue( m_motifHierarchy, varName, heatIter->second );
        }
    }

    if( calcAllVars || StringICmp( attrName, "setType" ) ) {
        varName = m_motifVars.GetValue( "setType", "" );
        m_setType = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_setType );
    }
    
    if( calcAllVars || StringICmp( attrName, "setTags" ) ) {
        varName = m_motifVars.GetValue( "setTags", "" );
        std::string joinedTags = JoinStrings( m_setTags );
        std::string tagCSV = MotifDef::GetVariableValue( m_motifHierarchy, varName, joinedTags );
        m_setTags = SplitStringOnDelimeter( tagCSV, ',', false );
    }

    int numResultEvents = (int)m_customResults.size();

    for( int resultIndex = 0; resultIndex < numResultEvents; resultIndex++ ) {
        CustomEvent&  cEvent = m_customResults[resultIndex];
        int numNames = (int)cEvent.attrNames.size();

        for( int nameIndex = 0; nameIndex < numNames; nameIndex++ ) {
            const std::string& eventAttrName = cEvent.attrNames[nameIndex];
            std::string& eventAttrValue = cEvent.attrValues[nameIndex];

            if( calcAllVars || StringICmp( attrName, eventAttrName ) ) {
                varName = m_motifVars.GetValue( eventAttrName, "" );
                eventAttrValue = MotifDef::GetVariableValue( m_motifHierarchy, varName, eventAttrValue );
            }
        }
    }

    heatIter = m_setHeatMap.begin();

    for( heatIter; heatIter != m_setHeatMap.end(); heatIter++ ) {
        std::string heatAttrName = Stringf( "setHeatMap%s", heatIter->first.c_str() );

        if( calcAllVars || StringICmp( attrName, heatAttrName ) ) {
            varName = m_motifVars.GetValue( heatAttrName, "" );
            heatIter->second = MotifDef::GetVariableValue( m_motifHierarchy, varName, heatIter->second );
        }
    }

    return false;
}


void MapGenStep::SetMotifs( const Strings& motifsToSet ) {
    m_motifHierarchy = motifsToSet;
}


void MapGenStep::AddParentMotifs( const Strings& parentMotifs ) {
    // Add at the back of the list (least important)
    m_motifHierarchy.insert( m_motifHierarchy.end(),parentMotifs.begin(), parentMotifs.end() );
}


void MapGenStep::AddChildMotifs( const Strings& motifsToAdd ) {
    // Add at the front of the list (most important)
    m_motifHierarchy.insert( m_motifHierarchy.begin(), motifsToAdd.begin(), motifsToAdd.end() );
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
            !IsInRange( heatMapValue, heatMapRange ) ) {
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
    RNG* mapRNG = theMap.GetMapRNG();
    Tile& tile = theMap.m_tiles[tileIndex];

    ChangeTileType( tile );
    ChangeTileTags( tile );
    ChangeTileHeatMaps( mapRNG, tile );
    ChangeTilesCustomResults( theMap, tile );
}


void MapGenStep::ChangeTile( Map& theMap, int tileX, int tileY ) const {
    int tileIndex = theMap.GetTileIndex( tileX, tileY );
    ChangeTile( theMap, tileIndex );
}


void MapGenStep::ChangeTileType( Tile& tile, const std::string& customType /*= "" */ ) const {
    std::string newType = (customType == "") ? m_setType : customType;

    if( newType != "" ) {
        tile.SetTileType( newType );
    }
}


void MapGenStep::ChangeTileTags( Tile& tile, const Strings& customTags /*= Strings() */ ) const {
    const Strings& tagsToSet = (customTags.empty()) ? m_setTags : customTags;

    if( !tagsToSet.empty() ) {
        Metadata* tileMetadata = tile.GetMetadata();
        Tags& tileTags = tileMetadata->m_tagData;

        tileTags.SetTags( tagsToSet );
    }
}


void MapGenStep::ChangeTileHeatMaps( RNG* mapRNG, Tile& tile, const HeatMaps& customHeatMaps /*= HeatMaps() */ ) const {
    const HeatMaps& heatMapToSet = (customHeatMaps.empty()) ? m_setHeatMap : customHeatMaps;
    std::map< std::string, FloatRange, StringCmpCaseI >::const_iterator heatMapIter = heatMapToSet.begin();

    for( heatMapIter; heatMapIter != m_setHeatMap.end(); heatMapIter++ ) {
        const std::string& heatMapName = heatMapIter->first;
        const FloatRange& heatMapRange = heatMapIter->second;

        float heatMapValue = mapRNG->GetRandomFloatInRange( heatMapRange );
        tile.SetHeatMap( heatMapName, heatMapValue );
    }
}


void MapGenStep::ChangeTilesCustomResults( Map& theMap, Tile& tile ) const {
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


// PROTECTED -------------------------------------------------
DevConsoleChannel MapGenStep::s_mgsChannel = DevConsole::CHANNEL_UNDEFINED;


Tile& MapGenStep::GetTile( Map& map, int tileIndex ) const {
    return map.m_tiles[tileIndex];
}


Tile& MapGenStep::GetTile( Map& map, int tileX, int tileY ) const {
    int tileIndex = map.GetTileIndex( tileX, tileY );
    return GetTile( map, tileIndex );
}
