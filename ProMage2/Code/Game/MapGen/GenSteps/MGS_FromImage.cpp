#include "Game/MapGen/GenSteps/MGS_FromImage.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/TileDef.hpp"

MGS_FromImage::MGS_FromImage( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    m_imageFilePath = ParseXMLAttribute( element, "imageFilePath",  m_motifVars, m_motifHierarchy,   m_imageFilePath );
    GUARANTEE_OR_DIE( m_imageFilePath != "", "ERROR: XML Attribute 'imageFilePath' missing for MapGenStep_FromImage" );
    Startup();

    m_alignX        = ParseXMLAttribute( element, "alignX",         m_motifVars, m_motifHierarchy,   m_alignX );
    m_alignY        = ParseXMLAttribute( element, "alignY",         m_motifVars, m_motifHierarchy,   m_alignY );
    m_numRotations  = ParseXMLAttribute( element, "numRotations",   m_motifVars, m_motifHierarchy,   m_numRotations );
}


MGS_FromImage::MGS_FromImage( const MGS_FromImage& copyFrom ) :
    MapGenStep( copyFrom ) {
    m_imageFilePath = copyFrom.m_imageFilePath;
    Startup();

    m_alignX        = copyFrom.m_alignX;
    m_alignY        = copyFrom.m_alignY;
    m_numRotations  = copyFrom.m_numRotations;
}


MGS_FromImage::MGS_FromImage( const Strings& motifHierarchy ) :
    MapGenStep( motifHierarchy ) {
}


MGS_FromImage::~MGS_FromImage() {
    Shutdown();
}


void MGS_FromImage::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetName( "FromImage" );
    MapGenStep::SaveToXml( document, element );

    element.SetAttribute( "imageFilePath", m_imageFilePath.c_str() );

    std::string varName = "alignX";

    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_alignX != FloatRange::ZEROTOONE ) {
        element.SetAttribute( varName.c_str(), m_alignX.GetAsString().c_str() );
    }

    varName = "alignY";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_alignY != FloatRange::ZEROTOONE ) {
        element.SetAttribute( varName.c_str(), m_alignY.GetAsString().c_str() );
    }

    varName = "numRotations";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_numRotations != IntRange::ZERO ) {
        element.SetAttribute( varName.c_str(), m_numRotations.GetAsString().c_str() );
    }
}


bool MGS_FromImage::RecalculateMotifVars( EventArgs& args ) {
    MapGenStep::RecalculateMotifVars( args );

    std::string attrName = args.GetValue( MAPGEN_ARG_ATTR_NAME, "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, MAPGEN_ARG_RECALC_ALL );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "imageFilePath" ) ) {
        varName = m_motifVars.GetValue( "imageFilePath", "" );
        m_imageFilePath = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_imageFilePath );
    }
    
    if( calcAllVars || StringICmp( attrName, "alignX" ) ) {
        varName = m_motifVars.GetValue( "alignX", "" );
        m_alignX = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_alignX );
    }
    
    if( calcAllVars || StringICmp( attrName, "alignY" ) ) {
        varName = m_motifVars.GetValue( "alignY", "" );
        m_alignY = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_alignY );
    }
    
    if( calcAllVars || StringICmp( attrName, "numRotations" ) ) {
        varName = m_motifVars.GetValue( "numRotations", "" );
        m_numRotations = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_numRotations );
    }

    return false;
}


// PRIVATE ---------------------------------------------
std::map< std::string, const TileDef* > MGS_FromImage::s_tileDefsByTexelColor;


void MGS_FromImage::Startup() const {
    m_image = new Image( m_imageFilePath );
}


void MGS_FromImage::Shutdown() const {
    CLEAR_POINTER( m_image );
}


void MGS_FromImage::RunOnce( Map& theMap ) const {
    if( m_image == nullptr || m_imageFilePath != m_image->GetImageFilePath() ) {
        // Likely the editor changed the file path
        Shutdown();
        Startup();
    }

    RNG* mapRNG = theMap.GetMapRNG();
    IntVec2 imageDimensions = m_image->GetDimensions();
    IntVec2 mapDimensions = theMap.GetMapDimensions();

    IntVec2 imageOffset = GetImageOffset( theMap );

    if( imageOffset == IntVec2::NEGONE ) {
        return; // Couldn't find spot to put it
    }

    int rotations = mapRNG->GetRandomIntInRange( m_numRotations );
    Image rotatedImage;
    m_image->GetRotated( rotations, rotatedImage );

    for( int texelX = 0, mapX = imageOffset.x; texelX < imageDimensions.x && mapX < mapDimensions.x; texelX++, mapX++ ) {
        for( int texelY = imageDimensions.y - 1, mapY = imageOffset.y; texelY >= 0 && mapY < mapDimensions.y; texelY--, mapY++ ) {
            Rgba texelColor = rotatedImage.GetTexelColor( texelX, texelY );
            float chanceToExist = texelColor.a;

            if( chanceToExist == 0.f ) {
                continue;
            }

            texelColor.a = 1.f; // color to match will always have alpha set to one
            const TileDef* tileDef = GetTileDef( texelColor );

            if( tileDef == nullptr ) {
                std::string warningMsg = Stringf( "(MGS_FromImage): TileDef not found with requested texelColor %s", texelColor.GetAsString().c_str() );
                g_theDevConsole->PrintString( warningMsg, s_mgsChannel | DevConsole::CHANNEL_WARNING );
                continue;
            }

            if( mapRNG->PercentChance( chanceToExist ) ) {
                int tileIndex = mapY * mapDimensions.x + mapX;
                Tile& tile = GetTile( theMap, tileIndex );
                tile.SetTileType( tileDef );
            }
        }
    }
}


bool MGS_FromImage::IsAlignmentValid( Map& theMap, const AABB2& imageBounds ) const {
    int minX = (int)imageBounds.mins.x;
    int maxX = (int)imageBounds.maxs.x; // Max value is float (far right of tile).. not tile index
    int minY = (int)imageBounds.mins.y;
    int maxY = (int)imageBounds.maxs.y; // Means tiles used are one less than max value (< not <= in loops)

    for( int tileY = minY; tileY < maxY; tileY++ ) {
        for( int tileX = minX; tileX < maxX; tileX++ ) {
            const Tile* tile = nullptr;

            if( theMap.GetTileIfValid( tile, IntVec2( tileX, tileY ) ) ) {
                if( !IsTileValid( theMap, *tile ) ) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    return true;
}


IntVec2 MGS_FromImage::GetImageOffset( Map& theMap ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    IntVec2 imageDimensions = m_image->GetDimensions();
    IntVec2 mapDimensions = theMap.GetMapDimensions();

    AABB2 imageBounds = AABB2::ZEROTOONE;
    int numAttempts = 0;

    do {
        if( numAttempts > 1000 ) {
            return IntVec2::NEGONE;
        }

        float alignX = mapRNG->GetRandomFloatInRange( m_alignX );
        float alignY = mapRNG->GetRandomFloatInRange( m_alignY );

        AABB2 mapBounds = AABB2( Vec2::ZERO, Vec2( mapDimensions ) );
        imageBounds = mapBounds.GetBoxWithin( Vec2( imageDimensions ), Vec2( alignX, alignY ) );

        numAttempts++;
    } while( !IsAlignmentValid( theMap, imageBounds ) );

    int minX = (int)imageBounds.mins.x;
    int minY = (int)imageBounds.mins.y;

    return IntVec2( minX, minY );
}


const TileDef* MGS_FromImage::GetTileDef( const Rgba& texelColor ) {
    std::string texelStr = texelColor.GetAsString();
    std::map< std::string, const TileDef* >::iterator tileDefIter = s_tileDefsByTexelColor.find( texelStr );

    if( tileDefIter != s_tileDefsByTexelColor.end() ) {
        return tileDefIter->second;
    } else {
        const TileDef* tileDef = TileDef::GetTileDefFromTexelColor( texelColor );
        s_tileDefsByTexelColor[texelStr] = tileDef;

        return tileDef;
    }
}
