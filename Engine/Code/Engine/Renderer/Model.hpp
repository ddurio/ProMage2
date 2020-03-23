#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Matrix44.hpp"


class GPUMesh;
class RenderContext;

class Model {
    public:
    Model() {};
    explicit Model( const std::string& filePath, RenderContext* renderer );
    ~Model();

    std::string GetMaterial() const;
    GPUMesh* GetMesh() const;
    Matrix44 GetModelMatrix() const;

    void SetMesh( GPUMesh* newMesh );
    void SetMaterial( const std::string& newMaterial );
    void SetModelMatrix( Matrix44 matrix );

    private:
    GPUMesh* m_mesh = nullptr;
    std::string m_materialName = "";
    Matrix44 m_modelMatrix = Matrix44::IDENTITY;

    void LoadObjFile( const std::string& filePath, bool invert, float scale, const std::string& transform );
};
