//--------------------------------------------------------------------------------------
// Vertex Input - currently using a built in vertex index
struct VertexShaderInput {
    float3 position      : POSITION;
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

cbuffer DebugDrawConstants : register(b4) {
    float4 TINT;
};

//cbuffer LightConstants : register(b5)


Texture2D<float4> tColor : register(t0); // Texutre used for albedo (color) information
SamplerState sColor : register(s0);      // Sampler used for the Albedo texture


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

    float4 localPos = float4( input.position, 1.f );
    float4 worldPos = mul( MODEL, localPos );
    float4 viewPos = mul( VIEW, worldPos );
    float4 clipPos = mul( PROJECTION, viewPos );

    //clipPos.z -= 1.f; // Fake moves units closer to camera (avoids clipping into terrain)

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
    float4 texColor = tColor.Sample( sColor, input.uv );
    float4 finalColor = texColor * input.color * TINT; // component wise multiply to "tint" the output

    clip( finalColor.a - 0.2f ); // Discards any pixel with less than .2f alpha

    return finalColor;
}
