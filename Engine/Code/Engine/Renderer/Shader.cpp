#include "Engine/Renderer/Shader.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )

// Engine Systems
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Vertex/VertexLit.hpp"
#include "Engine/Vertex/VertexPCU.hpp"


void Shader::SetBlendMode( D3D11_BLEND colorSource, D3D11_BLEND colorDest, D3D11_BLEND_OP colorOp, D3D11_BLEND alphaSource, D3D11_BLEND alphaDest, D3D11_BLEND_OP alphaOp ) {
    // Free old state
    DX_SAFE_RELEASE( m_blendState );
    m_drawMode = BLEND_MODE_CUSTOM;

    D3D11_BLEND_DESC desc;
    memset( &desc, 0, sizeof(D3D11_BLEND_DESC) );

    desc.AlphaToCoverageEnable = false;  // used in MSAA to treat alpha as coverage (usual example is foliage rendering, we will not be using this)
    desc.IndependentBlendEnable = false;   // if you have multiple outputs bound, you can set this to true to have different blend state per output

    // FinalColor = BlendOp( SrcFactor * outputColor, DestFactor * destColor )
    // FinalColor = outputColor.a * outputColor + (1.0f - outputColor.a) * destColor;  
    desc.RenderTarget[0].BlendEnable = TRUE;  // we want to blend

    // Color fractions
    desc.RenderTarget[0].SrcBlend = colorSource;
    desc.RenderTarget[0].DestBlend = colorDest;
    desc.RenderTarget[0].BlendOp = colorOp;

    // Alpha fractions (take largest of the two alphas)
    desc.RenderTarget[0].SrcBlendAlpha = alphaSource;
    desc.RenderTarget[0].DestBlendAlpha = alphaDest;
    desc.RenderTarget[0].BlendOpAlpha = alphaOp;

    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // Finally, create the blend state
    m_d3dDevice->CreateBlendState( &desc, &m_blendState );
}


void Shader::SetBlendMode( BlendMode mode ) {
    // if we already have a good blend state - keep it; 
    if( (m_blendState != nullptr) && (m_drawMode == mode) ) {
        return;
    }

    D3D11_BLEND colorSource;
    D3D11_BLEND colorDest;
    D3D11_BLEND_OP colorOp;
    D3D11_BLEND alphaSource;
    D3D11_BLEND alphaDest;
    D3D11_BLEND_OP alphaOp;

    if( mode == BLEND_MODE_ALPHA ) {
        // Color fractions
        colorSource = D3D11_BLEND_SRC_ALPHA;       // New alpha
        colorDest = D3D11_BLEND_INV_SRC_ALPHA;  // (1 - New alpha)
        colorOp = D3D11_BLEND_OP_ADD;

        // Alpha fractions (take largest of the two alphas)
        alphaSource = D3D11_BLEND_ONE;
        alphaDest = D3D11_BLEND_ONE;
        alphaOp = D3D11_BLEND_OP_MAX;
    } else if( mode == BLEND_MODE_ADDITIVE ) {
        // Color fractions
        colorSource = D3D11_BLEND_SRC_ALPHA;       // New alpha
        colorDest = D3D11_BLEND_ONE;     // Old alpha
        colorOp = D3D11_BLEND_OP_ADD;

        // Alpha fractions
        alphaSource = D3D11_BLEND_ONE;
        alphaDest = D3D11_BLEND_ONE;
        alphaOp = D3D11_BLEND_OP_ADD;
    } else {
        ERROR_AND_DIE( Stringf( "(Shader) Unknown blend mode (%s) requested.", mode ) );
    }

    SetBlendMode( colorSource, colorDest, colorOp, alphaSource, alphaDest, alphaOp );
    m_drawMode = mode;
}


void Shader::SetDepthMode( DepthCompareOp op, bool writeEnabled ) {
    if( (m_depthOp != op) || (m_depthWriteEnabled != writeEnabled) || (m_depthState == nullptr) ) {
        m_depthWriteEnabled = writeEnabled;
        m_depthOp = op;
        D3D11_DEPTH_STENCIL_DESC ds_desc = {};

        ds_desc.DepthEnable = (m_depthWriteEnabled || m_depthOp != COMPARE_ALWAYS); // Only false if !writeEnalbed and compareAlways
        ds_desc.DepthWriteMask = m_depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = GetDXCompareOp( m_depthOp ); //  

        // Stencil - just use defaults for now; 
        ds_desc.StencilEnable = false;
        ds_desc.StencilReadMask = 0xFF;
        ds_desc.StencilWriteMask = 0xFF;

        D3D11_DEPTH_STENCILOP_DESC default_stencil_op = {};
        default_stencil_op.StencilFailOp = D3D11_STENCIL_OP_KEEP;      // what to do if stencil fails
        default_stencil_op.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // What to do if stencil succeeds but depth fails
        default_stencil_op.StencilPassOp = D3D11_STENCIL_OP_KEEP;      // what to do if the stencil succeeds
        default_stencil_op.StencilFunc = D3D11_COMPARISON_ALWAYS;      // function to test against

        // can have different rules setup for front and backface
        ds_desc.FrontFace = default_stencil_op;
        ds_desc.BackFace = default_stencil_op;

        DX_SAFE_RELEASE( m_depthState );
        m_d3dDevice->CreateDepthStencilState( &ds_desc, &m_depthState );
    }
}


void Shader::SetFillMode( FillMode mode ) {
    if( m_rasterState != nullptr && m_fillMode == mode ) {
        return;
    }

    m_fillMode = mode;

    D3D11_RASTERIZER_DESC desc;
    memset( &desc, 0, sizeof(D3D11_RASTERIZER_DESC) );


    desc.FillMode = (m_fillMode == FILL_MODE_SOLID ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME);
    desc.CullMode = D3D11_CULL_BACK;
    desc.FrontCounterClockwise = false;
    desc.DepthBias = 0;
    desc.SlopeScaledDepthBias = 0.f;
    desc.DepthBiasClamp = 0.f;
    desc.DepthClipEnable = true;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;

    DX_SAFE_RELEASE( m_rasterState );
    m_d3dDevice->CreateRasterizerState( &desc, &m_rasterState );
}


std::string Shader::GetFilePath() const {
    return m_shaderFilePath;
}


void Shader::GetVertexInfo( CopyFromMasterCallback& callbackOut, size_t& strideOut ) const {
    callbackOut = m_vertexCopyCB;
    strideOut = m_vertexStride;
}


D3D11_BLEND Shader::ParseXMLAttribute( const XMLElement& element, const char* attributeName, D3D11_BLEND defaultValue ) {
    std::string blendString = ::ParseXMLAttribute( element, attributeName, "" );

    if( blendString == "zero" ) {
        return D3D11_BLEND_ZERO;
    } else if( blendString == "one" ) {
        return D3D11_BLEND_ONE;
    } else if( blendString == "srcColor" ) {
        return D3D11_BLEND_SRC_COLOR;
    } else if( blendString == "invSrcColor" ) {
        return D3D11_BLEND_INV_SRC_COLOR;
    } else if( blendString == "srcAlpha" ) {
        return D3D11_BLEND_SRC_ALPHA;
    } else if( blendString == "invSrcAlpha" ) {
        return D3D11_BLEND_INV_SRC_ALPHA;
    } else if( blendString == "destAlpha" ) {
        return D3D11_BLEND_DEST_ALPHA;
    } else if( blendString == "invDestAlpha" ) {
        return D3D11_BLEND_INV_DEST_ALPHA;
    } else if( blendString == "destColor" ) {
        return D3D11_BLEND_DEST_COLOR;
    } else if( blendString == "invDestColor" ) {
        return D3D11_BLEND_INV_DEST_COLOR;
    } else {
        return defaultValue;
    }
}


D3D11_BLEND_OP Shader::ParseXMLAttribute( const XMLElement& element, const char* attributeName, D3D11_BLEND_OP defaultValue ) {
    std::string blendOpString = ::ParseXMLAttribute( element, attributeName, "" );

    if( blendOpString == "add" ) {
        return D3D11_BLEND_OP_ADD;
    } else if( blendOpString == "sub" ) {
        return D3D11_BLEND_OP_SUBTRACT;
    } else if( blendOpString == "revSub" ) {
        return D3D11_BLEND_OP_REV_SUBTRACT;
    } else if( blendOpString == "min" ) {
        return D3D11_BLEND_OP_MIN;
    } else if( blendOpString == "max" ) {
        return D3D11_BLEND_OP_MAX;
    } else {
        return defaultValue;
    }
}


DepthCompareOp Shader::ParseXMLAttribute( const XMLElement& element, const char* attributeName, DepthCompareOp defaultValue ) {
    std::string op = ::ParseXMLAttribute( element, attributeName, "" );

    if( op == "never" ) {
        return COMPARE_NEVER;
    } else if( op == "always" ) {
        return COMPARE_ALWAYS;
    } else if( op == "<" ) {
        return COMPARE_LESS;
    } else if( op == "<=" ) {
        return COMPARE_LESS_EQUAL;
    } else if( op == "==" ) {
        return COMPARE_EQUAL;
    } else if( op == "!=" || op == "~=" ) {
        return COMPARE_NOT_EQUAL;
    } else if( op == ">=" ) {
        return COMPARE_GREATER_EQUAL;
    } else if( op == ">" ) {
        return COMPARE_GREATER;
    } else {
        return defaultValue;
    }
}


Shader::Shader( ID3D11Device* device, const std::string& shaderFilePath ) :
    m_d3dDevice(device) {
    m_shaderFilePath = shaderFilePath;
    int extensionIndex = (int)shaderFilePath.rfind( '.' );
    std::string extension = StringToLower( shaderFilePath.substr( extensionIndex + 1 ) );

    if( extension == "shader" ) {
        StartupXML( device, shaderFilePath );
    } else if( extension == "hlsl" ) {
        StartupHLSL( device, shaderFilePath );
    } else if( shaderFilePath == SHADER_CONSOLE ) {
        StartupHLSL( device, shaderFilePath, RenderContext::DEFAULT_SHADER_UNLIT );
        CreateInputLayout<VertexPCU>();
    } else if( shaderFilePath == SHADER_INVALID ) {
        StartupHLSL( device, shaderFilePath, RenderContext::DEFAULT_SHADER_INVALID );
        CreateInputLayout<VertexPCU>();
    } else if( shaderFilePath == SHADER_LIT ) {
        StartupHLSL( device, shaderFilePath, RenderContext::DEFAULT_SHADER_LIT );
        CreateInputLayout<VertexLit>();
    } else if( shaderFilePath == SHADER_UNLIT ) {
        StartupHLSL( device, shaderFilePath, RenderContext::DEFAULT_SHADER_UNLIT );
        CreateInputLayout<VertexPCU>();
    } else {
        ERROR_AND_DIE( Stringf( "ERROR: Unknown shader extension for file %s", shaderFilePath.c_str() ) );
    }
}


Shader::Shader( const Shader& copyFrom ) :
    Shader( copyFrom.m_d3dDevice, copyFrom.m_shaderFilePath ) {
}


Shader::~Shader() {
    delete m_vertexShader;
    m_vertexShader = nullptr;

    delete m_pixelShader;
    m_pixelShader = nullptr;

    DX_SAFE_RELEASE( m_inputLayout );
    DX_SAFE_RELEASE( m_blendState );
    DX_SAFE_RELEASE( m_depthState );
    DX_SAFE_RELEASE( m_rasterState );
}


void Shader::StartupHLSL( ID3D11Device* device, const std::string& shaderFilePath, const char* inputBuffer /*= nullptr*/ ) {
    size_t bufferSize;
    const char* buffer = nullptr;

    if( inputBuffer == nullptr ) {
        char* fileBuffer = nullptr;
        bool result = ReadFromFile( shaderFilePath.c_str(), &fileBuffer, bufferSize );

        if( !result ) {
            LoadInvalid();
            return;
        }

        buffer = fileBuffer;
    } else {
        buffer = inputBuffer;
        bufferSize = strlen( buffer );
    }

    m_vertexShader = new ShaderStage( device, shaderFilePath.c_str(), buffer, bufferSize, SHADER_STAGE_VERTEX );
    m_pixelShader = new ShaderStage( device, shaderFilePath.c_str(), buffer, bufferSize, SHADER_STAGE_PIXEL );

    SetBlendMode( BLEND_MODE_ALPHA );
    SetDepthMode( COMPARE_LESS_EQUAL, true );
    SetFillMode( FILL_MODE_SOLID );
}


void Shader::StartupXML( ID3D11Device* device, const std::string& shaderFilePath ) {
    XmlDocument shaderConfig = new XmlDocument();
    tinyxml2::XMLError loadSuccess = shaderConfig.LoadFile( shaderFilePath.c_str() );

    if( loadSuccess != tinyxml2::XML_SUCCESS ) {
        ERROR_RECOVERABLE( Stringf( "Failed to load XML file: %s", shaderFilePath.c_str() ) );
        LoadInvalid();
        return;
    }

    XMLElement* root = shaderConfig.FirstChildElement();

    if( root == nullptr ) {
        ERROR_RECOVERABLE( "Poorly constructed XML file" );
        LoadInvalid();
        return;
    }

    XMLElement* pass = root->FirstChildElement( "Pass" );
    std::string sourceFilePath = ::ParseXMLAttribute( *pass, "source", "" );

    size_t bufferSize;
    char* buffer = nullptr;
    bool result = ReadFromFile( sourceFilePath.c_str(), &buffer, bufferSize );

    if( !result ) {
        LoadInvalid();
        return;
    }

    // Create ShaderStages
    XMLElement* vertex = pass->FirstChildElement( "Vertex" );
    XMLElement* fragment = pass->FirstChildElement( "Fragment" );

    std::string vertexEntry = ::ParseXMLAttribute( *vertex, "entry", "" );
    std::string fragmentEntry = ::ParseXMLAttribute( *fragment, "entry", "" );

    m_vertexShader = new ShaderStage( device, shaderFilePath.c_str(), buffer, bufferSize, SHADER_STAGE_VERTEX, vertexEntry );
    m_pixelShader = new ShaderStage( device, shaderFilePath.c_str(), buffer, bufferSize, SHADER_STAGE_PIXEL, fragmentEntry );

    // Set Blend Mode
    XMLElement* blend = pass->FirstChildElement( "Blend" );
    XMLElement* color = blend->FirstChildElement( "Color" );
    XMLElement* alpha = blend->FirstChildElement( "Alpha" );

    D3D11_BLEND    colorSource = ParseXMLAttribute( *color, "source", D3D11_BLEND_SRC_ALPHA );
    D3D11_BLEND    colorDest   = ParseXMLAttribute( *color, "dest", D3D11_BLEND_INV_SRC_ALPHA );
    D3D11_BLEND_OP colorOp     = ParseXMLAttribute( *color, "op", D3D11_BLEND_OP_ADD );

    D3D11_BLEND    alphaSource = ParseXMLAttribute( *alpha, "source", D3D11_BLEND_ONE );
    D3D11_BLEND    alphaDest   = ParseXMLAttribute( *alpha, "dest", D3D11_BLEND_ONE );
    D3D11_BLEND_OP alphaOp     = ParseXMLAttribute( *alpha, "op", D3D11_BLEND_OP_MAX );

    SetBlendMode( colorSource, colorDest, colorOp, alphaSource, alphaDest, alphaOp );

    // Set Depth Mode
    XMLElement* depth = pass->FirstChildElement( "Depth" );

    DepthCompareOp op = ParseXMLAttribute( *depth, "compareOp", COMPARE_LESS_EQUAL );
    bool writeDepth = ::ParseXMLAttribute( *depth, "write", true );

    SetDepthMode( op, writeDepth );
}


bool Shader::CreateInputLayout( const BufferAttribute* attrList, size_t stride, CopyFromMasterCallback copyCB ) {
    std::scoped_lock localLock( m_mutex );

    if( m_inputLayout != nullptr && m_vertexCopyCB == copyCB ) {
        return true;
    }

    m_vertexCopyCB = copyCB;
    m_vertexStride = stride;
    DX_SAFE_RELEASE( m_inputLayout );

    int attrIndex = 0;
    const BufferAttribute* attribute = &attrList[attrIndex];
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescriptions;


    while( !attribute->isNull() ) {
        D3D11_INPUT_ELEMENT_DESC desc;
        memset( &desc, 0, sizeof( D3D11_INPUT_ELEMENT_DESC ) );

        desc.SemanticName = attribute->semanticName.c_str();           // __semantic__ name we gave this input -> float3 pos : POSITION; 
        desc.SemanticIndex = 0;                                        // Semantics that share a name (or are large) are spread over multiple indices (matrix4x4s are four floats for instance)
        desc.Format = GetFormatFromAttribute(*attribute);              // Type this data is (float3/vec3 - so 3 floats)
        desc.InputSlot = 0U;                                           // Input Pipe this comes from (ignored unless doing instanced rendering)
        desc.AlignedByteOffset = attribute->offset;                    // memory offset this data starts (where is position relative to the vertex, 0 in this case)
        desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;             // What is this data for (per vertex data is the only one we'll be dealing with in this class)
        desc.InstanceDataStepRate = 0U;                                // If this were instance data - how often do we step it (0 for vertex data)

        inputDescriptions.push_back( desc );

        attrIndex++;
        attribute = &attrList[attrIndex];
    }


    ID3D10Blob *vs_bytecode = m_vertexShader->m_byteCode;
    HRESULT hr = m_d3dDevice->CreateInputLayout( inputDescriptions.data(), (unsigned int)inputDescriptions.size(), vs_bytecode->GetBufferPointer(), vs_bytecode->GetBufferSize(), &m_inputLayout );

    return SUCCEEDED( hr );
}


void Shader::LoadInvalid() {
    StartupHLSL( m_d3dDevice, "Invalid", RenderContext::DEFAULT_SHADER_INVALID );
}


DXGI_FORMAT Shader::GetFormatFromAttribute( const BufferAttribute& attr ) {
    switch( attr.type ) {
        case( ATTR_TYPE_FLOAT ): {
            return DXGI_FORMAT_R32_FLOAT;
        } case( ATTR_TYPE_VEC2 ): {
            return DXGI_FORMAT_R32G32_FLOAT;
        } case( ATTR_TYPE_VEC3 ): {
            return DXGI_FORMAT_R32G32B32_FLOAT;
        } case( ATTR_TYPE_RGBA ): {
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        } default: {
            return DXGI_FORMAT_R32_FLOAT; // return FLOAT???
        }
    }
}
