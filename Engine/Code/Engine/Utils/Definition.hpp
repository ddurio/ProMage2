#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Utils/XMLUtils.hpp"


template< typename T, typename TDef >
class Definition {
    friend class EngineCommon;

    public:
    static void LoadFromFile( const std::string& filePath, const char* tagName );
    static void SaveToFile( const std::string& filePath, const char* tagName );
    static void DestroyDefs();
    static const TDef* GetDefinition( std::string defType );
    static Strings GetAllDefinitionTypes();

    virtual void DefineObject( T& theObject ) const = 0;
    const std::string& GetDefintionType() const;

    protected:
    static std::map< std::string, TDef*, StringCmpCaseI > s_definitions;
    static std::string s_defClass;

    std::string m_defType = "";


    static void AddDefinition( TDef* defToAdd );
};


template< typename T, typename TDef >
std::map< std::string, TDef*, StringCmpCaseI > Definition<T, TDef>::s_definitions;


template< typename T, typename TDef >
std::string Definition<T, TDef>::s_defClass = "Definition";


template< typename T, typename TDef >
void Definition<T, TDef>::LoadFromFile( const std::string& filePath, const char* tagName ) {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( filePath, document );
    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        // Construct it
        TDef* def = new TDef( *element );
        AddDefinition( def );
    }
}


template< typename T, typename TDef >
void Definition<T, TDef>::SaveToFile( const std::string& filePath, const char* tagName ) {
    XmlDocument document = new XmlDocument();
    XMLElement* root = document.NewElement( Stringf( "%ss", s_defClass.c_str() ).c_str() );
    document.InsertFirstChild( root );

    typename std::map< std::string, TDef*, StringCmpCaseI >::const_iterator defIter = s_definitions.begin();

    while( defIter != s_definitions.end() ) {
        XMLElement* defEle = document.NewElement( tagName );
        root->InsertEndChild( defEle );
        defIter->second->SaveToXml( document, *defEle );

        defIter++;
    }

    tinyxml2::XMLError saveSuccess = document.SaveFile( filePath.c_str() );

    if( saveSuccess != tinyxml2::XML_SUCCESS ) {
        ERROR_RECOVERABLE( Stringf( "Failed to save file (%s)", filePath.c_str() ) );
    }
}


template< typename T, typename TDef >
void Definition<T, TDef>::DestroyDefs() {
    typename std::map<std::string, TDef*, StringCmpCaseI>::iterator mapIter = s_definitions.begin();

    for( mapIter; mapIter != s_definitions.end(); mapIter++ ) {
        CLEAR_POINTER( mapIter->second );
    }

    s_definitions.clear();
}


template< typename T, typename TDef >
const TDef* Definition<T, TDef>::GetDefinition( std::string defType ) {
     auto defIter = s_definitions.find( defType );

    if( defIter != s_definitions.end() ) {
        return defIter->second;
    } else {
        return nullptr;
    }
}


template< typename T, typename TDef >
const std::string& Definition<T, TDef>::GetDefintionType() const {
    return m_defType;
}


template< typename T, typename TDef >
Strings Definition<T, TDef>::GetAllDefinitionTypes() {
    Strings allDefTypes;
    typename std::map< std::string, TDef*, StringCmpCaseI >::const_iterator defIter = s_definitions.begin();

    for( defIter; defIter != s_definitions.end(); defIter++ ) {
        allDefTypes.push_back( defIter->first );
    }

    return allDefTypes;
}


template< typename T, typename TDef >
void Definition<T, TDef>::AddDefinition( TDef* defToAdd ) {
    // Print to DevConsole
    const char* classStr = s_defClass.c_str();
    std::string msg = Stringf( "(%s) Loaded new %s (%s)", classStr, classStr, defToAdd->m_defType.c_str() );
    g_theDevConsole->PrintString( msg );

    // Add to map
    s_definitions[defToAdd->m_defType] = defToAdd;
}
