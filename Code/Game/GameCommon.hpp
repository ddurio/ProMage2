#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/Keyboard.hpp"


class App;
class AudioSystem;
class DevConsole;
class Entity;
class Game;
class InputSystem;
class RenderContext;
class RNG;
class WindowContext;

extern App* g_theApp;
extern AudioSystem* g_theAudio;
extern DevConsole* g_theDevConsole;
extern Game* g_theGame;
extern InputSystem* g_theInput;
extern RenderContext* g_theRenderer;
extern RNG* g_RNG;
extern WindowContext* g_theWindow;

typedef std::vector<Entity*> EntityList;


constexpr int APP_MIN_FPS = 10;
constexpr char  APP_TITLE[] = "Adventure (ddurio)";
constexpr float APP_MAX_DELTA_SECONDS = 1.f / (float)APP_MIN_FPS;

constexpr float GAME_ATTRACT_AUDIO_DELAY = 3.f;
constexpr float GAME_END_SCREEN_TIME_SECONDS = 3.f;
constexpr int GAME_NUM_MAPS = 3;

constexpr char  TEXTURE_TERRAIN[] = "Data/Images/Terrain_32x32.png";

constexpr char  FONT_NAME_SQUIRREL[] = "SquirrelFixedFont";

constexpr char  DATA_ACTOR_DEFS[] = "Data/Gameplay/ActorDefs.xml";
constexpr char  DATA_ITEM_DEFS[] = "Data/Gameplay/ItemDefs.xml";
constexpr char  DATA_MAP_DEFS[] = "Data/Gameplay/MapDefs.xml";
constexpr char  DATA_PROJECT_CONFIG[] = "Data/ProjectConfig.xml";
constexpr char  DATA_TILE_DEFS[] = "Data/Gameplay/TileDefs.xml";

constexpr float CLIENT_ASPECT = (16.f / 9.f);
constexpr float CLIENT_MAX_SCREEN_PERCENT = 0.9f;
