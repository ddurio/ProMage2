#pragma once
// Warning --> Error
#pragma warning( error: 4150 ) // Deletion of pointer to incomplete type
#pragma warning( error: 4172 ) // Return address of local variable
#pragma warning( error: 4715 ) // Not all paths return a value
#pragma warning( error: 4840 ) // Non-portable use of class std::string to a variadric function

// Warning --> Disabled
#pragma warning( disable: 4505 ) // Unreferenced local function has been removed


#include "Engine/Utils/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"

#include "map"
#include "vector"


#define UNUSED(x) (void)(x);

// D3D11 Release Resource
#define DX_SAFE_RELEASE(dx_resource)   if ((dx_resource) != nullptr) { dx_resource->Release(); dx_resource = nullptr; }
#define CLEAR_POINTER(pointer)   if ((pointer) != nullptr) { delete pointer; pointer = nullptr; }

// BitFlag Creator
#define BIT_FLAG(b) ((1U) << b)

// Macro Helpers
#define MACRO_COMBINE1( X, Y ) X##Y // helper, not for direct usage
#define MACRO_COMBINE( X, Y ) MACRO_COMBINE1( X, Y )


class DebugDraw;
class DevConsole;
class EventSystem;
class ImGuiSystem;
class LogSystem;
class NamedStrings;
class ProfilerSystem;

extern DebugDraw*       g_theDebugger;
extern DevConsole*      g_theDevConsole;
extern EventSystem*     g_theEventSystem;
extern ImGuiSystem*     g_theGui;
extern LogSystem*       g_theLogger;
extern NamedStrings     g_theGameConfigBlackboard;
extern ProfilerSystem*  g_theProfiler;

typedef unsigned int DevConsoleChannel;


struct Vec2;
extern Vec2 ALIGN_CENTER;
extern Vec2 ALIGN_BOTTOM_LEFT;
extern Vec2 ALIGN_BOTTOM_RIGHT;
extern Vec2 ALIGN_TOP_LEFT;
extern Vec2 ALIGN_TOP_RIGHT;
extern Vec2 ALIGN_TOP_CENTER;
extern Vec2 ALIGN_BOTTOM_CENTER;
extern Vec2 ALIGN_CENTER_LEFT;
extern Vec2 ALIGN_CENTER_RIGHT;


constexpr char EVENT_SCREENSHOT[] = "takeScreenshot";


class EngineCommon {
    public:

    template< typename MapSecond >
    static void ClearMap( std::map<std::string, MapSecond*>& theMap ) {
        typename std::map<std::string, MapSecond*>::iterator mapIter = theMap.begin();

        for( mapIter; mapIter != theMap.end(); mapIter++ ) {
            CLEAR_POINTER( mapIter->second );
        }

        theMap.clear();
    }


    template< typename VectorObj >
    static void ClearVector( std::vector<VectorObj*>& theVector ) {
        int numObjects = (int)theVector.size();

        for( int objectIndex = 0; objectIndex < numObjects; objectIndex++ ) {
            VectorObj*& object = theVector[objectIndex];

            if( object != nullptr ) {
                CLEAR_POINTER( object );
            }
        }

        theVector.clear();
    }


    template< typename VectorObj >
    static void DeleteFromVector( VectorObj* theObject, std::vector<VectorObj*>& theVector ) {
        int numObjects = (int)theVector.size();

        for( int objectIndex = 0; objectIndex < numObjects; objectIndex++ ) {
            VectorObj*& vectorObj = theVector[objectIndex];

            if( vectorObj == theObject ) {
                CLEAR_POINTER( vectorObj );
            }
        }
    }


    template< typename VectorObj >
    static bool VectorContains( const std::vector< VectorObj >& theVector, const VectorObj& theObject ) {
        int numObjects = (int)theVector.size();

        for( int objectIndex = 0; objectIndex < numObjects; objectIndex++ ) {
            const VectorObj& vectorObj = theVector[objectIndex];

            if( vectorObj == theObject ) {
                return true;
            }
        }

        return false;
    }
};
