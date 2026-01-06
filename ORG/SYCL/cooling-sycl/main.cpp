#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <sycl/sycl.hpp>

typedef float Real;

// Primordial hydrogen/helium cooling curve derived according to Katz et al. 1996.
// set heat_flag to 1 for photoionization & heating

Real primordial_cool(Real n, Real T, int heat_flag)
{
  Real n_h, Y, y, g_ff, cool;
  Real n_h0, n_hp, n_he0, n_hep, n_hepp, n_e, n_e_old;
  Real alpha_hp, alpha_hep, alpha_d, alpha_hepp, gamma_eh0, gamma_ehe0, gamma_ehep;
  Real le_h0, le_hep, li_h0, li_he0, li_hep, lr_hp, lr_hep, lr_hepp, ld_hep, l_ff;
  Real gamma_lh0, gamma_lhe0, gamma_lhep, e_h0, e_he0, e_hep, H;
  int n_iter;
  Real diff, tol;

  Y = 0.24f; //helium abundance by mass
  y = Y/(4 - 4*Y);

  // set the hydrogen number density
  n_h = n;

  // calculate the recombination and collisional ionization rates
  // (Table 2 from Katz 1996)
  alpha_hp = (8.4e-11f) * (1.0f / sycl::sqrt(T)) *
             sycl::pow((T / 1e3f), (-0.2f)) *
             (1.0f / (1.0f + sycl::pow((T / 1e6f), (0.7f))));
  alpha_hep = (1.5e-10f) * (sycl::pow(T, (-0.6353f)));
  alpha_d = (1.9e-3f) * (sycl::pow(T, (-1.5f))) *
            sycl::exp(-470000.0f / T) * (1.0f + 0.3f * sycl::exp(-94000.0f / T));
  alpha_hepp = (3.36e-10f) * (1.0f / sycl::sqrt(T)) *
               sycl::pow((T / 1e3f), (-0.2f)) *
               (1.0f / (1.0f + sycl::pow((T / 1e6f), (0.7f))));
  gamma_eh0 = (5.85e-11f) * sycl::sqrt(T) * sycl::exp(-157809.1f / T) *
              (1.0f / (1.0f + sycl::sqrt(T / 1e5f)));
  gamma_ehe0 = (2.38e-11f) * sycl::sqrt(T) * sycl::exp(-285335.4f / T) *
               (1.0f / (1.0f + sycl::sqrt(T / 1e5f)));
  gamma_ehep = (5.68e-12f) * sycl::sqrt(T) * sycl::exp(-631515.0f / T) *
               (1.0f / (1.0f + sycl::sqrt(T / 1e5f)));
  // externally evaluated integrals for photoionization rates
  // assumed J(nu) = 10^-22 (nu_L/nu)
  gamma_lh0 = 3.19851e-13f;
  gamma_lhe0 = 3.13029e-13f;
  gamma_lhep = 2.00541e-14f;
  // externally evaluated integrals for heating rates
  e_h0 = 2.4796e-24f;
  e_he0 = 6.86167e-24f;
  e_hep = 6.21868e-25f;

  // assuming no photoionization, solve equations for number density of
  // each species
  n_e = n_h; //as a first guess, use the hydrogen number density
  n_iter = 20;
  diff = 1.0f;
  tol = 1.0e-6f;
  if (heat_flag) {
    for (int i=0; i<n_iter; i++) {
      n_e_old = n_e;
      n_h0   = n_h*alpha_hp / (alpha_hp + gamma_eh0 + gamma_lh0/n_e);
      n_hp   = n_h - n_h0;
      n_hep  = y*n_h / (1.0f + (alpha_hep + alpha_d)/(gamma_ehe0 + gamma_lhe0/n_e) + (gamma_ehep + gamma_lhep/n_e)/alpha_hepp);
      n_he0  = n_hep*(alpha_hep + alpha_d) / (gamma_ehe0 + gamma_lhe0/n_e);
      n_hepp = n_hep*(gamma_ehep + gamma_lhep/n_e)/alpha_hepp;
      n_e    = n_hp + n_hep + 2*n_hepp;
      diff = sycl::fabs(n_e_old - n_e);
      if (diff < tol) break;
    }
  }
  else {
    n_h0   = n_h*alpha_hp / (alpha_hp + gamma_eh0);
    n_hp   = n_h - n_h0;
    n_hep  = y*n_h / (1.0f + (alpha_hep + alpha_d)/(gamma_ehe0) + (gamma_ehep)/alpha_hepp);
    n_he0  = n_hep*(alpha_hep + alpha_d) / (gamma_ehe0);
    n_hepp = n_hep*(gamma_ehep)/alpha_hepp;
    n_e    = n_hp + n_hep + 2*n_hepp;
  }

  // using number densities, calculate cooling rates for
  // various processes (Table 1 from Katz 1996)
  le_h0 = (7.50e-19f) * sycl::exp(-118348.0f / T) *
          (1.0f / (1.0f + sycl::sqrt(T / 1e5f))) * n_e * n_h0;
  le_hep = (5.54e-17f) * sycl::pow(T, (-0.397f)) *
           sycl::exp(-473638.0f / T) * (1.0f / (1.0f + sycl::sqrt(T / 1e5f))) *
           n_e * n_hep;
  li_h0 = (1.27e-21f) * sycl::sqrt(T) * sycl::exp(-157809.1f / T) *
          (1.0f / (1.0f + sycl::sqrt(T / 1e5f))) * n_e * n_h0;
  li_he0 = (9.38e-22f) * sycl::sqrt(T) * sycl::exp(-285335.4f / T) *
           (1.0f / (1.0f + sycl::sqrt(T / 1e5f))) * n_e * n_he0;
  li_hep = (4.95e-22f) * sycl::sqrt(T) * sycl::exp(-631515.0f / T) *
           (1.0f / (1.0f + sycl::sqrt(T / 1e5f))) * n_e * n_hep;
  lr_hp = (8.70e-27f) * sycl::sqrt(T) * sycl::pow((T / 1e3f), (-0.2f)) *
          (1.0f / (1.0f + sycl::pow((T / 1e6f), (0.7f)))) * n_e * n_hp;
  lr_hep = (1.55e-26f) * sycl::pow(T, (0.3647f)) * n_e * n_hep;
  lr_hepp = (3.48e-26f) * sycl::sqrt(T) * sycl::pow((T / 1e3f), (-0.2f)) *
            (1.0f / (1.0f + sycl::pow((T / 1e6f), (0.7f)))) * n_e * n_hepp;
  ld_hep = (1.24e-13f) * sycl::pow(T, (-1.5f)) *
           sycl::exp(-470000.0f / T) * (1.0f + 0.3f * sycl::exp(-94000.0f / T)) *
           n_e * n_hep;
  g_ff = 1.1f + 0.34f * sycl::exp(-(5.5f - sycl::log(T)) * (5.5f - sycl::log(T)) /
                                3.0f); // Gaunt factor
  l_ff = (1.42e-27f) * g_ff * sycl::sqrt(T) * (n_hp + n_hep + 4 * n_hepp) * n_e;

  // calculate total cooling rate (erg s^-1 cm^-3)
  cool = le_h0 + le_hep + li_h0 + li_he0 + li_hep + lr_hp + lr_hep + lr_hepp + ld_hep + l_ff;

  // calculate total photoionization heating rate
  H = 0.0f;
  if (heat_flag) {
    H = n_h0*e_h0 + n_he0*e_he0 + n_hep*e_hep;
  }

  cool -= H;

  return cool;
}


void cool_kernel (
  const int  num,
  const Real n,
  const Real *__restrict T,
        Real *__restrict r,
  const int  heat_flag,
  sycl::nd_item<1> &item)
{
  int i = item.get_global_id(0);
  if (i < num)
    r[i] = primordial_cool(n, T[i], heat_flag);
}

void reference (
  const int  num,
  const Real n,
  const Real *__restrict T,
        Real *__restrict r,
  const int  heat_flag)
{
  for (int i = 0; i < num; i++) 
    r[i] = primordial_cool(n, T[i], heat_flag);
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    printf("Usage: %s <number of points> <repeat>\n", argv[0]);
    return 1;
  }
  const int num = atoi(argv[1]);
  const int repeat = atoi(argv[2]);
    
  const size_t size_bytes = sizeof(Real) * num;

  const Real n = 0.0899f; // density

  Real *T = (Real*) malloc (size_bytes);
  for (int i = 0; i < num; i++) {
    T[i] = -275.0f + i * 275 * 2.0f / num;
  }

  Real *r = (Real*) malloc (size_bytes);
  Real *h_r = (Real*) malloc (size_bytes);

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  Real *d_T, *d_r;
  d_T = (Real *)sycl::malloc_device(size_bytes, q);
  q.memcpy(d_T, T, size_bytes);

  d_r = (Real *)sycl::malloc_device(size_bytes, q);

  sycl::range<1> gws ((num + 255) / 256 * 256);
  sycl::range<1> lws (256);

  // warmup
  for (int i = 0; i < repeat; i++) {
    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class noheat>(
        sycl::nd_range<1>(gws, lws), [=] (sycl::nd_item<1> item) {
        cool_kernel(num, n, d_T, d_r, 0, item);
      });
    });
  }
  q.wait();

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < repeat; i++) {
    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class heat>(
      sycl::nd_range<1>(gws, lws), [=] (sycl::nd_item<1> item) {
        cool_kernel(num, n, d_T, d_r, 1, item);
      });
    });
  }
  q.wait();

  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time %f (ms)\n", (time * 1e-6f) / repeat);

  // verify
  q.memcpy(r, d_r, size_bytes).wait();

  reference(num, n, T, h_r, 1);
  
  bool error = false;
  for (int i = 0; i < num; i++) {
    if (fabsf(r[i] - h_r[i]) > 1e-3) {
      error = true;
      break;
    }
  }
  printf("%s\n", error ? "FAIL" : "PASS");

  sycl::free(d_T, q);
  sycl::free(d_r, q);
  free(T);
  free(r);
  free(h_r);
  return 0;
}
