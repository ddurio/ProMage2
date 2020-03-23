#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderTypes.hpp"


enum ShaderStageType {
    SHADER_STAGE_INVALID = -1,
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_PIXEL
};

class RenderContext;
class Shader;

class ShaderStage {
    friend class RenderContext;
    friend class Shader;

    private:
    ShaderStage( ID3D11Device* device, const char* shaderFilePath, const char* buffer, const size_t bufferSize, ShaderStageType stage, std::string entry = "" );
    ~ShaderStage();

    static const char* GetShaderEntryForStage( ShaderStageType stage );
    static const char* GetShaderModelForStage( ShaderStageType stage );
    static ID3D10Blob* CompileHLSLToShaderBlob( const char* opt_filename, const char* source_code, const size_t source_code_size, const char* entrypoint, char const* model );

    ShaderStageType m_stage = SHADER_STAGE_INVALID;
    ID3D11Resource* m_resource = nullptr; // Either ID3D11VertexShader or ID3D11PixelShader to start with (both derive from ID3D11Resource)
    ID3D10Blob* m_byteCode = nullptr;
};
