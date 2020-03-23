#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/SpriteDef.hpp"
#include "Engine/Utils/XMLUtils.hpp"


class RenderContext;

struct IntVec2;


class SpriteSheet {
    public:
    explicit SpriteSheet( const XMLElement& element );
    explicit SpriteSheet( const std::string& texturePath, const IntVec2& gridLayout, const std::string& sheetName = "" );

    static void Initialize( const std::string& filePath, const char* tagName, RenderContext* renderer = nullptr );
    static const SpriteSheet& GetSpriteSheet( const std::string& name );
    static Strings GetAllTexturePaths();

    std::string GetTexturePath() const;
    const SpriteDef& GetSpriteDef( int spriteIndex ) const;
    const SpriteDef& GetSpriteDef( IntVec2 spriteCoords ) const;

    int GetSpriteIndexFromCoords( IntVec2 spriteCoords ) const;

    private:
    static std::map< std::string, SpriteSheet* > s_sheets;

    std::string m_texturePath = "";
    std::vector<SpriteDef> m_spriteDefs;
    int m_gridWidth = 0;


    void SetFromTextureAndGrid( const std::string& texturePath, const IntVec2& gridLayout );
};