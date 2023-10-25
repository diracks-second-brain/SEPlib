/*$


=head1 NAME

Kirch_2d_depth - Kirchhoff 2D migration

=head1 SYNOPSIS

Kirch_2d_depth  <in.H >out.H tfile=tfile.H  pars


=head1 INPUT PARAMETERS

=over 4

=item stdin	  -	sepfile

    File for input seismic traces (must be a sep3d file)

=item stdout	 - sepfile

    File for common offset migration output  (	sep regular cube)

=item ttfile	 -	sepfile

   File for input traveltime tables. First axis is Z, second axis X, third

    axis is regular spaced source locations. If not provided will be
  
   calculated.

=item velfile	 -	sepfile

   Velocity file (only needed if travel time file not provided).

=item dxm  -  float

   [d3 of grid] Sampling interval of input midpoints 		

=item fzo  - float

  z-coordinate of first point in output trace 	

=item dzo - float

  vertical spacing of output trace 		

=item nzo -  int

  number of points in output trace			

=item fmax - float

  [0.25/dt]		frequency-highcut for input traces		

=item aperx - float

  [nxt*dxt/2]  	migration lateral aperature 			

=item angmax - float

 [60]		migration angle aperature from vertical 	

=item v0  - float

  [1500](m/s)		reference velocity value at surface			

=item dvz - float

 [0.0]  		reference velocity vertical gradient		

=item  ls  -int 

 [1]	                flag for line source				

=item  verb  - int

  [0]   Whether or not to be verbose

=item  max_size  - int

  [100]  Approximate maximum memory to use


=back

=head1 DESCRIPTION

Performs 2D Kirchoff depth migraion. 

=head1 NOTES

1. Output regular grid is the basis of the output cmp_x and offset_x

   parameters. The input dataset should be sep3d gridded data.

   Axis 1 - time, axis 2 - trace_in_bin, axis3-cmp_x, axis4 - offset_x .

   The header keys offset_x and cmp_x must exist in the dataset.

2. This is SU's sukdmig2d code with only very slight modifications.

   All credit belongs to  CWP.  If you use it in a paper

   make sure to credit them.

3. Only works meters/feet for some reason???

4. Traveltime tables were generated by program rayt2d (or other ones)	

   on relatively coarse grids, with dimension ns*nxt*nzt. In the	

   migration process, traveltimes are interpolated into shot/gephone 	

   positions and output grids.					',

5. If the offset value of an input trace is not in the offset array 	

   of output, the nearest one in the array is chosen. 		

6. Memory requirement for this program is about			

   	[ns*nxt*nzt+noff*nxo*nzo+4*nr*nzt+5*nxt*nzt+npa*(2*ns*nxt*nzt   

+noff*nxo*nzo+4*nxt*nzt)]*4 bytes				

   where nr = 1+min(nxt*dxt,0.5*offmax+aperx)/dxo. 			

6. Amplitudes are computed using the reference velocity profile, v(z),

   specified by the parameters v0= and dvz=.				

=head1 AUTHOR

   Zhenyue Liu, 03/01/95,  Colorado School of Mines 

=head1 CATEGORY

B<seis/image>


=cut



*/




















/* Copyright (c) Colorado School of Mines, 1999.*/
/* All rights reserved.                       */

/* SUKDMIG2D: $Revision: 1.2 $ ; $Date: 2004/07/08 18:15:32 $	*/

#include <sulib.h>
#include <sepmath.h>
#include <sep3dc.h>
#include <sep.main>
#include <septravel.h>

 
/**************** end self doc ***********************************/
  void resit(int nx,float fx,float dx,int nz,int nr,float dr,
	float **tb,float **t,float x0);
  void interpx(int nxt,float fxt,float dxt,int nx,float fx,float dx,
	int nzt,float **tt,float **t);
  void sum2(int nx,int nz,float a1,float a2,float **t1,float **t2,float **t);
  void timeb(int nr,int nz,float dr,float dz,float fz,float a,
	float v0,float **t,float **p,float **sig,float **ang);
  void mig2d(float *trace,int nt,float ft,float dt,
	float sx,float gx,float **mig,float aperx,
  	int nx,float fx,float dx,float nz,float fz,float dz,
	int ls,int mtmax,float dxm,float fmax,float angmax,
	float **tb,float **pb,float **cs0b,float **angb,int nr,
	float **tsum,int nzt,float fzt,float dzt,int nxt,float fxt,float dxt,
	int npv,float **cssum,float **tvsum,float **mig1);
  int  calc_ttfiles(sep3d input,int verb);

/* segy trace */

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Kirch_2d_depth - Kirchhoff 2D migration");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Kirch_2d_depth <in.H >out.H tfile=tfile.H pars");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    stdin - sepfile");\
 sep_add_doc_line("            File for input seismic traces (must be a sep3d file)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    stdout - sepfile");\
 sep_add_doc_line("            File for common offset migration output  (  sep regular cube)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ttfile - sepfile");\
 sep_add_doc_line("           File for input traveltime tables. First axis is Z, second axis X, third");\
 sep_add_doc_line("");\
 sep_add_doc_line("            axis is regular spaced source locations. If not provided will be");\
 sep_add_doc_line("");\
 sep_add_doc_line("           calculated.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    velfile - sepfile");\
 sep_add_doc_line("           Velocity file (only needed if travel time file not provided).");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dxm - float");\
 sep_add_doc_line("           [d3 of grid] Sampling interval of input midpoints            ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    fzo - float");\
 sep_add_doc_line("          z-coordinate of first point in output trace   ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dzo - float");\
 sep_add_doc_line("          vertical spacing of output trace              ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nzo - int");\
 sep_add_doc_line("          number of points in output trace                      ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    fmax - float");\
 sep_add_doc_line("          [0.25/dt]             frequency-highcut for input traces              ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    aperx - float");\
 sep_add_doc_line("          [nxt*dxt/2]   migration lateral aperature                     ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    angmax - float");\
 sep_add_doc_line("         [60]           migration angle aperature from vertical         ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    v0 - float");\
 sep_add_doc_line("          [1500](m/s)           reference velocity value at surface                     ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dvz - float");\
 sep_add_doc_line("         [0.0]                  reference velocity vertical gradient            ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ls -int");\
 sep_add_doc_line("         [1]                    flag for line source                            ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    verb - int");\
 sep_add_doc_line("          [0]   Whether or not to be verbose");\
 sep_add_doc_line("");\
 sep_add_doc_line("    max_size - int");\
 sep_add_doc_line("          [100]  Approximate maximum memory to use");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Performs 2D Kirchoff depth migraion.");\
 sep_add_doc_line("");\
 sep_add_doc_line("NOTES");\
 sep_add_doc_line("    1. Output regular grid is the basis of the output cmp_x and offset_x");\
 sep_add_doc_line("");\
 sep_add_doc_line("       parameters. The input dataset should be sep3d gridded data.");\
 sep_add_doc_line("");\
 sep_add_doc_line("       Axis 1 - time, axis 2 - trace_in_bin, axis3-cmp_x, axis4 - offset_x .");\
 sep_add_doc_line("");\
 sep_add_doc_line("       The header keys offset_x and cmp_x must exist in the dataset.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    2. This is SU's sukdmig2d code with only very slight modifications.");\
 sep_add_doc_line("");\
 sep_add_doc_line("       All credit belongs to  CWP.  If you use it in a paper");\
 sep_add_doc_line("");\
 sep_add_doc_line("       make sure to credit them.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    3. Only works meters/feet for some reason???");\
 sep_add_doc_line("");\
 sep_add_doc_line("    4. Traveltime tables were generated by program rayt2d (or other ones)");\
 sep_add_doc_line("");\
 sep_add_doc_line("       on relatively coarse grids, with dimension ns*nxt*nzt. In the        ");\
 sep_add_doc_line("");\
 sep_add_doc_line("       migration process, traveltimes are interpolated into shot/gephone    ");\
 sep_add_doc_line("");\
 sep_add_doc_line("       positions and output grids.                                  ',");\
 sep_add_doc_line("");\
 sep_add_doc_line("    5. If the offset value of an input trace is not in the offset array");\
 sep_add_doc_line("");\
 sep_add_doc_line("       of output, the nearest one in the array is chosen.           ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    6. Memory requirement for this program is about");\
 sep_add_doc_line("");\
 sep_add_doc_line("            [ns*nxt*nzt+noff*nxo*nzo+4*nr*nzt+5*nxt*nzt+npa*(2*ns*nxt*nzt   ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    +noff*nxo*nzo+4*nxt*nzt)]*4 bytes");\
 sep_add_doc_line("");\
 sep_add_doc_line("       where nr = 1+min(nxt*dxt,0.5*offmax+aperx)/dxo.                      ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    6. Amplitudes are computed using the reference velocity profile, v(z),");\
 sep_add_doc_line("");\
 sep_add_doc_line("       specified by the parameters v0= and dvz=.                            ");\
 sep_add_doc_line("");\
 sep_add_doc_line("AUTHOR");\
 sep_add_doc_line("       Zhenyue Liu, 03/01/95,  Colorado School of Mines ");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/image");\
 sep_add_doc_line("");


MAIN(){

	int 	nt,nzt,nxt,nzo,nxo,ns,noff,nr,is,io,ixo,izo;
	int 	ls,ntr,jtr,ktr,mtr,npv,mtmax;
	ssize_t	nseek;
	float   ft,fzt,fxt,fzo,fxo,fs,off0,dt,dzt,dxt,dzo,dxo,ds,doff,dxm,
		ext,ezt,ezo,exo,es,s,scal;	
	float v0,dvz,fmax,angmax,offmax,rmax,aperx,sx,gx;
	float ***mig,***ttab,**tb,**pb,**cs0b,**angb,**tsum,**tt;
  float *cmp_x, *offset_x,*data;
	float **tvsum=NULL,***mig1=NULL,***cs=NULL,***tv=NULL,
		**cssum=NULL;
  int verb,max_size,ioff,itr;
  float ipct=.001;
  int nwind[3],fwind[3],jwind[3];
  int ntime,ftime,jtime,nh,ih;
	    float as,res;
  
	
  char temp_ch[1024];
  sep3d input,ttfile,output;

   sep_begin_prog();
  init_3d();

  if(0!=init_sep3d_tag("in",&input,"INPUT"))
    seperr("trouble initializing sep3dc structure\n");

  if(0!=strcmp(input.file_format,"GRID"))
    seperr("expecting a grided input \n");

  if(0!=strcmp(input.label[2],"cmp_x"))
    seperr("expecting axis 3 of the grid to be cmp_x \n");

  if(0!=strcmp(input.label[3],"offset_x"))
    seperr("expecting axis 3 of the grid to be offset_x \n");



 	if (0==getch("dxm","f",&dxm))  dxm = input.d[2];

	if  (dxm<0.0000001) seperr("dxm must be positive!\n");

  nt=input.n[0]; dt=input.d[0]; ft=input.o[0];

if(getch("verb","d",&verb)==0) verb=0;

	/* get traveltime tabel parameters	*/
  if(NULL==auxin("ttfile")){
    if(0!=calc_ttfiles(input,verb))
      seperr("trouble calculating traveltimes \n");
  }
  auxclose("ttfile");

  if(0!=init_sep3d_tag("ttfile",&ttfile,"SCRATCH"))
    seperr("trouble initializing sep3dc structure ttfile\n");

   nzt=ttfile.n[0]; fzt=ttfile.o[0]; dzt=ttfile.d[0];
   nxt=ttfile.n[1]; fxt=ttfile.o[1]; dxt=ttfile.d[1];
   ns =ttfile.n[2]; fs =ttfile.o[2]; ds =ttfile.d[2];


	ext = fxt+(nxt-1)*dxt;
	ezt = fzt+(nzt-1)*dzt;
	es = fs+(ns-1)*ds;

	/* optional parameters	*/

  nxo=input.n[2];
  fxo=input.o[2];
  dxo=input.d[2];

	if (0==getch("nzo","d",&nzo)) nzo = (nzt-1)*5+1;
	if (0==getch("fzo","f",&fzo)) fzo = fzt;
	if (0==getch("dzo","f",&dzo)) dzo = dzt*0.2;
	exo = fxo+(nxo-1)*dxo;
	ezo = fzo+(nzo-1)*dzo;
	if(fxt>fxo || ext<exo || fzt>fzo || ezt<ezo) {
    fprintf(stderr,"x %f %f %f %f \n",fxt,fxo,ext,exo);
    fprintf(stderr,"z %f %f %f %f \n",fzt,fzo,ezt,ezo);
		seperr(" migration output range is out of traveltime table!\n");
  }

  if(0==getch("v0","f",&v0)) v0=1500.;
  if(0==getch("dvz","f",&dvz)) dvz=0.;
  if(0==getch("angmax","f",&angmax)) angmax=60.;
	if  (angmax<0.00001) seperr("angmax must be positive!\n");
	mtmax = 2*dxm*sin(angmax*PI/180.)/(v0*dt);
	if(mtmax<1) mtmax = 1;
  if(0==getch("aperx","f",&aperx)) aperx=0.5*nxt*dxt;
  if(0==getch("offmax","f",&offmax)) offmax=3000.;
  if(0==getch("fmax","f",&fmax)) fmax=.25/dt;

  noff=input.n[3]; off0=input.o[3]; doff=input.d[3];

  if(0==getch("ls","d",&ls)) ls=1;

	npv = 0;


if(verb>0){
	fprintf(stderr,"\n");
	fprintf(stderr," Migration parameters\n");
	fprintf(stderr," ================\n");
	fprintf(stderr," \n");
	fprintf(stderr," nzt=%d fzt=%g dzt=%g\n",nzt,fzt,dzt);
	fprintf(stderr," nxt=%d fxt=%g dxt=%g\n",nxt,fxt,dxt);
 	fprintf(stderr," ns=%d fs=%g ds=%g\n",ns,fs,ds);
	fprintf(stderr," \n");
	fprintf(stderr," nzo=%d fzo=%g dzo=%g\n",nzo,fzo,dzo);
	fprintf(stderr," nxo=%d fxo=%g dxo=%g\n",nxo,fxo,dxo);
	fprintf(stderr," \n");
 }
	
	/* compute reference traveltime and slowness  */
	rmax = MAX(es-fxt,ext-fs);
	rmax = MIN(rmax,0.5*offmax+aperx);
	nr = 2+(int)(rmax/dxo);
	tb = ealloc2float(nzt,nr);
	pb = ealloc2float(nzt,nr);
	cs0b = ealloc2float(nzt,nr);
	angb = ealloc2float(nzt,nr);
	timeb(nr,nzt,dxo,dzt,fzt,dvz,v0,tb,pb,cs0b,angb);

if(verb==1){
	fprintf(stderr," nt=%d ft=%g dt=%g \n",nt,ft,dt);
 	fprintf(stderr," dxm=%g fmax=%g\n",dxm,fmax);
 	fprintf(stderr," noff=%d off0=%g doff=%g\n",noff,off0,doff);
	fprintf(stderr," v0=%g dvz=%g \n",v0,dvz);
 	fprintf(stderr," aperx=%g offmax=%g angmax=%g\n",aperx,offmax,angmax);
 	fprintf(stderr," ls=%d npv=%d\n",ls,npv);
	fprintf(stderr," ================\n");
	fflush(stderr);
}


if(0==getch("max_size","d",&max_size)) max_size=100;
max_size=max_size*250000;

if(max_size < nzo*nxo*2 + nzt*nxt*ns + nzt*nxt*2)
  seperr("Memory requirements not sufficient require at least %d MB \n",
    (int)(((float)(nzo*nxo*2 + nzt*nxt*ns + nzt*nxt*2))/250000.));



	/* allocate space */
/*	mig = ealloc3float(nzo,nxo,noff);*/
	mig = ealloc3float(nzo,nxo,1);
	ttab = ealloc3float(nzt,nxt,ns);
	tt = ealloc2float(nzt,nxt);
	tsum = ealloc2float(nzt,nxt);
	mig1 = ealloc3float(1,1,noff);

/* Setup output */
 sep3d_initialize(&output);
 sep3d_axes_allocate(&output,3);
 output.n[0]=nzo; output.d[0]=dzo; output.o[0]=fzo;
 output.n[1]=input.n[2]; output.d[1]=input.d[2]; output.o[1]=input.o[2];
 output.n[2]=input.n[3]; output.d[2]=input.d[3]; output.o[2]=input.o[3];
 output.ntraces=output.n[1]*output.n[2];
 strcpy(output.data_format,"FLOAT");
 strcpy(output.file_format,"REGULAR");
 strcpy(output.usage,"OUTPUT");
 if(0!=sep3dc_write_description("out",&output))
   seperr("trouble writing out description\n");
 hclose();


 	fprintf(stderr," input traveltime tables \n");
                       
	/* compute traveltime residual	*/
	for(is=0; is<ns; ++is){
		nseek = nxt*nzt*is;
    sseek("ttfile",nseek*4,0);
    if(4*nxt*nzt!=sreed("ttfile",ttab[is][0],4*nxt*nzt))
      seperr("trouble reading in traveltime tables \n");
		s = fs+is*ds;
		resit(nxt,fxt,dxt,nzt,nr,dxo,tb,ttab[is],s);
	}

  if(verb) {
	  fprintf(stderr," start migration ... \n");
	  fprintf(stderr," \n");
	  fflush(stderr);
  }
	

	jtr = 1;
	ktr = 0;
  jtr=0;
  ntime=input.n[0]; ftime=0; jtime=1;
  nwind[0]=input.n[1]; fwind[0]=0; jwind[0]=1;
  nwind[1]=input.n[2]; fwind[1]=0; jwind[1]=1;
  nwind[2]=1;          fwind[2]=0; jwind[2]=1;
  for(ioff=0; ioff < input.n[3]; ioff++){
    fwind[2]=ioff;
    if(0!=sep3dc_grab_headers("in",&input,&nh,nwind,fwind,jwind))
      seperr("trouble reading in headers");

   	memset((void *) mig[0][0],(int) '\0',noff*nzo*sizeof(float)); 

  if(nh>0){
    
		 data = ealloc1float(nt*nh);
		 cmp_x = ealloc1float(nh);
		 offset_x = ealloc1float(nh);
     if(0!=sep3dc_grab_key_vals(&input,"cmp_x",cmp_x) ||
        sep3dc_grab_key_vals(&input,"offset_x",offset_x) )
        seperr("trouble grabbing header key values \n");
     if(0!=sep3dc_read_data("in",&input,(char*)data,ntime,ftime,jtime))
       seperr("trouble reading data \n");

     for(ih=0; ih < nh; ih++){
       sx=cmp_x[ih]-offset_x[ih] *0.5;      
       gx=cmp_x[ih]+offset_x[ih] *0.5;      
		/* determine offset index	*/
      io=0; /*where it belongs in the  output*/

	    if(MIN(sx,gx)>=fs && MAX(sx,gx)<=es && 
	       MAX(gx-sx,sx-gx)<=offmax ){
		/*     migrate this trace	*/
	    	as = (sx-fs)/ds;
	    	is = (int)as;
		if(is==ns-1) is=ns-2;
		res = as-is;
		if(res<=0.01) res = 0.0;
		if(res>=0.99) res = 1.0;
		sum2(nxt,nzt,1-res,res,ttab[is],ttab[is+1],tsum);
		
	    	as = (gx-fs)/ds;
	    	is = (int)as;
		if(is==ns-1) is=ns-2;
		res = as-is;
		if(res<=0.01) res = 0.0;
		if(res>=0.99) res = 1.0;
		sum2(nxt,nzt,1-res,res,ttab[is],ttab[is+1],tt);
		sum2(nxt,nzt,1,1,tt,tsum,tsum);
		mig2d(&data[nt*ih],nt,ft,dt,sx,gx,mig[io],aperx,
		  nxo,fxo,dxo,nzo,fzo,dzo,
		  ls,mtmax,dxm,fmax,angmax,
		  tb,pb,cs0b,angb,nr,tsum,nzt,fzt,dzt,nxt,fxt,dxt,
		  npv,cssum,tvsum,mig1[io]);

	        ktr++;
         if(((float)jtr)/((float)input.ntraces) > ipct && verb==1){
			      fprintf(stderr," Finished   %d percent \n",(int)
            ( 100.* (float)(jtr)/((float)input.ntraces)));
            ipct+=.05;
			      fflush(stderr);
	    	}
	    }
	    jtr++;
   }
	  scal = 4/sqrt(PI)*dxm/v0;
    free1float(cmp_x); free1float(offset_x);
    free1float(data);
   
   }
      if(nzo*nxo*4!=srite("out",mig[0][0],4*nzo*nxo))
        seperr("trouble writing out image \n");
}

	free2float(tsum);
	free2float(tt);
	free2float(pb);
	free2float(tb);
	free2float(cs0b);
	free2float(angb);
	free3float(ttab);
	free3float(mig);
	free3float(mig1);
        sep_end_prog();
	return EXIT_SUCCESS;
}

/* residual traveltime calculation based  on reference   time	*/
  void resit(int nx,float fx,float dx,int nz,int nr,float dr,
		float **tb,float **t,float x0)
{
	int ix,iz,jr;
	float xi,ar,sr,sr0;

	for(ix=0; ix<nx; ++ix){
		xi = fx+ix*dx-x0;
		ar = abs(xi)/dr;
		jr = (int)ar;
		sr = ar-jr;
		sr0 = 1.0-sr;
		if(jr>nr-2) jr = nr-2;
		for(iz=0; iz<nz; ++iz)
			t[ix][iz] -= sr0*tb[jr][iz]+sr*tb[jr+1][iz];
	}
} 

/* lateral interpolation	*/

/* sum of two tables	*/
  void sum2(int nx,int nz,float a1,float a2,float **t1,float **t2,float **t)
{
	int ix,iz;

	for(ix=0; ix<nx; ++ix) 
		for(iz=0; iz<nz; ++iz)
			t[ix][iz] = a1*t1[ix][iz]+a2*t2[ix][iz];
}
 
/* compute  reference traveltime and slowness	*/
      void timeb(int nr,int nz,float dr,float dz,float fz,float a,
	float v0,float **t,float **p,float **cs0,float **ang)
{
	int  ir,iz;
	float r,z,v,rc,oa,temp,rou,zc;


	if( a==0.0) {
		for(ir=0,r=0;ir<nr;++ir,r+=dr)
			for(iz=0,z=fz;iz<nz;++iz,z+=dz){
				rou = sqrt(r*r+z*z);
				if(rou<dz) rou = dz;
				t[ir][iz] = rou/v0;
				p[ir][iz] = r/(rou*v0);
				cs0[ir][iz] = z/rou;
				ang[ir][iz] = asin(r/rou);
			}
	} else {
		oa = 1.0/a; 	zc = v0*oa;
		for(ir=0,r=0;ir<nr;++ir,r+=dr)
			for(iz=0,z=fz+zc;iz<nz;++iz,z+=dz){
				rou = sqrt(r*r+z*z);
				v = v0+a*(z-zc);
				if(ir==0){ 
					t[ir][iz] = log(v/v0)*oa;
					p[ir][iz] = 0.0;
					ang[ir][iz] = 0.0;
					cs0[ir][iz] = 1.0;
				} else {
					rc = (r*r+z*z-zc*zc)/(2.0*r);
					rou = sqrt(zc*zc+rc*rc);
					t[ir][iz] = log((v*(rou+rc))
						/(v0*(rou+rc-r)))*oa;
					p[ir][iz] = sqrt(rou*rou-rc*rc)
						/(rou*v0);
					temp = v*p[ir][iz];
					if(temp>1.0) temp = 1.0;
					ang[ir][iz] = asin(temp);
					cs0[ir][iz] = rc/rou;
				}
			}
	}
/*----------------------------------NOT CHANGED END  --------------------------------*/
}

void filt(float *trace,int nt,float dt,float fmax,int ls,int m,float *trf);

  void mig2d(float *trace,int nt,float ft,float dt,
	float sx,float gx,float **mig,float aperx,
  	int nx,float fx,float dx,float nz,float fz,float dz,
	int ls,int mtmax,float dxm,float fmax,float angmax,
	float **tb,float **pb,float **cs0b,float **angb,int nr,
	float **tsum,int nzt,float fzt,float dzt,int nxt,float fxt,float dxt,
	int npv,float **cssum,float **tvsum,float **mig1)
/*****************************************************************************
Migrate one trace 
******************************************************************************
Input:
*trace		one seismic trace 
nt		number of time samples in seismic trace
ft		first time sample of seismic trace
dt		time sampleing interval in seismic trace
sx,gx		lateral coordinates of source and geophone 
aperx		lateral aperature in migration
nx,fx,dx,nz,fz,dz	dimension parameters of migration region
ls		=1 for line source; =0 for point source
mtmax		number of time samples in triangle filter
dxm		midpoint sampling interval
fmax		frequency-highcut for input trace	 
angmax		migration angle aperature from vertical 	 
tb,pb,cs0b,angb		reference traveltime, lateral slowness, cosine of 
		incident angle, and emergent angle
nr		number of lateral samples in reference quantities
tsum		sum of residual traveltimes from shot and receiver
nxt,fxt,dxt,nzt,fzt,dzt		dimension parameters of traveltime table
npv=0		flag of computing quantities for velocity analysis
cssume		sum of cosine of emergence angles from shot and recerver 
tvsum		sum of  traveltime variations from shot and recerver 

Output:
mig		migrated section
mig1		additional migrated section for velocity analysis if npv>0
*****************************************************************************/
{
	int nxf,nxe,nxtf,nxte,ix,iz,iz0,izt0,nzp,jrs,jrg,jz,jt,mt,jx;
	float xm,x,dis,rxz,ar,srs,srg,srs0,srg0,sigp,z0,rdz,ampd,res0,
	      angs,angg,cs0s,cs0g,ax,ax0,pmin,
	      odt=1.0/dt,pd,az,sz,sz0,at,td,res,temp;
	float **tmt,**ampt,**ampti,**ampt1=NULL,*tm,*amp,*ampi,*amp1=NULL,
		*tzt,*trf,*zpt;

	tmt = alloc2float(nzt,nxt);
	ampt = alloc2float(nzt,nxt);
	ampti = alloc2float(nzt,nxt);
	tm = alloc1float(nzt);
	tzt = alloc1float(nzt);
	amp = alloc1float(nzt);
	ampi = alloc1float(nzt);
	zpt = alloc1float(nxt);
	trf = alloc1float(nt+2*mtmax);

	z0 = (fz-fzt)/dzt;
	rdz = dz/dzt;
	pmin = 1.0/(2.0*dxm*fmax);
	
	filt(trace,nt,dt,fmax,ls,mtmax,trf);

	xm = 0.5*(sx+gx);
	rxz = (angmax==90)?0.0:1.0/tan(angmax*PI/180.);
	nxtf = (xm-aperx-fxt)/dxt;
	if(nxtf<0) nxtf = 0;
	nxte = (xm+aperx-fxt)/dxt+1;
	if(nxte>=nxt) nxte = nxt-1;

	/* compute amplitudes and filter length	*/
	for(ix=nxtf; ix<=nxte; ++ix){
		x = fxt+ix*dxt;
		dis = (xm>=x)?xm-x:x-xm;
		izt0 = ((dis-dxt)*rxz-fzt)/dzt-1;
		if(izt0<0) izt0 = 0;
		if(izt0>=nzt) izt0 = nzt-1;

		ar = (sx>=x)?(sx-x)/dx:(x-sx)/dx;
		jrs = (int)ar;
		if(jrs>nr-2) jrs = nr-2;
		srs = ar-jrs;
		srs0 = 1.0-srs;
		ar = (gx>=x)?(gx-x)/dx:(x-gx)/dx;
		jrg = (int)ar;
		if(jrg>nr-2) jrg = nr-2;
		srg = ar-jrg;
		srg0 = 1.0-srg;
		sigp = ((sx-x)*(gx-x)>0)?1.0:-1.0;
		zpt[ix] = fzt+(nzt-1)*dzt;

		for(iz=izt0; iz<nzt; ++iz){
			angs = srs0*angb[jrs][iz]+srs*angb[jrs+1][iz]; 
			angg = srg0*angb[jrg][iz]+srg*angb[jrg+1][iz]; 
			cs0s = srs0*cs0b[jrs][iz]+srs*cs0b[jrs+1][iz]; 
			cs0g = srg0*cs0b[jrg][iz]+srg*cs0b[jrg+1][iz]; 
			ampd = (cs0s+cs0g)*cos(0.5*(angs-angg));
			if(ampd<0.0) ampd = -ampd;
			ampt[ix][iz] = ampd;

			pd = srs0*pb[jrs][iz]+srs*pb[jrs+1][iz]+sigp 
			     *(srg0*pb[jrg][iz]+srg*pb[jrg+1][iz]);
			if(pd<0.0) pd = -pd;
			temp = pd*dxm*odt;
			if(temp<1) temp = 1.0;
			if(temp>mtmax) temp = mtmax;
			ampti[ix][iz] = ampd/(temp*temp);
			tmt[ix][iz] = temp;
			if(pd<pmin && zpt[ix]>fzt+(nzt-1.1)*dzt) 
				zpt[ix] = fzt+iz*dzt;

		}
	}

	nxf = (xm-aperx-fx)/dx+0.5;
	if(nxf<0) nxf = 0;
	nxe = (xm+aperx-fx)/dx+0.5;
	if(nxe>=nx) nxe = nx-1;
	
	/* interpolate amplitudes and filter length along lateral	*/
	for(ix=nxf; ix<=nxe; ++ix){
		x = fx+ix*dx;
		dis = (xm>=x)?xm-x:x-xm;
		izt0 = (dis*rxz-fzt)/dzt;
		if(izt0<0) izt0 = 0;
		if(izt0>=nzt) izt0 = nzt-1;
		iz0 = (dis*rxz-fz)/dz;
		if(iz0<0) iz0 = 0;
		if(iz0>=nz) iz0 = nz-1;

		ax = (x-fxt)/dxt;
		jx = (int)ax;
		ax = ax-jx;
		if(ax<=0.01) ax = 0.;
		if(ax>=0.99) ax = 1.0;
		ax0 = 1.0-ax;
		if(jx>nxte-1) jx = nxte-1;
		if(jx<nxtf) jx = nxtf;

		ar = (sx>=x)?(sx-x)/dx:(x-sx)/dx;
		jrs = (int)ar;
		if(jrs>nr-2) jrs = nr-2;
		srs = ar-jrs;
		srs0 = 1.0-srs;
		ar = (gx>=x)?(gx-x)/dx:(x-gx)/dx;
		jrg = (int)ar;
		if(jrg>nr-2) jrg = nr-2;
		srg = ar-jrg;
		srg0 = 1.0-srg;

		for(iz=izt0; iz<nzt; ++iz){
		    tzt[iz] = ax0*tsum[jx][iz]+ax*tsum[jx+1][iz]
				+srs0*tb[jrs][iz]+srs*tb[jrs+1][iz]
				+srg0*tb[jrg][iz]+srg*tb[jrg+1][iz];

		    amp[iz] = ax0*ampt[jx][iz]+ax*ampt[jx+1][iz];
		    ampi[iz] = ax0*ampti[jx][iz]+ax*ampti[jx+1][iz];
		    tm[iz] = ax0*tmt[jx][iz]+ax*tmt[jx+1][iz];


		}

		nzp = (ax0*zpt[jx]+ax*zpt[jx+1]-fz)/dz+1.5;
		if(nzp<iz0) nzp = iz0;
		if(nzp>nz) nzp = nz;

		/* interpolate along depth if operater aliasing 	*/
		for(iz=iz0; iz<nzp; ++iz) {
			az = z0+iz*rdz;
			jz = (int)az;
			if(jz>=nzt-1) jz = nzt-2;
			sz = az-jz;
			sz0 = 1.0-sz;
			td = sz0*tzt[jz]+sz*tzt[jz+1];
			at = (td-ft)*odt+mtmax;
			jt = (int)at;
			if(jt > mtmax && jt < nt+mtmax-1){
			    ampd = sz0*ampi[jz]+sz*ampi[jz+1];
			    mt = (int)(0.5+sz0*tm[jz]+sz*tm[jz+1]);
			    res = at-jt;
			    res0 = 1.0-res;
 			    temp = (res0*(-trf[jt-mt]+2.0*trf[jt]-trf[jt+mt]) 
				+res*(-trf[jt-mt+1]+2.0*trf[jt+1]
				-trf[jt+mt+1]))*ampd;
			    mig[ix][iz] += temp;

			}
		}

		/* interpolate along depth if not operater aliasing 	*/
		for(iz=nzp; iz<nz; ++iz) {
			az = z0+iz*rdz;
			jz = (int)az;
			if(jz>=nzt-1) jz = nzt-2;
			sz = az-jz;
			sz0 = 1.0-sz;
			td = sz0*tzt[jz]+sz*tzt[jz+1];
			at = (td-ft)*odt;
			jt = (int)at;
			if(jt > 0 && jt < nt-1){
			    ampd = sz0*amp[jz]+sz*amp[jz+1];
			    res = at-jt;
			    res0 = 1.0-res;
 			    temp = (res0*trace[jt]+res*trace[jt+1])*ampd; 
			    mig[ix][iz] += temp;
			}
		}

	}

	free2float(ampt);
	free2float(ampti);
	free2float(tmt);
	free1float(amp);
	free1float(ampi);
	free1float(zpt);
	free1float(tm);
	free1float(tzt);
	free1float(trf);
}

/* Low-pass filter, integration and phase shift for input data	 
   input: 
    trace(nt)	single seismic trace
   fmax	high cut frequency
    ls		ls=1, line source; ls=0, point source
  output:
    trace(nt) 	filtered and phase-shifted seismic trace 
    tracei(nt) 	filtered, integrated and phase-shifted seismic trace 
 */
void filt(float *trace,int nt,float dt,float fmax,int ls,int m,float *trf)
{
	static int nfft=0, itaper, nw, nwf;
	static float *taper, *amp, *ampi, dw;
	int  it, iw, itemp;



	float temp, ftaper, const2, *rt;



	complex *ct;

	fmax *= 2.0*PI;
	ftaper = 0.1*fmax;
	const2 = sqrt(2.0);

	if(nfft==0) {
        	/* Set up FFT parameters */
        	nfft = npfaro(nt+m, 2 * (nt+m));
        	if (nfft >= SU_NFLTS || nfft >= 720720)
                	seperr("Padded nt=%d -- too big", nfft);

        	nw = nfft/2 + 1;
		dw = 2.0*PI/(nfft*dt);

		itaper = 0.5+ftaper/dw;
		taper = ealloc1float(2*itaper+1);
		for(iw=-itaper; iw<=itaper; ++iw){
			temp = (float)iw/(1.0+itaper); 
			taper[iw+itaper] = (1-temp)*(1-temp)*(temp+2)/4;
		}

		nwf = 0.5+fmax/dw;
		if(nwf>nw-itaper-1) nwf = nw-itaper-1;
		amp = ealloc1float(nwf+itaper+1);
		ampi = ealloc1float(nwf+itaper+1);
		amp[0] = ampi[0] = 0.;
		for(iw=1; iw<=nwf+itaper; ++iw){
			amp[iw] = sqrt(dw*iw)/nfft;
			ampi[iw] = 0.5/(1-cos(iw*dw*dt));
		}
	}

        /* Allocate fft arrays */
        rt   = ealloc1float(nfft);
        ct   = ealloc1complex(nw);

        memcpy(rt, trace, nt*sizeof(float));
        memset((void *) (rt + nt), (int) '\0', (nfft-nt)*sizeof(float)); 
        pfarc(1, nfft, rt, (float*)ct);

	for(iw=nwf-itaper;iw<=nwf+itaper;++iw){
		itemp = iw-(nwf-itaper);
		ct[iw].re = taper[itemp]*ct[iw].re; 
		ct[iw].im = taper[itemp]*ct[iw].im; 
	}
	for(iw=nwf+itaper+1;iw<nw;++iw){
		ct[iw].re = 0.; 
		ct[iw].im = 0.; 
	}

       	if(!ls){
		for(iw=0; iw<=nwf+itaper; ++iw){
			/* phase shifts PI/4 	*/
			temp = (ct[iw].re-ct[iw].im)*amp[iw]*const2;
			ct[iw].im = (ct[iw].re+ct[iw].im)*amp[iw]*const2;
			ct[iw].re = temp;
		    }
	} else {
		for(iw=0; iw<=nwf+itaper; ++iw){
			ct[iw].im = ct[iw].im*amp[iw];
			ct[iw].re = ct[iw].re*amp[iw];
		}
	}              
        pfacr(-1, nfft, ct, rt);
		
        /* Load traces back in */
	for (it=0; it<nt; ++it) trace[it] = rt[it];

        /* Integrate traces   */
	for(iw=0; iw<=nwf+itaper; ++iw){
		ct[iw].im = ct[iw].im*ampi[iw];
		ct[iw].re = ct[iw].re*ampi[iw];
	}
        pfacr(-1, nfft, ct, rt);
        for (it=0; it<m; ++it)  trf[it] = rt[nfft-m+it];
        for (it=0; it<nt+m; ++it)  trf[it+m] = rt[it];

	free1float(rt);
	free1complex(ct);
}
int  calc_ttfiles(sep3d input,int verb){
sep3d vel;
int nzo,i;
float fzo,dzo;
int nt1,nt2,nt3;
float dt1,dt2,dt3;
float ot1,ot2,ot3;
int if1,ie1,if2,ie2,ib,is,f[2],j[2],n[2],ndim,esize,ix;
int jprint;
float *velb,*tt,x;

auxclose("ttfile");

if(NULL==auxin("velfile")) 
  return(sepwarn(-1,"Trouble opening up velfile \n"));
  
auxclose("velfile");
if(0!=init_sep3d_tag("velfile",&vel,"INPUT"))
    return(sepwarn(-1,"trouble initializing velfile structure\n"));

if(vel.ndims!=2)
    return(sepwarn(-1,"velfile should be 2 dimensions\n"));

if(0==getch("nzo","d",&nzo) || 0==getch("fzo","f",&fzo) ||
  0==getch("dzo","f",&dzo)) 
    return(sepwarn(-1,"must provide nzo,fzo, and dzo \n"));
if(vel.o[0] > fzo || vel.o[0] + vel.d[0] *(vel.n[0]-1) <
   fzo+dzo*(nzo-1))
    return(sepwarn(-1,"output space beyond velocity depth range  \n"));
if(vel.o[1] > input.o[2] || vel.o[1] + vel.d[1] *(vel.n[1]-1) <
   input.o[2]+input.d[2]*(input.n[2]-1))
    return(sepwarn(-1,"output space beyond velocity  x range  \n"));

auxout("ttfile");

dt1=vel.d[0]*5; nt1=vel.n[0]*.2; ot1=vel.o[0];
dt2=vel.d[1]*2; nt2=vel.n[1]*.5; ot2=vel.o[1];
ot3=input.o[2]; dt3=input.d[2]*4.; nt3=input.n[2]/4.;

i=1;  sep_put_data_axis_par("ttfile",&i,&nt1,&ot1,&dt1,"Depth");
i=2;  sep_put_data_axis_par("ttfile",&i,&nt2,&ot2,&dt2,"Depth");
i=3;  sep_put_data_axis_par("ttfile",&i,&nt3,&ot3,&dt3,"Depth");


velb=(float*)malloc(sizeof(float)*nt1*nt2);
tt=(float*)malloc(sizeof(float)*nt1*nt2);
f[0]=0;f[1]=0;
j[0]=5;j[1]=2;
n[0]=nt1;n[1]=nt2;
ndim=2; esize=4;

if(0!=sreed_window("velfile",&ndim,vel.n,n,f,j,esize,velb))
    return(sepwarn(-1,"trouble reading in vel  \n"));

jprint=MAX(1,nt3/100);

for(i=0; i < nt1*nt2; i++) velb[i]=1./velb[i];

for(i=0; i < nt3; i++){
  if(verb==1 && i%jprint==0) 
     fprintf(stderr,"travel times calculated at  %d of %d \n",i,nt3);
   x=ot3+dt3*i;
   ix=(x-ot2)/dt2+.5;
   fastmarch(2,0.,x-ot2,0.,1,1,1,nt1,nt2,1,dt1,dt2,.0001,
     1./velb[nt1*ix],velb,tt);
   srite("ttfile",tt,nt1*nt2*4);
}
    


free(tt);free(velb);


}


