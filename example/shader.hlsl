cbuffer RenderState : register(b0) {
	matrix modelview_matrix : packoffset(c0);
	matrix projection_matrix : packoffset(c4);
};

struct VSInput {
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct VSOutput {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

VSOutput vertex_main(VSInput input)
{
	VSOutput res;

	float4 vpos = mul(input.pos, modelview_matrix);

	res.pos = mul(vpos, projection_matrix);
	res.color = input.color;
	return res;
}


float4 pixel_main(VSOutput input) : SV_TARGET
{
	return input.color;
}