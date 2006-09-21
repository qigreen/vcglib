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

#ifndef VCG_SPACE_NORMAL_EXTRAPOLATION_H
#define VCG_SPACE_NORMAL_EXTRAPOLATION_H

#include <vcg/math/matrix33.h>
#include <vcg/math/linear.h> 
#include <vcg/math/lin_algebra.h>

#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/index/octree.h>

#include <vcg/math/disjoint_set.h>

#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <stdlib.h>

namespace vcg
{
	/*!
	*/
	template < class VERTEX_CONTAINER >
	class NormalExtrapolation
	{
	public:
		typedef typename VERTEX_CONTAINER::value_type	VertexType;
		typedef typename VertexType									 *VertexPointer;
		typedef typename VERTEX_CONTAINER::iterator		VertexIterator;
		typedef typename VertexType::CoordType				CoordType;
		typedef typename VertexType::NormalType				NormalType;
		typedef typename VertexType::ScalarType				ScalarType;
		typedef typename vcg::Box3< ScalarType >			BoundingBoxType;
		typedef typename vcg::Matrix33<ScalarType> MatrixType;

		enum NormalOrientation {IsCorrect=0, MustBeFlipped=1};

	public:
		/*!
		*/
		static void ExtrapolateNormlas(const VertexIterator &begin, const VertexIterator &end, int k, const int root_index=-1, NormalOrientation orientation=IsCorrect)
		{
			/*************************************************
			*		Inner class definitions
			**************************************************/
			// Dummy class: no object marker is needed
			class DummyObjectMarker {};
			// Object functor: return the bounding-box enclosing a given vertex
			struct BoundingBoxForVertexFunctor
			{
				inline BoundingBoxType operator()( const VertexType &vertex ) const 
				{ BoundingBoxType bb; bb.Set(vertex.P()); return bb; }
			};

			// Object functor: compute the distance between a vertex and a point
			struct VertPointDistanceFunctor
			{
				inline bool operator()(const VertexType &v, const CoordType &p, ScalarType &d, CoordType &q) const 
				{ 
					float distance = vcg::Distance(p, v.P()); 
					if (distance>d) 
						return false;

					d = distance;
					q = v.P();
					return true;
				}
			};
			// Plane structure: identify a plain as a <center, normal> pair
			struct Plane
			{
				Plane() { center.Zero(); normal.Zero();};
				CoordType		center;
				NormalType	normal;
				int					index;
			};

			// Object functor: compute the distance between a point and the plane
			struct PlanePointDistanceFunctor
			{
				inline bool operator()(const Plane &plane, const vcg::Point3f &p, float &d, vcg::Point3f &q) const 
				{ 
					float distance = vcg::Distance(p, plane.center); 
					if (distance>d) 
						return false;

					d = distance;
					q = plane.center;
					return true;
				}
			};

			// Object functor: return the bounding-box enclosing a given plane
			struct BoundingBoxForPlaneFunctor
			{
				inline BoundingBoxType	operator()( const Plane &plane ) const 
				{ BoundingBoxType bb; bb.Set(plane.center); return bb; }
			};

			// Represent an edge in the Riemannian graph
			struct RiemannianEdge
			{
				RiemannianEdge(Plane *p=NULL, ScalarType w=std::numeric_limits<ScalarType>::max())  {plane=p; weight=w; }

				Plane			*plane;
				ScalarType weight;
			};
			// Represent an edge in the MST tree
			struct MSTEdge
			{
				MSTEdge(Plane *p0=NULL, Plane *p1=NULL, ScalarType w=std::numeric_limits<ScalarType>::max()) {u=p0; v=p1; weight=w;};
				inline bool operator<(const MSTEdge &e) const {return weight<e.weight;}

				Plane			*u;
				Plane			*v;
				ScalarType weight;
			};
			// Represent a node in the MST tree
			struct MSTNode
			{
				MSTNode(MSTNode* p=NULL)  {parent=p;}

				MSTNode											*parent;
				VertexPointer								 vertex;
				std::vector< MSTNode* >			 sons;
			};
			
			/*************************************************
			*		The Algorithm
			**************************************************/
			BoundingBoxType dataset_bb;
			for (VertexIterator iter=begin; iter!=end; iter++)
				dataset_bb.Add(iter->P());
			float max_distance = dataset_bb.Diag();

			// Step 1: identify the tangent planes used to locally approximate the surface
			int vertex_count = int( std::distance(begin, end) );
			std::vector< Plane > tangent_planes(vertex_count);		
			vcg::Octree< VertexType, ScalarType > octree_for_planes;
			octree_for_planes.Set< VertexIterator , BoundingBoxForVertexFunctor >(begin, end, dataset_bb, BoundingBoxForVertexFunctor());

			std::vector< VertexPointer > nearest_vertices;
			std::vector< CoordType	 	 > nearest_points;
			std::vector< ScalarType		 > distances;
			for (VertexIterator iter=begin; iter!=end; iter++)
			{
				octree_for_planes.GetKClosest<VertPointDistanceFunctor, DummyObjectMarker, std::vector<VertexPointer>, std::vector<ScalarType>, std::vector<CoordType> >
					(VertPointDistanceFunctor(), DummyObjectMarker(), k, iter->P(), max_distance, nearest_vertices, distances, nearest_points);

				// for each vertex *iter, compute the centroid as avarege of the k-nearest vertices of *iter
				Plane *plane = &tangent_planes[ std::distance(begin, iter) ];
				for (int n=0; n<k; n++)
					plane->center += nearest_points[n];
				plane->center /= float(k);

				// then, identity the normal associated to the centroid
				MatrixType	covariance_matrix;
				CoordType diff;
				covariance_matrix.SetZero();
				for (int n=0; n<k; n++)
				{
					diff = nearest_points[n] - plane->center;
					for (int i=0; i<3; i++) 
						for (int j=0; j<3; j++) 
							covariance_matrix[i][j]+=diff[i]*diff[j];
				}

				CoordType		eigenvalues;
				MatrixType	eigenvectors;
				int required_rotations;
				vcg::Jacobi< MatrixType, CoordType >(covariance_matrix, eigenvalues, eigenvectors, required_rotations);
				vcg::SortEigenvaluesAndEigenvectors< MatrixType, CoordType >(eigenvalues, eigenvectors);
				for (int d=0; d<3; d++)
					plane->normal[d] = eigenvectors[d][2];
				plane->normal.Normalize();
				
				plane->index = int( std::distance(begin, iter) );
			}
			
			// Step 2: build the Riemannian graph, i.e. the graph where each point is connected to the k-nearest neigbours.
			dataset_bb.SetNull();
			std::vector< Plane >::iterator ePlane = tangent_planes.end();
			for (std::vector< Plane >::iterator iPlane=tangent_planes.begin(); iPlane!=ePlane; iPlane++)
				dataset_bb.Add(iPlane->center);
			max_distance = dataset_bb.Diag();

			vcg::Octree< Plane, ScalarType > octree_for_plane;
			octree_for_plane.Set< std::vector<Plane>::iterator, BoundingBoxForPlaneFunctor >(tangent_planes.begin(), tangent_planes.end(), dataset_bb, BoundingBoxForPlaneFunctor());
			std::vector< Plane* >	nearest_planes(distances.size());
			std::vector< std::vector< RiemannianEdge > > riemannian_graph(vertex_count); //it's probably that we are wasting the last position...
			for (std::vector< Plane >::iterator iPlane=tangent_planes.begin(); iPlane!=ePlane; iPlane++)
			{
				octree_for_plane.GetKClosest< PlanePointDistanceFunctor, DummyObjectMarker,  std::vector< Plane* >, std::vector< ScalarType >, std::vector< CoordType > >
					(PlanePointDistanceFunctor(), DummyObjectMarker(), k, iPlane->center, max_distance, nearest_planes, distances, nearest_points, true, false);

				for (int n=0; n<k; n++)
					if (iPlane->index<nearest_planes[n]->index)
						riemannian_graph[iPlane->index].push_back( RiemannianEdge( nearest_planes[n],  1.0f - fabs(iPlane->normal * nearest_planes[n]->normal)) );
			}

			// Step 3: compute the minimum spanning tree (MST) over the Riemannian graph (we use the Kruskal algorithm)
			std::vector< MSTEdge > E;
			std::vector< std::vector< RiemannianEdge > >::iterator iRiemannian = riemannian_graph.begin();
			std::vector< RiemannianEdge >::iterator iRiemannianEdge, eRiemannianEdge;
			for (int i=0; i<vertex_count; i++, iRiemannian++)
				for (iRiemannianEdge=iRiemannian->begin(), eRiemannianEdge=iRiemannian->end(); iRiemannianEdge!=eRiemannianEdge; iRiemannianEdge++)
					E.push_back(MSTEdge(&tangent_planes[i], iRiemannianEdge->plane, iRiemannianEdge->weight));

			std::sort( E.begin(), E.end() );
			vcg::DisjointSet<Plane> set;
			
			for (std::vector< Plane >::iterator iPlane=tangent_planes.begin(); iPlane!=ePlane; iPlane++)
				set.MakeSet( &*iPlane );

			std::vector< MSTEdge >::iterator iMSTEdge = E.begin();
			std::vector< MSTEdge >::iterator eMSTEdge = E.end();
			std::vector< MSTEdge > unoriented_tree;
			Plane *u, *v;
			for ( ; iMSTEdge!=eMSTEdge; iMSTEdge++)
				if ((u=set.FindSet(iMSTEdge->u))!=(v=set.FindSet(iMSTEdge->v)))
					unoriented_tree.push_back( *iMSTEdge ), set.Union(u, v);
			E.clear();

			// compute for each plane the list of sorting edges
			std::vector< std::vector< int > > incident_edges(vertex_count);
			iMSTEdge = unoriented_tree.begin();
			eMSTEdge = unoriented_tree.end();
			for ( ; iMSTEdge!=eMSTEdge; iMSTEdge++)
			{
				int u_index = int(iMSTEdge->u->index);
				int v_index = int(iMSTEdge->v->index);
				incident_edges[ u_index ].push_back( v_index ),
				incident_edges[ v_index ].push_back( u_index );
			}

			// Traverse the incident_edges vector and build the MST
			VertexIterator iCurrentVertex, iSonVertex;
			std::vector< MSTNode > MST(vertex_count);
			
			std::vector< Plane >::iterator iFirstPlane = tangent_planes.begin();
			std::vector< Plane >::iterator iCurrentPlane, iSonPlane;
			
			MSTNode *mst_root;
			int r_index = (root_index!=-1)? root_index : rand()*vertex_count/RAND_MAX;
			mst_root = &MST[ r_index ];
			mst_root->parent = mst_root; //the parent of the root is the root itself
			
			if (orientation==MustBeFlipped)
			{
				iCurrentVertex = begin;
				std::advance(iCurrentVertex, r_index);
				iCurrentVertex->N() = iCurrentVertex->N()*ScalarType(-1.0f);
			}

			{ // just to limit the scope of the variable border
				std::queue< int > border;
				border.push(r_index);
				while (!border.empty())
				{
					int current_node_index = border.front();  border.pop();

					MSTNode *current_node = &MST[current_node_index];					//retrieve the pointer to the current MST node
					std::advance((iCurrentVertex=begin), current_node_index);	//retrieve the pointer to the correspective vertex
					current_node->vertex = &*iCurrentVertex;									//and associate it to the MST node

					std::vector< int >::iterator iSon = incident_edges[ current_node_index ].begin(); 
					std::vector< int >::iterator eSon = incident_edges[ current_node_index ].end();
					for ( ; iSon!=eSon; iSon++)
					{
						MSTNode *son = &MST[ *iSon ];
						if (son->parent==NULL) // the node hasn't been visited
						{
							son->parent = current_node;								// Update the MST nodes
							current_node->sons.push_back(son);
							//std::advance((iSonVertex=begin), *iSon);//retrieve the pointer to the Vertex associated to son
							border.push( *iSon );
						}
					}
				}
			}

			// and finally visit the MST tree in order to propagate the normals
			{
				std::queue< MSTNode* > border;
				border.push(mst_root);
				while (!border.empty())
				{
					MSTNode *current_node = border.front(); border.pop();
					//std::vector< MSTNode* >::iterator iMSTSon = current_node->sons.begin();
					//std::vector< MSTNode* >::iterator eMSTSon = current_node->sons.end();
					for (int s=0; s<int(current_node->sons.size()); s++)
					{
						if (current_node->vertex->N()*current_node->sons[s]->vertex->N()<ScalarType(0.0f))
							current_node->sons[s]->vertex->N() *= ScalarType(-1.0f);
						border.push( current_node->sons[s] );
					}
				}
			}
		};
	};

};//end of namespace vcg

#endif //end of VCG_SPACE_NORMAL_EXTRAPOLATION_H