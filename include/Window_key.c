/*$

=head1 NAME

Window_key - Window a dataset according to key info

=head1 SYNOPSIS

Window_key  <in.H >out.H  pars

=head1 INPUT PARAMETERS

=over 4

=item verb       -  int 

      [0]  Whether or not to ber berbose

=item maxsize,max_size    -  int 

      [20] Maximum size of memory to use for headers

=item synch      -  int 

      [0]  Whether (1) or not (0) to synch data

=item pct_print      -  int 

      [10]  If verbose how often to print progress

=item key (1..)  -  char     

      Name of the key the window

=item kindex(1..)-  int      

      Index of key to window

=item maxk(1..)  -  float    

      Max float or int size

=item mink(1..)  -  float    

      Max float or int size

=item reverse_logic   -  int

      Defaults to 0. If set to one only trace within given window will be rejected

=back

=head1 DESCRIPTION

Windows SEP3D headers dataset according to key values

=head1 COMMENTS

Relies on superset
Use to be written in ratfor77

=head1 SEE ALSO

L<Window3d>

=head1 CATEGORY

B<util/headers>

=cut


>*/ 
/*
-------------------------------------------------

Author: Robert Clapp, ESMB 463, 7230253

Date Created:Sat Jul 31 18:18:18 PDT 1999

Purpose: 

*/	 

#include <sepaux.h>
#include <sep3dc.h>
#if defined(HAVE_LIMITS_H)
#include <limits.h>
#endif

#ifndef INT_MAX
#define INT_MAX  2147483647
#endif
#ifndef INT_MIN
#define INT_MIN  -2147483647+1
#endif

#define STRING_SIZE 256
#define IS_INT 1
#define IS_FLOAT 0
int verb;

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int init_basics(sep3d *in, sep3d *out, int *synch, int *nkeys,
  int *max_size, float *pct_print,int *verb);
int grab_min_max(sep3d *in,int nkeys, int *int_min, int *int_max, int *key_num,
  int *key_type,  float *float_min, float *float_max);
_XFUNCPROTOEND
#endif

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Window_key - Window a dataset according to key info");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Window_key <in.H >out.H pars");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    verb - int");\
 sep_add_doc_line("              [0]  Whether or not to ber berbose");\
 sep_add_doc_line("");\
 sep_add_doc_line("    maxsize,max_size - int");\
 sep_add_doc_line("              [20] Maximum size of memory to use for headers");\
 sep_add_doc_line("");\
 sep_add_doc_line("    synch - int");\
 sep_add_doc_line("              [0]  Whether (1) or not (0) to synch data");\
 sep_add_doc_line("");\
 sep_add_doc_line("    pct_print - int");\
 sep_add_doc_line("              [10]  If verbose how often to print progress");\
 sep_add_doc_line("");\
 sep_add_doc_line("    key (1..) - char");\
 sep_add_doc_line("              Name of the key the window");\
 sep_add_doc_line("");\
 sep_add_doc_line("    kindex(1..)- int");\
 sep_add_doc_line("              Index of key to window");\
 sep_add_doc_line("");\
 sep_add_doc_line("    maxk(1..) - float");\
 sep_add_doc_line("              Max float or int size");\
 sep_add_doc_line("");\
 sep_add_doc_line("    mink(1..) - float");\
 sep_add_doc_line("              Max float or int size");\
 sep_add_doc_line("");\
 sep_add_doc_line("    reverse_logic - int");\
 sep_add_doc_line("              Defaults to 0. If set to one only trace within given window will be rejected");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Windows SEP3D headers dataset according to key values");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("    Relies on superset Use to be written in ratfor77");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Window3d");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    util/headers");\
 sep_add_doc_line("");
#include <sep.main> 
MAIN()
{ 
int maxs_ize,synch,edit,n_keys;
int nkeys,ndim,nlimits,i;
int *int_header,*int_min,*int_max,*key_num,*key_type,*drn_in,*drn_out;
float *float_min,*float_max;
float *float_header,o1,d1,*float_buf;
int *int_buf,*buf_out,n,esize,nblock;
int nwind[2],fwind[2],jwind[2];
char *buffer;
int nlimit,max_block,read,one,nh,i2,logic,i1,write_data;
sep3d in,out;
int done,found,block;
int *coords;
float *fhead;
int *ihead,*ilogic;
int *coord;
char *header_block;
int fout[2],jout[2],nout[2],max_size,ikey;
float pct,pct2;
int ipct,ipct2;
float pct_print;
float print_next;
int reverse,inorder;

sep_begin_prog();
getch_add_string("in.ignore_gff=1"); /*ignore the grid*/
init_3d();

if(0!=init_basics(&in,&out,&synch,&nkeys,&max_size,&pct_print,&verb))
  seperr("trouble initializing input and output\n");

int_min=(int*)malloc(sizeof(int)*nkeys);
int_max=(int*)malloc(sizeof(int)*nkeys);
key_num=(int*)malloc(sizeof(int)*nkeys);
key_type=(int*)malloc(sizeof(int)*nkeys);
float_min=(float*)malloc(sizeof(int)*nkeys);
float_max=(float*)malloc(sizeof(int)*nkeys);
if(0==getch("reverse_logic","d",&reverse)) reverse=0;
putch("reverse_logic","d",&reverse);

if(0!=grab_min_max(&in,nkeys,int_min,int_max,key_num,key_type,
  float_min,float_max)) seperr("trouble grabbing key ranges \n");

buffer=SEPNULL2;
if(synch==1){
  nblock=max_size*1000.*1000/(4*(out.nkeys+1)+out.n[0]*sep3dc_get_esize(&out));
  buffer=(char*) malloc(out.n[0]*sep3dc_get_esize(&out)*nblock);

}
else nblock=max_size*1000.*1000./(4*(out.nkeys+1));


fhead=(float*) malloc(nblock*sizeof(float));
ihead=(int*) malloc(nblock*sizeof(int));
ilogic=(int*) malloc(nblock*sizeof(int));
coord=(int*) malloc(nblock*sizeof(int));
drn_in=(int*) malloc(nblock*sizeof(int));
drn_out=(int*) malloc(nblock*sizeof(int));
header_block=(char*)malloc(nblock*in.nkeys*sizeof(char)*4);
if(0!=sep3d_grab_inorder("in",&inorder))
  seperr("trouble grabbing iorder");




nwind[0]=in.n[0];fwind[0]=fwind[1]=0; jwind[0]=jwind[1]=1;
fout[0]=fout[1]=0; jout[0]=jout[1]=1;nout[0]=in.n[0];
print_next=0;

if(synch==1){
  if(0!=sep3dc_inorder(&out))
    seperr("trouble  marking traces in order \n");
}
else  {
  sep_copy_data_pointer("in","out");
}

  
if(0!=sep3dc_write_description("out",&out))
  seperr("trouble writing out description \n");

while(fwind[1] < in.n[1]){
  nwind[1]=MIN(in.n[1]-fwind[1],nblock);
  if(0!=sep3dc_grab_headers("in",&in,&nh,&nwind[1],&fwind[1],&jwind[1]))
    seperr("trouble grabbing headers \n");

  for(i=0;i< nwind[1]; i++) ilogic[i]=1;
  if(inorder==1){
    for(i=0;i< nwind[1]; i++) drn_in[i]=i+fwind[1]+1;
  }
  else{
    if(0!=sep3dc_grab_drn(&in,drn_in)) seperr("trouble grabbing drn");
  }

  for(ikey=0; ikey < nkeys; ikey++){
    if(key_type[ikey]==IS_INT){
      if(0!=sep3dc_grab_key_vali(&in,key_num[ikey],(float*)ihead))
        seperr("trouble grabbing key value %d \n",ikey);
      for(i=0; i < nwind[1]; i++){
        if(ihead[i] < int_min[ikey] || ihead[i] >int_max[ikey])
          ilogic[i]=0;
      }
    }
    else{
      if(0!=sep3dc_grab_key_vali(&in,key_num[ikey],(float*)fhead))
        seperr("trouble grabbing key value %d \n",ikey);
      for(i=0; i < nwind[1]; i++){
        if(fhead[i] < float_min[ikey] || fhead[i] >float_max[ikey])
          ilogic[i]=0;
      }
    }
  }
  if(reverse==1){
    for(i=0; i < nwind[1]; i++){
      if(ilogic[i]==0) ilogic[i]=1;
      else ilogic[i]=0;
    }
  }
  for(nh=0,i=0;i< nwind[1]; i++) if(ilogic[i]==1) nh++;

  if(nh>0){
    if(0!=sep3dc_set_number_headers(&out,nh))
      seperr("trouble setting number of headers \n");

    if(0!=sep3dc_grab_header_block(&in,(void*)header_block))
      seperr("trouble grabbing header block\n");

   for(nh=0,i=0;i< nwind[1]; i++) {
     if(ilogic[i]==1){
       if(nh!=i)
         memcpy((void*)(header_block+4*in.nkeys*nh),
          (const void*)(header_block+4*in.nkeys*i),in.nkeys*4);
       drn_out[nh]=drn_in[i];
/*       coord[nh]=fwind[1]+i+1;*/
       nh++;
     }
   }
        
  if(0!=sep3dc_set_header_block(&out,(void*) header_block))
    seperr("trouble storing header block\n");

  if(synch==1){
    if(0!=sep3d_read_list("in",in.n[0],in.n[0],0,1,sep3dc_get_esize(&in),
     nh,drn_out,(char*)buffer))
       seperr("trouble reading trace list\n");
  }
  nout[1]=nh;
  if(synch==1){
    if(0!=sep3dc_inorder(&out))
      seperr("trouble  marking traces in order \n");
  }
  else if(0!=sep3dc_set_drn(&out,drn_out)) seperr("trouble setting drn");

  if(0!=sep3dc_write_data("out",&out,buffer,nout,fout,jout,nh,1,0))
   seperr("trouble writing out data \n"); 
      

  fout[1]+=nout[1];
 }
 fwind[1]+=nwind[1];
 pct=(float)(fwind[1])/(float)in.n[1];
 ipct=pct*1000;
 if(verb==1 && ipct > print_next ){
    pct2=((float)(fout[1]))/((float)(fwind[1]));
    ipct2=pct2*1000.;
    fprintf(stderr,"Finished %g pct. Found %d  valid traces (%g pct of input) \n",  (double)ipct/10.,fout[1],(double)ipct2/10.);
    print_next+=pct_print;
  }
}
out.ntraces=fout[1];
if(0!=sep3d_set_sep3d(&out))
  seperr("trouble setting output structure\n");
if(0!=sep3d_rite_num_traces("out",&out))
  seperr("trouble writing number of traces \n");
hclose();


sep_end_prog();
return(SUCCESS);
}

int init_basics(sep3d *in, sep3d *out, int *synch, int *nkeys,
  int *max_size, float *pct_print,int *verb)
{
char temp_ch[99];
char temp2_ch[99];
int i;
if(0!=init_sep3d_tag("in",in,"INPUT"))
 seperr("trouble initializing input \n");

if(0!=init_sep3d_struct(*in,out,"OUTPUT"))
  seperr("trouble initializing output \n");

 for(i=2; i < out->ndims; i++){
   out->n[1]=out->n[1]*out->n[i];
   out->n[i]=1;
 }
 if(out->ndims>2) sep3d_set_sep3d(out);
 
  


if(0==getch("pct_print","d",&i)) i=10;
*pct_print=i*10.;

if(0==getch("verb","d",verb)) *verb=0;
if(0==getch("synch","d",synch)) *synch=0;

if(0==getch("max_size","d",max_size)) 
  if(0==getch("maxsize","d",max_size))  *max_size=20;

i=1;
sprintf(temp_ch,"key%d",i);
while(1==getch(temp_ch,"s",temp2_ch)){
  i++;
  putch(temp_ch,"s",temp2_ch);
  sprintf(temp_ch,"key%d",i);
}
*nkeys=i-1;

if(i==1)
 return(sepwarn(NOT_MET,"Must specify at least one key \n"));

return(0);
}

int grab_min_max(sep3d *in,int nkeys, int *int_min, int *int_max, int *key_num,
  int *key_type,  float *float_min, float *float_max)
{
int i;
char temp1[1024],temp2[1024],key[1024],temp_ch[1024];

for(i=0; i < nkeys; i++){
  sprintf(temp_ch,"key%d",i+1);
  sprintf(temp1,"mink%d",i+1);
  sprintf(temp2,"maxk%d",i+1);
  getch(temp_ch,"s",key);
  key_num[i]=sep3dc_key_index(in,key);
  if(key_num[i]==-1) 
    return(sepwarn(NOT_MET,"key %s doesn't exist \n",key));
   
  if(0==strcmp("scalar_float",in->keytype[key_num[i]-1])){
		if(0==getch(temp1,"f",&float_min[i])) float_min[i]=-9e20;
    if(0==getch(temp2,"f",&float_max[i])) float_max[i]=9e20;
    key_type[i]=IS_FLOAT;
    putch(temp1,"f",&float_min[i]);
    putch(temp2,"f",&float_max[i]);
  }
  else if(0==strcmp("scalar_int",in->keytype[key_num[i]-1])){
    if(0==getch(temp1,"d",&int_min[i])) int_min[i]=INT_MIN;
    if(0==getch(temp2,"d",&int_max[i])) int_max[i]=INT_MAX;
    putch(temp1,"d",&int_min[i]);
    putch(temp2,"d",&int_min[i]);
    key_type[i]=IS_INT;
  }
  else return(sepwarn(NOT_MET,"Can't deal with key=%s type=%s \n",
    key,in->keytype[key_num[i]-1]));
}

 return(0);
}
