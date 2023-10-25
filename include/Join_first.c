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

#define HEAD "/dev/null"
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");

int localize_sep3d(int iown, distrib *spread, sep3d *combo, sep3d *local);
int calc_input_sections(sep3d *big, distrib *spread,sep3d *structs);

void init_mpi_seplib(int argc, char **argv);
int main(int argc, char **argv){
  int nblock[9],dff_axis[9];
  sep3d data,*sects;
  int e2,n123,iloop;
  int impi,nmpi;
  distrib spread;
  int *fwind,*jwind,*nwind;
  int isect,ierr,ia,i,verb,idim,ibig;
  int ifirst,nfirst;
  int max_size,esize,nh,ipt,wh,wg,add;
  float **buf_in,*buf_out,*buf_temp;
  char temp_ch[1204];
  MPI_Status status;
  int restart,finished;
  int nsz,nsmall,nbig,nwrite,ido,ndo;

  init_mpi_seplib(argc,argv);
fprintf(stderr,"BEFORE GET DOIS (%d) \n",sep_thread_num());
sep_mpi_stop();
  if(0!=get_distrib_info(&spread,0,"in_"))
    seperr("trouble getting distribution information");

fprintf(stderr,"1EFORE GET DOIS (%d) \n",sep_thread_num());
sep_mpi_stop();
  impi=sep_thread_num();
  nmpi=sep_num_thread();

  sects=(sep3d*)malloc(sizeof(sep3d)*spread.nown);

fprintf(stderr,"2EFORE GET DOIS (%d) \n",sep_thread_num());
sep_mpi_stop();
  if(0!=create_global_file(&spread,sects,&data))
   seperr("trouble createing global file \n");
fprintf(stderr,"3EFORE GET DOIS (%d) \n",sep_thread_num());
sep_mpi_stop();

  if(0!=calc_input_sections( &data, &spread,sects))
    return(sepwarn(NOT_MET,"trouble calculating sections"));
fprintf(stderr,"4EFORE GET DOIS (%d) \n",sep_thread_num());
sep_mpi_stop();

 if(0==getch("finished","d",&finished)) finished=0;
 if(0==getch("restart","d",&restart)) restart=0;
 if(0==getch("add","d",&add)) add=0;
                                                                                                
  if(sep_thread_num()==0){
    auxinout("combo");
    if(add==0 && restart==0)
      if(0!=sep3dc_write_description("combo",&data))
       seperr("trouble writing description");
  }


  if(data.file_format[0]=='R') {wh=0; wg=0;}
  else seperr("Patching only functions with regular datasets for now");


  nwind=(int*) malloc(sizeof(int)*data.ndims);
  fwind=(int*) malloc(sizeof(int)*data.ndims);
  jwind=(int*) malloc(sizeof(int)*data.ndims);
  for(i=0; i < data.ndims; i++) jwind[i]=1;

        if(0==getch("verb","d",&verb)) verb=0;
  if(0==getch("max_size","d",&max_size)) max_size=500;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize/e2/2;

fprintf(stderr,"WHAT THE 0 \n");

  nsmall=data.n[0]; nbig=e2;
  for(i=1; i < data.ndims; i++) nbig=nbig*data.n[i];
  if(sep_thread_num()!=0)
    nwrite=MAX(1,MIN(nbig,1000000/spread.nown/e2/
      (spread.axis_end[0][0]-spread.axis_beg[0][0]+1)));
                                                                                
                                                                                
  buf_temp=(float*) malloc(sizeof(float)*nsmall*e2);
  buf_out=(float*) malloc(sizeof(float)*nsmall*e2);
  if(spread.nown >0){

    buf_in=(float**) malloc(sizeof(float*)*spread.nown);

    for(i=0; i < spread.nown; i++)
      ipt=spread.iown[i];
      buf_in[i]=(float*) malloc(sizeof(float)*e2*
      (spread.axis_end[ipt][0]-spread.axis_beg[ipt][0]+1));
  }
fprintf(stderr,"WHAT THE 1 %d \n",sep_thread_num());
sep_mpi_stop();

  ibig=0;
  while(ibig <  nbig){
    ndo=MIN(nbig-ibig,nwrite);
    if(verb==1) fprintf(stderr,"Working from %d to %d of %d \n",ibig,ibig+ndo,nbig);


    if(sep_thread_num()!=0){
      /*read in local data*/
      for(isect=0; isect < spread.nown; isect++){
        if(4*e2*ndo!=sreed(spread.tag_sect[spread.iown[isect]], buf_in,4*e2*ndo))
          seperr("trouble reading in data");
      }
    }

    for(ido=0; ido < ndo; ido++){
fprintf(stderr,"0HAFDS %d %d \n",ido,ndo);
sep_mpi_stop();
      if(add==1 && impi==0){
        if(4*e2*nsmall!=sreed("combo", buf_temp,4*nsmall*e2))
          seperr("trouble reading in data");
        sseek("combo",-nsmall*e2*4,1); 
      }
      else{
        for(i=0; i < nsmall*e2; i++) buf_temp[i]=0.;
      }
      for(i=0; i < nsmall*e2; i++) buf_out[i]=0.;
fprintf(stderr,"1HAFDS %d %d \n",ido,ndo);
sep_mpi_stop();
      if(impi!=0){
        /*combine the local data*/
        for(isect=0; isect < spread.nown; isect++){
          ipt=spread.iown[isect];
          nsz=spread.axis_end[ipt][0]-spread.axis_beg[ipt][0]+1;
          for(ia=0,i=spread.axis_beg[isect][0]; i <= spread.axis_beg[isect][0];i++,ia++)
            buf_out[i]=buf_in[isect][ia+nsz*ido];
        }
      }
      /*send and add*/
fprintf(stderr,"2HAFDS %d %d \n",ido,ndo);
sep_mpi_stop();
      if(impi!=nmpi-1){
         MPI_Recv(buf_temp, e2*nsmall, MPI_FLOAT, impi-1, 13, MPI_COMM_WORLD,&status);
         for(i=0; i < e2*nsmall; i++) buf_out[i]+=buf_temp[i];
      }
      if(impi!=0) MPI_Send(buf_out, e2*nsmall, MPI_FLOAT, impi+1, 13, MPI_COMM_WORLD);

fprintf(stderr,"3HAFDS %d %d \n",ido,ndo);
sep_mpi_stop();

      /*write out data*/
      if(sep_thread_num()==0){
        if(e2*4*nsmall!=srite("combo",buf_out,e2*nsmall*4))
          seperr("trouble writing out combo");
      }
fprintf(stderr,"4HAF2S  \n");
sep_mpi_stop();
   }
   ibig+=ndo;
 }
  if(impi==0)
    if(1==getch("stat_good","s",temp_ch)) auxputch("junk","d",&impi,temp_ch);
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

