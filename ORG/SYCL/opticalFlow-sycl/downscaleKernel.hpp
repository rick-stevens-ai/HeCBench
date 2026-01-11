#include "common.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief downscale image
///
/// Simplified version without texture/image API
/// \param[in]  src     source image data
/// \param[in]  width   output image width
/// \param[in]  height  output image height
/// \param[in]  stride  output image stride
/// \param[in]  src_width   input image width
/// \param[in]  src_stride  input image stride
/// \param[out] out     result
///////////////////////////////////////////////////////////////////////////////
void DownscaleKernel(const float *src, int width, int height, int stride,
                     int src_width, int src_stride, float *out,
                     const sycl::nd_item<3> &item) {
  const int ix = item.get_global_id(2);
  const int iy = item.get_global_id(1);

  if (ix >= width || iy >= height) {
    return;
  }

  int srcx = ix * 2;
  int srcy = iy * 2;

  // Manually sample 4 pixels with boundary clamping
  float val = 0.0f;

  // Sample (srcx, srcy)
  int sx = sycl::min(srcx, src_width - 1);
  int sy = sycl::min(srcy, src_stride - 1);
  val += src[sy * src_stride + sx];

  // Sample (srcx, srcy + 1)
  sx = sycl::min(srcx, src_width - 1);
  sy = sycl::min(srcy + 1, src_stride - 1);
  val += src[sy * src_stride + sx];

  // Sample (srcx + 1, srcy)
  sx = sycl::min(srcx + 1, src_width - 1);
  sy = sycl::min(srcy, src_stride - 1);
  val += src[sy * src_stride + sx];

  // Sample (srcx + 1, srcy + 1)
  sx = sycl::min(srcx + 1, src_width - 1);
  sy = sycl::min(srcy + 1, src_stride - 1);
  val += src[sy * src_stride + sx];

  out[iy * stride + ix] = 0.25f * val;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief downscale image
///
/// \param[in]  src     image to downscale
/// \param[in]  width   image width
/// \param[in]  height  image height
/// \param[in]  stride  image stride
/// \param[out] out     result
///////////////////////////////////////////////////////////////////////////////
static void Downscale(const float *src, float *pI0_h, float *I0_h, float *src_p,
                      int width, int height, int stride,
                      int newWidth, int newHeight, int newStride, float *out,
                      sycl::queue q) {
  sycl::range<3> threads(1, 8, 32);
  sycl::range<3> blocks(1, iDivUp(newHeight, threads[1]),
                        iDivUp(newWidth, threads[2]));

  q.submit([&](sycl::handler &cgh) {
    cgh.parallel_for(sycl::nd_range<3>(blocks * threads, threads),
                     [=](sycl::nd_item<3> item) {
                       DownscaleKernel(src, newWidth, newHeight, newStride,
                                       width, stride, out, item);
                     });
  });
}
