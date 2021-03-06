//--------------------------------------------------------------------------------------
// Vertex Input - currently using a built in vertex index
struct VertexShaderInput {
    float4 position      : POSITION;
    float4 color         : COLOR;
    float2 uv            : UV;
};


struct VertexToFragment {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
};


//--------------------------------------------------------------------------------------
// Uniform Input
// ------
// Uniform Data is also externally provided data, but instead of changing
// per vertex call, it is constant for all vertices, hence the name "Constant Buffer"
// or "Uniform Buffer".  This is read-only memory; 
//
// I tend to use all cap naming here, as it is effectively a 
// constant from the shader's perspective. 
//
// register(b2) determines the buffer unit/slot to use.  In this case
// we'll say this data is coming from buffer slot 2. 
//
// Last - the layout needs to match the CPU side memory layout, with some 
// caveats.
// Objects MUST start on either a 4-byte, 8-byte, or 16-byte boundary (or multiple of 16).
// Members must be on their own byte alignment, or the next larges (so float2 is on 8-byte alignment
// float3 is on 16 (since there is no 12-byte alignment)
//--------------------------------------------------------------------------------------
cbuffer FrameBuffer : register(b1) {
    float FRAME_COUNT;
    float GAME_TIME;
    float COS_TIME;
    float SIN_TIME;
}


cbuffer CameraConstants : register(b2) {
    float4x4 VIEW;
    float4x4 PROJECTION;
};

cbuffer ModelConstants : register(b3) {
    float4x4 MODEL;
};


Texture2D<float4> tAlbedo : register(t0); // Texutre used for albedo (color) information
SamplerState sAlbedo : register(s0);      // Sampler used for the Albedo texture


//--------------------------------------------------------------------------------------
// Static Constants - i.e. built into the shader
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex Shader
VertexToFragment VertexFunction( VertexShaderInput input ) {
    // Only outputs in the following ranges will display..
    // [-1, 1] on the x (left to right)
    // [-1, 1] on the y (bottom to top)
    // [ 0, 1] on the z 

    // (note: technically clip_space is a homogeneous coordinate
    //  so the above is not 100% accurate, but more on that later)
    float4 localPos = input.position;
    //float4 worldPos = localPos; // Missing step with the MODEL matrix?
    float4 worldPos = mul( MODEL, localPos );
    float4 viewPos = mul( VIEW, worldPos );
    float4 clipPos = mul( PROJECTION, viewPos );

    // For now, we'll just set w to 1, and forward.
    VertexToFragment v2f = (VertexToFragment)0;
    v2f.position = clipPos;
    v2f.color = input.color;
    v2f.uv = input.uv;

    return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned is being drawn to the first bound color target.
float4 FragmentFunction( VertexToFragment input ) : SV_Target0 {
    float4 texColor = tAlbedo.Sample( sAlbedo, input.uv );
    float4 finalColor = texColor * input.color; // component wise multiply to "tint" the output

    return finalColor;
}
