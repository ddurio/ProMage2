#include "Engine/Renderer/SpriteSheet.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDef.hpp"


std::map< std::string, SpriteSheet* > SpriteSheet::s_sheets;


SpriteSheet::SpriteSheet( const XMLElement& element ) {
    std::string name = ParseXMLAttribute( element, "name", "UNKNOWN" );
    GUARANTEE_OR_DIE( name != "UNKNOWN", "(SpriteSheet) Missing required XML attribute name." );

    m_texturePath = ParseXMLAttribute( element, "texturePath", m_texturePath );
    GUARANTEE_OR_DIE( m_texturePath != "", "(SpriteSheet) Missing required XML attribute texturePath." );

    IntVec2 gridLayout = ParseXMLAttribute( element, "gridLayout", IntVec2::ZERO );

    if( gridLayout.x <= 0 || gridLayout.y <= 0 ) {
        std::string msg = Stringf( "(SpriteSheet) Invalid or missing XML attribute gridLayout (%s).", gridLayout.GetAsString().c_str() );
        ERROR_AND_DIE( msg );
    }

    SetFromTextureAndGrid( m_texturePath, gridLayout );

    g_theDevConsole->PrintString( Stringf( "(SpriteSheet) Loaded new SpriteSheeet (%s)", name.c_str() ) );
    name = StringToLower( name );
    s_sheets[name] = this;
}


SpriteSheet::SpriteSheet( const std::string& texturePath, const IntVec2& gridLayout, const std::string& sheetName /*= "" */ ) {
    SetFromTextureAndGrid( texturePath, gridLayout );

    if( sheetName != "" ) {
        g_theDevConsole->PrintString( Stringf( "(SpriteSheet) Loaded new SpriteSheeet (%s)", sheetName.c_str() ) );
        std::string lowerName = StringToLower( sheetName );
        s_sheets[lowerName] = this;
    }
}


void SpriteSheet::Initialize( const std::string& filePath, const char* tagName, RenderContext* renderer /*= nullptr */ ) {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( filePath, document );

    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        SpriteSheet* sheet = new SpriteSheet( *element ); // Upon construction, adds self to static registry

        if( renderer != nullptr ) {
            renderer->GetOrCreateTextureView2D( sheet->GetTexturePath() ); // Make sure the texture is loaded too
        }
    }
}


const SpriteSheet& SpriteSheet::GetSpriteSheet( const std::string& name ) {
    std::string lowerName = StringToLower( name );
    auto spriteSheetIter = s_sheets.find( lowerName );

    if( spriteSheetIter != s_sheets.end() ) {
        return *(spriteSheetIter->second);
    } else {
        ERROR_AND_DIE( Stringf( "(SpriteSheet) Unknown spriteSheet name (%s)", name.c_str() ) );
    }
}


Strings SpriteSheet::GetAllTexturePaths() {
    Strings allPaths;
    std::map< std::string, SpriteSheet* >::const_iterator sheetIter = s_sheets.begin();

    for( sheetIter; sheetIter != s_sheets.end(); sheetIter++ ) {
        const std::string& path = sheetIter->second->GetTexturePath();
        allPaths.push_back( path );
    }

    return allPaths;
}


std::string SpriteSheet::GetTexturePath() const {
    return m_texturePath;
}


const SpriteDef& SpriteSheet::GetSpriteDef( int spriteIndex ) const {
    return m_spriteDefs[spriteIndex];
}


const SpriteDef& SpriteSheet::GetSpriteDef( IntVec2 spriteCoords ) const {
    int spriteIndex = GetSpriteIndexFromCoords( spriteCoords );
    return GetSpriteDef( spriteIndex );
}


int SpriteSheet::GetSpriteIndexFromCoords( IntVec2 spriteCoords ) const {
    return (spriteCoords.y * m_gridWidth) + spriteCoords.x;
}


void SpriteSheet::SetFromTextureAndGrid( const std::string& texturePath, const IntVec2& gridLayout ) {
    m_texturePath = texturePath;
    m_gridWidth = gridLayout.x;

    float uStride = 1.f / (float)gridLayout.x;
    float vStride = 1.f / (float)gridLayout.y;

    const float offset = 0.0005f; // Avoiding bad "nearest" texel rounding

    for( int gridY = 0; gridY < gridLayout.y; gridY++ ) {
        for( int gridX = 0; gridX < gridLayout.x; gridX++ ) {
            float minU = uStride * (float)gridX;
            float maxU = minU + uStride;

            float maxV = vStride * (float)gridY;
            float minV = maxV + vStride;

            Vec2 minUV = Vec2( minU + offset, minV - offset );
            Vec2 maxUV = Vec2( maxU - offset, maxV + offset );

            m_spriteDefs.push_back( SpriteDef( minUV, maxUV, m_texturePath ) );
        }
    }
}
