
// Light 정보
cbuffer cbNeverChanges : register( b0 )
{
    float3 Light;
};

cbuffer cbChangesEveryFrame : register( b1 )
{
    matrix WorldViewProj;
	matrix WorldView;
    matrix World;
	// Eye 위치
	float3 Eye;
};
cbuffer cbMaterialProperties : register( b2 )
{
   float4  Emissive;
   float4  Ambient;        // 16 bytes
   float4  Diffuse;        // 16 bytes
   float4  Specular;       // 16 bytes
   float	Shininess;		// 4 bytes
   //16 bytes 단위로 Packing 하기 위한 Padding
   float	Padding[3];
};
struct VS_INPUT
{
    float3 Pos          : POSITION;         //position
    float3 Norm         : NORMAL;           //normal
    float2 Tex          : TEXCOORD0;        //texture coordinate
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 eyeNorm : TEXCOORD2;
	float3 eyeView : TEXCOORD3;
	float3 eyeLight : TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

    output.Pos = mul( float4(input.Pos,1), WorldViewProj );

	output.eyeNorm = mul(input.Norm, World); // N
	float3 PosWorld = normalize(mul(input.Pos, World)); 
    output.eyeView = Eye - PosWorld; // V
	output.eyeLight = Light;

    return output;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float4 baseAmbient = float4( 0.3, 0.3, 0.3, 1.0 );
	float4 baseDiffuse = float4( 0.7, 0.7, 0.7, 1.0 );
	float4 baseSpecular = float4( 1.0, 1.0, 1.0, 1.0 );

	float4 outputColor;

 	float3 Normal = normalize(input.eyeNorm);
	float3 LightDir = normalize(input.eyeLight);
 	float3 ViewDir = normalize(input.eyeView); 

 	float4 diff = saturate(dot(Normal, LightDir)); 
 
 	float3 Reflect = normalize(2 * diff * Normal - LightDir); 
	float4 spec = pow(saturate(dot(Reflect, ViewDir)), Shininess); // R.V^n

	float4 finalColor = float4( 0.0, 0.0, 0.0, 0.0 );
	finalColor += baseAmbient * Ambient;
	finalColor += baseDiffuse * Diffuse * diff;
	finalColor += baseSpecular * Specular * spec;

	// Premultiplied alpha blending
	outputColor.rgb = finalColor.rgb;
    outputColor.a = 1;

	return outputColor;
}
