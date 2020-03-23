#include "Engine/Debug/Callstack.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "DbgHelp.h"
#pragma comment( lib, "DbgHelp.lib" )

#include "Engine/DevConsole/DevConsole.hpp"


bool Callstack::s_initialized = false;


Callstack Callstack::GetCallstack( int numSkippedFrames /*= 0 */ ) {
    Callstack callstack;

    // 0 numSkippedFrames should mean from where GetCallstack was called
    // Always need to skip the current frame for GetCallstack! (+1)
    callstack.m_numFrames = CaptureStackBackTrace( numSkippedFrames + 1, s_maxNumFrames, callstack.m_pointerStack, (PDWORD)&(callstack.m_hashKey) );

    return callstack;
}


void* Callstack::GetHash() const {
    return m_hashKey;
}


Strings Callstack::GetAsStrings() const {
    if( !s_initialized ) {
        HANDLE processHandle = GetCurrentProcess();
        SymInitialize( processHandle, NULL, TRUE );
        s_initialized = true;
    }

    Strings stringStack;
    HANDLE processHandle = GetCurrentProcess();

    for( int frameIndex = 0; frameIndex < m_numFrames; frameIndex++ ) {
        const void* framePointer = m_pointerStack[frameIndex];

        char buffer[sizeof( SYMBOL_INFO ) + 256 * sizeof( TCHAR )];
        SYMBOL_INFO* symInfo = (PSYMBOL_INFO)buffer;
        symInfo->SizeOfStruct = sizeof( SYMBOL_INFO );
        symInfo->MaxNameLen = 256;

        IMAGEHLP_LINE lineInfo;
        lineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );
        DWORD charOffset = 0;

        HRESULT funcResult = SymFromAddr( processHandle, (DWORD64)framePointer, 0, symInfo );
        HRESULT lineResult = SymGetLineFromAddr( processHandle, (DWORD64)framePointer, &charOffset, &lineInfo );

        if( SUCCEEDED( funcResult ) && SUCCEEDED( lineResult ) ) {
            std::string fileName = lineInfo.FileName;
            std::string funcName = symInfo->Name;
            int lineNum = lineInfo.LineNumber;
            int charNum = charOffset;

            std::string frameStr = Stringf( "%s(%d,%d): %s", fileName.c_str(), lineNum, charNum, funcName.c_str() );
            stringStack.push_back( frameStr );
        }
    }

    return stringStack;
}


void Callstack::Print( const DevConsoleChannel& extraChannels /*= DevConsole::CHANNEL_UNDEFINED */ ) const {
    DevConsoleChannel channel = DevConsole::CHANNEL_ERROR | extraChannels;
    Strings stackStrs = GetAsStrings();
    int numStrings = (int)stackStrs.size();

    for( int strIndex = 0; strIndex < numStrings; strIndex++ ) {
        // VS Output
        std::string& stackStr = stackStrs[strIndex];
        DebuggerPrintf( "\n" );
        DebuggerPrintf( stackStr.c_str() );

        // DevConsole
        std::string consoleStr = "";

        int engineIndex = (int)stackStr.find( "\\engine\\code\\" );
        int gameIndex = (int)stackStr.find( "\\code\\game\\" );

        if( engineIndex != -1 ) { // Engine code
            consoleStr = stackStr.substr( engineIndex + 13 );
        } else if( gameIndex != -1 ) { // Game code
            consoleStr = stackStr.substr( gameIndex + 6 );
        }

        if( consoleStr != "" ) {
            g_theDevConsole->PrintString( consoleStr, channel );
        }
    }

    DebuggerPrintf( "\n" );
}
