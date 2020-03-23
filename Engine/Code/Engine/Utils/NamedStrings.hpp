#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Utils/XMLUtils.hpp"

#include "map"


struct FloatRange;
struct IntRange;
struct IntVec2;
struct Rgba;
struct Vec2;
struct Vec3;

class NamedStrings {
    public:
    NamedStrings();
    explicit NamedStrings( const XMLElement& element );
    ~NamedStrings() {};

    void SetFromXMLElementAttributes( const XMLElement& element );
    void SaveToXMLElementAttributes( XMLElement& element, tinyxml2::XMLDocument* document = nullptr ) const;

    void SetValue( const std::string& key, const std::string& value );
    void ClearValue( const std::string& key );

    bool GetValue( const std::string& key, bool defaultValue ) const;
    int GetValue( const std::string& key, int defaultValue ) const;
    float GetValue( const std::string& key, float defaultValue ) const;
    std::string GetValue( const std::string& key, std::string defaultValue ) const;
    std::string GetValue( const std::string& key, const char* defaultValue ) const;
    Rgba GetValue( const std::string& key, Rgba defaultValue ) const;
    Vec2 GetValue( const std::string& key, Vec2 defaultValue ) const;
    Vec3 GetValue( const std::string& key, Vec3 defaultValue ) const;
    IntVec2 GetValue( const std::string& key, IntVec2 defaultValue ) const;
    IntRange GetValue( const std::string& key, IntRange defaultValue ) const;
    FloatRange GetValue( const std::string& key, FloatRange defaultValue ) const;
    DevConsoleChannel GetValue( const std::string& key, DevConsoleChannel defaultValue ) const;

    std::string& GetReference( const std::string& key );

    std::string GetAsString() const;

    bool IsNameSet( const std::string& key ) const;


    private:
    std::map<std::string, std::string> m_keyValuePairs;
};
