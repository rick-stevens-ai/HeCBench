#include "common.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief helper function to clamp and read pixel value
///////////////////////////////////////////////////////////////////////////////
inline float clamped_read(const float *src, int width, int height, int stride,
                          int x, int y) {
  x = sycl::clamp(x, 0, width - 1);
  y = sycl::clamp(y, 0, height - 1);
  return src[y * stride + x];
}

///////////////////////////////////////////////////////////////////////////////
/// \brief compute image derivatives
///
/// \param[in]  I0      source image
/// \param[in]  I1      tracked image
/// \param[in]  width   image width
/// \param[in]  height  image height
/// \param[in]  stride  image stride
/// \param[out] Ix      x derivative
/// \param[out] Iy      y derivative
/// \param[out] Iz      temporal derivative
///////////////////////////////////////////////////////////////////////////////
void ComputeDerivativesKernel(const float *I0, const float *I1,
                              int width, int height, int stride,
                              float *Ix, float *Iy, float *Iz,
                              const sycl::nd_item<3> &item) {
  const int ix = item.get_global_id(2);
  const int iy = item.get_global_id(1);

  const int pos = ix + iy * stride;

  if (ix >= width || iy >= height) return;

  float t0, t1;

  // x derivative using centered difference
  t0 = clamped_read(I0, width, height, stride, ix - 2, iy);
  t0 -= clamped_read(I0, width, height, stride, ix - 1, iy) * 8.0f;
  t0 += clamped_read(I0, width, height, stride, ix + 1, iy) * 8.0f;
  t0 -= clamped_read(I0, width, height, stride, ix + 2, iy);
  t0 /= 12.0f;

  t1 = clamped_read(I1, width, height, stride, ix - 2, iy);
  t1 -= clamped_read(I1, width, height, stride, ix - 1, iy) * 8.0f;
  t1 += clamped_read(I1, width, height, stride, ix + 1, iy) * 8.0f;
  t1 -= clamped_read(I1, width, height, stride, ix + 2, iy);
  t1 /= 12.0f;

  Ix[pos] = (t0 + t1) * 0.5f;

  // t derivative (temporal)
  Iz[pos] = clamped_read(I1, width, height, stride, ix, iy) -
            clamped_read(I0, width, height, stride, ix, iy);

  // y derivative using centered difference
  t0 = clamped_read(I0, width, height, stride, ix, iy - 2);
  t0 -= clamped_read(I0, width, height, stride, ix, iy - 1) * 8.0f;
  t0 += clamped_read(I0, width, height, stride, ix, iy + 1) * 8.0f;
  t0 -= clamped_read(I0, width, height, stride, ix, iy + 2);
  t0 /= 12.0f;

  t1 = clamped_read(I1, width, height, stride, ix, iy - 2);
  t1 -= clamped_read(I1, width, height, stride, ix, iy - 1) * 8.0f;
  t1 += clamped_read(I1, width, height, stride, ix, iy + 1) * 8.0f;
  t1 -= clamped_read(I1, width, height, stride, ix, iy + 2);
  t1 /= 12.0f;

  Iy[pos] = (t0 + t1) * 0.5f;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief compute image derivatives
///
/// \param[in]  I0  source image
/// \param[in]  I1  tracked image
/// \param[in]  w   image width
/// \param[in]  h   image height
/// \param[in]  s   image stride
/// \param[out] Ix  x derivative
/// \param[out] Iy  y derivative
/// \param[out] Iz  temporal derivative
///////////////////////////////////////////////////////////////////////////////
static void ComputeDerivatives(const float *I0, const float *I1, float *pI0_h,
                               float *pI1_h, float *I0_h, float *I1_h,
                               float *src_d0, float *src_d1, int w, int h,
                               int s, float *Ix, float *Iy, float *Iz,
                               sycl::queue q) {
  sycl::range<3> threads(1, 6, 32);
  sycl::range<3> blocks(1, iDivUp(h, threads[1]), iDivUp(w, threads[2]));

  q.submit([&](sycl::handler &cgh) {
    cgh.parallel_for(
        sycl::nd_range<3>(blocks * threads, threads),
        [=](sycl::nd_item<3> item) {
          ComputeDerivativesKernel(I0, I1, w, h, s, Ix, Iy, Iz, item);
        });
  });
}
