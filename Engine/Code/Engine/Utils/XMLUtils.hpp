#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"


struct FloatRange;
struct IntVec2;
struct IntRange;
struct Rgba;
struct Vec2;
struct Vec3;

typedef tinyxml2::XMLAttribute XMLAttribute;
typedef tinyxml2::XMLDocument XmlDocument; // D3D conflicts with the preferred naming convention XMLDocument
typedef tinyxml2::XMLElement XMLElement;

// Any XML Convenience Functions
const XMLElement& ParseXMLRootElement( const std::string& fileName, XmlDocument& document );

int ParseXMLAttribute( const XMLElement& element, const char* attributeName, int defaultValue );
char ParseXMLAttribute( const XMLElement& element, const char* attributeName, char defaultValue );
bool ParseXMLAttribute( const XMLElement& element, const char* attributeName, bool defaultValue );
float ParseXMLAttribute( const XMLElement& element, const char* attributeName, float defaultValue );
Rgba ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Rgba& defaultValue );
Vec2 ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Vec2& defaultValue );
Vec3 ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Vec3& defaultValue );
IntVec2 ParseXMLAttribute( const XMLElement& element, const char* attributeName, const IntVec2& defaultValue );
IntRange ParseXMLAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange ParseXMLAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue );
std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue );
std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue );
