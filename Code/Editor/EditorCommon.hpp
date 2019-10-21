#pragma once
#include "Game/GameCommon.hpp"


class Editor;
extern Editor* g_theEditor;


constexpr char  EVENT_EDITOR_CHANGE_STEP[]   = "setStepIndex";
constexpr char  EVENT_EDITOR_GENERATE_MAP[]  = "generateMap";
constexpr char  EVENT_EDITOR_MOTIF_CHANGED[] = "motifVarChanged";

constexpr char  TEXTURE_EDITOR_MEDIA_START[]    = "Data/Images/Media/SkipPrevious.png";
constexpr char  TEXTURE_EDITOR_MEDIA_END[]      = "Data/Images/Media/SkipNext.png";
constexpr char  TEXTURE_EDITOR_MEDIA_PLAY[]     = "Data/Images/Media/Play.png";
constexpr char  TEXTURE_EDITOR_MEDIA_PAUSE[]    = "Data/Images/Media/Pause.png";
constexpr char  TEXTURE_EDITOR_MEDIA_LOOP[]     = "Data/Images/Media/Sync.png";
