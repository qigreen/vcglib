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
****************************************************************************/

#ifndef __VCG_MINDISTPOINT
#define __VCG_MINDISTPOINT
#include <math.h>

#include <vcg/space/point3.h>
#include <vcg/space/box3.h>
#include <vcg/space/point4.h>
#include <vcg/math/base.h>
#include <vcg/space/index/grid_static_ptr.h>


using namespace vcg;


/*
aka MetroCore
data una mesh m e una ug sulle sue facce trova il punto di m piu' vicino ad
un punto dato.
*/

// input: mesh, punto, griglia, distanza limite
// output: normale alla faccia e punto piu' vicino su di essa

// Nota che il parametro template GRID non ci dovrebbe essere, visto che deve essere 
// UGrid<MESH::FaceContainer >, ma non sono riuscito a definirlo implicitamente 

template <class MESH, class GRID, class SCALAR>
void MinDistPoint( MESH & mesh, const Point3<SCALAR> & p, GRID & gr, SCALAR & mdist, 
									Point3<SCALAR> & normf, Point3<SCALAR> & bestq, typename MESH::FaceType * &f, Point3<SCALAR> &ip)
{
	typedef SCALAR scalar;
  typedef Point3<scalar> Point3x;
  typedef Box3<SCALAR> Box3x;
	
	if(!gr.bbox.IsIn(p)) return;
	typedef typename GridStaticPtr<typename MESH::FaceContainer>::Link A2UGridLink;
  scalar ax = p[0] - gr.bbox.min[0];	// Real coodinate of point refer to
  scalar ay = p[1] - gr.bbox.min[1];	
  scalar az = p[2] - gr.bbox.min[2];

  int gx = int( ax/gr.voxel[0] );		// Integer coordinate of the point 
  int gy = int( ay/gr.voxel[1] );		// voxel
  int gz = int( az/gr.voxel[2] );

  scalar vx = gr.bbox.min[0]+gx*gr.voxel[0];	// Real world coordinate of the Voxel
  scalar vy = gr.bbox.min[1]+gy*gr.voxel[1];	// origin
	scalar vz = gr.bbox.min[2]+gz*gr.voxel[2];

	scalar dx = math::Min(p[0] - vx, vx+gr.voxel[0]-p[0]);  // Dist from the voxel
  scalar dy = math::Min(p[1] - vy, vy+gr.voxel[1]-p[1]);
  scalar dz = math::Min(p[2] - vz, vz+gr.voxel[2]-p[2]);

	scalar vdist,vstep;

	if(dx<dy && dx<dz)
	{
	    vdist = dx;
	    vstep = gr.voxel[0];
	}
	else if(dy<dz)
	{
	    vdist = dy;
	    vstep = gr.voxel[1];
	}
	else
	{
	    vdist = dz;
	    vstep = gr.voxel[2];
	}

	//scalar error = gr.bbox.SquaredDiag();
	//scalar error = gr.bbox.Diag();
	scalar error = mdist;
	Point3x q;
	typename MESH::FaceIterator bestf = (typename MESH::FaceIterator)0;

  mesh.UnMarkAll();

	int mxsd = gr.siz[0];
	if(mxsd<gr.siz[1]) mxsd = gr.siz[1];
	if(mxsd<gr.siz[2]) mxsd = gr.siz[2];
	for(int s=0;s<mxsd;++s)
	{
		if(s==0)
		{

			A2UGridLink *first, *last, *l;
			gr.Grid( gx, gy, gz, first, last );
			for(l=first;l!=last;++l)

				if( ! mesh.IsMarked( &*(l->Elem())) )
			{
				if( Dist((*(l->Elem())), p, error, q) )
				{
					bestq = q;
					bestf = l->Elem();
					typename MESH::ScalarType alfa=1, beta=1, gamma=1;
					
					//bestf->InterpolationParameters(q, alfa, beta);
					//calcolo normale con interpolazione trilineare
					/*normf = (1-(alfa+beta))*(bestf->V(0)->Normal())+
							(alfa*(bestf->V(1)->Normal()))+
							(beta*(bestf->V(2)->Normal()));*/
					bool ret=bestf->InterpolationParameters(q, alfa, beta, gamma);
					//assert(ret);
					normf =       (bestf->V(0)->cN())*alfa+
							          (bestf->V(1)->cN())*beta+
							          (bestf->V(2)->cN())*gamma;
					normf.Normalize();
					ip[0]=alfa;ip[1]=beta;ip[2]=gamma;

				}

				mesh.Mark( &*(l->Elem()) );
			}
		}
		else
		{
			for(int ix=gx-s;ix<=gx+s;++ix)
				if( ix>=0 && ix<gr.siz[0] )
				{
					for(int iy=gy-s;iy<=gy+s;++iy)
						if( iy>=0 && iy<gr.siz[1] )
						{
							int sz = ( ix==gx-s || ix==gx+s ||
								       iy==gy-s || iy==gy+s   )?1:2*s;
							for(int iz=gz-s;iz<=gz+s;iz+=sz)
								if( iz>=0 && iz<gr.siz[2] )
								{
									A2UGridLink *first, *last, *l;
									gr.Grid( ix, iy, iz, first, last );
									for(l=first;l!=last;++l)
									if( ! mesh.IsMarked( &*(l->Elem())) )
									{
										if( Dist((*(l->Elem())),  p, error, q) )
										{
											bestq = q;
											bestf = l->Elem();
											typename MESH::ScalarType alfa, beta, gamma;
											//bestf->InterpolationParameters(q, alfa, beta);
											//calcolo normale con interpolazione trilineare
											bestf->InterpolationParameters(q, alfa, beta, gamma);
											normf =  (bestf->V(0)->cN())*alfa+
											         (bestf->V(1)->cN())*beta+
													     (bestf->V(2)->cN())*gamma ;
											ip[0]=alfa;ip[1]=beta;ip[2]=gamma;
											//normf.Normalize(); inutile si assume le normali ai vertici benfatte
										}
										mesh.Mark(&*l->Elem());
									}
								}
						}
				}
		}

		if( fabs(error)<vdist )
			break;
		vdist += vstep;
	}
	f=&*bestf;
	mdist = scalar(fabs(error));
}

template <class MESH, class GRID, class SCALAR>
void MinDistPoint( MESH & mesh, const Point3<SCALAR> & p, GRID & gr, SCALAR & mdist, 
									Point3<SCALAR> & normf, Point3<SCALAR> & bestq, typename MESH::face_type * &f)
{
	Point3<SCALAR> ip;
	MinDistPoint(mesh,p,gr,mdist,normf,bestq,f,ip);
}
#endif
