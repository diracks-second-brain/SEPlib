/*$

=head1 NAME

Interp - Interpolate dataset using sinc, linearm or nearest neighbor

=head1 SYNOPSIS

Interp < in.H o1 pars > out.H

=head1 INPUT PARAMETERS

=over 4

=item lsinc - integer 

	[10]: length of interpolation operator 
      (recommend < 20; MUST BE EVEN)

=item type  - int     

	 [2]: Type of interpolation,0 nearest neighbor,1-linear,2-sinc

=item o1out - float   

	[o1]: First sample on axis1

=item o2out - float   

	[o2]: First sample on axis2

=item o3out - float   

	[o3]: First sample on axis3

=item d1out - float   

	[d1]: Sampling of the output axis 1

=item d2out - float   

	[d2]: Sampling of the output axis 2

=item d3out - float   

	[d3]: Sampling of the output axis 3

=item n1out - int     

	[max in/dout]: Number of samples in axis 1

=item n2out - int     

	[max in/dout]: Number of samples in axis 2

=item n3out - int     

	[max in/dout]: Number of samples in axis 3

=item maxsize-int     

	[20]:  Amount of memory to use in megabytes

=item ntab  - int     

	[101]: Interpolation table size (aka if outspace
      corresponds to inspace .012 .01 table will be chosen)

=back

=head1 DESCRIPTION

Interpolate dataset using sinc, linear, or nearest neighbor,
up to 3 dimensions(if it can be held in memory). If any of the
n1out,n2out,n3out, o1out,o2out,o3out or d1out,d2out,d3out is
omitted the corresponding value in the input data is used.
 
=head1 EXAMPLE

	Interp < in.H lsinc=12 type=2 > out.H
	conputes a 12-point sinc-interpolator on the input data.
	All of the standard n's, d's and o's are taken from the
	input data

=head1 CATEGORY

B<seis/filter>

=cut

>*/
/*
D. Rothman, 27Nov83
 * Keyword: sinc-interpolation
 *
 * Modified: May 26, 1998 - Bob - Changed to direct reference
 *                          to sinc rather than through psinc because
 *                          of 64Bit SGI
 * Modified: Aug 1, 1999   Bob Conbined with Interp2, extended options,
 *                          etc
 *
 */

#include <stdlib.h>
#include <sep3d.h>
#include <sepfilter.h>

#define YES 1
#define NO 0
#define NEAREST 0
#define LINEAR 1
#define SINC 2

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Interp - Interpolate dataset using sinc, linearm or nearest neighbor");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Interp < in.H o1 pars > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    lsinc - integer");\
 sep_add_doc_line("                [10]: length of interpolation operator ");\
 sep_add_doc_line("              (recommend < 20; MUST BE EVEN)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    type - int");\
 sep_add_doc_line("                 [2]: Type of interpolation,0 nearest neighbor,1-linear,2-sinc");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1out - float");\
 sep_add_doc_line("                [o1]: First sample on axis1");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o2out - float");\
 sep_add_doc_line("                [o2]: First sample on axis2");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o3out - float");\
 sep_add_doc_line("                [o3]: First sample on axis3");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1out - float");\
 sep_add_doc_line("                [d1]: Sampling of the output axis 1");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d2out - float");\
 sep_add_doc_line("                [d2]: Sampling of the output axis 2");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d3out - float");\
 sep_add_doc_line("                [d3]: Sampling of the output axis 3");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n1out - int");\
 sep_add_doc_line("                [max in/dout]: Number of samples in axis 1");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n2out - int");\
 sep_add_doc_line("                [max in/dout]: Number of samples in axis 2");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n3out - int");\
 sep_add_doc_line("                [max in/dout]: Number of samples in axis 3");\
 sep_add_doc_line("");\
 sep_add_doc_line("    maxsize-int");\
 sep_add_doc_line("                [20]:  Amount of memory to use in megabytes");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ntab - int");\
 sep_add_doc_line("                [101]: Interpolation table size (aka if outspace");\
 sep_add_doc_line("              corresponds to inspace .012 .01 table will be chosen)");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Interpolate dataset using sinc, linear, or nearest neighbor, up to 3");\
 sep_add_doc_line("    dimensions(if it can be held in memory). If any of the");\
 sep_add_doc_line("    n1out,n2out,n3out, o1out,o2out,o3out or d1out,d2out,d3out is omitted the");\
 sep_add_doc_line("    corresponding value in the input data is used.");\
 sep_add_doc_line("");\
 sep_add_doc_line("EXAMPLE");\
 sep_add_doc_line("            Interp < in.H lsinc=12 type=2 > out.H");\
 sep_add_doc_line("            conputes a 12-point sinc-interpolator on the input data.");\
 sep_add_doc_line("            All of the standard n's, d's and o's are taken from the");\
 sep_add_doc_line("            input data");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/filter");\
 sep_add_doc_line("");
#include <sep.main>
MAIN()
{
	int nt=0, nx=0,  lsinc, nintp, nsint, ntout, i, ix, itint;
	int it, it0, j, itc,ndim ;
	float dt=0., dtout=0., *data, *idata, *sinc, **psinc, *space;
	char temp_ch[256];
	int n,n1,n2,n3,n1out,n2out,n3out,nbig;
	float o,d,d1out,d2out,d3out,o1out,o2out,o3out,o1,o2,o3,d1,d2,d3;
	int type,do_axis[3],ntab,verb,maxsize;
	int *iaxis1,*iaxis2,*iaxis3;
	int *spt1,*spt2,*spt3;
	float *faxis1,*faxis2,*faxis3;
	int nlen1,nlen2,nlen3;
	float *sinc_table,dsinc;
	float *input,*output;
	int in_size,out_size;
	int in_loc,out_loc;
	int n1_lin,n2_lin,n3_lin;
	int n1_lout,n2_lout,n3_lout;
	int a,b,c,esize,nmax,unit;
	int loc1,loc2,loc3;
	int iloc1,iloc2,iloc3;
	float f1,f2,f3,loc,val;
	int i1,i2,i3,i4;
	int b3,b2,b1,e3,e2,e1;

         sep_begin_prog();
	
	/* information about input data set */
	if(hetch("esize","d",&esize)==0) esize=4;
	if(esize!=4) seperr("Can only deal with float data \n");
	if(0!=sep_get_number_data_axes("in",&ndim))
		seperr("trouble getting number of data axes\n");
	
	nbig=1;
	o2=0;o3=0;d2=1;d3=1;n2=1;n3=1;
	for(i=1;i<=ndim;i++){
		if(0!=sep_get_data_axis_par("in",&i,&n,&o,&d,temp_ch))
			seperr("trouble grabbing data axes %d \n",i);
		if(i==1){ n1=n;o1=o; d1=d;}
		else if(i==2){ n2=n;o2=o; d2=d;}
		else if(i==3){ n3=n;o3=o; d3=d;}
		else nbig=nbig*n;
	}

	/* get interpolation parameters */
	if(0==getch("type","d",&type)) type=SINC;
	if(type>2 || type<0) seperr("Invalid type (0-nint 1-linear 2-sync)\n");
	do_axis[0]=NO; do_axis[1]=NO; do_axis[2]=NO;
	if(0==getch("o1out","f",&o1out)) o1out=o1;else do_axis[0]=YES;
	if(0==getch("d1out","f",&d1out)) d1out=d1;else do_axis[0]=YES;
	if(0==getch("o2out","f",&o2out)) o2out=o2;else do_axis[1]=YES;
	if(0==getch("d2out","f",&d2out)) d2out=d2;else do_axis[1]=YES;
	if(0==getch("o3out","f",&o3out)) o3out=o3;else do_axis[2]=YES;
	if(0==getch("d3out","f",&d3out)) d3out=d3;else do_axis[2]=YES;
	if(0==getch("n1out","d",&n1out)) n1out=((o1+d1*(n1-1)-o1out)/d1out+1);
	else do_axis[0]=YES;
	if(0==getch("n2out","d",&n2out)) n2out=((o2+d2*(n2-1)-o2out)/d2out+1);
	else do_axis[1]=YES;
	if(0==getch("n3out","d",&n3out)) n3out=((o3+d3*(n3-1)-o3out)/d3out+1);
	else do_axis[2]=YES;
	if(0==getch("lsinc","d",&lsinc)) lsinc=10;
	if(0==getch("ntab","d",&ntab)) ntab=101;
	if(0==getch("verb","d",&verb)) verb=0;
	if(0==getch("maxsize","d",&maxsize)) maxsize=20;
	maxsize=maxsize*1000000;
	
	/*take care of the output space */
	if(do_axis[0]==NO && do_axis[1]==NO && do_axis[2]==NO)	
		seperr("You aren't interpolating along any axis \n");
	if(type==NEAREST) strcpy(temp_ch,"nearest neighbor");
	else if(type==LINEAR) strcpy(temp_ch,"linear interpolation");
	else sprintf(temp_ch,"sinc interpolation with %d points",lsinc);
	putlin(temp_ch);
	if(verb==1) fprintf(stderr,"%s\n",temp_ch);
	if(do_axis[0]==YES){
		sprintf(temp_ch,"n1=%d o1=%f d1=%f",n1out,o1out,d1out);
		putlin(temp_ch);if(verb==1) fprintf(stderr,"%s\n",temp_ch);
	}
	if(do_axis[1]==YES){
		sprintf(temp_ch,"n2=%d o2=%f d2=%f",n2out,o2out,d2out);
		putlin(temp_ch);if(verb==1) fprintf(stderr,"%s\n",temp_ch);
	}
	if(do_axis[2]==YES){
		sprintf(temp_ch,"n3=%d o3=%f d3=%f",n3out,o3out,d3out);
		putlin(temp_ch);if(verb==1) fprintf(stderr,"%s\n",temp_ch);
	}
		
	if(do_axis[2]==NO){ 
		nbig=n3*nbig;
		nmax=2; 
		if(do_axis[1]==NO){
			nbig=nbig*n2; nmax=1; in_size=n1;out_size=n1out;
			n2_lin=1; n3_lin=1;
			n2_lout=1; n3_lout=1;
		}
		else{
			in_size=(n1*n2);out_size=(n1out*n2out); 
			n3_lin=1; n2_lin=n2;
			n3_lout=1; n2_lout=n2out;
		}
	}	
	else{ 
		in_size=(n1*n2*n3);out_size=n1out*n2out*n3out;
		n3_lout=n3out; n2_lout=n2out;
		n3_lin=n3; n2_lin=n2;
	}

	n1_lout=n1out;
	n1_lin=n1;

	unit=in_size+out_size;
	if(unit*esize  > maxsize) 
		seperr("maxsize is not big enough, must be %d with current pars\n",
     unit*esize/1000/1000);
			
	/*NOW LETS ALLOCATE THE TABLES*/
	iaxis1=(int*) malloc(n1out*sizeof(int));
	iaxis2=(int*) malloc(n2out*sizeof(int));
	iaxis3=(int*) malloc(n3out*sizeof(int));
	if(type==LINEAR){
		faxis1=(float*) malloc(n1out*sizeof(float));
		faxis2=(float*) malloc(n2out*sizeof(float));
		faxis3=(float*) malloc(n3out*sizeof(float));
		if(do_axis[0]==YES)nlen1=2; else nlen1=1;	
		if(do_axis[1]==YES)nlen2=2; else nlen2=1;	
		if(do_axis[2]==YES)nlen3=2; else nlen3=1;	
	}
	else if(type==SINC){
		spt1=(int*) malloc(n1out*sizeof(int));
		spt2=(int*) malloc(n2out*sizeof(int));
		spt3=(int*) malloc(n3out*sizeof(int));
		sinc_table=(float*) malloc(lsinc*sizeof(float)*ntab);
		dsinc=1./(ntab-1);
		if(do_axis[0]==YES)nlen1=lsinc; else nlen1=1;	
		if(do_axis[1]==YES)nlen2=lsinc; else nlen2=1;	
		if(do_axis[2]==YES)nlen3=lsinc; else nlen3=1;	
	}
	else{ nlen1=1; nlen2=1; nlen3=1;}


	/* DO POINTERS FOR AXIS 1 */
	for(i=0; i< n1out; i++){
		loc=((o1out+d1out*i)-o1)/d1;
		iaxis1[i]=loc;
		if(do_axis[0]==YES){
	 		if(type==LINEAR) faxis1[i]=loc-iaxis1[i];
			else if(type==SINC) spt1[i]=((loc-iaxis1[i])/dsinc)+.5;
		}
		else{
	 		if(type==LINEAR) faxis1[i]=0.;
			else if(type==SINC) spt1[i]=1;
		}
	}

	/* DO POINTERS FOR AXIS 2 */
	for(i=0; i< n2out; i++){
		loc=((o2out+d2out*i)-o2)/d2;
		iaxis2[i]=loc;
		if(do_axis[1]==YES){
	 		if(type==LINEAR) faxis2[i]=loc-iaxis2[i];
			else if(type==SINC) spt2[i]=((loc-iaxis2[i])/(dsinc))+.5;
		}
		else{
	 		if(type==LINEAR) faxis2[i]=0.;
			else if(type==SINC) spt2[i]=1;
		}
	}

	/* DO POINTERS FOR AXIS 3 */
	for(i=0; i< n3out; i++){
		loc=((o3out+d3out*i)-o3)/d3;
		iaxis3[i]=loc;
		if(do_axis[2]==YES){
	 		if(type==LINEAR) faxis3[i]=loc-iaxis3[i];
			else if(type==SINC) spt3[i]=((loc-iaxis3[i])/(dsinc))+.5;
		}
		else{
	 		if(type==LINEAR) faxis3[i]=0.;
			else if(type==SINC) spt3[i]=1;
		}
	}

	if(verb) fprintf(stderr,"Finished constructing pointers \n");

	if(type==SINC){
		space = (float *) alloc ( lsinc * 3 * sizeof(float) );
		/*first deal with possible boundary problem*/
		for(i=0;i<n1out;i++)if(spt1[i]>=ntab ||  spt1[i]<0) spt1[i]=0;
		for(i=0;i<n2out;i++)if(spt2[i]>=ntab ||  spt2[i]<0) spt2[i]=0;
		for(i=0;i<n3out;i++)if(spt3[i]>=ntab ||  spt3[i]<0) spt3[i]=0;
		/* contruct sinc table */
		for(i=0; i< ntab; i++){
			d=i*dsinc;
			mksinc( &sinc_table[i*lsinc], lsinc, d, space);
		}
		if(verb) 
			fprintf(stderr,"finished constructing sinc table of size %d by %d\n",
				lsinc,ntab);
	}

	/* close history file */
	hclose();	

	/*now it is time to get to work doing interpolation */
	input=(float*)malloc(in_size*sizeof(float));
	output=(float*)malloc(out_size*sizeof(float));

	for(i4=0; i4 < nbig; i4++){
		if(in_size*esize!=sreed("in",input,in_size*esize))
			seperr("trouble reading in data \n");

		/*NEAREST NEIGHBOR CASE */
		if(type==NEAREST){
			for(i3=0; i3 < n3_lout; i3++){
				iloc3=MIN(MAX(iaxis3[i3]+1,0),n3_lin-1);
				for(i2=0; i2 < n2_lout; i2++){
					iloc2=MIN(MAX(iaxis2[i2]+1,0),n2_lin-1);
					for(i1=0; i1 < n1_lout; i1++){
						iloc1=MIN(MAX(iaxis1[i1]+1,0),n1_lin-1);
						output[i1+n1_lout*i2+i3*n1_lout*n2_lout]=
							input[iloc1+n1_lin*iloc2+iloc3*n1_lin*n2_lin];
					}
				}
			}
		}
		else if(type==LINEAR){
			for(i3=0; i3 < n3_lout; i3++){
				b3=MAX(MIN(iaxis3[i3],n3_lin-1),0);
				e3=MIN(MAX(iaxis3[i3]+1,0),n3_lin-1);
				for(i2=0; i2 < n2_lout; i2++){
					b2=MAX(MIN(iaxis2[i2],n2_lin-1),0);
					e2=MIN(MAX(iaxis2[i2]+1,0),n2_lin-1);
					for(i1=0; i1 < n1_lout; i1++){
						b1=MAX(MIN(iaxis1[i1],n1_lin-1),0);
						e1=MIN(MAX(iaxis1[i1]+1,0),n1_lin-1);
						output[i1+n1_lout*i2+i3*n1_lout*n2_lout]=
							(1.-faxis1[i1])*(1-faxis2[i2])*(1.-faxis3[i3])*
							input[b1+b2*n1_lin+b3*n1_lin*n2_lin]+
							(faxis1[i1])*(1-faxis2[i2])*(1.-faxis3[i3])*
							input[e1+b2*n1_lin+b3*n1_lin*n2_lin]+
							(1.-faxis1[i1])*(faxis2[i2])*(1.-faxis3[i3])*
							input[b1+e2*n1_lin+b3*n1_lin*n2_lin]+
							(faxis1[i1])*(faxis2[i2])*(1.-faxis3[i3])*
							input[e1+e2*n1_lin+b3*n1_lin*n2_lin]+
							(1.-faxis1[i1])*(1.-faxis2[i2])*(faxis3[i3])*
							input[b1+b2*n1_lin+e3*n1_lin*n2_lin]+
							(faxis1[i1])*(1.-faxis2[i2])*(faxis3[i3])*
							input[e1+b2*n1_lin+e3*n1_lin*n2_lin]+
							(1.-faxis1[i1])*(faxis2[i2])*(faxis3[i3])*
							input[b1+e2*n1_lin+e3*n1_lin*n2_lin]+
							(faxis1[i1])*(faxis2[i2])*(faxis3[i3])*
							input[e1+e2*n1_lin+e3*n1_lin*n2_lin];
					}
				}
			}
		}
		else{  /* sinc interpolation */
			for(i3=0; i3 < n3_lout; i3++){
				for(i2=0; i2 < n2_lout; i2++){
					for(i1=0; i1 < n1_lout; i1++){
						val=0.;
						for(c=0; c<nlen3; c++){
							if(nlen3==1){ 
								f3=1; loc3=i3;
							}
							else{
								loc3=iaxis3[i3]+c-lsinc/2+1;
								loc3=MIN(MAX(loc3,0),n3_lin-1);
								f3=sinc_table[spt3[i3]*lsinc+c];
							}
							for(b=0; b<nlen2; b++){
								if(nlen2==1){ 
									f2=1.;
									loc2=i2;
								}
								else{
									f2=sinc_table[spt2[i2]*lsinc+b];
									loc2=iaxis2[i2]+b-lsinc/2+1;
									loc2=MIN(MAX(loc2,0),n2_lin-1);
								}
/*								fprintf(stderr,"check (%d %d) %d %f %d\n",
									i1,i2,loc2,f2,spt2[i2]);*/
								for(a=0; a<nlen1; a++){
									if(nlen1==1){ 
										f1=1.;
										loc1=i1;
									}
									else{ 
										f1=sinc_table[spt1[i1]*lsinc+a];
										loc1=iaxis1[i1]+a-lsinc/2+1;
										loc1=MIN(MAX(loc1,0),n1_lin-1);
									}
/*									fprintf(stderr,"check me %d %d %d %f %f %f\n",
										i1,i2,i3,f1,f2,f3);*/
									val+=f1*f2*f3*input[loc1+loc2*n1_lin+
										loc3*n2_lin*n1_lin];
								}
							}
						}
						output[i1+i2*n1_lout+i3*n1_lout*n2_lout]=val;
					}
				}
			}
		}

		if(out_size*esize!=srite("out",output,out_size*esize))
			seperr("troube writing out data data \n");
	}

 sep_end_prog();
 return(0);
}

