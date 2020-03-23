#include "Engine/Renderer/ShaderStage.hpp"
// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )

// HLSL Compiler
#include <d3dcompiler.h>
#pragma comment( lib, "d3dcompiler.lib" )

// Engine Headers
#include "Engine/Renderer/RenderContext.hpp"



ShaderStage::ShaderStage( ID3D11Device* device, const char* shaderFilePath, const char* buffer, const size_t bufferSize, ShaderStageType stage, std::string entry /*= ""*/ ) {
    m_stage = stage;

    const char* entryPoint = (entry == "" ? GetShaderEntryForStage( m_stage ) : entry.c_str());
    const char* model      = GetShaderModelForStage( m_stage );
    ID3D10Blob* byteCode = CompileHLSLToShaderBlob( shaderFilePath, buffer, bufferSize, entryPoint, model );

    if( byteCode == nullptr ) {
        byteCode = CompileHLSLToShaderBlob( nullptr, RenderContext::DEFAULT_SHADER_INVALID, strlen( RenderContext::DEFAULT_SHADER_INVALID ), entryPoint, model );

        if( byteCode == nullptr ) {
            ERROR_AND_DIE( "(ShaderStage) Failed to compile BuiltIn/Invalid shader." );
        }
    }

    void* bufferPointer = byteCode->GetBufferPointer(); // why not just use the input value...?
    size_t size = byteCode->GetBufferSize(); // why not just use the input value...?

    switch( stage ) {
        case SHADER_STAGE_VERTEX: {   // Compile the byte code to the final shader (driver/hardware specific program)
            device->CreateVertexShader( bufferPointer, size, nullptr, (ID3D11VertexShader**)(&m_resource) );
            m_byteCode = byteCode;
            break;
        } case SHADER_STAGE_PIXEL: {
            device->CreatePixelShader( bufferPointer, size, nullptr, (ID3D11PixelShader**)(&m_resource) );
            DX_SAFE_RELEASE( byteCode );
            break;
        }
    }

    if( m_resource == nullptr ) {
        ERROR_AND_DIE( Stringf( "(ShaderStage) Failed to create shader stage %s.", m_stage ) );
    }
}


ShaderStage::~ShaderStage() {
    DX_SAFE_RELEASE( m_resource );
    DX_SAFE_RELEASE( m_byteCode );
}


const char* ShaderStage::GetShaderEntryForStage( ShaderStageType stage ) {
    switch( stage ) {
        case SHADER_STAGE_VERTEX: {
            return "VertexFunction";
        } case SHADER_STAGE_PIXEL: {
            return "FragmentFunction";
        } default: {
            GUARANTEE_OR_DIE( false, "Unknown shader stage" );
        }
    }
}


const char* ShaderStage::GetShaderModelForStage( ShaderStageType stage ) {
    switch( stage ) {
        case SHADER_STAGE_VERTEX: {
            return "vs_5_0";
        } case SHADER_STAGE_PIXEL: {
            return "ps_5_0";
        } default: {
            GUARANTEE_OR_DIE( false, "Unknown shader stage" );
        }
    }
}


#if defined(_DEBUG)
    #define DEBUG_SHADERS
#endif


ID3DBlob* ShaderStage::CompileHLSLToShaderBlob( const char* opt_filename, const char* source_code, const size_t source_code_size, const char* entrypoint, char const* model ) {
    DWORD compile_flags = 0U;
#if defined(DEBUG_SHADERS)
    compile_flags |= D3DCOMPILE_DEBUG;
    compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else 
    // compile_flags |= D3DCOMPILE_SKIP_VALIDATION;       // Only do this if you know for a fact this shader works with this device (so second run through of a game)
    compile_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;   // Yay, fastness (default is level 1)
#endif

    ID3DBlob *code = nullptr;
    ID3DBlob *errors = nullptr;

    HRESULT hr = D3DCompile( source_code,
        source_code_size,                   // plain text source code
        opt_filename,                       // optional, used for error messages (If you HLSL has includes - it will not use the includes names, it will use this name)
        nullptr,                            // pre-compiler defines - used more for compiling multiple versions of a single shader (different quality specs, or shaders that are mostly the same outside some constants)
        D3D_COMPILE_STANDARD_FILE_INCLUDE,  // include rules - this allows #includes in the shader to work relative to the src_file path or my current working directly
        entrypoint,                         // Entry Point for this shader
        model,                              // Compile Target (MSDN - "Specifying Compiler Targets")
        compile_flags,                      // Flags that control compilation
        0,                                  // Effect Flags (we will not be doing Effect Files)
        &code,                              // [OUT] ID3DBlob (buffer) that will store the byte code.
        &errors );                          // [OUT] ID3DBlob (buffer) that will store error information

    if( FAILED( hr ) || (errors != nullptr) ) {
        if( errors != nullptr ) {
            char *error_string = (char*)errors->GetBufferPointer();

            ERROR_RECOVERABLE( Stringf( "Failed to compile [%s].  Compiler gave the following output;\n%s",
                opt_filename,
                error_string ) );

            DX_SAFE_RELEASE( errors );
            return nullptr;
        } else {
            ERROR_AND_DIE( Stringf( "Failed with HRESULT: %u", hr ) );
        }
    }

    // will be nullptr if it failed to compile
    return code;
}
