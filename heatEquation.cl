#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

const sampler_t s_linear = CLK_FILTER_LINEAR | CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE;	


__kernel void heatEquationStep( float dt
                              , read_only image3d_t volumeTexIn
                              , write_only image3d_t volumeTexOut
                              , float cellWidth
							  , float alpha )
{
  int i = 1 + get_global_id(0);
  int j = 1 + get_global_id(1);
  int k = 1 + get_global_id(2);

  float val  = read_imagef( volumeTexIn, s_linear, (int4)(i, j, k, 0) ).x;
  float valU = read_imagef( volumeTexIn, s_linear, (int4)(i+1, j, k, 0) ).x;
  float valD = read_imagef( volumeTexIn, s_linear, (int4)(i-1, j, k, 0) ).x;
  float valN = read_imagef( volumeTexIn, s_linear, (int4)(i, j+1, k, 0) ).x;
  float valS = read_imagef( volumeTexIn, s_linear, (int4)(i, j-1, k, 0) ).x;
  float valE = read_imagef( volumeTexIn, s_linear, (int4)(i, j, k+1, 0) ).x;
  float valW = read_imagef( volumeTexIn, s_linear, (int4)(i, j, k-1, 0) ).x;

  float negSixVal = -6.0f*val;

  float result = val + dt * alpha * (negSixVal + valU + valD + valN + valS + valE + valW) / (2.0f * cellWidth * cellWidth);
  
  write_imagef( volumeTexOut, (int4)(i, j, k, 0), (float4)(result, 0.0f, 0.0f, 0.0f) );
}