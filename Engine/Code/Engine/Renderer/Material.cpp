#include "Engine/Renderer/Material.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"


void Material::SetShader( Shader* shader ) {
    m_shader = shader;
}


void Material::SetShader( const std::string& shaderPath ) {
    Shader* shader = m_renderer->GetOrCreateShader( shaderPath.c_str() );
    m_shader = shader;
}


void Material::SetSampler( Sampler* sampler, int slotIndex /*= 0 */ ) {
    while( (int)m_samplers.size() <= slotIndex ) {
        m_samplers.push_back( nullptr );
    }

    m_samplers[slotIndex] = sampler;
}


void Material::SetTexture( const std::string& filePath /*= ""*/, int slotIndex /*= 0 */ ) {
    while( (int)m_textures.size() <= slotIndex ) {
        m_textures.push_back( "" );
    }
    
    m_textures[slotIndex] = filePath;
}


void Material::SetDiffuse( const std::string& filePath /*= "" */ ) {
    SetTexture( filePath, 0 );
}


void Material::SetNormal( const std::string& filePath /*= "" */ ) {
    SetTexture( filePath, 1 );
}


void Material::SetSpecular( const std::string& filePath /*= "" */ ) {
    SetTexture( filePath, 2 );
}


void Material::SetEmissive( const std::string& filePath /*= "" */ ) {
    SetTexture( filePath, 3 );
}


void Material::SetUBO( const void* data, const int size ) {
    if( m_userBuffer == nullptr ) {
        m_userBuffer = new UniformBuffer( m_renderer );
    }

    m_userBuffer->CopyCPUToGPU( data, size );
}


void Material::SetUBO( UniformBuffer* ubo ) {
    CLEAR_POINTER( m_userBuffer );
    m_userBuffer = ubo;
}


std::string Material::GetID() const {
    return m_materialID;
}


Shader* Material::GetShader() const {
    return m_shader;
}


std::string Material::GetShaderName() const {
    return m_shader->GetFilePath();
}


std::vector<Sampler*> Material::GetSamplers() const {
    return m_samplers;
}


Strings Material::GetTextures() const {
    return m_textures;
}


UniformBuffer* Material::GetUserBuffer() const {
    return m_userBuffer;
}


Material::Material( RenderContext* renderer ) :
    m_renderer( renderer ) {
    SetTexture( "", 0 );
}


Material::Material( RenderContext* renderer, XMLElement* element ) :
    m_renderer( renderer ) {

    m_materialID = ::ParseXMLAttribute( *element, "id", m_materialID );
    GUARANTEE_OR_DIE( m_materialID != "", "ERROR: Material XML is missing required attribute ID" );

    std::string shaderPath = ::ParseXMLAttribute( *element, "shader", "" );
    Shader* shader = m_renderer->GetOrCreateShader( shaderPath.c_str() );
    m_shader = m_renderer->CloneShader( shader );

    XMLElement* child = element->FirstChildElement();

    while( child != nullptr ) {
        std::string name = child->Name();

        if( name == "Texture" ) {
            int index = ::ParseXMLAttribute( *child, "index", 0 );
            std::string texturePath = ::ParseXMLAttribute( *child, "source", "" );
            SetTexture( texturePath, index );
        } else if( name == "Diffuse" ) {
            std::string texturePath = ::ParseXMLAttribute( *child, "source", "" );
            SetDiffuse( texturePath );
        } else if( name == "Normal" ) {
            std::string texturePath = ::ParseXMLAttribute( *child, "source", "" );
            SetNormal( texturePath );
        } else if( name == "Specular" ) {
            std::string texturePath = ::ParseXMLAttribute( *child, "source", "" );
            SetSpecular( texturePath );
        } else if( name == "Emissive" ) {
            std::string texturePath = ::ParseXMLAttribute( *child, "source", "" );
            SetEmissive( texturePath );
        } else if( name == "Sampler" ) {
            int index = ::ParseXMLAttribute( *child, "index", 0 );
            SampleMode samplerMode = Sampler::ParseXMLAttribute( *child, "mode", SAMPLE_MODE_INVALID );

            if( samplerMode != SAMPLE_MODE_INVALID ) {
                Sampler* sampler = m_renderer->GetSampler( samplerMode );
                SetSampler( sampler, index );
            } else {
                FilterInfo defaultFilter;
                FilterInfo filterInfo;
                filterInfo.minFilterMode = Sampler::ParseXMLAttribute( *child, "minFilter", filterInfo.minFilterMode );
                filterInfo.magFilterMode = Sampler::ParseXMLAttribute( *child, "magFilter", filterInfo.magFilterMode );

                UVInfo defaultUV;
                UVInfo uvInfo;
                uvInfo.uMode = Sampler::ParseXMLAttribute( *child, "uMode", uvInfo.uMode );
                uvInfo.vMode = Sampler::ParseXMLAttribute( *child, "vMode", uvInfo.vMode );
                uvInfo.wMode = Sampler::ParseXMLAttribute( *child, "wMode", uvInfo.wMode );

                if( filterInfo != defaultFilter || uvInfo != defaultUV ) {
                    Sampler* sampler = m_renderer->CreateSampler( filterInfo, uvInfo );
                    SetSampler( sampler );
                }
            }
        }

        child = child->NextSiblingElement();
    }
}


Material::~Material() {
    CLEAR_POINTER( m_userBuffer );
}
