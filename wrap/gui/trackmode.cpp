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
Revision 1.20  2007/05/15 14:58:57  benedetti
Main restructuring. added many new modes

Revision 1.19  2006/08/30 07:01:54  cignoni
Reverted to version 1.17. Version 1.18 was wrongly done starting from a outdated version.

Revision 1.17  2006/07/26 13:54:45  cignoni
Reversed the direction of wheel scaling and added middle mouse panning

Revision 1.16  2006/02/13 13:10:27  cignoni
Added Zmode for moving objects along the perpendicular to the viewplane

Revision 1.15  2006/01/12 15:40:05  cignoni
Corrected small bugs on rotating after scaling+translating
changed void PlaneMode::Apply and void SphereMode::Apply

Revision 1.14  2005/07/15 16:39:30  callieri
in SphereMode::Hit added a check on the sphere intersection, if no intersection, calculating distance could generate a NAN exception

Revision 1.13  2005/06/29 15:22:26  callieri
changed the name of some intersection functions to avoid ambiguity

Revision 1.12  2005/02/11 11:44:07  tommyfranken
Trackball translation corrected

Revision 1.11  2005/02/02 16:46:41  pietroni
some warning corrected

Revision 1.10  2005/01/14 15:44:03  ganovelli
PlaneMode completed

Revision 1.9  2004/09/09 22:59:21  cignoni
Removed many small warnings

Revision 1.8  2004/07/18 06:54:08  cignoni
Added Scaling

Revision 1.7  2004/07/11 22:06:56  cignoni
Added scaling by wheel

Revision 1.6  2004/06/09 14:01:13  cignoni
Heavily restructured. To be completed only rotation works...

Revision 1.5  2004/05/14 03:15:09  ponchio
Redesigned partial version.

Revision 1.4  2004/05/07 12:46:08  cignoni
Restructured and adapted in a better way to opengl

Revision 1.3  2004/04/07 10:54:11  cignoni
Commented out unused parameter names and other minor warning related issues

Revision 1.2  2004/03/25 14:55:25  ponchio
Adding copyright.


****************************************************************************/

#include <gl/glew.h>

#include <wrap/gui/trackmode.h>
#include <wrap/gui/trackball.h>
#include <wrap/gui/trackutils.h>

 
using namespace std;
using namespace vcg;
using namespace vcg::trackutils;

// the old implementation is not used anymore, some of the old support functions, 
// like HitViewPlane, GetViewPlane, HitHyper and SphereMode::Hit were made
// class-independent and moved to trackutils.h
/*
void TrackMode::Apply(Trackball *trackball, float WheelNotch) {
}
void ScaleMode::Apply(Trackball *tb, Point3f new_point) {
}
Plane3f TrackMode::GetViewPlane(const View<float> &camera, const Point3f &center) {
}
Point3f TrackMode::HitViewPlane(Trackball *tb, const Point3f &p) {
}
void SphereMode::Apply(Trackball *tb, Point3f new_point) {
}
bool SphereMode::HitHyper(Point3f center,  float radius, Point3f viewpoint, Plane3f vp, Point3f hitplane, Point3f &hit) 
{
}
Point3f SphereMode::Hit(Trackball *tb, const Point3f &p) {
}
void PlaneMode::Apply(Trackball *tb, Point3f new_point) {

}
void ZMode::Apply(Trackball *tb, Point3f new_point) {
}
*/

// Track mode implementation, dummy.
void TrackMode::Apply (Trackball * , float ){}

void TrackMode::Apply (Trackball * , Point3f ){}

void TrackMode::Draw(Trackball * ){}

void TrackMode::SetAction (){}

void TrackMode::Reset (){}

// draw an inactive trackball
void InactiveMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,false);
}
 
// Sphere mode implementation.
// the most important function; given a new point in window coord,
// it update the transformation computed by the trackball.
// General scheme : the transformation is a function of just 
//   the begin and current mouse positions, with greater precision 
//   is function of just two 3d points over the manipulator.
void SphereMode::Apply (Trackball * tb, Point3f new_point)
{
  Point3f hitOld = HitSphere (tb, tb->last_point);
  Point3f hitNew = HitSphere (tb, new_point);
  tb->Hits.push_back (hitNew);
  Point3f center = tb->center;
  Point3f axis = (hitNew - center) ^ (hitOld - center);
  //  Figure out how much to rotate around that axis.
  float phi = Distance (hitNew, hitOld) / tb->radius;
  tb->track.rot = tb->last_track.rot * Quaternionf (phi, axis);
}

void SphereMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
}

// Pan mode implementation.
void PanMode::Apply (Trackball * tb, Point3f new_point)
{
  Point3f hitOld = HitViewPlane (tb, tb->last_point);
  Point3f hitNew = HitViewPlane (tb, new_point);
  tb->Translate (hitNew - hitOld);
}

void PanMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  DrawUglyPanMode(tb);
}

// Z mode implementation.
void ZMode::Apply (Trackball * tb, float WheelNotch)
{
  Point3f dir= (GetViewPlane (tb->camera, tb->center)).Direction();
  dir.Normalize();
  tb->Translate (dir * (-WheelNotch));
}

void ZMode::Apply (Trackball * tb, Point3f new_point)
{
  Point3f dir= (GetViewPlane (tb->camera, tb->center)).Direction();
  dir.Normalize();
  tb->Translate (dir * ( -2.0f * getDeltaY(tb,new_point)));
}

void ZMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  DrawUglyZMode(tb);
 }

// Scale mode implementation.
void ScaleMode::Apply (Trackball * tb, float WheelNotch)
{
  tb->track.sca *= pow (1.2f, -WheelNotch);
}
  
void ScaleMode::Apply (Trackball * tb, Point3f new_point)
{
  tb->track.sca = tb->last_track.sca * pow (3.0f, -(getDeltaY(tb,new_point)));
}

void ScaleMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  DrawUglyScaleMode(tb);
}

// Axis mode implementation.
void AxisMode::Apply (Trackball * tb, float WheelNotch)
{
  tb->Translate (axis.Direction () * (WheelNotch / 10.0f));
}

void AxisMode::Apply (Trackball * tb, Point3f new_point)
{
  pair< Point3f,bool > hitOld = HitNearestPointOnAxis (tb, axis, tb->last_point);
  pair< Point3f,bool > hitNew = HitNearestPointOnAxis (tb, axis, new_point);
  if (hitOld.second && hitNew.second){
    tb->Translate (hitNew.first - hitOld.first);
  }
}

void AxisMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  DrawUglyAxisMode(tb,axis);
}

// Plane mode implementation.
void PlaneMode::Apply (Trackball * tb, Point3f new_point)
{
  pair< Point3f, bool > hitOld = HitPlane(tb,tb->last_point,plane);
  pair< Point3f, bool > hitNew = HitPlane(tb,new_point,plane);
  if(hitOld.second && hitNew.second){
  	  tb->Translate (hitNew.first - hitOld.first);  	
  }
}

void PlaneMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  DrawUglyPlaneMode(tb, plane);
}

// Cylinder mode implementation.
void CylinderMode::Apply (Trackball * tb, float WheelNotch)
{
  const float PI2=6.283185307179586232;
  tb->track.rot = tb->last_track.rot * Quaternionf (WheelNotch/(tb->radius * PI2),axis.Direction());
}

void CylinderMode::Apply (Trackball * tb, Point3f new_point)
{
  Plane3f viewplane=GetViewPlane (tb->camera, tb->center);
  Line3f axisproj;
  axisproj=ProjectLineOnPlane(axis,viewplane);
  float angle;
  const float EPSILON=0.005f; // this IS scale independent
  if(axisproj.Direction().Norm() < EPSILON){
    angle=(10.0 * getDeltaY(tb,new_point)) / tb->radius; 
  } else {
    Point3f hitOld = HitViewPlane (tb, tb->last_point);
    Point3f hitNew = HitViewPlane (tb, new_point);
    axisproj.Normalize();
    Point3f plusdir= viewplane.Direction() ^ axisproj.Direction();  
    float distOld = signedDistance(axisproj,hitOld,plusdir);
    float distNew = signedDistance(axisproj,hitNew,plusdir);
    angle= (distNew-distOld) / tb->radius;
  }
  tb->track.rot = tb->last_track.rot * Quaternionf (angle,axis.Direction());
}

void CylinderMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  DrawUglyCylinderMode(tb,axis);
}

// Path mode implementation.
void PathMode::Init(const vector < Point3f > &pts)
{
  unsigned int npts=pts.size();
  assert(npts >= 2);
  points.reserve(npts);
  for(unsigned int i=0;i<npts;i++){
    points.push_back(pts[i]);
  }
  path_length=0.0f;
  min_seg_length=Distance(points[0],points[1]);
  float seg_length;
  for(unsigned int i=1;i<npts;i++){
    seg_length=Distance(points[i-1],points[i]);
    path_length += seg_length;
    min_seg_length = min(seg_length,min_seg_length);
  }
  if(wrap){
    seg_length=Distance(points[npts-1],points[0]);
    path_length += seg_length;
    min_seg_length = min(seg_length,min_seg_length);
  }
}

void PathMode::Reset()
{
  current_state=initial_state;
}

Point3f PathMode::SetStartNear(Point3f point)
{
  float p0_state=0;
  Point3f p0,p1;
  float nearest_state=0;
  Point3f nearest_point=points[0];
  float nearest_distance=Distance(nearest_point,point);  
  unsigned int npts=points.size();
  for(unsigned int i = 1;i <= npts;i++){
  	if( i == npts){
  	  if (wrap){
  	  	p0=points[npts-1];
  	  	p1=points[0];
  	  } else {
  	  	break;
  	  }
  	} else {
  		p0=points[i-1];
  		p1=points[i];
  	}
    Point3f segment_point=ClosestPoint(Segment3f(p0,p1),point);
    float distance=Distance(segment_point,point);  
    if(distance<nearest_distance){
      nearest_point=segment_point;
      nearest_distance=distance;
      nearest_state=p0_state+(Distance(p0,nearest_point)/path_length);
    } 
    float segment_norm= Distance(p0,p1) / path_length;   
    p0_state+=segment_norm;
  }
  assert( nearest_state >= 0.0 );
  if(nearest_state > 1.0){
    nearest_state=1.0;
    nearest_point=( wrap ? points[0] : points[npts-1] );
  }
  initial_state=nearest_state;
  return nearest_point;
}

void PathMode::GetPoints(float state, Point3f & point, Point3f & prev_point, Point3f & next_point)
{
  assert(state >= 0.0f);
  assert(state <= 1.0f);  
  float remaining_norm=state;  
  Point3f p0,p1;
  unsigned int npts=points.size();
  for(unsigned int i = 1;i <= npts;i++){
  	if( i == npts){
  	  if (wrap){
  	  	p0=points[npts-1];
  	  	p1=points[0];
  	  } else {
  	  	break;
  	  }
  	} else {
  		p0=points[i-1];
  		p1=points[i];
  	}
    float segment_norm= Distance(p0,p1) / path_length;
    if (segment_norm < remaining_norm){
       remaining_norm -= segment_norm;
      continue;
    }
    prev_point = p0;
    next_point = p1;
    float ratio= remaining_norm / segment_norm;
    point = prev_point + (( next_point - prev_point ) * ratio);
  	const float EPSILON=min_seg_length * 0.01f;
    if(Distance(point,prev_point) < EPSILON){
      point=prev_point;
      if (i > 1){
       	prev_point=points[i-2];
      } else if (wrap){
        prev_point=points[npts-1];
      }     
    } else if (Distance(point,next_point) < EPSILON){
      point=next_point;
      if( i < (npts-1)){
       	next_point=points[i+1];
      } else {
       	if (wrap){
      	  next_point=points[1];
      	} else {
      	  next_point=points[npts-1];
      	}
      }      
    }
    return;
  }  
  // rounding errors can lead out of the for..
  prev_point = p0;
  point = p1;
  if (wrap){
    next_point=points[1];
  }else{
    next_point = points[npts-1];
  }
}

void PathMode::Apply (Trackball * tb, float WheelNotch)
{
  const float STEP_COEFF = min_seg_length * 0.5f;
  float delta=(WheelNotch*STEP_COEFF)/path_length;
  Point3f old_point,new_point,prev_point,next_point;
  GetPoints(current_state,old_point,prev_point,next_point);
  current_state=Normalize(current_state+delta);
  GetPoints(current_state,new_point,prev_point,next_point);
  tb->Translate (new_point - old_point);	
}

float PathMode::Normalize(float state)
{
  if ( wrap ) {
    double intpart;
    float fractpart;
    fractpart =(float) modf(state,&intpart);
    if( fractpart < 0.0f )
      fractpart += 1.0f;
    return fractpart;
  }
  if ( state < 0.0f )
    return 0.0f; 
  if ( state > 1.0f )
    return 1.0f;  
  return state;
}

int PathMode::Verse(Point3f reference_point,Point3f current_point,Point3f prev_point,Point3f next_point)
{
  Point3f reference_dir = reference_point - current_point ;
  Point3f prev_dir = prev_point - current_point ;
  Point3f next_dir = next_point - current_point ;
  const float EPSILON=min_seg_length * 0.005f;
  if (reference_dir.Norm()  < EPSILON)
    reference_dir = Point3f(0,0,0);
  if (prev_dir.Norm() < EPSILON)
    prev_dir = Point3f(0,0,0);
  if (next_dir.Norm()   < EPSILON)
    next_dir = Point3f(0,0,0);
  reference_dir.Normalize();
  prev_dir.Normalize();
  next_dir.Normalize();
  float prev_coeff,next_coeff;
  prev_coeff = prev_dir * reference_dir;
  next_coeff = next_dir * reference_dir;
  if (prev_coeff < 0.0f)
    prev_coeff = 0.0f;
   if (next_coeff < 0.0f)
    next_coeff = 0.0f;
  if( (prev_coeff == 0.0f) && (next_coeff == 0.0f)){
    return 0;
  }
  if ( prev_coeff <= next_coeff ){
    return 1;
  }
  return -1;
}

float PathMode::HitPoint(float state, Ray3fN ray, Point3f &hit_point)
{
 Point3f current_point, next_point, prev_point;
  GetPoints(state,current_point,prev_point,next_point);
  
  Point3f closest_point;
  closest_point=ray.ClosestPoint(current_point);
  int verse=Verse(closest_point,current_point,prev_point,next_point);
  if (verse == 0){
    hit_point=current_point;
    return 0.0f;
  }
 
  Segment3f active_segment;
  if (verse > 0){
  	active_segment=Segment3f(current_point,next_point);
  } else {
    active_segment= Segment3f(current_point,prev_point);
  } 

  hit_point=ClosestPoint(active_segment,closest_point);

  return verse * ((hit_point-current_point).Norm() / path_length);
}  

void PathMode::SetAction (){
  Point3f temp1,temp2;    
  GetPoints(current_state,old_hitpoint,temp1,temp2);
}
  
void PathMode::Apply (Trackball * tb, Point3f new_point)
{
    Ray3fN ray = line2ray(tb->camera.ViewLineFromWindow (new_point));
    Point3f hit_point;
    float delta_state=HitPoint(current_state,ray,hit_point);
    current_state=Normalize(current_state+delta_state);
    tb->Translate (hit_point - old_hitpoint);
}

void PathMode::Draw(Trackball * tb){
  DrawSphereIcon(tb,true );
  Point3f current_point,prev_point,next_point;
  GetPoints(current_state,current_point,prev_point,next_point);
  DrawUglyPathMode(tb,points,current_point,prev_point,
  next_point,old_hitpoint,wrap);
}

// Area mode implementation.
void AreaMode::Init(const vector < Point3f > &pts)
{
  unsigned int npts=pts.size();
  
  assert(npts >= 3);
  //get the plane
  Point3f p0=pts[0];
  unsigned int onethird=(unsigned int)floor(npts/3.0);
  const float EPSILON=0.005;
  bool pts_not_in_line=false;
  Point3f a,b;
  for(unsigned int i=0;i<onethird;i++){
   	 a=(pts[(i+   onethird )%npts] - pts[i%npts]).Normalize();
  	 b=(pts[(i+(2*onethird))%npts] - pts[i%npts]).Normalize();
     pts_not_in_line = (a ^ b).Norm() > EPSILON;
     if(pts_not_in_line){
     	plane.Init( pts[i%npts],
     	            pts[(i+(onethird))%npts],
                    pts[(i+(2*onethird))%npts]);
     	break;
     }
  }
  assert(pts_not_in_line);  
  float ncx,ncy,ncz;
  ncx=fabs(plane.Direction()[0]);
  ncy=fabs(plane.Direction()[1]);
  ncz=fabs(plane.Direction()[2]);  
  if(( ncx > ncy ) && ( ncx > ncz )){
    first_coord_kept=1;
    second_coord_kept=2;
  } else if(( ncy > ncx ) && ( ncy > ncz)){
    first_coord_kept=0;
    second_coord_kept=2;
  } else { 
    first_coord_kept=0;
    second_coord_kept=1;
  }
  points.reserve(npts);  
  for(unsigned int i=0;i<npts;i++){
   	points.push_back(plane.Projection(pts[i]));  	
  }
  min_side_length=Distance(points[0],points[1]);
  for(unsigned int i=1;i<npts;i++){
  	min_side_length=min(Distance(points[i-1],points[i]),min_side_length);
  }
  rubberband_handle=old_status=status=initial_status=p0;
}

void AreaMode::Reset()
{
  rubberband_handle=old_status=status=initial_status;
  path.clear();
}

void AreaMode::Apply (Trackball * tb, Point3f new_point)
{
  if(begin_action){
    delta_mouse=tb->camera.Project(status)-new_point;    
    begin_action=false; 
  }	
  pair< Point3f, bool > hitNew = HitPlane(tb,new_point+delta_mouse,plane);
  if(! hitNew.second){
  	  return;
  }
  Point3f hit_point=hitNew.first;  
  Point3f delta_status=Move(status,hit_point);
  status += delta_status;
  tb->Translate (status - old_status); 
  rubberband_handle=hit_point;
}


void AreaMode::SetAction ()
{
  begin_action=true;  
  old_status=status;
  
 
  path.clear();
  path.push_back(status);
  rubberband_handle=status;
}

Point3f AreaMode::Move(Point3f start,Point3f end)
{
  const float EPSILON=min_side_length*0.001f;
  Point3f pt=start;
  bool done=false;
  bool end_inside=Inside(end);
  while(!done){
  	path.push_back(pt); 
    Segment3f segment(pt,end);
    bool p_on_side = false;
    bool hit=false;
    
    Point3f pside,phit;
    bool slide,mid_inside;
    
    int np=points.size(), i, j;
    for (i = 0, j = np-1; i < np; j = i++) {
      Segment3f side(points[i],points[j]);
      Point3f pseg,psid;
      pair<float,bool> res=SegmentSegmentDistance(segment,side,pseg,psid);
      if(res.first < EPSILON && ! res.second){
      	float dist= Distance(pt,pseg);
      	if(dist < EPSILON){
          Point3f pn=ClosestPoint(side,end);
          if(!p_on_side || (Distance(pn,end)<Distance(end,pside))){
            pside=pn;
            p_on_side=true;
          }
      	} else {
      	  if (!hit || Distance(pt,pseg) < Distance(pt,phit)){
      	    phit=pseg;
      	    hit=true;
      	  }
      	}
      }
    }
    if (p_on_side)
      slide = Distance(pside,pt) > EPSILON;
    
    if (hit) 
      mid_inside = Inside( pt + ( ( phit - pt ) / 2) );

    if ( !hit && end_inside ){
      pt = end;
      done = true;
    } else if ( hit && (!p_on_side || (p_on_side && mid_inside))) {
      pt = phit;
    } else if ( p_on_side && slide) { 
      pt = pside;
    } else {
      done = true;
    }    
  }
  path.push_back(pt); 
  return pt - start;
}
  
// adapted from the original C code by W. Randolph Franklin
// http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
bool AreaMode::Inside(Point3f point)
{
  bool inside=false;
  float x=point[first_coord_kept];
  float y=point[second_coord_kept];
  float yi, yj, xi, xj;
  int i, j, np=points.size();
  for (i = 0, j = np-1; i < np; j = i++) {
  	xi=points[i][first_coord_kept];
  	yi=points[i][second_coord_kept];
  	xj=points[j][first_coord_kept];
  	yj=points[j][second_coord_kept];
    if ( ( ( (yi<=y) && (y<yj) ) || ( (yj<=y) && (y<yi) ) ) &&
         ( x < ( xj - xi ) * ( y - yi ) / ( yj - yi ) + xi )  )
    {
  	  inside=!inside;
  	}  	
   }
  return inside;
}

Point3f AreaMode::SetStartNear(Point3f point)
{
  Point3f candidate=plane.Projection(point);
  if (Inside(candidate)){
    initial_status=candidate;
    return initial_status;
  }
  Point3f nearest_point=initial_status;
  float nearest_distance=Distance(nearest_point,candidate);  
  int i, j, np=points.size();
  for (i = 0, j = np-1; i < np; j = i++) {
    Segment3f side(points[i],points[j]);
    Point3f side_point=ClosestPoint(side,candidate);
    float distance=Distance(side_point,candidate);
    if( distance < nearest_distance ){
      nearest_point=side_point;
      nearest_distance=distance;
    }    
  }
  initial_status=nearest_point;
  return initial_status;
}

void AreaMode::Draw(Trackball * tb)
{
  DrawSphereIcon(tb,true );
  DrawUglyAreaMode(tb,points,status,old_status,plane,path,rubberband_handle);
}
