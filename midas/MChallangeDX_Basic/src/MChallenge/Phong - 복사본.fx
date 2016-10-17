
// Light 정보
cbuffer cbNeverChanges : register( b0 )
{
    float4 Light;
};

cbuffer cbChangesEveryFrame : register( b1 )
{
    matrix WorldViewProj;
   matrix WorldView;
    matrix World;
   // 월드 좌표계에서의 Eye 위치
   float3 Eye;
   // 월드 좌표계에서의 광원 위치
   float4 PointLight1;
   float4 PointLight2;
   float4 rgbV;
   float4 rgbV_m;
};
cbuffer cbMaterialProperties : register( b2 )
{

   float4  Emissive;
   float4  Ambient;        // 16 bytes
   float4  Diffuse;        // 16 bytes
   float4  Specular;       // 16 bytes
   float   Shininess;      // 4 bytes
   //float Opacity;
   //16 bytes 단위로 Packing 하기 위한 Padding
   int      isStoreTex;
   float   Opacity;
   float   Padding;
};

SamplerState MeshTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinear : register( s0 );

Texture2D AlphaTexture      : register( t0 );
Texture2D AmbientTexture   : register( t1 );
Texture2D DiffuseTexture   : register( t2 );
Texture2D SpecularTexture   : register( t3 );
Texture2D NormalTexture      : register( t4 );


struct VS_INPUT
{
    float3 Pos          : POSITION;         //position
    float3 Norm         : NORMAL;           //normal
    float2 Tex          : TEXCOORD0;        //texture coordinate
   float3 tangent      : TANGENT;
   float3 bitangent   : BITANGENT;
};

struct PS_INPUT
{
   // view 좌표계에서의 위치
    float4 Pos : SV_POSITION;
   float2 Tex : TEXCOORD0;
   // 월드 좌표계의 노말
   float3 eyeNorm : TEXCOORD2;
   // 월드 좌표계에서 물체 정점으로부터 Eye까지의 벡터
   float3 eyeView : TEXCOORD3;
   // Direction Light의 위치
   // Direction Light 빛의 방향의 반대
   float3 eyeLight : TEXCOORD4;

   float3 tangent      : TEXCOORD5;
   float3 bitangent   : TEXCOORD6;
   float3 worldpos : TEXCOORD7;
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

   output.Tex = input.Tex;
   output.worldpos = PosWorld;

    return output;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
   float4 baseAmbient = float4( 0.1, 0.1, 0.1, 0.1 );
   float4 baseDiffuse = float4( 0.7, 0.7, 0.7, 1.0 );
   float4 baseSpecular = float4( 1.0, 1.0, 1.0, 1.0 );
   float4 alpha = float4(0.0f, 0.0f, 0.0f, 1.0f);
   float4 baseBump   = float4(0.0f, 0.0f, 0.0f, 1.0f);

   if(isStoreTex & 1) // Alpha
   {
      alpha = AlphaTexture.Sample(samLinear, input.Tex);
   }
   if(isStoreTex & 2) // Ambient
   {
      baseAmbient = AmbientTexture.Sample(samLinear, input.Tex);
   }
   if(isStoreTex & 4) // Diffuse
   {
      baseDiffuse = DiffuseTexture.Sample(samLinear, input.Tex);
      baseAmbient = baseDiffuse*0.5;
   }
   if(isStoreTex & 8) // Specular
   {
      baseSpecular = SpecularTexture.Sample(samLinear, input.Tex);
   }
   if(isStoreTex & 16) // Normal
   {
      baseBump = NormalTexture.Sample(samLinear, input.Tex);
   }


   float4 outputColor;

    float3 Normal = normalize(input.eyeNorm + baseBump.x *input.tangent + baseBump.y * input.bitangent );
   float3 LightDir = normalize(input.eyeLight);
    float3 ViewDir = normalize(input.eyeView); 

    float4 diff = saturate(dot(Normal, LightDir)); 
 
    float3 Reflect = normalize(2 * diff * Normal - LightDir); 
   float4 spec = pow(saturate(dot(Reflect, ViewDir)), Shininess); // R.V^n

   float4 finalColor = float4( 0.0, 0.0, 0.0, 1.0 );
   finalColor += baseAmbient * Ambient;
   finalColor += baseDiffuse * Diffuse * diff;
   finalColor += baseSpecular * Specular * spec;
   float3 worldNorm = normalize(input.eyeNorm);

   float3 worldViewToLight1 = normalize(PointLight1 - input.worldpos);
   float3 worldViewToLight2 = normalize(PointLight2 - input.worldpos);

   float4 LightColor1=rgbV;
   float4 LightColor2=float4(0.0,0.0,0.0,1.0);
   // 월드 노말과 월드 정점에서 빛점 벡터
   float4 vPointLightColor1 = LightColor1 * (saturate(dot(worldNorm, worldViewToLight1)));
   float4 vPointLightColor2 = LightColor2 * (saturate(dot(worldNorm, worldViewToLight2)));
 //  finalColor.rgb*=rgbV;
  // finalColor.rgb+=vPointLightColor1;
  // finalColor.rgb+=vPointLightColor2;
   outputColor.rgb = finalColor.rgb * Opacity *  rgbV_m;
   outputColor.a = Opacity;

   return outputColor;
}