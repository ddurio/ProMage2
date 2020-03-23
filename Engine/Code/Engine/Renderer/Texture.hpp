#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


class Image;


class Texture {
    public:
    ID3D11Resource* GetHandle() const;
    std::string GetFilePath() const;
    TextureType GetType() const;


    protected:
    explicit Texture( ID3D11Device* device, const TextureInfo& info );
    virtual ~Texture();

    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11Resource* m_d3dTexture = nullptr;

    GPUMemoryUsage m_memoryUsage = GPU_MEMORY_USAGE_STATIC;
    TextureUsageBits m_textureUsage = TEXTURE_USAGE_TEXTURE_BIT;

    std::string m_filePath = "";
    TextureType m_type = TEXTURE_TYPE_IMAGE;
};