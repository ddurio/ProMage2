#include "Engine/Renderer/GPUMesh.hpp"

#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Vertex/VertexUtils.hpp"


GPUMesh::GPUMesh( RenderContext* renderContext ) {
    m_renderer = renderContext;
    m_vertexBuffer = new VertexBuffer( m_renderer );
    m_indexBuffer = new IndexBuffer( m_renderer );
}


GPUMesh::~GPUMesh() {
    CLEAR_POINTER( m_vertexBuffer );
    CLEAR_POINTER( m_indexBuffer );
}


void GPUMesh::CopyVertsFromCPUMesh( const CPUMesh* mesh, const std::string& shaderPath /*= "" */ ) {
    int numVerts = mesh->GetVertexCount();

    CopyFromMasterCallback copyCB = nullptr;
    size_t stride = 0;

    if( shaderPath == "" ) {
        Shader* shader = m_renderer->GetCurrentShader();
        shader->GetVertexInfo( copyCB, stride );
    } else {
        Shader* shader = m_renderer->GetOrCreateShader( shaderPath );
        shader->GetVertexInfo( copyCB, stride );
    }

    void* vertList = malloc( stride * numVerts );
    copyCB( vertList, mesh->m_verts );

    m_vertexBuffer->CopyCPUToGPU( vertList, numVerts, stride );
    m_indexBuffer->CopyCPUToGPU( &(mesh->m_indices[0]), mesh->GetIndexCount() );

    SetDrawCall( mesh->IsUsingIndexBuffer(), mesh->GetElementCount() );

    free( vertList );
}


void GPUMesh::SetDrawCall( bool useIndexBuffer, int numElements ) {
    m_useIndexBuffer = useIndexBuffer;
    m_numElements = numElements;
}


void GPUMesh::SetDefaultMaterial( const std::string& defaulMaterial ) {
    m_defaultMaterial = defaulMaterial;
}

int GPUMesh::GetElementCount() const {
    return m_numElements;
}


std::string GPUMesh::GetDefaultMaterial() const {
    return m_defaultMaterial;
}
