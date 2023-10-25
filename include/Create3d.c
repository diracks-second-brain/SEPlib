/*$

=head1 NAME


 Create3d - Create a SEPlib3d dataset
 
=head1 SYNOPSIS

 Create3d <in.H >out.H  [headers= ] [keyname1= keyname2 ....)

 

=head1 INPUT PARAMETERS

=over 4


=item  headers  - sepfile

       file containing headers

=item  keyname_ -  int

       defaults to key number (nkeys must equal n1 
       of headers)

=item  verb -  int

       [0]  whether or not to be verbose

=back
	
=head1 DESCRIPTION

Program that creates a SEPLIB3d file 
from either two S77 files or a single s77 file

WARNING!
The input file headers must NOT be out.H@@


=head1 CATEGORY

B<util/headers>

=cut


 
>*/
/*
 Created: Robert G. Clapp
 
 August 14, 1995 Started
 December 7, 1995 Combined two versions, updataed to current libraries
 July 4, 1997 Revised again
 July 30, 1999 Converted to C


>*/ 
/*
-------------------------------------------------

Author: Robert Clapp, ESMB 463, 7230253

Date Created:Fri Jul 30 13:38:44 PDT 1999

Purpose: 

*/	 

#include<sep3d.h>
#define HAVE_HEAD 1
#define NO_HEAD 0
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("     Create3d - Create a SEPlib3d dataset");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("     Create3d <in.H >out.H  [headers= ] [keyname1= keyname2 ....)");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    headers - sepfile");\
 sep_add_doc_line("               file containing headers");\
 sep_add_doc_line("");\
 sep_add_doc_line("    keyname_ - int");\
 sep_add_doc_line("               defaults to key number (nkeys must equal n1 ");\
 sep_add_doc_line("               of headers)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    verb - int");\
 sep_add_doc_line("               [0]  whether or not to be verbose");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Program that creates a SEPLIB3d file from either two S77 files or a");\
 sep_add_doc_line("    single s77 file");\
 sep_add_doc_line("");\
 sep_add_doc_line("    WARNING! The input file headers must NOT be out.H@@");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    util/headers");\
 sep_add_doc_line("");

#include <sep.main> 
MAIN() 
{ 

  int *n,i,ndim;
	float *d,*o;
	char label[128],temp_ch[128],keyname[128];
  float *headers;
	int verb,mode,ntr,nhelix,nkeys,one,nhead,*iloc,ndhead;
	int i2,tempi;
  float myo,myd;


	init_3d();
	if(0==hetch("esize","d",&i))	i=4;

	if(0==getch("verb","d",&verb))	 verb=0;
	
	if(0!=sep_get_number_data_axes("in",&ndim))
		seperr("trouble getting number of data axes \n");

	if(auxin("headers")!=0){
		 mode=HAVE_HEAD;
			if(1!=auxpar("n1","d",&nkeys,"headers")) 
				seperr("trouble getting n1 from headers\n");
	}
	else{
	 mode=NO_HEAD;
	 nkeys=ndim-1;
	}
	   

	if(verb==1){
		if(mode==HAVE_HEAD) 
			fprintf(stderr,"Creating sep3d from two files, nkeys=%d\n",nkeys);
		else 
			fprintf(stderr,"Converting a sep2d dataset to a sep3d dataset,nkeys=%d\n",
       nkeys);
	}


	if(0!=sep_put_number_keys("out",&nkeys))
		seperr("trouble putting number of keys\n");
	

	ntr=1;
	n=(int*) malloc(sizeof(int)*(ndim-1));
	o=(float*) malloc(sizeof(int)*(ndim-1));
	d=(float*) malloc(sizeof(int)*(ndim-1));
	for(i2=1;i2<=nkeys; i2++){
		sprintf(temp_ch,"keyname%d",i2);
		if(0==getch(temp_ch,"s",keyname)){
			if(0==strcmp(label,"none")){
				strcpy(keyname,temp_ch);
			}
			else strcpy(keyname,label);
  		}
		if(verb==1) fprintf(stderr,"setting header key %d to %s \n",i2,keyname);
		tempi=i2;
		if(0!=sep_put_key("out",keyname,"scalar_float","xdr_float",&tempi))
			seperr("trouble putting key %d \n",i2);
		sprintf(temp_ch,"n%d",i2);one=1;
	}
/*		if(i2>2) putch(temp_ch,"d",&one);*/
	for(i2=2;i2<=ndim; i2++){
		strcpy(label,"none");
		if(0!=sep_get_data_axis_par("in",&i2,&n[i2-2],&o[i2-2],&d[i2-2],label))
			seperr("trouble getting data axis label \n");
		ntr=ntr*n[i2-2];
/*		if(0!=sep_put_header_axis_par("out",&i2,&n[i2-2],&o[i2-2],&d[i2-2],label))*/
/*			seperr("trouble putting header axis \n");*/
	}
  i2=2;myo=1.; myd=1;
	if(0!=sep_put_header_axis_par("out",&i2,&ntr,&myo,&myd,"trace_in_bin"))
		seperr("trouble putting header axis \n");
	if(0!=sep_put_data_axis_par("out",&i2,&ntr,&myo,&myd,"trace_in_bin"))
		seperr("trouble putting data axis \n");

	if(mode==HAVE_HEAD){
		if(0!=sep_get_number_data_axes("headers",&ndhead))
			seperr("trouble gettin data axes from headers \n");

		nhead=1;
		for(i=2;i<=ndhead;i++){
			sprintf(temp_ch,"n%d",i);
			auxpar(temp_ch,"d",&i2,"headers");
			nhead=nhead*i2;
		}
fprintf(stderr,"nhead=%d, ntr=%d\n",nhead,ntr);
		if(nhead!=ntr)
			seperr("number of header values in headers not equal to number traces\n");
	}

		
	sep_copy_data_pointer("in","out");
	sep_3d_close();
	headers=(float*)malloc(nkeys*sizeof(float));
	
	one=1;
	if(mode==HAVE_HEAD){
		for(i=1;i<=ntr; i++){ 
			if(nkeys*4!=sreed("headers",headers,nkeys*4))
				seperr("trouble reading in headers \n");
			if(0!=sep_put_val_headers("out",&i,&one,headers))
				seperr("trouble putting headers \n");
		}

	}
	else{ /* no headers */
		iloc=(int*) malloc(sizeof(int)*(ndim-1));
		for(i=0; i <ntr;i++){
			h2c(i,n,ndim-1,iloc);
			for(i2=0; i2 < ndim-1;i2++) headers[i2]=o[i2]+d[i2]*iloc[i2];
			i2=i+1;
		  if(0!=sep_put_val_headers("out",&i2,&one,headers))
				seperr("trouble putting headers \n");
		}
	}
	
			


 
 
 
return(0);
 } 


/*  $Id: Create3d.csep,v 1.1.1.1 2004/03/25 06:37:29 cvs Exp $ */
