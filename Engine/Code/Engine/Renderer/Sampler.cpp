#include "Engine/Renderer/Sampler.hpp"

// D3D11
#include <d3d11.h>  
#include <DXGI.h>    

#include <dxgidebug.h>
// #pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "DXGI.lib" )


ID3D11SamplerState* Sampler::GetHandle() const {
    return m_state;
}


SampleMode Sampler::ParseXMLAttribute( const XMLElement& element, const char* attributeName, SampleMode defaultValue ) {
    std::string modeText = ::ParseXMLAttribute( element, attributeName, "invalid" );
    modeText = StringToLower( modeText );

    if( modeText == "linear" ) {
        return SAMPLE_MODE_LINEAR;
    } else if( modeText == "point" ) {
        return SAMPLE_MODE_POINT;
    } else if( modeText == "default" ) {
        return SAMPLE_MODE_DEFAULT;
    }

    return defaultValue;
}


FilterMode Sampler::ParseXMLAttribute( const XMLElement& element, const char* attributeName, FilterMode defaultValue ) {
    std::string filterText = ::ParseXMLAttribute( element, attributeName, "invalid" );
    filterText = StringToLower( filterText );

    if( filterText == "linear" ) {
        return FILTER_MODE_LINEAR;
    } else if( filterText == "point" ) {
        return FILTER_MODE_POINT;
    }

    return defaultValue;
}


UVMode Sampler::ParseXMLAttribute( const XMLElement& element, const char* attributeName, UVMode defaultValue ) {
    std::string filterText = ::ParseXMLAttribute( element, attributeName, "invalid" );
    filterText = StringToLower( filterText );

    if( filterText == "border" ) {
        return UV_MODE_BORDER;
    } else if( filterText == "clamp" ) {
        return UV_MODE_CLAMP;
    } else if( filterText == "mirror" ) {
        return UV_MODE_MIRROR;
    } else if( filterText == "mirroronce" ) {
        return UV_MODE_MIRROR_ONCE;
    } else if( filterText == "wrap" ) {
        return UV_MODE_WRAP;
    }

    return defaultValue;
}


Sampler::Sampler( FilterInfo filter, UVInfo uvInfo, ID3D11Device* device ) :
    m_minFilter(filter.minFilterMode),
    m_magFilter(filter.magFilterMode) {
    D3D11_SAMPLER_DESC desc;
    memset( &desc, 0, sizeof(D3D11_SAMPLER_DESC) );

    desc.Filter = GetDXFilter( m_minFilter, m_magFilter );
    desc.MaxAnisotropy = 1U; // anistropic filtering (we're not using this... yet)

    // set texture to wrap on UV, but clamp on W (mip).  
    // (if you're smapling 3D wrapping noise, you may want wrap in all dimensions)
    // (if you're doing single sprites, you may want clamp all so you don't get edge bleeding)
    desc.AddressU = GetD3DUVMode( uvInfo.uMode );
    desc.AddressV = GetD3DUVMode( uvInfo.vMode );
    desc.AddressW = GetD3DUVMode( uvInfo.wMode );

    // set that it can sample from all mip levels; 
    desc.MinLOD = -FLT_MAX;
    desc.MaxLOD = FLT_MAX;
    desc.MipLODBias = 0.0f;  // will shift the mip level for a sample by this

    desc.ComparisonFunc = D3D11_COMPARISON_NEVER; // will come into play doing shadow maps; 

    // Create!
    device->CreateSamplerState( &desc, &m_state );

    GUARANTEE_OR_DIE( m_state != nullptr, "(Sampler) Failed to create SamplerState." );
}


Sampler::~Sampler() {
    DX_SAFE_RELEASE( m_state );
}


D3D11_TEXTURE_ADDRESS_MODE Sampler::GetD3DUVMode( const UVMode& mode ) {
    switch( mode ) {
        case( UV_MODE_BORDER ): {
            return D3D11_TEXTURE_ADDRESS_BORDER;
        } case( UV_MODE_CLAMP ): {
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        } case( UV_MODE_MIRROR ): {
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        } case( UV_MODE_MIRROR_ONCE ): {
            return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        } case( UV_MODE_WRAP ): {
            return D3D11_TEXTURE_ADDRESS_WRAP;
        } default: {
            return D3D11_TEXTURE_ADDRESS_WRAP;
        }
    }
}
