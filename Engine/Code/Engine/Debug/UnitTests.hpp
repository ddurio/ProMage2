#pragma once
#include "Engine/Core/EngineCommon.hpp"


class UnitTest;
typedef void (*TestBodyCallback)( UnitTest* theTest );

struct TestFailure;


class UnitTest {
    public:
    UnitTest( const char* name, const char* category, int priority, TestBodyCallback callback );

    static UnitTest* s_testList;

    static void RunAll( int maxPriority = INT_MAX );
    static void RunCategory( const char* category, int maxPriority = INT_MAX );

    static void VerifyResult( bool isCorrect, const char* failureMessage, UnitTest* theTest );

    private:
    static DevConsoleChannel s_testChannel;
    static int s_numTotalTests;
    static int s_numPassedTests;
    static std::map< std::string, std::vector< TestFailure > > s_failuresByCategory;

    const char* m_testName = "";
    const char* m_category = "";
    int m_priority = 0;
    TestBodyCallback m_callback = nullptr;

    // In-place linked list
    UnitTest* m_nextTest = nullptr;


    static void PrintAllResults( int priority );
    static void PrintCategoryResults( const std::string& category, int priority );
    static void PrintResults();
    static bool PrintFailures( const std::string& category );

    static std::string GetEntryString( const std::string& category, int priority );
};


// Macro pseudo-code:
// Function forward declaraction:   static void     UniqueFuncName();
// Actual UnitTest object:          static UnitTest UniequeObjName();
// Actual function definition:      static void     UniqueFuncName()
// Author will add { ...body... } in their .cpp
#define UNITTEST( name, category, priority ) \
    static void     MACRO_COMBINE( __UnitTestBody_, __LINE__ )( UnitTest* theTest ); /* Function forward declaration */ \
    static UnitTest MACRO_COMBINE( __UnitTestObj_,  __LINE__ )( name, category, priority, MACRO_COMBINE( __UnitTestBody_, __LINE__ ) ); /* UnitTest class object */ \
    static void     MACRO_COMBINE( __UnitTestBody_, __LINE__ )( UnitTest* theTest ) /* Function signature. Body written by UnitTest authors */
