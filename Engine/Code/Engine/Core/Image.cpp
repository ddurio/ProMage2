#include "Engine/Core/Image.hpp"

#pragma warning( push )
#pragma warning( disable: 4100 ) // Unreferenced formal parameter
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image_write.h"
#pragma warning( pop )


Image::Image( const std::string& imageFilePath ) :
    m_imageFilePath(imageFilePath) {
    if( m_imageFilePath == "" ) {
        CreateDefaultImage();
        return;
    } else {
        std::map<std::string, const Rgba&>::const_iterator colorIter = Rgba::s_colors.find( StringToUpper( imageFilePath ) );

        if( colorIter != Rgba::s_colors.end() ) {
            CreateDefaultImage( colorIter->second );
            return;
        }
    }

    int imageTexelSizeX = 0; // Filled in for us to indicate image width
    int imageTexelSizeY = 0; // Filled in for us to indicate image height
    int numComponents = 0; // Filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
    int numComponentsRequested = 4; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

    //stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
    m_rawData = stbi_load( m_imageFilePath.c_str(), &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

    // Check if the load was successful
    GUARANTEE_OR_DIE( m_rawData, Stringf( "Failed to load image \"%s\"", imageFilePath.c_str() ) );
    GUARANTEE_OR_DIE( (numComponents == 3 || numComponents == 4) && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath.c_str(), numComponents, imageTexelSizeX, imageTexelSizeY ) );

    m_isRawDataValid = true;
    m_numComponents = numComponents;
    m_dimensions = IntVec2( imageTexelSizeX, imageTexelSizeY );

    int numTexels = m_dimensions.x * m_dimensions.y;
    m_texels.resize( numTexels );

    for( int texelIndex = 0; texelIndex < numTexels; texelIndex++ ) {
        // Get Byte Indices
        int redIndex    = texelIndex * numComponents;
        int greenIndex  = redIndex + 1;
        int blueIndex   = redIndex + 2;
        int alphaIndex  = redIndex + 3;

        // Get Byte Data
        float redByte   = m_rawData[redIndex];
        float greenByte = m_rawData[greenIndex];
        float blueByte  = m_rawData[blueIndex];
        float alphaByte = 255.f;

        if( numComponents == 4 ) {
            alphaByte = m_rawData[alphaIndex];
        }

        // Set Rgba Values
        m_texels[texelIndex].SetFromBytes( redByte, greenByte, blueByte, alphaByte );
    }
}


Image::Image( const Rgba& imageColor, const IntVec2& dimensions /*= IntVec2::ONE */ ) {
    // Set file path
    if( dimensions == IntVec2::ONE ) {
        m_imageFilePath = imageColor.GetAsString();
    } else {
        m_imageFilePath = Stringf( "%s_%s", imageColor.GetAsString().c_str(), dimensions.GetAsString().c_str() );
    }

    CreateDefaultImage( imageColor, dimensions );
}


Image::Image( const Image& copyFrom ) :
    m_imageFilePath( copyFrom.m_imageFilePath ),
    m_dimensions( copyFrom.m_dimensions ),
    m_texels( copyFrom.m_texels ),
    m_rawData( nullptr ),
    m_isRawDataValid( false ),
    m_numComponents( copyFrom.m_numComponents ) {
}


Image::~Image() {
    if( m_rawData != nullptr ) {
        stbi_image_free( m_rawData );
    }
}


IntVec2 Image::GetDimensions() const {
    return m_dimensions;
}


Rgba Image::GetTexelColor( int texelX, int texelY ) const {
    int texelIndex = texelY * m_dimensions.x + texelX;
    return m_texels[texelIndex];
}


Rgba Image::GetTexelColor( const IntVec2& texelCoords ) const {
    return GetTexelColor( texelCoords.x, texelCoords.y );
}


std::string Image::GetImageFilePath() const {
    return m_imageFilePath;
}


unsigned char* Image::GetRawData() const {
    if( !m_isRawDataValid ) {
        // recreate rawData?
        return nullptr;
    }

    return m_rawData;
}


int Image::GetNumComponents() const {
    return m_numComponents;
}


void Image::GetRotated( int numRotations, Image& out_rotatedImage ) const {
    // Free whatever raw data you used to have
    if( out_rotatedImage.m_isRawDataValid ) {
        stbi_image_free( out_rotatedImage.m_rawData );
    }

    // Copy everything EXCEPT raw data pointer (can't allow multiple people to delete it)
    out_rotatedImage = *this;
    out_rotatedImage.m_rawData = nullptr;
    out_rotatedImage.m_isRawDataValid = false;

    for( int rotationIndex = 0; rotationIndex < numRotations; rotationIndex++ ) {
        int numTexels = m_dimensions.x * m_dimensions.y;
        std::vector<Rgba> texelCopies = out_rotatedImage.m_texels;

        for( int texelIndex = 0; texelIndex < numTexels; texelIndex++ ) {
            int texelX = texelIndex % m_dimensions.x;
            int texelY = texelIndex / m_dimensions.x;

            int newTexelX = m_dimensions.y - 1 - texelY;
            int newTexeY = texelX;

            int newTexelIndex = newTexeY * m_dimensions.x + newTexelX;

            out_rotatedImage.m_texels[newTexelIndex] = texelCopies[texelIndex];
        }
    }
}


void Image::SetTexelColor( int texelX, int texelY, const Rgba& texelColor ) {
    int texelIndex = texelY * m_dimensions.x + texelX;
    m_texels[texelIndex] = texelColor;

    int charIndex = texelIndex * 4;
    unsigned char* dataHead = m_rawData + charIndex;
    texelColor.GetAsBytes( dataHead );
}


void Image::SetTexelColor( const IntVec2& texelCoords, const Rgba& texelColor ) {
    SetTexelColor( texelCoords.x, texelCoords.y, texelColor );
}


bool Image::WriteToPNG( const std::filesystem::path& pathToWrite ) const {
    int writeReturn = stbi_write_png( pathToWrite.string().c_str(), m_dimensions.x, m_dimensions.y, 4, m_rawData, 4 * m_dimensions.x );
    return (writeReturn != 0);
}


void Image::CreateDefaultImage( const Rgba& defaultColor /*= Rgba::WHITE*/, const IntVec2& dimensions /*= IntVec2::ONE */ ) {
    // Set raw data
    int numTexels = dimensions.x * dimensions.y;
    int numChars = numTexels * 4;

    m_texels.resize( numTexels, defaultColor );
    m_rawData = new unsigned char[numChars];

    for( int texelIndex = 0; texelIndex < numTexels; texelIndex++ ) {
        int charIndex = texelIndex * 4;
        unsigned char* dataHead = m_rawData + charIndex;

        defaultColor.GetAsBytes( dataHead );
    }

    // Set metadata
    m_isRawDataValid = true;
    m_numComponents = 4;
    m_dimensions = dimensions;

    // Set file path

}

