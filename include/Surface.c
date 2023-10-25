/*$

=head1 NAME
 
Surface - make surfaces 


=head1 SYNOPSIS

Surface [n1= n2= n3=1 nsurf=1 ] > out.H

=head1  INPUT PARAMETERS

=over 4

=item n1,n2,n3   -   integer  

      output cube dimensions.

=item mag        -   real

      real     [1.] determines the magnitude of the spike.
      Crossing spikes add their magnitudes.

=item nsurf        -   integer  

      [1] number of surfaces

=item a            -    float*

      [n1/2]   - static shift

=item b2           -    float*

      [0] what to scale the 2nd axis by (b2*(o2+d2*i2))

=item b3           -    float*

      [0] what to scale the 3rd axis by (b3*(o3+d3*i3))

=item c2,e2           -    float*

      [0] what to scale the 2nd axis by (c2*(o2+d2*i2-e2)^2)

=item c3,e3           -    float*

      [0] what to scale the 3rd axis by (c3*(o3+d3*i3-e3)^2)

=item min_ext2     -    float*
   
       [o2]  minimum extent along extent

=item min_ext3     -    float*
   
       [o3]  minimum extent along extent

=item max_ext2     -    float*
   
       [o2+d2*(n2-1)]  minimum extent along extent

=item max_ext3     -    float*
   
       [o3+d3*(n3-1)]  minimum extent along extent

=item title      -   char*    

       ' '  get putched into output header file .

=item o1,o2,o3   -   real     
       
      [0.] standard seplib

=item d1,d2,d3   -   real     

      [.004],[.1],[.1] standard seplib parameters

=item  add       - integer

       [1]   Add crossing surfaces (1) or replace value (0)

=item  layers    - integer

       [0]   If 1, perform causal integration 

=item label1..3  -   char*    

       ['sec'],['km],['km']  standard seplib meaning

=back

=head1 DESCRIPTION

Make linear and parolbic surfaces. Spikes
are placed at nearest integer location corresponding to the
formula:

iz=floor(((a+b2*(o2+d2*i2)+b3*(o3+d3*i3)+
     c2*(o2+d2*i2)^2 + c3*(o3+d3*i3)^2)-o1)/d1+.5)

=head1 COMMENTS

=head1 SEE ALSO

L<Gauss>, L<Vel>, L<Surface>

=head1 CATEGORY

B<seis/model>

=cut
>*/
/*
 Author - RGC 7/2001
*/

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Surface - make surfaces");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Surface [n1= n2= n3=1 nsurf=1 ] > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    n1,n2,n3 - integer");\
 sep_add_doc_line("              output cube dimensions.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    mag - real");\
 sep_add_doc_line("              real     [1.] determines the magnitude of the spike.");\
 sep_add_doc_line("              Crossing spikes add their magnitudes.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nsurf - integer");\
 sep_add_doc_line("              [1] number of surfaces");\
 sep_add_doc_line("");\
 sep_add_doc_line("    a - float*");\
 sep_add_doc_line("              [n1/2]   - static shift");\
 sep_add_doc_line("");\
 sep_add_doc_line("    b2 - float*");\
 sep_add_doc_line("              [0] what to scale the 2nd axis by (b2*(o2+d2*i2))");\
 sep_add_doc_line("");\
 sep_add_doc_line("    b3 - float*");\
 sep_add_doc_line("              [0] what to scale the 3rd axis by (b3*(o3+d3*i3))");\
 sep_add_doc_line("");\
 sep_add_doc_line("    c2,e2 - float*");\
 sep_add_doc_line("              [0] what to scale the 2nd axis by (c2*(o2+d2*i2-e2)^2)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    c3,e3 - float*");\
 sep_add_doc_line("              [0] what to scale the 3rd axis by (c3*(o3+d3*i3-e3)^2)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    min_ext2 - float*");\
 sep_add_doc_line("               [o2]  minimum extent along extent");\
 sep_add_doc_line("");\
 sep_add_doc_line("    min_ext3 - float*");\
 sep_add_doc_line("               [o3]  minimum extent along extent");\
 sep_add_doc_line("");\
 sep_add_doc_line("    max_ext2 - float*");\
 sep_add_doc_line("               [o2+d2*(n2-1)]  minimum extent along extent");\
 sep_add_doc_line("");\
 sep_add_doc_line("    max_ext3 - float*");\
 sep_add_doc_line("               [o3+d3*(n3-1)]  minimum extent along extent");\
 sep_add_doc_line("");\
 sep_add_doc_line("    title - char*");\
 sep_add_doc_line("               ' '  get putched into output header file .");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1,o2,o3 - real");\
 sep_add_doc_line("              [0.] standard seplib");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1,d2,d3 - real");\
 sep_add_doc_line("              [.004],[.1],[.1] standard seplib parameters");\
 sep_add_doc_line("");\
 sep_add_doc_line("    add - integer");\
 sep_add_doc_line("               [1]   Add crossing surfaces (1) or replace value (0)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    layers - integer");\
 sep_add_doc_line("               [0]   If 1, perform causal integration");\
 sep_add_doc_line("");\
 sep_add_doc_line("    label1..3 - char*");\
 sep_add_doc_line("               ['sec'],['km],['km']  standard seplib meaning");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Make linear and parolbic surfaces. Spikes are placed at nearest integer");\
 sep_add_doc_line("    location corresponding to the formula:");\
 sep_add_doc_line("");\
 sep_add_doc_line("    iz=floor(((a+b2*(o2+d2*i2)+b3*(o3+d3*i3)+ c2*(o2+d2*i2)^2 +");\
 sep_add_doc_line("    c3*(o3+d3*i3)^2)-o1)/d1+.5)");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Gauss, Vel, Surface");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/model");\
 sep_add_doc_line("");


#include <sep.main> 

MAIN()
{ 

  int n1,n2,n3,n4,n5;
	float o1,o2,o3,o4,o5;
	float d1,d2,d3,d4,d5;
	int esize;
	char temp_ch[16384],temp2_ch[1024];
  float *array;
	float *a,*b2,*c2,*b3,*c3;
  float *min_ext2,*min_ext3,*max_ext2,*max_ext3;
  int   *min2,*min3,*max2,*max3;
	int nsurf,layers;
	float *mag,t,*e_2,*e_3;
	int i1,i2,i3,ierr,isurf,i4,add;
  float zero,one2,one3,two2,two3,e2,e3;
  float val;

   sep_begin_prog();

	/*n parameters */
	if(0==getch("n1","d",&n1)) seperr("must supply n1 \n"); putch("n1","d",&n1);
	if(0==getch("n2","d",&n2)) n2=1; putch("n2","d",&n2);
	if(0==getch("n3","d",&n3)) n3=1; putch("n3","d",&n3);
	if(0==getch("n4","d",&n4)) n4=1; putch("n4","d",&n4);
	if(0==getch("n5","d",&n5)) n5=1; putch("n5","d",&n5);
	n4=n4*n5;

	/*o parameters */
	if(0==getch("o1","f",&o1)) o1=0.; putch("o1","f",&o1);
	if(0==getch("o2","f",&o2)) o2=0.; putch("o2","f",&o2);
	if(0==getch("o3","f",&o3)) o3=0.; putch("o3","f",&o3);
	if(0==getch("o4","f",&o4)) o4=0.; putch("o4","f",&o4);
	if(0==getch("o5","f",&o5)) o5=0.; putch("o5","f",&o5);
	
	/*d parameters */
	if(0==getch("d1","f",&d1)) d1=0.004; putch("d1","f",&d1);
	if(0==getch("d2","f",&d2)) d2=.1; putch("d2","f",&d2);
	if(0==getch("d3","f",&d3)) d3=.1; putch("d3","f",&d3);
	if(0==getch("d4","f",&d4)) d4=.1; putch("d4","f",&d4);
	if(0==getch("d5","f",&d5)) d5=.1; putch("d5","f",&d5);

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
	if(0==getch("nsurf","d",&nsurf)) nsurf=1;
	mag=(float*)malloc(sizeof(float)*nsurf);
	b2=(float*)malloc(sizeof(float)*nsurf);
	b3=(float*)malloc(sizeof(float)*nsurf);
	c2=(float*)malloc(sizeof(float)*nsurf);
	c3=(float*)malloc(sizeof(float)*nsurf);
	e_2=(float*)malloc(sizeof(float)*nsurf);
	e_3=(float*)malloc(sizeof(float)*nsurf);
	a=(float*)malloc(sizeof(float)*nsurf);
	min_ext2=(float*)malloc(sizeof(float)*nsurf);
	min_ext3=(float*)malloc(sizeof(float)*nsurf);
	max_ext2=(float*)malloc(sizeof(float)*nsurf);
	max_ext3=(float*)malloc(sizeof(float)*nsurf);
	min2=(int*)malloc(sizeof(int)*nsurf);
	min3=(int*)malloc(sizeof(int)*nsurf);
	max2=(int*)malloc(sizeof(int)*nsurf);
	max3=(int*)malloc(sizeof(int)*nsurf);
for(i1=0;i1 < nsurf; i1++) {e_2[i1]=0; e_3[i1]=0.;}

  /* Set the defaults */
	for(i1=0; i1 < nsurf; i1++) { 
    mag[i1]=1.; a[i1]=0.;
    b2[i1]=0; b3[i1]=0; c2[i1]=0; c3[i1]=0;
    min_ext2[i1]=o2; max_ext2[i1]=o2+d2*(n2-1); 
    min_ext3[i1]=o3; max_ext3[i1]=o3+d3*(n3-1); 
  }

	ierr=getch("mag","f",mag); if(ierr>nsurf) seperr("number mag > nsurf ");
  sprintf(temp_ch,"mag=%f",mag[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",mag[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("a","f",a); if(ierr>nsurf) seperr("number a > nsurf ");
  sprintf(temp_ch,"a=%f",a[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",a[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("b2","f",b2); if(ierr>nsurf) seperr("number b2 > nsurf ");
  sprintf(temp_ch,"b2=%f",b2[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",b2[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("c2","f",c2); if(ierr>nsurf) seperr("number c2 > nsurf ");
  sprintf(temp_ch,"c2=%f",c2[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",c2[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("c3","f",c3); 
  sprintf(temp_ch,"c3=%f",c3[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",c3[i1]);strcat(temp_ch,temp2_ch);
  }
	ierr=getch("e3","f",e_3); 
  sprintf(temp_ch,"e3=%f",e_3[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",e_3[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("e2","f",e_2); 
  sprintf(temp_ch,"e2=%f",e_2[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",e_2[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("b3","f",b3); if(ierr>nsurf) seperr("number b3 > nsurf ");
  sprintf(temp_ch,"b3=%f",b3[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",b3[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("min_ext2","f",min_ext2); 
  if(ierr>nsurf) seperr("number min_ext2 > nsurf ");
  sprintf(temp_ch,"min_ext2=%f",min_ext2[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",min_ext2[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("max_ext2","f",max_ext2); 
  if(ierr>nsurf) seperr("number max_ext2 > nsurf ");
  sprintf(temp_ch,"max_ext2=%f",max_ext2[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",max_ext2[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("min_ext3","f",max_ext3); 
  if(ierr>nsurf) seperr("number min_ext3 > nsurf ");
  sprintf(temp_ch,"min_ext3=%f",min_ext3[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",min_ext3[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

	ierr=getch("max_ext3","f",max_ext3); 
  if(ierr>nsurf) seperr("number max_ext3 > nsurf ");
  sprintf(temp_ch,"max_ext3=%f",max_ext3[0]);
  for(i1=1;i1<ierr;i1++){
   sprintf(temp2_ch,",%f",max_ext3[i1]);strcat(temp_ch,temp2_ch);
  }
  putlin(temp_ch);

  if(0==getch("add","d",&add)) add=1;
  if(0==getch("layers","d",&layers)) layers=0;



	/* check for bad parameters, convert to C */
	for(i1=0; i1<nsurf; i1++){
    min2[i1]=(min_ext2[i1]-o2)/d2+.5;
    min3[i1]=(min_ext3[i1]-o3)/d3+.5;
    max2[i1]=(max_ext2[i1]-o2)/d2+.5;
    max3[i1]=(max_ext3[i1]-o3)/d3+.5;
    if(min2[i1] < 0) min2[i1]=0.;
    if(min3[i1] < 0) min3[i1]=0.;
    if(max2[i1] > n2-1) min2[i1]=n2-1.;
    if(max3[i1] > n3-1) max3[i1]=n3-1.;
	}
	hclose();

	/* allocate and initialize array */
	array=(float*)malloc(sizeof(float)*n1);

/*create model */
for(i4=0; i4 < n4; i4++){
for(i3=0,one3=o3;i3<n3;i3++,one3+=d3){
  two3=one3*one3;
	for(i2=0,one2=o2;i2<n2;i2++,one2+=d2){
  	two2=one2*one2;
		for(i1=0; i1 < n1; i1++) array[i1]=0.;
		for(isurf=0; isurf < nsurf; isurf++){
      if(i2>=min2[isurf] && i2 <=max2[isurf] &&
         i3>=min3[isurf] && i3 <=max3[isurf]){
        two3=(one3-e_3[isurf])*(one3-e_3[isurf]);
        two2=(one2-e_2[isurf])*(one2-e_2[isurf]);
        val=(a[isurf]+one2*b2[isurf]+two2*c2[isurf]+
          one3*b3[isurf]+two3*c3[isurf] -o1)/d1+.5;
        i1=val;
        if(i1>=0 && i1 < n1){ 
          if(add==0) array[i1]=mag[isurf];
          else array[i1]+=mag[isurf];
        }
			}
		}
       if(layers==1) for(i1=1; i1 < n1; i1++) array[i1]=array[i1-1]+array[i1];
       
 
		if(n1*4!=srite("out",array,4*n1)) seperr("trouble writing out data");
	}
}
}
sep_end_prog();
return(0);


}
