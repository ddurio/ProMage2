#include "Engine/Utils/XMLUtils.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"


const XMLElement& ParseXMLRootElement( const std::string& fileName, XmlDocument& document ) {
    tinyxml2::XMLError loadSuccess = document.LoadFile( fileName.c_str() );
    GUARANTEE_OR_DIE( loadSuccess == tinyxml2::XML_SUCCESS, Stringf("Failed to load XML file: %s", fileName.c_str() ) );

    tinyxml2::XMLNode* root = document.FirstChild();
    GUARANTEE_OR_DIE( root != nullptr, Stringf( "Poorly constructed XML file: %s", fileName.c_str() ) );

    return *root->ToElement();
}


int ParseXMLAttribute( const XMLElement& element, const char* attributeName, int defaultValue ) {
    return element.IntAttribute( attributeName, defaultValue );
}


char ParseXMLAttribute( const XMLElement& element, const char* attributeName, char defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? attributeValue.c_str()[0] : defaultValue);
}


bool ParseXMLAttribute( const XMLElement& element, const char* attributeName, bool defaultValue ) {
    return element.BoolAttribute( attributeName, defaultValue );
}


float ParseXMLAttribute( const XMLElement& element, const char* attributeName, float defaultValue ) {
    return element.FloatAttribute( attributeName, defaultValue );
}


Rgba ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Rgba& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? Rgba( attributeValue.c_str() ) : Rgba( defaultValue ));
}


Vec2 ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Vec2& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? Vec2( attributeValue.c_str() ) : Vec2( defaultValue ));
}


Vec3 ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Vec3& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? Vec3( attributeValue.c_str() ) : Vec3( defaultValue ));
}


IntVec2 ParseXMLAttribute( const XMLElement& element, const char* attributeName, const IntVec2& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? IntVec2( attributeValue.c_str() ) : IntVec2( defaultValue ));
}


IntRange ParseXMLAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? IntRange( attributeValue.c_str() ) : IntRange( defaultValue ));
}


FloatRange ParseXMLAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue ) {
    std::string attributeValue = ParseXMLAttribute( element, attributeName, std::string("") );

    return (attributeValue != "" ? FloatRange( attributeValue.c_str() ) : FloatRange( defaultValue ));
}


std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue ) {
    const char* attributeValue = element.Attribute( attributeName );

    return (attributeValue != nullptr ? std::string( attributeValue ) : std::string(defaultValue));
}


std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue ) {
    return ParseXMLAttribute( element, attributeName, std::string( defaultValue ) );
}
