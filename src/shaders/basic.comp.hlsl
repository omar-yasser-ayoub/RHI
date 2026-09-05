RWTexture2D<float4> Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    Output[dispatchThreadId.xy] = float4(1.0, 0.0, 1.0, 1.0);
}
