#include "Engine/DevConsole/ConsoleRenderer.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Debug/Profiler.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Memory/Memory.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Utils/NamedStrings.hpp"


void ConsoleRenderer::Startup( bool fullStartup /*= true */ ) {
    UNUSED( fullStartup );

    if( m_camera == nullptr ) {
        m_camera = new Camera();
    }
}


void ConsoleRenderer::Shutdown() {
    CLEAR_POINTER( m_camera );
}


void ConsoleRenderer::Render( RenderContext* renderer ) const {
    if( m_mode == DEV_CONSOLE_OFF ) {
        return;
    }

    std::string fontName = g_theGameConfigBlackboard.GetValue( "consoleFont", "" );

    if( fontName == "" ) {
        fontName = g_theGameConfigBlackboard.GetValue( "defaultFont", "" );
    }

    if( fontName == "" ) {
        ERROR_RECOVERABLE( "ERROR: No font found for Console" );
        return;
    }

    const BitmapFont* font = renderer->GetOrCreateBitmapFont( fontName );

    AABB2 consoleBounds;
    AABB2 inputBounds;

    renderer->BeginCamera( m_camera );

    TextureView2D* ctv = renderer->GetCurrentRenderTarget();
    float aspect = ctv->GetAspect();
    m_camera->SetOrthoView( Vec2::ZERO, Vec2( aspect, 1.f ) );
    float lineHeight = 1.f / (float)MAX_LINES_PER_SCREEN;

    Shader* shader = renderer->GetOrCreateShader( SHADER_CONSOLE );
    shader->SetDepthMode( COMPARE_ALWAYS, false );
    renderer->BindShader( shader );

    renderer->BindModelMatrix();
    consoleBounds = m_camera->GetBounds();
    RenderMemTracking( renderer, lineHeight, consoleBounds, font );
    RenderConsole( renderer, lineHeight, consoleBounds, inputBounds );
    RenderStrings( renderer, lineHeight, consoleBounds, font );
    RenderInput( renderer, lineHeight, inputBounds, font );

    renderer->EndCamera( m_camera );
}


void ConsoleRenderer::SetMode( DevConsoleMode mode ) {
    m_mode = mode;
}


void ConsoleRenderer::Toggle() {
    m_mode = (DevConsoleMode)(1 - m_mode);
}


DCRenderInfo ConsoleRenderer::GetRenderInfo() const {
    DCRenderInfo info;

    info.mode = m_mode;
    info.maxLinesPerScreen = MAX_LINES_PER_SCREEN - 3; // 2 lines @ 1.5 times (input and memTracker)

    return info;
}


// PRIVATE -------------------------------------------------------
void ConsoleRenderer::RenderMemTracking( RenderContext* renderer, float lineHeight, AABB2& consoleBounds, const BitmapFont* font ) const {
    if( !TrackedMemory::IsEnabled() ) {
        return;
    }

    VertexList memVerts;

    AABB2 memBounds = consoleBounds.CarveBoxOffBottom( 0.f, 1.5f * lineHeight );
    AddVertsForAABB2D( memVerts, memBounds, Rgba::WHITE );

    Vec2 memDimensions = memBounds.GetDimensions();
    Vec2 innerDimensions = memDimensions - Vec2( 0.5f * lineHeight, 0.5f * lineHeight );
    AABB2 innerBounds = memBounds.GetBoxWithin( innerDimensions, ALIGN_CENTER );

    renderer->BindTexture();
    renderer->DrawVertexArray( memVerts );
    memVerts.clear();

    // Mode
    std::string memMode = TrackedMemory::GetMode();
    font->AddVertsForTextInBox2D( memVerts, innerBounds, lineHeight, memMode, Rgba::BLUE, 1.f, ALIGN_CENTER_LEFT );

    // Num Allocations
    int numAllocs = TrackedMemory::GetNumLiveAllocations();
    std::string allocStr = Stringf( "Live Allocs: %d", numAllocs );
    font->AddVertsForTextInBox2D( memVerts, innerBounds, lineHeight, allocStr, Rgba::BLUE, 1.f, ALIGN_CENTER );

    // Num Bytes
    std::string numBytes = TrackedMemory::GetLiveByteString();
    std::string byteStr = Stringf( "Live Bytes: %s", numBytes.c_str() );
    font->AddVertsForTextInBox2D( memVerts, innerBounds, lineHeight, byteStr, Rgba::BLUE, 1.f, ALIGN_CENTER_RIGHT );

    renderer->BindTexture( font->GetTexturePath() );
    renderer->DrawVertexArray( memVerts );
}


void ConsoleRenderer::RenderConsole( RenderContext* renderer, float lineHeight, AABB2& consoleBounds, AABB2& inputBounds ) const {
    // Background
    VertexList consoleVerts;
    AddVertsForAABB2D( consoleVerts, consoleBounds, Rgba( 0.f, 0.f, 0.f, 0.5f ) );

    // Command Line
    inputBounds = consoleBounds.CarveBoxOffBottom( 0.f, 1.5f * lineHeight );
    Rgba inputBoxColor = Rgba::GRAY;
    inputBoxColor.a = 0.5f;
    AddVertsForAABB2D( consoleVerts, inputBounds, inputBoxColor );

    // Draw!
    renderer->BindTexture();
    renderer->DrawVertexArray( consoleVerts );
}


void ConsoleRenderer::RenderStrings( RenderContext* renderer, float lineHeight, const AABB2& consoleBounds, const BitmapFont* font ) const {
    AABB2 remainingBounds = consoleBounds;
    VertexList textVerts;

    DCInterfaceInfo info = GetInterfaceInfo();
    int linesToScroll = info.numLinesScroll;

    std::scoped_lock< std::recursive_mutex > localLock( GetMutex() );
    DCLoggerInfo logInfo = GetLoggerInfo();
    std::vector<DevConsoleLine>::const_reverse_iterator lineIter = logInfo.lines.rbegin();

    for( lineIter; lineIter != logInfo.lines.rend(); lineIter++ ) {
        // Is there room to print it?
        if( remainingBounds.GetDimensions().y < lineHeight ) {
            break;
        }

        // Is the channel active?
        if( (logInfo.activeChannels & lineIter->m_channel) == 0 ) {
            continue;
        }

        // Is it scrolled off the bottom
        if( linesToScroll > 0 ) {
            linesToScroll--;
            continue;
        }

        AABB2 lineBox = remainingBounds.CarveBoxOffBottom( 0.f, lineHeight );
        std::string lineText = Stringf( "%d, %.02f: %s", lineIter->m_frameNumber, lineIter->m_printTime, lineIter->m_string.c_str() );

        font->AddVertsForTextInBox2D( textVerts, lineBox, lineHeight, lineText, lineIter->m_color, 1.f, ALIGN_CENTER_LEFT );
    }

    renderer->BindTexture( font->GetTexturePath() );
    renderer->DrawVertexArray( textVerts );
}


void ConsoleRenderer::RenderInput( RenderContext* renderer, float lineHeight, AABB2 inputBounds, const BitmapFont* font ) const {
    AABB2 fpsBounds = inputBounds.CarveBoxOffRight( 0.2f );

    Vec2 inputDimensions = inputBounds.GetDimensions();
    Vec2 innerDimensions = inputDimensions - Vec2( 0.5f * lineHeight, 0.5f * lineHeight );
    AABB2 innerBounds = inputBounds.GetBoxWithin( innerDimensions, ALIGN_CENTER );

    DCInterfaceInfo info = GetInterfaceInfo();

    std::string inputString = "> " + info.inputStr;
    int inputLength = (int)inputString.length();
    int cursorPosition = inputLength - info.cursorInversePosition;

    // Draw highlight verts first
    if( info.isHighlighting ) {
        // Account for "> " once here
        int highlightStart = Min( info.highlightStartIndex + 2, cursorPosition );
        int highlightEnd = Max( info.highlightStartIndex + 2, cursorPosition );

        RenderInputHighlight( renderer, lineHeight, innerBounds, font, highlightStart, highlightEnd );
    }

    // Add verts for input
    VertexList textVerts;
    font->AddVertsForTextInBox2D( textVerts, innerBounds, lineHeight, inputString, Rgba::WHITE, 1.f, ALIGN_CENTER_LEFT );

    // Add verts for FPS
    double avgFrameSeconds = g_theProfiler->GetAverageFrameSeconds();
    std::string avgFrameStr = GetProfileTimeString( avgFrameSeconds );

    int avgFPS = RoundToInt( 1. / avgFrameSeconds );
    std::string fpsStr = Stringf( "%dFPS (%s)", avgFPS, avgFrameStr.c_str() );
    Rgba fpsColor = (avgFPS >= 60) ? Rgba::ORGANIC_GREEN : Rgba::ORGANIC_RED;

    font->AddVertsForTextInBox2D( textVerts, fpsBounds, lineHeight, fpsStr, fpsColor );

    // Draw text verts
    renderer->BindTexture( font->GetTexturePath() );
    renderer->DrawVertexArray( textVerts );

    double blinkRate = 1.5f * GetCurrentTimeSeconds();
    bool caretVisible = (int)blinkRate % 2;

    // Draw Cursor
    if( caretVisible ) {
        AABB2 characterBounds = GetCharacterBoundsInFont( cursorPosition, inputString, lineHeight, font );
        characterBounds += innerBounds.mins;
        VertexList cursorVerts;

        if( info.isVimMode ) {
            AddVertsForAABB2D( cursorVerts, characterBounds, Rgba::WHITE );
        } else {
            Vec2 cursorBottom = characterBounds.mins;
            Vec2 cursorTop = cursorBottom + Vec2( 0.f, lineHeight );

            AddVertsForLine2D( cursorVerts, cursorBottom, cursorTop, 0.1f * lineHeight, Rgba::WHITE );
        }

        renderer->BindTexture();
        renderer->DrawVertexArray( cursorVerts );
    }
}


void ConsoleRenderer::RenderInputHighlight( RenderContext* renderer, float lineHeight, const AABB2& inputBounds, const BitmapFont* font, int startIndex, int endIndex ) const {
    if( startIndex > endIndex ) {
        // Invalid indices
        return;
    }

    DCInterfaceInfo info = GetInterfaceInfo();
    std::string inputString = "> " + info.inputStr;
    AABB2 startBounds = GetCharacterBoundsInFont( startIndex, inputString, lineHeight, font );
    startBounds += inputBounds.mins;
    AABB2 endBounds;

    if( endIndex == startIndex ) {
        endBounds = startBounds;
    } else {
        endBounds = GetCharacterBoundsInFont( endIndex, inputString, lineHeight, font );
        endBounds += inputBounds.mins;
    }

    AABB2 highlightBounds = AABB2( startBounds.mins, endBounds.maxs );
    Rgba highlightColor = Rgba::YELLOW;
    highlightColor.a = 0.5f;

    VertexList highlighVerts;
    AddVertsForAABB2D( highlighVerts, highlightBounds, highlightColor );

    renderer->BindTexture();
    renderer->DrawVertexArray( highlighVerts );
}


AABB2 ConsoleRenderer::GetCharacterBoundsInFont( int charIndex, const std::string& inputString, float lineHeight, const BitmapFont* font ) const {
    float cursorX = 0.f;

    for( int index = 0; index < charIndex; index++ ) {
        cursorX += lineHeight * font->GetGlyphAspect( inputString[index] );
    }

    Vec2 mins = Vec2( cursorX, 0.f );

    float charWidth;
    if( charIndex < (int)inputString.length() ) {
        charWidth = lineHeight * font->GetGlyphAspect( inputString[charIndex] );
    } else {
        // Assume aspect of 1 since the character doesn't exist
        charWidth = lineHeight;
    }
    Vec2 maxs = Vec2( cursorX + charWidth, lineHeight );

    return AABB2( mins, maxs );
}
