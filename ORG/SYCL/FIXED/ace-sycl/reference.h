#ifndef REFERENCE_H
#define REFERENCE_H

// Reference CPU implementations
void ref_dFphi(float *phi, float *u, float *lambda, float *result);
void ref_GradientX(float *phi, float *dx, float *dy, float *dz, int *x, int *y, int *z, float *result);
void ref_GradientY(float *phi, float *dx, float *dy, float *dz, int *x, int *y, int *z, float *result);
void ref_GradientZ(float *phi, float *dx, float *dy, float *dz, int *x, int *y, int *z, float *result);
void ref_Divergence(float *phix, float *phiy, float *phiz, float *dx, float *dy, float *dz, 
                 int *x, int *y, int *z, float *result);
void ref_Laplacian(float *phi, float *dx, float *dy, float *dz, int *x, int *y, int *z, float *result);
void ref_calculateForce(float *phi, float *Fx, float *Fy, float *Fz, float *dx, float *dy, float *dz,
                     float *epsilon, float *W0, float *tau0, int *ix, int *iy, int *iz);
void ref_allenCahn(float *phinew, float *phiold, float *uold, float *Fx, float *Fy, float *Fz,
                 float *epsilon, float *W0, float *tau0, float *lambda, float *dt, float *dx, 
                 float *dy, float *dz, int *ix, int *iy, int *iz);
void ref_thermalEquation(float *unew, float *uold, float *phinew, float *phiold, float *D,
                      float *dt, float *dx, float *dy, float *dz, int *ix, int *iy, int *iz);

#endif // REFERENCE_H
