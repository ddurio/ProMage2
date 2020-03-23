#include "Engine/DevConsole/DevConsole.hpp"


DevConsole* g_theDevConsole = new DevConsole();


void DevConsole::Startup( bool fullStartup /*= true */ ) {
    ConsoleInterface::Startup( fullStartup );
    ConsoleLogger::Startup( fullStartup );
    ConsoleRenderer::Startup( fullStartup );
}


void DevConsole::Shutdown() {
    ConsoleInterface::Shutdown();
    ConsoleLogger::Shutdown();
    ConsoleRenderer::Shutdown();
}


DCInterfaceInfo DevConsole::GetInterfaceInfo() const {
    return ConsoleInterface::GetInterfaceInfo();
}


// Must lock DevConsole::m_mutex if using info.lines
DCLoggerInfo DevConsole::GetLoggerInfo() const {
    return ConsoleLogger::GetLoggerInfo();
}


DCRenderInfo DevConsole::GetRenderInfo() const {
    return ConsoleRenderer::GetRenderInfo();
}


std::recursive_mutex& DevConsole::GetMutex() const {
    return m_mutex;
}
