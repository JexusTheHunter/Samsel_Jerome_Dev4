
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

OutputVertex main( InputVertex input )
{
    OutputVertex output = (OutputVertex) 0;
    output.xyzw = input.xyzw;
    output.rgba = input.rgba;
    
    return output;
}