#pragma once
#include "Game/GameCommon.hpp"


class Editor;
extern Editor* g_theEditor;


constexpr char  EVENT_EDITOR_STEP_INDEX[]   = "setStepIndex";
constexpr char  EVENT_EDITOR_GENERATE_MAP[] = "generateMap";
