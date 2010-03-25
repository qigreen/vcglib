/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#ifndef __VCGLIB_EDGE_SUPPORT
#define __VCGLIB_EDGE_SUPPORT

#include <vector>
#include <vcg/complex/trimesh/allocate.h>
#include <vcg/complex/trimesh/clean.h>
#include <vcg/complex/trimesh/update/topology.h>
#include <vcg/complex/trimesh/base.h>

namespace vcg
{
	namespace tri{
		/// \ingroup trimesh 

		/// \headerfile edge_support.h vcg/complex/trimesh/edge_support.h

		/// \brief This class is used to build edge based data structure from indexed data structure and viceversa

		/**
		*/

		template <class MeshType  >
		class HEdgeSupport{
		public:
			typedef typename MeshType::VertexType VertexType;
			typedef typename MeshType::VertexPointer VertexPointer;
			typedef typename MeshType::HEdgePointer HEdgePointer;
			typedef typename MeshType::HEdgeType HEdgeType;
			typedef typename MeshType::HEdgeIterator HEdgeIterator;
			typedef typename MeshType::FaceIterator FaceIterator;
			typedef typename MeshType::FaceType FaceType;

			struct VertexPairEdgePtr{
				VertexPairEdgePtr(VertexPointer _v0,VertexPointer _v1,HEdgePointer _ep):v0(_v0),v1(_v1),ep(_ep){if(v0>v1) std::swap(v0,v1);}
                                const bool operator <(const VertexPairEdgePtr &o) const {return (v0 == o.v0)? (v1<o.v1):(v0<o.v0);}
                                const bool operator ==(const VertexPairEdgePtr &o) const {return (v0 == o.v0)&& (v1==o.v1);}

				VertexPointer v0,v1;
				HEdgePointer ep;
			};
			struct FacePtrInt{
				FacePtrInt ( FaceType * _f,int _i):f(_f),i(_i){}
				FaceType * f;
				int i;
			};

			typedef std::vector<bool> BitVector;

			/**
			build a half-edge data structure from an indexed data structure. Note that the half-edges are allocated here for the first time.
			If you have a mesh where there are already edges, they will be removed and the data lost, so do not use this function
			to just "update" the topology of half edges.
			**/
			static void ComputeHalfEdgeFromIndexed(MeshType & m){
				assert(HasFVAdjacency(m));
				assert(HasHOppAdjacency(m));
				assert(HasHNextAdjacency(m));

                                typename MeshType::template PerFaceAttributeHandle<BitVector> flagVisited =
                                        vcg::tri::Allocator<MeshType>::template AddPerFaceAttribute<BitVector>(m,"");
				std::vector<FacePtrInt > borderEdges;

				// allocate all new half edges
				FaceIterator fi;
				int n_edges = 0;

				// count how many half edge to allocate
				for(fi = m.face.begin(); fi != m.face.end(); ++fi) if(! (*fi).IsD()) 
				{n_edges+=(*fi).VN();	
				 for(int i = 0; i < (*fi).VN(); ++i)
					 if(vcg::face::IsBorder<FaceType>((*fi),(i)))
						++n_edges;
				}

				// allocate the half edges
				typename MeshType::HEdgeIterator ei = vcg::tri::Allocator<MeshType>::AddHEdges(m,n_edges);
				
				std::vector<VertexPairEdgePtr> all;
				int firstEdge = 0;
				for(fi = m.face.begin(); fi != m.face.end(); ++fi)if(!(*fi).IsD()){
					assert((*fi).VN()>2);
					if(flagVisited[*fi].empty()) {flagVisited[*fi].resize((*fi).VN());}

					for(int i  = 0; i < (*fi).VN(); ++i,++ei)
					{
						(*ei).HVp()	= (*fi).V(i);
						(*ei).HNp()	= &m.hedge[firstEdge + (i +1) % (*fi).VN()];
						if(MeshType::HEdgeType::HasHFAdjacency())
							(*ei).HFp() = &(*fi);
						if( MeshType::FaceType::HasFHAdjacency())
							(*fi).FHp() = &(*ei);
						if(MeshType::HEdgeType::HasHPrevAdjacency())
							(*ei).HPp()	= &m.hedge[firstEdge + (i +(*fi).VN()-1) % (*fi).VN()];
						if(HasVHAdjacency(m))
							(*ei).HVp()->VHp() = &(*ei);
						all.push_back(VertexPairEdgePtr((*fi).V(i), (*fi).V((*fi).Next(i)),&(*ei)));// it will be used to link the hedges

						if(  vcg::face::IsBorder<FaceType>((*fi),(i)))
							borderEdges.push_back(FacePtrInt(&(*fi),i));
					}
					firstEdge += (*fi).VN();
				}

				// add all the border edges
				int borderLength; 
				typename std::vector<FacePtrInt >::iterator ebi;
				for( ebi = borderEdges.begin(); ebi != borderEdges.end(); ++ebi)
					if( !flagVisited[(*ebi).f][(*ebi).i])// not already inserted
					{
						 
						borderLength = 0;
						vcg::face::Pos<FaceType> bp((*ebi).f,(*ebi).i);
						FaceType * start = (*ebi).f;
						do{
							all.push_back( VertexPairEdgePtr ( bp.f->V( bp.f->Next(bp.z) ),bp.f->V( bp.z ),&(*ei)));
							(*ei).HVp()	=  bp.f->V(bp.f->Next(bp.z)) ;
							flagVisited[bp.f][bp.z] = true;
							++ei;
							bp.NextB();
							++borderLength;
							}while (bp.f != start);
				 
						// run over the border edges to link the adjacencies
						for(int be = 0; be < borderLength; ++be){
							if(MeshType::HEdgeType::HasHFAdjacency())
								m.hedge[firstEdge + be].HFp() = NULL;
							if(MeshType::HEdgeType::HasHPrevAdjacency())
								m.hedge[firstEdge + be].HPp()	= &m.hedge[firstEdge + (be +borderLength-1) % borderLength];
							m.hedge[firstEdge + be].HNp()	= &m.hedge[firstEdge + (be +1) % borderLength];
						}
						firstEdge+=borderLength;
				}
                                vcg::tri::Allocator<MeshType>:: template DeletePerFaceAttribute<BitVector>(m,flagVisited );
				
				std::sort(all.begin(),all.end());
				assert(all.size() == n_edges);
				for(int i = 0 ; i < all.size(); )
					if(all[i]  == all[i+1]) 
					{
						all[i].ep->HOp()	= all[i+1].ep;
						all[i+1].ep->HOp() = all[i].ep;
						i+=2;
					} 
					else
					{
						all[i].ep->HOp() = all[i].ep;
						i+=1;
					}

			}
			/**
			builds an indexed data structure from a half-edge data structure.
			Note: if  the half edge have the pointer to face   
			their relation FV (face-vertex) will be computed and the data possibly stored in the
			face will be preserved.
			**/
			static void ComputeIndexedFromHalfEdge(  MeshType & m ){
				assert(HasFVAdjacency(m));
				assert(MeshType::HEdgeType::HasHNextAdjacency());
				assert(MeshType::HEdgeType::HasHVAdjacency());
				assert(MeshType::HEdgeType::HasHOppAdjacency());
				assert(MeshType::FaceType::HasFHAdjacency());
				bool createFace,hasHEF,hasFHE;

//				typename MeshType::template PerHEdgeAttributeHandle<bool> hV = Allocator<MeshType>::template AddPerHEdgeAttribute<bool>(m,"");


				typename MeshType::HEdgeIterator ei;
				typename MeshType::FacePointer fp;
				typename MeshType::FaceIterator fi;
				typename MeshType::HEdgePointer ep,epF;
				int vi = 0;
				vcg::SimpleTempData<typename MeshType::HEdgeContainer,bool> hV(m.hedge);

				hasHEF = (MeshType::HEdgeType::HasHFAdjacency());
				assert( !hasHEF || (hasHEF && m.fn>0));

				// if the edgetype has the pointer to face   
				// it is assumed the the edget2face pointer (HEFp) are correct
				// and the faces are allocated
					for ( ei = m.hedge.begin(); ei != m.hedge.end(); ++ei)
					if(!(*ei).IsD())								// it has not been deleted
					if(!hasHEF || ( hasHEF &&  (*ei).HFp()!=NULL))	// if it has a pointer to the face it is
																	// not null (i.e. it is not a border edge)
					if(!hV[(*ei)] )									// it has not be visited yet
					{
						if(!hasHEF)// if it has
							fp = &(* Allocator<MeshType>::AddFaces(m,1));
						else
							fp = (*ei).HFp();

						ep = epF = &(*ei);
						std::vector<VertexPointer> vpts;
						do{vpts.push_back((*ep).HVp()); ep=ep->HNp();}while(ep!=epF);
						int idbg  =fp->VN();
						if(fp->VN() != vpts.size()){
							fp->Dealloc();
							fp ->Alloc(vpts.size());
						}
						int idbg1  =fp->VN();
						for(unsigned int i  = 0; i < vpts.size();++i) fp ->V(i) = vpts[i];// set the pointer from face to vertex

  					hV[(*ei)] = true;
				}
				//Allocator<MeshType>::DeletePerHEdgeAttribute(m,hV);
			}

	/**
	Checks pointers FHEp() are valid
	**/
	static bool CheckConsistency_FHp(MeshType &  m){
		assert(MeshType::FaceType::HasFHAdjacency());
		FaceIterator fi;
		for(fi = m.face.begin(); fi != m.face.end(); ++fi)
			if(!(*fi).IsD()){
				if((*fi).FHp() <  &(*m.hedge.begin())) return false;
				if((*fi).FHp() >  &(m.hedge.back())) return false;
			}
		return true;
	}

	/**
	Checks that half edges and face relation are consistent
	**/
	static bool CheckConsistency(MeshType & m){
		assert(MeshType::HEdgeType::HasHNextAdjacency());
		assert(MeshType::HEdgeType::HasHOppAdjacency());
		assert(MeshType::HEdgeType::HasHVAdjacency());
		assert(MeshType::FaceType::HasFHAdjacency());

		bool hasHEF = ( MeshType::HEdgeType::HasHFAdjacency());
		bool hasHEP = ( MeshType::HEdgeType::HasHPrevAdjacency());

		FaceIterator fi;
		HEdgePointer ep,ep1;
		int cnt = 0;
		if(( MeshType::HEdgeType::HasHFAdjacency())){
			int iDb = 0;
			for(fi = m.face.begin(); fi != m.face.end(); ++fi,++iDb)
				if(!(*fi).IsD())
				{
					ep = ep1 = (*fi).FHp();
					do{
						if(ep->IsD()) 
							return false; // the edge should not be connected, it has been deleted
						if(ep->HFp() != &(*fi))
							return false;// edge is not pointing to the rigth face
						ep = ep->HNp();
						if(cnt++ > m.hn)
							return false; // edges are ill connected (HENp())
					}while(ep!=ep1);
				}
		}

		HEdgePointer epPrev;
		HEdgeIterator ei;
		bool extEdge ;
		for( ei  = m.hedge.begin(); ei != m.hedge.end(); ++ei)
			if(!(*ei).IsD())
		{
			cnt = 0;
			epPrev = ep = ep1 = &(*ei);
			do{
				extEdge = (ep->HFp()==NULL);
				if(hasHEP){
					if( ep->HNp()->HPp() != ep)
						return false; // next and prev relation are not mutual
					if( ep->HPp() == ep)
						return false; // the previous of an edge cannot be the edge itself
				}
				if( ep->HOp()  == ep)
					return false; // opposite relation is not mutual
				if( ep->HOp()->HOp() != ep)
					return false; // opposite relation is not mutual
				if(ep->HNp() == ep)
					return false; //  the next of an edge cannot be the edge itself
				ep = ep->HNp();
				if( ep->HVp() != epPrev->HOp()->HVp())
					return false; // the opposite edge points to a vertex different that the vertex of the next edge
				epPrev = ep;
				if(cnt++ > m.hn) 
					return false; // edges are ill connected (HENp())
			}while(ep!=ep1);
		}

	return true;
	}

	/** Set the relations HFp(), FHp() from a loop of edges to a face
	*/
	private:
	static void SetRelationsLoopFace(HEdgeType * e0, FaceType * f){
		assert(HEdgeType::HasHNextAdjacency());
		assert(FaceType::HasFHAdjacency());

		HEdgeType *e = e0;
		assert(e!=NULL);
		do{ e->HFp() = f; e = e->HNp(); } while(e != e0);
		f->FHp() = e0;
	}

	/**
	Merge the two faces. This will probably become a class template or a functor
	*/
	static void MergeFaces(FaceType *, FaceType *){};

	/**
	Find previous hedge in the loop
	*/
	static HEdgeType *  PreviousEdge(HEdgeType * e0){
		HEdgeType *  ep = e0;
		do{
			if(ep->HNp() == e0) return ep;
				ep = ep->HNp();
			}while(ep!=e0);
		assert(0); // degenerate loop
		return 0;
	}

	public:
	/** Adds an edge between the sources of e0 and e1 and set all the topology relations.
	If the edges store the pointers to the faces then a new face is created.
    <--- e1 ---- X <------e1_HEPp---
                 ^ 	
                 ||
             ei0 || ei1     
                 ||
                  v
	 ----e0_HEPp-> X ----- e0 ------>
	*/
	static void AddHEdge(MeshType &m, HEdgeType * e0, HEdgeType * e1){
		HEdgeType *iii =e0->HNp();
		assert(e1!=e0->HNp());
		assert(e0!=e1->HNp());
		HEdgePointer tmp;
		bool hasP =  MeshType::HEdgeType::HasHPrevAdjacency();
		assert(e0->HOp() != e1); // the hedge already exists
		assert(e0!=e1->HNp());

		std::vector<typename MeshType::HEdgePointer* > toUpdate;
		toUpdate.push_back(&e0);
		toUpdate.push_back(&e1);
		HEdgeIterator ei0  = vcg::tri::Allocator<MeshType>::AddHEdges(m,2,toUpdate);

		HEdgeIterator ei1 = ei0; ++ei1;
		(*ei0).HNp() = e1;(*ei0).HVp() = e0->HVp();
		(*ei1).HNp() = e0;(*ei1).HVp() = e1->HVp();

		HEdgePointer e0_HEPp = 0,e1_HEPp = 0,ep =0;
		if(hasP){
			e0_HEPp = e0->HPp();
			e1_HEPp = e1->HPp();
		}else{// does not have pointer to previous, it must be computed
			ep = e0;
			do{
				if(ep->HNp() == e0) e0_HEPp = ep;
				if(ep->HNp() == e1) e1_HEPp = ep;
				ep = ep->HNp();
			}while(ep!=e0);
		}
		if(hasP){
			(*ei0).HPp() = e0->HPp();
			(*ei1).HPp() = e1->HPp();
			e0->HPp() = &(*ei1);
			e1->HPp() = &(*ei0);
		}
		e0_HEPp -> HNp() = &(*ei0);
		e1_HEPp -> HNp() = &(*ei1);

		(*ei0).HOp() = &(*ei1);
		(*ei1).HOp() = &(*ei0);


		if( HEdgeType::HasHFAdjacency() && FaceType::HasFHAdjacency()){
			FaceIterator fi0  = vcg::tri::Allocator<MeshType>::AddFaces(m,1);
			m.face.back().ImportLocal(*e0->HFp());

			SetRelationsLoopFace(&(*ei0),e1->HFp());		// one loop to the old face
			SetRelationsLoopFace(&(*ei1),&m.face.back());	// the other  to the new face
		}
	}

	/** Detach the topology relations of a given edge
    <--- e->HENPp -X --- <---------eO_HEPp---
                   ^ 	
                   ||
               e   || e->HEOp()     
                   ||
                    v
	 ----e_HEPp--> X ----- e->HEOp->HENPp() ------>
	 
	*/
	static void RemoveHEdge(MeshType &m, HEdgeType * e){
		assert(MeshType::HEdgeType::HasHNextAdjacency());
		assert(MeshType::HEdgeType::HasHOppAdjacency());
		assert(MeshType::FaceType::HasFHAdjacency());

		bool hasP =  MeshType::HEdgeType::HasHPrevAdjacency();
		HEdgePointer e_HEPp,eO_HEPp;

		if(hasP){
			e_HEPp = e->HPp();
			eO_HEPp = e->HOp()->HPp();
		}else{
			e_HEPp = PreviousEdge(e);
			eO_HEPp = PreviousEdge(e->HOp());
		}

		assert(e_HEPp->HNp() == e);
		assert(eO_HEPp->HNp() == e->HOp());
		e_HEPp->HNp() = e->HOp()->HNp();
		eO_HEPp->HNp() = e-> HNp();

		if(hasP) {
			e->HOp()->HNp()->HPp() = e_HEPp;
			e->HNp()->HPp() = eO_HEPp;

			e->HPp() = NULL;
			e-> HOp()->HPp() = NULL;
		}


		// take care of the faces
		if(MeshType::HEdgeType::HasHFAdjacency()){
			MergeFaces(e_HEPp->HFp(),eO_HEPp->HFp());
			vcg::tri::Allocator<MeshType>::DeleteFace(m,*eO_HEPp->HFp());
			SetRelationsLoopFace(e_HEPp,e_HEPp->HFp());

		}
		 vcg::tri::Allocator<MeshType>::DeleteHEdge(m,*e->HOp());
		 vcg::tri::Allocator<MeshType>::DeleteHEdge(m,*e);

	}

	};// end class EdgeSupport 
} // end namespace vcg
}
#endif // __VCGLIB_EDGE_SUPPORT
