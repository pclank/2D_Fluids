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

kernel void Divergence(float half_rdx, read_only image2d_t vector_field, write_only image2d_t out)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	// Neighbors stuff
	float4 left = read_imagef(vector_field, sampler, coords - (int2)(1, 0));
	float4 right = read_imagef(vector_field, sampler, coords + (int2)(1, 0));
	float4 bottom = read_imagef(vector_field, sampler, coords - (int2)(0, 1));
	float4 top = read_imagef(vector_field, sampler, coords + (int2)(0, 1));

	//float4 div = (float4)(half_rdx * (right.x - left.x + top.y - bottom.y));
	float4 div = (float4)((right.x - left.x + top.y - bottom.y));

	write_imagef(out, coords, div);
}

kernel void Jacobi(float alpha, float rBeta, read_only image2d_t x_vector, read_only image2d_t b_vector, write_only image2d_t x_new)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	// Neighbors stuff
	float4 left = read_imagef(x_vector, sampler, coords - (int2)(1, 0));
	float4 right = read_imagef(x_vector, sampler, coords + (int2)(1, 0));
	float4 bottom = read_imagef(x_vector, sampler, coords - (int2)(0, 1));
	float4 top = read_imagef(x_vector, sampler, coords + (int2)(0, 1));

	float4 bC = read_imagef(b_vector, sampler, coords);

	float4 pixel = (left + right + bottom + top + (alpha * bC)) * rBeta;
	write_imagef(x_new, coords, pixel);
}

kernel void Gradient(float half_rdx, read_only image2d_t pressure, read_only image2d_t w, write_only image2d_t u_new)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	// Neighbors stuff
	//h1texRECTneighbors(p, coords, pL, pR, pB, pT);
	float4 pressure_left = read_imagef(pressure, sampler, coords - (int2)(1, 0));
	float4 pressure_right = read_imagef(pressure, sampler, coords + (int2)(1, 0));
	float4 pressure_bottom = read_imagef(pressure, sampler, coords - (int2)(0, 1));
	float4 pressure_top = read_imagef(pressure, sampler, coords + (int2)(0, 1));

	// TODO: Give meaning to "x" value
	float2 grad = (float2)(pressure_right.x - pressure_left.x, pressure_top.x - pressure_bottom.x) * half_rdx;

	float4 u_new_val = read_imagef(w, sampler, coords);
	u_new_val.xy -= grad;
	write_imagef(u_new, coords, u_new_val);
}

kernel void Vorticity(float half_rdx, read_only image2d_t u, write_only image2d_t vort)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	// Neighbors stuff
	float4 uL = read_imagef(u, sampler, coords - (int2)(1, 0));
	float4 uR = read_imagef(u, sampler, coords + (int2)(1, 0));
	float4 uB = read_imagef(u, sampler, coords - (int2)(0, 1));
	float4 uT = read_imagef(u, sampler, coords + (int2)(0, 1));

	float4 vort_val = (float4)(half_rdx * ((uR.y - uL.y) - (uT.x - uB.x)));
	write_imagef(vort, coords, vort_val);
}

kernel void VorticityConfinement(float half_rdx, float timestep, float dxscale_x, float dxscale_y, read_only image2d_t vort, read_only image2d_t u, write_only image2d_t uNew)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	float4 dxscale = (float4)(dxscale_x, dxscale_y, dxscale_x, dxscale_y);

	// Neighbors stuff
	float4 vL = read_imagef(vort, sampler, coords - (int2)(1, 0));
	float4 vR = read_imagef(vort, sampler, coords + (int2)(1, 0));
	float4 vB = read_imagef(vort, sampler, coords - (int2)(0, 1));
	float4 vT = read_imagef(vort, sampler, coords + (int2)(0, 1));

	float4 vC = read_imagef(vort, sampler, coords);

	float4 force = half_rdx * (float4)(fabs(vT.x) - fabs(vB.x), fabs(vR.x) - fabs(vL.x), fabs(vT.x) - fabs(vB.x), fabs(vR.x) - fabs(vL.x));

	// safe normalize
	float EPSILON = 2.4414e-4; // 2^-12
	float magSqr = max(EPSILON, dot(force, force));
	force = force * rsqrt(magSqr);

	force *= dxscale * vC * (float4)(1, -1, 1, -1);

	float4 uNew_val = read_imagef(u, sampler, coords);

	uNew_val += timestep * force;

	uint seed = x + y * get_image_width(vort);
	//uNew_val = -normalize(uNew_val) * (float4)(AdvancedRandomFloat(seed));

	write_imagef(uNew, coords, uNew_val);
}

kernel void Boundary(float scale, read_only image2d_t u, write_only image2d_t uNew)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	float4 bv = read_imagef(u, sampler, coords);
	if (coords.x == 0)
		bv.x = 0.0f;
	else if (coords.x == get_image_width(u) - 1)
		bv.x = 0.0f;

	if (coords.y == 0)
		bv.y = 0.0f;
	else if (coords.y == get_image_width(u) - 1)
		bv.y = 0.0f;

	//bv = scale * read_imagef(u, sampler, coords);

	write_imagef(uNew, coords, bv);
}

kernel void NeumannBoundary(float scale, read_only image2d_t u, write_only image2d_t uNew)
{
	int x = get_global_id(0);

	int case_d = x / get_image_width(u);
	int rest = x - case_d * get_image_width(u);

	int2 coords = (int2)(0);
	int2 offset = (int2)(0);

	if (case_d == 0)
	{
		coords = (int2)(0, rest);
		offset = (int2)(1, 0);
	}
	else if (case_d == 1)
	{
		coords = (int2)(get_image_width(u) - 1, rest);
		offset = (int2)(-1, 0);
	}
	else if (case_d == 2)
	{
		coords = (int2)(rest, get_image_width(u) - 1);
		offset = (int2)(0, -1);
	}
	else if (case_d == 3)
	{
		coords = (int2)(rest, 0);
		offset = (int2)(0, 1);
	}

	float4 bv = scale * read_imagef(u, sampler, coords + offset);

	write_imagef(uNew, coords, bv);
}

kernel void DisplayConvert(read_only image2d_t src, write_only image2d_t tgt)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	float4 src_val = read_imagef(src, sampler, coords);

	src_val.x = (src_val.x < 0.0f) ? -src_val.x : src_val.x;
	src_val.y = (src_val.y < 0.0f) ? -src_val.y : src_val.y;
	src_val.z = 1.0f;

	write_imagef(tgt, coords, src_val);
}

kernel void Mix(float bias, read_only image2d_t t1, read_only image2d_t t2, write_only image2d_t t3)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	float4 t1_val = read_imagef(t1, sampler, coords);
	float4 t2_val = read_imagef(t2, sampler, coords);

	write_imagef(t3, coords, bias * t1_val - t2_val);
}

kernel void RandomizeTexture(write_only image2d_t tgt)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	uint seed = x + y * get_image_width(tgt);
	float4 tgt_val = (float4)(AdvancedRandomFloat(seed), AdvancedRandomFloat(seed), AdvancedRandomFloat(seed), 1.0f);

	write_imagef(tgt, coords, tgt_val);
}

kernel void RandomForce(float scale, int dir_flag, read_only image2d_t src, write_only image2d_t tgt)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int2 coords = (int2)(x, y);

	uint seed = x + y * get_image_width(tgt);

	/*if (AdvancedRandomFloat(seed) < 0.2f)
		return;*/

	float4 src_val = read_imagef(src, sampler, coords);
	float4 tgt_val = src_val + scale * (float4)(AdvancedRandomFloat(seed), AdvancedRandomFloat(seed), AdvancedRandomFloat(seed), 1.0f);

	// Randomize Direction
	if (dir_flag == 1)
	{
		float dir_val = AdvancedRandomFloat(seed);

		if (dir_val < 0.2f)
			tgt_val.x = -tgt_val.x;
		else if (dir_val >= 0.2f && dir_val < 0.4f)
			tgt_val.y = -tgt_val.y;
		else if (dir_val >= 0.4f)
		{
			tgt_val.x = -tgt_val.x;
			tgt_val.y = -tgt_val.y;
		}
	}

	write_imagef(tgt, coords, tgt_val);
}
