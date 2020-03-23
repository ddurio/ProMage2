#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderTypes.hpp"
#include "Engine/Renderer/ShaderStage.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/Vertex/VertexMaster.hpp"

#include "mutex"


class RenderContext;

class Shader {
    friend class EngineCommon;
    friend class RenderContext;

    public:
    void SetBlendMode( D3D11_BLEND colorSource, D3D11_BLEND colorDest, D3D11_BLEND_OP colorOp, D3D11_BLEND alphaSource, D3D11_BLEND alphaDest, D3D11_BLEND_OP alphaOp );
    void SetBlendMode( BlendMode mode );
    void SetDepthMode( DepthCompareOp op, bool writeEnabled );
    void SetFillMode( FillMode mode );

    std::string GetFilePath() const;
    void GetVertexInfo( CopyFromMasterCallback& callbackOut, size_t& strideOut ) const;

    template< typename VertexType >
    bool CreateInputLayout();

    static D3D11_BLEND ParseXMLAttribute( const XMLElement& element, const char* attributeName, D3D11_BLEND defaultValue );
    static D3D11_BLEND_OP ParseXMLAttribute( const XMLElement& element, const char* attributeName, D3D11_BLEND_OP defaultValue );
    static DepthCompareOp ParseXMLAttribute( const XMLElement& element, const char* attributeName, DepthCompareOp defaultValue );


    private:
    explicit Shader( ID3D11Device* device, const std::string& shaderFilePath );
    explicit Shader( const Shader& copyFrom );
    ~Shader();

    ID3D11Device* m_d3dDevice = nullptr;
    std::string m_shaderFilePath = "";

    ShaderStage* m_vertexShader = nullptr;
    ShaderStage* m_pixelShader = nullptr;

    CopyFromMasterCallback m_vertexCopyCB = nullptr;
    size_t m_vertexStride = 0;
    ID3D11InputLayout* m_inputLayout = nullptr;
    ID3D11BlendState* m_blendState = nullptr;
    BlendMode m_drawMode = BLEND_MODE_ALPHA;

    ID3D11DepthStencilState* m_depthState = nullptr;
    DepthCompareOp m_depthOp = COMPARE_LESS_EQUAL;
    bool m_depthWriteEnabled = false;

    FillMode m_fillMode = FILL_MODE_SOLID;
    ID3D11RasterizerState* m_rasterState = nullptr;

    std::mutex m_mutex;


    void StartupHLSL( ID3D11Device* device, const std::string& shaderFilePath, const char* inputBuffer = nullptr );
    void StartupXML( ID3D11Device* device, const std::string& shaderFilePath );

    bool CreateInputLayout( const BufferAttribute* attrList, size_t stride, CopyFromMasterCallback copyCB );

    void LoadInvalid();

    static DXGI_FORMAT GetFormatFromAttribute( const BufferAttribute& attr );
};


template< typename VertexType >
bool Shader::CreateInputLayout() {
    return CreateInputLayout( &VertexType::LAYOUT[0], sizeof( VertexType ), VertexType::CopyFromMaster );
}
