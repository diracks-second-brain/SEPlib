/*
=head1 NAME






=cut
*/


#include<seplib.h>
#include<sep3dc.h>
#include<sepaux.h>
#include<superset.h>
#ifndef SEPNULL2
#define SEPNULL2 ((void *) 0)
#endif
#include<mpi.h>


struct _distrib{
  int nsect;
  char **tag_sect;
  int *isect;
  int *sect_thread;
  int *dff_axis;
  int **axis_beg;
  int **axis_end;
  int nown;
  int *nblock;
  int *noverlap;
  int *iown;
  int *ilocal_pt;
  int ndim;
};

struct _pik{
 int index;
 int tr;
};
typedef struct _pik pik;

   

typedef struct _distrib distrib;



#define HEAD "/dev/null"
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");

int localize_sep3d(int iown, distrib *spread, sep3d *combo, sep3d *local);
int grid_to_head( int isect, sep3d *data, distrib *spread, int *nwind, int *fwind, int *nout,
  int *fout, int *i_h, int *hnum, int *grid_in, int *grid_out,int *head_out);



int transfer_sect(int isect,sep3d *big,distrib *spread,int *nwind, int *fwind, int *nsect, 
   int *fsect, sep3d  *small, int *nout, int *fout, float *buf1);
int pass_data(int impi,char *data, int n1, int n2,int ifrom, int ito);
int broadcast_data(int impi, int nmpi,float *data, int n1, int n2);

int any_data(sep3d *data, distrib *spread, int *nwind, int *fwind);
int create_regular_headers(int isect, sep3d *data, distrib *spread, int *i_h, int *hnum);
int head_to_data(distrib *spread, int drn,int *nwind, int *fwind, int *i_c, 
  int n_h, int *i_d, int *hnum, int *dnum, int *nhout, int *fhout, 
   int *headers, int *headers_out);
int parse_data(sep3d *data, int esize, int ntr, int *trnum,
     int *i_c,int n_d, int *dnum, int *ndout,int *fdout, float *buf_in, float *buf_out);
int sort_drns(int n, pik *inums);
void quicksort(int first, int last,pik *list) ;
int partition(int first, int last, pik *index) ;
void copy_pik(pik *new, pik *old);
int check_any_irregular(int impi,int nmpi,  distrib *spread, int *nwind, int *fwind, int nsz, int *i_c, int *dnum);
int calc_num_list( distrib *spread,int *i_h, int **hnum, int *inums, int *nnum);
void init_mpi_seplib(int argc, char **argv);
int condense_data(int impi, float *buf, int nlen, int ntr, int ftr,
   int *idone, int ntraces, int *itraces, int *nhave, int *tr_have);

#define REGULAR 1
#define HEADERS 2
#define GRID 3

int main(int argc, char **argv){
  int nblock[9],dff_axis[9];
  sep3d data,*sects;
  int e2,nnum;
  int impi,nmpi;
  distrib spread;
  int *fwind,*jwind,*nwind;
  int *fsect,*nsect;
  int *fout,*nout,*i_h,*i_d,*i_c;
  int isect,ierr,ia,i,verb,iloc[9];
  int max_size,esize,ipt,wh,wg;
  int f_f, nh[2],nd[2],drn;
  float *buf,*buf_out,zero,one;
  float *buf_send;
  char temp_ch[1204];
  char grid_tag[1024],header_tag[1024],**grid_out,**header_out;
  int **hnum,**dnum,*grid,*gout,*ngout,inum;
  pik *locs;
  int *headers,**headers_out,*head_out;
  int **ndout,**fdout,nhout[2],fhout[2];
  int *inums;
  int idim,ih,nd2,n123,two,ntest;
  MPI_Status status;
  int ilocation;
  int ig,ib;
  int *tr_have, nhave, itr_loc,ifirst ;
  char hfile[10024];
  int nlen;
  

  init_mpi_seplib(argc,argv);
  if(0!=get_distrib_info(&spread,1,"out_"))
    seperr("trouble getting distribution information");

  impi=sep_thread_num();
  nmpi=sep_num_thread();

  if(impi==0)
    if(0!= init_sep3d_tag("combo",&data,"SCRATCH"))
       return(sepwarn(NOT_MET,"trouble initializing input \n"));

  if(0!=sep3dc_broadcast_headers(&data,0))
    return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));

  if(0!=calc_output_sections( &data, &spread))
    return(sepwarn(NOT_MET,"trouble calculating sections"));

  if(data.file_format[0]=='R') seperr("only for irregular datasets");




  if(impi==0) {
      grab_history("combo",hfile,10023,&nlen);
          sep_get_number_keys("combo",&nh[0]);
    if(0!=sep_get_key_index("combo","data_record_number",&drn)) drn=0;
    auxpar("n2","d",&nd[1],"combo");
  }
if(impi==1) fprintf(stderr,"DRN %d \n",drn);
  if(impi!=0) {
     MPI_Recv(&drn, 1, MPI_INT, impi-1, 245, MPI_COMM_WORLD,&status);
     MPI_Recv(nh  , 1, MPI_INT, impi-1, 246, MPI_COMM_WORLD,&status);
     MPI_Recv(nd  , 2, MPI_INT, impi-1, 247, MPI_COMM_WORLD,&status);
  }
  if(impi!=nmpi-1){
     MPI_Send(&drn, 1, MPI_INT, impi+1, 245, MPI_COMM_WORLD);
     MPI_Send(nh, 1, MPI_INT, impi+1, 246, MPI_COMM_WORLD);
     MPI_Send(nd, 2, MPI_INT, impi+1, 247, MPI_COMM_WORLD);
  }
  drn=drn-1;
if(impi==1) fprintf(stderr,"D2N %d \n",drn);
  if(data.file_format[0]=='H') { f_f=HEADERS; }
  else if(data.file_format[0]=='G'){f_f=GRID;fget_grid_format_tag("combo",grid_tag);}

  if(impi!=0){
    grid_out=(char**)malloc(sizeof(char*)*spread.nown);
    header_out=(char**)malloc(sizeof(char*)*spread.nown);
    hnum=(int**)malloc(sizeof(int*)*spread.nown);
    dnum=(int**)malloc(sizeof(int*)*spread.nown);
    i_c=(int*)malloc(sizeof(int)*spread.nown);
    i_h=(int*)malloc(sizeof(int)*spread.nown);
    i_d=(int*)malloc(sizeof(int)*spread.nown);
    ndout=(int**)malloc(sizeof(int*)*spread.nown);
    fdout=(int**)malloc(sizeof(int*)*spread.nown);
    head_out=(int*)malloc(sizeof(int)*spread.nown);
    for(i=0; i < spread.nown; i++){
      i_h[i]=0;
      i_d[i]=0;
      i_c[i]=0;
      head_out[i]=0;
      hnum[i]=(int*)malloc(sizeof(int)*nd[1]);
      dnum[i]=(int*)malloc(sizeof(int)*nd[1]);
      ndout[i]=(int*)malloc(sizeof(int)*2);
      fdout[i]=(int*)malloc(sizeof(int)*2);
      fdout[i][1]=0;
      ndout[i][1]=0;
      grid_out[i]=(char*) malloc(sizeof(char)*1024);
      header_out[i]=(char*) malloc(sizeof(char)*1024);
    }
  }
  
    if(impi!=0){
     MPI_Recv(&nlen,1,MPI_INT,impi-1, 9999,MPI_COMM_WORLD,&status);
     MPI_Recv(hfile,nlen,MPI_CHAR,impi-1, 3999,MPI_COMM_WORLD,&status);
   }
   if(impi!=nmpi-1){
     MPI_Send(&nlen,1,MPI_INT,impi+1,9999, MPI_COMM_WORLD);
     MPI_Send(hfile,nlen,MPI_CHAR,impi+1,3999, MPI_COMM_WORLD);
   }

  /*create the local files*/
  sects=(sep3d*)malloc(sizeof(sep3d)*spread.nown);
  for(isect=0; isect < spread.nown; isect++){
    if(0!=localize_sep3d(spread.iown[isect],&spread,&data,&sects[isect]))
      seperr("trouble localizing struct");
    if(impi!=0){
       auxputhead(spread.tag_sect[spread.iown[isect]],"-----------Copied from input file--------");
    auxputhead(spread.tag_sect[spread.iown[isect]],"%s\n",hfile);
      if(0!=sep3dc_write_description(spread.tag_sect[spread.iown[isect]],
        &sects[isect])) 
         seperr("trouble writing description for %s",
           spread.tag_sect[spread.iown[isect]]);
if(impi==1) fprintf(stderr,"D3N %d \n",drn);
      if(drn>0){
        if(0!=sep_put_number_keys(spread.tag_sect[spread.iown[isect]],
          &nh[0]))
            seperr("trouble putting number keys");
        drn++;
        if(0!=sep_put_key(spread.tag_sect[spread.iown[isect]],"data_record_number",
           "scalar_int","xdr_int",&drn))
            seperr("trouble putting data_record_key");
        drn--;
        for(i=drn,ia=drn+1; i < nh[0]-1; i++,ia++){
          ib=ia+1;
          if(0!=sep_put_key(spread.tag_sect[spread.iown[isect]],data.keyname[i],
            data.keytype[i],data.keyfmt[i],&ib))
              seperr("trouble putting key");
              
        }
      }
      if(f_f==GRID)
        fget_grid_format_tag(spread.tag_sect[spread.iown[isect]],grid_out[isect]);
      fget_header_format_tag(spread.tag_sect[spread.iown[isect]],header_out[isect]);
    }
  }

  fget_header_format_tag("combo",header_tag);
  
  
  nh[1]=data.ntraces;
  nd[0]=data.n[0];
  nd2=data.ndims-1; 



  nwind=(int*) malloc(sizeof(int)*data.ndims);
  fwind=(int*) malloc(sizeof(int)*data.ndims);
  jwind=(int*) malloc(sizeof(int)*data.ndims);
  nsect=(int*) malloc(sizeof(int)*data.ndims);
  fsect=(int*) malloc(sizeof(int)*data.ndims);
  nout=(int*) malloc(sizeof(int)*data.ndims);
  ngout=(int*) malloc(sizeof(int)*data.ndims);
  fout=(int*) malloc(sizeof(int)*data.ndims);
  for(i=0; i < data.ndims; i++) jwind[i]=1;

  if(0==getch("verb","d",&verb)) verb=0;
  if(0==getch("max_size","d",&max_size)) max_size=10;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize/2;


  if(0!=init_loop_calc(data.ndims,data.n,"GrId",1900000000))
    seperr("trouble initializing loop calc Grid");
  if(0!=init_loop_calc(2,nh,"HeAdErS",100000))
    seperr("trouble initializing loop calc Headers");
  if(0!=init_loop_calc(2,nd,"DaTa",max_size))
    seperr("trouble initializing loop calc Data");

  ntest=0;
  /*first process the grid*/
  ig=0;
  if(f_f==GRID){
    while(0==do_sep_loop("GrId",nwind,fwind)){
      if(any_data(&data,&spread,nwind,fwind)==0){
        n123=1; for(i=1; i < data.ndims; i++) n123=n123*nwind[i];
        grid=(int*)malloc(sizeof(int)*n123);
        gout=(int*)malloc(sizeof(int)*n123);
        if(impi==0) {
          if(verb==1){
            fprintf(stderr,"Grid window ");
            for(i=1; i< data.ndims;i++)
               fprintf(stderr,"n%d=%d f%d=%d ",i+1,nwind[i],i+1,fwind[i]);
             fprintf(stderr,"n123=%d \n",n123);
          }
          /*Read the grid window */
          if(0!=sreed_window(grid_tag,&nd2,
            &data.n[1],&nwind[1],&fwind[1],&jwind[1],4,grid));
        }
        if(0!=impi)
           MPI_Recv(grid, n123, MPI_INT, impi-1, 2333, MPI_COMM_WORLD,&status);
        if(impi!=nmpi-1)
          MPI_Send(grid , n123, MPI_INT, impi+1, 2333, MPI_COMM_WORLD);

        if(impi!=0){
          for(ia=0; ia < spread.nown; ia++){
            isect=spread.iown[ia];
            ierr=grid_to_head(isect,&data,&spread,nwind,fwind,nout,fout,&i_h[ia],hnum[ia],
              grid,gout,&head_out[ia]);
            if(ierr==0 && impi!=0){
              for(i=1; i < data.ndims; i++) ngout[i]=data.n[i];
              for(i=0; i < spread.ndim; i++) {
                ngout[spread.dff_axis[i]]=(spread.axis_end[isect][i]-spread.axis_beg[isect][i]+1);
              } 
              if(0!=srite_window(grid_out[ia],&nd2,&ngout[1],&nout[1],&fout[1],
                jwind,4,gout)) seperr("trouble writing out grid (%d)",sep_thread_num());
            }   
          }
        }
        free(grid); free(gout);
      }
    }
  }
  else{
    for(ia=0; ia < spread.nown; ia++){
      if(0!=create_regular_headers(isect,&data,&spread,&i_h[ia],hnum[ia]))
      seperr("trouble creating regular headers");
    }
  }
  nnum=0;
  if(impi!=nmpi-1) MPI_Recv(&nnum , 1   , MPI_INT, impi+1, 840, MPI_COMM_WORLD,&status);
  if(impi!=0){
    for(ia=0; ia < spread.nown; ia++) nnum+=i_h[ia];
    MPI_Send(&nnum , 1   , MPI_INT, impi-1, 840, MPI_COMM_WORLD);
  }
  if(impi!=0) MPI_Recv(&nnum , 1   , MPI_INT, impi-1, 841, MPI_COMM_WORLD,&status);
  if(impi!=nmpi-1) MPI_Send(&nnum , 1   , MPI_INT, impi+1, 841, MPI_COMM_WORLD);
  inums=(int*) malloc(sizeof(int)*nnum);



  if(sep_thread_num()!=0) {
     headers_out=(int**) malloc(sizeof(int)*spread.nown);
     for(i=0; i <spread.nown; i++) {
       headers_out[i]=(int*) malloc(i_h[i]*nh[0]*sizeof(int));
     }
  }



  if(0!=calc_num_list(&spread,i_h,hnum,inums,&nnum))
    seperr("trouble calculating trace numbers");

  ilocation=0;
  /*now time to deal with the headers */
  while(0==do_sep_loop("HeAdErS",nwind,fwind)){
    if(0==check_any_irregular(impi,nmpi,&spread,nwind,fwind,nnum,&ilocation,inums)){
      n123=nwind[0]*nwind[1];
      headers=(int*)malloc(n123*sizeof(int));
      if(impi==0){
        i=2;
        if(verb==1){
          fprintf(stderr,"Header window ");
          for(i=1; i< 2;i++)
             fprintf(stderr,"n%d=%d f%d=%d ",i+1,nwind[i],i+1,fwind[i]);
           fprintf(stderr,"\n");
        }
        if(0!=sreed_window(header_tag,&i,nh,nwind,fwind,jwind,4,headers))
          seperr("trouble reading headers");
      }
      if(0!=impi)
         MPI_Recv(headers, n123, MPI_INT, impi-1, 2334, MPI_COMM_WORLD,&status);
      if(impi!=nmpi-1)
        MPI_Send(headers , n123, MPI_INT, impi+1, 2334, MPI_COMM_WORLD);
      if(impi!=0){
        for(ia=0; ia < spread.nown; ia++){
            i=2;
            ierr=head_to_data(&spread,drn,nwind,fwind,&i_c[ia],i_h[ia],
            &i_d[ia],hnum[ia], dnum[ia],nhout,fhout, headers, headers_out[ia]);
        }
      }
      free(headers); 
    }
  }

  /*fix the drns if relevant*/
  if(impi!=0){
fprintf(stderr,"CHECK NOWN %d %d \n",spread.nown,impi);
    for(ia=0; ia < spread.nown; ia++){
      i_c[ia]=0;
      if(drn!=0 ){
        locs=(pik*)malloc(sizeof(pik)*i_d[ia]*2);
        for(i=0; i < i_d[ia]; i++){
          locs[i].tr=dnum[ia][i];
          locs[i].index=i;
        }
//if(impi==1) fprintf(stderr,"D4N %d \n",drn);
        if(0!=sort_drns(i_d[ia],locs))
         seperr("trouble sorting data record numbers");
        for(i=0; i < i_d[ia]; i++){
            headers_out[ia][locs[i].index*nh[0]+drn]=i+1; /*locs[i].index;*/
//if(impi==1) fprintf(stderr,"CHECK THIS drn=%d nh=%d i=%d ia=%d  %d=dnum \n",drn,nh[0],i,ia,locs[i].tr);
            dnum[ia][i]=locs[i].tr;
        }
        i=2;fhout[0]=0; fhout[1]=0   ;
        nh[1]=i_d[ia];
        if(0!=srite_window(header_out[ia],&i,nh,nh,fhout,jwind,4,headers_out[ia]))
         seperr("trouble writing out headers");
        free(locs);
      }
    }
  }
  
  if(sep_thread_num()!=0){
    for(i=0; i < spread.nown; i++) free(headers_out[i]);
    free(headers_out);
  }


  if(0!=calc_num_list(&spread,i_d,dnum,inums,&nnum))
    seperr("trouble calculating trace numbers");

  ilocation=0;  ifirst=1;
  /*finally time to deal with data*/
  while(0==do_sep_loop("DaTa",nwind,fwind)){
    if(0==check_any_irregular(impi,nmpi,&spread,nwind,fwind,nnum,&ilocation,inums)){
/*      fprintf(stderr,"PASSING (%d) %d \n",sep_thread_num(),fwind[1]);*/
      n123=nwind[0]*nwind[1]*esize/2;
      if(ifirst==1){
        itr_loc=0;
        tr_have=(int*) malloc(sizeof(int)*nwind[1]);
        buf=(float*)malloc(n123*sizeof(float));
        buf_out=(float*)malloc(n123*sizeof(float));
        ifirst=0;
      }
      if(impi==0){
        i=2; 
        if(verb==1){
          fprintf(stderr,"Data window ");
          for(i=1; i< 2;i++)
             fprintf(stderr,"n%d=%d f%d=%d ",i+1,nwind[i],i+1,fwind[i]);
           fprintf(stderr,"\n");
        }
        if(0!=sreed_window("combo",&i,nd,nwind,fwind,jwind,esize,buf))
          seperr("trouble reading headers");
      }
      if(0!=condense_data(impi,buf,esize/4*nwind[0],nwind[1],fwind[1],
        &itr_loc,nnum,inums,&nhave,tr_have))
         seperr("trouble condensing data");
      if(0!=broadcast_data(impi,nmpi,buf,esize/4*nwind[0],nhave))
        seperr("touble broadcasting data");
      if(impi!=0){
        for(ia=0; ia < spread.nown; ia++){
           if(0==parse_data(&data,esize,nhave,tr_have,&i_c[ia],i_d[ia],dnum[ia],
             ndout[ia],fdout[ia], buf, buf_out)){
             i=2; 
fprintf(stderr,"WRITING OUT  %d %d \n",impi,ndout[ia][1]);
              if(0!=srite_window(spread.tag_sect[spread.iown[ia]],&i,nd,ndout[ia],fdout[ia],
                jwind,esize,buf_out)) seperr("trouble writing out data");
 
           }
        }
      }
    }
/*
    else
      fprintf(stderr,"NOT PASSING (%d) %d \n",sep_thread_num(),fwind[1]);
fprintf(stderr,"IN DATA LOOP %d (%d) \n",fwind[1],sep_thread_num());
*/
  }
 free(buf); free(buf_out); free(tr_have);
  if(impi!=0){
    for(i=0; i < spread.nown; i++){
      ia=2;one=1;zero=0;
      if(0!=sep_put_header_axis_par(spread.tag_sect[spread.iown[i]],&ia,&i_d[i],&zero,
        &one,"trace number")) seperr("trouble putting number of traces");
      if(0!=sep_put_data_axis_par(spread.tag_sect[spread.iown[i]],&ia,&i_d[i],&zero,
        &one,"trace number")) seperr("trouble putting number of traces");
       if(drn!=0) 
         auxputch("same_record_number","d",&zero,spread.tag_sect[spread.iown[i]]);
      if(i_d[i]==0) {
        fget_header_format_tag(spread.tag_sect[spread.iown[i]],header_out[i]);

         srite(header_out[i],&ia,4);
         srite(spread.tag_sect[spread.iown[i]],&ia,4);
      } 

    } 
  }
/*
fprintf(stderr,"ready to finish \n");
*/
   
  if(impi==0){
    if(1==getch("stat_good","s",temp_ch)){
      auxputch("junk","d",&impi,temp_ch);
    }
  } 
  MPI_Finalize();
  return(0);
}

int any_data(sep3d *data, distrib *spread, int *nwind, int *fwind)
{
  int isect,iax,igood,i;
  for(isect=0 ; isect < spread->nsect; isect++){
    igood=1;
    for(iax=0; iax < spread->ndim; iax++){
      i=spread->dff_axis[iax];
      if(fwind[i]  > spread->axis_end[isect][iax]) igood=0;
      if(fwind[i]+nwind[i]  < spread->axis_beg[isect][iax]) igood=0;
    }
    if(igood==1) return(0);
  }
  return(1);
}

int grid_to_head( int isect, sep3d *data, distrib *spread, int *nwind, int *fwind, int *nout,
  int *fout, int *i_h, int *hnum, int *grid_in, int *grid_out,int *head_out)
{
  int fsect[9],nsect[9];
  int  b[9],e[9],s[9],nfound;
  int i1,i2,i3,i4,i5,i6,i7,i8,ib;
  int tot1,tot3,tot5,tot7,ngood=0;
  int c1,c2,c3,c4,c5,c6,c7,c8,ierr,i,iold;
  ierr=   local_window(isect,data,spread,nwind,fwind,nsect,fsect,nout,fout);
  if(ierr!=0) return(ierr);
  create_bounds(isect,spread,data->ndims,data->n,nwind,fwind,b,e,s);
  for(i=1; i < data->ndims; i++) {
    b[i]=b[i]-fwind[i];
    e[i]=e[i]-fwind[i];
  }
  for(i=2; i < data->ndims; i++) s[i]=(s[i]*nwind[i-1])/data->n[i-1];
  i=0;iold=*i_h;
  for(i8=b[8],c8=b[8]*s[8]; i8 <= e[8]; i8++,c8+=s[8]){
  for(i7=b[7],c7=b[7]*s[7]; i7 <= e[7]; i7++,c7+=s[7]){
    tot7=c7+c8;
    for(i6=b[6],c6=b[6]*s[6]; i6 <= e[6]; i6++,c6+=s[6]){
    for(i5=b[5],c5=b[5]*s[5]; i5 <= e[5]; i5++,c5+=s[5]){
      tot5=c5+c6+tot7;
      for(i4=b[4],c4=b[4]*s[4]; i4 <= e[4]; i4++,c4+=s[4]){
      for(i3=b[3],c3=b[3]*s[3]; i3 <= e[3]; i3++,c3+=s[3]){
        tot3=c4+c3+tot5;
        for(i2=b[2],c2=b[2]*s[2]; i2 <= e[2]; i2++,c2+=s[2]){
        for(i1=b[1],c1=b[1]*s[1]; i1 <= e[1]; i1++,c1+=s[1],i++){
          ib=c1+c2+tot3;
          if(grid_in[ib] > 0){
             hnum[ngood+iold]=grid_in[ib];
             ngood+=1;
             grid_out[i]=ngood+iold;
          }
          else grid_out[i]=-1;
        }}
      }}
    }}
  }}
  if(ierr!=0) return(ierr);

  if(i==0) return(1);
  *i_h=*i_h+ngood;
  *head_out=*head_out+ngood;
  return(0);

}


int create_regular_headers(int isect, sep3d *data, distrib *spread, int *i_h, int *hnum)
{
  int i;
  int fwind[9];
  int b[9],e[9],s[9];
  int c1,c2,c3,c4,c5,c6,c7,c8;
  int i1,i2,i3,i4,i5,i6,i7,i8;
  int tot7,tot5,tot3;
  
  for(i=0; i < 9; i++) fwind[i]=0;
  create_bounds(isect,spread,data->ndims,data->n,data->n,fwind,b,e,s);

  i=0;
  for(i8=b[8],c8=b[8]*s[8]; i8 <= e[8]; i8++,c8+=s[8]){
  for(i7=b[7],c7=b[7]*s[7]; i7 <= e[7]; i7++,c7+=s[7]){
    tot7=c7+c8;
    for(i6=b[6],c6=b[6]*s[6]; i6 <= e[6]; i6++,c6+=s[6]){
    for(i5=b[5],c5=b[5]*s[5]; i5 <= e[5]; i5++,c5+=s[5]){
      tot5=c5+c6+tot7;
      for(i4=b[4],c4=b[4]*s[4]; i4 <= e[4]; i4++,c4+=s[4]){
      for(i3=b[3],c3=b[3]*s[3]; i3 <= e[3]; i3++,c3+=s[3]){
        tot3=c4+c3+tot5;
        for(i2=b[2],c2=b[2]*s[2]; i2 <= e[2]; i2++,c2+=s[2]){
        for(i1=b[1],c1=b[1]*s[1]; i1 <= e[1]; i1++,c1+=s[1],i++){
          hnum[i]=c1+c2+tot3+1;
        }}
      }}
    }}
  }}

  *i_h=i;
  return(0);
}

int head_to_data(distrib *spread, int drn,int *nwind, int *fwind, int *i_c, 
   int n_h, int *i_d, 
   int *hnum, int *dnum, int *nhout, int *fhout, 
   int *headers_in, int *headers_out)
{

  int itr,ntot;
  int ibeg,iend;
  int i2,iout,iold;

  ntot=n_h;

  itr=*i_c;
  ibeg=fwind[1]+1;
  iend=fwind[1]+nwind[1];
 

  if(itr >= ntot ) return(1); /*already parsed all data in this section*/


  if(hnum[itr] < ibeg) seperr("Internal error with calculations itr=%d hnum=%d ibeg=%d iend=%d  (%d)\n",itr,hnum[itr],ibeg,iend,sep_thread_num());
  if(hnum[itr] > iend) return(1); /*next trace is outside this range*/

  fhout[0]=0; fhout[1]=hnum[itr]-1;
  nhout[0]=nwind[0];
  nhout[1]=0;

  /*loop traces*/
  iold=*i_c;
  for(i2=0; i2 < nwind[1];i2++){
    /*if a trace belongs to this section*/
    if(fwind[1]+i2+1 == hnum[itr]){


      memcpy((void*)(headers_out+(iold+nhout[1])*nwind[0]),
       (const void*)(headers_in+i2*nwind[0]),4*nwind[0]);
      if(drn!=-1){
        dnum[itr]=headers_out[nwind[0]*(iold+nhout[1])+drn];
      }
      else dnum[itr]=hnum[itr];
      itr++;
      nhout[1]++;
    }
  }
  *i_c=nhout[1]+*i_c;
  *i_d=*i_d+nhout[1];
  return(0);
}

int sort_drns(int n, pik *inums)
{
  quicksort(0,n-1,inums);
  return(0);
}
void quicksort(int first, int last,pik *index) {
    int pivindx; /* index of the element separating the two sub-arrays*/
    if (last > first) {  /* More than one element to be sorted?*/
        pivindx = partition(first, last,index);
        quicksort(first, pivindx - 1,index);
        quicksort(pivindx + 1, last,index);
    }
}
                                                                                
void copy_pik(pik *new, pik *old)
{
   new->index=old->index;
   new->tr=old->tr;
}

int partition(int first, int last, pik *index) {
    int pivindx, top, i;
    pik pivot;
                                                                                        
    /* Choose a pivot: select a random index between first and last.*/
    i = rand() % (last - first + 1) + first;
                                                                                        
    /* Put the pivot first, remember pivot, initialise ready for loop.*/
                copy_pik(&pivot,&index[i]);/* remember the pivot */
                copy_pik(&(index[i]),&index[first]);
                copy_pik(&(index[first]),&pivot);  /* pivot now first */
    pivindx = first;
    top = last;
                                                                                        
    while (top > pivindx) {         /* Still unknown elements */
        /* top indicates the highest unknown element */
        if (index[top].tr >= pivot.tr) {
            top--;              /* where it belongs, count as >=*/
        } else {
                                          copy_pik(&(index[pivindx]),&index[top]);/*shift down*/
                                          copy_pik(&(index[top]),&index[pivindx+1]);/*shift displaced element up*/
                                          copy_pik(&(index[pivindx+1]),&pivot);/*Put pivot back*/
            pivindx++;            /* Alter record of pivot location*/
        }
    }
    return pivindx;
}



/*Adapted from http://ironbark.bendigo.latrobe.edu.au/courses/bcomp/c103/sem296/lectures/Lecture2.html*/
                                                                                                          
                                                                                                          

int check_any_irregular(int impi,int nmpi,  distrib *spread, int *nwind, int *fwind,  int nloc,int *iloc, int *locs)
{
int i;

i=*iloc;
if(i ==nloc) return(1);
if(fwind[1]>= locs[i])
  seperr("internal error: somehow missed a trace fwind=%d locs=%d nwind=%d nloc=%d", fwind[1],locs[i],nwind[1],nloc);
if(fwind[1]+nwind[1] < locs[i]) return(1);
else{
  while(i < nloc  && locs[i] <= fwind[1]+nwind[1]) i++;
  *iloc=i;
  return(0);
}



/*
  int inext,i,imin,imax;
  int ifirst,ilast,iold;
  int ineed;
  MPI_Status status;

  ifirst=fwind[1]+1;
  ilast=fwind[1]+nwind[1];

  inext=0;
  if(impi !=0){
    ineed=0;
    for(i=0; i <spread->nown; i++){
      if(i_c[i] < nsz[i] -1){
        if(dnum[i][i_c[i]] < ifirst) seperr("Internal error with calculations i=%d i_c=%d dnum=%d ifirst=%d ilast=%d (%d)",i,i_c[i],dnum[i][i_c[i]],ifirst,ilast,sep_thread_num());
        if(dnum[i][i_c[i]] <= ilast) ineed=1;
      }
    }
  }
  if(impi!=nmpi-1){
     MPI_Recv(&inext, 1, MPI_INT, impi+1, 2337, MPI_COMM_WORLD,&status);
     if(inext==1) ineed=1;
  }
  if(impi!=0)
     MPI_Send(&ineed, 1, MPI_INT, impi-1, 2337, MPI_COMM_WORLD);

  if(impi!=0)
     MPI_Recv(&ineed, 1, MPI_INT, impi-1, 2338, MPI_COMM_WORLD,&status);

  if(impi!=nmpi-1)
   MPI_Send(&ineed, 1, MPI_INT, impi+1, 2338, MPI_COMM_WORLD);

  if(ineed==0) return(1);
  return(0);
*/

}



int parse_data(sep3d *data, int esize, int ntr, int *trnum,
     int *i_c,int n_d, int *dnum, int *ndout,int *fdout, float *buf_in, float *buf_out)
{
  int itr,ntot;
  int ibeg,iend;
  int i2,iout;

  ntot=n_d;
  itr=*i_c;
  ibeg=trnum[0];
  iend=trnum[ntr-1];
 

  if(itr >=ntot ) return(1); /*already parsed all data in this section*/

  if(dnum[itr] < ibeg) seperr("Internal error with calculations");
  if(dnum[itr] > iend) return(1); /*next trace is outside this range*/

  fdout[0]=0; 
  fdout[1]=fdout[1]+ndout[1];
  ndout[0]=data->n[0];
  ndout[1]=0;

  /*loop traces*/
  for(i2=0; i2 < ntr; i2++){
    if(trnum[i2]==dnum[itr]){
    /*if a trace belongs to this section*/
      memcpy((void*)(buf_out+ndout[1]*esize/4*data->n[0]),
       (const void*)(buf_in+i2*data->n[0]*esize/4),esize*data->n[0]);
      itr++;
      ndout[1]++;
    }
  }
  *i_c=ndout[1]+*i_c;
  return(0);
}
  


int transfer_sect(int isect,sep3d *big, distrib *spread, int *nwind,int *fwind, 
 int *nsect, int *fsect, sep3d  *small, int *nout, int *fout,float *buf1)
{
  int *headers;
  int *coords;
  int ih,idim,nd,iax,i,ikeep,igood,ia,ib,esize,i2;
  int iloc[9],n123,ibeg[9],iend[9],e2;

  for(iax=0,n123=1; iax < big->ndims; iax++) {
    n123=n123*nout[iax];
    ibeg[iax]=0; iend[iax]=big->n[iax];
  }

  /*create the begining for each axis for this section*/
  for(iax=0; iax < spread->ndim; iax++){
   ibeg[spread->dff_axis[iax]]=spread->axis_beg[isect][iax];
   iend[spread->dff_axis[iax]]=spread->axis_end[isect][iax];
  }


for(i=0; i < big->ndims;i++)
  e2=sep3dc_get_esize(big)/4;

  /*loop through output */
  for(i2=0;  i2 < n123/nout[0]; i2++){
    h2c(i2,&nout[1],big->ndims-1,&iloc[1]);  /*convert to local indicies*/

    /*convert to global indicies*/
    for(iax=1; iax<big->ndims; iax++) iloc[iax]=iloc[iax]+fsect[iax];
    
    /*convert to window indicies indicies*/
    for(iax=1; iax<big->ndims; iax++) iloc[iax]=iloc[iax]-fwind[iax];
    
    /*convert to global helix number*/
    c2h(&ih,&nwind[1],big->ndims-1,&iloc[1]);  /*convert to local indicies*/

    memcpy((void*)(buf1+i2*e2*nout[0]),
      (const void*)(buf1+ih*e2*nwind[0]+fsect[0]*e2), e2*sizeof(float)*nout[0]);
  } 
  return(0);
}

int localize_sep3d(int iown, distrib *spread, sep3d *combo, sep3d *local)
{
  int idim,iax;

  if(0!=init_sep3d_struct(*combo,local,"SCRATCH"))
   seperr("trouble initializing section");

  for(idim=0; idim < spread->ndim; idim++){
    iax=spread->dff_axis[idim];

    local->o[iax]+=local->d[iax]*spread->axis_beg[iown][idim];
    local->n[iax]=spread->axis_end[iown][idim]-spread->axis_beg[iown][idim]+1;
  }
  sep3d_set_sep3d(local);
  return(0);
}

int broadcast_data(int impi, int nmpi,float *data, int n1, int n2){
int to_do,done;
int block,i;
MPI_Status status;
                                                                                      
done=0;
to_do=n1*n2;
                                                                                      
                                                                                      
i=0;
while (done<to_do){
  i++;
/*  block=MIN(n1*10,to_do-done); */
  block=to_do;
  if(impi!=0){
    MPI_Recv((data+done),block, MPI_FLOAT, impi-1,i, MPI_COMM_WORLD,&status);
  }
  if(impi!=nmpi-1){
    MPI_Send((data+done),block, MPI_FLOAT, impi+1,i, MPI_COMM_WORLD);
  }
  done+=block;
}
return(0);
}
                                                                                      


int pass_data(int impi,char *data, 
int n1, int n2,
int ifrom, int ito)
{
  int to_do,done;
  int block;
  MPI_Status status;
                                                                                      
  done=0;
  to_do=n1*n2;
                                                                                      
  while (done< to_do){
    block=MIN(to_do-done,10000000);
    if(ifrom==impi)
      MPI_Send((data+done) , block, MPI_CHAR, ito, 2333, MPI_COMM_WORLD);
    if(ito==impi)
      MPI_Recv((data+done), block, MPI_CHAR, ifrom, 2333,
        MPI_COMM_WORLD,&status);
    done+=(double)block;
  }
  return(0);
}
int calc_num_list( distrib *spread,int *i_h, int **hnum, int *inums, int *nnum)
{
  int ia,iloc,impi,nmpi,i,nold,nnew,ilast;
  MPI_Status status;
  pik *locs;

  nold=0;
  impi=sep_thread_num();
  nmpi=sep_num_thread();

  if(impi!=nmpi-1){
    MPI_Recv(&nold , 1   , MPI_INT, impi+1, 845, MPI_COMM_WORLD,&status);
    MPI_Recv(inums,nold, MPI_INT, impi+1, 846, MPI_COMM_WORLD,&status);
  }
  if(impi!=0){
    iloc=nold;
    for(ia=0; ia < spread->nown; ia++){
      memcpy((void*)(inums+iloc),(const void*)hnum[ia],sizeof(int)*i_h[ia]);
      iloc+=i_h[ia];
    }
    nold=iloc;
    MPI_Send(&nold , 1   , MPI_INT, impi-1, 845, MPI_COMM_WORLD);
    MPI_Send(inums,nold, MPI_INT, impi-1, 846, MPI_COMM_WORLD);
  }

  if(impi==0){
    locs=(pik*)malloc(sizeof(pik)*(nold));
    for(i=0; i < nold; i++){
      locs[i].tr=inums[i];
      locs[i].index=i;
    }
    if(0!=sort_drns(nold,locs)) seperr("trouble sorting numbers");
    ilast=-1;
    for(i=0,nnew=0; i < nold; i++) {
      if(locs[i].tr != ilast){
        inums[nnew]=locs[i].tr;
        ilast=locs[i].tr;
        nnew++;
      }
    }
    free(locs);
    *nnum=nnew;
  }

  if(impi!=0){
    MPI_Recv(nnum , 1   , MPI_INT, impi-1, 847, MPI_COMM_WORLD,&status);
    MPI_Recv(inums,*nnum, MPI_INT, impi-1, 848, MPI_COMM_WORLD,&status);
  }
  if(impi!=nmpi-1){
    MPI_Send(nnum , 1   , MPI_INT, impi+1, 847, MPI_COMM_WORLD);
    MPI_Send(inums,*nnum, MPI_INT, impi+1, 848, MPI_COMM_WORLD);
  }
  return(0);
}
void init_mpi_seplib(int argc, char **argv)
{
  int impi,nmpi,tempi,i;
  char buf[1000];
  FILE *infp;
  MPI_Status status;
                                                                                             
  MPI_Init(&argc,&argv);
  initpar(argc,argv);
  MPI_Comm_size(MPI_COMM_WORLD,&nmpi);
  MPI_Comm_rank(MPI_COMM_WORLD,&impi);
  if (impi==0) mpi_sep_send_args(nmpi,10,0);
  else mpi_sep_receive_args();
  /*now send arguments from distrib file*/
  if(impi==0) {
    if(1!=getch("distrib_file","s",buf))
      seperr("must provide the distrib_file");
    infp = fopen(buf,"r");
    while (NULL != fgets(buf, 999, infp)){
      tempi=1+(int)strlen(buf);
      for(i=0; i < tempi; i++) {if(buf[i]=='\n') buf[i]='\0';}
      tempi=1+(int)strlen(buf);
      MPI_Send(&tempi, 1, MPI_INT, 1, 12, MPI_COMM_WORLD);
      MPI_Send(buf , tempi, MPI_CHAR, 1, 13, MPI_COMM_WORLD);
      getch_add_string(buf);
    }
    tempi=-1; MPI_Send(&tempi, 1, MPI_INT, 1, 12, MPI_COMM_WORLD);
    fclose(infp);
  }
  else{
    tempi=1;
    while(tempi !=-1){
      MPI_Recv(&tempi, 1, MPI_INT, impi-1, 12, MPI_COMM_WORLD,&status);
      if(impi!=nmpi-1)MPI_Send(&tempi, 1, MPI_INT, impi+1, 12, MPI_COMM_WORLD);
      if(tempi>0){
        MPI_Recv(buf, tempi, MPI_INT, impi-1, 13, MPI_COMM_WORLD,&status);
        if(impi!=nmpi-1)MPI_Send(buf, tempi, MPI_INT, impi+1, 13, MPI_COMM_WORLD);
        getch_add_string(buf);
      }
    }
  }

                                                                                             
  init_3d();
  doc(SOURCE);
                                                                                             
}

int condense_data(int impi, float *buf, int nlen, int ntr, int ftr,
   int *idone, int ntraces, int *itraces, int *nhave, int *tr_have)
{
  int i,id,nh;
  nh=0;
  id=*idone;
  for(i=0; i < ntr; i++) { /*loop over the window we have read in*/
    if(ntraces > id){  /*if we haven't reached the end of the traces*/
      if(ftr+i+1==itraces[id]){/*if we have this trace*/
        tr_have[nh]=itraces[id];
        if(impi==0){
          memcpy((void*)(buf+(nh)*nlen),(const void*)(buf+(i*nlen)),
            nlen*sizeof(float));
        }
        nh++; id++;
      }
    }
  }
  *idone=id;
  *nhave=nh;
  return(0);
} 
        

