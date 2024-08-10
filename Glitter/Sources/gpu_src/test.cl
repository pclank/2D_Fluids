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

	//write_imagef(tgt_tex, (int2)(x, y), (float4)(0.1f * x, 0.1f * y, 0.0f, 1.0f));
	write_imagef(tgt_tex, (int2)(x, y), (float4)(1.0f, 0.0f, 0.0f, 1.0f));
}

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void tex_read_test(read_only image2d_t tgt_tex, __global float* debug_buf)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	float4 pixel = read_imagef(tgt_tex, sampler, (int2)(x, y));

	debug_buf[x + y * get_image_width(tgt_tex)] = pixel.x;
}