#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/DevConsole/DevConsoleLine.hpp"

#include "mutex"
#include "queue"


struct DCLoggerInfo;


class ConsoleLogger {
    public:
    virtual void Startup( bool fullStartup = true );
    virtual void Shutdown();

    virtual void BeginFrame();
    virtual void EndFrame();

    void PrintString( std::string inputString, DevConsoleChannel inputChannel = CHANNEL_INFO, const Rgba& inputColor = Rgba::CLEAR );
    void PrintString( std::string inputString, const std::string& inputChannelName, const Rgba& inputColor = Rgba::CLEAR );

    DevConsoleChannel GetChannelFromString( const std::string& inputString ) const;

    DevConsoleChannel AddChannel( const std::string& inputName, const Rgba& color, bool enableChannel = true );
    void SwitchToChannel( DevConsoleChannel channel );
    void SwitchToChannel( const std::string& channelName );
    void UpdateActiveChannels( int numChannels, ... );
    void UpdateActiveChannels( const std::string& channelNamesCSV );


    static DevConsoleChannel CHANNEL_UNDEFINED;
    static DevConsoleChannel CHANNEL_COMMAND;
    static DevConsoleChannel CHANNEL_INFO;
    static DevConsoleChannel CHANNEL_WARNING;
    static DevConsoleChannel CHANNEL_ERROR;


    protected:
    virtual ~ConsoleLogger() {};

    DCLoggerInfo GetLoggerInfo() const;


    private:
    struct ChannelDetails {
        std::string name            = "";
        DevConsoleChannel bitMask   = CHANNEL_UNDEFINED;
        Rgba color                  = Rgba::WHITE;
        int priority                = 0;
    };

    struct ChannelPriorityCmp {
        bool operator()( const ChannelDetails& detailsA, const ChannelDetails& detailsB ) const;
    };


    typedef std::priority_queue< ChannelDetails, std::vector< ChannelDetails >, ChannelPriorityCmp > ChannelQueue;

    struct ChannelList {
        ChannelQueue channelQueue;

        ChannelQueue* operator->();
        const ChannelQueue* operator->() const;
        ChannelDetails operator[]( const std::string& channelName ) const;
        ChannelDetails operator[]( const DevConsoleChannel& channelMask ) const;
    };


    static constexpr int MAX_CHANNELS = sizeof( DevConsoleChannel ) * 8; // DCC acting as bitFlag, can't have more channels than would fit in this bitFlag

    int m_frameNumber = 0;
    int m_unfilteredLines = 0;

    std::vector<DevConsoleLine> m_lines;

    DevConsoleChannel m_activeChannels = 0x00;
    ChannelList m_channels;


    virtual std::recursive_mutex& GetMutex() const = 0;

    int GetPriority( const std::string& channelName ) const;
    void UpdateActiveChannels( const Strings& channels );
    void EnableChannel( DevConsoleChannel channelBitmask );
    void DisableChannel( DevConsoleChannel channelBitmask );
    void CountUnfilteredLines();
};
