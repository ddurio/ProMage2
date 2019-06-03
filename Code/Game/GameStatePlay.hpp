#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/EventSystem.hpp"

#include "Game/GameState.hpp"


class GameInput;
class Map;
class Material;
class Model;
class RTSCamera;
class TestEntity;
class TextureView2D;
class Timer;
class UIWidget;
class UniformBuffer;


class GameStatePlay : public GameState {
    public:
    GameStatePlay();
    ~GameStatePlay();

    void Startup() override;
    void Shutdown() override;

    void Update() override;
    void Render() override;

    bool HandleKeyPressed( unsigned char keyCode ) override;
    bool HandleKeyReleased( unsigned char keyCode ) override;
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f ) override;
    bool HandleQuitRequested() override;

    GameInput* GetGameInput() const override;

    static bool Command_PauseGame( EventArgs& args );


    private:
    GameInput* m_gameInput = nullptr;
    std::vector<Material*> m_materials;

    bool m_isPaused = false;

    Map* m_map = nullptr;
    UIWidget* m_pauseUI = nullptr;
    UniformBuffer* m_pauseUBO = nullptr;
    Timer* m_fadeTimer = nullptr;
    float m_fadeSeconds = 0.75f;
    bool m_fadeOut = false;

    std::string m_pauseViewName = "gameStatePlay/pauseEffect";
    std::string m_pauseMatName = "Data/Materials/PauseEffect.xml";
    std::string m_grayscaleMatName = "Data/Materials/GrayscaleEffect.xml";

    void BuildPauseUI();
};
