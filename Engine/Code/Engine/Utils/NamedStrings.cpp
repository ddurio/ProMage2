#include "Engine/Utils/NamedStrings.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"



NamedStrings::NamedStrings() {
}


NamedStrings::NamedStrings( const XMLElement& element ) {
    SetFromXMLElementAttributes( element );
}


void NamedStrings::SetFromXMLElementAttributes( const XMLElement& element ) {
    for( const XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next() ) {
        SetValue( attribute->Name(), attribute->Value() );
    }

    if( GetValue( "version", 1 ) == 2 ) { // Version 2 says parse from child elements too
        const XMLElement* childEle = element.FirstChildElement();

        for( childEle; childEle != nullptr; childEle = childEle->NextSiblingElement() ) {
            std::string value = childEle->Attribute( "value" );
            SetValue( childEle->Name(), value );
        }
    }
}


void NamedStrings::SaveToXMLElementAttributes( XMLElement& element, tinyxml2::XMLDocument* document /*= nullptr */ ) const {
    int version = GetValue( "version", 1 );

    if( version == 1 ) {
        std::map< std::string, std::string >::const_iterator keyValueIter = m_keyValuePairs.begin();

        for( keyValueIter; keyValueIter != m_keyValuePairs.end(); keyValueIter++ ) {
            element.SetAttribute( keyValueIter->first.c_str(), keyValueIter->second.c_str() );
        }
    } else {
        std::map< std::string, std::string >::const_iterator keyValueIter = m_keyValuePairs.begin();

        for( keyValueIter; keyValueIter != m_keyValuePairs.end(); keyValueIter++ ) {
            std::string key = keyValueIter->first;
            std::string value = keyValueIter->second;

            if( key == "name" || key == "version" ) {
                element.SetAttribute( key.c_str(), value.c_str() );
            } else {
                if( document == nullptr ) {
                    ERROR_RECOVERABLE( "(NamedStrings) WARNING -- Cannot save version 2 to file without XML Document" );
                    break;
                }

                XMLElement* childEle = document->NewElement( key.c_str() );
                
                if( childEle != nullptr ) {
                    childEle->SetAttribute( "value", value.c_str() );
                    element.InsertEndChild( childEle );
                }
            }
        }
    }
}


void NamedStrings::SetValue( const std::string& key, const std::string& value ) {
    std::map<std::string, std::string>::iterator valueIter = m_keyValuePairs.find( key );

    if( valueIter == m_keyValuePairs.end() ) {
        m_keyValuePairs.insert( { key, value } );
    } else {
        m_keyValuePairs[key] = value;
    }
}


void NamedStrings::ClearValue( const std::string& key ) {
    m_keyValuePairs.erase( key );
}


bool NamedStrings::GetValue( const std::string& key, bool defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return StringToBool( valueIter->second );
    }
}


int NamedStrings::GetValue( const std::string& key, int defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return atoi( valueIter->second.c_str() );
    }
}


float NamedStrings::GetValue( const std::string& key, float defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return (float)atof( valueIter->second.c_str() );
    }
}


std::string NamedStrings::GetValue( const std::string& key, std::string defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return valueIter->second;
    }
}


std::string NamedStrings::GetValue( const std::string& key, const char* defaultValue ) const {
    return GetValue( key, std::string( defaultValue ) );
}


Rgba NamedStrings::GetValue( const std::string& key, Rgba defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return Rgba( valueIter->second.c_str() );
    }
}


Vec2 NamedStrings::GetValue( const std::string& key, Vec2 defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return Vec2( valueIter->second.c_str() );
    }
}


Vec3 NamedStrings::GetValue( const std::string& key, Vec3 defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return Vec3( valueIter->second.c_str() );
    }
}


IntVec2 NamedStrings::GetValue( const std::string& key, IntVec2 defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return IntVec2( valueIter->second.c_str() );
    }

}


IntRange NamedStrings::GetValue( const std::string& key, IntRange defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return IntRange( valueIter->second.c_str() );
    }

}


FloatRange NamedStrings::GetValue( const std::string& key, FloatRange defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return FloatRange( valueIter->second.c_str() );
    }
}


DevConsoleChannel NamedStrings::GetValue( const std::string& key, DevConsoleChannel defaultValue ) const {
    std::map<std::string, std::string>::const_iterator valueIter = m_keyValuePairs.find( key );
    
    if( valueIter == m_keyValuePairs.end() ) {
        return defaultValue;
    } else {
        return g_theDevConsole->GetChannelFromString( valueIter->second );
    }
}


std::string& NamedStrings::GetReference( const std::string& key ) {
    return m_keyValuePairs[key];
}


std::string NamedStrings::GetAsString() const {
    std::string argsAsString = "";
    std::map<std::string, std::string>::const_iterator pairsIter = m_keyValuePairs.begin();

    while( pairsIter != m_keyValuePairs.end() ) {
        std::string key = pairsIter->first;
        std::string value = pairsIter->second;
        argsAsString = Stringf( "%s %s=%s", argsAsString.c_str(), key.c_str(), value.c_str() );
        pairsIter++;
    }

    if( argsAsString == "" ) {
        return argsAsString; // No arguments, just return empty string
    } else {
        return argsAsString.substr( 1 ); // Remove leading space
    }
}


bool NamedStrings::IsNameSet( const std::string& key ) const {
    std::map< std::string, std::string >::const_iterator propIter = m_keyValuePairs.find( key );
    return (propIter != m_keyValuePairs.end());
}
