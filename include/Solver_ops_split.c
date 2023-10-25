/*<
head1 NAME

Solver_ops_par - Mathematical operations for out-of-core solver

=head1 SYNOPSIS

Solver_ops_par  file1=in.H  op= [file2= scale1_r= scale2_r= scale2_i= scale1_i=]

=head1 INPUT PARAMETERS

=over 4

=item  file1-  sepfile

       Seplib file to perform mathematical operation on 

=item  op-  char*

       Mathematical operation (dot, multiply, zero, add, scale_add_scale,scale_add,scale)

=item  file2-  sepfile

       Seplib file used for mathematical operation

=item  scale1_r,scale2_r,scale1_i,scale_i-  float

       Scalars to apply during mathematical operation

=item  verb-  logical

       [0] Verbosity

=back

=head1 DESCRIPTION

 Apply simple mathematical operation on SEPlib files needed

 for solver operations.

L<Math>

=head1 CATEGORY

B<util/par>
>*/
#include<seplib.h>
#include<sep_par.h>
#include<sep3dc.h>
#include<sepaux.h>
#include<superset.h>
#include<time.h>
#ifndef SEPNULL2
#define SEPNULL2 ((void *) 0)
#endif
#include<mpi.h>

#define BUF_SIZE 10000000
#define HEAD "/dev/null"
#define MY_SEP_DOC 
void init_mpi_seplib(int argc, char **argv);

int main(int argc, char **argv){
  char *name;
  int nmpi, impi,esize,e2,nh,i,wh,wg,add;
  int *nwind,*fwind,*jwind;
  int max_size,verb;
  float *buf;
  sep3d *file1_s,*file2_s;
  char temp_ch[512],my_tag[512];
  MPI_Status status;
  float *buf_temp;
  distrib spread1,spread2; 
  char op[128],file1[1024],file2[1024];
  sep3d *sects1,*sects2,data1,data2;
  int two_file=1;
  double dot1r,dot2r;
  double dot1c_r,dot1c_i;
  double temp1;
  int *sect_map_1_2,j;
  int maxsize,block,ipt,ipt2,nblock;
  long long n123,ndone,nsect;
  float *buf1,*buf2;
  float scale1_r,scale1_i,scale2_r,scale2_i;
  int isect,ifound;
  float t1r,t1i,t2r,t2i;
  


  /*initialize MPI*/
  init_mpi_seplib(argc, argv);
  MPI_Comm_size(MPI_COMM_WORLD,&(nmpi));
  MPI_Comm_rank(MPI_COMM_WORLD,&(impi));

  if(0==getch("verb","d",&verb)) verb=0;

  if(0==getch("op","s",op)) seperr("must specify op");
  if(0==getch("file1","s",file1)) seperr("must specify file1");
  if(0==getch("file2","s",file2)) strcpy(file2,"none");

  if(0!=get_distrib_info(&spread1,1,"file1_"))
    seperr("trouble getting distribution information");
  sects1=(sep3d*)malloc(sizeof(sep3d)*spread1.nown);
  if(0!=create_global_file(&spread1,sects1,&data1))
   seperr("trouble createing global file \n");

  if(0!=strcmp(file2,"none")){
    if(0!=get_distrib_info(&spread2,0,"file2_"))
      seperr("trouble getting distribution information");
  sects2=(sep3d*)malloc(sizeof(sep3d)*spread2.nown);
    if(0!=create_global_file(&spread2,sects2,&data2))
     seperr("trouble createing global file \n");
  }

                                                                                
  if(0==getch("scale1_r","f",&scale1_r)) scale1_r=1.;
  if(0==getch("scale2_r","f",&scale2_r)) scale2_r=1.;
  if(0==getch("scale1_i","f",&scale1_i)) scale1_i=0.;
  if(0==getch("scale2_i","f",&scale2_i)) scale2_i=0.;


  if (0==strcmp(op,"dot")){
    if(0==strcmp("none",file2)) two_file=0;
    dot1r=0.;
    dot1c_r=0.;
    dot1c_i=0.;
  } 
  else if (0==strcmp(op,"multiply")) {}
  else if (0==strcmp(op,"zero")) two_file=0;
  else if (0==strcmp(op,"add")) {}
  else if (0==strcmp(op,"scale_addscale")) {}
  else if (0==strcmp(op,"scale_add")) {}
  else if(0== strcmp(op,"scale")) two_file=0;
  else if(0== strcmp(op,"random")){
     two_file=0;
     srand((unsigned)time(NULL));
  }
  else seperr("unknown op %s",op);

  


  e2=sep3dc_get_esize(&data1)/4;
  maxsize=10000000; 
  if(two_file==1) {
    maxsize=maxsize/2;
    if(spread1.nsect != spread2.nsect) 
      seperr("number of sections must be the same for the two files");
    for(i=0; i < MIN(data1.ndims,data2.ndims); i++){
     if(data1.n[i] != data2.n[i])
       seperr("Expecting datasets to be of the same size n%d = (%d,%d)",
        i+1,data1.n[i],data2.n[i]);
    }
    for(i=MIN(data1.ndims,data2.ndims); i < MAX(data1.ndims,data2.ndims); i++){
      if(data1.ndims > i )
        if(data1.n[i]!=1) seperr("Dataset 1 to many dimensions");
      if(data2.ndims > i )
        if(data2.n[i]!=1) seperr("Dataset 2 to many dimensions");
    }
    sect_map_1_2=(int*)malloc(spread1.nsect*sizeof(int));
    for(j=0; j < spread1.nsect; j++){ 
     for(ifound=0,i=0;i < spread2.nsect; i++){
       if(spread1.isect[j]==spread2.isect[i]) {
         sect_map_1_2[j]=i; ifound=1;
       }
     }
    }
     if(ifound==0) seperr("can't find section %d in data1 in data2 \n",j);
  }


  for(i=0,n123=1; i < data1.ndims; i++) {
     n123=n123*(long long)data1.n[i];
  }

  if(sep3dc_get_esize(&data1)==8) n123=n123*(long long)2;


/*  if(maxsize > n123) maxsize=n123;*/
  buf1=(float*)malloc(sizeof(float)*maxsize);
  if(two_file==1)
    buf2=(float*)malloc(sizeof(float)*maxsize);
  else buf2=buf1;
  for(isect=0; isect < spread1.nsect; isect++){


  }

  /*loop over the sections*/
  for(isect=0; isect < spread1.nsect; isect++){
    ipt=spread1.ilocal_pt[isect];
    if(two_file==1) ipt2=spread2.ilocal_pt[sect_map_1_2[isect]];
    else ipt2=-1;
    if(sep_thread_num()==0){ ipt=-1; ipt2=-1;}
    if(ipt2 >=0 || ipt >=0) { /*we have this section*/
      if(ipt>=0){
        for(nsect=e2,i=0; i < sects1[ipt].ndims; i++) 
          nsect=nsect*(long long)sects1[ipt].n[i];
      }
      if(ipt2>=0){
        for(nsect=e2,i=0; i < sects2[ipt2].ndims; i++) 
          nsect=nsect*(long long)sects2[ipt2].n[i];
     }
     ndone=0;
    if(ipt >=0)
     while(ndone < nsect){
       nblock=(int)MIN((nsect-ndone),(long long)maxsize);
       if(0!=strcmp(op,"zero")){
         if(ipt>=0){ /*we own  file1*/
           if(4*nblock!=sreed(spread1.tag_sect[isect],buf1,nblock*4))
             seperr("trouble reading from %s thread=%d  \n",
               spread1.tag_sect[isect],sep_thread_num());
         }
         if(ipt2>=0){ /*we own  file1*/
           if(4*nblock!=sreed(spread2.tag_sect[sect_map_1_2[isect]],buf2,
             nblock*4)) seperr("trouble reading from %s thread=%d  \n",
               spread2.tag_sect[sect_map_1_2[isect]],sep_thread_num());
           if(ipt==-1){ /*we need to pass this section to another node*/
             MPI_Send(buf2,nblock,MPI_FLOAT,spread1.sect_thread[isect],isect,
               MPI_COMM_WORLD); 
           }
         }
         else if(two_file==1 ){ /*we need to receive this file*/
           MPI_Recv(buf2,nblock,MPI_FLOAT,
            spread2.sect_thread[sect_map_1_2[isect]],isect,MPI_COMM_WORLD,
                &status);
         }
       }
       if(ipt>=0){ /*if we are holding file1*/
   
         if(0==strcmp(op,"dot")){
           if(e2==1){
             for(i=0; i < nblock; i++)   dot1r=dot1r+buf1[i];
           }
           else{
             for(i=0; i < nblock/2; i++) {
               dot1c_r+=buf1[i*2]*buf2[i*2];
               dot1c_i-=buf1[i*2]*buf2[i*2+1];
               dot1c_r+=buf1[i*2+1]*buf2[i*2+1];
               dot1c_i+=buf1[i*2+1]*buf2[i*2];
             }
           } 
         }
         else if(0==strcmp(op,"multiply")){
           if(e2==1){ 
             for(i=0; i < nblock; i++){
                buf1[i]=buf1[i]*buf2[i];
             }
           }
           else{
             for(i=0; i < nblock/2; i++) {
               buf1[i*2]=buf1[i*2]*buf2[i*2]- /* a.r=a.r*b.r  + */
                 buf1[i*2+1]*buf2[i*2+1];     /*     a.i*b.i   */
               buf1[i*2+1]=buf1[i*2]*buf2[i*2+1]+ /* a.i=a.r*b.i  + */
                 buf1[i*2+1]*buf2[i*2];     /*     a.i*b.r   */
             }
           }
         }
         else if (0==strcmp(op,"zero")){
           for(i=0; i < nblock; i++) buf1[i]=0.;
         }
         else if (0==strcmp(op,"random")){
           for(i=0; i < nblock; i++) buf1[i]=(double)(rand()%20000)/20000.-1.;
         }
         else if(0==strcmp(op,"add")){
           for(i=0; i < nblock; i++) buf1[i]+=buf2[i];
         }
         else if(0==strcmp(op,"scale_addscale")){
           if(e2==1){
             for(i=0; i < nblock; i++)buf1[i]=buf1[i]*scale1_r+
              scale2_r*buf2[i];
           }
           else{ 
             for(i=0; i < nblock/2; i++){
               t1r=scale1_r*buf1[i*2]  -scale1_i*buf1[i*2+1];
               t2r=scale2_r*buf2[i*2]  -scale2_i*buf2[i*2+1];
               t1i=scale1_r*buf1[i*2+1]+scale1_i*buf1[i*2];
               t2i=scale2_r*buf2[i*2+1]+scale2_i*buf2[i*2];
               buf1[2*i]=t1r+t2r;
               buf1[2*i+1]=t1i+t2i;
             }
           }
         }
         else if(0==strcmp(op,"scale_add")){
           if(e2==1){
             for(i=0; i < nblock; i++)buf1[i]=buf1[i]+scale1_r*buf2[i];
           }
           else{ 
             for(i=0; i < nblock/2; i++){
               t1r=scale1_r*buf2[i*2]  -scale1_i*buf2[i*2+1];
               t1i=scale1_r*buf2[i*2+1]+scale1_i*buf2[i*2];
               buf1[2*i]  +=t1r;
               buf1[2*i+1]+=t1i;
             }
           }
         }
         else if(0==strcmp(op,"scale")){
           if(e2==1){
             for(i=0; i < nblock; i++)buf1[i]=buf1[i]+scale1_r;
           }
           else{ 
             for(i=0; i < nblock/2; i++){
               t1r=scale1_r*buf1[i*2]  -scale1_i*buf1[i*2+1];
               t1i=scale1_r*buf1[i*2+1]+scale1_i*buf1[i*2];
               buf1[2*i]   =t1r;
               buf1[2*i+1] =t1i;
             }
           }
         }
         if(0!=strcmp(op,"dot")){
           if(0!=strcmp(op,"zero")) sseek(spread1.tag_sect[isect],-nblock*4,1);
           if(4*nblock!=srite(spread1.tag_sect[isect],buf1,nblock*4))
             seperr("trouble writing out file");
         }
       }
       ndone=ndone+(long long)nblock;
     }
   }
  fprintf(stderr,"Thread %d finished %d of %d \n",sep_thread_num(),isect,spread1.nsect);
 } 
 if(0==strcmp(op,"dot")){
   if(e2==1){
     if(impi!=nmpi-1){
       MPI_Recv(&temp1,1,MPI_DOUBLE,impi+1,234,MPI_COMM_WORLD,&status);
       dot1r+=temp1; 
     }
     if(impi!=0){
       MPI_Send(&dot1r,1,MPI_DOUBLE,impi-1,234,MPI_COMM_WORLD);
     }
     if(impi==0) fprintf(stderr,"DOT RESULT %g\n",dot1r);
   }
   else{
     if(impi!=nmpi-1){
       MPI_Recv(&temp1,1,MPI_DOUBLE,impi+1,234,MPI_COMM_WORLD,&status);
       dot1c_r+=temp1; 
       MPI_Recv(&temp1,1,MPI_DOUBLE,impi+1,235,MPI_COMM_WORLD,&status);
       dot1c_i+=temp1; 
     }
     if(impi!=0){
       MPI_Send(&dot1c_r,1,MPI_DOUBLE,impi-1,234,MPI_COMM_WORLD);
       MPI_Send(&dot1c_i,1,MPI_DOUBLE,impi-1,235,MPI_COMM_WORLD);
     }
     if(impi==0) fprintf(stderr,"DOT RESULT (%g,%g)\n",dot1c_r,dot1c_i);
   }
 }
 fprintf(stderr,"Thread %d finished  \n",sep_thread_num());
 MPI_Finalize();
 if(impi==0){
  if(1==getch("stat_good","s",temp_ch)) auxputch("junk","d",&impi,temp_ch);
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


