#pragma once
#include "Engine/Core/EngineCommon.hpp"

class CPUMesh;
class IndexBuffer;
class RenderContext;
class VertexBuffer;


class GPUMesh {
    friend class RenderContext;

    public:
    GPUMesh( RenderContext* renderContext );
    ~GPUMesh();

    void CopyVertsFromCPUMesh( const CPUMesh* mesh, const std::string& shaderPath = "" );

    void SetDrawCall( bool useIndexBuffer, int numElements );
    void SetDefaultMaterial( const std::string& defaulMaterial );

    int GetElementCount() const;
    std::string GetDefaultMaterial() const;

    private:
    RenderContext* m_renderer = nullptr;
    VertexBuffer* m_vertexBuffer = nullptr;
    IndexBuffer* m_indexBuffer = nullptr;

    int m_numElements = 0;
    bool m_useIndexBuffer = false;
    std::string m_defaultMaterial = "";
};
