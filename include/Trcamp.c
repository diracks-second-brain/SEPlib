/*$

=head1 NAME

 Trcamp - Calculate total energy in a tapered time window

=head1 SYNOPSIS

Trcamp < in.H  isum=1 itaper=0  > out.H

=head1 INPUT PARAMETERS

=over 4

=item  isum - integer 

	[1]: type of sum in time gate
	  1: RMS sum
	  2: sum of squares
	  3:  LOG_{10} of sum of squares

=item  t1   -  real   

	[t0]: start of time gate

=item  t2   -  real   

	[tmax]: end of time gate

=item itaper -integer 

	[0]: symmetric triangle taper in window: (0-1-0).

=back

=head1  DESCRIPTION

Calculate total amplitude/energy in a (tapered) time window

=head1 SYNOPSIS

	Trcamp < in.H  isum=2 itaper=0  > out.H

=head1 CATEGORY

B<seis/filter>


=cut


>*/
/*
 Author:  David E. Lumley,   7/93
 added taper option - david 9/94
	Modified Bob 8/99 Converted to C

*/	 

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("     Trcamp - Calculate total energy in a tapered time window");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Trcamp < in.H isum=1 itaper=0 > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    isum - integer");\
 sep_add_doc_line("                [1]: type of sum in time gate");\
 sep_add_doc_line("                  1: RMS sum");\
 sep_add_doc_line("                  2: sum of squares");\
 sep_add_doc_line("                  3:  LOG_{10} of sum of squares");\
 sep_add_doc_line("");\
 sep_add_doc_line("    t1 - real");\
 sep_add_doc_line("                [t0]: start of time gate");\
 sep_add_doc_line("");\
 sep_add_doc_line("    t2 - real");\
 sep_add_doc_line("                [tmax]: end of time gate");\
 sep_add_doc_line("");\
 sep_add_doc_line("    itaper -integer");\
 sep_add_doc_line("                [0]: symmetric triangle taper in window: (0-1-0).");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Calculate total amplitude/energy in a (tapered) time window");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("            Trcamp < in.H  isum=2 itaper=0  > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/filter");\
 sep_add_doc_line("");
#include <sep.main> 

MAIN()
{ 
	int n1,it1,it2,itm,idhd,i,esize,n3,n2;
	int nbig,isum,itape,one=1;
	float eps,t1,t2,o1,o2,o3,sum;
	float *in,*out;
	char temp_ch[128];
	int ndim;
	float d1,d2,d3,taper;
	int itaper,ithd,i3,i2,i1,it;

        sep_begin_prog();

	/*Deal with input*/
	if(hetch("esize","d",&esize)==0) esize=4;
	if(esize!=4) seperr("esize must be 4 \n");

	if(0!=sep_get_number_data_axes("in",&ndim))
		seperr("trouble getting number of dimensions \n");
	if(ndim<2 || ndim>3) seperr("expecting 2 or 3 input \n");
	
	i=1;if(0!=sep_get_data_axis_par("in",&i,&n1,&o1,&d1,temp_ch))
		seperr("trouble grabbing data axes\n");
	i=2;if(0!=sep_get_data_axis_par("in",&i,&n2,&o2,&d2,temp_ch))
		seperr("trouble grabbing data axes\n");
	i=1;if(0!=sep_put_data_axis_par("out",&i,&n2,&o2,&d2,temp_ch))
		seperr("trouble putting data axes\n");
	i=3;if(0!=sep_get_data_axis_par("in",&i,&n3,&o3,&d3,temp_ch))
		seperr("trouble grabbing data axes\n");
	i=2;if(0!=sep_put_data_axis_par("out",&i,&n3,&o3,&d3,temp_ch))
		seperr("trouble putting data axes\n");
	i=3;if(0!=sep_put_data_axis_par("out",&i,&one,&o2,&d2,temp_ch))
		seperr("trouble putting data axes\n");
	
	/*grab parameters */
	if(0==getch("isum","d",&isum)) isum=1;putch("isum","d",&isum);
	if(0==getch("itaper","d",&itaper)) isum=1;putch("itaper","d",&itaper);
	if(0==getch("t1","f",&t1)) t1=o1;putch("t1","f",&t1);
	if(0==getch("t2","f",&t2)) t2=o1+d1*(n1-1);putch("t2","f",&t2);

	eps= 1.0e-06;
	it1= (t1-o1)/d1 + .5;  it1= MIN(n1, MAX(1,it1));
	it2= (t2-o1)/d1 + .5;  it2= MIN(n1, MAX(1,it2));
	ithd= (it2-it1)/2;  itm= it1+ithd;

	/* allocate space for input and output traces */
	in=(float*) malloc(sizeof(float)*n1);
	out=(float*) malloc(sizeof(float)*n2);

	/* main loop over data */
	for(i3=0; i3 < n3; i3++){
		for(i2=0; i2 < n2; i2++){
			
			/* read in one trace */
			if(n1*esize!= sreed("in", in, n1*4))
				seperr("trouble reading in data \n");

			if(itaper==1) {       /* taper trace window?*/
				for(it=it1;it<it2;it++){
					if(it<= itm)  taper= 1.*(it-it1)/(1.*ithd);
					else   taper= 1.*(it2-it)/(1.*ithd);
					in[it]= taper*in[it];
				}
			}
			sum= 0.;
			for(i1=it1; i1 < it2; i1++) sum=in[i1]*in[i1]+sum; /*energy sum*/
			if(isum==1)  sum= sqrt(sum)/((float)(it2-it1+1));/*rms sum */
			else if(isum==3) sum=log(sum+eps)/log(10.); /*log 10 energy */
			out[i2]=sum;
		}

		/* output one trace */
		if(n2*esize!=srite("out",out,n2*esize))
			seperr("trouble writing output \n");
	} 
       sep_end_prog();
	return(0);
}
