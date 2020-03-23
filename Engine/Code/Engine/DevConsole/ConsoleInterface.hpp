#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/DevConsole/DevConsoleLine.hpp"

#include "mutex"


enum MouseEvent : int;

struct DCInterfaceInfo;
struct DCLoggerInfo;
struct DCRenderInfo;


class ConsoleInterface {
    public:
    virtual void Startup( bool fullStartup = true );
    virtual void Shutdown();

    virtual bool HandleKeyPressed( unsigned char keyCode );
    virtual bool HandleKeyReleased( unsigned char keyCode );
    virtual bool HandleCharTyped( unsigned char character );
    virtual bool HandleMouseEvent( MouseEvent event, float scrollAmount = 0 );

    void ExecuteCommandString( const std::string& commandString );

    bool IsTakingInput() const;
    void RunCommandTests();


    protected:
    virtual ~ConsoleInterface() {};

    DCInterfaceInfo GetInterfaceInfo() const;


    private:
    std::string m_currentInput = "";
    std::string m_temporaryInput = "";

    int m_scrollLines = 0;
    int m_cursorInversePosition = 0; // Position from the end of the string
    int m_highlightStartIndex = 0;

    int m_historyInverseIndex = 0;
    int m_historyMaxCommands = 10;

    bool m_vimExecMode = false;
    bool m_inputHighlight = false;

    bool m_controlPressed = false;
    bool m_shiftPressed = false;
    //bool m_altPressed = false;


    // Commands
    bool Command_Help( EventArgs& args );
    bool Command_Exec( EventArgs& args );
    bool Command_Test( EventArgs& args );
    bool Command_Echo( EventArgs& args );
    bool Command_ChangeChannel( EventArgs& args );

    // Input Key Events
    bool HandleVimCommand( unsigned char character );
    bool IsCharacterValid( unsigned char character ) const;

    void ShowPrevHistoryCommand();
    void ShowNextHistoryCommand();

    void MoveCursorRight();
    void MoveCursorLeft();
    void MoveCursorStartOfNextWord();
    void MoveCursorStartOfPrevWord();
    void MoveCursorEndOfNextWord();

    void DeleteCharacterIfValid( int characterIndex );
    void DeleteSelection();
    void DeleteEntireLine();

    void ExecuteCommandLine( const std::string& command );

    // DevConsole Communication
    virtual std::recursive_mutex& GetMutex() const = 0;
    virtual DCLoggerInfo GetLoggerInfo() const = 0;
    virtual DCRenderInfo GetRenderInfo() const = 0;
};
