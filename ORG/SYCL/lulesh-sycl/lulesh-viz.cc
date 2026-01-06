#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "lulesh.h"

void DumpMeshMetadata(const Domain& domain, const char *meshName)
{
   if (meshName != nullptr) {
      printf("mesh %s, numElem=%d\n", 
             meshName, domain.numElem());
   }
}

void DumpMeshCoordinates(const Domain& domain, const char *label)
{
   if (label != nullptr) {
      printf("\n%s mesh coordinates\n", label);
   }

   for (Index_t i=0; i<domain.numElem(); ++i) {
      printf("elem %d : {%.1f, %.1f, %.1f}\n",
             i,
             domain.x(i),
             domain.y(i),
             domain.z(i));
   }
}

void DumpMeshVelocities(const Domain& domain, const char *label)
{
   if (label != nullptr) {
      printf("\n%s mesh velocities\n", label);
   }

   for (Index_t i=0; i<domain.numElem(); ++i) {
      printf("elem %d : {%.1f, %.1f, %.1f}\n",
             i,
             domain.xd(i),
             domain.yd(i),
             domain.zd(i));
   }
}

void DumpMeshForces(const Domain& domain, const char *label)
{
   if (label != nullptr) {
      printf("\n%s mesh forces\n", label);
   }

   for (Index_t i=0; i<domain.numElem(); ++i) {
      printf("elem %d : {%.1f, %.1f, %.1f}\n",
             i,
             domain.fx(i),
             domain.fy(i),
             domain.fz(i));
   }
}

void PrintSYCLInfo(const Domain& domain)
{
   auto& queue = domain.q_device;
   if (queue != nullptr) {
      auto device = queue->get_device();
      std::cout << "\nSYCL Device Info:"
               << "\n  Name: " << device.get_info<sycl::info::device::name>()
               << "\n  Vendor: " << device.get_info<sycl::info::device::vendor>()
               << "\n  Version: " << device.get_info<sycl::info::device::version>()
               << "\n  Driver Version: " << device.get_info<sycl::info::device::driver_version>()
               << std::endl;
   }
}
