#pragma once
#include "Engine/Core/EngineCommon.hpp"
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef std::vector<std::string> Strings;

const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

const Strings SplitStringOnDelimeter( const std::string& originalString, char delimeter, bool returnEmptyValues = true );
const std::string JoinStrings( const Strings& stringArray, const std::string& delimeter = "," );

bool StringToBool( const std::string inString );
bool StringToBool( const std::string inString, bool& outUnrecognized );
bool TextToBool( const char* inText );

const std::string StringToLower( const std::string& inputString );
const std::string StringToUpper( const std::string& inputString );

bool StringICmp( const std::string& stringA, const std::string& stringB );
bool StringICmp( const std::string& stringA, const char*        stringB );
bool StringICmp( const char*        stringA, const std::string& stringB );
bool StringICmp( const char*        stringA, const char*        stringB );

// Useful for std::map third parameter to make case insensitive:  std::map< std::string, T, StrCompareCaseInsensitive >
struct StringCmpCaseI {
    bool operator()( const std::string& stringA, const std::string& stringB ) const;
};
