#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/Keyboard.hpp"


class App;
class AudioSystem;
class DevConsole;
class Entity;
class Game;
class InputSystem;
class PhysicsSystem;
class RenderContext;
class RNG;
class WindowContext;

extern App*             g_theApp;
extern AudioSystem*     g_theAudio;
extern DevConsole*      g_theDevConsole;
extern Game*            g_theGame;
extern InputSystem*     g_theInput;
extern PhysicsSystem*   g_thePhysicsSystem;
extern RenderContext*   g_theRenderer;
extern RNG*             g_RNG;
extern WindowContext*   g_theWindow;

typedef std::vector<Entity*> EntityList;


constexpr int   APP_MIN_FPS = 10;
constexpr char  APP_TITLE[] = "Adventure (ddurio)";
constexpr float APP_MAX_DELTA_SECONDS = 1.f / (float)APP_MIN_FPS;

constexpr char  TEXTURE_GOLD_PILE[]         = "Data/Images/GoldPile.png";
constexpr char  TEXTURE_TERRAIN[]           = "Data/Images/Terrain_32x32.png";

constexpr char  FONT_NAME_SQUIRREL[]        = "SquirrelFixedFont";
constexpr char  MAT_NAME_PAPER_DOLL[]       = "PaperDoll";

constexpr char  DATA_ACTOR_DEFS[]           = "Data/Gameplay/ActorDefs.xml";
constexpr char  DATA_INVENTORY_SPRITES[]    = "Data/Gameplay/Inventory.xml";
constexpr char  DATA_ITEM_DEFS[]            = "Data/Gameplay/ItemDefs.xml";
constexpr char  DATA_MAP_DEFS[]             = "Data/Gameplay/MapDefs.xml";
constexpr char  DATA_PAPER_DOLL_ANIMS[]     = "Data/Gameplay/PaperDoll.SpriteAnims.xml";
constexpr char  DATA_PAPER_DOLL_ISO_ANIMS[] = "Data/Gameplay/PaperDoll.IsoAnims.xml";
constexpr char  DATA_PAPER_DOLL_SPRITES[]   = "Data/Gameplay/PaperDoll.SpriteSheets.xml";
constexpr char  DATA_PROJECT_CONFIG[]       = "Data/ProjectConfig.xml";
constexpr char  DATA_TILE_DEFS[]            = "Data/Gameplay/TileDefs.xml";

constexpr char  ANIM_INVENTORY_CHEST[]      = "Inventory.Chest";
constexpr char  ANIM_INVENTORY_EMPTY[]      = "Inventory.Empty";
constexpr char  ANIM_INVENTORY_FEET[]       = "Inventory.Feet";
constexpr char  ANIM_INVENTORY_HELM[]       = "Inventory.Helm";
constexpr char  ANIM_INVENTORY_LEGS[]       = "Inventory.Legs";
constexpr char  ANIM_INVENTORY_SHOULDER[]   = "Inventory.Shoulder";
constexpr char  ANIM_INVENTORY_WEAPON[]     = "Inventory.Weapon";

constexpr char  ANIM_PAPER_DOLL_DIE[]       = "PaperDoll.Die";
constexpr char  ANIM_PAPER_DOLL_IDLE[]      = "PaperDoll.Idle";
constexpr char  ANIM_PAPER_DOLL_SHOOT[]     = "PaperDoll.Shoot";
constexpr char  ANIM_PAPER_DOLL_SLASH[]     = "PaperDoll.Slash";
constexpr char  ANIM_PAPER_DOLL_SPELL[]     = "PaperDoll.Spell";
constexpr char  ANIM_PAPER_DOLL_THRUST[]    = "PaperDoll.Thrust";
constexpr char  ANIM_PAPER_DOLL_WALK[]      = "PaperDoll.Walk";

constexpr float CLIENT_ASPECT = (16.f / 9.f);
constexpr float CLIENT_MAX_SCREEN_PERCENT = 0.9f;

constexpr char  TAG_STAIRS_UP[]             = "stairUpAccess";
constexpr char  TAG_STAIRS_DOWN[]           = "stairDownAccess";
constexpr char  TAG_MERCHANT[]              = "merchantAccess";
