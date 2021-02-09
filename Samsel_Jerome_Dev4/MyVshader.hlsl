#pragma pack_matrix(row_major)

struct InputVertex
{
    float4 xyzw : POSITION;
    float4 rgba : COLOR;
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
    output.xyzw = input.xyzw;
    output.rgba = input.rgba;
    
    output.xyzw = mul(output.xyzw, worldMatrix);
    output.xyzw = mul(output.xyzw, viewMatrix);
    output.xyzw = mul(output.xyzw, projectionMatrix);
    
    return output;
}