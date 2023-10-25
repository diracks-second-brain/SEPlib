/*$

=head1 NAME

Spike - make delta functions and impulsive plane waves.


=head1 SYNOPSIS

Spike [n1= n2= n3= k1= k2= k3= mag=1 d1= d2= d3= nsp=1] > out.H

=head1  INPUT PARAMETERS

=over 4

=item n1,n2,n3,n4,n5,n6,n7   -   integer  

      output cube dimensions.

=item k1,k2,k3,k4,k5   -   integer  

      specify Fortran index of location of delta function
      If a kN is absent, the delta function
      becomes a constant function in the N-th dimension.
      If any kN is -1, no spike will be produced.

=item mag        -   real

      real     [1.] determines the magnitude of the spike.
      Crossing spikes add their magnitudes.

=item nsp        -   integer  

      [1] number of spikes (maximum 15)

=item title      -   char*    

       ' '  get putched into output header file .

=item o1,o2,o3,o4,o5   -   real     
       
      [0.] standard seplib

=item d1,d2,d3,d4,d5   -   real     

      [.004],[.1],[.1] standard seplib parameters

=item label1..3  -   char*    

       ['sec'],['km],['km']  standard seplib meaning

=back

=head1 DESCRIPTION

Make delta functions and impulsive plane waves.

=head1 COMMENTS

Spike n1=8 n2=6 n3=4 nsp=3 k1=6,3,7 k2=5,0,3 k3=1 | Disfil
will put a spike at (6,5,1), a plane with n1=3, and n2,n3
variable and a line with n1=7, n2=3, n3 variable).

=head1 SEE ALSO

L<Gauss>, L<Vel>

=head1 CATEGORY

B<seis/model>

=cut
>*/
/*
 Author - JFC
 Revised - stew 7/31/86  changed some getch's to getch's
 Revised - jos  9/12/86  multiple spikes
 Revised - joe  4/1/89   Bumped up MAXNT and MAXSP
       (The program should be rewritten to allocate
       on the fly, but I don't know how to do
       that in FORTRAN.)
       Documented that 'k1=-1' is indeed legal and safe.
       (I use it all the time!)
       Added 'mag' option.
   JFC 8-12-89   getch and putch title.
 Revised - Ray Abma 25 March 1992
              Bumped up MAXNT to match the old program.
 Revised - Ray Abma 2 August 1993
              Made crossing events add instead of multiply.
              Dynamicly allocated input and spike arrays.
	Revised Bob Sept 12, 1999 Rewritten in C (g77 and allocation trick is gone)
*/


#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Spike - make delta functions and impulsive plane waves.");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Spike [n1= n2= n3= k1= k2= k3= mag=1 d1= d2= d3= nsp=1] > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    n1,n2,n3,n4,n5,n6,n7 - integer");\
 sep_add_doc_line("              output cube dimensions.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    k1,k2,k3,k4,k5 - integer");\
 sep_add_doc_line("              specify Fortran index of location of delta function");\
 sep_add_doc_line("              If a kN is absent, the delta function");\
 sep_add_doc_line("              becomes a constant function in the N-th dimension.");\
 sep_add_doc_line("              If any kN is -1, no spike will be produced.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    mag - real");\
 sep_add_doc_line("              real     [1.] determines the magnitude of the spike.");\
 sep_add_doc_line("              Crossing spikes add their magnitudes.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nsp - integer");\
 sep_add_doc_line("              [1] number of spikes (maximum 15)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    title - char*");\
 sep_add_doc_line("               ' '  get putched into output header file .");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1,o2,o3,o4,o5 - real");\
 sep_add_doc_line("              [0.] standard seplib");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1,d2,d3,d4,d5 - real");\
 sep_add_doc_line("              [.004],[.1],[.1] standard seplib parameters");\
 sep_add_doc_line("");\
 sep_add_doc_line("    label1..3 - char*");\
 sep_add_doc_line("               ['sec'],['km],['km']  standard seplib meaning");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Make delta functions and impulsive plane waves.");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("    Spike n1=8 n2=6 n3=4 nsp=3 k1=6,3,7 k2=5,0,3 k3=1 | Disfil will put a");\
 sep_add_doc_line("    spike at (6,5,1), a plane with n1=3, and n2,n3 variable and a line with");\
 sep_add_doc_line("    n1=7, n2=3, n3 variable).");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Gauss, Vel");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/model");\
 sep_add_doc_line("");


#include <sep.main> 

MAIN()
{ 

  int n1,n2,n3,n4,n5,n6,n7,i6;
	float o1,o2,o3,o4,o5,o6,o7;
	float d1,d2,d3,d4,d5,d6,d7;
	int esize;
	char temp_ch[128];
  float *array;
	int *k1,*k2,*k3,*k4,*k5;
	int nsp;
	float *mag;
	int i1,i2,i3,ierr,isp,i4,i5;

        sep_begin_prog();
	/*n parameters */
	if(0==getch("n1","d",&n1)) seperr("must supply n1 \n"); putch("n1","d",&n1);
	if(0==getch("n2","d",&n2)) n2=1; putch("n2","d",&n2);
	if(0==getch("n3","d",&n3)) n3=1; putch("n3","d",&n3);
	if(0==getch("n4","d",&n4)) n4=1; putch("n4","d",&n4);
	if(0==getch("n5","d",&n5)) n5=1; putch("n5","d",&n5);
	if(0==getch("n6","d",&n6)) n6=1; putch("n6","d",&n6);
	if(0==getch("n7","d",&n7)) n7=1; putch("n7","d",&n7);

	/*o parameters */
	if(0==getch("o1","f",&o1)) o1=0.; putch("o1","f",&o1);
	if(0==getch("o2","f",&o2)) o2=0.; putch("o2","f",&o2);
	if(0==getch("o3","f",&o3)) o3=0.; putch("o3","f",&o3);
	if(0==getch("o4","f",&o4)) o4=0.; putch("o4","f",&o4);
	if(0==getch("o5","f",&o5)) o5=0.; putch("o5","f",&o5);
	if(0==getch("o6","f",&o6)) o6=0.; putch("o6","f",&o6);
	if(0==getch("o7","f",&o7)) o7=0.; putch("o7","f",&o7);
	
	/*d parameters */
	if(0==getch("d1","f",&d1)) d1=0.004; putch("d1","f",&d1);
	if(0==getch("d2","f",&d2)) d2=.1; putch("d2","f",&d2);
	if(0==getch("d3","f",&d3)) d3=.1; putch("d3","f",&d3);
	if(0==getch("d4","f",&d4)) d4=.1; putch("d4","f",&d4);
	if(0==getch("d5","f",&d5)) d5=.1; putch("d5","f",&d5);
	if(0==getch("d6","f",&d6)) d6=.1; putch("d6","f",&d6);
	if(0==getch("d7","f",&d7)) d7=.1; putch("d7","f",&d7);

	/*label parameters*/
	if(0==getch("label1","s",temp_ch)) strcpy(temp_ch,"sec"); 
		putch("label1","s",temp_ch);
	if(0==getch("label2","s",temp_ch)) strcpy(temp_ch,"km"); 
		putch("label2","s",temp_ch);
	if(0==getch("label3","s",temp_ch)) strcpy(temp_ch,"km"); 
		putch("label3","s",temp_ch);
	if(0==getch("label4","s",temp_ch)) strcpy(temp_ch,"km"); 
		putch("label4","s",temp_ch);
	if(0==getch("label5","s",temp_ch)) strcpy(temp_ch,"km"); 
		putch("label5","s",temp_ch);
	
	/*title esize */
	if(0==getch("title","s",temp_ch)) strcpy(temp_ch,"Impulsive_plane_wave"); 
		putch("title","s",temp_ch);
	esize=4; putch("esize","d",&esize);
		

	/*gather info about the spikes */
	if(0==getch("nsp","d",&nsp)) nsp=1;
	mag=(float*)malloc(sizeof(float)*nsp);
	k1=(int*)malloc(sizeof(int)*nsp);
	k2=(int*)malloc(sizeof(int)*nsp);
	k3=(int*)malloc(sizeof(int)*nsp);
	k4=(int*)malloc(sizeof(int)*nsp);
	k5=(int*)malloc(sizeof(int)*nsp);

	for(i1=0; i1 < nsp; i1++) { k1[i1]=0;k2[i1]=0; k3[i1]=0; k4[i1]=0;k5[i1]=0.;mag[i1]=1;}

	ierr=getch("k1","d",k1); if(ierr>nsp) seperr("number k1 > nsp ");
	ierr=getch("k2","d",k2); if(ierr>nsp) seperr("number k2 > nsp ");
	ierr=getch("k3","d",k3); if(ierr>nsp) seperr("number k3 > nsp ");
	ierr=getch("k4","d",k4); if(ierr>nsp) seperr("number k4 > nsp ");
	ierr=getch("k5","d",k5); if(ierr>nsp) seperr("number k5 > nsp ");
	ierr=getch("mag","f",mag); if(ierr>nsp) seperr("number mag > nsp ");

	/* check for bad parameters, convert to C */
	for(i1=0; i1<nsp; i1++){
		if(k1[i1] < 0 ) k1[i1]=-2;
		else if(k1[i1]<=n1) k1[i1]--;
		else seperr("invalid k1=%d \n",k1[i1]);
		if(k2[i1] < 0 ) k2[i1]=-2;
		else if(k2[i1]<=n2) k2[i1]--;
		else seperr("invalid k2=%d \n",k2[i1]);
		if(k3[i1] < 0 ) k3[i1]=-2;
		else if(k3[i1]<=n3) k3[i1]--;
		else seperr("invalid k3=%d \n",k3[i1]);
		if(k4[i1] < 0 ) k4[i1]=-2;
		else if(k4[i1]<=n4) k4[i1]--;
		else seperr("invalid k3=%d \n",k4[i1]);
		if(k5[i1] < 0 ) k5[i1]=-2;
		else if(k5[i1]<=n5) k5[i1]--;
		else seperr("invalid k5=%d \n",k5[i1]);
	}

	hclose();

	/* allocate and initialize array */
	array=(float*)malloc(sizeof(float)*n1);

/*create model */
for(i6=0;i6<n6*n7;i6++){
for(i5=0; i5 < n5; i5++){
for(i4=0; i4 < n4; i4++){
for(i3=0; i3 < n3; i3++){
	for(i2=0; i2 < n2; i2++){
		for(i1=0; i1 < n1; i1++) array[i1]=0.;
		for(isp=0; isp < nsp; isp++){
			if(k1[isp]==-2 && k2[isp]==-2 || k3[isp]==-2 ||
          k4[isp]==-2 && k5[isp]==-2){}  /*put 0 spikes */
			else{
			if(k1[isp]!=-1){ /*one spike per trace */
				if(k2[isp]==-1 || k2[isp]==i2){
					if(k3[isp]==-1 || k3[isp]==i3){
					if(k4[isp]==-1 || k4[isp]==i4){
					if(k5[isp]==-1 || k5[isp]==i5){
						array[k1[isp]]+=mag[isp];
					}}}
				}
			}
			else{ /* more than one spike per trace */
				if(k2[isp]==-1 || k2[isp]==i2){ 
					if(k5[isp]==-1 || k5[isp]==i5){
					if(k4[isp]==-1 || k4[isp]==i4){
					if(k3[isp]==-1 || k3[isp]==i3){
						for(i1=0; i1 < n1; i1++){  array[i1]+=mag[isp];}
					}}}
				}
			}
		}}
		if(n1*4!=srite("out",array,4*n1)) seperr("trouble writing out data");
	}
}
}
}
}
sep_end_prog();
return(0);


}
