#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/DevConsole/DevConsoleLine.hpp"

#include "mutex"


class BitmapFont;
class Camera;
class ConsoleInterface;
class ConsoleSystem;
class RenderContext;

struct AABB2;
struct DCInterfaceInfo;
struct DCLoggerInfo;
struct DCRenderInfo;


enum DevConsoleMode {
    DEV_CONSOLE_OFF,
    DEV_CONSOLE_FULLSCREEN,
    //DEV_CONSOLE_COMPACT
};


class ConsoleRenderer {
    public:
    virtual void Startup( bool fullStartup = true );
    virtual void Shutdown();

    void Render( RenderContext* renderer ) const;

    void SetMode( DevConsoleMode mode );
    void Toggle();


    protected:
    virtual ~ConsoleRenderer() {};

    virtual DCRenderInfo GetRenderInfo() const;


    private:
    Camera* m_camera = nullptr;
    DevConsoleMode m_mode = DEV_CONSOLE_OFF;

    static constexpr int MAX_LINES_PER_SCREEN = 55;


    void RenderMemTracking( RenderContext* renderer, float lineHeight, AABB2& consoleBounds, const BitmapFont* font ) const;
    void RenderConsole( RenderContext* renderer, float liNeHeight, AABB2& consoleBounds, AABB2& inputBounds ) const;
    void RenderStrings( RenderContext* renderer, float lineHeight, const AABB2& consoleBounds, const BitmapFont* font ) const;
    void RenderInput(   RenderContext* renderer, float lineHeight, const AABB2& inputBounds, const BitmapFont* font ) const;
    void RenderInputHighlight( RenderContext* renderer, float lineHeight, const AABB2& inputBounds, const BitmapFont* font, int startIndex, int endIndex ) const;

    AABB2 GetCharacterBoundsInFont( int charIndex, const std::string& inputString, float lineHeight, const BitmapFont* font ) const;

    // DevConsole communication
    virtual DCInterfaceInfo GetInterfaceInfo() const = 0;
    virtual DCLoggerInfo GetLoggerInfo() const = 0;

    virtual std::recursive_mutex& GetMutex() const = 0;
};
