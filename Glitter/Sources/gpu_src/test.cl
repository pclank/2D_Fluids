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
	write_imagef(tgt_tex, (int2)(x, y), (float4)(0.1 * x, 0.0f, 0.0f, 0.0f));
}