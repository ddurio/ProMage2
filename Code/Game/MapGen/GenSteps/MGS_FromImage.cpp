#include "Game/MapGen/GenSteps/MGS_FromImage.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/TileDef.hpp"


MGS_FromImage::MGS_FromImage( const XMLElement& element ) :
    MapGenStep(element) {
    std::string imageFilePath = ParseXMLAttribute( element, "imageFilePath", "" );
    GUARANTEE_OR_DIE( imageFilePath != "", "ERROR: XML Attribute 'imageFilePath' missing for MapGenStep_FromImage" );
    m_image = new Image( imageFilePath );

    m_alignX       = ParseXMLAttribute( element, "alignX", m_alignX );
    m_alignY       = ParseXMLAttribute( element, "alignY", m_alignY );
    m_numRotations = ParseXMLAttribute( element, "numRotations", m_numRotations );
}


MGS_FromImage::~MGS_FromImage() {
    CLEAR_POINTER( m_image );
}


// PRIVATE ---------------------------------------------
std::map< std::string, const TileDef* > MGS_FromImage::s_tileDefsByTexelColor;


void MGS_FromImage::RunOnce( Map& theMap ) const {
    IntVec2 imageDimensions = m_image->GetDimensions();
    IntVec2 mapDimensions = theMap.GetMapDimensions();

    IntVec2 imageOffset = GetImageOffset( theMap );

    int rotations = m_mapRNG->GetRandomIntInRange( m_numRotations );
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

            if( m_mapRNG->PercentChance( chanceToExist ) ) {
                int tileIndex = mapY * mapDimensions.x + mapX;
                Tile& tile = GetTile( theMap, tileIndex );
                tile.SetTileType( tileDef );
            }
        }
    }
}


bool MGS_FromImage::IsAlignmentValid( Map& theMap, const AABB2& imageBounds ) const {
    int minX = (int)imageBounds.mins.x;
    int maxX = (int)imageBounds.maxs.x;
    int minY = (int)imageBounds.mins.y;
    int maxY = (int)imageBounds.maxs.y;

    for( int tileY = minY; tileY <= maxY; tileY++ ) {
        for( int tileX = minX; tileX <= maxX; tileX++ ) {
            const Tile* tile = nullptr;

            if( theMap.GetTileIfValid( tile, IntVec2( tileX, tileY ) ) ) {
                if( !IsTileValid( theMap, *tile ) ) {
                    return false;
                }
            }
        }
    }

    return true;
}


IntVec2 MGS_FromImage::GetImageOffset( Map& theMap ) const {
    IntVec2 imageDimensions = m_image->GetDimensions();
    IntVec2 mapDimensions = theMap.GetMapDimensions();

    AABB2 imageBounds = AABB2::ZEROTOONE;
    int numAttempts = 0;

    do {
        if( numAttempts > 1000 ) {
            return IntVec2::NEGONE;
        }

        float alignX = m_mapRNG->GetRandomFloatInRange( m_alignX );
        float alignY = m_mapRNG->GetRandomFloatInRange( m_alignY );

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