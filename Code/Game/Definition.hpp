#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/XMLUtils.hpp"


class BaseDefinition {
    protected:
    std::string m_defType = "";
    NamedProperties m_properties;
};


template< typename T >
class Definition : public BaseDefinition {
    friend class EngineCommon;

    public:
    static void Initialize( const std::string& filePath, const char* tagName );
    static void DestroyDefs();
    static const Definition<T>* GetDefinition( std::string defType );

    void Define( T& theObject ) const;
    const std::string& GetDefintionType() const;

    template< typename PropT >
    PropT GetProperty( const std::string propName, const PropT& defaultValue ) const;

    protected:
    explicit Definition<T>( const XMLElement& element );

    static std::map< std::string, Definition<T>*, StringCmpCaseI > s_definitions;
};


template< typename T >
std::map< std::string, Definition<T>*, StringCmpCaseI > Definition<T>::s_definitions;


template< typename T >
void Definition<T>::Initialize( const std::string& filePath, const char* tagName ) {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( filePath, document );
    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        new Definition<T>( *element ); // Upon construction, adds self to static registry
    }
}


template< typename T >
void Definition<T>::DestroyDefs() {
    EngineCommon::ClearMap( s_definitions );
}


template< typename T >
const Definition<T>* Definition<T>::GetDefinition( std::string defType ) {
    //defType = StringToLower( defType );
     auto defIter = s_definitions.find( defType );

    if( defIter != s_definitions.end() ) {
        return defIter->second;
    } else {
        return nullptr;
    }
}


template< typename T >
const std::string& Definition<T>::GetDefintionType() const {
    return m_defType;
}


template< typename T >
template< typename PropT >
PropT Definition<T>::GetProperty( const std::string propName, const PropT& defaultValue ) const {
    return m_properties.GetValue( propName, defaultValue );
}


// Custom Implementations Required!
template< typename T >
Definition<T>::Definition( const XMLElement& element ) {
    ERROR_AND_DIE( "(Definition) Unrecognized Definition type. Missing constructor template specialization." );
}


template< typename T >
void Definition<T>::Define( T& theObject ) const {
    ERROR_AND_DIE( "(Definition) Unrecognized Definition type. Missing Define template specialization." );
}
