#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "filesystem"


class Image {
    friend class Texture2D;

    public:
    Image() {};
    explicit Image( const std::string& imageFilePath );
    explicit Image( const Rgba& imageColor, const IntVec2& dimensions = IntVec2::ONE );
    explicit Image( const Image& copyFrom );
    ~Image();

    IntVec2 GetDimensions() const;
    Rgba GetTexelColor( int texelX, int texelY ) const;
    Rgba GetTexelColor( const IntVec2& texelCoords ) const;

    std::string GetImageFilePath() const;
    unsigned char* GetRawData() const;
    int GetNumComponents() const;
    void GetRotated( int numRotations, Image& out_rotatedImage ) const;

    void SetTexelColor( int texelX, int texelY, const Rgba& texelColor );
    void SetTexelColor( const IntVec2& texelCoords, const Rgba& texelColor );

    bool WriteToPNG( const std::filesystem::path& pathToWrite ) const;


    private:
    std::string m_imageFilePath = "";
    IntVec2 m_dimensions = IntVec2::ZERO;
    std::vector<Rgba> m_texels;
    unsigned char* m_rawData = nullptr;
    bool m_isRawDataValid = false;
    int m_numComponents = 0;

    void CreateDefaultImage( const Rgba& defaultColor = Rgba::WHITE, const IntVec2& dimensions = IntVec2::ONE );
};