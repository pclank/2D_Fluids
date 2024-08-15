__global uint sd = 0x13567528;

uint WangHash(uint seed)
{
	seed *= 17;
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

uint WangHash2()
{
	sd *= 17;
	sd = (sd ^ 61) ^ (sd >> 16);
	sd *= 9;
	sd = sd ^ (sd >> 4);
	sd *= 0x27d4eb2d;
	sd = sd ^ (sd >> 15);
	return sd;
}

// local seed
uint RandomUInt(uint seed)
{
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return seed;
}

uint RandomUInt2()
{
	sd ^= sd << 13;
	sd ^= sd >> 17;
	sd ^= sd << 5;
	return sd;
}

uint WangHashAd(uint s) { s = (s ^ 61) ^ (s >> 16), s *= 9, s = s ^ (s >> 4), s *= 0x27d4eb2d, s = s ^ (s >> 15); return s; }
uint RandomIntAd(uint s) { s ^= s << 13, s ^= s >> 17, s ^= s << 5; return s; }
float RandomFloatAd(uint s) { return RandomIntAd(s) * 2.3283064365387e-10f; /* = 1 / (2^32-1) */ }

float RandomFloat(uint seed)
{
	//return RandomUInt(seed) * 2.3283064365387e-10f;
	return RandomUInt(WangHash(seed + 1)) * 2.3283064365387e-10f;
	//return RandomUInt(WangHash2() * seed) * 2.3283064365387e-10f;
	//return RandomUInt2() * 2.3283064365387e-10f;
}

float AdvancedRandomFloat(uint seed)
{
	return RandomUInt(WangHash(RandomUInt(WangHash2()) + seed) + 1) * 2.3283064365387e-10f;
}

float4 lerp(float4 a, float4 b, float t)
{
	return (1.0f - t) * a + t * b;
}

kernel void test(__global int* test_buf)
{
	int x = get_global_id(0);
	//int y = get_global_id(1);

	test_buf[x] = x;
}

kernel void tex_test(write_only image2d_t tgt_tex)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	write_imagef(tgt_tex, (int2)(x, y), (float4)(0.001f * x, 0.001f * y, 0.0f, 1.0f));
	//write_imagef(tgt_tex, (int2)(x, y), (float4)(1.0f, 0.0f, 0.0f, 1.0f));
}

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void tex_read_test(read_only image2d_t tgt_tex, __global float* debug_buf)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	float4 pixel = read_imagef(tgt_tex, sampler, (int2)(x, y));

	debug_buf[x + y * get_image_width(tgt_tex)] = pixel.x;
}

kernel void AvectFluid(float timestep, float rdx,
	// 1 / grid scale
	read_only image2d_t u,		// input velocity
	read_only image2d_t xOld,	// qty to advect
	write_only image2d_t xNew	// advected qty
)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	// follow the velocity field "back in time"
	float2 pos = (float2)(coords.x, coords.y) - timestep * rdx * read_imagef(u, sampler, coords).xy;

	// find 4 closest texel positions
	float4 st;

	st.xy = floor(pos - 0.5f) + 0.5f;
	st.zw = st.xy + 1.0f;

	float2 t = pos - st.xy;

	float4 tex11 = read_imagef(xOld, sampler, st.xy);
	float4 tex21 = read_imagef(xOld, sampler, st.zy);
	float4 tex12 = read_imagef(xOld, sampler, st.xw);
	float4 tex22 = read_imagef(xOld, sampler, st.zw);

	// bilinearly interpolate
	float4 interpolated = lerp(lerp(tex11, tex21, t.x), lerp(tex12, tex22, t.x), t.y);

	uint seed = x + y * get_image_width(xOld);
	//interpolated = (float4)(AdvancedRandomFloat(seed));

	//interpolated = (AdvancedRandomFloat(seed) <= 0.5f) ? interpolated : (float4)(AdvancedRandomFloat(seed));

	write_imagef(xNew, coords, interpolated);
}

kernel void CopyTexture(read_only image2d_t a, write_only image2d_t b)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	float4 pixel = read_imagef(a, sampler, coords);

	write_imagef(b, coords, pixel);
}
