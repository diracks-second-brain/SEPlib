/*$

=head1 NAME
									
Fdmod - Finite-Difference MODeling (2nd order) for acoustic wave equation

=head1 USAGE
									
Fdmod intag=vfile outtag=wfile nx= nz= tmax= xs= zs= [optional parameters]	
									
=head1 PARAMETERS

=over 4

intag  - sepfile

  		file containing velocity[nx][nz]		

outtag - sepfile

		file containing waves[nx][nz] for time steps	

no_stdout - int

	  [0] Don't write  out wavefield

tmax  - float

			maximum time					


xs  - float*

			x coordinates of sources				

nxs, oxs, dxs - int, float, float

    Source X axis

nzs, ozs, dzs - int, float, float

    Source Z axis

zs	- float*	

    z coordinates of source				

nt  - int 
 
 [1+tmax/dt]  number of time samples (dt determined for stability)

mint  - float

 [0.]		minimum time for hsfile and vsfile

jt  - int 

 [1]		number of time steps (dt) per output time step (hsfile, vsfile)
									
mt  - int 

 [1]		number of time steps (dt) per output time step	
									
fmax  -float

[vmin/(10.0*h)]  maximum frequency in source wavelet		

fpeak  -float

[0.5*fmax]	peak frequency in ricker wavelet		
									
dfile- sepfile		

  input file containing density[nx][nz]		


vsx - float			

  [o2] x coordinate of vertical line of seismograms	

hsz - float			

   [o1] z coordinate of horizontal line of seismograms	

bx - int

   [(xs[0]-o2)/d2] distance from edge] Samples along x axis left of source

ex - int

   [(o2+d2*(n2-1)-xs[0])/d2] distance from edge] Samples along x axis right of source

bz - int

   [(zs[0]-o1)/d1] distance from edge] Samples along z axis top of source

ez - int

   [(o1+d1*(n1-1)-zs[0])/d1] distance from edge] Samples along z axis right of source

vsfile -sepfile	

  [none] 	output file for vertical line of seismograms[nz][nt]

hsfile -sepfile	

  [none] 	output file for horizontal line of seismograms[nx][nt]

verb -int

  [0]		=1 for diagnostic messages, =2 for more		
									
print_master -float

  [2]		=delta pct print for the master thread
									
print_slave -float

  [10]		=delta pct print for the slave thread(s)
									
abs - int[4]

  [1,1,1,1] 		Absorbing boundary conditions on top,left,bottom,right

			sides of the model. 0,1,1,1 for free surface condition on the top		
                                                                      
pml_max - float

  [1000.0]       PML absorption parameter                        

pml_thick - int

 [0]           half-thickness of pml layer (0 = do not use PML)

=head1 NOTES


1. This is sufdmod2 for SEPlib 


									
Notes:								
This program uses the traditional explicit second order differencing	
method. 								
									
Two different absorbing boundary condition schemes are available. The 
first is a traditional absorbing boundary condition scheme created by 
Hale, 1990. The second is based on the perfectly matched layer (PML)	
method of Berenger, 1995.						

=head1 CATEGORY

B<seis/model>

=cut

*/

/*
 * Authors:  CWP:Dave Hale
 *           CWP:modified for SU by John Stockwell, 1993.
 *           CWP:added frequency specification of wavelet: Craig Artley, 1993
 *           TAMU:added PML absorbing boundary condition:
 *                  Michael Holzrichter, 1998
 *
 * References: (Hale's absobing boundary conditions)
 * Clayton, R. W., and Engquist, B., 1977, Absorbing boundary conditions
 * for acoustic and elastic wave equations, Bull. Seism. Soc. Am., 6,
 *	1529-1540. 
 *
 * Clayton, R. W., and Engquist, B., 1980, Absorbing boundary conditions
 * for wave equation migration, Geophysics, 45, 895-904.
 *
 * Hale, D.,  1990, Adaptive absorbing boundaries for finite-difference
 * modeling of the wave equation migration, unpublished report from the
 * Center for Wave Phenomena, Colorado School of Mines.
 *
 * Richtmyer, R. D., and Morton, K. W., 1967, Difference methods for
 * initial-value problems, John Wiley & Sons, Inc, New York.
 *
 * Thomee, V., 1962, A stable difference scheme for the mixed boundary problem
 * for a hyperbolic, first-order system in two dimensions, J. Soc. Indust.
 * Appl. Math., 10, 229-245.
 *
 * Toldi, J. L., and Hale, D., 1982, Data-dependent absorbing side boundaries,
 * Stanford Exploration Project Report SEP-30, 111-121.
 *
 * References: (PML boundary conditions)
 * Jean-Pierre Berenger, ``A Perfectly Matched Layer for the Absorption of
 * Electromagnetic Waves,''  Journal of Computational Physics, vol. 114,
 * pp. 185-200.
 *
 * Hastings, Schneider, and Broschat, ``Application of the perfectly
 * matched layer (PML) absorbing boundary condition to elastic wave
 * propogation,''  Journal of the Accoustical Society of America,
 * November, 1996.
 *
 * Allen Taflove, ``Electromagnetic Modeling:  Finite Difference Time
 * Domain Methods'', Baltimore, Maryland: Johns Hopkins University Press,
 * 1995, chap. 7, pp. 181-195.
 *
 *
 * Trace header fields set: ns, delrt, tracl, tracr, offset, d1, d2,
 *                          sdepth, trid
 */
/**************** end self doc ********************************/

#define	ABS0	1
#define	ABS1	1
#define	ABS2	1
#define	ABS3	1
#define HEAD "/dev/null"



struct _print_stat{
int i;
int jprint; /*how often to print*/
int fprint;
int nprint;
};
typedef struct _print_stat print_stat;

struct _my_wind{
int j[6],f[6],n[6];
};
typedef struct _my_wind my_wind;

struct _my_pars{
float *xs,*zs;
int abs[4];
float tmax;
int nt,mt,jt,ft;
int bx,ex,ez,bz;
float hsz,vsx;
float pml_max;
float fmax,fpeak;
int pml_thick;
int nxs,nzs;
int master_data;
int nsect;
int restart;
float oxs,ozs,dxs,dzs;
};
typedef struct _my_pars my_pars;



#include<sulib.h>
#include<sep3dc.h>


/* Prototypes for PML absorbing boundary conditions */
static void pml_alloc (int nx, int nz, float dx, float dz, float dt, float **dvv);
static void pml_init (int nx, int nz, float dx, float dz, float dt, float **dvv);
static void pml_absorb (int nx, float dx, int nz, float dz, float dt,
        float **dvv, float **od, float **pm, float **p, float **pp,
        int *abs);
void do_shot(sep3d *space, print_stat *pstat, my_pars *par,float *loc, float dt, float tdelay, float **s, float **dvv, float **od, float **pm, float **p, float **pp,  int no_stdout,my_wind *out_wind, sep3d *output,int hs1, float **hs,int vs2, float **vs );
void put_params(char *tag, my_pars *par);

int file_setup(sep3d *input, my_pars *par, float dt, int *hsfile_u, sep3d *hsfile_s,
 int *no_stdout, sep3d *output, int *vsfile_u, sep3d *vsfile_s);

/* PML related global variables */
int pml_thickness;
float pml_max;

float **cax_b, **cax_r;
float **cbx_b, **cbx_r;
float **caz_b, **caz_r;
float **cbz_b, **cbz_r;
float **dax_b, **dax_r;
float **dbx_b, **dbx_r;
float **daz_b, **daz_r;
float **dbz_b, **dbz_r;

float **ux_b,  **ux_r;
float **uz_b,  **uz_r;
float **v_b,   **v_r;
float **w_b,   **w_r;

float dvv_0, dvv_1, dvv_2, dvv_3;
float sigma, sigma_ex, sigma_ez, sigma_mx, sigma_mz;


/* Prototypes for finite differencing */
void ptsrc (float xs, float zs,
	int nx, float dx, float fx,
	int nz, float dz, float fz,
	float dt, float t, float fmax, float fpeak, float tdelay, float **s);
void exsrc (int ns, float *xs, float *zs,
	int nx, float dx, float fx,
	int nz, float dz, float fz,
	float dt, float t, float fmax, float **s);
void tstep2 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp, int *abs);
void extract_close(my_pars *par,sep3d *input, sep3d *space, float *loc, float *loca_out,float **dvv,
  float **d_vv,  float **od, float **o_d);



#include<seplib.h>

int main(int argc, char **argv)
{
	int it,is;	/* counters */
	int nx,nz;	/* x,z,t,tsizes */

	int verbose;		/* is verbose? */
	int nxs;		/* number of source x coordinates */
	int nzs;		/* number of source y coordinates */
	int ns;			/* total number of sources ns=nxs=nxz */

	int vs2;		/* depth in samples of horiz rec line */
	int hs1;		/* horiz sample of vert rec line */

	float fx;		/* first x value */
	float dx;		/* x sample interval */

	float fz;		/* first z value */
	float dz;		/* z sample interval */
	float h;		/* minumum spatial sample interval */


	float dt;		/* time sample interval */
	float tdelay=0.;	/* time delay of source beginning */

	float vmin;		/* minimum wavespeed in vfile */
	float vmax;		/* maximum wavespeed in vfile */

	float dmin;		/* minimum density in dfile */
	float dmax;		/* maximum density in dfile */

	float t;		/* time */
	float **s;		/* array of source pressure values */
	float **dvv;		/* array of velocity values from vfile */
	float **od;		/* array of density values from dfile */
	float **d_vv;		/* array of velocity values from vfile */
	float **o_d;		/* array of density values from dfile */

  float *junk;

	/* pressure field arrays */
	float **pm;		/* pressure field at t-dt */
	float **p;		/* pressure field at t */
	float **pp;		/* pressure field at t+dt */
	float **ptemp;		/* temp pressure array */

	/* output data arrays */
	float **ss;		/* source point seismogram array */
	float **hs;		/* seismograms from horiz receiver line */
	float **vs;		/* seismograms from vert receiver line */

	/* file names */
	int dfile_u;		/* use density file name */
	int vsfile_u;	/* use vert receiver seismogram line file  name */
	int hsfile_u;	/*  use horiz receiver seismogram line file name */

	/* SEGY fields */
	long tracl=0;		/* trace number within a line */
	long tracr=0;		/* trace number within a reel */

	/* Absorbing boundary conditions related stuff*/
	int nabs;		/* number of values given */
   int i,j,ifirst=1;
   float oxs,dxs,ozs,dzs;
   float fj1,fj2;
   char temp_ch[1024];
   float zero=0;

   /*sep3d structures*/
   sep3d input,hsfile_s,vsfile_s,output,space,density,*valid_s;
   int no_stdout;
   my_wind owind,hwind,vwind;
   print_stat pstat;
   float loc[2],loc_out[2],mint;
   int verb,*ndone;
   float print_master,print_slave;
   my_pars par;
   int ix,iz,i1,i2,i3,i4,i5,i6;
   int n_x,n_z,ith,nth,isect;


  initpar(argc,argv);
  sep_begin_prog(); 
/*  init_3d();*/
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Fdmod - Finite-Difference MODeling (2nd order) for acoustic wave");\
 sep_add_doc_line("    equation");\
 sep_add_doc_line("");\
 sep_add_doc_line("USAGE");\
 sep_add_doc_line("    Fdmod intag=vfile outtag=wfile nx= nz= tmax= xs= zs= [optional");\
 sep_add_doc_line("    parameters]");\
 sep_add_doc_line("");\
 sep_add_doc_line("PARAMETERS");\
 sep_add_doc_line("        intag - sepfile");\
 sep_add_doc_line("");\
 sep_add_doc_line("                        file containing velocity[nx][nz]");\
 sep_add_doc_line("");\
 sep_add_doc_line("        outtag - sepfile");\
 sep_add_doc_line("");\
 sep_add_doc_line("                        file containing waves[nx][nz] for time steps");\
 sep_add_doc_line("");\
 sep_add_doc_line("        no_stdout - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("                  [0] Don't write  out wavefield");\
 sep_add_doc_line("");\
 sep_add_doc_line("        tmax - float");\
 sep_add_doc_line("");\
 sep_add_doc_line("                                maximum time");\
 sep_add_doc_line("");\
 sep_add_doc_line("        xs - float*");\
 sep_add_doc_line("");\
 sep_add_doc_line("                                x coordinates of sources");\
 sep_add_doc_line("");\
 sep_add_doc_line("        nxs, oxs, dxs - int, float, float");\
 sep_add_doc_line("");\
 sep_add_doc_line("            Source X axis");\
 sep_add_doc_line("");\
 sep_add_doc_line("        nzs, ozs, dzs - int, float, float");\
 sep_add_doc_line("");\
 sep_add_doc_line("            Source Z axis");\
 sep_add_doc_line("");\
 sep_add_doc_line("        zs - float*");\
 sep_add_doc_line("");\
 sep_add_doc_line("            z coordinates of source");\
 sep_add_doc_line("");\
 sep_add_doc_line("        nt - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("         [1+tmax/dt]  number of time samples (dt determined for stability)");\
 sep_add_doc_line("");\
 sep_add_doc_line("        mint - float");\
 sep_add_doc_line("");\
 sep_add_doc_line("         [0.]           minimum time for hsfile and vsfile");\
 sep_add_doc_line("");\
 sep_add_doc_line("        jt - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("         [1]            number of time steps (dt) per output time step (hsfile, vsfile)");\
 sep_add_doc_line("");\
 sep_add_doc_line("        mt - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("         [1]            number of time steps (dt) per output time step");\
 sep_add_doc_line("");\
 sep_add_doc_line("        fmax -float");\
 sep_add_doc_line("");\
 sep_add_doc_line("        [vmin/(10.0*h)] maximum frequency in source wavelet");\
 sep_add_doc_line("");\
 sep_add_doc_line("        fpeak -float");\
 sep_add_doc_line("");\
 sep_add_doc_line("        [0.5*fmax] peak frequency in ricker wavelet");\
 sep_add_doc_line("");\
 sep_add_doc_line("        dfile- sepfile");\
 sep_add_doc_line("");\
 sep_add_doc_line("          input file containing density[nx][nz]");\
 sep_add_doc_line("");\
 sep_add_doc_line("        vsx - float");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [o2] x coordinate of vertical line of seismograms");\
 sep_add_doc_line("");\
 sep_add_doc_line("        hsz - float");\
 sep_add_doc_line("");\
 sep_add_doc_line("           [o1] z coordinate of horizontal line of seismograms");\
 sep_add_doc_line("");\
 sep_add_doc_line("        bx - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("           [(xs[0]-o2)/d2] distance from edge] Samples along x axis left of source");\
 sep_add_doc_line("");\
 sep_add_doc_line("        ex - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("           [(o2+d2*(n2-1)-xs[0])/d2] distance from edge] Samples along x axis right of source");\
 sep_add_doc_line("");\
 sep_add_doc_line("        bz - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("           [(zs[0]-o1)/d1] distance from edge] Samples along z axis top of source");\
 sep_add_doc_line("");\
 sep_add_doc_line("        ez - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("           [(o1+d1*(n1-1)-zs[0])/d1] distance from edge] Samples along z axis right of source");\
 sep_add_doc_line("");\
 sep_add_doc_line("        vsfile -sepfile");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [none]        output file for vertical line of seismograms[nz][nt]");\
 sep_add_doc_line("");\
 sep_add_doc_line("        hsfile -sepfile");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [none]        output file for horizontal line of seismograms[nx][nt]");\
 sep_add_doc_line("");\
 sep_add_doc_line("        verb -int");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [0]           =1 for diagnostic messages, =2 for more");\
 sep_add_doc_line("");\
 sep_add_doc_line("        print_master -float");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [2]           =delta pct print for the master thread");\
 sep_add_doc_line("");\
 sep_add_doc_line("        print_slave -float");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [10]          =delta pct print for the slave thread(s)");\
 sep_add_doc_line("");\
 sep_add_doc_line("        abs - int[4]");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [1,1,1,1]             Absorbing boundary conditions on top,left,bottom,right");\
 sep_add_doc_line("");\
 sep_add_doc_line("                                sides of the model. 0,1,1,1 for free surface condition on the top");\
 sep_add_doc_line("");\
 sep_add_doc_line("        pml_max - float");\
 sep_add_doc_line("");\
 sep_add_doc_line("          [1000.0]       PML absorption parameter");\
 sep_add_doc_line("");\
 sep_add_doc_line("        pml_thick - int");\
 sep_add_doc_line("");\
 sep_add_doc_line("         [0]           half-thickness of pml layer (0 = do not use PML)");\
 sep_add_doc_line("");\
 sep_add_doc_line("NOTES");\
 sep_add_doc_line("    1. This is sufdmod2 for SEPlib");\
 sep_add_doc_line("");\
 sep_add_doc_line("    Notes: This program uses the traditional explicit second order");\
 sep_add_doc_line("    differencing method.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    Two different absorbing boundary condition schemes are available. The");\
 sep_add_doc_line("    first is a traditional absorbing boundary condition scheme created by");\
 sep_add_doc_line("    Hale, 1990. The second is based on the perfectly matched layer (PML)");\
 sep_add_doc_line("    method of Berenger, 1995.");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/model");\
 sep_add_doc_line("");\
 sep_add_doc_line("POD ERRORS");\
 sep_add_doc_line("    Hey! The above document had some coding errors, which are explained");\
 sep_add_doc_line("    below:");\
 sep_add_doc_line("");\
 sep_add_doc_line("    Around line 136:");\
 sep_add_doc_line("        You forgot a '=back' before '=head1'");\
 sep_add_doc_line("");
  MY_SEP_DOC
  doc(SOURCE);



   if(0!=init_sep3d_tag("intag",&input,"INPUT"))
     seperr("trouble initializing input\n");


   if(sep3dc_ndims(&input)!=2) seperr("expecting input to be two dimensional \n");
   nx=input.n[1]; fx=input.o[1]; dx=input.d[1];
   nz=input.n[0]; fz=input.o[0]; dz=input.d[0];
   par.nsect=1;


	/* get required parameters */
	/* get dimensions of model, maximum duration */
	if (0==getch("tmax","f",&par.tmax)) seperr("must specify tmax!");
	if (0==getch("restart","d",&par.restart)) par.restart=0;



	  junk = alloc1float(1000);
    nxs=getch("xs","f",junk);
    nzs=getch("zs","f",junk);
	  if (nxs!=nzs)
		  seperr("number of xs = %d must equal number of zs = %d",
			nxs,nzs);
    free1(junk); 
    ns=nxs;
    if(ns!=0){
	    par.xs = alloc1float(ns);
	    par.zs = alloc1float(ns);
      nxs=getch("xs","f",par.xs);
      nzs=getch("zs","f",par.zs);
      oxs=dxs=1.; ozs=dzs=1.;nxs=ns; nzs=1;
      par.nxs=ns; par.dxs=1; par.oxs=par.xs[0];
      par.nzs=1; par.ozs=par.zs[0]; par.dzs=1;
    }
    else{
      par.nxs=par.nzs=1;
      par.oxs=input.o[1];
      par.dxs=input.d[1];
      par.ozs=input.o[0];
      par.dzs=input.d[0];
      i1=getch("nxs","d",&par.nxs);
      i2=getch("oxs","f",&par.oxs);
      i3=getch("dxs","f",&par.dxs);
      i4=getch("nzs","d",&par.nzs);
      i5=getch("ozs","f",&par.ozs);
      i6=getch("dzs","f",&par.dzs);
      if(i1==0 && i2==0 && i3==0 && i4==0 && i5==0 && i6==0){
        par.oxs=input.o[1]+input.n[1]*input.d[1]/2.;
        par.ozs=input.o[0]+input.n[0]*input.d[0]/2.;
      } 
      ns=par.nxs*par.nzs;
	    par.xs = alloc1float(ns);
	    par.zs = alloc1float(ns);
      for(i3=0,i1=0; i3 < par.nzs; i3++){
        for(i2=0; i2 < par.nxs; i2++,i1++){
          par.xs[i1]=par.oxs+par.dxs*i2;
          par.zs[i1]=par.ozs+par.dzs*i3;
          if(par.xs[i1]< input.o[1] || par.xs[i1] >= input.o[1]+input.d[1]*(input.n[1]-1)) seperr("invalid source location x[%d]=%f range(%f-%f  \n",
   i1,par.xs[i1], input.o[1] ,  input.o[1]+input.d[1]*(input.n[1]-1));
          if(par.zs[i1]< input.o[0] || par.zs[i1] >= input.o[0]+input.d[0]*(input.n[0]-1)) seperr("invalid source location x[%d]=%f range(%f-%f  \n",
   i1,par.xs[i1], input.o[0] ,  input.o[0]+input.d[0]*(input.n[0]-1));
        } 
      }
    }

   par.bx=(par.xs[0]-input.o[1])/input.d[1]+.001;
   par.ex=(input.o[1]+input.d[1]*(input.n[1]-1)-par.xs[0])/input.d[1]+.001;
   par.bz=(par.zs[0]-input.o[0])/input.d[0]+.001;
   par.ez=(input.o[0]+input.d[0]*(input.n[0]-1)-par.zs[0])/input.d[0]+.00001;
   getch("bz","d",&par.bz); getch("ez","d",&par.ez);
   getch("bx","d",&par.bx); getch("ex","d",&par.ex);

  n_x=par.bx+par.ex+1;
  n_z=par.bz+par.ez+1;

	/* Get absorbing boundary information */
  par.abs[0] = ABS0; par.abs[1] = ABS1; par.abs[2] = ABS2; par.abs[3] = ABS3;
	nabs = getch("abs","d",par.abs);
	
	/* get optional parameters */
	if (0==getch("nt","d",&par.nt)) par.nt = 0;
	if (0==getch("mt","d",&par.mt)) par.mt = 1;



        if (0==getch("pml_max","f",&par.pml_max)) par.pml_max = 1000.0;
    pml_max=par.pml_max;
        if (0==getch("pml_thick","d",&par.pml_thick)) par.pml_thick = 0;
        pml_thickness = 2 * par.pml_thick;



	/* z-coorinate of horizontal line of detectors */
	if (0==getch("hsz","f",&par.hsz)) par.hsz =fz; 
	hs1 = NINT( (par.hsz - fz)/dz );

	/* x-coordinate of vertical line of detectors */
	if (0==getch("vsx","f",&par.vsx)) par.vsx = fx;
	vs2 = NINT((par.vsx - fx)/dx );
	

	/* allocate space */
	dvv = alloc2float(nz,nx);
	od = alloc2float(nz,nx);

	

	/* read velocities */
  if(0!=sep3dc_read_data("intag",&input,(char*)dvv[0],nz,0,1))
    seperr("trouble reading data \n");
	
	/* determine minimum and maximum velocities */
	vmin = vmax = dvv[0][0];
	for (ix=0; ix<nx; ++ix) {
		for (iz=0; iz<nz; ++iz) {
			vmin = MIN(vmin,dvv[ix][iz]);
			vmax = MAX(vmax,dvv[ix][iz]);
		}
	}
	
	/* determine mininum spatial sampling interval */
	h = MIN(ABS(dx),ABS(dz));
	
	/* determine time sampling interval to ensure stability */
	dt = h/(2.0*vmax);

	
	/* determine maximum temporal frequency to avoid dispersion */
	if (0==getch("fmax","f", &par.fmax))	par.fmax = vmin/(10.0*h);

	/* compute or set peak frequency for ricker wavelet */
	if (0==getch("fpeak","f", &par.fpeak))	par.fpeak = 0.5*par.fmax;
fprintf(stderr,"CHECK params %f %f \n",par.fmax,par.fpeak);;

	/* determine number of time steps required to reach maximum time */
	if (par.nt==0) par.nt = 1+par.tmax/dt;


   

if(0!=file_setup(&input,&par,dt, &hsfile_u,&hsfile_s,&no_stdout,&output,&vsfile_u,&vsfile_s))
  seperr("trouble setting up output files \n");


  valid_s=NULL;
	if (hsfile_u==1){
    hwind.n[0]=par.nt; hwind.f[0]=0; hwind.j[0]=1;
    hwind.n[1]=n_x; hwind.f[1]=0; hwind.j[1]=1;
    valid_s=&hsfile_s;
  }
  else hs=NULL;

  if(no_stdout==0){
     owind.n[2]=1; owind.f[2]=0; owind.j[2]=1;
     owind.n[1]=nx; owind.f[1]=0; owind.j[1]=1;
     owind.n[0]=nz; owind.f[0]=0; owind.j[0]=1;
     valid_s=&   output;
   }

	if (vsfile_u==1) {
     vwind.n[0]=par.nt; vwind.f[0]=0; vwind.j[0]=1;
     vwind.n[1]=n_z; vwind.f[1]=0; vwind.j[1]=1;
    valid_s=&vsfile_s;
	} 
  else vs = NULL;
  hwind.n[3]=hwind.n[2]=1;
  hwind.j[3]=hwind.j[2]=1;
  hwind.f[3]=hwind.f[2]=0;

  vwind.n[3]=vwind.n[2]=1;
  vwind.j[3]=vwind.j[2]=1;
  vwind.f[3]=vwind.f[2]=0;

  owind.n[4]=owind.n[3]=1;
  owind.f[4]=owind.f[3]=0;
  owind.j[4]=owind.j[3]=1;

  if(0!=init_sep3d_par(&space,"OUTPUT","FLOAT","REGULAR",2,0))
         seperr("troulbe initializing space \n");

  space.n[0]=n_z; space.o[0]=fz; space.d[0]=dz; 
  space.n[1]=n_x; space.o[1]=fx; space.d[1]=dx; 

	/* ... vertical line of seismograms */
		ss = NULL;
	
	/* if specified, read densities */
	if (1==getch("dfile","s",temp_ch)) {
     if(0!=init_sep3d_tag("dfile",&density,"INPUT"))
        seperr("trouble initializing input\n");
    dfile_u=1;
    if(0!=sep3dc_conform(&density,&input))
      seperr("density and velocity don't conform\n");
    
    if(0!=sep3dc_read_data("dfile",&density,(char*)od[0],nz,0,1))
       seperr("trouble reading density \n");
	
		dmin = dmax = od[0][0];
		for (ix=0; ix<nx; ++ix) {
			for (iz=0; iz<nz; ++iz) {
				dmin = MIN(dmin,od[ix][iz]);
				dmax = MAX(dmax,od[ix][iz]);
			}
		}
    if(verb>0 && sep_thread_num()==0) fprintf(stderr,"Density supplied min=%f max=%f \n",
       dmin,dmax);
	}
  else dfile_u=0;
	
	/* if densities not specified or constant, make densities = 1 */
	if (dfile_u==0 || dmin==dmax ) {
		for (ix=0; ix<nx; ++ix)
			for (iz=0; iz<nz; ++iz)
				od[ix][iz] = 1.0;
		dmin = dmax = 1.0;
	}
	

	pm = alloc2float(n_z,n_x);
	s = alloc2float(n_z,n_x);
	p = alloc2float(n_z,n_x);
	pp = alloc2float(n_z,n_x);
	d_vv = alloc2float(n_z,n_x);
  if(od!=NULL) o_d = alloc2float(n_z,n_x);
  else o_d=NULL;

	/* compute density*velocity^2 and 1/density and zero time slices */	
	for (ix=0; ix<nx; ++ix) {
		for (iz=0; iz<nz; ++iz) {
			dvv[ix][iz] = od[ix][iz]*dvv[ix][iz]*dvv[ix][iz];
			od[ix][iz] = 1.0/od[ix][iz];
		}
	}
	
	/* if densities constant, free space and set NULL pointer */
	if (dmin==dmax) {
		free2float(od);
		od = NULL;
		o_d = NULL;
	}
	

   if (pml_thickness > 0) pml_alloc (n_x, n_z, dx, dz, dt, dvv);
   if (pml_thickness > 0) pml_init (n_x, n_z, dx, dz, dt, dvv);
      ifirst=0;
      tdelay=1./par.fpeak;
   if (0==getch("mint","f",&mint)) mint = 0.;
   if (0==getch("jt","d",&par.jt)) par.jt = 1;
   par.ft=(mint+tdelay/dt);

      if(no_stdout==0 && par.restart!=1){
        if(sep_thread_num()==0) copy_history("intag","outtag");
         output.o[2]=-tdelay;
         put_params("outtag",&par);
         sep3d_set_sep3d(&output);
        if(0!=  sep3dc_write_description("outtag",&output))
       seperr("trouble writing out description\n");
      }
      if(hsfile_u==1 && par.restart!=1){
         hsfile_s.n[0]=ceil(((float)(hsfile_s.n[0]-par.ft))/
          ((float)(par.jt)));
         hsfile_s.o[0]=-tdelay+par.ft*dt;
         hsfile_s.d[0]=dt*par.jt;
         hwind.n[0]=hsfile_s.n[0];
         if(sep_thread_num()==0)copy_history("intag","hsfile");
         put_params("hsfile",&par);
         sep3d_set_sep3d(&hsfile_s);
       if(0!=  sep3dc_write_description("hsfile",&hsfile_s))
         seperr("trouble writing out description\n");
    hs = alloc2float(hwind.n[0],n_x);
      }
      if(vsfile_u==1 && par.restart!=1){
         vsfile_s.n[0]=ceil(((float)(vsfile_s.n[0]-par.ft))/
          ((float)(par.jt)));
         vsfile_s.d[0]=par.jt*dt;
         vsfile_s.o[0]=-tdelay+par.ft*dt;
         vwind.n[0]=vsfile_s.n[0];
         if(sep_thread_num()==0)copy_history("intag","vsfile");
         put_params("vsfile",&par);
         sep3d_set_sep3d(&vsfile_s);
         if(0!=  sep3dc_write_description("vsfile",&vsfile_s))
          seperr("trouble writing out description\n");
		vs = alloc2float(vwind.n[0],nz);
       }


  if(getch("verb","d",&verb)==0) verb=1;
  if(verb==1){
     pstat.i=0;
     if(sep_thread_num()==0){
       if(getch("print_master","f",&print_master)==0) print_master=2.;
       pstat.jprint=(float)print_master/100.*par.nt;
     }
     else{
       if(getch("print_slave","f",&print_slave)==0) print_slave=10.;
       pstat.jprint=(float)print_slave/100.*par.nt;
     }
     pstat.fprint=0;
     pstat.nprint=par.nt;
   }
   else{
    pstat.fprint=(par.nt+1);
    pstat.jprint=(par.nt+1);
    pstat.nprint=(par.nt);
   }


isect=0;
if(vsfile_u ==1 || hsfile_u==1){
  for(i=0; i < valid_s->n[2]; i++){
    isect++;
  } 
}
else{
  for(i=0; i < valid_s->n[3]; i++){
    isect++;
  } 
}
pstat.jprint=pstat.jprint*isect;
pstat.nprint=pstat.nprint*isect;

ndone=(int*) malloc(sizeof(int)*par.nsect);
for(i=0; i < par.nsect; i++) ndone[i]=-1;
 for(i=0,iz=0; iz < par.nzs; iz++){
   for(ix=0; ix < par.nxs; ix++,i++){
                                                                                                

       owind.f[2]=0;
       loc[0]=par.zs[i]; loc[1]=par.xs[i];
       for(i2=0; i2 < n_x; i2++){
         for(i1=0; i1 < n_z; i1++){
           s[i2][i1]=0.;
           pm[i2][i1]=0.;
           p[i2][i1]=pp[i2][i1]=0.;
         }
       }
      extract_close(&par,&input,&space,loc,loc_out,dvv,d_vv,od,o_d);


      do_shot(&space,&pstat,&par,loc,dt,tdelay,s,d_vv,o_d,pm,p,pp, 
            no_stdout,&owind, &output,hs1, hs,vs2, vs );
      sep_prog_stat("Fdmod",i,par.nxs*par.nzs,1);


       	/* if requested, write horizontal line of seismograms */
       	if (hs!=NULL) {
          if(0!=sep3dc_write_data("hsfile",&hsfile_s,(char*)hs[0],hwind.n,hwind.f,hwind.j,
            nx,0,0)) seperr("trouble writing out hsfile \n");
	      }

       	/* if requested, write vertical line of seismograms */
	      if (vs!=NULL) {
          if(0!=sep3dc_write_data("vsfile",&vsfile_s,(char*)vs[0],vwind.n,vwind.f,vwind.j,
            nz,0,0)) seperr("trouble writing out vsfile \n");
	      }



   hwind.f[2]++;
   vwind.f[2]++;
   owind.f[3]++;
 }
 hwind.f[2]=0;
 vwind.f[2]=0;
 owind.f[2]=0;
 hwind.f[3]++;
 vwind.f[3]++;
 owind.f[4]++;
}

	/* free space before returning */
	free2float(s);
	free2float(dvv);
	free2float(pm);
	free2float(p);
	free2float(pp);
	
	if (od!=NULL) free2float(od);
	if (hs!=NULL) free2float(hs);
	if (vs!=NULL) free2float(vs);
	if (ss!=NULL) free2float(ss);
	

      sep_end_prog();
	return EXIT_SUCCESS;
}

void extract_close(my_pars *par,sep3d *input, sep3d *space, float *loc, float *loc_out,float **dvv,
  float **d_vv,  float **od, float **o_d)
{
int ix,iz,i1,i2,i_x,i_z,ib,ia;

 iz=(loc[0]-input->o[0])/input->d[0];
 ix=(loc[1]-input->o[1])/input->d[1];
 space->o[0]=loc[0]-par->bz*input->d[0];
 space->o[1]=loc[1]-par->bx*input->d[1];

 for(i2=ix-par->bx,ib=0; i2 <= ix+par->ex; i2++,ib++){
   if(i2<0) i_x=0;
   else if(i2>=input->n[1]) i_x=input->n[1]-1;
   else i_x=i2;
   for(i1=iz-par->bz,ia=0; i1 <= iz+par->ez; i1++,ia++){
     if(i1<0) i_x=0;
     else if(i1>=input->n[0]) i_x=input->n[0]-1;
     else i_z=i1;
     d_vv[ib][ia]=dvv[i_x][i_z];
     if(od!=NULL) o_d[ib][ia]=od[i_x][i_z];
   }
 }

}

int file_setup(sep3d *input, my_pars *par, float dt, int *hsfile_u, sep3d *hsfile_s,
 int *no_stdout, sep3d *output, int *vsfile_u, sep3d *vsfile_s)
{
char hsfile[1024];
char vsfile[1024];
int j;
float fj1;

	*hsfile_u=getch("hsfile","s",hsfile);
	if (*hsfile_u==1) {
    if(par->restart!=1){
     if(0!=init_sep3d_par(hsfile_s,"OUTPUT","FLOAT","REGULAR",4,0))
       return(sepwarn(NOT_MET,"troulbe initializing hsfile \n"));
     hsfile_s->n[0]=par->nt; hsfile_s->o[0]=0; hsfile_s->d[0]=dt; 
/*     hsfile_s->n[1]=input->n[1]; hsfile_s->o[1]=input->o[1]; hsfile_s->d[1]=input->d[1]; */
     hsfile_s->n[1]=par->bx+par->ex+1; hsfile_s->o[1]=-par->bx*input->d[1]; 
     hsfile_s->d[1]=input->d[1]; 
     hsfile_s->n[2]=par->nxs; hsfile_s->o[2]=par->oxs; hsfile_s->d[2]=par->dxs; 
     hsfile_s->n[3]=par->nzs; hsfile_s->o[3]=par->ozs; hsfile_s->d[3]=par->dzs; 
     strcpy(hsfile_s->label[0],"time");
     strcpy(hsfile_s->label[1],"X offset");
     strcpy(hsfile_s->label[2],"Src_x");
     strcpy(hsfile_s->label[3],"Src_y");
    }
    else {
       if(0!=init_sep3d_tag("hsfile",hsfile_s,"SCRATCH"))
        seperr("trouble initializing tag for restarting tag=%s \n","hsfile_s");
  }

	}

    *no_stdout=1; getch("no_stdout","d",no_stdout);
    if(*no_stdout==0){
      if(par->restart!=1){
       if(0!=init_sep3d_par(output,"OUTPUT","FLOAT","REGULAR",5,0))
       return(sepwarn(NOT_MET,"troulbe initializing output \n"));
       output->n[0]=input->n[0]; output->o[0]=input->o[0]; output->d[0]=input->d[0]; 
       output->n[1]=input->n[1]; output->o[1]=input->o[1]; output->d[1]=input->d[1]; 

       output->n[0]=par->bz+par->ez+1; output->o[0]=-par->bz*input->d[0]; 
       output->d[0]=input->d[0]; 
       output->n[1]=par->bx+par->ex+1; output->o[1]=-par->bx*input->d[1]; 
       output->d[1]=input->d[1]; 

       strcpy(output->label[2],"time");
       strcpy(output->label[0],"Z Position");
       strcpy(output->label[1],"X Position");
       strcpy(output->label[3],"Src_x");
       strcpy(output->label[4],"Src_y");
       output->n[3]=par->nxs; output->o[3]=par->oxs; output->d[3]=par->dxs; 
       output->n[4]=par->nzs; output->o[4]=par->ozs; output->d[4]=par->dzs; 
       fj1=dt*par->mt;j=ceil(((double)par->nt)/((double)par->mt));
       output->n[2]=j; output->o[2]=0; output->d[2]=fj1; 
     }
    else if(0!=init_sep3d_tag("outtag",output,"SCRATCH"))
      seperr("trouble initializing tag for restarting tag=%s \n","outtag");
    }
	*vsfile_u=getch("vsfile","s",vsfile);
	if (*vsfile_u==1) {
    if(par->restart!=1){
     if(0!=init_sep3d_par(vsfile_s,"OUTPUT","FLOAT","REGULAR",4,0))
       return(sepwarn(NOT_MET,"troulbe initializing vsfile \n"));
     vsfile_s->n[0]=par->nt; vsfile_s->o[0]=0; vsfile_s->d[0]=dt; 
     hsfile_s->n[1]=par->bz+par->ez+1; hsfile_s->o[0]=-par->bz*input->d[0]; 
     hsfile_s->d[1]=input->d[0]; 
     vsfile_s->n[2]=par->nxs; vsfile_s->o[2]=par->oxs; vsfile_s->d[2]=par->dxs; 
     vsfile_s->n[3]=par->nzs; vsfile_s->o[3]=par->ozs; vsfile_s->d[3]=par->dzs; 
     strcpy(vsfile_s->label[0],"time");
     strcpy(vsfile_s->label[1],"Z Position");
     strcpy(vsfile_s->label[2],"Src_x");
     strcpy(vsfile_s->label[3],"Src_y");
   }
    else if(0!=init_sep3d_tag("vsfile",vsfile_s,"SCRATCH"))
      seperr("trouble initializing tag for restarting tag=%s \n","vsfile_s");
	}
  return(SUCCESS);
}


void put_params(char *tag, my_pars *par){
if(sep_thread_num()==0){

auxputch("tmax","f",&par->tmax,tag);
auxputch("abs1","d",&par->abs[0],tag);
auxputch("abs2","d",&par->abs[1],tag);
auxputch("abs3","d",&par->abs[2],tag);
auxputch("abs4","d",&par->abs[3],tag);
auxputch("nt","d",&par->nt,tag);
auxputch("mt","d",&par->mt,tag);
auxputch("hsx","f",&par->hsz,tag);
auxputch("vsx","f",&par->vsx,tag);
auxputch("pml_thick","d",&par->pml_thick,tag);
auxputch("pml_max","f",&par->pml_max,tag);
auxputch("fmax","f",&par->fmax,tag);
auxputch("fpeak","f",&par->fpeak,tag);
}

}
void do_shot(sep3d *space, print_stat *pstat, my_pars *par, float *loc, float dt, 
float tdelay, float **s, float **dvv, float **od, float **pm, float **p, float **pp, 
int no_stdout,my_wind *out_wind, sep3d *output, int hs1, float **hs, 
int vs2,float **vs )
{
int it,mm,ix,iz,iht=0,ivt=0,i1,i2;
float t,tot;
float **ptemp;


	/* loop  ver time steps */
	for (it=0,t=0.0; it<par->nt; ++it,t+=dt, pstat->i++) {
	
		/* if verbose, print time step */
    if(pstat->i > pstat->fprint){
       pstat->fprint+=pstat->jprint;
       mm=((double)(pstat->i*1000.))/((double)pstat->nprint);
       fprintf(stderr,"Thread=%d finished %g\%\n",
           sep_thread_num(),mm/10.);
    }
for(i2=0; i2 < space->n[1]; i2++){
for(i1=0; i1 < space->n[0]; i1++){
  tot+=pp[i2][i1];
}}
	
		/* update source function */
			ptsrc(loc[1],loc[0],space->n[1],space->d[1],space->o[1],
          space->n[0] ,space->d[0],space->o[0],dt,t,
			      par->fmax,par->fpeak,tdelay,s);
		/* do one time step */
		tstep2(space->n[1],space->d[1],space->n[0],space->d[0],dt,dvv,od,s,pm,p,pp,par->abs);
		
		/* write waves */
		if (it%par->mt==0) {

      if(no_stdout==0){
        if(0!=sep3dc_write_data("outtag",output,(char*)pp[0],out_wind->n,
         out_wind->f,out_wind->j,space->n[1],0,0))
         seperr("trouble writing out wavefield \n");
        out_wind->f[2]++;
      }
           
		}

		/* if requested, save horizontal line of seismograms */
		if (hs!=NULL && (it-par->ft)%par->jt ==0  && it >par->ft) {
			for (ix=0; ix<space->n[1]; ++ix)
				hs[ix][iht] = pp[ix][hs1];
                   iht+=1;
		}

		/* if requested, save vertical line of seismograms */
		if (vs!=NULL && (it-par->ft)%par->jt ==0  && it >par->ft) {
			for (iz=0; iz<space->n[0]; ++iz)
				vs[iz][ivt] = pp[vs2][iz];
                   ivt+=1;
		}

		/* roll time slice pointers */
		ptemp = pm;
		pm = p;
		p = pp;
		pp = ptemp;
	}



 return;
}





void exsrc (int ns, float *xs, float *zs,
	int nx, float dx, float fx,
	int nz, float dz, float fz,
	float dt, float t, float fmax, float **s)
/*****************************************************************************
exsrc - update source pressure function for an extended source
******************************************************************************
Input:
ns		number of x,z coordinates for extended source
xs		array[ns] of x coordinates of extended source
zs		array[ns] of z coordinates of extended source
nx		number of x samples
dx		x sampling interval
fx		first x sample
nz		number of z samples
dz		z sampling interval
fz		first z sample
dt		time step (ignored)
t		time at which to compute source function
fmax		maximum frequency

Output:
s		array[nx][nz] of source pressure at time t+dt
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 03/01/90
******************************************************************************/
{
	int ix,iz,ixv,izv,is;
	float sigma,tbias,ascale,tscale,ts,xn,zn,
		v,xv,zv,dxdv,dzdv,xvn,zvn,amp,dv,dist,distprev;
	static float *vs,(*xsd)[4],(*zsd)[4];
	static int made=0;
	
	/* if not already made, make spline coefficients */
	if (!made) {
		vs = alloc1float(ns);
		xsd = (float(*)[4])alloc1float(ns*4);
		zsd = (float(*)[4])alloc1float(ns*4);
		for (is=0; is<ns; ++is)
			vs[is] = is;
		cmonot(ns,vs,xs,xsd);
		cmonot(ns,vs,zs,zsd);
		made = 1;
	}
	
	/* zero source array */
	for (ix=0; ix<nx; ++ix)
		for (iz=0; iz<nz; ++iz)
			s[ix][iz] = 0.0 *dt ;
	
	/* compute time-dependent part of source function */
	sigma = 0.25/fmax;
	tbias = 3.0*sigma;
	ascale = -exp(0.5)/sigma;
	tscale = 0.5/(sigma*sigma);
	if (t>2.0*tbias) return;
	ts = ascale*(t-tbias)*exp(-tscale*(t-tbias)*(t-tbias));
	
	/* loop over extended source locations */
	for (v=vs[0],distprev=0.0,dv=1.0; dv!=0.0; distprev=dist,v+=dv) {
		
		/* determine x(v), z(v), dx/dv, and dz/dv along source */
		intcub(0,ns,vs,xsd,1,&v,&xv);
		intcub(0,ns,vs,zsd,1,&v,&zv);
		intcub(1,ns,vs,xsd,1,&v,&dxdv);
		intcub(1,ns,vs,zsd,1,&v,&dzdv);
		
		/* determine increment along extended source */
		if (dxdv==0.0)
			dv = dz/ABS(dzdv);
		else if (dzdv==0.0)
			dv = dx/ABS(dxdv);
		else
			dv = MIN(dz/ABS(dzdv),dx/ABS(dxdv));
		if (v+dv>vs[ns-1]) dv = vs[ns-1]-v;
		dist = dv*sqrt(dzdv*dzdv+dxdv*dxdv)/sqrt(dx*dx+dz*dz);
		
		/* determine source amplitude */
		amp = (dist+distprev)/2.0;
		
		/* let source contribute within limited distance */
		xvn = (xv-fx)/dx;
		zvn = (zv-fz)/dz;
		ixv = NINT(xvn); 
		izv = NINT(zvn);
		for (ix=MAX(0,ixv-3); ix<=MIN(nx-1,ixv+3); ++ix) {
			for (iz=MAX(0,izv-3); iz<=MIN(nz-1,izv+3); ++iz) {
				xn = ix-xvn;
				zn = iz-zvn;
				s[ix][iz] += ts*amp*exp(-xn*xn-zn*zn);
			}
		}
	}
}

/* prototype of subroutine used internally */
static float ricker (float t, float fpeak);

void ptsrc (float xs, float zs,
	int nx, float dx, float fx,
	int nz, float dz, float fz,
	float dt, float t, float fmax, float fpeak, float tdelay, float **s)
/*****************************************************************************
ptsrc - update source pressure function for a point source
******************************************************************************
Input:
xs		x coordinate of point source
zs		z coordinate of point source
nx		number of x samples
dx		x sampling interval
fx		first x sample
nz		number of z samples
dz		z sampling interval
fz		first z sample
dt		time step (ignored)
t		time at which to compute source function
fmax		maximum frequency (ignored)
fpeak		peak frequency

Output:
tdelay		time delay of beginning of source function
s		array[nx][nz] of source pressure at time t+dt
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 03/01/90
******************************************************************************/
{
	int ix,iz,ixs,izs;
	float ts,xn,zn,xsn,zsn;
float min,max;
	
	/* zero source array */
	for (ix=0; ix<nx; ++ix)
		for (iz=0; iz<nz; ++iz)
			s[ix][iz] = 0.0 * dt*fmax;
	
	/* compute time-dependent part of source function */
	/* fpeak = 0.5*fmax;  this is now getparred */

	tdelay = 1.0/fpeak;
	if (t>2.0*tdelay) return;
	ts = ricker(t-tdelay,fpeak);
	
	/* let source contribute within limited distance */
	xsn = (xs-fx)/dx;
	zsn = (zs-fz)/dz;
	ixs = NINT(xsn);
	izs = NINT(zsn);
min=max=s[ixs][izs];
	for (ix=MAX(0,ixs-3); ix<=MIN(nx-1,ixs+3); ++ix) {
		for (iz=MAX(0,izs-3); iz<=MIN(nz-1,izs+3); ++iz) {
			xn = ix-xsn;
			zn = iz-zsn;
			s[ix][iz] = ts*exp(-xn*xn-zn*zn);
if(min< s[ix][iz]) min=s[ix][iz];
if(max> s[ix][iz]) max=s[ix][iz];
		}
	}
}

static float ricker (float t, float fpeak)
/*****************************************************************************
ricker - Compute Ricker wavelet as a function of time
******************************************************************************
Input:
t		time at which to evaluate Ricker wavelet
fpeak		peak (dominant) frequency of wavelet
******************************************************************************
Notes:
The amplitude of the Ricker wavelet at a frequency of 2.5*fpeak is 
approximately 4 percent of that at the dominant frequency fpeak.
The Ricker wavelet effectively begins at time t = -1.0/fpeak.  Therefore,
for practical purposes, a causal wavelet may be obtained by a time delay
of 1.0/fpeak.
The Ricker wavelet has the shape of the second derivative of a Gaussian.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 04/29/90
******************************************************************************/
{
	float x,xx;
	
	x = PI*fpeak*t;
	xx = x*x;
	/* return (-6.0+24.0*xx-8.0*xx*xx)*exp(-xx); */
	/* return PI*fpeak*(4.0*xx*x-6.0*x)*exp(-xx); */
	return exp(-xx)*(1.0-2.0*xx);
}

/* 2D finite differencing subroutine */

/* functions declared and used internally */
static void star1 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp);
static void star2 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp);
static void star3 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp);
static void star4 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp);
static void absorb (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **pm, float **p, float **pp,
	int *abs);

void tstep2 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp, int *abs)
/*****************************************************************************
One time step of FD solution (2nd order in space) to acoustic wave equation
******************************************************************************
Input:
nx		number of x samples
dx		x sampling interval
nz		number of z samples
dz		z sampling interval
dt		time step
dvv		array[nx][nz] of density*velocity^2
od		array[nx][nz] of 1/density (NULL for constant density=1.0)
s		array[nx][nz] of source pressure at time t+dt
pm		array[nx][nz] of pressure at time t-dt
p		array[nx][nz] of pressure at time t

Output:
pp		array[nx][nz] of pressure at time t+dt
******************************************************************************
Notes:
This function is optimized for special cases of constant density=1 and/or
equal spatial sampling intervals dx=dz.  The slowest case is variable
density and dx!=dz.  The fastest case is density=1.0 (od==NULL) and dx==dz.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 03/13/90
******************************************************************************/
{
	/* convolve with finite-difference star (special cases for speed) */
	if (od!=NULL && dx!=dz) {
		star1(nx,dx,nz,dz,dt,dvv,od,s,pm,p,pp);
	} else if (od!=NULL && dx==dz) {
		star2(nx,dx,nz,dz,dt,dvv,od,s,pm,p,pp);
	} else if (od==NULL && dx!=dz) {
		star3(nx,dx,nz,dz,dt,dvv,od,s,pm,p,pp);
	} else {
		star4(nx,dx,nz,dz,dt,dvv,od,s,pm,p,pp);
	}
	
	/* absorb along boundaries */
        if (pml_thickness == 0) {
	   absorb(nx,dx,nz,dz,dt,dvv,od,pm,p,pp,abs);
        } else {
           pml_absorb(nx,dx,nz,dz,dt,dvv,od,pm,p,pp,abs);
	}
}

/* convolve with finite-difference star for variable density and dx!=dz */
static void star1 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp)
{
	int ix,iz;
	float xscale1,zscale1,xscale2,zscale2;
		
	/* determine constants */
	xscale1 = (dt*dt)/(dx*dx);
	zscale1 = (dt*dt)/(dz*dz);
	xscale2 = 0.25*xscale1;
	zscale2 = 0.25*zscale1;
	
	/* do the finite-difference star */
	for (ix=1; ix<nx-1; ++ix) {
		for (iz=1; iz<nz-1; ++iz) {
			pp[ix][iz] = 2.0*p[ix][iz]-pm[ix][iz] +
				dvv[ix][iz]*(
					od[ix][iz]*(
						xscale1*(
							p[ix+1][iz]+
							p[ix-1][iz]-
							2.0*p[ix][iz]
						) +
						zscale1*(
							p[ix][iz+1]+
							p[ix][iz-1]-
							2.0*p[ix][iz]
						)
					) +
					(
						xscale2*(
							(od[ix+1][iz]-
							od[ix-1][iz]) *
							(p[ix+1][iz]-
							p[ix-1][iz])
						) +
						zscale2*(
							(od[ix][iz+1]-
							od[ix][iz-1])*
							(p[ix][iz+1]-
							p[ix][iz-1])
						)
					)
				) +
				s[ix][iz];
		}
	}
}

/* convolve with finite-difference star for variable density and dx==dz */
static void star2 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp)
{
	int ix,iz;
	float scale1,scale2;
	
	if ( dx != dz ) 
		fprintf(stderr,"ASSERT FAILED: dx != dz in star2 \n");
	/* determine constants */
	scale1 = (dt*dt)/(dx*dx);
	scale2 = 0.25*scale1;
	
	/* do the finite-difference star */
	for (ix=1; ix<nx-1; ++ix) {
		for (iz=1; iz<nz-1; ++iz) {
			pp[ix][iz] = 2.0*p[ix][iz]-pm[ix][iz] +
				dvv[ix][iz]*(
					od[ix][iz]*(
						scale1*(
							p[ix+1][iz]+
							p[ix-1][iz]+
							p[ix][iz+1]+
							p[ix][iz-1]-
							4.0*p[ix][iz]
						)
					) +
					(
						scale2*(
							(od[ix+1][iz]-
							od[ix-1][iz]) *
							(p[ix+1][iz]-
							p[ix-1][iz]) +
							(od[ix][iz+1]-
							od[ix][iz-1]) *
							(p[ix][iz+1]-
							p[ix][iz-1])
						)
					)
				) +
				s[ix][iz];
		}
	}
}

/* convolve with finite-difference star for density==1.0 and dx!=dz */
static void star3 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp)
{
	int ix,iz;
	float xscale,zscale;
		
	if ( od != ((float **) NULL) ) 
		fprintf(stderr,"ASSERT FAILED: od !=  NULL in star3 \n");
	/* determine constants */
	xscale = (dt*dt)/(dx*dx);
	zscale = (dt*dt)/(dz*dz);
	
	/* do the finite-difference star */
	for (ix=1; ix<nx-1; ++ix) {
		for (iz=1; iz<nz-1; ++iz) {
			pp[ix][iz] = 2.0*p[ix][iz]-pm[ix][iz] +
				dvv[ix][iz]*(
					xscale*(
						p[ix+1][iz]+
						p[ix-1][iz]-
						2.0*p[ix][iz]
					) +
					zscale*(
						p[ix][iz+1]+
						p[ix][iz-1]-
						2.0*p[ix][iz]
					)
				) +
				s[ix][iz];
		}
	}
}

/* convolve with finite-difference star for density==1.0 and dx==dz */
static void star4 (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **s,
	float **pm, float **p, float **pp)
{
	int ix,iz;
	float scale;
	
	/* determine constants */
	if ( od != ((float **) NULL) ) 
		fprintf(stderr,"ASSERT FAILED: od !=  NULL in star4\n");
	if ( dz != dx ) 
		fprintf(stderr,"ASSERT FAILED: dz !=  dx in star4\n");
	scale = (dt*dt)/(dx*dz);
	
	/* do the finite-difference star */
	for (ix=1; ix<nx-1; ++ix) {
		for (iz=1; iz<nz-1; ++iz) {
			pp[ix][iz] = 2.0*p[ix][iz]-pm[ix][iz] +
				scale*dvv[ix][iz]*(
					p[ix+1][iz]+
					p[ix-1][iz]+
					p[ix][iz+1]+
					p[ix][iz-1]-
					4.0*p[ix][iz]
				) +
				s[ix][iz];
		}
	}
}

static void absorb (int nx, float dx, int nz, float dz, float dt,
	float **dvv, float **od, float **pm, float **p, float **pp,
	int *abs)
/*****************************************************************************
absorb - absorbing boundary conditions 
*****************************************************************************
Input:
nx	number of samples in x direction
dx	spatial sampling interval in x direction
nz	number of samples in z direction
dz	spatial sampling interval in z direction
dt	time sampling interval
dvv	array of velocity values from vfile
od	array of density values from dfile
pm	pressure field at time t-1
p	pressure field at time t
pp	pressure field at t+dt
abs	flag indicating to absorb or not to absorb

*****************************************************************************
Notes:
This method is an improvement on the method of Clayton and Engquist, 1977
and 1980. The method is described in Hale, 1990.

*****************************************************************************
References:
Clayton, R. W., and Engquist, B., 1977, Absorbing boundary conditions
for acoustic and elastic wave equations, Bull. Seism. Soc. Am., 6, 1529-1540. 

Clayton, R. W., and Engquist, B., 1980, Absorbing boundary conditions
for wave equation migration, Geophysics, 45, 895-904.

Hale, D.,  1990, Adaptive absorbing boundaries for finite-difference
modeling of the wave equation migration, unpublished report from the
Center for Wave Phenomena, Colorado School of Mines.

Richtmyer, R. D., and Morton, K. W., 1967, Difference methods for
initial-value problems, John Wiley & Sons, Inc, New York.

Thomee, V., 1962, A stable difference scheme for the mixed boundary problem
for a hyperbolic, first-order system in two dimensions, J. Soc. Indust.
Appl. Math., 10, 229-245.

Toldi, J. L., and Hale, D., 1982, Data-dependent absorbing side boundaries,
Stanford Exploration Project Report SEP-30, 111-121.

*****************************************************************************
Author: CWP: Dave Hale 1990 
******************************************************************************/
{
	int ix,iz;
	float ov,ovs,cosa,beta,gamma,dpdx,dpdz,dpdt,dpdxs,dpdzs,dpdts;

	/* solve for upper boundary */
	iz = 1;
	for (ix=0; ix<nx; ++ix) {

		if (abs[0]!=0) {

			if (od!=NULL)
				ovs = 1.0/(od[ix][iz]*dvv[ix][iz]);
			else
				ovs = 1.0/dvv[ix][iz];
			ov = sqrt(ovs);
			if (ix==0)
				dpdx = (p[1][iz]-p[0][iz])/dx;
			else if (ix==nx-1)
				dpdx = (p[nx-1][iz]-p[nx-2][iz])/dx;
			else
				dpdx = (p[ix+1][iz]-p[ix-1][iz])/(2.0*dx);
			dpdt = (pp[ix][iz]-pm[ix][iz])/(2.0*dt);
			dpdxs = dpdx*dpdx;
			dpdts = dpdt*dpdt;
			if (ovs*dpdts>dpdxs)
				cosa = sqrt(1.0-dpdxs/(ovs*dpdts));
			else 
				cosa = 0.0;

			beta = ov*dz/dt*cosa;
			gamma = (1.0-beta)/(1.0+beta);

			pp[ix][iz-1] = gamma*(pp[ix][iz]-p[ix][iz-1])+p[ix][iz];
		} else {
			pp[ix][iz-1] = 0.0;
		}
	}


	/* extrapolate along left boundary */
	ix = 1;
	for (iz=0; iz<nz; ++iz) {
		if (abs[1]!=0) {
			if (od!=NULL)
				ovs = 1.0/(od[ix][iz]*dvv[ix][iz]);
			else
				ovs = 1.0/dvv[ix][iz];
			ov = sqrt(ovs);
			if (iz==0)
				dpdz = (p[ix][1]-p[ix][0])/dz;
			else if (iz==nz-1)
				dpdz = (p[ix][nz-1]-p[ix][nz-2])/dz;
			else
				dpdz = (p[ix][iz+1]-p[ix][iz-1])/(2.0*dz);
			dpdt = (pp[ix][iz]-pm[ix][iz])/(2.0*dt);
			dpdzs = dpdz*dpdz;
			dpdts = dpdt*dpdt;
			if (ovs*dpdts>dpdzs)
				cosa = sqrt(1.0-dpdzs/(ovs*dpdts));
			else
				cosa = 0.0;

			beta = ov*dx/dt*cosa;
			gamma = (1.0-beta)/(1.0+beta);
			pp[ix-1][iz] = gamma*(pp[ix][iz]-p[ix-1][iz])+p[ix][iz];
		} else {
			pp[ix-1][iz] = 0.0;
		}
	}

	/* extrapolate along lower boundary */
	iz = nz-2;
	for (ix=0; ix<nx; ++ix) {
		if (abs[2]!=0) {
			if (od!=NULL)
				ovs = 1.0/(od[ix][iz]*dvv[ix][iz]);
			else
				ovs = 1.0/dvv[ix][iz];
			ov = sqrt(ovs);
			if (ix==0)
				dpdx = (p[1][iz]-p[0][iz])/dx;
			else if (ix==nx-1)
				dpdx = (p[nx-1][iz]-p[nx-2][iz])/dx;
			else
				dpdx = (p[ix+1][iz]-p[ix-1][iz])/(2.0*dx);
			dpdt = (pp[ix][iz]-pm[ix][iz])/(2.0*dt);
			dpdxs = dpdx*dpdx;
			dpdts = dpdt*dpdt;
			if (ovs*dpdts>dpdxs)
				cosa = sqrt(1.0-dpdxs/(ovs*dpdts));
			else 
				cosa = 0.0;

			beta = ov*dz/dt*cosa;
			gamma = (1.0-beta)/(1.0+beta);

			pp[ix][iz+1] = gamma*(pp[ix][iz]-p[ix][iz+1])+p[ix][iz];
		} else {
			pp[ix][iz+1] = 0.0;
		}
	}

	/* extrapolate along right boundary */
	ix = nx-2;
	for (iz=0; iz<nz; ++iz) {
		if (abs[3]!=0) {
			if (od!=NULL)
				ovs = 1.0/(od[ix][iz]*dvv[ix][iz]);
			else
				ovs = 1.0/dvv[ix][iz];
			ov = sqrt(ovs);
			if (iz==0)
				dpdz = (p[ix][1]-p[ix][0])/dz;
			else if (iz==nz-1)
				dpdz = (p[ix][nz-1]-p[ix][nz-2])/dz;
			else
				dpdz = (p[ix][iz+1]-p[ix][iz-1])/(2.0*dz);
			dpdt = (pp[ix][iz]-pm[ix][iz])/(2.0*dt);
			dpdzs = dpdz*dpdz;
			dpdts = dpdt*dpdt;
			if (ovs*dpdts>dpdzs)
				cosa = sqrt(1.0-dpdzs/(ovs*dpdts));
			else
				cosa = 0.0;

			beta = ov*dx/dt*cosa;
			gamma = (1.0-beta)/(1.0+beta);
			pp[ix+1][iz] =gamma*(pp[ix][iz]-p[ix+1][iz])+p[ix][iz];
		} else {
			pp[ix+1][iz] = 0.0;
		}
	}
}


/*

   pml_absorb uses the perfectly matched layer absorbing boundary condition.
   The PML formulation is specialized to the acoustic case.

   References:
   Jean-Pierre Berenger, ``A Perfectly Matched Layer for the Absorption of
   Electromagnetic Waves,''  Journal of Computational Physics, vol. 114,
   pp. 185-200.

   Hastings, Schneider, and Broschat, ``Application of the perfectly
   matched layer (PML) absorbing boundary condition to elastic wave
   propogation,''  Journal of the Accoustical Society of America,
   November, 1996.

   Allen Taflove, ``Electromagnetic Modeling:  Finite Difference Time
   Domain Methods'', Baltimore, Maryland: Johns Hopkins University Press,
   1995, chap. 7, pp. 181-195.

   The PML ABC is implemented by extending the modeled region on
   the bottom and right sides and treating the modeled region as
   periodic.

   In the extended region, the differential equations of PML are
   modeled.  The extension is accomplished by using additional
   arrays which record the state in the extended regions.  The
   result is a nasty patchwork of arrays.  (It is possible to use
   the PML differential equations to model the absorbing and
   non-absorbing regions.  This greatly simplifies things at the
   expense of memory.)

   The size of the new arrays and the location of their (0,0)
   element in the coordinate space of the main p arrays are:


   Array        Size        Location of [0][0] with respect to p [0][0]
   -----   ---------------  -------------------------------------------
   ux_b    (nx+pml, pml+2)  (0, nz-1)
   uz_b         "               "
   dax_b        "               "
   dbx_b        "               "
   daz_b        "               "
   dbz_b        "               "
 
   ux_r    (pml+2, nz)      (nx-1, 0)
   uz_r         "               "
   dax_r        "               "
   dbx_r        "               "
   daz_r        "               "
   dbz_r        "               "

   v_b     (nx+pml, pml+3)  (0, nz-1.5)
   cax_b        "               "
   cbx_b        "               "

   w_b     (nx+pml, pml+2)  (-0.5, nz-1)
   caz_b        "               "
   cbz_b        "               "

   v_r     (pml+2, nz)      (nx-1, -0.5)
   cax_r        "               "
   cbx_r        "               "

   w_r     (pml+3, nz)      (nx-1.5, 0)
   caz_r        "               "
   cbz_r        "               "
 
*/

static void pml_absorb (int nx, float dx, int nz, float dz, float dt,
        float **dvv, float **od, float **pm, float **p, float **pp,
        int *abs)
{
        int ix, iz, jx, jz;

   /* Calculate v for bottom pad above and below main domain */

   for (ix=0, jz=pml_thickness+2; ix<nx; ++ix) {
      v_b  [ix][ 0] = cax_b [ix][ 0] * v_b [ix][ 0] +
                      cbx_b [ix][ 0] * (ux_b [ix][   0] + uz_b [ix][   0]
                                       -((abs[2]!=0) ? p [ix][nz-2] : 0.0));

      v_b  [ix][jz] = cax_b [ix][jz] * v_b [ix][jz] +
                      cbx_b [ix][jz] * (((abs[0]!=0) ? p [ix][   1] : 0.0)
                                       -ux_b [ix][jz-1] - uz_b [ix][jz-1]);
   }


   /* Calculate v for bottom pad above and below right pad */

   for (ix=nx, jx=1, jz=pml_thickness+2; ix<nx+pml_thickness; ++ix, ++jx) {
      v_b  [ix][ 0] = cax_b [ix][ 0] * v_b [ix][ 0] +
                      cbx_b [ix][ 0] * (ux_b [ix][   0] + uz_b [ix][   0]
                                       -ux_r [jx][nz-2] - uz_r [jx][nz-2]);

      v_b  [ix][jz] = cax_b [ix][jz] * v_b [ix][jz] +
                      cbx_b [ix][jz] * (ux_r [jx][   1] + uz_r [jx][   1]
                                       -ux_b [ix][jz-1] - uz_b [ix][jz-1]);
   }


   /* Calculate v for main part of bottom pad */

   for (ix=0; ix<nx+pml_thickness; ++ix) {
      for (iz=1; iz<pml_thickness+2; ++iz) {
         v_b [ix][iz] = cax_b [ix][iz] * v_b [ix][iz] +
                        cbx_b [ix][iz] * (ux_b [ix][iz  ] + uz_b [ix][iz  ]
                                         -ux_b [ix][iz-1] - uz_b [ix][iz-1]);
      }
   }


   /* Calculate w for left edge of bottom pad */

   for (iz=0, ix=nx+pml_thickness-1; iz<pml_thickness+2; ++iz) {
      w_b [ 0][iz] = caz_b [ 0][iz] * w_b [ 0][iz] +
                     cbz_b [ 0][iz] * (ux_b [ix][iz] + uz_b [ix][iz]
                                      -ux_b [ 0][iz] - uz_b [ 0][iz]);
   }


   /* Calculate w for main part of bottom pad */

   for (ix=1; ix<nx+pml_thickness; ++ix) {
      for (iz=0; iz<pml_thickness+2; ++iz) {
         w_b [ix][iz] = caz_b [ix][iz] * w_b [ix][iz] +
                        cbz_b [ix][iz] * (ux_b [ix-1][iz] + uz_b [ix-1][iz]
                                         -ux_b [ix  ][iz] - uz_b [ix  ][iz]);
      }
   }


   /* Calculate v along top and bottom edge of right pad */

   for (ix=0, jx=nx-1, jz=pml_thickness; ix<pml_thickness+2; ++ix, ++jx) {
      if (jx == nx+pml_thickness) jx = 0;

      v_r [ix][   0] = cax_r [ix][   0] * v_r [ix][   0] +
                       cbx_r [ix][   0] * (ux_r [ix][ 0] + uz_r [ix][ 0]
                                          -ux_b [jx][jz] - uz_b [jx][jz]);

      v_r [ix][nz-1] = cax_r [ix][nz-1] * v_r [ix][nz-1] +
                       cbx_r [ix][nz-1] * (ux_b [jx][   0] + uz_b [jx][   0]
                                          -ux_r [ix][nz-2] - uz_r [ix][nz-2]);
   }


   /* Calculate v in rest of right pad */

   for (ix=0; ix<pml_thickness+2; ++ix) {
      for (iz=1; iz<nz-1; ++iz) {
         v_r [ix][iz] = cax_r [ix][iz] * v_r [ix][iz] +
                        cbx_r [ix][iz] * (ux_r [ix][iz  ] + uz_r [ix][iz  ]
                                         -ux_r [ix][iz-1] - uz_r [ix][iz-1]);
      }
   }


   /* Calculate w along left and right sides of right pad */

   for (iz=0, jx=pml_thickness+2; iz<nz; ++iz) {
      w_r [ 0][iz] = caz_r [ 0][iz] * w_r [ 0][iz] +
                     cbz_r [ 0][iz] * (((abs[3]!=0) ? p [nx-2][iz] : 0.0)
                                      -ux_r [   0][iz] - uz_r [   0][iz]);

      w_r [jx][iz] = caz_r [jx][iz] * w_r [jx][iz] +
                     cbz_r [jx][iz] * (ux_r [jx-1][iz] + uz_r [jx-1][iz]
                                      -((abs[1]!=0) ? p [1][iz] : 0.0));
   }


   /* Calculate w in main part of right pad */

   for (ix=1; ix<pml_thickness+2; ++ix) {
      for (iz=0; iz<nz; ++iz) {
         w_r [ix][iz] = caz_r [ix][iz] * w_r [ix][iz] +
                        cbz_r [ix][iz] * (ux_r [ix-1][iz] + uz_r [ix-1][iz]
                                         -ux_r [ix  ][iz] - uz_r [ix  ][iz]);
      }
   }


   /* Calculate ux and uz in bottom pad */

   for (ix=0; ix<nx+pml_thickness-1; ++ix) {
      for (iz=0; iz<pml_thickness+2; ++iz) {
         ux_b [ix][iz] = dax_b [ix][iz] * ux_b [ix  ][iz] +
                         dbx_b [ix][iz] * (w_b [ix][iz  ] - w_b [ix+1][iz]);
      }
   }

   for (ix=nx+pml_thickness-1, iz=0; iz<pml_thickness+2; ++iz) {
      ux_b [ix][iz] = dax_b [ix][iz] * ux_b [ix  ][iz] + 
                      dbx_b [ix][iz] * (w_b [ix][iz  ] - w_b [   0][iz]);
   }


   for (ix=0; ix<nx+pml_thickness; ++ix) {
      for (iz=0; iz<pml_thickness+2; ++iz) {
         uz_b [ix][iz] = daz_b [ix][iz] * uz_b [ix][iz  ] +
                         dbz_b [ix][iz] * (v_b [ix][iz+1] - v_b [ix][iz  ]);
      }
   }


   /* Calculate ux and uz in right pad */

   for (ix=0; ix<pml_thickness+2; ++ix) {
      for (iz=0; iz<nz; ++iz) {
         ux_r [ix][iz] = dax_r [ix][iz] * ux_r [ix  ][iz] +
                         dbx_r [ix][iz] * (w_r [ix  ][iz] - w_r [ix+1][iz]);
      }
   }

   for (ix=0; ix<pml_thickness+2; ++ix) {
      for (iz=0; iz<nz-1; ++iz) {
         uz_r [ix][iz] = daz_r [ix][iz] * uz_r [ix][iz  ] +
                         dbz_r [ix][iz] * (v_r [ix][iz+1] - v_r [ix][iz  ]);
      }
   }

   for (ix=0, iz=nz-1, jx=nx-1; ix<pml_thickness+2; ++ix, ++jx) {
      if (jx == nx+pml_thickness) jx = 0;

      uz_r [ix][ 0] = uz_b [jx][pml_thickness+1];
      uz_r [ix][iz] = uz_b [jx][              0];
   }


   /* Update top and bottom edge of main grid with new field values */

   for (ix=0, jz=pml_thickness+1; ix<nx; ++ix) {
      if (abs [0] != 0) pp [ix][   0] = ux_b [ix][jz] + uz_b [ix][jz];
      if (abs [2] != 0) pp [ix][nz-1] = ux_b [ix][ 0] + uz_b [ix][ 0];
   }


   /* Update left and right edges of main grid with new field values */

   for (iz=1, jx=pml_thickness+1; iz<nz-1; ++iz) {
      if (abs [1] != 0) pp [   0][iz] = ux_r [jx][iz] + uz_r [jx][iz];
      if (abs [3] != 0) pp [nx-1][iz] = ux_r [ 0][iz] + uz_r [ 0][iz];
   }
}


static void pml_alloc (int nx, int nz, float dx, float dz, float dt, float **dvv)
{
   cax_r = alloc2float (nz, pml_thickness+2);
   cbx_r = alloc2float (nz, pml_thickness+2);
   caz_r = alloc2float (nz, pml_thickness+3);
   cbz_r = alloc2float (nz, pml_thickness+3);
   dax_r = alloc2float (nz, pml_thickness+2);
   dbx_r = alloc2float (nz, pml_thickness+2);
   daz_r = alloc2float (nz, pml_thickness+2);
   dbz_r = alloc2float (nz, pml_thickness+2);

   ux_r  = alloc2float (nz, pml_thickness+2);
   uz_r  = alloc2float (nz, pml_thickness+2);
   v_r   = alloc2float (nz, pml_thickness+2);
   w_r   = alloc2float (nz, pml_thickness+3);


   /* Zero out arrays for pad on right */
}
static void pml_init (int nx, int nz, float dx, float dz, float dt, float **dvv)
{
   int ix, iz;

   /* Allocate arrays for pad on right */


   for (ix=0; ix<pml_thickness+2; ++ix) {
      for (iz=0; iz<nz; ++iz) {
         ux_r  [ix][iz] = uz_r  [ix][iz] = 0.0;
         v_r   [ix][iz] = w_r   [ix][iz] = 0.0;

         cax_r [ix][iz] = cbx_r [ix][iz] = 0.0;
         caz_r [ix][iz] = cbz_r [ix][iz] = 0.0;
         dax_r [ix][iz] = dbx_r [ix][iz] = 0.0;
         daz_r [ix][iz] = dbz_r [ix][iz] = 0.0;
      }
   }


   /* Zero out extra bit on right pad */

   for (ix=pml_thickness+2, iz=0; iz<nz; ++iz) {
      caz_r [ix][iz] = cbz_r [ix][iz] = 0.0;
      w_r   [ix][iz] = 0.0;
   }


   /* Allocate arrays for pad on bottom */

   cax_b = alloc2float (pml_thickness+3, nx + pml_thickness);
   cbx_b = alloc2float (pml_thickness+3, nx + pml_thickness);
   caz_b = alloc2float (pml_thickness+2, nx + pml_thickness);
   cbz_b = alloc2float (pml_thickness+2, nx + pml_thickness);
   dax_b = alloc2float (pml_thickness+2, nx + pml_thickness);
   dbx_b = alloc2float (pml_thickness+2, nx + pml_thickness);
   daz_b = alloc2float (pml_thickness+2, nx + pml_thickness);
   dbz_b = alloc2float (pml_thickness+2, nx + pml_thickness);

   ux_b  = alloc2float (pml_thickness+2, nx + pml_thickness);
   uz_b  = alloc2float (pml_thickness+2, nx + pml_thickness);
   v_b   = alloc2float (pml_thickness+3, nx + pml_thickness);
   w_b   = alloc2float (pml_thickness+2, nx + pml_thickness);
	

   /* Zero out arrays for pad on bottom */

   for (ix=0; ix<nx+pml_thickness; ++ix) {
      for (iz=0; iz<pml_thickness+2; ++iz) {
         ux_b  [ix][iz] = uz_b  [ix][iz] = 0.0;
         v_b   [ix][iz] = w_b   [ix][iz] = 0.0;

         cax_b [ix][iz] = cbx_b [ix][iz] = 0.0;
         caz_b [ix][iz] = cbz_b [ix][iz] = 0.0;
         dax_b [ix][iz] = dbx_b [ix][iz] = 0.0;
         daz_b [ix][iz] = dbz_b [ix][iz] = 0.0;
      }
   }


   /* Zero out extra bit on bottom pad */

   for (ix=0, iz=pml_thickness+2; ix<nx+pml_thickness; ++ix) {
      cax_b [ix][iz] = cbx_b [ix][iz] = 0.0;
      v_b   [ix][iz] = 0.0;
   }


   /* Initialize cax & cbx arrays */

   for (ix=0; ix<pml_thickness+2; ++ix) {
      for (iz=0; iz<nz; ++iz) {
         sigma_ez = 0.0;

         cax_r [ix][iz] = (2.0 - (sigma_ez * dt)) / (2.0 + (sigma_ez * dt));
         cbx_r [ix][iz] = (2.0 * dt / dz)         / (2.0 + (sigma_ez * dt));
      }
   }

   for (ix=0; ix<nx+pml_thickness; ++ix) {
      for (iz=0; iz<pml_thickness+3; ++iz) {
         if ((iz == 0) || (iz == pml_thickness + 2)) {
            sigma_ez = 0.0;
         } else {
            sigma_ez = pml_max * 0.5 * (1.0 - cos (2*PI*(iz-0.5)/(pml_thickness+1)));
         }

         cax_b [ix][iz] = (2.0 - (sigma_ez * dt)) / (2.0 + (sigma_ez * dt));
         cbx_b [ix][iz] = (2.0 * dt / dz)         / (2.0 + (sigma_ez * dt));
      }
   }


   /* Initialize caz & cbz arrays */

   for (ix=0; ix<pml_thickness+3; ++ix) {
      for (iz=0; iz<nz; ++iz) {
         if ((ix == 0) || (ix == pml_thickness+2)) {
            sigma_ex = 0.0;
         } else {
            sigma_ex = pml_max * 0.5 * (1.0 - cos (2*PI*(ix-0.5)/(pml_thickness+1)));
         }

         caz_r [ix][iz] = (2.0 - (sigma_ex * dt)) / (2.0 + (sigma_ex * dt));
         cbz_r [ix][iz] = (2.0 * dt / dz)         / (2.0 + (sigma_ex * dt));
      }
   }


   for (ix=0; ix<nx+pml_thickness; ++ix) {
      for (iz=0; iz<pml_thickness+2; ++iz) {
         if (ix == 0) {
            sigma_ex = pml_max * 0.5 * (1.0 - cos (2*PI*(0.5)/(pml_thickness+1)));
         } else if (ix < nx) {
            sigma_ex = 0.0;
         } else {
            sigma_ex = pml_max * 0.5 * (1.0 - cos (2*PI*(ix-nx+0.5)/(pml_thickness+1)));
         }

         caz_b [ix][iz] = (2.0 - (sigma_ex * dt)) / (2.0 + (sigma_ex * dt));
         cbz_b [ix][iz] = (2.0 * dt / dz)         / (2.0 + (sigma_ex * dt));
      }
   }


   /* Initialize right pad's dax dbx, daz, & dbz arrays */

   for (ix=0; ix<pml_thickness+2; ++ix) {
      for (iz=0; iz<nz; ++iz) {

         /* Determine sigma_mx and sigma_mz */

         if ((ix == 0) || (ix == pml_thickness+1)) {
            sigma_mx = 0.0;
         } else {
            sigma_mx = pml_max * 0.5 * (1.0 - cos (2*PI*(ix)/(pml_thickness+1)));
         }

         sigma_mz = 0.0;


         /* Determine velocity, interpolate */

         if (ix == 0) {
            dvv_0 = sqrt (dvv [nx-1][iz]);
         } else if (ix == pml_thickness+1) {
            dvv_0 = sqrt (dvv [   0][iz]);
         } else {
            dvv_0 = sqrt (dvv [nx-1][iz]);
            dvv_1 = sqrt (dvv [   0][iz]);

            dvv_0 = ((ix) * dvv_1 + (1+pml_thickness-ix)*dvv_0);
            dvv_0 /= (pml_thickness+1);
         }

         dvv_0 = dvv_0 * dvv_0;


         dax_r [ix][iz] = (2.0 - (sigma_mx * dt)) / (2.0 + (sigma_mx * dt));
         dbx_r [ix][iz] = (2.0 * dt * dvv_0 / dx) / (2.0 + (sigma_mx * dt));

         daz_r [ix][iz] = (2.0 - (sigma_mz * dt)) / (2.0 + (sigma_mz * dt));
         dbz_r [ix][iz] = (2.0 * dt * dvv_0 / dz) / (2.0 + (sigma_mz * dt));
      }
   }


   /* Initialize bottom pad's dax, dbx, daz, & dbz arrays */

   for (ix=0; ix<nx+pml_thickness; ++ix) {
      for (iz=0; iz<pml_thickness+2; ++iz) {

         /* Determine sigma_mx and sigma_mz */

         if (ix < nx) {
            sigma_mx = 0.0;
         } else {
            sigma_mx = pml_max * 0.5 * (1.0 - cos (2*PI*(ix-nx+1)/(pml_thickness+1)));
         }

         if ((iz == 0) || (iz == pml_thickness+1)) {
            sigma_mz = 0.0;
         } else {
            sigma_mz = pml_max * 0.5 * (1.0 - cos (2*PI*(iz)/(pml_thickness+1)));
         }


         /* Determine velocity, interpolate */

         if (ix < nx) {
            if (iz == 0) {
               dvv_0 = sqrt (dvv [ix][nz-1]);
            } else if (iz == pml_thickness+1) {
               dvv_0 = sqrt (dvv [ix][   0]);
            } else {
               dvv_0 = sqrt (dvv [ix][nz-1]);
               dvv_1 = sqrt (dvv [ix][   0]);

               dvv_0 = ((iz) * dvv_1 + (1+pml_thickness-iz)*dvv_0);
               dvv_0 /= (pml_thickness+1);
            }
         } else {
            if (iz == 0) {
               dvv_0 = sqrt (dvv [nx-1][nz-1]);
               dvv_1 = sqrt (dvv [   0][nz-1]);
            } else if (iz == pml_thickness+1) {
               dvv_0 = sqrt (dvv [nx-1][   0]);
               dvv_1 = sqrt (dvv [   0][   0]);
            } else {
               dvv_2 = sqrt (dvv [nx-1][nx-1]);
               dvv_3 = sqrt (dvv [nx-1][   0]);

               dvv_0 = ((iz) * dvv_3 + (1+pml_thickness-iz)*dvv_2);
               dvv_0 /= (pml_thickness+1);

               dvv_2 = sqrt (dvv [0][nx-1]);
               dvv_3 = sqrt (dvv [0][   0]);

               dvv_1 = ((iz) * dvv_3 + (1+pml_thickness-iz)*dvv_2);
               dvv_1 /= (pml_thickness+1);
            }

            dvv_0 = ((ix-nx+1) * dvv_1 + (nx+pml_thickness-ix)*dvv_0);
            dvv_0 /= (pml_thickness+1);
         }

         dvv_0 = dvv_0 * dvv_0;

         dax_b [ix][iz] = (2.0 - (sigma_mx * dt)) / (2.0 + (sigma_mx * dt));
         dbx_b [ix][iz] = (2.0 * dt * dvv_0 / dx) / (2.0 + (sigma_mx * dt));

         daz_b [ix][iz] = (2.0 - (sigma_mz * dt)) / (2.0 + (sigma_mz * dt));
         dbz_b [ix][iz] = (2.0 * dt * dvv_0 / dz) / (2.0 + (sigma_mz * dt));
      }
   }
}
