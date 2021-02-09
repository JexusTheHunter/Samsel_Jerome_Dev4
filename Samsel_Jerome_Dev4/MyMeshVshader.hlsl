#pragma pack_matrix(row_major)

struct InputVertex
{
    float3 xyz : POSITION;
    float3 rgb : TEXCOORD;
    float3 nrm : NORMAL;
};

struct OutputVertex
{
    float4 xyzw : SV_POSITION;
    float4 rgba : OCOLOR;
};

cbuffer SHADER_VARS : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

OutputVertex main( InputVertex input )
{
    OutputVertex output = (OutputVertex) 0;
    output.xyzw = float4(input.xyz, 1);
    output.rgba.rbg = input.nrm;
    
    output.xyzw = mul(output.xyzw, worldMatrix);
    output.xyzw = mul(output.xyzw, viewMatrix);
    output.xyzw = mul(output.xyzw, projectionMatrix);
    
    return output;
}