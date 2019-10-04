#pragma once
#include "Game/GameCommon.hpp"


class Editor;
extern Editor* g_theEditor;


constexpr char  EVENT_EDITOR_CHANGE_STEP[]   = "setStepIndex";
constexpr char  EVENT_EDITOR_GENERATE_MAP[] = "generateMap";
