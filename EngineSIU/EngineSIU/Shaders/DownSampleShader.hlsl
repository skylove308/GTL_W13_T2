
Texture2D DownSampleTarget : register(t100);
SamplerState DownSampleSampler : register(s0);

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

float4 main(PS_Input Input) : SV_TARGET
{
    return DownSampleTarget.Sample(DownSampleSampler, Input.UV);
}
