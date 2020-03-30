#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "map"


struct ImVec4;


struct Rgba {
	public:
	float r = 1.f;
	float g = 1.f;
	float b = 1.f;
	float a = 1.f;

	Rgba();
	explicit Rgba( float r, float g, float b, float a = 1.f );
    explicit Rgba( int initialR, int initialG, int initialB, int initialA = 255 );
    explicit Rgba( const std::string& initialValues );

	bool operator==( const Rgba& compare ) const;				// vec2 == vec2
	bool operator!=( const Rgba& compare ) const;				// vec2 != vec2

    const std::string GetAsString() const;
    void GetAsArray( float floatArray[4] ) const;
    void GetAsBytes( unsigned char byateArray[4] ) const;
    ImVec4 GetAsImGui() const;

    void SetFromText( const std::string& valuesAsText );
    void SetFromBytes( float redByte, float greenByte, float blueByte, float alphaByte = 255.f );

    Rgba GetBlended( const Rgba& toColor, float blendFractionA ) const;
    Rgba GetHSLA() const;
    Rgba GetRGBA() const;

    static const Rgba BLACK;
    static const Rgba BLUE;
    static const Rgba CLEAR;
    static const Rgba CLEAR_BLACK;
    static const Rgba CYAN;
    static const Rgba FLAT; // Used for normal maps
    static const Rgba GRAY;
    static const Rgba GREEN;
    static const Rgba MAGENTA;
    static const Rgba ORANGE;
    static const Rgba PURPLE;
    static const Rgba RED;
    static const Rgba WHITE;
    static const Rgba YELLOW;

    static const Rgba ORGANIC_BLUE;
    static const Rgba ORGANIC_BROWN;
    static const Rgba ORGANIC_GRAY;
    static const Rgba ORGANIC_GREEN;
    static const Rgba ORGANIC_ORANGE;
    static const Rgba ORGANIC_PURPLE;
    static const Rgba ORGANIC_RED;
    static const Rgba ORGANIC_YELLOW;

    static const Rgba DIM_BLUE;
    static const Rgba DIM_GREEN;
    static const Rgba DIM_PURPLE;
    static const Rgba DIM_RED;
    static const Rgba DIM_YELLOW;
    static const Rgba DIM_WHITE;

    static const std::map<std::string, const Rgba&> s_colors;
};
