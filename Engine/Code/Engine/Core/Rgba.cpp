#include "Engine/Core/Rgba.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Math/MathUtils.hpp"


const Rgba Rgba::BLACK       = Rgba( 0.f,   0.f,    0.f,    1.f );
const Rgba Rgba::BLUE        = Rgba( 02,    34,     221          ); // equates to #0222DD
const Rgba Rgba::CLEAR       = Rgba( 1.f,   1.f,    1.f,    0.f );
const Rgba Rgba::CLEAR_BLACK = Rgba( 0.f,   0.f,    0.f,    0.f );
const Rgba Rgba::CYAN        = Rgba( 0.f,   1.f,    1.f,    1.f );
const Rgba Rgba::FLAT        = Rgba( 0.5f,  0.5f,   1.f,    1.f );
const Rgba Rgba::GRAY        = Rgba( 0.2f,  0.2f,   0.2f,   1.f );
const Rgba Rgba::GREEN       = Rgba( 0.f,   0.75f,  0.f,    1.f );
const Rgba Rgba::MAGENTA     = Rgba( 1.f,   0.f,    1.f,    1.f );
const Rgba Rgba::ORANGE      = Rgba( 0.9f,  0.6f,   0.1f,   1.f );
const Rgba Rgba::PURPLE      = Rgba( 0.75f, 0.09f,  0.88f,  1.f );
const Rgba Rgba::RED         = Rgba( 1.f,   0.f,    0.f,    1.f );
const Rgba Rgba::WHITE       = Rgba( 1.f,   1.f,    1.f,    1.f );
const Rgba Rgba::YELLOW      = Rgba( 1.f,   1.f,    0.f,    1.f );

const Rgba Rgba::ORGANIC_BLUE       = Rgba(  22,   157,    255  );
const Rgba Rgba::ORGANIC_BROWN      = Rgba( 133,    84,     57  );
const Rgba Rgba::ORGANIC_GRAY       = Rgba( 102,   102,    102  );
const Rgba Rgba::ORGANIC_GREEN      = Rgba(  45,   180,      0  );
const Rgba Rgba::ORGANIC_ORANGE     = Rgba( 255,   112,     11  );
const Rgba Rgba::ORGANIC_PURPLE     = Rgba( 190,    40,    255  );
const Rgba Rgba::ORGANIC_RED        = Rgba( 255,    44,     44  );
const Rgba Rgba::ORGANIC_YELLOW     = Rgba( 255,   214,      0  );

const Rgba Rgba::DIM_BLUE           = Rgba(   0,    53,     91  );
const Rgba Rgba::DIM_GREEN          = Rgba(  21,    70,      0  );
const Rgba Rgba::DIM_PURPLE         = Rgba(  76,     0,    109  );
const Rgba Rgba::DIM_RED            = Rgba( 114,     0,      0  );
const Rgba Rgba::DIM_YELLOW         = Rgba(  62,    52,      0  );
const Rgba Rgba::DIM_WHITE          = Rgba( 220,   220,    220  );


const std::map<std::string, const Rgba&> Rgba::s_colors = {
    { "BLACK",          Rgba::BLACK          },
    { "BLUE",           Rgba::BLUE           },
    { "CLEAR",          Rgba::CLEAR          },
    { "CLEAR_BLACK",    Rgba::CLEAR_BLACK    },
    { "CYAN",           Rgba::CYAN           },
    { "FLAT",           Rgba::FLAT           },
    { "GRAY",           Rgba::GRAY           },
    { "GREEN",          Rgba::GREEN          },
    { "MAGENTA",        Rgba::MAGENTA        },
    { "PURPLE",         Rgba::PURPLE         },
    { "ORANGE",         Rgba::ORANGE         },
    { "RED",            Rgba::RED            },
    { "WHITE",          Rgba::WHITE          },
    { "YELLOW",         Rgba::YELLOW         },
    { "ORGANIC_BLUE",   Rgba::ORGANIC_BLUE   },
    { "ORGANIC_BROWN",  Rgba::ORGANIC_BROWN  },
    { "ORGANIC_GRAY",   Rgba::ORGANIC_GRAY   },
    { "ORGANIC_GREEN",  Rgba::ORGANIC_GREEN  },
    { "ORGANIC_ORANGE", Rgba::ORGANIC_ORANGE },
    { "ORGANIC_PURPLE", Rgba::ORGANIC_PURPLE },
    { "ORGANIC_RED",    Rgba::ORGANIC_RED    },
    { "ORGANIC_YELLOW", Rgba::ORGANIC_YELLOW },
    { "DIM_BLUE",       Rgba::DIM_BLUE       },
    { "DIM_GREEN",      Rgba::DIM_GREEN      },
    { "DIM_PURPLE",     Rgba::DIM_PURPLE     },
    { "DIM_RED",        Rgba::DIM_RED        },
    { "DIM_YELLOW",     Rgba::DIM_YELLOW     },
};


Rgba::Rgba() {

}


Rgba::Rgba( float initialR, float initialG, float initialB, float initialA/*=1.f */ ) :
	r( initialR ),
	g( initialG ),
	b( initialB ),
	a( initialA ) {
}


Rgba::Rgba( const std::string& initialValues ) {
    SetFromText( initialValues );
}


Rgba::Rgba( int initialR, int initialG, int initialB, int initialA /*= 255 */ ) {
    SetFromBytes( (float)initialR, (float)initialG, (float)initialB, (float)initialA );
}


bool Rgba::operator==( const Rgba& compare ) const {
    return (compare.r == r && compare.g == g && compare.b == b && compare.a == a);
}


bool Rgba::operator!=( const Rgba& compare ) const {
    return !operator==( compare );
}


const std::string Rgba::GetAsString() const {
    if( a == 1.f ) {
        return Stringf( "%.02f,%.02f,%.02f", r, g, b );
    } else {
        return Stringf( "%.02f,%.02f,%.02f,%.02f", r, g, b, a );
    }
}


void Rgba::GetAsArray( float floatArray[4] ) const {
    floatArray[0] = r;
    floatArray[1] = g;
    floatArray[2] = b;
    floatArray[3] = a;
}


void Rgba::GetAsBytes( unsigned char byateArray[4] ) const {
    float floatArray[4];
    GetAsArray( floatArray );

    float rByte = floatArray[0] * 255.f;
    float gByte = floatArray[1] * 255.f;
    float bByte = floatArray[2] * 255.f;
    float aByte = floatArray[3] * 255.f;

    byateArray[0] = (unsigned char)rByte;
    byateArray[1] = (unsigned char)gByte;
    byateArray[2] = (unsigned char)bByte;
    byateArray[3] = (unsigned char)aByte;
}


ImVec4 Rgba::GetAsImGui() const {
    ImVec4 color = ImVec4( r, g, b, a );
    return color;
}


void Rgba::SetFromText( const std::string& valuesAsText ) {
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ',' );
    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 1 || numValues == 3 || numValues == 4, Stringf( "Invalid initialization string (%s) for Rgba", valuesAsText.c_str() ) );

    // Try setting from color name first
    if( numValues == 1 ) {
        std::string valuesAsUpper = StringToUpper( valuesAsText );
        std::map<std::string, const Rgba&>::const_iterator colorIter = s_colors.find( valuesAsUpper );

        GUARANTEE_OR_DIE( colorIter != s_colors.end(), Stringf( "Color name (%s) not found for Rgba", valuesAsText.c_str() ) );

        *this = colorIter->second;
        return;
    }

    // Try setting from numerical values second
    r = (float)atof( splitString[0].c_str() );
    g = (float)atof( splitString[1].c_str() );
    b = (float)atof( splitString[2].c_str() );

    if( r > 1.f || g > 1.f || b > 1.f ) {
         r /= 255.f;
         g /= 255.f;
         b /= 255.f;
    }

    if( numValues == 3 ) {
        a = 1.0f;
    } else {
        a = (float)atof( splitString[3].c_str() );

        if( a > 1.f ) {
            a /= 255.f;
        }
    }
}


void Rgba::SetFromBytes( float redByte, float greenByte, float blueByte, float alphaByte /*= 255 */ ) {
    r = redByte   / 255.f;
    g = greenByte / 255.f;
    b = blueByte  / 255.f;
    a = alphaByte / 255.f;
}


Rgba Rgba::GetBlended( const Rgba& toColor, float blendFractionA ) const {
    /* // Rgba blend
    float newR = blendFractionA * r + (1 - blendFractionA) * toColor.r;
    float newG = blendFractionA * g + (1 - blendFractionA) * toColor.g;
    float newB = blendFractionA * b + (1 - blendFractionA) * toColor.b;
    float newA = blendFractionA * a + (1 - blendFractionA) * toColor.a;

    return Rgba( newR, newG, newB, newA );
    */

    // HSL Blend
    Rgba thisAsHSLA = GetHSLA();
    Rgba toColorAsHSLA = toColor.GetHSLA();

    float newH = blendFractionA * thisAsHSLA.r + (1 - blendFractionA) * toColorAsHSLA.r;
    float newS = blendFractionA * thisAsHSLA.g + (1 - blendFractionA) * toColorAsHSLA.g;
    float newL = blendFractionA * thisAsHSLA.b + (1 - blendFractionA) * toColorAsHSLA.b;
    float newA = blendFractionA * thisAsHSLA.a + (1 - blendFractionA) * toColorAsHSLA.a;

    Rgba blendedHSLA = Rgba( newH, newS, newL, newA );
    return blendedHSLA.GetRGBA();
}


// https://www.rapidtables.com/convert/color/rgb-to-hsl.html 
Rgba Rgba::GetHSLA() const {
    float maxC = Max( r, Max( g, b ) );
    float minC = Min( r, Min( g, b ) );
    float diffC = maxC - minC;
    float sumC = maxC + minC;

    float hue = 0.f;
    float saturation = 0.f;
    float lightness = sumC * 0.5f;
    float alpha = a;

    if( diffC == 0.f ) { // All equal
        hue = 0.f;
        saturation = 0.f;
    } else {
        if( r >= g && r >= b ) { // Red is largest
            hue = 60.f * fmodf( ((g - b) / diffC), 6.f );
        } else if( g >= r && g >= b ) { // Green is largest
            hue = 60.f * (((b - r) / diffC) + 2);
        } else if( b >= r && b >= g ) { // Blue is largest
            hue = 60.f * (((r - g) / diffC) + 4);
        }

        saturation = diffC / (1 - abs( 2.f * lightness - 1.f ));
    }

    // Always want positive angle
    while( hue < 0.f ) {
        hue += 360.f;
    }

    return Rgba( hue, saturation, lightness, alpha );
}


// https://www.rapidtables.com/convert/color/hsl-to-rgb.html
Rgba Rgba::GetRGBA() const {
    float hue = fmodf(r, 360.f);
    float saturation = g;
    float lightness = b;
    float alpha = a;

    float c = saturation * (1 - abs( 2.f * lightness - 1.f ));
    float x = c * (1.f - abs( fmodf( hue / 60.f, 2.f ) - 1.f ));
    float m = lightness - c * 0.5f;

    float newR = m;
    float newG = m;
    float newB = m;
    float newA = alpha;

    if( hue >= 0.f && hue < 60.f ) {
        newR += c;
        newG += x;
    } else if( hue >= 60.f && hue < 120.f ) {
        newR += x;
        newG += c;
    } else if( hue >= 120.f && hue < 180.f ) {
        newG += c;
        newB += x;
    } else if( hue >= 180.f && hue < 240.f ) {
        newG += x;
        newB += c;
    } else if( hue >= 240.f && hue < 300.f ) {
        newR += x;
        newB += c;
    } else if( hue >= 300.f && hue < 360.f ) {
        newR += c;
        newB += x;
    }

    return Rgba( newR, newG, newB, newA );
}
