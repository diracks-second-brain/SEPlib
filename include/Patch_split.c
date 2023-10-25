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



#define HEAD "/dev/null"
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");

int localize_sep3d(int iown, distrib *spread, sep3d *combo, sep3d *local);
int transfer_sect(int isect,sep3d *big,distrib *spread,int *nwind, int *fwind, int *nsect, 
   int *fsect, sep3d  *small, int *nout, int *fout, float *buf2,float *buf1);
int pass_data(int impi,char *data, int n1, int n2,int ifrom, int ito);
int broadcast_data(int impi, int nmpi,float *data, int n1, int n2,int ifirst);
int any_data(sep3d *data, distrib *spread, int *nwind, int *fwind);
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
  int input_begin,n123,id;
  float *buf,*buf2,*buf3;
  char temp_ch[1204],hfile[10024];
  int nlen;
  MPI_Status status;

  init_mpi_seplib(argc,argv);

  if(0!=get_distrib_info(&spread,1,"out_"))
    seperr("trouble getting distribution information for output");

  impi=sep_thread_num();
  nmpi=sep_num_thread();

  /*get the input info*/
  if(0==getch("input_begin","d",&input_begin)) input_begin=1;
  if(input_begin>1) {
    /*if the input is spread across many nodes*/
    if(0!=get_distrib_info(&spread_in,0,"in_"))
        seperr("trouble getting distribution information for input");
    sects_in=(sep3d*)malloc(sizeof(sep3d)*spread_in.nown);

    if(0!= create_global_file(&spread_in,sects_in,&data))
      seperr("trouble creating global files");

    if(0!=calc_input_sections( &data, &spread_in,sects_in))
      return(sepwarn(NOT_MET,"trouble calculating sections"));
  }
  else{
    /*if the input is one the master node*/
    if(impi==0)
      if(0!= init_sep3d_tag("combo",&data,"SCRATCH"))
         return(sepwarn(NOT_MET,"trouble initializing input \n"));
     if(0!=sep3dc_broadcast_headers(&data,0))
      return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));
         grab_history("combo",hfile,10023,&nlen);
  }

  /*sections for output*/
  if(0!=calc_output_sections( &data, &spread))
    return(sepwarn(NOT_MET,"trouble calculating sections"));
   
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
    auxputhead(spread.tag_sect[spread.iown[isect]],"%s\n",hfile);
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
  if(0==getch("max_size","d",&max_size)) max_size=700;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize/e2/3;
  if(0!=init_loop_calc(data.ndims,data.n,"MAIN",max_size))
   seperr("trouble initializing loop");


  id=0;
  while(0==do_sep_loop("MAIN",nwind,fwind)){
    if(impi==0) {
      if(verb==1){
              fprintf(stderr,"Window ");
        for(i=0; i< data.ndims;i++)
           fprintf(stderr,"n%d=%d f%d=%d ",i+1,nwind[i],i+1,fwind[i]);
         fprintf(stderr,"\n");
      }
    }
    if(0==any_data(&data,&spread,nwind,fwind)){
      for(i=1,nh=1; i < data.ndims; i++) nh=nh*nwind[i];
      n123=nh*data.n[0];

      buf=(float*)malloc(e2*nh*data.n[0]*sizeof(float));
      buf2=(float*)malloc(e2*nh*data.n[0]*sizeof(float));
      buf3=(float*)malloc(e2*nh*data.n[0]*sizeof(float));

      if(input_begin==1){
        if(impi==0){
           if(0!=sep3dc_grab_headers("combo", &data,&nh,&nwind[1],
             &fwind[1],&jwind[1])) seperr("trouble reading data");
           if(0!=sep3dc_read_data("combo", &data,(void*)buf,nwind[0],
             fwind[0],jwind[0])) seperr("trouble reading data");
        } 
      }
      if(sep_thread_num() < input_begin && input_begin!=1){
        /*load the local  datasets*/
        if(0!=read_local_datasets(sects_in,&data,&spread_in,nwind,fwind,buf,buf2,buf3))
          seperr("trouble reading in local datasets");
                                                                                              
        if(0!=combo_data(impi,input_begin,buf,buf2,n123))
          seperr("trouble combining data");
      }
      if(sep_thread_num()==0 || sep_thread_num() >= input_begin){
        if(0!=broadcast_data(impi,nmpi,buf,e2*data.n[0],nh,input_begin))
         seperr("trouble passing data");

        if(impi!=0){
          for(ia=0; ia < spread.nsect; ia++){
            isect=ia;
            ierr=local_window(isect,&data,&spread,nwind,fwind,nsect,
              fsect,nout,fout);
            if(ierr==0){
              ipt=spread.ilocal_pt[ia];
              if(0!=transfer_sect(isect,&data,&spread,nwind,fwind,nsect,fsect,
                  &sects[ipt],nout,fout,buf2,buf))
                seperr("trouble transfering section");
             if(0!= sep3dc_write_data(spread.tag_sect[isect],&sects[ipt],
              (void*)buf2,nout,fout, jwind,nh,wh,wg)) 
               seperr("trouble writing outing data");
            }
          } 
        }
      }
      if(nh!=0) {
        free(buf);
        free(buf2);
        free(buf3);
      } 
    }
  }
  if(impi!=0){
    for(i=0; i < spread.nown; i++){
     if(0!=sep3dc_update_ntraces(&sects[i]))
        seperr("trouble updating traces");
     if(0!=sep3d_rite_num_traces(spread.tag_sect[spread.iown[i]],&sects[i]))
        seperr("trouble writing number of traces");
    }
  }
   
 if(impi==0){
   if(1==getch("stat_good","s",temp_ch)){
      auxputch("junk","d",&impi,temp_ch);
   }
 } 
  MPI_Finalize();
  return(0);
}

int transfer_sect(int isect,sep3d *big, distrib *spread, int *nwind,int *fwind, 
 int *nsect, int *fsect, sep3d  *small, int *nout, int *fout,float *buf2,float *buf1)
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

    memcpy((void*)(buf2+i2*e2*nout[0]),
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

int broadcast_data(int impi, int nmpi,float *data, int n1, int n2,int ifirst){
int to_do,done;
int block,i,ip;
MPI_Status status;
                                                                                      
done=0;
to_do=n1*n2;
                                                                                      
                                                                                      
i=0;
while (done<to_do){
  i++;
  block=to_do-done;
   if(impi!=nmpi-1) ip=impi+1;
   else ip=0;
  if(impi!=0){
    if(impi==ifirst) ip=0;
    else ip=impi-1;
    MPI_Recv((data+done),block, MPI_FLOAT, ip,i, MPI_COMM_WORLD,&status);
  }
  if(impi!=nmpi-1){
    if(impi==0) ip=ifirst;
    else ip=impi+1;
    MPI_Send((data+done),block, MPI_FLOAT, ip,i, MPI_COMM_WORLD);
   }
  done+=block;
}
return(0);
}
                                                                                      


int pass_data(int impi,char *data, int n1, int n2,int ifrom, int ito){
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
                                                                                


