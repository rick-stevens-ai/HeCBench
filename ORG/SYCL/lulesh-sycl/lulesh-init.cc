#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#if USE_MPI
#include <mpi.h>
#endif
#include "lulesh.h"

Domain::Domain(Int_t numRanks, Index_t colLoc,
             Index_t rowLoc, Index_t planeLoc,
             Index_t nx, Int_t tp, Int_t nr, Int_t balance, Int_t cost)
   :
   m_dtfixed(Real_t(-1.0)),
   m_time(Real_t(0.0)),
   m_stoptime(Real_t(1.0e-2)),
   m_deltatimemultlb(Real_t(1.1)),
   m_deltatimemultub(Real_t(1.2)),
   m_dtcourant(Real_t(1.0e+20)),
   m_dthydro(Real_t(1.0e+20)),
   m_cycle(0),
   m_numRanks(numRanks),
   m_colLoc(colLoc),
   m_rowLoc(rowLoc),
   m_planeLoc(planeLoc),
   m_tp(tp),
   m_cost(cost),
   m_numElem(0),
   m_numNode(0),
   m_e_cut(Real_t(1.0e-7)),
   m_p_cut(Real_t(1.0e-7)),
   m_q_cut(Real_t(1.0e-7)),
   m_v_cut(Real_t(1.0e-10)),
   m_u_cut(Real_t(1.0e-7)),
   m_hgcoef(Real_t(3.0)),
   m_ss4o3(Real_t(4.0)/Real_t(3.0)),
   m_qstop(Real_t(1.0e+12)),
   m_monoq_max_slope(Real_t(1.0)),
   m_monoq_limiter_mult(Real_t(2.0)),
   m_qlc_monoq(Real_t(0.5)),
   m_qqc_monoq(Real_t(2.0)/Real_t(3.0)),
   m_qqc(Real_t(2.0)),
   m_eosvmax(Real_t(1.0e+9)),
   m_eosvmin(Real_t(1.0e-9)),
   m_pmin(Real_t(0.)),
   m_emin(Real_t(-1.0e+15)),
   m_dvovmax(Real_t(0.1))
{
   Index_t edgeElems = nx;
   Index_t edgeNodes = edgeElems+1;
   
   try {
       // Create SYCL queue with GPU device
       q_device = new sycl::queue(sycl::gpu_selector_v, sycl::property::queue::in_order());
       
       std::cout << "Selected device: " 
                 << q_device->get_device().get_info<sycl::info::device::name>() << "\n"
                 << "Device vendor: "
                 << q_device->get_device().get_info<sycl::info::device::vendor>() << "\n";
   }
   catch (const sycl::exception& e) {
       std::cerr << "SYCL exception caught: " << e.what() << std::endl;
       throw;
   }

   this->AllocateNodePersistent(edgeNodes);
   this->AllocateElemPersistent(edgeElems);
   this->AllocateGradients(numElem(), 0);
   
   /* initialize field data */
   BuildMesh(nx, edgeNodes, edgeElems);
}

Domain::~Domain()
{
   delete q_device;
}
