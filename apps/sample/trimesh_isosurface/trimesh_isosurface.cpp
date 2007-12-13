#include <stdio.h>
#include <wrap/io_trimesh/export_ply.h>
#include <vcg/space/point3.h>
#include <vcg/space/box3.h>
#include <vcg/math/perlin_noise.h>
#include <vcg/simplex/vertexplus/base.h>
#include <vcg/simplex/faceplus/base.h>
#include <vcg/complex/trimesh/base.h>
#include <vcg/complex/trimesh/allocate.h>
#include <vcg/complex/trimesh/create/marching_cubes.h>
#include <vcg/complex/trimesh/create/extended_marching_cubes.h>


#include "simple_volume.h"
#include "trivial_walker.h"

using namespace std;
using namespace vcg;


typedef float ScalarType;

class MyEdge;
class MyFace;

class MyVertex     : public VertexSimp2< MyVertex,  MyEdge, MyFace, vert::Coord3f>{};
class MyFace       : public FaceSimp2< MyVertex,    MyEdge, MyFace, face::VertexRef, face::BitFlags> {};

//class MyVertex  : public vcg::Vertex< ScalarType, MyEdge, MyFace > {};
//class MyFace		: public vcg::Face< MyVertex, MyEdge, MyFace> {};

class MyMesh		: public vcg::tri::TriMesh< std::vector< MyVertex>, std::vector< MyFace > > {};



typedef SimpleVolume<SimpleVoxel> MyVolume;

int main(int /*argc*/ , char /**argv[]*/)
{
	MyVolume	volume;
  
  typedef vcg::tri::TrivialWalker<MyMesh,MyVolume>	MyWalker;
	typedef vcg::tri::MarchingCubes<MyMesh, MyWalker>	MyMarchingCubes;
	MyWalker walker;
	

  // Simple initialization of the volume with some cool perlin noise
	volume.Init(Point3i(64,64,64));
  for(int i=0;i<64;i++)
    for(int j=0;j<64;j++)
      for(int k=0;k<64;k++)
        volume.Val(i,j,k)=(j-32)*(j-32)+(k-32)*(k-32)  + i*10*(float)math::Perlin::Noise(i*.2,j*.2,k*.2);


	// MARCHING CUBES
	MyMesh		mc_mesh;
	printf("[MARCHING CUBES] Building mesh...");
	MyMarchingCubes					mc(mc_mesh, walker);
	walker.BuildMesh<MyMarchingCubes>(mc_mesh, volume, mc, 20*20);
	vcg::tri::io::ExporterPLY<MyMesh>::Save( mc_mesh, "marching_cubes.ply");

	printf("OK!\n");
};