#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderTypes.hpp"
#include "Engine/Utils/XMLUtils.hpp"


enum SampleMode {
    SAMPLE_MODE_INVALID = -1,

    SAMPLE_MODE_POINT,
    SAMPLE_MODE_LINEAR,
    //SAMPLE_MODE_BILINEAR,

    NUM_SAMPLE_MODES,
    SAMPLE_MODE_DEFAULT = SAMPLE_MODE_POINT
};


class Sampler {
    friend class EngineCommon;
    friend class RenderContext;

    public:
    ID3D11SamplerState* GetHandle() const;

    static SampleMode ParseXMLAttribute( const XMLElement& element, const char* attributeName, SampleMode defaultValue );
    static FilterMode ParseXMLAttribute( const XMLElement& element, const char* attributeName, FilterMode defaultValue );
    static UVMode     ParseXMLAttribute( const XMLElement& element, const char* attributeName, UVMode defaultValue );


    private:
    ID3D11SamplerState* m_state = nullptr;
    FilterMode m_minFilter = FILTER_MODE_INVALID;
    FilterMode m_magFilter = FILTER_MODE_INVALID;


    Sampler( FilterInfo filter, UVInfo uvInfo, ID3D11Device* device );
    ~Sampler();

    static D3D11_TEXTURE_ADDRESS_MODE GetD3DUVMode( const UVMode& mode );
};
