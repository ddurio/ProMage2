#include "Engine/DevConsole/ConsoleLogger.hpp"

#include "Engine/DevConsole/DevConsole.hpp"

#include "stdarg.h"


DevConsoleChannel ConsoleLogger::CHANNEL_UNDEFINED;
DevConsoleChannel ConsoleLogger::CHANNEL_COMMAND;
DevConsoleChannel ConsoleLogger::CHANNEL_INFO;
DevConsoleChannel ConsoleLogger::CHANNEL_WARNING;
DevConsoleChannel ConsoleLogger::CHANNEL_ERROR;


void ConsoleLogger::Startup( bool fullStartup /*= true */ ) {
    if( !fullStartup ) {
        return;
    }

    ConsoleLogger::CHANNEL_COMMAND  = AddChannel( "command",    Rgba::CYAN      );
    ConsoleLogger::CHANNEL_INFO     = AddChannel( "info",       Rgba::WHITE     );
    ConsoleLogger::CHANNEL_WARNING  = AddChannel( "warning",    Rgba::YELLOW    );
    ConsoleLogger::CHANNEL_ERROR    = AddChannel( "error",      Rgba::RED       );
}


void ConsoleLogger::Shutdown() {

}


void ConsoleLogger::BeginFrame() {
    m_frameNumber++;
}


void ConsoleLogger::EndFrame() {

}


void ConsoleLogger::PrintString( std::string inputString, DevConsoleChannel inputChannel /*= CHANNEL_INFO*/, const Rgba& inputColor /*= Rgba::CLEAR */ ) {
    std::scoped_lock< std::recursive_mutex > localLock( GetMutex() );

    Rgba messageColor = inputColor;

    if( messageColor == Rgba::CLEAR ) { // Pick from channel
        ChannelDetails details = m_channels[inputChannel];
        messageColor = details.color;
    }

    m_lines.emplace_back( inputString, m_frameNumber, messageColor, inputChannel );

    if( (m_activeChannels & inputChannel) != 0 ) {
        m_unfilteredLines++;
    }
}


void ConsoleLogger::PrintString( std::string inputString, const std::string& inputChannelName, const Rgba& inputColor /*= Rgba::CLEAR */ ) {
    DevConsoleChannel channel = GetChannelFromString( inputChannelName );
    PrintString( inputString, channel, inputColor );
}


DevConsoleChannel ConsoleLogger::GetChannelFromString( const std::string& inputString ) const {
    if( inputString == "" ) {
        return CHANNEL_UNDEFINED;
    }

    ChannelDetails details = m_channels[inputString];
    return details.bitMask;

/* What use case is there for converting a string to unsigned long?
        DevConsoleChannel integerChannel = strtoul( inputString.c_str(), NULL, 0 );

        if( integerChannel != 0 ) {
            channel = integerChannel;
        } else {
            channel = CHANNEL_INFO;
        }
*/
}


DevConsoleChannel ConsoleLogger::AddChannel( const std::string& channelName, const Rgba& channelColor, bool enableChannel /*= true */ ) {
    // Already hit max channels?
    int numChannels = (int)m_channels->size();

    if( numChannels >= MAX_CHANNELS ) {
        std::string errorMsg = Stringf( "Error: Max number of channels (%d) already exist. Not adding new channel (%s)", MAX_CHANNELS, channelName.c_str() );
        PrintString( errorMsg.c_str(), CHANNEL_ERROR );
        return CHANNEL_UNDEFINED;
    }


    // Does channel already exist by that name?
    ChannelDetails details = m_channels[channelName];

    if( details.name != "" ) {
        return details.bitMask;
    }


    // Passes all the checks... add to the registry
    details.name = channelName;
    details.color = channelColor;
    details.bitMask = BIT_FLAG( numChannels );
    details.priority = GetPriority( details.name );

    m_channels->push( details );

    if( enableChannel ) {
        EnableChannel( details.bitMask );
    }

    return details.bitMask;
}


void ConsoleLogger::SwitchToChannel( DevConsoleChannel channel ) {
    m_activeChannels = channel;
    CountUnfilteredLines();
}


void ConsoleLogger::SwitchToChannel( const std::string& channelName ) {
    DevConsoleChannel channel = GetChannelFromString( channelName );
    SwitchToChannel( channel );
}


DCLoggerInfo ConsoleLogger::GetLoggerInfo() const {
    DCLoggerInfo info( m_lines );

    info.numUnfilteredLines = m_unfilteredLines;
    info.activeChannels = m_activeChannels;

    return info;
}


// PRIVATE ------------------------------------------------------------
bool ConsoleLogger::ChannelPriorityCmp::operator()( const ChannelDetails& detailsA, const ChannelDetails& detailsB ) const {
    return (detailsA.priority > detailsB.priority);
}


ConsoleLogger::ChannelQueue* ConsoleLogger::ChannelList::operator->() {
    return &channelQueue;
}


const ConsoleLogger::ChannelQueue* ConsoleLogger::ChannelList::operator->() const {
    return &channelQueue;
}


ConsoleLogger::ChannelDetails ConsoleLogger::ChannelList::operator[]( const std::string& channelName ) const {
    ChannelQueue channels = channelQueue;

    while( !channels.empty() ) {
        if( StringICmp( channels.top().name, channelName ) ) {
            return channels.top();
        }

        channels.pop();
    }

    return ChannelDetails();
}


ConsoleLogger::ChannelDetails ConsoleLogger::ChannelList::operator[]( const DevConsoleChannel& channelMask ) const {
    ChannelQueue channels = channelQueue;

    while( !channels.empty() ) {
        const ChannelDetails& details = channels.top();

        if( (details.bitMask & channelMask) != 0 ) {
            return details;
        }

        channels.pop();
    }

    return ChannelDetails();
}


int ConsoleLogger::GetPriority( const std::string& channelName ) const {
    if( StringICmp( channelName, "Error" ) ) {
        return 0;
    } else if( StringICmp( channelName, "Warning" ) ) {
        return 1;
    } else if( StringICmp( channelName, "MemTracking" ) ) { // Special because it's done very first
        return 2;
    } else if( StringICmp( channelName, "Command" ) ) {
        return MAX_CHANNELS - 2;
    } else if( StringICmp( channelName, "Info" ) ) {
        return MAX_CHANNELS - 1;
    } else {
        int numChannels = (int)m_channels->size();
        return numChannels - 2;
    }
}


void ConsoleLogger::UpdateActiveChannels( int numChannels, ... ) {
    Strings channels;
    va_list inputChannels;
    va_start( inputChannels, numChannels );

    for( int inputIndex = 0; inputIndex < numChannels; inputIndex++ ) {
        const std::string& inputName = va_arg( inputChannels, std::string );
        channels.push_back( inputName );
    }

    va_end( inputChannels );

    UpdateActiveChannels( channels );
}


void ConsoleLogger::UpdateActiveChannels( const std::string& channelNamesCSV ) {
    Strings channels = SplitStringOnDelimeter( channelNamesCSV, ',' );
    UpdateActiveChannels( channels );
}


void ConsoleLogger::UpdateActiveChannels( const Strings& channels ) {
    int numChannels = (int)channels.size();

    for( int inputIndex = 0; inputIndex < numChannels; inputIndex++ ) {
        std::string channelName = channels[inputIndex];
        bool clearChannel = false;

        if( channelName[0] == '!' ) {
            clearChannel = true;
            channelName = channelName.substr( 1 );
        }

        ChannelDetails details = m_channels[channelName];

        if( details.name != "" ) { // Found channel
            if( clearChannel ) {
                DisableChannel( details.bitMask );
            } else {
                EnableChannel( details.bitMask );
            }
        } else { // Need to create channel
            const char* enableText = clearChannel ? "" : " and enabling";
            std::string warningMsg = Stringf( "Warning: Specified channel (%s) does not exist.  Adding%s it for you.", channelName.c_str(), enableText );
            PrintString( warningMsg.c_str(), CHANNEL_WARNING );

            AddChannel( channelName, Rgba::WHITE, !clearChannel );
        }
    }

    CountUnfilteredLines();
}


void ConsoleLogger::EnableChannel( DevConsoleChannel channelBitmask ) {
    m_activeChannels |= channelBitmask;

    CountUnfilteredLines();
}


void ConsoleLogger::DisableChannel( DevConsoleChannel channelBitmask ) {
    m_activeChannels &= ~channelBitmask;

    CountUnfilteredLines();
}


void ConsoleLogger::CountUnfilteredLines() {
    m_unfilteredLines = 0;
    int numLines = (int)m_lines.size();

    for( int lineIndex = 0; lineIndex < numLines; lineIndex++ ) {
        if( (m_activeChannels & m_lines[lineIndex].m_channel) != 0 ) {
            m_unfilteredLines++;
        }

    }
}
