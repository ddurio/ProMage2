#include "Engine/Utils/StringUtils.hpp"

#include <stdarg.h>
#include <cctype>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... ) {
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... ) {
	char textLiteralSmall[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[maxLength];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[maxLength - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


const Strings SplitStringOnDelimeter( const std::string& originalString, char delimeter, bool returnEmptyValues /*= true */ ) {
    std::string stringToSplit = originalString;
    std::vector<std::string> splitString;
    int endIndex = (int)stringToSplit.find_first_of( delimeter );

    while( endIndex != std::string::npos ) {
        std::string subString = std::string( stringToSplit, 0, endIndex );

        if( returnEmptyValues || subString != "" ) {
            splitString.push_back( subString );
        }

        stringToSplit = stringToSplit.substr( endIndex + 1 );
        endIndex = (int)stringToSplit.find_first_of( delimeter );
    }

    if( returnEmptyValues || stringToSplit != "" ) {
        splitString.push_back( stringToSplit );
    }

    return splitString;
}


const std::string JoinStrings( const Strings& stringArray, const std::string& delimeter /*= "," */ ) {
    int numStrings = (int)stringArray.size();

    if( numStrings == 0 ) {
        return "";
    }

    std::string joinedString = stringArray[0];

    for( int stringIndex = 1; stringIndex < numStrings; stringIndex++ ) {
        joinedString = Stringf( "%s%s%s", joinedString.c_str(), delimeter.c_str(),  stringArray[stringIndex].c_str() );
    }

    return joinedString;
}


bool StringToBool( const std::string inString ) {
    std::string lowerString = StringToLower( inString );

    if( inString == "0" ||
        inString == "f" ||
        inString == "false" ) {
        return false;
    } else if( inString == "1" ||
        inString == "t" ||
        inString == "true" ) {
        return true;
    } else {
        return false;
    }
}


bool StringToBool( const std::string inString, bool& outUnrecognized ) {
    std::string lowerString = StringToLower( inString );
    outUnrecognized = false;

    if( inString == "0" ||
        inString == "f" ||
        inString == "false" ) {
        return false;
    } else if( inString == "1" ||
        inString == "t" ||
        inString == "true" ) {
        return true;
    } else {
        outUnrecognized = true;
        return false;
    }
}


bool TextToBool( const char* inText ) {
    return StringToBool( inText );
}


const std::string StringToLower( const std::string& inputString ) {
    std::string outputString = inputString;
    std::string::iterator strIter;

    for( strIter = outputString.begin(); strIter != outputString.end(); strIter++ ) {
        char& letter = *strIter;
        letter = (char)std::tolower( letter );
    }

    return outputString;
}


const std::string StringToUpper( const std::string& inputString ) {
    std::string outputString = inputString;
    std::string::iterator strIter;

    for( strIter = outputString.begin(); strIter != outputString.end(); strIter++ ) {
        char& letter = *strIter;
        letter = (char)std::toupper( letter );
    }

    return outputString;
}


bool StringICmp( const std::string& stringA, const std::string& stringB ) {
    return StringICmp( stringA.c_str(), stringB.c_str() );
}


bool StringICmp( const std::string& stringA, const char* stringB ) {
    return StringICmp( stringA.c_str(), stringB );
}


bool StringICmp( const char* stringA, const std::string& stringB ) {
    return StringICmp( stringA, stringB.c_str() );
}


bool StringICmp( const char* stringA, const char* stringB ) {
    int compareResult = _stricmp( stringA, stringB );

    return (compareResult == 0);
}


bool StringCmpCaseI::operator()( const std::string& stringA, const std::string& stringB ) const {
    return (_stricmp( stringA.c_str(), stringB.c_str() ) < 0);
}
