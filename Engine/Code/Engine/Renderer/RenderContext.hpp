#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderTypes.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Vertex/VertexUtils.hpp"

#include "filesystem"
#include "map"
#include "shared_mutex"
#include "vector"


struct FrameBuffer {
    float frameCount;
    float gameTimeSeconds;
    float cosTime;
    float sinTime;
};


struct LightDesc {
    Rgba color = Rgba::CLEAR;

    Vec3 position;
    float isDirectional;
    
    Vec3 direction;
    float padding0;

    Vec3 diffuseAttentuation = Vec3( 0.f, 1.f, 0.f );
    float padding1;

    Vec3 specularAttenuation = Vec3( 0.f, 1.f, 0.f );
    float padding2;
};


struct LightBuffer {
    Rgba ambient;
    Rgba emissive;

    float specularFactor;
    float specularPower;
    float renderMode = 0.f;
    float padding0;

    LightDesc lights[8];
};


constexpr char SHADER_CONSOLE[] = "BuiltIn/DevConsole";
constexpr char SHADER_INVALID[] = "BuiltIn/Invalid";
constexpr char SHADER_UNLIT[]   = "BuiltIn/Unlit";
constexpr char SHADER_LIT[]     = "BuiltIn/Lit";


class BitmapFont;
class CPUMesh;
class GPUMesh;
class ImGuiSystem;
class IndexBuffer;
class JobSystem;
class Material;
class Model;
class Sampler;
class Shader;
class Texture2D;
class TextureView2D;
class UniformBuffer;
class VertexBuffer;
class WindowContext;


class RenderContext {
    friend class RenderBuffer;

	public:
	void Startup( WindowContext* windowContext );
	void Shutdown();

	void BeginFrame();
	void EndFrame();

	void ClearRenderTarget( const Rgba& clearColor );
	void BeginCamera( Camera* camera );
	void EndCamera( const Camera* camera );

	void DrawVertexArray( int numVertexes, const VertexPCU* vertexes, BlendMode mode = BLEND_MODE_ALPHA );
    void DrawVertexArray( const VertexList& vertexes, BlendMode mode = BLEND_MODE_ALPHA );
    void DrawMesh( const GPUMesh* mesh, const Matrix44& modelMatrix = Matrix44::ZERO, BlendMode mode = BLEND_MODE_ALPHA );
    void DrawModel( const Model* model );

    Material* GetOrCreateMaterial( const std::string& filePath );
    CPUMesh* GetOrCreateMesh( const std::string& filePath );
    void AddMeshToDatabase( const std::string& filePath, CPUMesh* existingMesh );
    void GetMeshXMLParams( const std::string& filePath, std::string& outObjFile, bool& outInvert, float& outScale, std::string& outTransform, std::string& outMatName ) const;

    void CreateTexture( const std::string& filePath );
    TextureView2D* GetOrCreateTextureView2D( const std::string& filePath );
    TextureView2D* GetOrCreateTextureView2D( Image* existingImage );
    TextureView2D* GetOrCreateTextureView2D( const Rgba& textureColor );
    BitmapFont* GetOrCreateBitmapFont( const std::string& fontName );
    Shader* GetOrCreateShader( const std::string& shaderFilePath );
    Shader* CloneShader( const Shader* cloneFrom );
    Sampler* CreateSampler( FilterInfo filter, UVInfo uvInfo, SampleMode sampleMode = SAMPLE_MODE_INVALID );
    Sampler* GetSampler( SampleMode sampleMode ) const;

    void DestroyTextureView2D( Image* existingImage );

    IntVec2 GetBackBufferDimensions() const;
    TextureView2D* GetCurrentRenderTarget() const;
    TextureView2D* GetCurrentDepthBuffer() const;
    std::string GetTextureID( const TextureView2D* viewToFind ) const;
    Shader* GetCurrentShader() const;

    TextureView2D* GetOrCreateRenderTarget( const std::string& targetID, const IntVec2& dimensions = IntVec2::NEGONE );
    TextureView2D* GetOrCreateDepthTarget( const std::string& targetID, const IntVec2& dimensions = IntVec2::NEGONE );
    void ClearDepthStencilTarget( float depth = 1.f, unsigned int stencil = 0u ) const;

    LightDesc GetDynamicLight( int lightIndex ) const;

    void SetAmbientLight( const Rgba& color );
    void SetEmissiveLight( const Rgba& color );
    void SetRenderMode( int renderMode );
    void SetDynamicLight( int lightIndex, const LightDesc& light );

    void BindModelMatrix( Matrix44 model = Matrix44::IDENTITY );
    void BindMaterial( const std::string& materialName );
    void BindMaterial( const Material* material );
    void BindTexture( const std::string& filePath = "", int slotIndex = 0 );
    void BindTextureView2D( const TextureView2D* textureView, int slotIndex = 0 );
    void BindShader( const std::string& shaderPath );
    void BindShader( Shader* shader );
    void BindSampler( Sampler* sampler, int slotIndex = 0 );
    void BindUBO( const UniformBuffer* ubo, int slotIndex = 0 );

    void ApplyEffect( TextureView2D* source, const std::string& materialID, TextureView2D* destination = nullptr );
    void ApplyEffect( TextureView2D* source, Material* effectMaterial, TextureView2D* destination = nullptr );
    void CopyD3DResource( const TextureView2D* source, const TextureView2D* destination );

    // D3D11
    const ID3D11Device* GetRenderDevice() const;
    static void SetD3DDebugName( ID3D11DeviceChild* child, const std::string& name );
    void SetupImGuiInternals( ImGuiSystem* imGui ) const;

    bool RequestScreenshot( EventArgs& args );

    static const char* DEFAULT_SHADER_UNLIT;
    static const char* DEFAULT_SHADER_LIT;
    static const char* DEFAULT_SHADER_INVALID;


	private:
    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_d3dContext = nullptr;
    IDXGISwapChain* m_d3dSwapChain = nullptr;
    ID3D11InfoQueue* m_d3dQueue = nullptr;
    Strings m_d3dPrintedWarnings;

    Texture2D* m_frameBufferTexture = nullptr;
    TextureView2D* m_frameBufferView = nullptr;
    std::string m_frameBufferID = "renderContext/frameBufferView";
    TextureView2D* m_depthBuffer = nullptr;

    Camera* m_effectCamera = nullptr;
    Camera* m_boundCamera = nullptr;
    Shader* m_boundShader = nullptr;
    UniformBuffer* m_frameUBO = nullptr;
    UniformBuffer* m_modelUBO = nullptr;
    UniformBuffer* m_lightUBO = nullptr;
    VertexBuffer* m_immediateVBO = nullptr;

    Matrix44 m_modelMatrix = Matrix44();
    LightBuffer m_lightBuffer;
    bool m_lightBufferDirty = true;

    std::map<std::string, Material*> m_loadedMaterials;
    std::map<std::string, CPUMesh*> m_loadedMeshes;
    std::map<std::string, TextureView2D*> m_loadedTextures;
    std::map<std::string, BitmapFont*> m_loadedFonts;
    std::map<std::string, Shader*> m_loadedShaders;
    std::vector<Sampler*> m_loadedSamplers;

    mutable std::recursive_mutex m_renderMutex;

    DevConsoleChannel m_consoleChannel = 0x00;
    float m_frameCount = 0;

    JobSystem* m_screenshotJobSys = nullptr;
    std::filesystem::path m_screenshotRequested = "";


    void StartupD3D( void* windowHandleIn );
    void StartupD3DDebug();
    void PrintD3DDebug();
    void ShutdownD3DDebug() const;

    void BindVertexStream( VertexBuffer* vbo );
    void BindIndexStream( IndexBuffer* ibo );

    void Draw( int vertCount, int byteOffset = 0, bool useIndexBuffer = false );

    Material* CreateMaterial( const std::string& filePath );
    CPUMesh* CreateMesh( const std::string& filePath );
    TextureView2D* CreateTextureView2D( const std::string& filePath );
    TextureView2D* CreateTextureView2D( Image* existingImage );
    TextureView2D* CreateTextureView2D( const Rgba& textureColor );
    TextureView2D* CreateRenderTarget( const std::string& targetID, const IntVec2& dimensions = IntVec2::NEGONE );
    TextureView2D* CreateDepthTarget( const std::string& targetID, const IntVec2& dimensions = IntVec2::NEGONE );
    BitmapFont* CreateBitmapFont( const std::string& fontName );
    Shader* CreateShader( const std::string& shaderFilePath );
};
