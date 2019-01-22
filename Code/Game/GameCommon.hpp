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

class Entity;
typedef std::vector<Entity*> EntityList;

constexpr int APP_MIN_FPS = 10;
constexpr float APP_MAX_DELTA_SECONDS = 1.f / (float)APP_MIN_FPS;

constexpr float GAME_ATTRACT_AUDIO_DELAY = 3.f;
constexpr float GAME_END_SCREEN_TIME_SECONDS = 3.f;
constexpr int GAME_NUM_MAPS = 3;

//constexpr char  AUDIO_ANTICIPATION[] = "Data/Audio/Anticipation.mp3";

constexpr char  TEXTURE_STBI_TEST[] = "Data/Images/Test_StbiFlippedAndOpenGL.png";
constexpr char  TEXTURE_ANIMATION_TEST[] = "Data/Images/Test_SpriteSheet8x2.png";

constexpr char  FONT_NAME_SQUIRREL[] = "SquirrelFixedFont";

constexpr char  DATA_PROJECT_CONFIG[] = "Data/ProjectConfig.xml";
constexpr char  DATA_XML_TEST[] = "Data/Gameplay/XMLTest.xml";

constexpr int   MAP_WIDTH = 16;
constexpr int   MAP_HEIGHT = 30;
constexpr float MAP_STONE_TILES_FRACTION = 0.2f;
constexpr int   MAP_STARTING_SAFE_ZONE_SIZE_X = 5;
constexpr int   MAP_STARTING_SAFE_ZONE_SIZE_Y = 5;
constexpr int   MAP_RAYCAST_NUM_SAMPLES = 100;
constexpr float MAP_RAYCAST_MAX_DISTANCE = 10.f;

constexpr float CLIENT_ASPECT = (16.f / 9.f);
constexpr float CAMERA_PLAYER_HEIGHT = 7.f;
constexpr float CAMERA_PLAYER_WIDTH = CLIENT_ASPECT * CAMERA_PLAYER_HEIGHT;
constexpr float CAMERA_DEBUG_HEIGHT = (float)MAP_HEIGHT;
constexpr float CAMERA_DEBUG_WIDTH = CLIENT_ASPECT * CAMERA_DEBUG_HEIGHT;
constexpr float CAMERA_SHAKE_MAX = 2.f;
constexpr float CAMERA_SHAKE_REDUCTION_PER_SECOND = 1.f;

constexpr float ATTRACT_FADE_RATE_PER_SECOND = 0.4f;

constexpr float PLAYERTANK_PHYSICS_RADIUS = 0.32f;
constexpr float PLAYERTANK_COSMETIC_RADIUS = 0.64f;
constexpr float PLAYERTANK_COSMETIC_BOX_OFFSET = 0.55f;
constexpr float PLAYERTANK_MAX_SPEED = 1.3f;
constexpr float PLAYERTANK_TURN_SPEED = 160.f;
constexpr float PLAYERTANK_TOP_TURN_SPEED = 225.f;
constexpr float PLAYERTANK_RATE_OF_FIRE_SECONDS = 0.3f;
constexpr int   PLAYERTANK_MAX_HEALTH = 5;
constexpr int   PLAYERTANK_EXTRA_LIVES = 3;


constexpr float ENEMYTURRET_PHYSICS_RADIUS = 0.32f;
constexpr float ENEMYTURRET_COSMETIC_RADIUS = 0.64f;
constexpr float ENEMYTURRET_COSMETIC_BOX_OFFSET = 0.55f;
constexpr float ENEMYTURRET_TOP_TURN_SPEED = 15.f;
constexpr float ENEMYTURRET_MAX_SIGHT_RANGE = MAP_RAYCAST_MAX_DISTANCE;
constexpr float ENEMYTURRET_RATE_OF_FIRE_SECONDS = 1.3f;
constexpr float ENEMYTURRET_SCAN_TIME_SECONDS = 10.f;
constexpr float ENEMYTURRET_SCAN_ANGLE = 25.f;
constexpr int   ENEMYTURRET_MAX_HEALTH = 2;

constexpr float ENEMYTANK_PHYSICS_RADIUS = 0.32f;
constexpr float ENEMYTANK_COSMETIC_RADIUS = 0.64f;
constexpr float ENEMYTANK_COSMETIC_BOX_OFFSET = 0.55f;
constexpr float ENEMYTANK_MAX_SPEED = 1.f;
constexpr float ENEMYTANK_TURN_SPEED = 100.f;
constexpr float ENEMYTANK_TOP_TURN_SPEED = 100.f;
constexpr float ENEMYTANK_MAX_SIGHT_RANGE = MAP_RAYCAST_MAX_DISTANCE;
constexpr float ENEMYTANK_RATE_OF_FIRE_SECONDS = 1.7f;
constexpr float ENEMYTANK_WHISKER_RANGE = 1.f;
constexpr float ENEMYTANK_WHISKER_ANGLE = 25.f;
constexpr int   ENEMYTANK_MAX_HEALTH = 2;

constexpr float BULLET_PHYSICS_RADIUS = 0.1f;
constexpr float BULLET_COSMETIC_RADIUS = 0.25f;
constexpr float BULLET_COSMETIC_BOX_OFFSET = .1f;
constexpr float BULLET_MAX_SPEED = 5.f;
constexpr int   BULLET_LIFETIME_BOUNCES = 3;

constexpr int   BOULDER_SPRITE_INDEX = 3;
constexpr float BOULDER_PHYSICS_RADIUS = 0.45f;
constexpr float BOULDER_COSMETIC_RADIUS = 0.55f;
constexpr float BOULDER_COSMETIC_BOX_OFFSET = 0.5f;

constexpr float EXPLOSION_DURATION = 1.0f;
constexpr float EXPLOSION_SCALE_SMALL = 0.25f;
constexpr float EXPLOSION_SCALE_LARGE = 1.f;
