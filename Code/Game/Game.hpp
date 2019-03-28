#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexPCU.hpp"

#include "Game/Entity.hpp"


struct AABB2;
class Camera;
class CameraController;

class Game {
	public:
	Game();
	~Game();

	void Startup();
	void Shutdown();

	void Update( float deltaSeconds );
	void Render() const;
    bool IsDebugDrawingOn() const;
    void SetCameraShakeAmount( float newCameraShakeAmount );

	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool HandleQuitRequested();

    void ReturnToAttractScreen();

    Camera* GetActiveCamera() const;

    bool HasGameBeenBeaten() const;

    static bool Command_SetAmbientLight( EventArgs& args );


	private:
    static const int m_numDesktops = 3;
    Vec2 m_desktopDimensions[m_numDesktops] = {
        Vec2( 200.f, 100.f ),
        Vec2( 200.f, 100.f ),
        Vec2( 200.f, 100.f)
    };

    int m_frameNumber = -1;
    bool m_debugDrawing = true;
    float m_cameraShakeAmount = 0.f;

    bool m_onAttractScreen = false;
    bool m_hasBeatenTheGame = false;

    bool m_useDebugCamera = false;
    int m_activeDesktop = 0;
    Camera* m_playerCamera = nullptr;
    Camera* m_debugCamera = nullptr;
    CameraController* m_cameraPos = nullptr;

    std::vector<VertexPCU> m_xmlVerts;
    int m_channelIndex = 0;
    Rgba m_ambientColor = Rgba( 1.f, 1.f, 1.f, 0.5f ); // For changing ambient light (remove for new game)
    bool m_renderNormals = false; // RFNG
    int m_numDynamicLights = 4; // RFNG

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
    void RenderTexture( int desktopID ) const;
    void RenderSpriteAnimations( int desktopID ) const;
    void RenderAdditiveVenn( int desktopID ) const;
    void RenderTextInBox( int desktopID ) const;
    void RenderTextAlignment( int desktopID ) const;
    void RenderTextDrawMode( int desktopID ) const;
    void RenderTextMultiLine( int desktopID ) const;
    void RenderXML( int desktopID ) const;
    void RenderEntityArray( const Entity** entityArray, int numEntities ) const;

    const Vec2 GetDesktopOffset( int desktopID ) const;
    const AABB2 GetDesktopBounds( int desktopID ) const;
    void TransformVertexArrayToDesktop( int desktopID, int numArrays, ... ) const;
    void CheckCollisionBetweenEntityArrays( Entity** entity1Array, int maxEntity1, Entity** entity2Array, int maxEntity2);

    void DestroyEntity( Entity** entityArray, int entityIndex );
    void CollectGarbage();
};
