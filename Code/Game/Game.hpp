#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


struct AABB2;
class Camera;

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


	private:
    static const int m_numDesktops = 3;
    Vec2 m_desktopDimensions[m_numDesktops] = {
        Vec2( 200.f, 100.f ),
        Vec2( 200.f, 100.f ),
        Vec2( 200.f, 100.f)
    };

    int m_frameNumber = -1;
    bool m_debugDrawing = false;
    float m_cameraShakeAmount = 0.f;

    bool m_onAttractScreen = false;
    bool m_hasBeatenTheGame = false;

    bool m_useDebugCamera = false;
    int m_activeDesktop = 0;
    Camera* m_playerCamera = nullptr;
    Camera* m_debugCamera = nullptr;

    std::vector<Vertex_PCU> m_xmlVerts;
    int m_channelIndex = 0;

    void StartupAttract();
    void StartupGame();
    void StartupParseXMLTests();
    void StartupConsoleTests();
    void StartupEventTests();

    void UpdateAttractScreen( float deltaSeconds );
    void UpdateGame( float deltaSeconds );
    void UpdateEntityArray( Entity** entityArray, int numEntities, float deltaSeconds );
    void UpdateFromController( float deltaSeconds );
    void UpdateCameraShake( float deltaSeconds );
    void UpdateConsoleChannels( float deltaSeconds );

    void RenderAttractScreen() const;
    void RenderGame() const;
    void RenderTexture( int desktopID ) const;
    void RenderSpriteAnimations( int desktopID ) const;
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
