/*$

=head1 NAME

Synch3d - Synchronize headers and data

=head1 SYNOPSIS

Synch3d.x  <in.H >out.H 

=head1 INPUT PARAMETERS

=over 4

=item maxsize-  int    

      [5] Maximum amount of memory to use for headers in 
      megabytes


=item verb-int    

      [0]  Whether or not to be verbose

=back

=head1 DESCRIPTION

Synchs headers and data

=head1 SEE ALSO

B<sep_reorder_data>

=head1 CATEGORY

B<util/headers>

=cut

>*/ 
/*
-------------------------------------------------

Author: Robert Clapp, ESMB 463, 7230253

Date Created:Sat Jul 31 17:20:04 PDT 1999

Purpose: 

*/	 


#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Synch3d - Synchronize headers and data");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Synch3d.x <in.H >out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    maxsize- int");\
 sep_add_doc_line("              [5] Maximum amount of memory to use for headers in ");\
 sep_add_doc_line("              megabytes");\
 sep_add_doc_line("");\
 sep_add_doc_line("    verb-int");\
 sep_add_doc_line("              [0]  Whether or not to be verbose");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Synchs headers and data");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    sep_reorder_data");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    util/headers");\
 sep_add_doc_line("");
#include <sep.main> 
#include<sep3d.h>

MAIN()
{ 

  int n1,nkey,n2,read,npass,i1,i2,maxsize,ndim,drn,n,esize;
	int max_block,this_block,verb,mem; 
  int *headers,*locs;
	float o,d;
	char label[1024];


	init_3d();
	if(0==hetch("same_record_number","d",&i1)) i1=1;
	if(i1==1) seperr("Data is already in order \n");
	if(0==getch("maxsize","d",&maxsize)) maxsize=5;
	if(0==getch("verb","d",&verb)) verb=0;

	if(0!=sep_get_number_keys("in",&nkey))
		seperr("trouble getting number of keys \n");

	if(0!=sep_get_key_index("in","data_record_number",&drn))
		seperr("trouble getting data_record_number \n");

	if(0!=sep_get_number_header_axes("in",&ndim))
		seperr("trouble getting number of header axes \n");

	if(0!=sep_copy_header_keys("in","out"))
		seperr("trouble copying  keys \n");
	n=1;
	for(i2=2; i2<= ndim; i2++){
		if(0!=sep_get_header_axis_par("in",&i2,&n2,&o,&d,label))
			seperr("trouble grabbing header axis %d \n",i2);

		if(0!=sep_put_header_axis_par("out",&i2,&n2,&o,&d,label))
			seperr("trouble grabbing header axis %d \n",i2);

		n=n*n2;
	}
	if(1!=hetch("n1","d",&n1)) 
		seperr("trouble grabbing n1 from history file\n");
	if(1!=hetch("esize","d",&esize)) esize=4; 

	i2=1; putch("same_record_number","d",&i2);
	putch("n2","d",&n);
	sep_3d_close();


	max_block=maxsize*1000000/(nkey+1+2*n1)/esize;
 



	if(max_block > n) max_block=n;
	if(max_block<1) seperr("ERROR can't even hold a single header in memory\n");
  mem=max_block*n1*esize*2;
  mem=mem/1000/1000;
  if(mem==0) mem=1;

  if(verb==1) fprintf(stderr,"processing %d traces at a time \n",max_block);

	headers=(int*)malloc(nkey*max_block*sizeof(int));
	locs=(int*)malloc(max_block*sizeof(int));

	read=0;
	while(read < n){
		this_block=MIN(max_block,n-read);
		if(verb==1)fprintf(stderr,"Working on %d through %d of %d \n",
			read+1,read+max_block,n);
		i2=read+1;
		if(0!=sep_get_val_headers("in",&i2,&this_block,headers))
			seperr("trouble reading in headers \n");
		for(i2=0;i2< this_block;i2++){
			locs[i2]=headers[i2*nkey+drn-1];
			headers[i2*nkey+drn-1]=read+i2+1;
		}
		if(0!=sep_put_val_headers("out",&i2,&this_block,headers))
			seperr("trouble writing out headers \n");
/*
		if(0!= sep_reorder_data("in","out",this_block,n1*esize,locs))
			seperr("trouble reordering dataset \n");
*/
		if(0!= sep_reorder_data_fast("in","out",this_block,n1*esize,locs,mem))
			seperr("trouble reordering dataset \n");
		read+=this_block;
	}
	return(0);
 
 } 
