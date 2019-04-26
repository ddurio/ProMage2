#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexPCU.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Game/Entity.hpp"


class Camera;
class CameraController;
class Material;

struct AABB2;


enum RenderMode {
    RENDER_MODE_DEFAULT,
    RENDER_MODE_NORMALS,
    RENDER_MODE_SURFACE_NORMALS,
    RENDER_MODE_TANGENTS,
    RENDER_MODE_BITANGENTS,

    NUM_RENDER_MODES
};


class Game {
	public:
	Game();
	~Game();

	void Startup();
	void Shutdown();

	void Update();
	void Render() const;
    bool IsDebugDrawingOn() const;
    void SetCameraShakeAmount( float newCameraShakeAmount );

	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool HandleQuitRequested();
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    void ReturnToAttractScreen();

    Camera* GetActiveCamera() const;
    Clock* GetGameClock() const;

    bool HasGameBeenBeaten() const;

    static bool Command_SetAmbientLight( EventArgs& args );
    static bool Command_SetEmissiveLight( EventArgs& args );
    static bool Command_SetDirectionalLight( EventArgs& args );
    static bool Command_SetPointLights( EventArgs& args );


    private:
    Clock* m_gameClock = nullptr;
    bool m_isPaused = false;

    bool m_debugDrawing = true;
    float m_cameraShakeAmount = 0.f;

    bool m_onAttractScreen = false;
    bool m_hasBeatenTheGame = false;

    bool m_useDebugCamera = false;
    int m_activeDesktop = 0;
    Camera* m_playerCamera = nullptr;
    Camera* m_debugCamera = nullptr;
    CameraController* m_controller = nullptr;

    std::vector<VertexPCU> m_xmlVerts;
    int m_channelIndex = 0;
    Rgba m_ambientColor = Rgba( 1.f, 1.f, 1.f, 0.5f ); // For changing ambient light (remove for new game)
    Rgba m_emissiveColor = Rgba( 0.f, 0.f, 0.f, 0.f ); //RFNG
    RenderMode m_renderMode = RENDER_MODE_DEFAULT; // RFNG
    int m_numDynamicLights = 4; // RFNG
    bool m_forcePosition = false;
    std::vector<Material*> m_materials; // RFNG

    void StartupAttract();
    void StartupGame();
    void StartupParseXMLTests();
    void StartupConsoleTests();
    void StartupEventTests();
    void StartupLights();

    void UpdateAttractScreen( float deltaSeconds );
    void UpdateGame( float deltaSeconds );
    void UpdateEntityArray( Entity** entityArray, int numEntities, float deltaSeconds );
    void UpdateFromController( float deltaSeconds );
    void UpdateCameraShake( float deltaSeconds );
    void UpdateConsoleChannels( float deltaSeconds );

    void RenderAttractScreen() const;
    void RenderGame() const;
    void RenderDebugDraw() const;
    void RenderEntityArray( const Entity** entityArray, int numEntities ) const;

    void CheckCollisionBetweenEntityArrays( Entity** entity1Array, int maxEntity1, Entity** entity2Array, int maxEntity2);

    void DestroyEntity( Entity** entityArray, int entityIndex );
    void CollectGarbage();
};
