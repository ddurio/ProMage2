#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "vector"


class App;
extern App* g_theApp;

class AudioSystem;
extern AudioSystem* g_theAudio;

class DevConsole;
extern DevConsole* g_theDevConsole;

class Game;
extern Game* g_theGame;

class InputSystem;
extern InputSystem* g_theInput;

class RenderContext;
extern RenderContext* g_theRenderer;

class RNG;
extern RNG* g_RNG;

class WindowContext;
extern WindowContext* g_theWindow;

class Entity;
typedef std::vector<Entity*> EntityList;

constexpr int APP_MIN_FPS = 10;
constexpr char  APP_TITLE[] = "Protogame3D (ddurio)";
constexpr float APP_MAX_DELTA_SECONDS = 1.f / (float)APP_MIN_FPS;

constexpr float GAME_ATTRACT_AUDIO_DELAY = 3.f;
constexpr float GAME_END_SCREEN_TIME_SECONDS = 3.f;
constexpr int GAME_NUM_MAPS = 3;

constexpr char  TEXTURE_ANIMATION_TEST[] = "Data/Images/Test_SpriteSheet8x2.png";
constexpr char  TEXTURE_EXPLOSION[] = "Data/Images/Explosion_5x5.png";
constexpr char  TEXTURE_STBI_TEST[] = "Data/Images/Test_StbiFlippedAndOpenGL.png";
constexpr char  TEXTURE_TERRAIN[] = "Data/Images/Terrain_32x32.png";

constexpr char  FONT_NAME_SQUIRREL[] = "SquirrelFixedFont";

constexpr char  DATA_ACTOR_DEFS[] = "Data/Gameplay/ActorDefs.xml";
constexpr char  DATA_ITEM_DEFS[] = "Data/Gameplay/ItemDefs.xml";
constexpr char  DATA_MAP_DEFS[] = "Data/Gameplay/MapDefs.xml";
constexpr char  DATA_PROJECT_CONFIG[] = "Data/ProjectConfig.xml";
constexpr char  DATA_TILE_DEFS[] = "Data/Gameplay/TileDefs.xml";

constexpr float CLIENT_ASPECT = (16.f / 9.f);
constexpr float CLIENT_MAX_SCREEN_PERCENT = 0.9f;
