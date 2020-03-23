#include "Engine/Utils/NamedProperties.hpp"


NamedProperties::NamedProperties() {

}


NamedProperties::NamedProperties( const NamedProperties& copyFrom ) {
    std::map< std::string, BaseProperty*, StringCmpCaseI >::const_iterator pairsIter = copyFrom.m_keyValuePairs.begin();

    for( pairsIter; pairsIter != copyFrom.m_keyValuePairs.end(); pairsIter++ ) {
        m_keyValuePairs[pairsIter->first] = pairsIter->second->CopyToNewProperty();
    }
}


NamedProperties& NamedProperties::operator=( const NamedProperties& assignFrom ) {
    std::map< std::string, BaseProperty*, StringCmpCaseI >::const_iterator pairsIter = assignFrom.m_keyValuePairs.begin();

    for( pairsIter; pairsIter != assignFrom.m_keyValuePairs.end(); pairsIter++ ) {
        m_keyValuePairs[pairsIter->first] = pairsIter->second->CopyToNewProperty();
    }

    return *this;
}


std::string NamedProperties::GetAsString() const {
    std::string argsAsString = "";
    std::map<std::string, BaseProperty*, StringCmpCaseI>::const_iterator pairsIter = m_keyValuePairs.begin();

    while( pairsIter != m_keyValuePairs.end() ) {
        std::string key = pairsIter->first;
        std::string value = pairsIter->second->GetAsString();

        argsAsString = Stringf( "%s %s=%s", argsAsString.c_str(), key.c_str(), value.c_str() );
        pairsIter++;
    }

    if( argsAsString == "" ) {
        return argsAsString; // No arguments, just return empty string
    } else {
        return argsAsString.substr( 1 ); // Remove leading space
    }
}


Strings NamedProperties::GetNames() const {
    Strings propNames;
    std::map<std::string, BaseProperty*, StringCmpCaseI >::const_iterator pairIter = m_keyValuePairs.begin();

    for( pairIter; pairIter != m_keyValuePairs.end(); pairIter++ ) {
        propNames.push_back( pairIter->first );
    }

    return propNames;
}


bool NamedProperties::IsNameSet( const std::string& key ) const {
    std::map<std::string, BaseProperty*, StringCmpCaseI>::const_iterator propIter = m_keyValuePairs.find( key );
    return (propIter != m_keyValuePairs.end());
}


std::string NamedProperties::GetValue( const std::string& key, const char* defaultValue ) const {
    return GetValue<std::string>( key, defaultValue );
}


void NamedProperties::SetValue( const std::string& key, const char* value ) {
    SetValue<std::string>( key, value );
}


void NamedProperties::ClearValue( const std::string& key ) {
    m_keyValuePairs.erase( key );
}
