#ifndef _LULESH_H_
#define _LULESH_H_

#if !defined(USE_MPI)
# error "You should specify USE_MPI=0 or USE_MPI=1 on the compile line"
#endif

#include <sycl/sycl.hpp>
#include <iostream>
#include <math.h>
#include <vector>

using namespace sycl;

#define MAX(a, b) ( ((a) > (b)) ? (a) : (b))

using Real_t = double;
using Index_t = int32_t;
using Int_t = int32_t;

using Domain_member = Real_t;

template <typename T>
using AlignedVector = std::vector<T>;

enum { VolumeError = -1, QStopError = -2 } ;

class Domain {
public:
   void *operator new(size_t size)
   {
      void *ptr = 0;
      posix_memalign(&ptr, 64, size);
      return ptr;
   }

   void operator delete(void *ptr)
   {
      free(ptr);
   }

   Domain(Int_t numRanks, Index_t colLoc,
         Index_t rowLoc, Index_t planeLoc,
         Index_t nx, Int_t tp, Int_t nr, Int_t balance, Int_t cost);
   ~Domain();

   // Time Functions
   Real_t time()     const { return m_time ; }
   Real_t deltatime() const { return m_deltatime ; }
   Real_t stoptime() const { return m_stoptime ; }
   Real_t dtfixed()  const { return m_dtfixed ; }
   Real_t dthydro()  const { return m_dthydro ; }
   Real_t dtcourant()  const { return m_dtcourant ; }
   Real_t dthdryo()  const { return m_dthydro ; }
   Int_t cycle()     const { return m_cycle ; }
   Index_t numElem() const { return m_numElem ; }

   // Node-centered
   void AllocateNodePersistent(Int_t numNode);
   void AllocateElemPersistent(Int_t numElem);
   void AllocateGradients(Int_t numElem, Int_t allElem);

   // Element accessors
   Real_t& e(Index_t idx)      { return m_e[idx] ; }
   Real_t& x(Index_t idx)      { return m_x[idx] ; }
   Real_t& y(Index_t idx)      { return m_y[idx] ; }
   Real_t& z(Index_t idx)      { return m_z[idx] ; }
   Real_t& fx(Index_t idx)     { return m_fx[idx] ; }
   Real_t& fy(Index_t idx)     { return m_fy[idx] ; }
   Real_t& fz(Index_t idx)     { return m_fz[idx] ; }

   Real_t& xd(Index_t idx)     { return m_xd[idx] ; }
   Real_t& yd(Index_t idx)     { return m_yd[idx] ; }
   Real_t& zd(Index_t idx)     { return m_zd[idx] ; }

   Real_t& xdd(Index_t idx)    { return m_xdd[idx] ; }
   Real_t& ydd(Index_t idx)    { return m_ydd[idx] ; }
   Real_t& zdd(Index_t idx)    { return m_zdd[idx] ; }

   // Const versions for const Domain
   const Real_t& e(Index_t idx) const   { return m_e[idx] ; }
   const Real_t& x(Index_t idx) const   { return m_x[idx] ; }
   const Real_t& y(Index_t idx) const   { return m_y[idx] ; }
   const Real_t& z(Index_t idx) const   { return m_z[idx] ; }
   const Real_t& fx(Index_t idx) const  { return m_fx[idx] ; }
   const Real_t& fy(Index_t idx) const  { return m_fy[idx] ; }
   const Real_t& fz(Index_t idx) const  { return m_fz[idx] ; }

   const Real_t& xd(Index_t idx) const  { return m_xd[idx] ; }
   const Real_t& yd(Index_t idx) const  { return m_yd[idx] ; }
   const Real_t& zd(Index_t idx) const  { return m_zd[idx] ; }

   const Real_t& xdd(Index_t idx) const { return m_xdd[idx] ; }
   const Real_t& ydd(Index_t idx) const { return m_ydd[idx] ; }
   const Real_t& zdd(Index_t idx) const { return m_zdd[idx] ; }

   Real_t& nodalMass(Index_t idx) { return m_nodalMass[idx] ; }

   Index_t& symmX(Index_t idx) { return m_symmX[idx] ; }
   Index_t& symmY(Index_t idx) { return m_symmY[idx] ; }
   Index_t& symmZ(Index_t idx) { return m_symmZ[idx] ; }

   // Element-centered
   Real_t& delx_xi(Index_t idx)    { return m_delx_xi[idx] ; }
   Real_t& delx_eta(Index_t idx)   { return m_delx_eta[idx] ; }
   Real_t& delx_zeta(Index_t idx)  { return m_delx_zeta[idx] ; }

   Real_t& vdov(Index_t idx)       { return m_vdov[idx] ; }
   Real_t& p(Index_t idx)          { return m_p[idx] ; }
   Real_t& q(Index_t idx)          { return m_q[idx] ; }
   Real_t& ql(Index_t idx)         { return m_ql[idx] ; }
   Real_t& qq(Index_t idx)         { return m_qq[idx] ; }

   Real_t& v(Index_t idx)          { return m_v[idx] ; }
   Real_t& volo(Index_t idx)       { return m_volo[idx] ; }
   Real_t& delv(Index_t idx)       { return m_delv[idx] ; }

   Real_t& arealg(Index_t idx)     { return m_arealg[idx] ; }
   Real_t& ss(Index_t idx)         { return m_ss[idx] ; }
   Real_t& elemMass(Index_t idx)   { return m_elemMass[idx] ; }

   // Simulation
   void TimeIncrement() ;
   void LagrangeElements() ;
   void CalcTimeConstraintsForElems() ;

   // Communication Functions
   friend void CommRecv(Domain& domain, int msgType, Index_t xferFields,
                      Index_t dx, Index_t dy, Index_t dz,
                      bool doRecv, bool planeOnly);

   friend void CommSend(Domain& domain, int msgType,
                      Index_t xferFields, Domain_member *fieldData,
                      Index_t dx, Index_t dy, Index_t dz,
                      bool doSend, bool planeOnly);

   friend void CommSBN(Domain& domain, int xferFields, Domain_member *fieldData);

   friend void CommSyncPosVel(Domain& domain);

   // SYCL device queue
   sycl::queue* q_device;

private:
   void BuildMesh(Int_t nx, Int_t edgeNodes, Int_t edgeElems);

   // Member data
   Real_t  m_dtfixed ;           // fixed time increment
   Real_t  m_deltatime;          // current timestep
   Real_t  m_time ;              // current time
   Real_t  m_stoptime ;          // stop time for simulation
   Real_t  m_deltatimemultlb ;
   Real_t  m_deltatimemultub ;
   Real_t  m_dtcourant ;         // courant constraint
   Real_t  m_dthydro ;           // volume change constraint
   Int_t   m_cycle ;             // iteration count for simulation
   Int_t   m_numRanks ;          // number of ranks
   Index_t m_colLoc ;            // column location
   Index_t m_rowLoc ;            // row location
   Index_t m_planeLoc ;          // plane location
   Index_t m_tp ;                // total number of domains
   Int_t   m_cost ;              // imbalance cost

   Index_t m_numElem ;           // number of elements
   Index_t m_numNode ;           // number of nodes

   Real_t  m_e_cut ;             // energy tolerance
   Real_t  m_p_cut ;             // pressure tolerance
   Real_t  m_q_cut ;             // q tolerance
   Real_t  m_v_cut ;             // relative volume tolerance
   Real_t  m_u_cut ;             // velocity tolerance
   Real_t  m_hgcoef ;            // hourglass control
   Real_t  m_ss4o3 ;
   Real_t  m_qstop ;             // excessive q indicator
   Real_t  m_monoq_max_slope ;
   Real_t  m_monoq_limiter_mult ;
   Real_t  m_qlc_monoq ;         // linear term coef for q
   Real_t  m_qqc_monoq ;         // quadratic term coef for q
   Real_t  m_qqc ;
   Real_t  m_eosvmax ;
   Real_t  m_eosvmin ;
   Real_t  m_pmin ;              // pressure floor
   Real_t  m_emin ;              // energy floor
   Real_t  m_dvovmax ;           // maximum allowable volume change

   // Node-centered
   AlignedVector<Real_t> m_x ;   // coordinates
   AlignedVector<Real_t> m_y ;
   AlignedVector<Real_t> m_z ;

   AlignedVector<Real_t> m_xd ;  // velocities
   AlignedVector<Real_t> m_yd ;
   AlignedVector<Real_t> m_zd ;

   AlignedVector<Real_t> m_xdd ; // accelerations
   AlignedVector<Real_t> m_ydd ;
   AlignedVector<Real_t> m_zdd ;

   AlignedVector<Real_t> m_fx ;  // forces
   AlignedVector<Real_t> m_fy ;
   AlignedVector<Real_t> m_fz ;

   AlignedVector<Real_t> m_nodalMass ; // mass

   // Element-centered
   AlignedVector<Index_t> m_nodelist ; // elemToNode connectivity

   AlignedVector<Index_t> m_lxim ; // element connectivity through faces
   AlignedVector<Index_t> m_lxip ;
   AlignedVector<Index_t> m_letam ;
   AlignedVector<Index_t> m_letap ;
   AlignedVector<Index_t> m_lzetam ;
   AlignedVector<Index_t> m_lzetap ;

   AlignedVector<Int_t> m_elemBC ; // elem face symm/free-surface flag

   AlignedVector<Real_t> m_dxx ;  // principal strains -- temporary
   AlignedVector<Real_t> m_dyy ;
   AlignedVector<Real_t> m_dzz ;

   AlignedVector<Real_t> m_delv_xi ;    // velocity gradient -- temporary
   AlignedVector<Real_t> m_delv_eta ;
   AlignedVector<Real_t> m_delv_zeta ;

   AlignedVector<Real_t> m_delx_xi ;    // coordinate gradient -- temporary
   AlignedVector<Real_t> m_delx_eta ;
   AlignedVector<Real_t> m_delx_zeta ;
   
   AlignedVector<Real_t> m_e ;    // energy
   AlignedVector<Real_t> m_p ;    // pressure
   AlignedVector<Real_t> m_q ;    // q
   AlignedVector<Real_t> m_ql ;   // linear term for q
   AlignedVector<Real_t> m_qq ;   // quadratic term for q

   AlignedVector<Real_t> m_v ;      // relative volume
   AlignedVector<Real_t> m_volo ;   // reference volume
   AlignedVector<Real_t> m_vnew ;   // new relative volume -- temporary
   AlignedVector<Real_t> m_delv ;   // volume strain -- temporary
   AlignedVector<Real_t> m_vdov ;   // volume derivative over volume

   AlignedVector<Real_t> m_arealg ;  // elem characteristic length

   AlignedVector<Real_t> m_ss ;      // "sound speed"

   AlignedVector<Real_t> m_elemMass ;  // mass

   // Cutoffs (Death) 
   AlignedVector<Index_t> m_nodeElemCount ;
   AlignedVector<Index_t> m_nodeElemCornerList ;

   // Node Symmetry Plane Adj Lists
   AlignedVector<Index_t> m_symmX ;
   AlignedVector<Index_t> m_symmY ;
   AlignedVector<Index_t> m_symmZ ;
} ;

struct cmdLineOpts {
   Int_t its;           // -i
   Int_t nx;           // -s
   Int_t numReg;       // -r
   Int_t numFiles;     // -f
   Int_t showProg;     // -p
   Int_t quiet;        // -q
   Int_t viz;          // -v
   Int_t cost;         // -c
   Int_t balance;      // -b
   Int_t iteration_cap;
};

// Utility functions
void PrintSYCLInfo(const Domain& domain);
void DumpMeshMetadata(const Domain& domain, const char *meshName);
void DumpMeshCoordinates(const Domain& domain, const char *label);
void DumpMeshVelocities(const Domain& domain, const char *label);
void DumpMeshForces(const Domain& domain, const char *label);
void PrintSYCLInfo(const Domain& domain);

#endif  // _LULESH_H_
