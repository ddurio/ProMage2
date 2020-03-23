#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "dxgiformat.h"

#include "Engine/Math/IntVec2.hpp"


enum BlendMode {
    BLEND_MODE_ALPHA,
    BLEND_MODE_ADDITIVE,
    BLEND_MODE_CUSTOM
};


enum FilterMode {
    FILTER_MODE_INVALID = -1,

    FILTER_MODE_POINT,
    FILTER_MODE_LINEAR,
};


struct FilterInfo {
    FilterMode minFilterMode = FILTER_MODE_INVALID;
    FilterMode magFilterMode = FILTER_MODE_INVALID;


    FilterInfo() {};
    FilterInfo( FilterMode min, FilterMode mag ) : minFilterMode( min ), magFilterMode( mag ) {};

    bool operator==( const FilterInfo& otherFilter ) {
        return (minFilterMode == otherFilter.minFilterMode) && (magFilterMode == otherFilter.magFilterMode);
    }

    bool operator!=( const FilterInfo& otherFilter ) {
        return !(*this == otherFilter);
    }
};


enum FillMode {
    FILL_MODE_SOLID,
    FILL_MODE_WIREFRAME
};


enum DepthCompareOp {
    COMPARE_NEVER,
    COMPARE_ALWAYS,
    COMPARE_LESS,
    COMPARE_LESS_EQUAL,
    COMPARE_EQUAL,
    COMPARE_NOT_EQUAL,
    COMPARE_GREATER_EQUAL,
    COMPARE_GREATER
};


// Optional hints to GPU about usage patterns
enum GPUMemoryUsage {
    GPU_MEMORY_USAGE_GPU,     // Can be written/read from GPU only (Color Targets are a good example)
    GPU_MEMORY_USAGE_STATIC,  // Created, and are read only after that (ex: textures from images, sprite atlas)
    GPU_MEMORY_USAGE_DYNAMIC, // Update often from CPU, used by the GPU (CPU -> GPU updates, used by shaders.  ex: Uniform Buffers)
    GPU_MEMORY_USAGE_STAGING, // For getting memory from GPU to CPU (can be copied into, but not directly bound as an output.  ex: Screenshots system)
};


// Required: Represents what a buffer can be used for (multiple allowed)
enum RboUsageBit : unsigned int {
    RBO_USAGE_VERTEX_STREAM_BIT = BIT_FLAG( 0 ),   // Can be bound to an vertex input stream slot
    RBO_USAGE_INDEX_STREAM_BIT  = BIT_FLAG( 1 ),   // Can be bound as an index input stream.  
    RBO_USAGE_UNIFORMS_BIT      = BIT_FLAG( 2 ),   // Can be bound to a constant buffer slot; 
};

typedef unsigned int RboUsageBits; // Plural, combines one or more of the above


enum TextureSlotIndex {
    TEXTURE_SLOT_COLOR,
    TEXTURE_SLOT_NORMAL,
    TEXTURE_SLOT_SPECULAR,
    TEXTURE_SLOT_EMISSIVE
};


enum TextureUsageBit : unsigned int {
    TEXTURE_USAGE_TEXTURE_BIT               = BIT_FLAG( 0 ),    // Can be used to create a TextureView
    TEXTURE_USAGE_RENDER_TARGET_BIT         = BIT_FLAG( 1 ),    // Can be used to create a RenderTargetView
    TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT  = BIT_FLAG( 2 ),    // Can be used to create a DepthStencilTargetView
};

typedef unsigned int TextureUsageBits; // Plural, combines one or more of the above


enum UboSlotIndex {
    UBO_SLOT_FRAME  = 1,
    UBO_SLOT_CAMERA,
    UBO_SLOT_MODEL,
    UBO_SLOT_DEBUG_DRAW,
    UBO_SLOT_LIGHTING,
    UBO_SLOT_USER_MATERIAL
};


enum UVMode {
    UV_MODE_WRAP,
    UV_MODE_MIRROR,
    UV_MODE_CLAMP,
    UV_MODE_BORDER,
    UV_MODE_MIRROR_ONCE,
};


struct UVInfo {
    UVMode uMode = UV_MODE_WRAP;
    UVMode vMode = UV_MODE_WRAP;
    UVMode wMode = UV_MODE_CLAMP;

    bool operator==( const UVInfo& otherUV ) {
        return (uMode == otherUV.uMode) && (vMode == otherUV.vMode) && (wMode == otherUV.wMode);
    }

    bool operator!=( const UVInfo& otherUV ) {
        return !(*this == otherUV);
    }
};


enum TextureType {
    TEXTURE_TYPE_IMAGE,
    TEXTURE_TYPE_IMAGE_COLOR,
    TEXTURE_TYPE_COLOR_TARGET,
    TEXTURE_TYPE_DEPTH,
    TEXTURE_TYPE_STAGING
};


class Image;

struct TextureInfo {
    public:
    TextureUsageBits textureUsage = TEXTURE_USAGE_TEXTURE_BIT;
    GPUMemoryUsage memoryUsage = GPU_MEMORY_USAGE_GPU;
    DXGI_FORMAT format = (DXGI_FORMAT)28; // DXGI_FORMAT_R8G8B8A8_UNORM
    std::string imageFilePath = "";
    Image* existingImage = nullptr;
    IntVec2 dimensions = IntVec2::ZERO;
    TextureType type = TEXTURE_TYPE_IMAGE;
};


enum D3D11_BLEND : int;
enum D3D11_BLEND_OP : int;
enum D3D11_COMPARISON_FUNC : int;
enum D3D11_FILTER : int;
enum D3D11_TEXTURE_ADDRESS_MODE : int;
enum D3D11_USAGE : int;

struct ID3D10Blob;
struct ID3D11BlendState;
struct ID3D11Buffer;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11Device;
struct ID3D11DeviceChild;
struct ID3D11DeviceContext;
struct ID3D11InfoQueue;
struct ID3D11InputLayout;
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;
struct ID3D11Resource;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11View;
struct IDXGISwapChain;



D3D11_USAGE GetMemoryUsageDXFromGPU( GPUMemoryUsage gpuMemUsage );
unsigned int GetDXBindFromUsage( unsigned int usage );
D3D11_FILTER GetDXFilter( FilterMode min, FilterMode mag );
D3D11_COMPARISON_FUNC GetDXCompareOp( DepthCompareOp op );
