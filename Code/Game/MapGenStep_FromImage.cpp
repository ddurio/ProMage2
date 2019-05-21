#include "Game/MapGenStep_FromImage.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDef.hpp"

#include "math.h"


MapGenStep_FromImage::MapGenStep_FromImage( const XMLElement& element ) :
    MapGenStep(element) {
    std::string imageFilePath = ParseXMLAttribute( element, "imageFilePath", "" );
    GUARANTEE_OR_DIE( imageFilePath != "", "ERROR: XML Attribute 'imageFilePath' missing for MapGenStep_FromImage" );
    m_image = new Image( imageFilePath );

    m_alignX       = ParseXMLAttribute( element, "alignX", m_alignX );
    m_alignY       = ParseXMLAttribute( element, "alignY", m_alignY );
    m_numRotations = ParseXMLAttribute( element, "numRotations", m_numRotations );
}


MapGenStep_FromImage::~MapGenStep_FromImage() {
    delete m_image;
    m_image = nullptr;
}


void MapGenStep_FromImage::RunOnce( Map& map ) const {
    IntVec2 imageDimensions = m_image->GetDimensions();
    IntVec2 mapDimensions = map.GetMapDimensions();

    float alignX = g_RNG->GetRandomFloatInRange( m_alignX );
    float alignY = g_RNG->GetRandomFloatInRange( m_alignY );
    Vec2 alignment = Vec2( alignX, alignY );

    AABB2 mapBounds = AABB2( Vec2::ZERO, Vec2( (float)mapDimensions.x, (float)mapDimensions.y ) );
    AABB2 imageBounds = mapBounds.GetBoxWithin( Vec2( (float)imageDimensions.x, (float)imageDimensions.y ), alignment );

    int mapOffsetX = (int)imageBounds.mins.x;
    int mapOffsetY = (int)imageBounds.mins.y;

    int rotations = g_RNG->GetRandomIntInRange( m_numRotations );
    DebuggerPrintf( Stringf( "NumRotations=%d\n", rotations ).c_str() );
    Image* rotatedImage = new Image( *m_image );
    m_image->GetRotated( rotations, *rotatedImage );

    for( int texelX = 0, mapX = mapOffsetX; texelX < imageDimensions.x && mapX < mapDimensions.x; texelX++, mapX++ ) {
        for( int texelY = 0, mapY = mapOffsetY; texelY < imageDimensions.y && mapY < mapDimensions.y; texelY++, mapY++ ) {
            Rgba texelColor = rotatedImage->GetTexelColor( texelX, texelY );
            float chanceToExist = texelColor.a;

            if( chanceToExist == 0.f ) {
                continue;
            }

            texelColor.a = 1.f; // color match will always have alpha set to one
            const TileDef* tileDef = TileDef::GetTileDefFromTexelColor( texelColor );

            GUARANTEE_OR_DIE( tileDef != nullptr, Stringf( "ERROR: TileDef not found with requested texelColor %s", texelColor.GetAsString().c_str() ) );

            if( g_RNG->PercentChance( chanceToExist ) ) {
                int tileIndex = mapY * mapDimensions.x + mapX;
                Tile& tile = GetTile( map, tileIndex );
                tile.SetTileType( tileDef );
            }
        }
    }

    delete rotatedImage;
}
