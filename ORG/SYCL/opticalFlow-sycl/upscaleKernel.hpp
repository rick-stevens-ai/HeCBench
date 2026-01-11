#include "common.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief bilinear interpolation helper
///////////////////////////////////////////////////////////////////////////////
inline float bilinear_sample(const float *src, int width, int height, int stride,
                              float x, float y) {
  // Clamp to valid range
  x = sycl::clamp(x, 0.0f, (float)(width - 1));
  y = sycl::clamp(y, 0.0f, (float)(height - 1));

  int x0 = (int)sycl::floor(x);
  int y0 = (int)sycl::floor(y);
  int x1 = sycl::min(x0 + 1, width - 1);
  int y1 = sycl::min(y0 + 1, height - 1);

  float fx = x - x0;
  float fy = y - y0;

  float v00 = src[y0 * stride + x0];
  float v10 = src[y0 * stride + x1];
  float v01 = src[y1 * stride + x0];
  float v11 = src[y1 * stride + x1];

  float v0 = v00 * (1.0f - fx) + v10 * fx;
  float v1 = v01 * (1.0f - fx) + v11 * fx;

  return v0 * (1.0f - fy) + v1 * fy;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief upscale one component of a displacement field
/// \param[in]  src     source field
/// \param[in]  width   output field width
/// \param[in]  height  output field height
/// \param[in]  stride  output field stride
/// \param[in]  src_width   input field width
/// \param[in]  src_height  input field height
/// \param[in]  src_stride  input field stride
/// \param[in]  scale   scale factor (multiplier)
/// \param[out] out     result
///////////////////////////////////////////////////////////////////////////////
void UpscaleKernel(const float *src, int width, int height, int stride,
                   int src_width, int src_height, int src_stride,
                   float scale, float *out,
                   const sycl::nd_item<3> &item) {
  const int ix = item.get_global_id(2);
  const int iy = item.get_global_id(1);

  if (ix >= width || iy >= height) return;

  float x = ((float)ix - 0.5f) * 0.5f;
  float y = ((float)iy - 0.5f) * 0.5f;

  // Use bilinear interpolation and scale the result
  out[ix + iy * stride] = bilinear_sample(src, src_width, src_height,
                                           src_stride, x, y) * scale;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief upscale one component of a displacement field, kernel wrapper
/// \param[in]  src         field component to upscale
/// \param[in]  width       field current width
/// \param[in]  height      field current height
/// \param[in]  stride      field current stride
/// \param[in]  newWidth    field new width
/// \param[in]  newHeight   field new height
/// \param[in]  newStride   field new stride
/// \param[in]  scale       value scale factor (multiplier)
/// \param[out] out         upscaled field component
///////////////////////////////////////////////////////////////////////////////
static void Upscale(const float *src, float *pI0_h, float *I0_h, float *src_p,
                    int width, int height, int stride,
                    int newWidth, int newHeight, int newStride, float scale,
                    float *out, sycl::queue &q) {
  sycl::range<3> threads(1, 8, 32);
  sycl::range<3> blocks(1, iDivUp(newHeight, threads[1]),
                        iDivUp(newWidth, threads[2]));

  q.submit([&](sycl::handler &cgh) {
    cgh.parallel_for(sycl::nd_range<3>(blocks * threads, threads),
                     [=](sycl::nd_item<3> item) {
                       UpscaleKernel(src, newWidth, newHeight, newStride,
                                     width, height, stride, scale, out, item);
                     });
  });
}
