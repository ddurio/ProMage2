#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/DevConsole/ConsoleInterface.hpp"
#include "Engine/DevConsole/ConsoleLogger.hpp"
#include "Engine/DevConsole/ConsoleRenderer.hpp"

#include "mutex"


struct DCInterfaceInfo {
    std::string inputStr = "";

    int numLinesScroll = 0;
    int cursorInversePosition = 0;
    int highlightStartIndex = 0;

    bool isHighlighting = false;
    bool isVimMode = false;
};


struct DCLoggerInfo {
    const std::vector<DevConsoleLine>& lines;
    int numUnfilteredLines = 0;

    DevConsoleChannel activeChannels = 0x00;

    DCLoggerInfo( const std::vector<DevConsoleLine>& linesIn ) : lines( linesIn ) {};
};


struct DCRenderInfo {
    DevConsoleMode mode = DEV_CONSOLE_OFF;
    int maxLinesPerScreen = 0;
};


class DevConsole : public ConsoleInterface, virtual public ConsoleLogger, virtual public ConsoleRenderer {
    public:
    DevConsole() {};

    void Startup( bool fullStartup = true ) override;
    void Shutdown() override;


    private:
    mutable std::recursive_mutex m_mutex;


    ~DevConsole() {};

    DCInterfaceInfo GetInterfaceInfo() const override;
    DCLoggerInfo GetLoggerInfo() const override;
    DCRenderInfo GetRenderInfo() const override;

    std::recursive_mutex& GetMutex() const override;
};
