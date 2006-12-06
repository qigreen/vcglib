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
/****************************************************************************
History

$Log: not supported by cvs2svn $
Revision 1.25  2006/12/06 00:12:53  cignoni
Heavily restructured and corrected. Now a single Close ear function
Corrected Hole search function, and management of double non manifold vertex in a hole
Changed priority strategy in the heap, now a mix of quality and dihedral angle.
Changed but still untested IntersectionEar

Revision 1.24  2006/12/01 21:24:16  cignoni
Corrected bug in the search of holes. Removed output prints

Revision 1.23  2006/12/01 08:53:55  cignoni
Corrected pop_heap vs pop_back issue in heap usage

Revision 1.22  2006/12/01 00:11:17  cignoni
Added Callback, Corrected some spelling errors (adiacense -> adjacency).
Added Validity Check function for hole loops

Revision 1.21  2006/11/30 11:49:20  cignoni
small gcc compiling issues

Revision 1.20  2006/11/29 16:21:45  cignoni
Made static exposed funtions of the class

Revision 1.19  2006/11/29 15:25:22  giec
Removed limit.

Revision 1.18  2006/11/29 15:18:49  giec
Code refactory and bugfix.

Revision 1.17  2006/11/24 10:42:39  mariolatronico
Now compiles on gcc under linux.

Revision 1.16  2006/11/22 13:43:28  giec
Code refactory and added minimum weight triangolation.

Revision 1.15  2006/11/13 10:11:38  giec
Clear some useless code

Revision 1.14  2006/11/07 15:13:56  zifnab1974
Necessary changes for compilation with gcc 3.4.6. Especially the hash function is a problem

Revision 1.13  2006/11/07 11:47:11  cignoni
gcc compiling issues

Revision 1.12  2006/11/07 07:56:43  cignoni
Added missing std::

Revision 1.11  2006/11/06 16:12:29  giec
Leipa ear now compute max dihedral angle.

Revision 1.10  2006/10/31 11:30:41  ganovelli
changed access throught iterator with static call to comply 2005 compiler

Revision 1.9  2006/10/20 07:44:45  cignoni
Added missing std::

Revision 1.8  2006/10/18 15:06:47  giec
New policy for compute quality in TrivialEar.
Bugfixed LeipaEar.
Added new algorithm "selfintersection" with test for self intersection.

Revision 1.7  2006/10/10 09:12:02  giec
Bugfix and added a new type of ear (Liepa like)

Revision 1.6  2006/10/09 10:07:07  giec
Optimized version of "EAR HOLE FILLING", the Ear is selected according to its dihedral angle.

Revision 1.5  2006/10/06 15:28:14  giec
first working implementationof "EAR HOLE FILLING".

Revision 1.4  2006/10/02 12:06:40  giec
BugFix

Revision 1.3  2006/09/27 15:33:32  giec
It close one simple hole . . .

Revision 1.2  2006/09/27 09:29:53  giec
Frist working release whit a few bugs.
It almost fills the hole ...

Revision 1.1  2006/09/25 09:17:44  cignoni
First Non working Version

****************************************************************************/
#ifndef __VCG_TRI_UPDATE_HOLE
#define __VCG_TRI_UPDATE_HOLE

#include <wrap/callback.h>
#include <vcg/math/base.h>
#include <vcg/complex/trimesh/clean.h>
#include <vcg/space/point3.h>
#include <vector>
#include <float.h>

namespace vcg {
	namespace tri {

		/*
		Un ear e' identificato da due hedge pos.
		i vertici dell'ear sono
		e0.VFlip().v
		e0.v
		e1.v
		Vale che e1== e0.NextB();
		e che e1.FlipV() == e0;
		Situazioni ear non manifold, e degeneri (buco triangolare) 

		T  XXXXXXXXXXXXX    A        /XXXXX        B      en/XXXXX
		/XXXXXXXXXXXXXXX            /XXXXXX                /XXXXXX
		XXXXXXep==en XXX     ep\   /en XXXX               /e1 XXXX
		XXXXXX ----/| XX   ------ ----/| XX       ------ ----/|XXX
		XXXXXX|   /e1 XX   XXXXXX|   /e1 XX       XXXXXX|  o/e0 XX 
		XXXXXX|  /XXXXXX   XXXXXX|  /XXXXXX       XXXXXX|  /XXXXXX 
		XXX e0|o/XXXXXXX   XXX e0|o/XXXXXXX       XXX ep| /XXXXXXX
		XXX  \|/XXXXXXXX   XXX  \|/XXXXXXXX       XXX  \|/XXXXXXXX
		XXXXXXXXXXXXXXXX   XXXXXXXXXXXXXXXX       XXXXXXXXXXXXXXXX   
		*/
		template<class MESH> class TrivialEar
		{
		public:
      typedef typename MESH::FaceType FaceType;
      typedef typename MESH::FacePointer FacePointer;
			typedef typename face::Pos<FaceType>    PosType;
      typedef typename MESH::ScalarType ScalarType;
      typedef typename MESH::CoordType CoordType;

			PosType e0;	 
			PosType e1;	 
      CoordType n; // the normal of the face defined by the ear
      const char * Dump() {return 0;}
      const CoordType &cP(int i) const {return P(i);}
      const CoordType &P(int i) const {
        switch(i) {
          case 0 : return e0.v->cP();
          case 1 : return e1.v->cP();
          case 2 : return e0.VFlip()->cP();
          default: assert(0);
        }
        return e0.v->cP();
      }

			ScalarType quality;
			ScalarType angle;
			//std::vector<typename MESH::FaceType>* vf;
      TrivialEar(){}
			TrivialEar(const PosType & ep)
			{
				e0=ep;
				assert(e0.IsBorder());
				e1=e0;
				e1.NextB();
        n=Normal<TrivialEar>(*this);
				ComputeQuality();
				ComputeAngle();
			}

	    /// Compute the angle of the two edges of the ear.
      // it tries to make the computation in a precision safe way.
      // the angle computation takes into account the case of reversed ears 
			void ComputeAngle()
			{       
        angle=Angle(cP(2)-cP(0), cP(1)-cP(0));
        ScalarType flipAngle = n * e0.v->N();
        if(flipAngle<0)		angle = (2.0 *(float)M_PI) - angle;
    	}

			virtual inline bool operator < ( const TrivialEar & c ) const { return quality <  c.quality; }

			bool IsNull(){return e0.IsNull() || e1.IsNull();}
			void SetNull(){e0.SetNull();e1.SetNull();}
			virtual	void ComputeQuality() {	quality = QualityFace(*this) ; };
			bool IsUpToDate()	{return ( e0.IsBorder() && e1.IsBorder());};
      // An ear is degenerated if both of its two endpoints are non manifold.
      bool IsDegen(const int nonManifoldBit) 
      {
        if(e0.VFlip()->IsUserBit(nonManifoldBit) && e1.V()->IsUserBit(nonManifoldBit))
          return true;
        else return false;
      }
			bool IsConcave() const {return(angle > (float)M_PI);}

			virtual bool Close(PosType &np0, PosType &np1, FaceType * f)
			{
				// simple topological check
				if(e0.f==e1.f) {
					//printf("Avoided bad ear");
					return false;
				}

				//usato per generare una delle due nuove orecchie.
				PosType	ep=e0; ep.FlipV(); ep.NextB(); ep.FlipV(); // he precedente a e0 
				PosType	en=e1; en.NextB();												 // he successivo a e1
        
				(*f).V(0) = e0.VFlip();
				(*f).V(1) = e0.v;
				(*f).V(2) = e1.v;
        ComputeNormal(*f);

				(*f).FFp(0) = e0.f;
				(*f).FFi(0) = e0.z;
				(*f).FFp(1) = e1.f;
				(*f).FFi(1) = e1.z;
				(*f).FFp(2) = f;
				(*f).FFi(2) = 2;

				e0.f->FFp(e0.z)=f;
				e0.f->FFi(e0.z)=0;	

				e1.f->FFp(e1.z)=f;
				e1.f->FFi(e1.z)=1;	

				// caso ear degenere per buco triangolare
				if(ep==en)
				{
					//printf("Closing the last triangle");
					f->FFp(2)=en.f;
					f->FFi(2)=en.z;
					en.f->FFp(en.z)=f;
					en.f->FFi(en.z)=2;
					np0.SetNull();
					np1.SetNull();
				}
				// Caso ear non manifold a
				else if(ep.v==en.v)
				{
					//printf("Ear Non manif A\n");
					PosType	enold=en;
					en.NextB();
					f->FFp(2)=enold.f;
					f->FFi(2)=enold.z;
					enold.f->FFp(enold.z)=f;
					enold.f->FFi(enold.z)=2;
					np0=ep;
					np1=en;
				}
				// Caso ear non manifold b
				else if(ep.VFlip()==e1.v)
				{
					//printf("Ear Non manif B\n");
					PosType	epold=ep; 
					ep.FlipV(); ep.NextB(); ep.FlipV();
					f->FFp(2)=epold.f;
					f->FFi(2)=epold.z;
					epold.f->FFp(epold.z)=f;
					epold.f->FFi(epold.z)=2;
					np0=ep;  // assign the two new 
					np1=en;  // pos that denote the ears
				}
				else // caso standard // Now compute the new ears;
				{
					np0=ep;
					np1=PosType(f,2,e1.v);
				}

				return true;
			}
		};

		//Ear with FillHoleMinimumWeight's quality policy
		template<class MESH> class MinimumWeightEar : public TrivialEar<MESH>
		{
		public:
			typename MESH::ScalarType dihedralRad;
			typename MESH::ScalarType aspectRatio;
      const char * Dump() {
        static char buf[200]; 
        if(IsConcave()) sprintf(buf,"Dihedral (deg) %6.2f Quality %6.2f\n",math::ToDeg(dihedralRad),aspectRatio);
                  else sprintf(buf,"Dihedral-(deg) %6.2f Quality %6.2f\n",math::ToDeg(dihedralRad),aspectRatio);
        return buf;
      }
      
			MinimumWeightEar(){}
      MinimumWeightEar(const PosType & ep) : TrivialEar<MESH>(ep)
			{
        ComputeQuality();
			}

      // in the heap we retrieve the LARGEST value, 
      // so if we need the ear with minimal dihedral angle, we must reverse the sign of the comparison.
	/*		virtual inline bool operator <  ( const MinimumWeightEar & c ) const 
			{ 
				if(IsConcave() == c.IsConcave())
        {
				  if(dihedralRad > c.dihedralRad) return true; 
				  else return ((dihedralRad == c.dihedralRad) && (aspectRatio > c.aspectRatio));
        }
        if(IsConcave()) return true;
          return false;
			}*/

      	virtual inline bool operator <  ( const MinimumWeightEar & c ) const 
			{ 

				if(IsConcave() == c.IsConcave())
        {
          return pow(dihedralRad,1)> pow(c.dihedralRad,1)/c.aspectRatio;
        }
        if(IsConcave()) return true;
          return false;
			}

			virtual void ComputeQuality()
			{
				//compute quality by (dihedral ancgle, area/sum(edge^2) )
	      Point3f n1=e0.FFlip()->cN();
        Point3f n2=e1.FFlip()->cN();

				dihedralRad = std::max(Angle(n,n1),Angle(n,n2));
        aspectRatio = QualityFace(*this) ;
			}

		};
		//Ear for selfintersection algorithm
		template<class MESH> class SelfIntersectionEar : public MinimumWeightEar<MESH>
		{
		public:
	    static std::vector<FacePointer> &AdjacencyRing() 
      {
        static std::vector<FacePointer> ar;
        return ar;
      }

			SelfIntersectionEar(){}
      SelfIntersectionEar(const PosType & ep):MinimumWeightEar<MESH>(ep){}
			//{
			//	this->e0=ep;
			//	assert(this->e0.IsBorder());
			//	this->e1=this->e0;
			//	this->e1.NextB();
			//	this->ComputeQuality();
			//	this->ComputeAngle();
			//}

			virtual bool Close(PosType &np0, PosType &np1, FacePointer f)
			{
				PosType	ep=e0; ep.FlipV(); ep.NextB(); ep.FlipV(); // he precedente a e0 
				PosType	en=e1; en.NextB();												 // he successivo a e1
				//costruisco la faccia e poi testo, o copio o butto via.				
				(*f).V(0) = e0.VFlip();
				(*f).V(1) = e0.v;
				(*f).V(2) = e1.v;

				(*f).FFp(0) = e0.f;
				(*f).FFi(0) = e0.z;
				(*f).FFp(1) = e1.f;
				(*f).FFi(1) = e1.z;
				(*f).FFp(2) = f;
				(*f).FFi(2) = 2; 

				int a1, a2;
				a1= e0.z;
				a2= e1.z;

				e0.f->FFp(e0.z)=f;
				e0.f->FFi(e0.z)=0;	

				e1.f->FFp(e1.z)=f;
				e1.f->FFi(e1.z)=1;
				std::vector<FacePointer>::iterator it;
				for(it = AdjacencyRing().begin();it!= AdjacencyRing().end();++it)
				{
					if(!(*it)->IsD())
						if(	tri::Clean<MESH>::TestIntersection(&(*f),*it))
						{
							e0.f->FFp(e0.z)= e0.f;
							e0.f->FFi(e0.z)=a1;	

							e1.f->FFp(e1.z)=e1.f;
							e1.f->FFi(e1.z)=a2;
							return false;
						}
				}
        //return ((TrivialEar<MESH> *)this)->Close(np0,np1,f);
        e0.f->FFp(e0.z)= e0.f;
        e0.f->FFi(e0.z)=a1;	

        e1.f->FFp(e1.z)=e1.f;
        e1.f->FFi(e1.z)=a2;

        bool ret=TrivialEar<MESH>::Close(np0,np1,f);
        if(ret) AdjacencyRing().push_back(f);
        return ret;
      }
		};

		// Funzione principale per chiudier un buco in maniera topologicamente corretta.
		// Gestisce situazioni non manifold ragionevoli 
		// (tutte eccetto quelle piu' di 2 facce per 1 edge).
		// Controlla che non si generino nuove situazioni non manifold chiudendo orecchie
		// che sottendono un edge che gia'esiste.

template <class MESH>
class Hole
{
public:
			typedef typename MESH::VertexType				VertexType;
			typedef typename MESH::VertexPointer		VertexPointer;
			typedef	typename MESH::ScalarType				ScalarType;
			typedef typename MESH::FaceType					FaceType;
			typedef typename MESH::FacePointer			FacePointer;
			typedef typename MESH::FaceIterator			FaceIterator;
			typedef typename MESH::CoordType				CoordType;
      typedef typename vcg::Box3<ScalarType>  Box3Type;
			typedef typename face::Pos<FaceType>    PosType;

public:

		class Info
		{
		public: 
			Info(){}
			Info(PosType const &pHole, int  const pHoleSize, Box3<ScalarType> &pHoleBB)
			{
				p=pHole;	
				size=pHoleSize;
				bb=pHoleBB;
			}

			PosType p;
			int size;
			Box3Type  bb;
			
			bool operator <  (const  Info & hh) const {return size <  hh.size;}

			ScalarType Perimeter()
			{
				ScalarType sum=0;
				PosType ip = p;
				do
				{
					sum+=Distance(ip.v->cP(),ip.VFlip()->cP());
					ip.NextB();
				}
				while (ip != p);
				return sum;				
			}

      // Support function to test the validity of a single hole loop
      // for now it test only that all the edges are border;
      // The real test should check if all non manifold vertices 
      // are touched only by edges belonging to this hole loop.
      bool CheckValidity()
      {
       if(!p.IsBorder()) 
         return false;
       PosType ip=p;ip.NextB();
       for(;ip!=p;ip.NextB())
       {
          if(!ip.IsBorder()) 
            return false;
       }       
       return true;
      }
		};



template<class EAR>
	static void FillHoleEar(MESH &m, Info &h ,int UBIT, std::vector<FacePointer *> &app,std::vector<FaceType > *vf =0)
		{
      //Aggiungo le facce e aggiorno il puntatore alla faccia!
			FaceIterator f = tri::Allocator<MESH>::AddFaces(m, h.size-2, app);
			assert(h.p.f >= &*m.face.begin());
			assert(h.p.f < &*m.face.end());
      assert(h.p.IsBorder());//test fondamentale altrimenti qualcosa s'e' rotto!
			std::vector< EAR > H; 
			H.reserve(h.size);
      int nmBit= VertexType::NewBitFlag(); // non manifoldness bit

      //First loops around the hole to mark non manifold vertices.
      PosType ip = h.p;   // Pos iterator 
			do{  
        ip.V()->ClearUserBit(nmBit);
        ip.V()->ClearV();
        ip.NextB();
			} while(ip!=h.p); 

      ip = h.p;   // Re init the pos iterator for another loop (useless if everithing is ok!!)
			do{  
        if(!ip.V()->IsV())
             ip.V()->SetV();   // All the vertexes that are visited more than once are non manifold
        else ip.V()->SetUserBit(nmBit);
        ip.NextB();
			} while(ip!=h.p); 

			PosType fp = h.p;
			do{
				EAR app = EAR(fp);
				H.push_back( app );
        printf("Adding ear %s ",app.Dump());
				fp.NextB();
				assert(fp.IsBorder());
			}while(fp!=h.p);

			int cnt=h.size;
			
			make_heap(H.begin(), H.end());
  		
			//finche' il buco non e' chiuso o non ci sono piu' orecchie da analizzare.
			while( cnt > 2 && !H.empty() ) 
			{	   
        printf("Front of the heap is %s", H.front().Dump());
        pop_heap(H.begin(), H.end());	 // retrieve the MAXIMUM value and put in the back;
				PosType ep0,ep1;
        EAR BestEar=H.back();
        H.pop_back();
				if(BestEar.IsUpToDate() && !BestEar.IsDegen(nmBit))	
				{				 
						if(BestEar.Close(ep0,ep1,&*f))
						{
							if(!ep0.IsNull()){
								H.push_back(EAR(ep0));
								push_heap( H.begin(), H.end());
							}
							if(!ep1.IsNull()){
								H.push_back(EAR(ep1));
								push_heap( H.begin(), H.end());
							}
							--cnt;
							f->SetUserBit(UBIT);
							if(vf != 0)	(*vf).push_back(*f);
							++f;
						}
				}//is update()				
			}//fine del while principale.
			//tolgo le facce non utilizzate.
			while(f!=m.face.end())
			{
				(*f).SetD();
				++f;
				m.fn--;
			}

	  VertexType::DeleteBitFlag(nmBit); // non manifoldness bit
	}




template<class EAR>//!!!
	static void EarCuttingFill(MESH &m, int sizeHole,bool Selected = false, CallBackPos *cb=0)
		{
			std::vector< Info > vinfo;
			int UBIT = GetInfo(m, Selected,vinfo);

			typename std::vector<Info >::iterator ith;
			//Info app;
			int ind=0;

			std::vector<FacePointer *> vfp;
			for(ith = vinfo.begin(); ith!= vinfo.end(); ++ith)
					vfp.push_back( &(*ith).p.f );
			
			for(ith = vinfo.begin(); ith!= vinfo.end(); ++ith)
			{
				ind++;
        if(cb) (*cb)(ind*100/vinfo.size(),"Closing Holes");
        if((*ith).size < sizeHole){		
					FillHoleEar< EAR >(m, *ith,UBIT,vfp);
				}
			}

			FaceIterator fi;
			for(fi = m.face.begin(); fi!=m.face.end(); ++fi)
			{
				if(!(*fi).IsD())
					(*fi).ClearUserBit(UBIT);
			}
		}



template<class EAR>
	static void EarCuttingIntersectionFill(MESH &m, int sizeHole,bool Selected = false)
		{
			std::vector<Info > vinfo;
			int UBIT = GetInfo(m, Selected,vinfo);
			std::vector<FaceType > vf;
			PosType sp;
			PosType ap;
			typename std::vector<Info >::iterator ith;
			
   		// collect the face pointer that has to be updated by the various addfaces 
      std::vector<FacePointer *> vfpOrig;
			for(ith = vinfo.begin(); ith!= vinfo.end(); ++ith)
					vfpOrig.push_back( &(*ith).p.f );
	
      
			for(ith = vinfo.begin(); ith!= vinfo.end(); ++ith)
			{
				if((*ith).size < sizeHole){		
          std::vector<FacePointer *> vfp;
          
          vfp=vfpOrig;
          EAR::AdjacencyRing().clear();
					//Loops around the hole to collect the races .
  				PosType ip = (*ith).p;
					do
					{
					  PosType inp = ip;
						do
						{
							inp.FlipE();
							inp.FlipF();
							EAR::AdjacencyRing().push_back(inp.f);
						} while(!inp.IsBorder());
						ip.NextB();
					}while(ip != (*ith).p);	

          std::vector<FacePointer>::iterator fpi;
          for(fpi=EAR::AdjacencyRing().begin();fpi!=EAR::AdjacencyRing().end();++fpi)
					      vfp.push_back( &*fpi );
              
					FillHoleEar<EAR >(m, *ith,UBIT,vfp,&vf);
					EAR::AdjacencyRing().clear();
				}
			}
			FaceIterator fi;
			for(fi = m.face.begin(); fi!=m.face.end(); ++fi)
			{
				if(!(*fi).IsD())
					(*fi).ClearUserBit(UBIT);
			}
		}



	static int GetInfo(MESH &m,bool Selected ,std::vector<Info >& VHI)
		{
			FaceIterator fi;
			int UBIT = FaceType::LastBitFlag();

			for(fi = m.face.begin(); fi!=m.face.end(); ++fi)
			{
				if(!(*fi).IsD())
				{
					if(Selected && !(*fi).IsS())
					{
						//se devo considerare solo i triangoli selezionati e 
						//quello che sto considerando non lo e' lo marchio e vado avanti
						(*fi).SetUserBit(UBIT);
					}
					else
					{
							for(int j =0; j<3 ; ++j)
							{
                if( face::IsBorder(*fi,j) && !(*fi).IsUserBit(UBIT) )
								{//Trovato una faccia di bordo non ancora visitata.
									(*fi).SetUserBit(UBIT);
							    PosType sp(&*fi, j, (*fi).V(j));
									PosType fp=sp;
									int holesize=0;

									Box3Type hbox;
									hbox.Add(sp.v->cP());
                  //printf("Looping %i : (face %i edge %i) \n", VHI.size(),sp.f-&*m.face.begin(),sp.z);
                  sp.f->SetUserBit(UBIT);
									do
									{
										sp.f->SetUserBit(UBIT);
										hbox.Add(sp.v->cP());
										++holesize;
										sp.NextB();
										sp.f->SetUserBit(UBIT);
										assert(sp.IsBorder());
									}while(sp != fp);

									//ho recuperato l'inofrmazione su tutto il buco
									VHI.push_back( Info(sp,holesize,hbox) );
								}
							}//for sugli edge del triangolo
					}//S & !S
				}//!IsD()
			}//for principale!!!
			return UBIT;
		}

		//Minimum Weight Algorithm
		class Weight
		{
		public:

			Weight() { ang = 180; ar = FLT_MAX ;}
			Weight( float An, float Ar ) { ang=An ; ar= Ar;}
			~Weight() {}

			float angle() const { return ang; }
			float area()  const { return ar; }

			Weight operator+( const Weight & other ) const {return Weight( std::max( angle(), other.angle() ), area() + other.area());}
			bool operator<( const Weight & rhs ) const {return ( angle() < rhs.angle() ||(angle() == rhs.angle() && area() < rhs.area()));	}

		private:
			float ang;
			float ar;
		};

		/*     
    \ /        \/ 
   v1*---------*v4 
    / \       /
   /   \     /
  / 	  \   /
 /ear	   \ /
*---------*-
| v3      v2\
*/
		
	static float ComputeDihedralAngle(CoordType  p1,CoordType  p2,CoordType  p3,CoordType  p4)
		{
			CoordType  n1 = ((p1 - p2) ^ (p3 - p1) ).Normalize();
			CoordType	 n2 = ((p2 - p1) ^ (p4 - p2) ).Normalize();
			ScalarType t = (n1 * n2 )  ;
			return  math::ToDeg(acos(t));
		}

  static bool existEdge(PosType pi,PosType pf)
		{
			PosType app = pi;
			PosType appF = pi;
			PosType tmp;
			assert(pi.IsBorder());
			appF.NextB();
			appF.FlipV();
			do
			{
				tmp = app;
				tmp.FlipV();
				if(tmp.v == pf.v)
					return true;
				app.FlipE();
				app.FlipF();

				if(app == pi)return false;
			}while(app != appF);
			return false;
		}

	static Weight computeWeight( int i, int j, int k,
			std::vector<PosType > pv,
			std::vector< std::vector< int > >  v)
		{
			PosType pi = pv[i];
			PosType pj = pv[j];
			PosType pk = pv[k];

			//test complex edge
			if(existEdge(pi,pj) || existEdge(pj,pk)|| existEdge(pk,pi)	)	
			{
				return Weight();
			}
			// Return an infinite weight, if one of the neighboring patches
			// could not be created.
			if(v[i][j] == -1){return Weight();}
			if(v[j][k] == -1){return Weight();}

			//calcolo il massimo angolo diedrale, se esiste.
			float angle = 0.0f;
			PosType px;
			if(i + 1 == j)
			{
				px = pj; 
				px.FlipE(); px.FlipV();
				angle = std::max<float>(angle , ComputeDihedralAngle(pi.v->P(), pj.v->P(), pk.v->P(), px.v->P())	);
			}
			else
			{
				angle = std::max<float>( angle, ComputeDihedralAngle(pi.v->P(),pj.v->P(), pk.v->P(), pv[ v[i][j] ].v->P()));
			}

			if(j + 1 == k)
			{
				px = pk; 
				px.FlipE(); px.FlipV();
				angle = std::max<float>(angle , ComputeDihedralAngle(pj.v->P(), pk.v->P(), pi.v->P(), px.v->P())	);
			}
			else
			{
				angle = std::max<float>( angle, ComputeDihedralAngle(pj.v->P(),pk.v->P(), pi.v->P(), pv[ v[j][k] ].v->P()));
			}

			if( i == 0 && k == (int)v.size() - 1)
			{
				px = pi; 
				px.FlipE(); px.FlipV();
				angle = std::max<float>(angle , ComputeDihedralAngle(pk.v->P(), pi.v->P(),  pj.v->P(),px.v->P() )	);
			}

			ScalarType area = ( (pj.v->P() - pi.v->P()) ^ (pk.v->P() - pi.v->P()) ).Norm() * 0.5;

			return Weight(angle, area);
		}

	static std::vector<VertexPointer > calculateMinimumWeightTriangulation(MESH &m, std::vector<PosType > vv )
		{
			std::vector< std::vector< Weight > > w; //matrice dei pesi minimali di ogni orecchio preso in conzideraione
			std::vector< std::vector< int    > > vi;//memorizza l'indice del terzo vertice del triangolo

			//hole size
			int nv = vv.size();

			w.clear();
			w.resize( nv, std::vector<Weight>( nv, Weight() ) );

			vi.resize( nv, std::vector<int>( nv, 0 ) );

			//inizializzo tutti i pesi possibili del buco
			for ( int i = 0; i < nv-1; ++i )
				w[i][i+1] = Weight( 0, 0 );

			//doppio ciclo for per calcolare di tutti i possibili triangoli i loro pesi.
			for ( int j = 2; j < nv; ++j )
			{
				for ( int i = 0; i + j < nv; ++i )
				{
					//per ogni triangolazione mi mantengo il minimo valore del peso tra i triangoli possibili
					Weight minval;

					//indice del vertice che da il peso minimo nella triangolazione corrente
					int minIndex = -1;

					//ciclo tra i vertici in mezzo a i due prefissati
					for ( int m = i + 1; m < i + j; ++m )
					{
						Weight a = w[i][m];
						Weight b = w[m][i+j];
						Weight newval =  a + b + computeWeight( i, m, i+j, vv, vi);
						if ( newval < minval )
						{
							minval = newval;
							minIndex = m;
						}
					}
					w[i][i+j] = minval;
					vi[i][i+j] = minIndex;
				}
			}

			//Triangulate
			int i, j;
			i=0; j=nv-1;
			std::vector<VertexPointer > vf;

			vf.clear();

			triangulate(vf, i, j, vi, vv);
			return vf;
		}


	static void triangulate(std::vector<VertexPointer > &m,int i, int j, std::vector< std::vector<int> > vi, 
			std::vector<PosType > vv)
		{
			if(i + 1 == j){return;}
			if(i==j)return;

			int k = vi[i][j];

			if(k == -1)	return;

			m.push_back(vv[i].v);
			m.push_back(vv[k].v);
			m.push_back(vv[j].v);

			triangulate(m, i, k, vi, vv);
			triangulate(m, k, j, vi, vv);
		}

  static void MinimumWeightFill(MESH &m, bool Selected)
		{
			FaceIterator fi;
			std::vector<PosType > vvi;
			std::vector<FacePointer * > vfp;

			std::vector<Info > vinfo;
			typename std::vector<Info >::iterator VIT;
			int UBIT = GetInfo(m, Selected,vinfo);

			for(VIT = vinfo.begin(); VIT != vinfo.end();++VIT)
			{
				vvi.push_back(VIT->p);
			}

			typename std::vector<PosType >::iterator ith;
			typename std::vector<PosType >::iterator ithn;
			typename std::vector<VertexPointer >::iterator itf;

			std::vector<PosType > app;
			PosType ps;
			std::vector<FaceType > tr;
			std::vector<VertexPointer > vf;

			for(ith = vvi.begin(); ith!= vvi.end(); ++ith)
			{
				tr.clear();
				vf.clear();
				app.clear();
				vfp.clear();

				for(ithn = vvi.begin(); ithn!= vvi.end(); ++ithn)
					vfp.push_back(&(ithn->f));

				ps = *ith;
				getBoundHole(ps,app);

				vf = calculateMinimumWeightTriangulation(m, app);

				if(vf.size() == 0)continue;//non e' stata trovata la triangolazione

				FaceIterator f = tri::Allocator<MESH>::AddFaces(m, app.size()-2, vfp);

				for(itf = vf.begin();itf != vf.end(); )
				{
					(*f).V(0) = (*itf++);
					(*f).V(1) = (*itf++);
					(*f).V(2) = (*itf++);
					++f;
				}

			}		

		}


	static void getBoundHole (PosType sp,std::vector<PosType >&ret)
		{
			PosType fp = sp;
			//take vertex around the hole
			do
			{
				assert(fp.IsBorder());
				ret.push_back(fp);
				fp.NextB();
			}while(sp != fp);
		}

};//close class Hole
			

			

	
		

	} // end namespace
}
#endif
