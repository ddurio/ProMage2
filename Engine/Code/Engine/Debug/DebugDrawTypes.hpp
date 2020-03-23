#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


enum DrawSpace {
    DRAW_SPACE_CAMERA,
    DRAW_SPACE_SCREEN,
    DRAW_SPACE_WORLD
};


enum DrawDepthMode {
    DRAW_DEPTH_MODE_DEPTH,
    DRAW_DEPTH_MODE_ALWAYS,
    DRAW_DEPTH_MOD_XRAY
};


struct DebugDrawOptions {
    // Space
    DrawSpace coordinateSpace = DRAW_SPACE_WORLD;
    DrawDepthMode  depthMode  = DRAW_DEPTH_MODE_DEPTH;
    FillMode fillMode = FILL_MODE_SOLID;
    bool isBillboarded = false;

    // Position
    Matrix44 modelMatrix = Matrix44::IDENTITY;

    // Style
    float durationSeconds = 0.f;
    float remainingSeconds = 0.f;

    float size = s_defaultPointSize;
    float length = s_defaultPointSize;
    float depth = 0.f;

    Rgba colorStart = Rgba::WHITE;
    Rgba colorEnd = Rgba::WHITE;
    std::string texturePath = "";


    static const float s_defaultPointSize;
    static const float s_defaultLineSize;
};
