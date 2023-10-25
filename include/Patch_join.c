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
int patch_boundary(sep3d *data,distrib *spread,int *nwind,  int isect, int *nsect, int *fout,int *fsect, float *buf2);
int calc_input_sections(sep3d *big, distrib *spread,sep3d *structs);

void init_mpi_seplib(int argc, char **argv);
int main(int argc, char **argv){
  int nblock[9],dff_axis[9];
  sep3d data,*sects;
  int e2,n123,iloop;
  int impi,nmpi;
  distrib spread;
  int *fwind,*jwind,*nwind;
  int isect,ierr,ia,i,verb,idim;
  int max_size,esize,nh,ipt,wh,wg,add;
  float *buf1,*buf2,*buf3;
  char temp_ch[1204];
  MPI_Status status;
  int restart,finished;
  char hfile[10024];
  int nlen;

  init_mpi_seplib(argc,argv);
  if(0!=get_distrib_info(&spread,0,"in_"))
    seperr("trouble getting distribution information");

  impi=sep_thread_num();
  nmpi=sep_num_thread();

  sects=(sep3d*)malloc(sizeof(sep3d)*spread.nown);

  if(0!=create_global_file(&spread,sects,&data))
   seperr("trouble createing global file \n");

  if(0!=calc_input_sections( &data, &spread,sects))
    return(sepwarn(NOT_MET,"trouble calculating sections"));

 if(0==getch("finished","d",&finished)) finished=0;
 if(0==getch("restart","d",&restart)) restart=0;
 if(0==getch("add","d",&add)) add=0;
    
    
   if(impi==0){
     MPI_Recv(&nlen,1,MPI_INT,impi-1, 9999,MPI_COMM_WORLD,&status);
     MPI_Recv(hfile,nlen,MPI_CHAR,impi-1, 3999,MPI_COMM_WORLD,&status);
   }
   if(impi==1){
     grab_history(spread.tag_sect[spread.iown[0]],hfile,10023,&nlen);
     MPI_Send(&nlen,1,MPI_INT,impi+1,9999, MPI_COMM_WORLD);
     MPI_Send(hfile,nlen,MPI_CHAR,impi+1,3999, MPI_COMM_WORLD);
   }
  if(sep_thread_num()==0){
/*    auxinout("combo");*/
    auxputhead("combo","-----------Copied from input file--------");
    auxputhead("combo","%s\n",hfile);
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

  if(0!=init_loop_calc(data.ndims,data.n,"MAIN",max_size))
   seperr("trouble initializing loop calc");



  iloop=0;
  while(0==do_sep_loop("MAIN",nwind,fwind)){
    iloop=iloop+1;
    for(idim=0,n123=e2; idim < data.ndims; idim++){
       n123=n123*nwind[idim];
    }
    if(iloop==1){
      buf1=(float*) malloc(sizeof(float)*n123*e2);
      buf2=(float*) malloc(sizeof(float)*n123*e2);
      buf3=(float*) malloc(sizeof(float)*n123*e2);
    }
    if(verb==1 && impi%10==0) {
      if(finished >= iloop) fprintf(stderr,"Skipping ");
      else fprintf(stderr,"Window (%d) ",impi);
      for(i=0; i< data.ndims;i++)
         fprintf(stderr,"n%d=%d f%d=%d ",i+1,nwind[i],i+1,fwind[i]);
       fprintf(stderr,"\n");
    }
    if(finished < iloop){

      /*load the local  datasets*/
      if(impi!=0){
        if(0!=read_local_datasets(sects,&data,&spread,nwind,fwind,buf1,buf2,buf3))
         seperr("trouble reading in local datasets");
      }


       if(0!=combo_data(impi,nmpi,buf1,buf2,n123))
         seperr("trouble combining data");

     if(impi==0){
       if(add==1) {
         if(0!=sep3dc_grab_headers("combo", &data,&nh,&nwind[1],
            &fwind[1],&jwind[1])) seperr("trouble reading data");
         if(0!=sep3dc_read_data("combo", &data,(void*)buf2,nwind[0],
            fwind[0],jwind[0])) seperr("trouble reading data");
         for(i =0;i < n123; i++) buf1[i]=buf1[i]+buf2[i];
       }
       if(0!= sep3dc_write_data("combo",&data,
         (void*)buf1,nwind,fwind,jwind,n123/data.n[0]/e2,wh,wg)) 
           seperr("trouble writing outing data");
         fprintf(stderr,"finished=%d\n",iloop);

      }
    }
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

