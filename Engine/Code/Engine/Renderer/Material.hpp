#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderTypes.hpp"
#include "Engine/Utils/XMLUtils.hpp"


class RenderContext;
class Sampler;
class Shader;
class UniformBuffer;


class Material {
    friend class EngineCommon;
    friend class RenderContext;

    public:
    void SetShader( Shader* shader );
    void SetShader( const std::string& shaderPath );

    void SetSampler( Sampler* sampler, int slotIndex = 0 );

    void SetTexture( const std::string& filePath = "", int slotIndex = 0 );

    void SetDiffuse( const std::string& filePath = "" );
    void SetNormal( const std::string& filePath = "" );
    void SetSpecular( const std::string& filePath = "" );
    void SetEmissive( const std::string& filePath = "" );

    void SetUBO( const void* data, const int size );
    void SetUBO( UniformBuffer* ubo );

    std::string GetID() const;
    Shader* GetShader() const;
    std::string GetShaderName() const;
    std::vector<Sampler*> GetSamplers() const;
    Strings GetTextures() const;
    UniformBuffer* GetUserBuffer() const;


    private:
    RenderContext* m_renderer = nullptr;
    std::string m_materialID = "";

    Shader* m_shader = nullptr;
    Strings m_textures;
    std::vector<Sampler*> m_samplers;
    UniformBuffer* m_userBuffer = nullptr;


    Material( RenderContext* renderer );
    Material( RenderContext* renderer, XMLElement* element );
    ~Material();
};
