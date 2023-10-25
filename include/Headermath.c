/*$

=head1 NAME

Headermath - Do mathematical operations on header keys

=head1 SYNOPSIS

Headermath <input.H pars  >output.H
 
=head1 INPUT PARAMETERS

=over 4

=item keyi- char*      

      keyname

=item eqni- char*      

      function recognizes header names and a number of
      functions, written in mathematical equation form

=item typei- char*     

      [scalar_float] type of output

=item maxsize - int    

      maximum size of temporary (in MB) defaults to 10 MB

=item verb   -  int    

      [0] whether or not to be verbose[1]

=item delete_keys-int* 

      [NULL] key indexes to delete

=item reciprocity_x - int 

      [0] whether or not to force all offset xs is to be the same
       sign (in this case positive).  Modifies the g_x, g_y, s_x,
         and s_y keys. All 4 of these keys  must exist. If h_x/offset_x and
         h_y/offset_y exist they are also modified

=item rotate-float   

      [optional] rotate using (s_x,s_y,g_x,g_y,cmp_x,cmp_y) the 
      coordinates of the dataset  (degrees)

=back

=head1 DESCRIPTION

Create new keys for a SEP3d dataset


=head1 COMMENTS

Example: To calculate cmp_x,cmp_y,azimuth
key1=cmp_x
eqn1=(sx+gx)/2
type1=scalar_float
key2=cmp_y
eqn2=(sy+gy)/2
type2=scalar_float
key3=azimuth
eqn3=@ATAN((gy-sy)/(gx-sx))
type3=scalar_float


Supported Functions (specified by @ at begining):

COS	SIN	TAN
ACOS	ASIN	ATAN
COSH	SINH	INT
EXP	LOG	SQRT
ABS

=head1 SEE ALSO

L<Math>,L<evaluate_expression>

=head1 CATEGORY

B<util/headers>

=cut

>*/
/*
Author:Robert G. Clapp

12/12/95 Begun
*/
#define IS_FLOAT 1
#define IS_INT 0
#define MAX_KEYS 40
#define KEEP_KEY 0
#define DELETE_KEY 1
#define MAX_EQN_LEN 1024
#define MAX_STR_LEN 128
#define FLOAT_TYPE "scalar_float"
#define INT_TYPE "scalar_int"
#define NOT_SET -1

#define MAX_SIZE 10
#include<ctype.h>
#include<sepvector.h>
#include<sepconvert.h>
#include<sep3d.h>
int how_many;

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int get_key_val(char *keyname,double *value);
int rotate_it(char *base,int *num_eqns, char key[MAX_KEYS][MAX_STR_LEN],char eqn_array[MAX_KEYS][MAX_EQN_LEN],int *type,float rot1,float rot2);
_XFUNCPROTOEND
#else
int get_key_val();
int rotate_it();
#endif

double *headers;
int num_keys;
int reciprocity_x;
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Headermath - Do mathematical operations on header keys");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Headermath <input.H pars >output.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    keyi- char*");\
 sep_add_doc_line("              keyname");\
 sep_add_doc_line("");\
 sep_add_doc_line("    eqni- char*");\
 sep_add_doc_line("              function recognizes header names and a number of");\
 sep_add_doc_line("              functions, written in mathematical equation form");\
 sep_add_doc_line("");\
 sep_add_doc_line("    typei- char*");\
 sep_add_doc_line("              [scalar_float] type of output");\
 sep_add_doc_line("");\
 sep_add_doc_line("    maxsize - int");\
 sep_add_doc_line("              maximum size of temporary (in MB) defaults to 10 MB");\
 sep_add_doc_line("");\
 sep_add_doc_line("    verb - int");\
 sep_add_doc_line("              [0] whether or not to be verbose[1]");\
 sep_add_doc_line("");\
 sep_add_doc_line("    delete_keys-int*");\
 sep_add_doc_line("              [NULL] key indexes to delete");\
 sep_add_doc_line("");\
 sep_add_doc_line("    reciprocity_x - int");\
 sep_add_doc_line("              [0] whether or not to force all offset xs is to be the same");\
 sep_add_doc_line("               sign (in this case positive).  Modifies the g_x, g_y, s_x,");\
 sep_add_doc_line("                 and s_y keys. All 4 of these keys  must exist. If h_x/offset_x and");\
 sep_add_doc_line("                 h_y/offset_y exist they are also modified");\
 sep_add_doc_line("");\
 sep_add_doc_line("    rotate-float");\
 sep_add_doc_line("              [optional] rotate using (s_x,s_y,g_x,g_y,cmp_x,cmp_y) the ");\
 sep_add_doc_line("              coordinates of the dataset  (degrees)");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Create new keys for a SEP3d dataset");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("    Example: To calculate cmp_x,cmp_y,azimuth key1=cmp_x eqn1=(sx+gx)/2");\
 sep_add_doc_line("    type1=scalar_float key2=cmp_y eqn2=(sy+gy)/2 type2=scalar_float");\
 sep_add_doc_line("    key3=azimuth eqn3=@ATAN((gy-sy)/(gx-sx)) type3=scalar_float");\
 sep_add_doc_line("");\
 sep_add_doc_line("    Supported Functions (specified by @ at begining):");\
 sep_add_doc_line("");\
 sep_add_doc_line("    COS SIN TAN ACOS ASIN ATAN COSH SINH INT EXP LOG SQRT ABS");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Math,evaluate_expression");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    util/headers");\
 sep_add_doc_line("");
#include<sep.main>
MAIN()
{
 int output_num_keys,logic,ierr,i1,i2,type[MAX_KEYS],*key_type,*tempia;
 int *key_out_index,*eqn_pointer,n2,ndim;
 float *fheaders,*head_out;
 int maxsize,i0,i10,ndelete,nkout=0;
 int tot_done,tempi=2,temp2i,n,*index,ierr1,ierr2,num_eqns,verb;
 int  delete_it,exists_in_header,kl,i,ncc;
 char label[MAX_STR_LEN],temp_ch[MAX_STR_LEN],temp2_ch[MAX_STR_LEN];
 char eqn_array[MAX_KEYS][MAX_EQN_LEN],key[MAX_KEYS][MAX_STR_LEN];
 char keyname[MAX_STR_LEN],keytype[MAX_STR_LEN],c1,c2,c3,c4;
 float tempr,o,d,rotate,rot1,rot2;
 float tempf; 
 int hxnum,hynum,gxnum,gynum,sxnum,synum,offsetxnum,offsetynum;
 double *result;
	char *equation;

sep_begin_prog(); 
/*SETUP SOME INITIAL VALUES */
i1=0; ierr1=1; ierr2=1; logic=0;
index= (int *) malloc (MAX_KEYS * sizeof(int));
 ierr= setvali(index ,MAX_KEYS,NOT_SET);

 if(0==getch("verb","d",&verb)) verb=0;
 if(0==getch("reciprocity_x","d",&reciprocity_x)) reciprocity_x=0;

/*GET PARAMETERS */

init_3d();
num_eqns=0;
for(i1=0;i1<MAX_KEYS;i1++){


  /* GET KEY NAME STRING */
  sprintf(temp_ch,"%s%d","key",i1+1);
  ierr2=getch(temp_ch,"s",key[i1]);

  /* GET KEY TYPE */
  sprintf(temp_ch,"%s%d","type",i1+1);
  if(0==getch(temp_ch,"s",temp2_ch))strcpy(temp2_ch,FLOAT_TYPE);

 	/* GET EQUATION STRING */
  sprintf(temp_ch,"%s%d","eqn",i1+1);
  ierr1=getch(temp_ch,"s",eqn_array[i1]) ;

  /* CHECK TO SEE IF TYPE MAKES SENSE AND STORE */
  if (0==strcmp(temp2_ch,"scalar_float")) type[i1]=IS_FLOAT;
  else if (0==strcmp(temp2_ch,"scalar_int")) type[i1]=IS_INT;
  else seperr("unrecognizable type %s=1 2= %s \n",temp_ch,temp2_ch);

  if(ierr1==1&&ierr2==1) num_eqns++;
	else break;
	putch(temp_ch,"s",eqn_array[i1]);
}

if(1==getch("rotate","f",&rotate)){
	sprintf(temp_ch,"rotating dataset %f degrees ",rotate);
	putlin(temp_ch);
	rot1=cos(rotate/45. * atan(1.));
	rot2=sin(rotate/45. * atan(1.));
	ierr=rotate_it("s",&num_eqns,key,eqn_array,type,rot1,rot2);
	ierr=rotate_it("g",&num_eqns,key,eqn_array,type,rot1,rot2);
	ierr=rotate_it("cmp",&num_eqns,key,eqn_array,type,rot1,rot2);
	ierr=rotate_it("offset",&num_eqns,key,eqn_array,type,rot1,rot2);
}

	



/*CHECK TO SEE IF PARAMETERS ARE REASONABLE*/
if (num_eqns==0 && reciprocity_x==0) seperr("must specify at least one equation \n");

if(0!=sep_get_number_keys("in",&num_keys))
	seperr("not valid SEP3d file \n");




/*************************************************/
/*NOW IT IS TIME TO FIGURE OUT OUTPUT HEADERS    */
/*************************************************/
tempia= (int *) malloc (num_keys * sizeof(int));
key_type = (int *) malloc (num_keys * sizeof( int));
key_out_index=(int *) malloc ((num_keys)*sizeof(int));
eqn_pointer=(int *) malloc ((num_eqns)*sizeof(int));
for(i1=0; i1 < num_eqns;i1++) eqn_pointer[i1]=NOT_SET;

ndelete=getch("delete_keys","d",tempia);
if(ndelete>num_keys) seperr("you can't delete more keys than you have \n");

for(i1=0; i1 < num_keys; i1++){ tempi=i1+1;
	if(0!=sep_get_key_name("in",&tempi,keyname) ||
    0!=sep_get_key_type("in",&tempi,keytype))
			seperr("trouble reading in key %d \n", tempi);


	/*int or float?*/
	if(0==strcmp(keytype,FLOAT_TYPE)) key_type[i1]=IS_FLOAT ;
	else if(0==strcmp(keytype,INT_TYPE)) key_type[i1]=IS_INT ;
	else seperr("unsuported keytype %s \n",keytype);


	/*should key be deleted?*/
	delete_it=0; 
	for(i2=0; i2 < ndelete; i2++){
		if(tempia[i2]-1==i1){
			 sprintf(temp_ch,"Deleting key %s",keyname);
			 putlin(temp_ch);
			 delete_it=1; break;
		}
	}
	if(delete_it==0){/* keep the key */ 
		key_out_index[i1]=nkout; nkout++;
	}
	else key_out_index[i1]=NOT_SET;


	/*is this key the output of an equation */
	exists_in_header=0;
	for(i2=0; i2 < num_eqns; i2++){
		if(0==strcmp(keyname,key[i2])){
			exists_in_header=1;
			eqn_pointer[i2]=i1;
			if(delete_it==1) seperr("you can not delete a key you are assigning\n");
			break;
		}
	}
}
free(tempia);
kl=nkout;

/* now find the equations where the output is not already a header key*/
for(i1=0; i1 < num_eqns; i1++){
	if(eqn_pointer[i1]==NOT_SET){ /* new key */
		eqn_pointer[i1]=nkout; nkout++;
	}	
}



/**********************************************/
/*time to start working on the output header  */
/**********************************************/

sep_get_number_header_axes("in",&ndim);
n2=1;
for(i2=2; i2 <= ndim;i2++){ 
 	sep_get_header_axis_par("in",&i2,&n,&o,&d,label);
	n2=n2*n;
 	sep_put_header_axis_par("out",&i2,&n,&o,&d,label);
}

sep_put_number_keys("out",&nkout);
for(i2=0; i2 < num_keys;i2++){ tempi=i2+1;
	if(key_out_index[i2]!=NOT_SET){ /* we are writing this key */
		sep_get_key_name("in",&tempi,keyname); tempi=key_out_index[i2]+1;
		if(key_type[i2]==IS_FLOAT) 
			sep_put_key("out",keyname,"scalar_float","xdr_float",&tempi);
		else
			sep_put_key("out",keyname,"scalar_int","xdr_int",&tempi);
	}
}
for(i2=0; i2 < num_eqns;i2++){ tempi=eqn_pointer[i2]+1;
	if(eqn_pointer[i2]>=kl){  /*this key did not already exist*/
		if(type[i2]==IS_FLOAT) 
			sep_put_key("out",key[i2],"scalar_float","xdr_float",&tempi);
		else
			sep_put_key("out",key[i2],"scalar_int","xdr_int",&tempi);
	}
}
		
     if(reciprocity_x==1){
	if(0!=sep_get_key_index("in","s_x",&sxnum) ||
	0!=sep_get_key_index("in","s_y",&synum) ||
	0!=sep_get_key_index("in","g_x",&gxnum) ||
	0!=sep_get_key_index("in","g_y",&gynum))
            seperr("when doing reciprocity expecting s_x,s_y,g_x,g_y to exist");
       if(0!=sep_get_key_index("in","offset_x",&offsetxnum)) offsetxnum=0;
       if(0!=sep_get_key_index("in","offset_y",&offsetynum)) offsetynum=0;
       if(0!=sep_get_key_index("in","h_x",&hxnum)) hxnum=0;
       if(0!=sep_get_key_index("in","h_y",&hynum)) hynum=0;
       sxnum-=1; synum-=1;gxnum-=1; gynum-=1;
       hxnum-=1; hynum-=1;offsetxnum-=1; offsetynum-=1;

    }

/*get maximum size  of arrays and calculate number of passes*/

if(0==getch("maxsize","d",&maxsize)) maxsize=MAX_SIZE;
ierr=sep_copy_data_pointer("in","out");
maxsize=maxsize*1000000;

if(1==getch("gff","s",temp_ch)){
	if(0!=strcmp(temp_ch,"-1")) sep_copy_gff("in","out");
}
sep_3d_close();


/***************************************************************/
/*                   NOW FOR THE REAL WORK                     */
/***************************************************************/
tot_done= 0;
maxsize = MIN(n2,maxsize / (4*nkout));
headers = (double *) malloc (num_keys * maxsize * sizeof( double));
result = (double *) malloc (maxsize * sizeof( double));
fheaders = (float*) malloc (num_keys * maxsize * sizeof(float));
head_out = (float *) malloc (nkout * maxsize * sizeof( float));
equation=(char *)  malloc (MAX_EQN_LEN * sizeof(char));

/* LOOP OVER HEADERS */
while(tot_done < n2){

	how_many= MIN(maxsize,n2-tot_done);

	if(verb>0) fprintf(stderr,
      "Working from %d, %d traces, of %d \n",tot_done,how_many,n2);


	/* read the data */
	i2=tot_done+1;
 	if(0!=sep_get_val_headers("in",&i2,&how_many,fheaders))
   	seperr("trouble getting headers  \n");



	/* convert them to doubles */
	i=0;
	for(i1=0; i1< num_keys; i1++){
		if(key_type[i1]==IS_INT){
			for(i2=0; i2 < how_many; i2++){
				tempr=fheaders[i1 + i2 * num_keys];
				ierr=getbackint(1,(int*)&tempr,&temp2i);
		 		headers[i2 * num_keys + i1]=(double)temp2i;
			}
		}
		else{
			for(i2=0; i2 < how_many; i2++)
		 		headers[i2 * num_keys + i1]=(double)fheaders[i1 + i2 * num_keys];
		}
		if(key_out_index[i1]!=NOT_SET){
			for(i2=0; i2 < how_many; i2++)
				head_out[key_out_index[i1]+i2*nkout]=fheaders[i1+i2*num_keys];
		}
	}
  for(i1=0;i1<num_eqns;i1++){
		strcpy(equation,eqn_array[i1]);
		 if(0!=evaluate_expression(equation,get_key_val,how_many,result))
			seperr("troublee evaluation expression \n");
		if(type[i1]==IS_FLOAT){
			for(i2=0; i2 < how_many; i2++) 
				head_out[i2*nkout+eqn_pointer[i1]]=(float) result[i2];
		}
		else{
			for(i2=0;i2 <how_many; i2++){
	 			temp2i=(int)result[i2];
	 			ierr=createfloat(1,(float*)&temp2i,&tempr);
	 			head_out[nkout * i2 + eqn_pointer[i1]]=tempr;
	 			ierr=getbackint(1,(int*)&tempr,&temp2i);
			}
		}
 	}
     if(reciprocity_x==1){
       for(i2=0; i2 < how_many; i2++){
        if(head_out[gxnum+num_keys*i2]-head_out[sxnum+num_keys*i2] <0.){

         tempf=head_out[gxnum+num_keys*i2];
         head_out[gxnum+num_keys*i2]=head_out[sxnum+num_keys*i2];
         head_out[sxnum+num_keys*i2]=tempf;

         tempf=head_out[gynum+num_keys*i2];
         head_out[gynum+num_keys*i2]=head_out[synum+num_keys*i2];
         head_out[synum+num_keys*i2]=tempf;
       }
     }

       for(i2=0; i2 < how_many; i2++){
         if(offsetxnum >=0) {
           if( head_out[offsetxnum+num_keys*i2] <0.){
             head_out[offsetxnum+num_keys*i2]=
                 head_out[offsetxnum+num_keys*i2]*-1;
             if(offsetynum >=0) 
              head_out[offsetynum+num_keys*i2]=
                head_out[offsetynum+num_keys*i2]*-1;
          }
        }
      } 
   ncc=0;
       for(i2=0; i2 < how_many; i2++){
         if(hxnum >=0) {
           if( head_out[hxnum+num_keys*i2] <0.){
ncc+=1;
             head_out[hxnum+num_keys*i2]=head_out[hxnum+num_keys*i2]*-1;
           if(hynum >=0) 
            head_out[hynum+num_keys*i2]=head_out[hynum+num_keys*i2]*-1;
          }
        }

       }
     }
 	i2=tot_done+1;
  if(0!=sep_put_val_headers("out",&i2,&how_many,head_out))
		seperr("trouble putting headers \n");
	tot_done= tot_done + how_many;
}


free (equation); free(headers); free(fheaders);
free(head_out);
sep_end_prog();
return 0;
}
#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int get_key_val(char *keyname,double *value)
_XFUNCPROTOEND
#else
int get_key_val(keyname,value)
char *keyname;
double *value;
#endif
{
int index=0,ierr,i1;
char whatname[MAX_STR_LEN];

for(i1=1;i1<=num_keys;i1++){
	ierr=sep_get_key_name("in",&i1,whatname);
	if(0==strcmp(whatname,keyname)){
		 index=i1; 
	}
}
if(index!=0) {
	for(i1=0; i1 <how_many; i1++)
		value[i1]=headers[index-1 + i1 * num_keys];
}
else seperr("can't find header %s \n",keyname);
ierr=ierr;
return 0;
}
	
#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int rotate_it(char *base,int *num_eqns, char key[MAX_KEYS][MAX_STR_LEN],char eqn_array[MAX_KEYS][MAX_EQN_LEN],int *type,float rot1, float rot2)
_XFUNCPROTOEND
#else
int rotate_it(base,num_eqns,key,eqn_array,type,rot1,rot2)
char *base;
int *num_eqns,*type;
float rot1,rot2;
char key[MAX_KEYS][MAX_STR_LEN],eqn_array[MAX_KEYS][MAX_EQN_LEN];
#endif
{
int ierr,ierr2,i1,tempi,n;
char x_c[MAX_STR_LEN],y_c[MAX_STR_LEN];

	sprintf(x_c,"%s_x",base);
	sprintf(y_c,"%s_y",base);

	n=*num_eqns;
	ierr=sep_get_key_index("in",x_c,&tempi);
	ierr2=sep_get_key_index("in",y_c,&tempi);
	if(ierr==0 && ierr2==0){
	
		if(n-2 >= MAX_KEYS) seperr("To many equations to rotate \n");
		for(i1=0; i1 < n; i1++){
			if(0==strcmp(x_c,key[i1]) || 0==strcmp(y_c,key[i1]))
				seperr("equation  %d is already modifying %s %s  \n",i1,x_c,y_c);
		}
                if(rot2 <0.)
		sprintf(eqn_array[n],"%f*%s%f*%s",rot1,x_c,rot2,y_c);
                else
		sprintf(eqn_array[n],"%f*%s+%f*%s",rot1,x_c,rot2,y_c);
		putch(x_c,"s",eqn_array[n]);
		strcpy(key[n],x_c); type[n]=IS_FLOAT; n++;

                if(rot2 <0.)
		sprintf(eqn_array[n],"%f*%s+%f*%s",rot1,y_c,-rot2,x_c);
                else if(rot2==0)
		sprintf(eqn_array[n],"%f*%s+%f*%s",rot1,y_c,rot2,x_c);
                else
		sprintf(eqn_array[n],"%f*%s-%f*%s",rot1,y_c,rot2,x_c);
		putch(y_c,"s",eqn_array[n]);
		strcpy(key[n],y_c); type[n]=IS_FLOAT; n++;
	
	}
	else if(ierr==0 || ierr2==0) fprintf(stderr,
		"WARNING BOTH %s and %s don't exist not rotating sources \n",x_c,y_c);

	*num_eqns=n;
	return(0);
}
