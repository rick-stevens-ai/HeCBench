#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <sycl/sycl.hpp>

//define the data set size (cubic volume)
#define DATAXSIZE 400
#define DATAYSIZE 400
#define DATAZSIZE 400

typedef float nRarray[DATAYSIZE][DATAXSIZE];

// square
#define SQ(x) ((x)*(x))

#ifdef VERIFY
#include <string.h>
#include "reference.h"
#endif

float dFphi(float phi, float u, float lambda)
{
  return (-phi*(1.0f-phi*phi)+lambda*u*(1.0f-phi*phi)*(1.0f-phi*phi));
}

float GradientX(float phi[][DATAYSIZE][DATAXSIZE], 
             float dx, float dy, float dz, int x, int y, int z)
{
  return (phi[x+1][y][z] - phi[x-1][y][z]) / (2.0f*dx);
}

float GradientY(float phi[][DATAYSIZE][DATAXSIZE], 
             float dx, float dy, float dz, int x, int y, int z)
{
  return (phi[x][y+1][z] - phi[x][y-1][z]) / (2.0f*dy);
}

float GradientZ(float phi[][DATAYSIZE][DATAXSIZE], 
             float dx, float dy, float dz, int x, int y, int z)
{
  return (phi[x][y][z+1] - phi[x][y][z-1]) / (2.0f*dz);
}

float Divergence(float phix[][DATAYSIZE][DATAXSIZE], 
              float phiy[][DATAYSIZE][DATAXSIZE],
              float phiz[][DATAYSIZE][DATAXSIZE], 
              float dx, float dy, float dz, int x, int y, int z)
{
  return GradientX(phix,dx,dy,dz,x,y,z) + 
         GradientY(phiy,dx,dy,dz,x,y,z) +
         GradientZ(phiz,dx,dy,dz,x,y,z);
}

float Laplacian(float phi[][DATAYSIZE][DATAXSIZE],
             float dx, float dy, float dz, int x, int y, int z)
{
  float phixx = (phi[x+1][y][z] + phi[x-1][y][z] - 2.0f * phi[x][y][z]) / SQ(dx);
  float phiyy = (phi[x][y+1][z] + phi[x][y-1][z] - 2.0f * phi[x][y][z]) / SQ(dy);
  float phizz = (phi[x][y][z+1] + phi[x][y][z-1] - 2.0f * phi[x][y][z]) / SQ(dz);
  return phixx + phiyy + phizz;
}

float An(float phix, float phiy, float phiz, float epsilon)
{
  if (phix != 0.0f || phiy != 0.0f || phiz != 0.0f){
    return ((1.0f - 3.0f * epsilon) * (1.0f + (((4.0f * epsilon) / (1.0f-3.0f*epsilon))*
           ((SQ(phix)*SQ(phix)+SQ(phiy)*SQ(phiy)+SQ(phiz)*SQ(phiz)) /
           ((SQ(phix)+SQ(phiy)+SQ(phiz))*(SQ(phix)+SQ(phiy)+SQ(phiz)))))));}
  else
  {
    return (1.0f-((5.0f/3.0f)*epsilon));
  }
}

float Wn(float phix, float phiy, float phiz, float epsilon, float W0)
{
  return (W0*An(phix,phiy,phiz,epsilon));
}

float taun(float phix, float phiy, float phiz, float epsilon, float tau0)
{
  return tau0 * SQ(An(phix,phiy,phiz,epsilon));
}

float dFunc(float l, float m, float n)
{
  if (l != 0.0f || m != 0.0f || n != 0.0f){
    return (((l*l*l*(SQ(m)+SQ(n)))-(l*(SQ(m)*SQ(m)+SQ(n)*SQ(n)))) /
            ((SQ(l)+SQ(m)+SQ(n))*(SQ(l)+SQ(m)+SQ(n))));
  }
  else
  {
    return 0.0f;
  }
}

void calculateForce(float phi[][DATAYSIZE][DATAXSIZE], 
                float Fx[][DATAYSIZE][DATAXSIZE],
                float Fy[][DATAYSIZE][DATAXSIZE],
                float Fz[][DATAYSIZE][DATAXSIZE],
                float dx, float dy, float dz,
                float epsilon, float W0, float tau0,
                sycl::nd_item<3> &item)
{
  unsigned iz = item.get_global_id(2);
  unsigned iy = item.get_global_id(1);
  unsigned ix = item.get_global_id(0);

  if ((ix < (DATAXSIZE-1)) && (iy < (DATAYSIZE-1)) && 
      (iz < (DATAZSIZE-1)) && (ix > (0)) && 
      (iy > (0)) && (iz > (0))) {

    float phix = GradientX(phi,dx,dy,dz,ix,iy,iz);
    float phiy = GradientY(phi,dx,dy,dz,ix,iy,iz);
    float phiz = GradientZ(phi,dx,dy,dz,ix,iy,iz);
    float sqGphi = SQ(phix) + SQ(phiy) + SQ(phiz);
    float c = 16.0f * W0 * epsilon;
    float w = Wn(phix,phiy,phiz,epsilon,W0);
    float w2 = SQ(w);

    Fx[ix][iy][iz] = w2 * phix + sqGphi * w * c * dFunc(phix,phiy,phiz);
    Fy[ix][iy][iz] = w2 * phiy + sqGphi * w * c * dFunc(phiy,phiz,phix);
    Fz[ix][iy][iz] = w2 * phiz + sqGphi * w * c * dFunc(phiz,phix,phiy);
  }
  else
  {
    Fx[ix][iy][iz] = 0.0f;
    Fy[ix][iy][iz] = 0.0f;
    Fz[ix][iy][iz] = 0.0f;
  }
}

void allenCahn(float phinew[][DATAYSIZE][DATAXSIZE], 
           float phiold[][DATAYSIZE][DATAXSIZE],
           float uold[][DATAYSIZE][DATAXSIZE],
           float Fx[][DATAYSIZE][DATAXSIZE],
           float Fy[][DATAYSIZE][DATAXSIZE],
           float Fz[][DATAYSIZE][DATAXSIZE],
           float epsilon, float W0, float tau0, float lambda,
           float dt, float dx, float dy, float dz,
           sycl::nd_item<3> &item)
{
  unsigned iz = item.get_global_id(2);
  unsigned iy = item.get_global_id(1);
  unsigned ix = item.get_global_id(0);

  if ((ix < (DATAXSIZE-1)) && (iy < (DATAYSIZE-1)) && 
      (iz < (DATAZSIZE-1)) && (ix > (0)) && 
      (iy > (0)) && (iz > (0))) {

    float phix = GradientX(phiold,dx,dy,dz,ix,iy,iz);
    float phiy = GradientY(phiold,dx,dy,dz,ix,iy,iz);
    float phiz = GradientZ(phiold,dx,dy,dz,ix,iy,iz); 

    phinew[ix][iy][iz] = phiold[ix][iy][iz] + 
     (dt / taun(phix,phiy,phiz,epsilon,tau0)) * 
     (Divergence(Fx,Fy,Fz,dx,dy,dz,ix,iy,iz) - 
      dFphi(phiold[ix][iy][iz], uold[ix][iy][iz],lambda));
  }
}

void boundaryConditionsPhi(float phinew[][DATAYSIZE][DATAXSIZE],
                       sycl::nd_item<3> &item)
{
  unsigned iz = item.get_global_id(2);
  unsigned iy = item.get_global_id(1);
  unsigned ix = item.get_global_id(0);

  if (ix == 0){
    phinew[ix][iy][iz] = -1.0f;
  }
  else if (ix == DATAXSIZE-1){
    phinew[ix][iy][iz] = -1.0f;
  }
  else if (iy == 0){
    phinew[ix][iy][iz] = -1.0f;
  }
  else if (iy == DATAYSIZE-1){
    phinew[ix][iy][iz] = -1.0f;
  }
  else if (iz == 0){
    phinew[ix][iy][iz] = -1.0f;
  }
  else if (iz == DATAZSIZE-1){
    phinew[ix][iy][iz] = -1.0f;
  }
}

void thermalEquation(float unew[][DATAYSIZE][DATAXSIZE],
                 float uold[][DATAYSIZE][DATAXSIZE],
                 float phinew[][DATAYSIZE][DATAXSIZE],
                 float phiold[][DATAYSIZE][DATAXSIZE],
                 float D, float dt, float dx, float dy, float dz,
                 sycl::nd_item<3> &item)
{
  unsigned iz = item.get_global_id(2);
  unsigned iy = item.get_global_id(1);
  unsigned ix = item.get_global_id(0);

  if ((ix < (DATAXSIZE-1)) && (iy < (DATAYSIZE-1)) && 
      (iz < (DATAZSIZE-1)) && (ix > (0)) && 
      (iy > (0)) && (iz > (0))){
    unew[ix][iy][iz] = uold[ix][iy][iz] + 
      0.5f*(phinew[ix][iy][iz]- phiold[ix][iy][iz]) +
      dt * D * Laplacian(uold,dx,dy,dz,ix,iy,iz);
  }
}

void boundaryConditionsU(float unew[][DATAYSIZE][DATAXSIZE], float delta,
                     sycl::nd_item<3> &item)
{
  unsigned iz = item.get_global_id(2);
  unsigned iy = item.get_global_id(1);
  unsigned ix = item.get_global_id(0);

  if (ix == 0){
    unew[ix][iy][iz] =  -delta;
  }
  else if (ix == DATAXSIZE-1){
    unew[ix][iy][iz] =  -delta;
  }
  else if (iy == 0){
    unew[ix][iy][iz] =  -delta;
  }
  else if (iy == DATAYSIZE-1){
    unew[ix][iy][iz] =  -delta;
  }
  else if (iz == 0){
    unew[ix][iy][iz] =  -delta;
  }
  else if (iz == DATAZSIZE-1){
    unew[ix][iy][iz] =  -delta;
  }
}

void swapGrid(float cnew[][DATAYSIZE][DATAXSIZE],
           float cold[][DATAYSIZE][DATAXSIZE],
           sycl::nd_item<3> &item)
{
  unsigned iz = item.get_global_id(2);
  unsigned iy = item.get_global_id(1);
  unsigned ix = item.get_global_id(0);

  if ((ix < (DATAXSIZE)) && 
      (iy < (DATAYSIZE)) &&
      (iz < (DATAZSIZE))) {
    float tmp = cnew[ix][iy][iz];
    cnew[ix][iy][iz] = cold[ix][iy][iz];
    cold[ix][iy][iz] = tmp;
  }
}

void initializationPhi(float phi[][DATAYSIZE][DATAXSIZE], float r0)
{
#ifdef _OPENMP
  #pragma omp parallel for collapse(3)
#endif
  for (int idx = 0; idx < DATAXSIZE; idx++) {
    for (int idy = 0; idy < DATAYSIZE; idy++) {
      for (int idz = 0; idz < DATAZSIZE; idz++) {
        float r = std::sqrt(SQ(idx-0.5f*DATAXSIZE) + SQ(idy-0.5f*DATAYSIZE) + SQ(idz-0.5f*DATAZSIZE));
        if (r < r0){
          phi[idx][idy][idz] = 1.0f;
        }
        else
        {
          phi[idx][idy][idz] = -1.0f;
        }
      }
    }
  }
}

void initializationU(float u[][DATAYSIZE][DATAXSIZE], float r0, float delta)
{
#ifdef _OPENMP
  #pragma omp parallel for collapse(3)
#endif
  for (int idx = 0; idx < DATAXSIZE; idx++) {
    for (int idy = 0; idy < DATAYSIZE; idy++) {
      for (int idz = 0; idz < DATAZSIZE; idz++) {
        float r = std::sqrt(SQ(idx-0.5f*DATAXSIZE) + SQ(idy-0.5f*DATAYSIZE) + SQ(idz-0.5f*DATAZSIZE));
        if (r < r0) {
          u[idx][

idy][idz] = 0.0f;
        }
        else
        {
          u[idx][idy][idz] = -delta * (1.0f - std::exp(-(r-r0)));
        }
      }
    }
  }
}

int main(int argc, char *argv[])
{
  const int num_steps = atoi(argv[1]);
  const float dx = 0.4f;
  const float dy = 0.4f;
  const float dz = 0.4f;
  const float dt = 0.01f;
  const float delta = 0.8f;
  const float r0 = 5.0f;
  const float epsilon = 0.07f;
  const float W0 = 1.0f;
  const float beta0 = 0.0f;
  const float D = 2.0f;
  const float d0 = 0.5f;
  const float a1 = 1.25f / std::sqrt(2.0f);
  const float a2 = 0.64f;
  const float lambda = (W0*a1)/(d0);
  const float tau0 = ((W0*W0*W0*a1*a2)/(d0*D)) + ((W0*W0*beta0)/(d0));

  // overall data set sizes
  const int nx = DATAXSIZE;
  const int ny = DATAYSIZE;
  const int nz = DATAZSIZE;
  const int vol = nx * ny * nz;
  const size_t vol_in_bytes = sizeof(float) * vol;

  // pointers for data set storage via malloc
  nRarray *phi_host;
  nRarray *u_host;

  phi_host = (nRarray *)malloc(vol_in_bytes);
  u_host = (nRarray *)malloc(vol_in_bytes);

  initializationPhi(phi_host,r0);
  initializationU(u_host,r0,delta);

#ifdef VERIFY
  nRarray *phi_ref = (nRarray *)malloc(vol_in_bytes);
  nRarray *u_ref = (nRarray *)malloc(vol_in_bytes);
  memcpy(phi_ref, phi_host, vol_in_bytes);
  memcpy(u_ref, u_host, vol_in_bytes);
  reference(phi_ref, u_ref, vol, num_steps);
#endif 

  auto offload_start = std::chrono::steady_clock::now();

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  // allocate GPU device buffers
  nRarray *d_phiold = (nRarray*) sycl::malloc_device(vol_in_bytes, q);
  nRarray *d_uold = (nRarray*) sycl::malloc_device(vol_in_bytes, q);
  nRarray *d_phinew = (nRarray*) sycl::malloc_device(vol_in_bytes, q);
  nRarray *d_unew = (nRarray*) sycl::malloc_device(vol_in_bytes, q);
  nRarray *d_Fx = (nRarray*) sycl::malloc_device(vol_in_bytes, q);
  nRarray *d_Fy = (nRarray*) sycl::malloc_device(vol_in_bytes, q);
  nRarray *d_Fz = (nRarray*) sycl::malloc_device(vol_in_bytes, q);

  q.memcpy(d_phiold, phi_host, vol_in_bytes);
  q.memcpy(d_uold, u_host, vol_in_bytes);

  // define the chunk sizes that each threadblock will work on
  sycl::range<3> gws ((DATAXSIZE+3)/4*4, (DATAYSIZE+7)/8*8, (DATAZSIZE+7)/8*8);
  sycl::range<3> lws (4, 8, 8);

  int t = 0;

  q.wait();
  auto start = std::chrono::steady_clock::now();

  while (t <= num_steps) {
    
    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class calc_force>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        calculateForce(d_phiold,
                     d_Fx,
                     d_Fy,
                     d_Fz,
                     dx,dy,dz,epsilon,W0,tau0,
                     item);
      });
    });

    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class allen_cahn>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        allenCahn(d_phinew,
                 d_phiold,
                 d_uold,
                 d_Fx,
                 d_Fy,
                 d_Fz,
                 epsilon,W0,tau0,lambda,
                 dt,dx,dy,dz,
                 item);
      });
    });

    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class bc_phi>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        boundaryConditionsPhi(d_phinew, item);
      });
    });

    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class thermal_equation>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        thermalEquation(d_unew,
                      d_uold,
                      d_phinew,
                      d_phiold,
                      D,dt,dx,dy,dz,
                      item);
      });
    });

    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class bc_u>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        boundaryConditionsU(d_unew, delta, item);
      });
    });

    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class swap_phi>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        swapGrid(d_phinew, d_phiold, item);
      });
    });

    q.submit([&] (sycl::handler &cgh) {
      cgh.parallel_for<class swap_u>(
        sycl::nd_range<3>(gws, lws), [=] (sycl::nd_item<3> item) {
        swapGrid(d_unew, d_uold, item);
      });
    });

    t++;
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Total kernel execution time: %.3f (ms)\n", time * 1e-6f);

  q.memcpy(phi_host, d_phiold, vol_in_bytes);
  q.memcpy(u_host, d_uold, vol_in_bytes);
  q.wait();

  sycl::free(d_phiold, q);
  sycl::free(d_phinew, q);
  sycl::free(d_uold, q);
  sycl::free(d_unew, q);
  sycl::free(d_Fx, q);
  sycl::free(d_Fy, q);
  sycl::free(d_Fz, q);

  auto offload_end = std::chrono::steady_clock::now();
  auto offload_time = std::chrono::duration_cast<std::chrono::nanoseconds>(offload_end - offload_start).count();
  printf("Offload time: %.3f (ms)\n", offload_time * 1e-6f);

#ifdef VERIFY
  bool ok = true;
  for (int idx = 0; idx < nx; idx++)
    for (int idy = 0; idy < ny; idy++)
      for (int idz = 0; idz < nz; idz++) {
        if (fabsf(phi_ref[idx][idy][idz] - phi_host[idx][idy][idz]) > 1e-3f) {
          ok = false; printf("phi: %f %f\n", phi_ref[idx][idy][idz], phi_host[idx][idy][idz]);
        }
        if (fabsf(u_ref[idx][idy][idz] - u_host[idx][idy][idz]) > 1e-3f) {
          ok = false; printf("u: %f %f\n", u_ref[idx][idy][idz], u_host[idx][idy][idz]);
        }
      }
  printf("%s\n", ok ? "PASS" : "FAIL");
  free(phi_ref);
  free(u_ref);
#endif

  free(phi_host);
  free(u_host);

  return 0;
}
