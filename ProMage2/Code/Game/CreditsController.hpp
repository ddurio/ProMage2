#pragma once
#include "Game/GameCommon.hpp"

#include "Game/ActorController.hpp"


class Camera;
class GPUMesh;
class Model;
class Timer;


class CreditsController : public ActorController {
    public:
    CreditsController( Actor* myActor );
    ~CreditsController();

    void Update( float deltaSeconds ) override;
    void RenderCredits() const;

    bool InteractWithActor( Actor* instigator ) override;

    private:
    bool m_creditsAreRolling = false;
    Camera* m_creditsCamera = nullptr;

    Model* m_creditsModel    = nullptr;
    GPUMesh* m_guildhallMesh = nullptr;
    GPUMesh* m_titleMesh     = nullptr;
    GPUMesh* m_thanksMesh    = nullptr;
    int m_numCycles = 3;

    bool m_fadeIn = false;
    bool m_fadeOut = false;
    float m_fadeSeconds = 0.75f;
    Timer* m_fadeTimer = nullptr;

    std::string m_creditsViewName = "creditsController/creditsView";
    std::string m_creditsMatName = "creditsController/creditsMat";
    std::string m_checkerMatName = "Data/Materials/PauseEffect.xml";
    std::string m_grayscaleMatName = "Data/Materials/GrayscaleEffect.xml";


    void BuildGuildhallMesh();
    void BuildTitleMesh();
    void BuildThanksMesh();
};
