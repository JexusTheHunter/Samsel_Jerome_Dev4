
struct InputVertex
{
    float4 xyzw : POSITION;
    float4 rgba : COLOR;
};

struct OutputVertex
{
    float4 xyzw : SV_POSITION;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}