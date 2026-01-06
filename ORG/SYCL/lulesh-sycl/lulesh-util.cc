#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "lulesh.h"

void Domain::AllocateNodePersistent(Int_t numNode)
{
   m_x.resize(numNode);
   m_y.resize(numNode);
   m_z.resize(numNode);

   m_xd.resize(numNode);
   m_yd.resize(numNode);
   m_zd.resize(numNode);

   m_xdd.resize(numNode);
   m_ydd.resize(numNode);
   m_zdd.resize(numNode);

   m_fx.resize(numNode);
   m_fy.resize(numNode);
   m_fz.resize(numNode);

   m_nodalMass.resize(numNode);

   m_symmX.resize(numNode);
   m_symmY.resize(numNode);
   m_symmZ.resize(numNode);
}

void Domain::AllocateElemPersistent(Int_t numElem)
{
   m_nodelist.resize(8*numElem);

   m_lxim.resize(numElem);
   m_lxip.resize(numElem);
   m_letam.resize(numElem);
   m_letap.resize(numElem);
   m_lzetam.resize(numElem);
   m_lzetap.resize(numElem);

   m_elemBC.resize(numElem);

   m_e.resize(numElem);
   m_p.resize(numElem);
   m_q.resize(numElem);
   m_ql.resize(numElem);
   m_qq.resize(numElem);

   m_v.resize(numElem);
   m_volo.resize(numElem);
   m_delv.resize(numElem);
   m_vdov.resize(numElem);

   m_arealg.resize(numElem);

   m_ss.resize(numElem);

   m_elemMass.resize(numElem);

   m_nodeElemCount.resize(numElem);
   m_nodeElemCornerList.resize(numElem);
}

void Domain::AllocateGradients(Int_t numElem, Int_t allElem)
{
   m_delx_xi.resize(numElem);
   m_delx_eta.resize(numElem);
   m_delx_zeta.resize(numElem);

   m_delv_xi.resize(numElem);
   m_delv_eta.resize(numElem);
   m_delv_zeta.resize(numElem);

   m_dxx.resize(numElem);
   m_dyy.resize(numElem);
   m_dzz.resize(numElem);

   m_vnew.resize(numElem);
}

void Domain::BuildMesh(Int_t nx, Int_t edgeNodes, Int_t edgeElems)
{
   Index_t meshEdgeElems = edgeElems ;
   m_numElem = meshEdgeElems*meshEdgeElems*meshEdgeElems ;
   m_numNode = edgeNodes*edgeNodes*edgeNodes ;

   Index_t idx = 0;
   for (Index_t plane=0; plane<edgeNodes; ++plane) {
      for (Index_t row=0; row<edgeNodes; ++row) {
         for (Index_t col=0; col<edgeNodes; ++col) {
            Real_t x = Real_t(col) ;
            Real_t y = Real_t(row) ;
            Real_t z = Real_t(plane) ;
            m_x[idx] = x ;
            m_y[idx] = y ;
            m_z[idx] = z ;
            ++idx ;
         }
      }
   }

   // Set up element connectivity
   idx = 0;
   for (Index_t plane=0; plane<meshEdgeElems; ++plane) {
      for (Index_t row=0; row<meshEdgeElems; ++row) {
         for (Index_t col=0; col<meshEdgeElems; ++col) {
            Index_t n0 = plane * edgeNodes * edgeNodes +
                        row   * edgeNodes            +
                        col ;
            Index_t n1 = n0 + 1 ;
            Index_t n2 = n1 + edgeNodes ;
            Index_t n3 = n0 + edgeNodes ;
            Index_t n4 = n0 + edgeNodes*edgeNodes ;
            Index_t n5 = n1 + edgeNodes*edgeNodes ;
            Index_t n6 = n2 + edgeNodes*edgeNodes ;
            Index_t n7 = n3 + edgeNodes*edgeNodes ;

            m_nodelist[8*idx+0] = n0 ;
            m_nodelist[8*idx+1] = n1 ;
            m_nodelist[8*idx+2] = n2 ;
            m_nodelist[8*idx+3] = n3 ;
            m_nodelist[8*idx+4] = n4 ;
            m_nodelist[8*idx+5] = n5 ;
            m_nodelist[8*idx+6] = n6 ;
            m_nodelist[8*idx+7] = n7 ;

            ++idx ;
         }
      }
   }

   // Initialize material data
   for (Index_t i=0; i<m_numElem; ++i) {
      m_e[i] = Real_t(0.0) ;
      m_p[i] = Real_t(0.0) ;
      m_q[i] = Real_t(0.0) ;
      m_ql[i] = Real_t(0.0) ;
      m_qq[i] = Real_t(0.0) ;

      m_v[i] = Real_t(1.0) ;
      m_volo[i] = Real_t(1.0) ;
      m_elemMass[i] = Real_t(1.0) ;
   }
}
