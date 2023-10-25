/*
=head1 NAME
 
 Copy_split
   
=head1 SYNOPSIS
 
  Copy_split combo=  
 
=head1 PARAMTERS
 
=over 4
 
=item assign - integer*
 
  thread to assign a given section to
 
=item max_size - integer
 
 [100]  maximum memory in megabyte to use on node (max memory will then be 2x)
 
=item axis - integer
 
 [2]  axis to distribute over
 
=item pattern - char*
 
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

#define HEAD "/dev/null"
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("     Copy_split");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("      Copy_split combo=  ");\
 sep_add_doc_line("");\
 sep_add_doc_line("PARAMTERS");\
 sep_add_doc_line("    assign - integer*");\
 sep_add_doc_line("          thread to assign a given section to");\
 sep_add_doc_line("");\
 sep_add_doc_line("    max_size - integer");\
 sep_add_doc_line("         [100]  maximum memory in megabyte to use on node (max memory will then be 2x)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    axis - integer");\
 sep_add_doc_line("         [2]  axis to distribute over");\
 sep_add_doc_line("");
int broadcast_data_old(float *data, int n1, int n2);
int broadcast_data(int impi,int nmpi,float *data, int n1, int n2);
void init_mpi_seplib(int argc, char **argv);

int main(int argc, char **argv){
  char *name;
  int nmpi, impi,esize,e2,nh,i,wh,wg,verb;
  int *nwind,*fwind,*jwind;
  float *buf;
  sep3d data;
  char temp_ch[512],my_tag[512],string[512];
  int max_size;
  MPI_Status status;
  char hfile[10024];
  int nlen;


  /*initialize MPI*/
  init_mpi_seplib(argc,argv);


  init_3d();
  MY_SEP_DOC
  doc(SOURCE);
  sep3d_initialize(&data);
  impi=sep_thread_num();
  nmpi=sep_num_thread();



  /*read in the size*/
  if(impi==0) {
getch("combo","s",temp_ch);
    if(0!= init_sep3d_tag("combo",&data,"SCRATCH"))
       return(sepwarn(NOT_MET,"trouble initializing input \n"));
       grab_history("combo",hfile,10023,&nlen);
  }


  
  if(impi!=0){ 
    sprintf(temp_ch,"out_datapath_sect%d",impi-1);
    if(0==getch(temp_ch,"s",my_tag))
      seperr("%s not specified",temp_ch);
    sprintf(string,"datapath=%s",my_tag);
    getch_add_string(string);
    sprintf(temp_ch,"out_tag_sect%d",impi-1);
    if(0==getch(temp_ch,"s",my_tag))
      seperr("%s not specified",temp_ch);
  }
  if(0!=sep3dc_broadcast_headers(&data,0))
    return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));

  if(data.file_format[0]=='R') {wh=0; wg=0;}
  else if(data.file_format[0]=='H') {wh=1; wg=0;}
  else if(data.file_format[0]=='G') {wh=1; wg=1;}
  else seperr("Unknown data type");

   if(impi!=0){
     MPI_Recv(&nlen,1,MPI_INT,impi-1, 9999,MPI_COMM_WORLD,&status);
     MPI_Recv(hfile,nlen,MPI_CHAR,impi-1, 3999,MPI_COMM_WORLD,&status);
   }
   if(impi!=nmpi-1){
     MPI_Send(&nlen,1,MPI_INT,impi+1,9999, MPI_COMM_WORLD);
     MPI_Send(hfile,nlen,MPI_CHAR,impi+1,3999, MPI_COMM_WORLD);
   }
  if(sep_thread_num()!=0){
    auxout(my_tag);
    auxclose(my_tag);
    auxputhead(my_tag,"-----------Copied from input file--------");
    auxputhead(my_tag,"%s\n",hfile);
    if(0!=sep3dc_write_description(my_tag,&data))
     seperr("trouble writing description");
  }

  jwind=(int*)malloc(data.ndims*sizeof(int));
  nwind=(int*)malloc(data.ndims*sizeof(int));
  fwind=(int*)malloc(data.ndims*sizeof(int));
  for(i=0; i< data.ndims; i++) jwind[i]=1;
  if(0==getch("verb","d",&verb)) verb=0;
  if(0==getch("max_size","d",&max_size)) max_size=50;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize;


  if(0!=init_loop_calc(data.ndims,data.n,"MAIN",max_size))
    seperr("trouble initializing loop calc");


  while(0==do_sep_loop("MAIN",nwind,fwind)){
    if(verb==1 && sep_thread_num()==0){
      fprintf(stderr,"Window ");
      for(i=0;i< data.ndims;i++)
        fprintf(stderr,"n%d=%d f%d=%d  ",i+1,nwind[i],i+1,fwind[i],data.n[i]);
      fprintf(stderr,"\n");
    }
    if(impi==0){ 
      if(0!= sep3dc_grab_headers("combo",&data,&nh,&nwind[1],&fwind[1],jwind))
        seperr("trouble grabbing data");
    }
    if(impi!=0){
      MPI_Recv(&nh,1,MPI_INT,impi-1, 9999,MPI_COMM_WORLD,&status);
      if(0!=sep3dc_pass_headers(&data,impi-1,impi))
        return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));
    }
    if(impi!=nmpi-1){
      MPI_Send(&nh,1,MPI_INT,impi+1,9999, MPI_COMM_WORLD);
      if(0!=sep3dc_pass_headers(&data,impi,impi+1))
        return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));
    }
    if(nh!=0){
      buf=(float*)malloc(sizeof(float)*nh*nwind[0]*e2);
      if(impi==0){
        if(0!=sep3dc_read_data("combo",&data,(void*)buf,nwind[0],
          fwind[0],jwind[0])) seperr("trouble reading data");
      }
      if(0!=broadcast_data(impi,nmpi,buf,data.n[0]*e2,nh))
        return(sepwarn(NOT_MET,"trouble broadcasting data"));
    } 
    else buf=0;
    if(impi!=0){
      if(0!= sep3dc_write_data(my_tag,&data,
         (char*)buf,nwind,fwind, jwind,nh,wh,wg)) 
         seperr("trouble writing outing data");
    }
    if(nh>0) free(buf);
  }

  if(impi!=0){
    if(0!=sep3dc_update_ntraces(&data))
      seperr("trouble updating traces");
    if(0!=sep3d_rite_num_traces(my_tag,&data))
      seperr("trouble writing number of traces");
  }


  sep_mpi_stop();

  if(impi==0){
    if(1==getch("stat_good","s",temp_ch)){
      auxputch("junk","d",&impi,temp_ch);
    }
  }

  MPI_Finalize();

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
  block=MIN(to_do-done,20000000);
  if(impi!=0) 
    MPI_Recv((data+done),block, MPI_FLOAT, impi-1,i, MPI_COMM_WORLD,&status);
fprintf(stderr,"DOne receive %d %d\n",impi,done);
  if(impi!=nmpi-1)
    MPI_Send((data+done),block, MPI_FLOAT, impi+1,i, MPI_COMM_WORLD);
fprintf(stderr,"DOne send %d %d\n",impi,done);
  done+=block;
}
return(0);
}


int broadcast_data_old(float *data, int n1, int n2){
int to_do,done;
int block;

done=0;
to_do=n1*n2;

while (done<to_do){
  block=to_do-done;
  MPI_Bcast((data+done),block, MPI_FLOAT, 0, MPI_COMM_WORLD);
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


