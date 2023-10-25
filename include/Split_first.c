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
#include<sep_par.h>
int time_base,time_read,time_rite,time_send,time_rect,time_sect;
int my_internal_send=123;



#define HEAD "/dev/null"
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");

int localize_sep3d(int iown, distrib *spread, sep3d *combo, sep3d *local);
int broadcast_data(int impi, int nmpi,float *data, int n1, int n2);
void init_mpi_seplib(int argc, char **argv);

int main(int argc, char **argv){
  int nblock[9],dff_axis[9];
  sep3d data,*sects,*sects_in;
  int e2;
  int impi,nmpi;
  distrib spread,spread_in;
  int *fwind,*jwind,*nwind;
  int *fsect,*nsect;
  int *fout,*nout;
  int isect,ierr,ia,i,verb;
  int max_size,esize,nh,ipt,wh,wg;
  int nbig,nsmall,nwrite,iwrite;
  int input_begin,n123,id,nsz,ibig;
  float *buf_in,**buf_out;
  char temp_ch[1204];
  MPI_Status status;

  init_mpi_seplib(argc,argv);

  if(0!=get_distrib_info(&spread,1,"out_"))
    seperr("trouble getting distribution information for output");

  impi=sep_thread_num();
  nmpi=sep_num_thread();

  /*get the input info*/
    /*if the input is one the master node*/
    if(impi==0)
      if(0!= init_sep3d_tag("combo",&data,"SCRATCH"))
         return(sepwarn(NOT_MET,"trouble initializing input \n"));
     if(0!=sep3dc_broadcast_headers(&data,0))
      return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));

  /*sections for output*/
  if(0!=calc_output_sections( &data, &spread))
    return(sepwarn(NOT_MET,"trouble calculating sections"));

  /*create the local files*/
  sects=(sep3d*)malloc(sizeof(sep3d)*spread.nown);
  for(isect=0; isect < spread.nown; isect++){
    if(0!=localize_sep3d(spread.iown[isect],&spread,&data,&sects[isect]))
      seperr("trouble localizing struct");
    if(impi!=0){
      if(0!=sep3dc_write_description(spread.tag_sect[spread.iown[isect]],
        &sects[isect])) 
         seperr("trouble writing description for %s",
           spread.tag_sect[spread.iown[isect]]);
    }
  }

  if(data.file_format[0]=='R') {wh=0; wg=0;}
  else seperr("Patching only works for regular data");


  nwind=(int*) malloc(sizeof(int)*data.ndims);
  fwind=(int*) malloc(sizeof(int)*data.ndims);
  jwind=(int*) malloc(sizeof(int)*data.ndims);
  nsect=(int*) malloc(sizeof(int)*data.ndims);
  fsect=(int*) malloc(sizeof(int)*data.ndims);
  nout=(int*) malloc(sizeof(int)*data.ndims);
  fout=(int*) malloc(sizeof(int)*data.ndims);
  for(i=0; i < data.ndims; i++) jwind[i]=1;

  if(0==getch("verb","d",&verb)) verb=0;
  if(0==getch("max_size","d",&max_size)) max_size=500;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize/e2/3;



  nsmall=data.n[0]; nbig=e2;
  for(i=1; i < data.ndims; i++) nbig=nbig*data.n[i];
  i=spread.nown;
  MPI_Bcast(&i,1, MPI_INT, 1, MPI_COMM_WORLD);

  nwrite=MAX(1,MIN(nbig,1000000/e2/i/
    (spread.axis_end[0][0]-spread.axis_beg[0][0]+1)));


  buf_in=(float*) malloc(sizeof(float)*nsmall*e2);
  if(spread.nown >0 &&sep_thread_num() !=0){
    buf_out=(float**) malloc(sizeof(float*)*spread.nown);
    for(i=0; i < spread.nown; i++)
      ipt=spread.iown[i];
      buf_out[i]=(float*) malloc(sizeof(float)*e2*nwrite*
      (spread.axis_end[ipt][0]-spread.axis_beg[ipt][0]+1));
  }

  for(ibig=0; ibig < nbig;ibig++){
    if(verb==1 && sep_thread_num()==0) fprintf(stderr,"Working on %d of %d \n",ibig,nbig);

    
    /* sreed_data*/
    if(sep_thread_num()==0){
      if(4*e2*nsmall!=sreed("combo",buf_in,4*e2*nsmall))
        seperr("trouble reading in data \n");

    }

    /*send data*/

    if(0!=broadcast_data(impi,nmpi,buf_in,e2,nsmall))
      seperr("trouble broadcasting data");


    /*split the data*/
    if(sep_thread_num()!=0){
      for(i=0; i < spread.nown; i++){
          ipt=spread.iown[i];
          nsz=spread.axis_end[ipt][0]-spread.axis_beg[ipt][0]+1;;
      memcpy((void*)&buf_out[i][0],
           (const void*)&buf_in[spread.axis_beg[ipt][0]*e2], nsz*e2*sizeof(float));
      }
      iwrite+=1;
    }

    /* write_if_applicapable*/
/*
    if(sep_thread_num()!=0){
      if(iwrite==nwrite || ibig+1==nbig){
         for(i=0; i < spread.nown; i++){  
            ipt=spread.iown[i];
            nsz=spread.axis_end[ipt][0]-spread.axis_beg[ipt][0]+1;
            if(4*e2*nsz*iwrite!=srite(spread.tag_sect[ipt],buf_out[i],
              iwrite*4*e2*nsz)) seperr("trouble writing out buffer");

         }
         iwrite=0;
      }
    }
*/
  }
 if(impi==0){
   if(1==getch("stat_good","s",temp_ch)){
      auxputch("junk","d",&impi,temp_ch);
   }
 } 
  MPI_Finalize();
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
int block,i,ip;
MPI_Status status;
                                                                                      
to_do=n1*n2;

i=my_internal_send;
                                                                                      
                                                                                      
  block=to_do;
  if(impi!=0){
    MPI_Recv((void*)data,block, MPI_FLOAT, impi-1,i+2, MPI_COMM_WORLD,&status);
  }
  if(impi!=nmpi-1){
    MPI_Send((const void*)data,block, MPI_FLOAT, impi+1,i+2, MPI_COMM_WORLD);
  }
my_internal_send=i+2;
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
                                                                                
