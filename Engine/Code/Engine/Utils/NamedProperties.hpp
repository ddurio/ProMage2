#pragma once
#include "Engine/Core/EngineCommon.hpp"


class BaseProperty {
    public:
    virtual std::string GetAsString() const = 0;
    virtual BaseProperty* CopyToNewProperty() const = 0;
};

template <typename T>
class TypedProperty : public BaseProperty {
    public:
    TypedProperty( const T& value ) : m_value( value ) {};
    TypedProperty( const TypedProperty& copyFrom ) : m_value( copyFrom.m_value ) {};

    virtual std::string GetAsString() const override {
        return ::GetAsString( m_value );
    }

    virtual BaseProperty* CopyToNewProperty() const override {
        return new TypedProperty<T>( *this );
    }

    T m_value;
};


template <typename T>
inline std::string GetAsString( const T& value ) {
    if constexpr( std::is_enum_v<T> ) {
        return GetAsString( (int)value );
    } else {
        return value.GetAsString();
    }
}


template <typename T>
inline std::string GetAsString( const std::vector< T >& value ) {
    UNUSED( value );
    return "";
}


template <>
inline std::string GetAsString( const unsigned int& value ) {
    return std::to_string( value );
}


template <>
inline std::string GetAsString( const int& value ) {
    return std::to_string( value );
}


template <>
inline std::string GetAsString( const float& value ) {
    return std::to_string( value );
}


template <>
inline std::string GetAsString( const bool& value ) {
    return (value ? "true" : "false");
}


template <>
inline std::string GetAsString( const std::string& value ) {
    return value;
}


// Is this needed? not right now apparently
/*
inline std::string GetAsString( const char* value ) {
    return std::string( value );
}
*/


template <typename T>
inline std::string GetAsString( const T* value ) {
    UNUSED( value );
    //return value->GetAsString();
    return "";
}


template <typename T>
inline std::string GetAsString( T* value ) {
    UNUSED( value );
    return "";
}


template <>
inline std::string GetAsString( const void* const pointer ) {
    UNUSED( pointer );
    return "";
}


class NamedProperties {
    public:
    NamedProperties();
    NamedProperties( const NamedProperties& copyFrom );

    std::string GetAsString() const;
    Strings GetNames() const;
    bool IsNameSet( const std::string& key ) const;
    
    // Getters
    template <typename T>
    T GetValue( const std::string& key, const T& defaultValue ) const;

    template <typename T>
    T* GetValue( const std::string& key, T* defaultPointer ) const;

    template <typename T>
    const T* GetValue( const std::string& key, const T* defaultPointer ) const;

    std::string GetValue( const std::string& key, const char* defaultValue ) const;

    // Setters
    template <typename T>
    void SetValue( const std::string& key, const T& value );

    template <typename T>
    void SetValue( const std::string& key, const T* valuePointer );

    template <typename T>
    void SetValue( const std::string& key, T* valuePointer );

    void SetValue( const std::string& key, const char* value );

    void ClearValue( const std::string& key );


    private:
    std::map<std::string, BaseProperty*, StringCmpCaseI > m_keyValuePairs;
};


// Global String Functions
template <typename T>
inline T SetFromString( const char* str, const T& defaultValue ) {
    if constexpr( std::is_enum<T>::value ) {
        return (T)SetFromString( str, (int)defaultValue );
    } else {
        UNUSED( defaultValue );
        return T( std::string(str) );
    }
}


template <typename T>
inline std::vector<T> SetFromString( const char* str, const std::vector<T>& defaultValue ) {
    UNUSED( str );
    return defaultValue;
}


template <typename T>
inline T* SetFromString( const char* str, const T* defaultValue ) {
    UNUSED( str );
    UNUSED( defaultValue );
    return nullptr;
}


template <>
inline float SetFromString( const char* str, const float& defaultValue ) {
    float value = defaultValue;

    try {
        value = std::stof( str );
    } catch ( const std::out_of_range& ) {
        return defaultValue;
    } catch( const std::invalid_argument& ) {
        return defaultValue;
    }

    return value;
}


template <>
inline int SetFromString( const char* str, const int& defaultValue ) {
    int value = defaultValue;

    try {
        value = std::stoi( str );
    } catch ( ... ) {
        return defaultValue;
    }

    return value;
}


template <>
inline unsigned int SetFromString( const char* str, const unsigned int& defaultValue ) {
    return (unsigned int)SetFromString<int>( str, defaultValue );
}


template <>
inline bool SetFromString( const char* str, const bool& defaultValue ) {
    bool unrecognized = false;
    bool strAsBool = StringToBool( str, unrecognized );

    if( unrecognized ) {
        return defaultValue;
    }

    return strAsBool;
}


// Getters
template <typename T>
inline T NamedProperties::GetValue( const std::string& key, const T& defaultValue ) const {
    std::map<std::string, BaseProperty*, StringCmpCaseI>::const_iterator propIter = m_keyValuePairs.find( key );

    if( propIter == m_keyValuePairs.end() ) {
        return defaultValue;
    }

    BaseProperty* baseProp = propIter->second;
    TypedProperty<T>* typeProp = dynamic_cast<TypedProperty<T>*>(baseProp);

    if( typeProp == nullptr ) {
        std::string propStr = baseProp->GetAsString();
        return SetFromString( propStr.c_str(), defaultValue );
    } else {
        return typeProp->m_value;
    }
}


template <typename T>
inline T* NamedProperties::GetValue( const std::string& key, T* defaultPointer ) const {
    std::map<std::string, BaseProperty*, StringCmpCaseI>::const_iterator propIter = m_keyValuePairs.find( key );

    if( propIter == m_keyValuePairs.end() ) {
        return defaultPointer;
    }

    BaseProperty* baseProp = propIter->second;
    TypedProperty<T*>* typeProp = dynamic_cast< TypedProperty<T*> *>(baseProp);

    if( typeProp == nullptr ) {
        return defaultPointer;
    } else {
        return typeProp->m_value;
    }
}


template <typename T>
const T* NamedProperties::GetValue( const std::string& key, const T* defaultPointer ) const {
    std::map<std::string, BaseProperty*, StringCmpCaseI>::const_iterator propIter = m_keyValuePairs.find( key );

    if( propIter == m_keyValuePairs.end() ) {
        return defaultPointer;
    }

    BaseProperty* baseProp = propIter->second;
    TypedProperty<const T*>* typeProp = dynamic_cast< TypedProperty<const T*> *>(baseProp);

    if( typeProp == nullptr ) {
        return defaultPointer;
    } else {
        return typeProp->m_value;
    }
}


// Setters
template <typename T>
inline void NamedProperties::SetValue( const std::string& key, const T& value ) {
    std::map<std::string, BaseProperty*, StringCmpCaseI>::const_iterator propIter = m_keyValuePairs.find( key );

    if( propIter != m_keyValuePairs.end() ) { // Key already exists
        delete propIter->second;
    }

    TypedProperty<T>* typedProp = new TypedProperty<T>( value );
    m_keyValuePairs[key] = typedProp;
}


template <typename T>
inline void NamedProperties::SetValue( const std::string& key, const T* valuePointer ) {
    SetValue<const T*>( key, valuePointer );
}


template <typename T>
inline void NamedProperties::SetValue( const std::string& key, T* valuePointer ) {
    SetValue<T*>( key, valuePointer );
}
