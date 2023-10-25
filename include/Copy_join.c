#include<seplib.h>
#include<sep3dc.h>
#include<sepaux.h>
#include<superset.h>
#ifndef SEPNULL2
#define SEPNULL2 ((void *) 0)
#endif
#include<mpi.h>

#define BUF_SIZE 50000000
#define HEAD "/dev/null"
#define MY_SEP_DOC 
int compress_data(int impi, int nmpi,float *data,int n1, int n2,float *buf);
void init_mpi_seplib(int argc, char **argv);
int compress_data_old(float *data, float *data2,int n1, int n2);
int main(int argc, char **argv){
  char *name;
  int nmpi, impi,esize,e2,nh,i,wh,wg,add,restart,ifinish,iloop;
  int *nwind,*fwind,*jwind;
  int max_size,verb;
  float *buf;
  sep3d data;
  char temp_ch[512],my_tag[512];
  MPI_Status status;
  float *buf_temp;
   char hfile[10024];
   int nlen;

  buf_temp=(float*)malloc(sizeof(float)*BUF_SIZE);

  /*initialize MPI*/
  init_mpi_seplib(argc,argv);
  MPI_Comm_size(MPI_COMM_WORLD,&(nmpi));
  MPI_Comm_rank(MPI_COMM_WORLD,&(impi));

  init_3d();
  MY_SEP_DOC
  doc(SOURCE);


  if(0==getch("finished","d",&ifinish)) ifinish=0;
  if(0==getch("restart","d",&restart)) restart=0;
  if(0==getch("verb","d",&verb)) verb=0;
  if(0==getch("add","d",&add)) add=0;

  /*read in the space*/
  if(impi==0) {
          grab_history("combo",hfile,10023,&nlen);  
     strcpy(my_tag,"combo");
  }else{ 
    sprintf(temp_ch,"in_tag_sect%d",impi-1);
    if(0==getch(temp_ch,"s",my_tag))
      seperr("%s not specified",temp_ch);
  }
 
  if(impi!=0)
    if(0!= init_sep3d_tag(my_tag, &data,"SCRATCH"))
       return(sepwarn(NOT_MET,"trouble initializing input \n"));
  if(impi < 2){
    if(0!=sep3dc_pass_headers(&data, 1,0))
       return(sepwarn(NOT_MET,"trouble passing headers \n"));
   }

  if(data.file_format[0]=='R') {wh=0; wg=0;}
  else if(data.file_format[0]=='H') {wh=1; wg=0;}
  else if(data.file_format[0]=='G') {wh=1; wg=1;}
  else seperr("Unknown data type %s \n",data.file_format);


   if(impi==0){
     MPI_Recv(&nlen,1,MPI_INT,impi+1, 9999,MPI_COMM_WORLD,&status);
     MPI_Recv(hfile,nlen,MPI_CHAR,impi+1, 3999,MPI_COMM_WORLD,&status);
   }
   if(impi==1){
     grab_history(my_tag,hfile,10023,&nlen);
     MPI_Send(&nlen,1,MPI_INT,impi-1,9999, MPI_COMM_WORLD);
     MPI_Send(hfile,nlen,MPI_CHAR,impi-1,3999, MPI_COMM_WORLD);
   }
  if(sep_thread_num()==0){

    if(add==0 && restart==0){
       fprintf(stderr,"DIE 1 \n");
          auxclose("combo");
          auxinout("combo");
          auxputhead("combo","-----------Copied from input file--------");
    fprintf(stderr,"DFIE2 \n");
      auxputhead("combo","%s\n",hfile);
      fprintf(stderr,"DIE34\n");
      if(0!=sep3dc_write_description("combo",&data))
       seperr("trouble writing description");
  }
  }
    

  jwind=(int*)malloc(data.ndims*sizeof(int));
  nwind=(int*)malloc(data.ndims*sizeof(int));
  fwind=(int*)malloc(data.ndims*sizeof(int));
  for(i=0; i< data.ndims; i++) jwind[i]=1;
  if(0==getch("max_size","d",&max_size)) max_size=10;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize;

  if(0!=init_loop_calc(data.ndims,data.n,"MAIN",max_size))
    seperr("trouble initializing loop calc");

  iloop=0;
  while(0==do_sep_loop("MAIN",nwind,fwind)){
    iloop++;
    if(verb==1 && impi==0) {
        if(ifinish >= iloop) fprintf(stderr,"Skipping ");
      else fprintf(stderr,"Window ");
      for(i=0; i< data.ndims;i++)
         fprintf(stderr,"n%d=%d f%d=%d ",i+1,nwind[i],i+1,fwind[i]);
       fprintf(stderr,"\n");
    }
    if(ifinish <iloop){
      if(impi!=0 || add==1){ 
        if(0!= sep3dc_grab_headers(my_tag,&data,&nh,&nwind[1],&fwind[1],jwind))
          seperr("trouble grabbing data");
      }
      if(add==0 && impi < 2){
        if(impi==1) MPI_Send(&nh,1,MPI_INT,0,234,MPI_COMM_WORLD);
        else MPI_Recv(&nh,1,MPI_INT,1,234,MPI_COMM_WORLD,&status);
        if(0!=sep3dc_pass_headers(&data,1,0))
          return(sepwarn(NOT_MET,"trouble passing headers\n"));
      }
      if(nh!=0){
        buf=(float*)malloc(sizeof(float)*nh*nwind[0]*e2);
        if(impi!=0 || add==1){
          if(0!=sep3dc_read_data(my_tag,&data,(void*)buf,nwind[0],
            fwind[0],jwind[0])) seperr("trouble reading data");
        }
        else{
          for(i=0; i < e2*nwind[0]*nh;i++) buf[i]=0.;
        }
        if(0!=compress_data(impi,nmpi,buf,nwind[0]*e2,nh,buf_temp))
          return(sepwarn(NOT_MET,"trouble broadcasting data"));
      } 
      else buf=0;
      if(impi==0){
        if(0!= sep3dc_write_data(my_tag,&data,
           (char*)buf,nwind,fwind, jwind,nh,wh,wg)) 
           seperr("trouble writing outing data");
         fprintf(stderr,"finished=%d\n",iloop);
      }
      if(nh>0) free(buf);
    }
/*         fprintf(stderr,"thread=%d finished=%d\n",impi,iloop);*/
  }
  if(impi==0){
    if(0!=sep3dc_update_ntraces(&data))
      seperr("trouble updating traces");
    if(0!=sep3d_rite_num_traces(my_tag,&data))
      seperr("trouble writing number of traces");
  }

                                                                                


  MPI_Finalize();
fprintf(stderr,"AFTER FINALIZE %d \n",impi);
 if(impi==0){
   if(1==getch("stat_good","s",temp_ch)){
      auxputch("junk","d",&impi,temp_ch);
   }
 }

  return(0);
}

int compress_data(int impi, int nmpi,float *data,int n1, int n2,float *data2){
int to_do,done;
int block,i,imsg;
MPI_Status status;
                                                                                
done=0;
to_do=n1*n2;
                                                                                
imsg=0;
while (done<to_do){
  imsg++;
  block=to_do-done;
  if(impi!=nmpi-1){
    MPI_Recv(data2,block, MPI_FLOAT, impi+1,imsg, MPI_COMM_WORLD,&status);
    for(i=0; i < block; i++) data[i+done]+=data2[i];
  }
  if(impi!=0)
    MPI_Send((data+done),block, MPI_FLOAT, impi-1,imsg, MPI_COMM_WORLD);
  done+=block; 
}
return(0);
}
int compress_data_old(float *data, float *data2,int n1, int n2){
int to_do,done;
int block;
                                                                                
done=0;
to_do=n1*n2;
                                                                                
while (done<to_do){
  block=MIN(to_do-done,10000000);
   fprintf(stderr,"CHCK THE READ %d %f %f %f %f \n",sep_thread_num(),data2[0],
   data2[10000],data2[50000],data[60000]);
  MPI_Reduce((void*)(data+done),(void*)(data2+done),block,MPI_FLOAT,MPI_SUM,0,
    MPI_COMM_WORLD);
/*sep_mpi_stop();*/
  done+=block; 
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



