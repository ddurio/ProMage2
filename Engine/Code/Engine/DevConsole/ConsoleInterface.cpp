#include "Engine/DevConsole/ConsoleInterface.hpp"

#include "Engine/Core/WindowContext.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Utils/NamedStrings.hpp"

#include "mutex"


DCInterfaceInfo ConsoleInterface::GetInterfaceInfo() const {
    DCInterfaceInfo info;

    info.inputStr = m_currentInput;

    info.numLinesScroll = m_scrollLines;
    info.cursorInversePosition = m_cursorInversePosition;
    info.highlightStartIndex = m_highlightStartIndex;

    info.isHighlighting = m_inputHighlight;
    info.isVimMode = m_vimExecMode;

    return info;
}


// PRIVATE --------------------------------------
void ConsoleInterface::Startup( bool fullStartup /*= true */ ) {
    if( !fullStartup ) {
        return;
    }

    g_theEventSystem->Subscribe( "Help", this, &ConsoleInterface::Command_Help );
    g_theEventSystem->Subscribe( "Exec", this, &ConsoleInterface::Command_Exec );
    g_theEventSystem->Subscribe( "Echo", this, &ConsoleInterface::Command_Echo );
    g_theEventSystem->Subscribe( "ChangeChannel", this, &ConsoleInterface::Command_ChangeChannel );
    g_theEventSystem->Subscribe( "Test", this, &ConsoleInterface::Command_Test );

    m_historyMaxCommands = g_theGameConfigBlackboard.GetValue( "CONSOLE_MAX_HISTORY", m_historyMaxCommands );
}


void ConsoleInterface::Shutdown() {
    g_theEventSystem->Unsubscribe( "Help", this, &ConsoleInterface::Command_Help );
    g_theEventSystem->Unsubscribe( "Exec", this, &ConsoleInterface::Command_Exec );
    g_theEventSystem->Unsubscribe( "Echo", this, &ConsoleInterface::Command_Echo );
    g_theEventSystem->Unsubscribe( "ChangeChannel", this, &ConsoleInterface::Command_ChangeChannel );
    g_theEventSystem->Unsubscribe( "Test", this, &ConsoleInterface::Command_Test );

}


bool ConsoleInterface::HandleKeyPressed( unsigned char keyCode ) {
    // SD2FIXME: Change all return false options in switch to true when implemented
    switch( keyCode ) {
        case('\b'): { // Backspace (0x08)
            if( m_inputHighlight ) {
                DeleteSelection();
            } else {
                // remove one character
                int inputLength = (int)m_currentInput.length();
                int characterIndex = inputLength - m_cursorInversePosition - 1;
                DeleteCharacterIfValid( characterIndex );
            }
            return true;
        } case(0x0D): { // Enter
            // submit command
            ExecuteCommandString( m_currentInput );
            m_currentInput = "";
            m_cursorInversePosition = 0;
            m_historyInverseIndex = 0;
            return true;
        } case(0x10): { // Shift
            // used for tracking text selection
            m_shiftPressed = true;
            return true;
        } case(0x11): { // Control
            // scrolling up
            // move cursor by word increments
            m_controlPressed = true;
            return false;
        } case(0x1B): { // Escape
            if( m_vimExecMode ) {
                m_inputHighlight = false;
            } else {
                m_vimExecMode = true;

                if( m_cursorInversePosition == 0 && m_currentInput != "") {
                    m_cursorInversePosition = 1;
                }
            }
            return true;
        } case(0x23): { // End
            // Move cursor to end of line
            m_cursorInversePosition = 0;
            return true;
        } case(0x24): { // Home
            // Move cursor to beginning of line
            m_cursorInversePosition = (int)m_currentInput.length();
            return true;
        } case(0x25): { // Arrow left
            // move cursor left
            MoveCursorLeft();
            return true;
        } case(0x26): { // Arrow up
            // go back in command history
            // scroll up if shift pressed
            ShowPrevHistoryCommand();
            return true;
        } case(0x27): { // Arrow right
            // move cursor right
            MoveCursorRight();
            return true;
        } case(0x28): { // Arrow down
            // go forward in command history
            // scroll down if shift pressed
            ShowNextHistoryCommand();
            return true;
        } case(0x2E): { // Delete
            if( m_inputHighlight ) {
                DeleteSelection();
            } else {
                // delete character ahead of cursor
                int inputLength = (int)m_currentInput.length();
                int characterIndex = inputLength - m_cursorInversePosition;
                DeleteCharacterIfValid( characterIndex );
                MoveCursorRight();
            }
            return true;
        }
    }

    return false;
}


bool ConsoleInterface::HandleKeyReleased( unsigned char keyCode ) {
    switch( keyCode ) {
        case(0x10): { // Shift
            m_shiftPressed = false;
            return true;
        } case(0x11): { // Control
            m_controlPressed = false;
            return true;
        }
    }

    return false;
}


bool ConsoleInterface::HandleCharTyped( unsigned char character ) {
    if( !IsCharacterValid(character) ) {
        return false;
    } else if( m_vimExecMode ) {
        return HandleVimCommand( character );
    }

    if( m_cursorInversePosition == 0 ) {
        m_currentInput += character;
    } else {
        int inputLength = (int)m_currentInput.length();
        std::string newInput = m_currentInput.substr( 0, inputLength - m_cursorInversePosition ); 
        newInput += character;
        newInput += m_currentInput.substr( inputLength - m_cursorInversePosition, m_cursorInversePosition );
        m_currentInput = newInput;
    }

    return true;
}


bool ConsoleInterface::HandleMouseEvent( MouseEvent event, float scrollAmount /*= 0*/  ) {
    if( event == MOUSE_EVENT_SCROLL ) {
        DCLoggerInfo logInfo = GetLoggerInfo();
        DCRenderInfo renderInfo = GetRenderInfo();

        int maxScroll = logInfo.numUnfilteredLines - renderInfo.maxLinesPerScreen + 3;
        // Handle console and memTracker line @ 1.5*lineHeight each

        if( maxScroll > 0 ) {
            m_scrollLines += (int)(scrollAmount);
            m_scrollLines = Clamp( m_scrollLines, 0, maxScroll );
        } else {
            m_scrollLines = 0;
        }

        return true;
    }

    return false;
}


void ConsoleInterface::ExecuteCommandString( const std::string& commandString ) {
    Strings commandList = SplitStringOnDelimeter( commandString, '\n' );
    int numCommands = (int)commandList.size();

    for( int commandIndex = 0; commandIndex < numCommands; commandIndex++ ) {
        const std::string& command = commandList[commandIndex];
        ExecuteCommandLine( command );
    }
}


bool ConsoleInterface::IsTakingInput() const {
    DCRenderInfo info = GetRenderInfo();

    return (info.mode == DEV_CONSOLE_FULLSCREEN);
}


void ConsoleInterface::RunCommandTests() {
    ExecuteCommandString( "BadCommand" ); // Should fail

    ExecuteCommandString( "Help" );       // Print HELP

    ExecuteCommandString( "Test badArgs = badFormat" ); // Should fail
    ExecuteCommandString( "Test goodArgs=goodFormat" ); // Call test function, print args

    ExecuteCommandString( "Test multiple=lines\nExec file=Data/Gameplay/TestExecFile.xml" ); // Print test with args, then print exec
}


bool ConsoleInterface::Command_Help( EventArgs& args ) {
    UNUSED( args );
    g_theDevConsole->PrintString( "Help Command Called" );

    Strings validCommands;
    g_theEventSystem->GetSubscribedEventsList( validCommands );

    int numCommands = (int)validCommands.size();

    for( int commandIndex = 0; commandIndex < numCommands; commandIndex++ ) {
        g_theDevConsole->PrintString( validCommands[commandIndex] );
    }

    return false;
}


bool ConsoleInterface::Command_Exec( EventArgs& args ) {
    std::string fileName = args.GetValue( "file", "" );

    if( fileName == "" ) {
        g_theDevConsole->PrintString( "ERROR: Exec missing required argument \"file\"", DevConsole::CHANNEL_ERROR );
        g_theDevConsole->PrintString( "     - Usage Example: Exec file=<fileName>", DevConsole::CHANNEL_ERROR );
        return true;
    }

    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( fileName.c_str(), document );

    const char* tagName = "DevConsoleCommand";
    const XMLElement* element = root.FirstChildElement( tagName );

    /*  TODO: Switch to clearer text when Echo command supports spaces and quotes
    std::string errorText = Stringf( "ERROR: (%s) DevConsoleCommnad missing required argument \"command\"", fileName.c_str() );
    std::string usageText = "     - Usage Example: <DevConsoleCommand command=\"<validCommand>\"/>";
    std::string errorCommand = Stringf( "Echo text=\"%s\n%s\" color=RED", errorText.c_str(), usageText.c_str() );
    */
    std::string errorCommand = "Echo text=ERROR:InvalidFileCommand color=RED";

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        std::string commandString = ParseXMLAttribute( *element, "command", errorCommand );
        ExecuteCommandString( commandString );
    }

    return false;
}


bool ConsoleInterface::Command_Test( EventArgs& args ) {
    g_theDevConsole->PrintString( "Test Command Called" );
    g_theDevConsole->PrintString( Stringf("- Args: \"%s\"", args.GetAsString().c_str() ) );
    return false;
}


//TODO: Improve to handle spaces and quotes
bool ConsoleInterface::Command_Echo( EventArgs& args ) {
    std::string text          = args.GetValue( "text", "" );
    //Rgba color                = args.GetValue( "color", CHANNEL_COLOR_INFO );
    DevConsoleChannel channel = args.GetValue<unsigned int>( "channel", DevConsole::CHANNEL_UNDEFINED );

    g_theDevConsole->PrintString( text, channel );
    return false;
}


bool ConsoleInterface::Command_ChangeChannel( EventArgs& args ) {
    std::string channel = args.GetValue( "channel", "" );

    if( channel != "" ) {
        g_theDevConsole->SwitchToChannel( channel );
    }

    return false;
}


bool ConsoleInterface::HandleVimCommand( unsigned char character ) {
    switch(character) {
        case('a'): {
            m_vimExecMode = false;
            MoveCursorRight();
            return true;
        } case('A'): {
            m_cursorInversePosition = 0;
            m_vimExecMode = false;
            return true;
        } case('b'): {
            MoveCursorStartOfPrevWord();
            return true;
        } case('e'): {
            MoveCursorEndOfNextWord();
            return true;
        } case('h'): {
            MoveCursorLeft();
            return true;
        } case('i'): {
            m_vimExecMode = false;
            return true;
        } case('I'): {
            m_cursorInversePosition = (int)m_currentInput.length();
            m_vimExecMode = false;
            return true;
        } case('l'): {
            MoveCursorRight();
            return true;
        } case('v'): {
            m_inputHighlight = true;
            m_highlightStartIndex = (int)m_currentInput.length() - m_cursorInversePosition;
            return true;
        } case('V'): {
            m_inputHighlight = true;
            m_highlightStartIndex = 0;
            m_cursorInversePosition = 0;
            return true;
        } case('w'): {
            MoveCursorStartOfNextWord();
            return true;
        } case('x'): {
            if( m_inputHighlight ) {
                DeleteSelection();
            } else {
                DeleteCharacterIfValid( (int)m_currentInput.length() - m_cursorInversePosition );
            }
            return true;
        } case('$'): {
            m_cursorInversePosition = 0;
            return true;
        } case('0'): {
            m_cursorInversePosition = (int)m_currentInput.length();
            return true;
        }
    }
    return false;
}


bool ConsoleInterface::IsCharacterValid( unsigned char character ) const {
    //g_theDevConsole->PrintString( Stringf( "KeyCode pressed: 0x%2x", character ) );

    switch( character ) {
        case(0x0D): // Enter
        case(0x1B): // Escape
        case('\b'): // Backspace character
        case('`'): { // Tilde ~
            return false;
        }
    }

    return true;
}


void ConsoleInterface::ShowPrevHistoryCommand() {
    if( m_historyInverseIndex == 0 ) {
        m_temporaryInput = m_currentInput;
    }

    m_historyInverseIndex++;
    int numCommandsFound = 0;
    std::string commandText = "";

    std::scoped_lock< std::recursive_mutex > localLock( GetMutex() );
    DCLoggerInfo logInfo = GetLoggerInfo();
    std::vector<DevConsoleLine>::const_reverse_iterator lineIter = logInfo.lines.rbegin();

    for( lineIter; lineIter != logInfo.lines.rend() && numCommandsFound < m_historyMaxCommands; lineIter++ ) {
        // Is this line a command?
        if( (DevConsole::CHANNEL_COMMAND & lineIter->m_channel) == 0 ) {
            // No
            continue;
        } else {
            // Yes
            std::string newCommandText = lineIter->m_string.substr( 2 ); // Remove first two characters '> '

            if( newCommandText != commandText ) {
                numCommandsFound++;
                commandText = newCommandText;

                if( numCommandsFound == m_historyInverseIndex ) {
                    m_currentInput = commandText;
                    return;
                }
            }
        }
    }

    // Requested history index not found, keep with current one
    m_historyInverseIndex--;
    m_currentInput = commandText;
}


void ConsoleInterface::ShowNextHistoryCommand() {
    if( m_historyInverseIndex == 0 ) {
        // Can't see the future, no commands
        return;
    } else if( m_historyInverseIndex == 1 ) {
        // Looking for current input, pull it out of temp storage
        m_currentInput = m_temporaryInput;
        m_historyInverseIndex--;
        return;
    }

    m_historyInverseIndex--;
    int numCommandsFound = 0;
    std::string commandText = "";

    std::scoped_lock< std::recursive_mutex > localLock( GetMutex() );
    DCLoggerInfo logInfo = GetLoggerInfo();
    std::vector<DevConsoleLine>::const_reverse_iterator lineIter = logInfo.lines.rbegin();

    for( lineIter; lineIter != logInfo.lines.rend(); lineIter++ ) {
        // Is this line a command?
        if( (DevConsole::CHANNEL_COMMAND & lineIter->m_channel) == 0 ) {
            // No
            continue;
        } else {
            // Yes
            std::string newCommandText = lineIter->m_string.substr( 2 ); // Remove first two characters '> '

            // Removes duplicate entries in a row
            if( newCommandText != commandText ) {
                numCommandsFound++;
                commandText = newCommandText;

                if( numCommandsFound == m_historyInverseIndex ) {
                    m_currentInput = commandText;
                    return;
                }
            }
        }

    }

    // Requested history index not found, keep with current one
    m_historyInverseIndex++;
    m_currentInput = commandText;
}


void ConsoleInterface::MoveCursorRight() {
    m_cursorInversePosition = Clamp( --m_cursorInversePosition, 0, (int)m_currentInput.length() );
}


void ConsoleInterface::MoveCursorLeft() {
    m_cursorInversePosition = Clamp( ++m_cursorInversePosition, 0, (int)m_currentInput.length() );
}


void ConsoleInterface::MoveCursorStartOfNextWord() {
    int inputLength = (int)m_currentInput.length();
    int firstChar = inputLength - m_cursorInversePosition + 1;
    bool foundWhiteSpace = false;

    for( int charIndex = firstChar; charIndex < inputLength; charIndex++ ) {
        unsigned char letter = m_currentInput[charIndex];

        if( letter == ' ' || letter == '\t' ) {
            // Found White Space
            foundWhiteSpace = true;
        } else {
            // Found a Character
            if( foundWhiteSpace ) {
                // If a character was found after white space, it's time to stop!
                m_cursorInversePosition = inputLength - charIndex;
                return;
            }
        }
    }

    // No more whitespace found.. assume first character
    m_cursorInversePosition = 1;
}


void ConsoleInterface::MoveCursorStartOfPrevWord() {
    int inputLength = (int)m_currentInput.length();
    int firstChar = inputLength - m_cursorInversePosition - 1;
    bool foundCharacter = false;

    for( int charIndex = firstChar; charIndex >= 0; charIndex-- ) {
        unsigned char letter = m_currentInput[charIndex];

        if( letter == ' ' || letter == '\t' ) {
            // Found White Space
            if( foundCharacter ) {
                // If a character was found before white space, it's time to stop!
                m_cursorInversePosition = inputLength - charIndex - 1;
                return;
            }
        } else {
            // Found a Character
            foundCharacter = true;
        }
    }

    // No more whitespace found.. assume first character
    m_cursorInversePosition = inputLength;
}


void ConsoleInterface::MoveCursorEndOfNextWord() {
    int inputLength = (int)m_currentInput.length();
    int firstChar = inputLength - m_cursorInversePosition + 1;
    bool foundCharacter = false;

    for( int charIndex = firstChar; charIndex < inputLength; charIndex++ ) {
        unsigned char letter = m_currentInput[charIndex];

        if( letter == ' ' || letter == '\t' ) {
            // Found White Space
            if( foundCharacter ) {
                // If a character was found before white space, it's time to stop!
                m_cursorInversePosition = inputLength - charIndex + 1;
                return;
            }
        } else {
            // Found a Character
            foundCharacter = true;
        }
    }

    // No more whitespace found.. assume first character
    m_cursorInversePosition = 1;
}


void ConsoleInterface::DeleteCharacterIfValid( int characterIndex ) {
    int inputLength = (int)m_currentInput.length();

    if( characterIndex >= inputLength || characterIndex < 0 ) {
        return;
    }

    m_currentInput.erase( characterIndex, 1 );
}


void ConsoleInterface::DeleteSelection() {
    int cursorPosition = (int)m_currentInput.length() - m_cursorInversePosition;
    int startIndex = Min( m_highlightStartIndex, cursorPosition );
    int endIndex   = Max( m_highlightStartIndex, cursorPosition );
    int numCharacters = endIndex - startIndex + 1;

    m_currentInput.erase( startIndex, numCharacters );
    m_inputHighlight = false;

    if( cursorPosition == startIndex ) {
        m_cursorInversePosition -= numCharacters;
        m_cursorInversePosition = Clamp( m_cursorInversePosition, 0, (int)m_currentInput.length() );
    } else {
        MoveCursorRight();
    }
}


void ConsoleInterface::DeleteEntireLine() {
    m_currentInput = "";
    m_cursorInversePosition = 0;
}


void ConsoleInterface::ExecuteCommandLine( const std::string& commandString ) {
    Strings args = SplitStringOnDelimeter( commandString, ' ' );
    std::string command = args[0];
    command = StringToLower( command );

    if( command != "echo" ) {
        g_theDevConsole->PrintString( Stringf( "> %s", commandString.c_str() ), DevConsole::CHANNEL_COMMAND );
    }

    // Check Valid Commands
    Strings validCommands;
    g_theEventSystem->GetSubscribedEventsList( validCommands );

    bool commandFound = false;
    int numValidCommands = (int)validCommands.size();

    for( int commandIndex = 0; commandIndex < numValidCommands; commandIndex++ ) {
        if( command == validCommands[commandIndex] ) {
            commandFound = true;
            break;
        }
    }

    if( !commandFound ) {
        g_theDevConsole->PrintString( Stringf( "ERROR: Invalid Command (%s)", command.c_str() ), DevConsole::CHANNEL_ERROR );
        g_theDevConsole->PrintString( "     - Type HELP for a list of available commands", DevConsole::CHANNEL_ERROR );
        return;
    }

    // Parse Arguments
    EventArgs eventArgs = EventArgs();
    int numArgs = (int)args.size();

    for( int argIndex = 1; argIndex < numArgs; argIndex++ ) {
        std::string keyValueString = args[argIndex];
        Strings splitKeyValue = SplitStringOnDelimeter( keyValueString, '=' );
        int numParts = (int)splitKeyValue.size();

        if( numParts != 2 ) {
            g_theDevConsole->PrintString( Stringf( "ERROR: Invalid Argument Format (%s)", keyValueString.c_str() ), DevConsole::CHANNEL_ERROR );
            g_theDevConsole->PrintString( Stringf( "     - Usage Example: %s key1=value1 key2=value2", command.c_str() ), DevConsole::CHANNEL_ERROR );
            return;
        }

        eventArgs.SetValue( splitKeyValue[0], splitKeyValue[1] );
    }

    // Finally Fire the Event
    g_theEventSystem->FireEvent( command, eventArgs );
}


