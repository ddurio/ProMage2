#include "Engine/Renderer/Model.hpp"

#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Utils/XMLUtils.hpp"


Model::Model( const std::string& filePath, RenderContext* renderer ) {
    const CPUMesh* builder = renderer->GetOrCreateMesh( filePath );

    m_materialName = builder->GetDefaultMaterial();
    Material* material = renderer->GetOrCreateMaterial( m_materialName );
    std::string shaderPath = material->GetShaderName();

    m_mesh = new GPUMesh( renderer );
    m_mesh->CopyVertsFromCPUMesh( builder, shaderPath );
    m_mesh->SetDefaultMaterial( m_materialName );
}


Model::~Model() {
    CLEAR_POINTER( m_mesh );
}


std::string Model::GetMaterial() const {
    return m_materialName;
}


GPUMesh* Model::GetMesh() const {
    return m_mesh;
}


Matrix44 Model::GetModelMatrix() const {
    return m_modelMatrix;
}


// Model now OWNS newMesh -- will be deleted by the Model
void Model::SetMesh( GPUMesh* newMesh ) {
    CLEAR_POINTER( m_mesh );
    m_mesh = newMesh;
}


void Model::SetMaterial( const std::string& newMaterial ) {
    m_materialName = newMaterial;
}


void Model::SetModelMatrix( Matrix44 matrix ) {
    m_modelMatrix = matrix;
}
