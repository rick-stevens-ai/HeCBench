#include <sycl/sycl.hpp>

void ccsd_tengy_gpu(sycl::queue &q,
    const float * __restrict f1n,    const float * __restrict f1t,
    const float * __restrict f2n,    const float * __restrict f2t,
    const float * __restrict f3n,    const float * __restrict f3t,
    const float * __restrict f4n,    const float * __restrict f4t,
    const float * __restrict dintc1, const float * __restrict dintx1,const float * __restrict t1v1,
    const float * __restrict dintc2, const float * __restrict dintx2, const float * __restrict t1v2,
    const float * __restrict eorb,   const float eaijk,
    float * __restrict emp4i, float * __restrict emp5i,
    float * __restrict emp4k, float * __restrict emp5k,
    const int ncor, const int nocc, const int nvir);

void ccsd_trpdrv(sycl::queue &q,
    float * __restrict f1n, float * __restrict f1t,
    float * __restrict f2n, float * __restrict f2t,
    float * __restrict f3n, float * __restrict f3t,
    float * __restrict f4n, float * __restrict f4t,
    float * __restrict eorb,
    int    * __restrict ncor_, int * __restrict nocc_, int * __restrict nvir_,
    float * __restrict emp4_, float * __restrict emp5_,
    int    * __restrict a_, int * __restrict i_, int * __restrict j_, int * __restrict k_, int * __restrict klo_,
    float * __restrict tij, float * __restrict tkj, float * __restrict tia, float * __restrict tka,
    float * __restrict xia, float * __restrict xka, float * __restrict jia, float * __restrict jka,
    float * __restrict kia, float * __restrict kka, float * __restrict jij, float * __restrict jkj,
    float * __restrict kij, float * __restrict kkj,
    float * __restrict dintc1, float * __restrict dintx1, float * __restrict t1v1,
    float * __restrict dintc2, float * __restrict dintx2, float * __restrict t1v2)
{
  float emp4 = *emp4_;
  float emp5 = *emp5_;

  float emp4i = 0.0;
  float emp5i = 0.0;
  float emp4k = 0.0;
  float emp5k = 0.0;

  const int ncor = *ncor_;
  const int nocc = *nocc_;
  const int nvir = *nvir_;

  /* convert from Fortran to C offset convention... */
  const int k   = *k_ - 1;
  const int a   = *a_ - 1;
  const int i   = *i_ - 1;
  const int j   = *j_ - 1;

  const float eaijk = eorb[a] - (eorb[ncor+i] + eorb[ncor+j] + eorb[ncor+k]);

  ccsd_tengy_gpu(q, f1n, f1t, f2n, f2t, f3n, f3t, f4n, f4t,
      dintc1, dintx1, t1v1, dintc2, dintx2, t1v2,
      eorb, eaijk, &emp4i, &emp5i, &emp4k, &emp5k,
      ncor, nocc, nvir);

  emp4 += emp4i;
  emp5 += emp5i;

  if (*i_ != *k_) {
    emp4 += emp4k;
    emp5 += emp5k;
  }

  *emp4_ = emp4;
  *emp5_ = emp5;

  return;
}

