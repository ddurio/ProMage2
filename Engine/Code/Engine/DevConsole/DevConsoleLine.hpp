#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"


struct DevConsoleLine {
    public:
    std::string m_string = "";
    Rgba m_color = Rgba::WHITE;
    DevConsoleChannel m_channel = 0x00;

    double m_printTime = 0;
    int m_frameNumber = 0;

    DevConsoleLine( std::string inputString, int frameNumber, Rgba inputColor = Rgba::WHITE, DevConsoleChannel inputChannel = 0x00 );
};
