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


Texture2D<float4> tHelm     : register(t0);
Texture2D<float4> tChest    : register(t1);
Texture2D<float4> tShoulder : register(t2);
Texture2D<float4> tLegs     : register(t3);
Texture2D<float4> tFeet     : register(t4);
Texture2D<float4> tWeapon   : register(t5);
Texture2D<float4> tBody     : register(t6);
Texture2D<float4> tEars     : register(t7);
Texture2D<float4> tHair     : register(t8);
SamplerState sColor : register(s0);      // Sampler used for the Color texture


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
    float4 bodyColor          = tBody.Sample( sColor, input.uv );
    float4 earsColor          = tEars.Sample( sColor, input.uv );
    float4 chestColor         = tChest.Sample( sColor, input.uv );
    float4 shoulderColor      = tShoulder.Sample( sColor, input.uv );
    float4 hairColor          = tHair.Sample( sColor, input.uv );
    float4 helmColor          = tHelm.Sample( sColor, input.uv );
    float4 legsColor          = tLegs.Sample( sColor, input.uv );
    float4 feetColor          = tFeet.Sample( sColor, input.uv );
    float4 weaponColor        = tWeapon.Sample( sColor, input.uv );

    // Uses Alpha blending logic
    // Top is srcAlpha1, next is 1-srcAlpha1, next is 1-srcAlpha1-srcAlpha2

    //--THIS ORDER DETERMINES DRAW ORDER--
    float remainingAlpha      = 1.f;

    float weaponFactor        = weaponColor.w * remainingAlpha;
    remainingAlpha           -= weaponFactor;
    float helmFactor          = helmColor.w * remainingAlpha;
    remainingAlpha           -= helmFactor;
    float hairFactor          = hairColor.w * remainingAlpha;
    remainingAlpha           -= hairFactor;
    float shoulderFactor      = shoulderColor.w * remainingAlpha;
    remainingAlpha           -= shoulderFactor;
    float chestFactor         = chestColor.w * remainingAlpha;
    remainingAlpha           -= chestFactor;
    float legsFactor          = legsColor.w * remainingAlpha;
    remainingAlpha           -= legsFactor;
    float feetFactor          = feetColor.w * remainingAlpha;
    remainingAlpha           -= feetFactor;
    float earsFactor          = earsColor.w * remainingAlpha;
    remainingAlpha           -= earsFactor;
    float bodyFactor          = bodyColor.w * remainingAlpha;
    remainingAlpha           -= bodyFactor;
    //------------------------------------

    float4 finalBodyColor     = bodyColor     * bodyFactor;
    float4 finalEarsColor     = earsColor     * earsFactor;
    float4 finalChestColor    = chestColor    * chestFactor;
    float4 finalShoulderColor = shoulderColor * shoulderFactor;
    float4 finalHairColor     = hairColor     * hairFactor;
    float4 finalHelmColor     = helmColor     * helmFactor;
    float4 finalLegsColor     = legsColor     * legsFactor;
    float4 finalFeetColor     = feetColor     * feetFactor;
    float4 finalWeaponColor   = weaponColor   * weaponFactor;

    float4 finalTexColor = finalBodyColor + finalEarsColor + finalChestColor + finalShoulderColor + finalHairColor + finalHelmColor + finalLegsColor + finalFeetColor + finalWeaponColor;
    //float4 finalColor = float4( finalTexColor.rgb * input.color.rgb, input.color.a * (1.0 - remainingAlpha) ); // component wise multiply to "tint" the output
    float4 finalColor = finalTexColor * input.color; // component wise multiply to "tint" the output

    return finalColor;
}
