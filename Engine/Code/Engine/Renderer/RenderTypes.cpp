#include "Engine/Renderer/RenderTypes.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )


D3D11_USAGE GetMemoryUsageDXFromGPU( GPUMemoryUsage gpuMemUsage ) {
    switch( gpuMemUsage ) {
        case GPU_MEMORY_USAGE_GPU: {
            return D3D11_USAGE_DEFAULT;
        } case GPU_MEMORY_USAGE_STATIC: {
            return D3D11_USAGE_IMMUTABLE;
        } case GPU_MEMORY_USAGE_DYNAMIC: {
            return D3D11_USAGE_DYNAMIC;
        } case GPU_MEMORY_USAGE_STAGING: {
            return D3D11_USAGE_STAGING;
        } default: {
            ASSERT_RETURN_VALUE( false, D3D11_USAGE_DYNAMIC );
        }
    }
}


unsigned int GetDXBindFromUsage( unsigned int usage ) {
    unsigned int binds = 0u;

    if( usage & TEXTURE_USAGE_TEXTURE_BIT ) {
        binds |= D3D11_BIND_SHADER_RESOURCE;
    }

    if( usage & TEXTURE_USAGE_RENDER_TARGET_BIT ) {
        binds |= D3D11_BIND_RENDER_TARGET;
    }

    if( usage & TEXTURE_USAGE_DEPTH_STENCIL_TARGET_BIT ) {
        binds |= D3D11_BIND_DEPTH_STENCIL;
    }

    return binds;
}


D3D11_FILTER GetDXFilter( FilterMode min, FilterMode mag ) {
    if( min == FILTER_MODE_POINT ) {
        if( mag == FILTER_MODE_POINT ) {
            return D3D11_FILTER_MIN_MAG_MIP_POINT;
        } else if( mag == FILTER_MODE_LINEAR ) {
            return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }
    } else if( min == FILTER_MODE_LINEAR ) {
        if( mag == FILTER_MODE_POINT ) {
            return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        } else if( mag == FILTER_MODE_LINEAR ) {
            return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }
    }

    return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

D3D11_COMPARISON_FUNC GetDXCompareOp( DepthCompareOp op ) {
    switch(op) {
        case(COMPARE_NEVER): {
            return D3D11_COMPARISON_NEVER;
        } case(COMPARE_ALWAYS): {
            return D3D11_COMPARISON_ALWAYS;
        } case(COMPARE_LESS): {
            return D3D11_COMPARISON_LESS;
        } case(COMPARE_LESS_EQUAL): {
            return D3D11_COMPARISON_LESS_EQUAL;
        } case(COMPARE_EQUAL): {
            return D3D11_COMPARISON_EQUAL;
        } case(COMPARE_NOT_EQUAL): {
            return D3D11_COMPARISON_NOT_EQUAL;
        }case(COMPARE_GREATER_EQUAL): {
            return D3D11_COMPARISON_GREATER_EQUAL;
        } case(COMPARE_GREATER): {
            return D3D11_COMPARISON_GREATER;
        } default: {
            return D3D11_COMPARISON_LESS_EQUAL;
        }
    }
}