#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Async/Job.hpp"

#include "Engine/Renderer/RenderTypes.hpp"


class Image;
class Texture2D;


class CopyTextureJob : public Job {
    public:
    CopyTextureJob( JobSystem* jobSystem, ID3D11DeviceContext* d3dContext, Texture2D* stagingTexture, Image* destination = nullptr, const std::string& callbackEvent = "" );
    ~CopyTextureJob();

    Image* GetImage() const;


    private:
    ID3D11DeviceContext* m_d3dContext = nullptr;
    Texture2D* m_stagingTexture = nullptr;
    Image* m_destImage = nullptr;


    void Execute() override;
};
