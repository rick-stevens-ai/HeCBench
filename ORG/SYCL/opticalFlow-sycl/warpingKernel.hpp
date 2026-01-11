#include "common.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief bilinear interpolation helper
///////////////////////////////////////////////////////////////////////////////
inline float bilinear_sample_warp(const float *src, int width, int height, int stride,
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
/// \brief warp image with a given displacement field
/// \param[in]  src     source image
/// \param[in]  width   image width
/// \param[in]  height  image height
/// \param[in]  stride  image stride
/// \param[in]  u       horizontal displacement
/// \param[in]  v       vertical displacement
/// \param[out] out     result
///////////////////////////////////////////////////////////////////////////////
void WarpingKernel(const float *src, int width, int height, int stride,
                   const float *u, const float *v, float *out,
                   const sycl::nd_item<3> &item) {
  const int ix = item.get_global_id(2);
  const int iy = item.get_global_id(1);

  const int pos = ix + iy * stride;

  if (ix >= width || iy >= height) return;

  float x = ((float)ix + u[pos]);
  float y = ((float)iy + v[pos]);

  out[pos] = bilinear_sample_warp(src, width, height, stride, x, y);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief warp image with provided vector field, kernel wrapper.
///
/// For each output pixel there is a vector which tells which pixel
/// from a source image should be mapped to this particular output
/// pixel.
/// It is assumed that images and the vector field have the same stride and
/// resolution.
/// \param[in]  src source image
/// \param[in]  w   width
/// \param[in]  h   height
/// \param[in]  s   stride
/// \param[in]  u   horizontal displacement
/// \param[in]  v   vertical displacement
/// \param[out] out warped image
///////////////////////////////////////////////////////////////////////////////
static void WarpImage(const float *src, float *pI0_h, float *I0_h, float *src_p,
                      int w, int h, int s, const float *u,
                      const float *v, float *out, sycl::queue &q) {
  sycl::range<3> threads(1, 6, 32);
  sycl::range<3> blocks(1, iDivUp(h, threads[1]), iDivUp(w, threads[2]));

  q.submit([&](sycl::handler &cgh) {
    cgh.parallel_for(sycl::nd_range<3>(blocks * threads, threads),
                     [=](sycl::nd_item<3> item) {
                       WarpingKernel(src, w, h, s, u, v, out, item);
                     });
  });
}
