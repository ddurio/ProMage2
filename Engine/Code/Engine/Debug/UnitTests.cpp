#include "Engine/Debug/UnitTests.hpp"

#include "Engine/Debug/Callstack.hpp"
#include "Engine/DevConsole/DevConsole.hpp"


struct TestFailure {
    std::string failureMessage = "";
    Callstack callstack;
};


UnitTest* UnitTest::s_testList = nullptr;
DevConsoleChannel UnitTest::s_testChannel = DevConsole::CHANNEL_UNDEFINED;
int UnitTest::s_numTotalTests  = 0;
int UnitTest::s_numPassedTests = 0;
std::map< std::string, std::vector< TestFailure > > UnitTest::s_failuresByCategory;


UnitTest::UnitTest( const char* name, const char* category, int priority, TestBodyCallback callback ) :
    m_testName( name ),
    m_category( category ),
    m_priority( priority ),
    m_callback( callback ) {
    m_nextTest = s_testList;
    s_testList = this;
}


void UnitTest::RunAll( int maxPriority /*= INT_MAX */ ) {
    UnitTest* testIter = s_testList;

    while( testIter != nullptr ) {
        if( testIter->m_priority <= maxPriority ) {
            testIter->m_callback( testIter );
        }

        testIter = testIter->m_nextTest;
    }

    PrintAllResults( maxPriority );
}


void UnitTest::RunCategory( const char* category, int maxPriority /*= INT_MAX */ ) {
    UnitTest* testIter = s_testList;

    while( testIter != nullptr ) {
        if( testIter->m_priority <= maxPriority && testIter->m_category == category ) {
            testIter->m_callback( testIter );
        }

        testIter = testIter->m_nextTest;
    }

    PrintCategoryResults( category, maxPriority );
}


void UnitTest::VerifyResult( bool isCorrect, const char* failureMessage, UnitTest* theTest ) {
    s_numTotalTests++;

    if( isCorrect ) {
        s_numPassedTests++;
    } else {
        TestFailure failure;
        failure.failureMessage = failureMessage;
        failure.callstack = Callstack::GetCallstack( 1 );

        std::string category = theTest->m_category;
        s_failuresByCategory[category].push_back( failure );
    }
}


void UnitTest::PrintAllResults( int priority ) {
    if( s_testChannel == DevConsole::CHANNEL_UNDEFINED ) {
        s_testChannel = g_theDevConsole->AddChannel( "UnitTests", Rgba::ORGANIC_BLUE );
    }

    std::string entryStr = GetEntryString( "", priority );
    std::string msg = Stringf( "(UnitTests) RunAll: %s", entryStr.c_str() );
    g_theDevConsole->PrintString( msg, s_testChannel );

    PrintResults();
    bool testsFailed = false;
    std::map< std::string, std::vector< TestFailure > >::const_iterator categoryIter = s_failuresByCategory.begin();

    while( categoryIter != s_failuresByCategory.end() ) {
        testsFailed = PrintFailures( categoryIter->first.c_str() ) || testsFailed;
        categoryIter++;
    }

    if( testsFailed ) {
        ERROR_RECOVERABLE( "(UnitTests) Consult DevConsole or VS Output for failed UnitTests" );
        s_failuresByCategory.clear();
    }
}


void UnitTest::PrintCategoryResults( const std::string& category, int priority ) {
    if( s_testChannel == DevConsole::CHANNEL_UNDEFINED ) {
        s_testChannel = g_theDevConsole->AddChannel( "UnitTests", Rgba::ORGANIC_BLUE );
    }

    std::string entryStr = GetEntryString( category, priority );
    std::string msg = Stringf( "(UnitTests) RunCategory: %s", entryStr.c_str() );
    g_theDevConsole->PrintString( msg, s_testChannel );

    PrintResults();
    bool testsFailed = PrintFailures( category );

    if( testsFailed ) {
        ERROR_RECOVERABLE( "(UnitTests) Consult DevConsole or VS Output for failed UnitTests" );
        s_failuresByCategory.erase( category );
    }
}


void UnitTest::PrintResults() {
    std::string resultStr = "";

    if( s_numPassedTests == s_numTotalTests ) {
        resultStr = Stringf( "All %d tests passed!", s_numPassedTests );
    } else {
        resultStr = Stringf( "%d / %d tests passed...", s_numPassedTests, s_numTotalTests );
    }

    g_theDevConsole->PrintString( resultStr, s_testChannel );
    s_numPassedTests = 0;
    s_numTotalTests = 0;
}


bool UnitTest::PrintFailures( const std::string& category ) {
    std::map< std::string, std::vector< TestFailure > >::iterator failureIter = s_failuresByCategory.find( category );

    if( failureIter != s_failuresByCategory.end() ) {
        std::vector< TestFailure >& failures = failureIter->second;
        int numFailures = (int)failures.size();

        for( int failureIndex = 0; failureIndex < numFailures; failureIndex++ ) {
            TestFailure& failure = failures[failureIndex];

            // Error message from unitTest
            std::string failureStr = Stringf( "(UnitTests) %s: %s", category.c_str(), failure.failureMessage.c_str() );
            g_theDevConsole->PrintString( failureStr, s_testChannel | DevConsole::CHANNEL_ERROR );

            DebuggerPrintf( "\n" );
            DebuggerPrintf( failureStr.c_str() );

            // Callstack
            failure.callstack.Print( s_testChannel );
        }

        return true;
    }

    return false;
}


std::string UnitTest::GetEntryString( const std::string& category, int priority ) {
    std::string commaStr    = (category == "")      ? "" : ", ";
    std::string priorityStr = (priority == INT_MAX) ? "" : Stringf( "%s%d", commaStr.c_str(), priority );

    std::string entryStr = Stringf( "%s%s", category.c_str(), priorityStr.c_str() );
    return entryStr;
}
