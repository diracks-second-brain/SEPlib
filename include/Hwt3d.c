/*$
=head1 NAME

Hwt3d - 3-D ray tracing

=head1 SYNOPSIS

< velocity.H Hwt3d par= > rays.H/travel times.H

=head1 DESCRIPTION

Does Huygens wavefront tracing.  Can produce either
a ray database or traveltime cube

=head1 INPUT PARAMETERS

=over 4

=item verb  - logical 

      [0] whether or not to be verbose

=item interp- logical 

      [0] wheter or not to interpolate rays

=back

=head2 SHOT(S) LOCATION

=over 4

=item shots - sepfile 

      Sepfile containing shot locations (3[x,y,z],nshots)

=item xshot - float   

      [o3+d3*(n3-1)/2] Location for shot(s) (limit 1000)

=item yshot - float   

      [o3+d3*(n3-1)/2] Location for shot(s) (limit 1000)

=item zshot - float   

      [o3+d3*(n3-1)/2] Location for shot(s) (limit 1000)

=back

=head2 RAY PARAMETERS

=over 4

=item aimi  - float  

      aim inclination; aimi=0 is along the positive z

=item aima  - float  

      aim azimuth;     aima=0 is along the positive x (inline)
      positive values rotate clockwise

=item oi    - float 

      [0.]

=item ni    - int   

      [91] number of steps for half aperture; 91 means 90 steps

=item di    - float 

      [1.] angular step in the aperture direction;
      (na-1)*da=max aperture angle

=item da    - float 

      [1.] radial step
      number of radial steps is computed internally as nr=360/sr

=item dt    - float 

      [.004]  Sampling in time

=item nt    - int   

      [1000]  Number of time samples

=back

=head2 INTERPOLATION PARAMETERS

=over 4

=item fast  - int   

      [1]    Whether or not to do fast interpolation of travel times

=back

=head1 SEE ALSO

L<FMeikonal>, L<hwt_travel_cube>, L<hwt_trace_rays>

=head1 CATEGORY
B<seis/travel>

=cut
>*/
/*
AUTHOR
Paul Sava - SEP 1998

Modifed:
Bob Sept 99: Converted i/o, one main sub, multi-shots, combined tt3d hwt3d

*/


#include <septravel.h>
#include <hwt.h>
#include <seplib.h>
#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
void get_pars(int *n, float *o, float *d,int *nshots, float *aimi, float *aima, float *oi, int *ni, float *di, float *da,int *na, float *dt, int *nt,int *verb, int *interp,int *fast);
void put_pars(int interp,int fast,int *n, float *o, float *d,int nshots,float aimi, float aima, float oi, int ni, float di, float da, int na, float dt, int nt);
void rite_ray_out(int nt,int na,int ni,float *rays_out,rays pRR);
_XFUNCPROTOEND
#else
void get_pars();
void put_pars();
void rite_ray_out();
#endif



float *shots;
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Hwt3d - 3-D ray tracing");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    < velocity.H Hwt3d par= > rays.H/travel times.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Does Huygens wavefront tracing. Can produce either a ray database or");\
 sep_add_doc_line("    traveltime cube");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    verb - logical");\
 sep_add_doc_line("              [0] whether or not to be verbose");\
 sep_add_doc_line("");\
 sep_add_doc_line("    interp- logical");\
 sep_add_doc_line("              [0] wheter or not to interpolate rays");\
 sep_add_doc_line("");\
 sep_add_doc_line("  SHOT(S) LOCATION");\
 sep_add_doc_line("    shots - sepfile");\
 sep_add_doc_line("              Sepfile containing shot locations (3[x,y,z],nshots)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    xshot - float");\
 sep_add_doc_line("              [o3+d3*(n3-1)/2] Location for shot(s) (limit 1000)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    yshot - float");\
 sep_add_doc_line("              [o3+d3*(n3-1)/2] Location for shot(s) (limit 1000)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    zshot - float");\
 sep_add_doc_line("              [o3+d3*(n3-1)/2] Location for shot(s) (limit 1000)");\
 sep_add_doc_line("");\
 sep_add_doc_line("  RAY PARAMETERS");\
 sep_add_doc_line("    aimi - float");\
 sep_add_doc_line("              aim inclination; aimi=0 is along the positive z");\
 sep_add_doc_line("");\
 sep_add_doc_line("    aima - float");\
 sep_add_doc_line("              aim azimuth;     aima=0 is along the positive x (inline)");\
 sep_add_doc_line("              positive values rotate clockwise");\
 sep_add_doc_line("");\
 sep_add_doc_line("    oi - float");\
 sep_add_doc_line("              [0.]");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ni - int");\
 sep_add_doc_line("              [91] number of steps for half aperture; 91 means 90 steps");\
 sep_add_doc_line("");\
 sep_add_doc_line("    di - float");\
 sep_add_doc_line("              [1.] angular step in the aperture direction;");\
 sep_add_doc_line("              (na-1)*da=max aperture angle");\
 sep_add_doc_line("");\
 sep_add_doc_line("    da - float");\
 sep_add_doc_line("              [1.] radial step");\
 sep_add_doc_line("              number of radial steps is computed internally as nr=360/sr");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dt - float");\
 sep_add_doc_line("              [.004]  Sampling in time");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nt - int");\
 sep_add_doc_line("              [1000]  Number of time samples");\
 sep_add_doc_line("");\
 sep_add_doc_line("  INTERPOLATION PARAMETERS");\
 sep_add_doc_line("    fast - int");\
 sep_add_doc_line("              [1]    Whether or not to do fast interpolation of travel times");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    FMeikonal, hwt_travel_cube, hwt_trace_rays");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/travel");\
 sep_add_doc_line("");

#include<sep.main>
MAIN()
{
  int  n[3];
  float o[3];
  float d[3];
  float shot[3];
  float aimi,aima,oi,di,da,dt;
  int ni,nt,na,verb,nshots;
  float *vel;
  float *rays_out,*travel,*length;
  rays pRR;
  cube pVV,pTT,pLL;
  int ii,it,i,j,ia,ishot,interp,fast;
  
  get_pars(n,o,d,&nshots,&aimi,&aima,&oi,&ni,&di,&da,&na,&dt,&nt,&verb,&interp,&fast);
  
  if(verb==1) {
    fprintf(stderr,"interp=%d\n",interp);
    fprintf(stderr,"  fast=%d\n",fast);

    fprintf(stderr,"  aima=%f\n",aima);
    fprintf(stderr,"  aimi=%f\n",aimi);
    
    fprintf(stderr,"    oi=%f\n",oi);
    fprintf(stderr,"    di=%f\n",di);
    fprintf(stderr,"    ni=%d\n",ni);
    
    fprintf(stderr,"    da=%f\n",da);
    
    fprintf(stderr,"    dt=%f\n",dt);
    fprintf(stderr,"    nt=%d\n",nt);
  }
  
  
  if(verb==1) fprintf(stderr,"finished grabbing parameters \n");
  
  /*ALLOCATION*/
  vel=(float*) alloc(sizeof(float)*n[0]*n[1]*n[2]);
  init_rays(&pRR,shot,nt,dt,aimi,aima,oi,di,ni,da,na);
  vel=(float*) alloc(sizeof(float)*n[0]*n[1]*n[2]);
  if(interp==1) travel=(float*) alloc(sizeof(float)*n[0]*n[1]*n[2]);
  else rays_out=(float*) alloc(sizeof(float)*na*nt*3);
  if(verb==1) fprintf(stderr,"finished allocation \n");
  
  
  /*READ AND CONVERT */
  if(n[0]*n[1]*n[2]*4!=sreed("in",vel,n[0]*n[1]*n[2]*4))
    seperr("trouble reading in velocity\n");
  init_cube(&pVV,vel,n,o,d);
  if(verb==1) fprintf(stderr,"finished reading in velocity and setting structs\n");
  
  put_pars(interp,fast,n,o,d,nshots,aimi,aima,oi,ni,di,da,na,dt,nt);
  if(verb==1) fprintf(stderr,"finished putting parameters \n");
  
  
  for(ishot=0; ishot < nshots; ishot++){	
    pRR.xshot=shots[ishot*3];
    pRR.yshot=shots[ishot*3+1];
    pRR.zshot=shots[ishot*3+2];
    
    tracerays(&pRR,&pVV);
    
    if(verb==1) fprintf(stderr,"finished tracing rays for shot %d (%f,%f,%f) \n",
			ishot,shots[ishot*3],shots[ishot*3+1],shots[ishot*3+2]);
    
 
    if(interp==0) rite_ray_out(nt,na,ni,rays_out,pRR);
    else{
      tesselate_rays(&pRR,&pVV,travel,fast);
      if(verb==1) fprintf(stderr,"Finished tesselate \n");
      if(n[0]*n[1]*n[2]*4!=srite("out",travel,n[0]*n[1]*n[2]*4))
	seperr("trouble writing out travel times");
      if(verb==1) fprintf(stderr,"Finished write \n");
    }
  }
  free(rays_out); 
  free(vel);
  if(interp==1) free(travel);
  return(0);
}


#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
void get_pars(int *n, float *o, float *d, int *nshots, float *aimi, float *aima, float *oi, int *ni, float *di, float *da, int *na,float *dt, int *nt,int *verb,int *interp,int *fast)
     _XFUNCPROTOEND
#else
void get_pars(n,o,d,nshots,aimi,aima,oi,ni,di,da,na,dt,nt,verb,interp,fast)
     int *n,*ni,*nt,*verb,*na,*nshots,*interp,*fast;
     float *o,*d, *aimi, *aima,*oi,*di,*da,*dt;
#endif
{
  int i;	/* local counting variable */
  char label[128];
  int ndim, esize,ierr,tempi;
  float tempr[1000];
  
  if(0!=sep_get_number_data_axes("in",&ndim))
    seperr("trouble obtaining number of data axes \n");
  
  if(ndim !=3 ) seperr("expecting a 3d velocity cube \n");
  
  if(0==hetch("esize","d",&esize)) esize=4;
  
  if(esize !=4) seperr("Unacceptable esize value \n");
  
  for(i=1; i<=ndim;i++){
    if(0!=sep_get_data_axis_par("in",&i,&n[i-1],&o[i-1],&d[i-1],label))
      seperr("trouble getting data axis %d from in \n", i);
  }
  
  if(0==getch("verb","l",verb)) *verb=0;
  if(0==getch("interp","l",interp)) *interp=0;
  if(0==getch("fast","l",fast)) *fast=0;
  
  if(auxin("shots")!=NULL){
    if(1!=auxpar("n1","d",&tempi,"shots")) 
      seperr("trouble getting n1 from shots");
    if(tempi!=3) seperr("shots must  have n1=3 \n");
    if(1!=auxpar("n2","d",nshots,"shots")) *nshots=1; 
    shots=(float*)alloc(3*sizeof(float)*(*nshots));
    if(12*(*nshots)!=sreed("shots",shots,12*(*nshots)))
      seperr("trouble reading in shots");
  }
  else{
    *nshots=getch("xshot","f",tempr);
    if(*nshots>1000)seperr("maximum number of shots on command line is 1000\n");
    if(*nshots!=0){
      shots=(float*)alloc(3*sizeof(float)*(*nshots));
      for(i=0; i < *nshots; i++) shots[i*3]=tempr[i];
      ierr=getch("yshot","f",tempr);
      if(ierr!=*nshots) seperr("xshots not equalt to yshots");
      for(i=0; i < *nshots; i++) shots[i*3+1]=tempr[i];
      ierr=getch("zshot","f",tempr);
      if(ierr!=*nshots) seperr("zshots not equalt to yshots");
      for(i=0; i < *nshots; i++) shots[i*3+2]=tempr[i];
    }
    else{
      *nshots=1;
      shots=(float*)alloc(3*sizeof(float));
      shots[0]=o[1]+ (n[1]-1)*d[1]/2.;
      shots[1]=o[2]+ (n[2]-1)*d[2]/2.;
      shots[2]=o[0]+ (n[0]-1)*d[0]/2.;
    }
  }
  if(0==getch("aimi","f",aimi)) *aimi=0.;
  if(0==getch("aima","f",aima)) *aima=0.;
  
  if(0==getch("oi","f",oi))  *oi=0.;
  if(0==getch("di","f",di))  *di=1.;
  if(0==getch("ni","d",ni))  *ni=91;
  
  if(0==getch("da","f",da)) *da=1.;
  if(0==getch("dt","f",dt)) *dt=.004;
  if(0==getch("nt","d",nt)) *nt=1000;
  *na=360/(*da);
  
  return;
}

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
void put_pars(int interp,int fast, int *n, float *o, float *d,int nshots,float aimi, float aima, float oi, int ni, float di, float da, int na, float dt, int nt)
     _XFUNCPROTOEND
#else
void put_pars(interp,fast,n,o,d,nshots,aimi,aima,oi,ni,di,da,na,dt,nt)
     int nshots,ni,nt,na,*n,interp,fast;
     float aimi, aima,oi,di,da,dt,*o,*d;
#endif
{
  float zero,one;
  int three,axis;
  
  
  three=3; zero=0.;one=1.;
  if(interp==0){
    axis=1;sep_put_data_axis_par("out",&axis,&three,&one,&one,"x y z");
    axis=2;sep_put_data_axis_par("out",&axis,&nt,&zero,&dt,"Time");
    axis=3;sep_put_data_axis_par("out",&axis,&na,&zero,&da,"Azimuth");
    axis=4;sep_put_data_axis_par("out",&axis,&ni,&oi,&di,"Inclination");
    axis=5;sep_put_data_axis_par("out",&axis,&nshots,&one,&one,"Shot number");
  }
  else{
    putch("fast","d",&fast);
    axis=1;sep_put_data_axis_par("out",&axis,&n[0],&o[0],&d[0],"Depth");
    axis=2;sep_put_data_axis_par("out",&axis,&n[1],&o[1],&d[1],"X coordinate");
    axis=3;sep_put_data_axis_par("out",&axis,&n[2],&o[2],&d[2],"Y coordinate");
    axis=4;sep_put_data_axis_par("out",&axis,&nshots,&one,&one,"Shot number"); 
  }
  hclose();
  
}

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
void rite_ray_out(int nt,int na,int ni,float *rays_out,rays pRR)
     _XFUNCPROTOEND
#else
void rite_ray_out(nt,na,ni,rays_out,pRR)
     int nt,na,ni;
     float *rays_out;
     rays pRR;
#endif
{
  int i,j,ii,ia,it;
  
  j=0;
  for(ii=0; ii < ni; ii++){
    i=0;
    for(ia=0; ia < na; ia++){
      for(it=0; it < nt; it++){
	rays_out[i++]=pRR.c0[ii][ia][it].x;
	rays_out[i++]=pRR.c0[ii][ia][it].y;
	rays_out[i++]=pRR.c0[ii][ia][it].z;
	j++;
      }
    }
    if(12*nt*na!=srite("out",rays_out,12*nt*na))
      seperr("trouble writing out rays_out \n");
  }
  
  return;
}
