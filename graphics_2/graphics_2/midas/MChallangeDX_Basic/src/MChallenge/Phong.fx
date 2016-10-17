
// Light 정보
cbuffer cbNeverChanges : register( b0 )
{
    float3 Light1;
	float3 Light2;
	float3 Light3;
	float4 LightColor1;
	float4 LightColor2;
	float4 LightColor3;
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
   int		isStoreTex;
   float	Padding[2];
};

SamplerState MeshTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};


Texture2D shaderAlphaTexture	: register( t0 );
Texture2D shaderAmbientTexture	: register( t1 );
Texture2D shaderDiffuseTexture	: register( t2 );
Texture2D shaderSpecularTexture	: register( t3 );
Texture2D shaderNormalTexture	: register( t4 );


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
	// 광원 위치
	//output.eyeLight = Light1;

    return output;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float4 baseAmbient = float4( 0.3, 0.3, 0.3, 1.0 );
	float4 baseDiffuse = float4( 1.0, 1.0, 1.0, 1.0 );
	float4 baseSpecular = float4( 1.0, 1.0, 1.0, 1.0 );


	if(isStoreTex & 1) // Alpha
	{
		//outputColor += float4(0.1f, 0.0f, 0.0f, 0.0f);
	}
	if(isStoreTex & 2) // Ambient
	{
		shaderAmbientTexture.Sampler( baseAmbient. MeshTextureSampler);
	}
	if(isStoreTex & 4) // Diffuse
	{
		outputColor += float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	if(isStoreTex & 8) // Specular
	{
		outputColor += float4(10.0f, 0.0f, 10.0f, 0.0f);
	}
	if(isStoreTex & 16) // Normal
	{
		outputColor += float4(10.0f, 0.0f, 10.0f, 0.0f);
	}


	float4 outputColor;

 	float3 Normal = normalize(input.eyeNorm);
	float3 LightDir1 = normalize(Light1);
	float3 LightDir2 = normalize(Light2);
	float3 LightDir3 = normalize(Light3);
 	float3 ViewDir = normalize(input.eyeView); 

 	float diff1 = saturate(dot(Normal, LightDir1)); 
	float diff2 = saturate(dot(Normal, LightDir2));
	float diff3 = saturate(dot(Normal, LightDir3));

 	float3 Reflect = normalize(2 * diff1 * Normal - LightDir1); 
	float4 spec = pow(saturate(dot(Reflect, ViewDir)), Shininess); // R.V^n

	float4 finalColor = float4( 0.0, 0.0, 0.0, 0.0 );
	finalColor += baseAmbient * Ambient;
	finalColor.rgb = baseDiffuse * Diffuse * (diff1*LightColor1 + diff2*LightColor2 + diff3*LightColor3);
	finalColor += baseSpecular * Specular * spec;
	
	// Premultiplied alpha blending
	outputColor.rgb = finalColor.rgb;
    outputColor.a = 1;




	return outputColor;
}
