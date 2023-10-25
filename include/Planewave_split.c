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
int broadcast_data(int impi,int nmpi,float *data, int n1, int n2);
void init_mpi_seplib(int argc, char **argv);
void ***alloc3 (int n1, int n2, int n3, int size);
__complex__ float ***allocate_c3(int n3, int n2, int n1);
void return_planes(int npx, float opx, float dpx, int npy, float opy, float dpy,
 float min_tilt, float max_tilt, float min_azimuth, float max_azimuth,float *pxl,float *pyl);
int return_number_planes(int npx, float opx, float dpx, int npy, float opy, float dpy,
   float min_tilt, float max_tilt, float min_azimuth, float max_azimuth,int impi);
   
 
int main(int argc, char **argv){
  char *name;
  int nmpi, impi,esize,e2,nh,i,wh,wg,verb;
  int *nwind,*fwind,*jwind;
  float *buf;
  sep3d data;
  char my_tag[512],string[512];
  int max_size,nlen;
  MPI_Status status;
int   nry,nrx,nhx,nw;
float dry,drx,dhx,dw;
float ory,orx,ohx,ow;
float o,d;
float  ry, rx, hx, w;
__complex__ float ***plane,***receiver,phase,*plist;
  int plane_nsx,plane_nsy,np;
  float x,oy,dy,osx,esx,dsx,plane_osx,plane_dsx,plane_osy,plane_dsy;
  float tx0tyw,dtxw,costx0tyw,sintx0tyw,cosdtxw,sindtxw;
  int  iw,irx,iry,ip,ihx;
  float cosolda,sinolda;
  float txw,syw,sxw0,sy;
  int ipx,ipy;
  float opx,opy,dpx,dpy,px,py;
  float min_azimuth,max_azimuth,min_tilt,max_tilt;
  float *px_list,*py_list;
  char *hfile;
  char temp_ch[100000];
  int npx,npy,fw,nsx;
    

  /*initialize MPI*/
  init_mpi_seplib(argc,argv);

  init_3d();
  MY_SEP_DOC
  doc(SOURCE);
  sep3d_initialize(&data);
  impi=sep_thread_num();
  nmpi=sep_num_thread();


  hfile=(char*)malloc(sizeof(char)*10024);

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

    sprintf(temp_ch,"min_azimuth_sect%d",impi-1);
    if(0==getch(temp_ch,"f",&min_azimuth) )seperr("must specify %s",temp_ch);

    sprintf(temp_ch,"max_azimuth_sect%d",impi-1);
    if(0==getch(temp_ch,"f",&max_azimuth)) seperr("must specify %s",temp_ch);

    sprintf(temp_ch,"min_tilt_sect%d",impi-1);
    if(0==getch(temp_ch,"f",&min_tilt)) seperr("must specify %s",temp_ch);

    sprintf(temp_ch,"max_tilt_sect%d",impi-1);
    if(0==getch(temp_ch,"f",&max_tilt) )seperr("must specify %s",temp_ch);

     getch("npx","d",&npx);
     getch("npy","d",&npy);
     getch("opx","f",&opx);
     getch("opy","f",&opy);
     getch("dpx","f",&dpx);
     getch("dpy","f",&dpy);
     np=return_number_planes(npx,opy,dpx,npy,opy,dpy,min_tilt,max_tilt,min_azimuth,max_azimuth,impi);
     px_list=(float*)malloc(sizeof(float)*np);
     py_list=(float*)malloc(sizeof(float)*np);
     return_planes(npx,opy,dpx,npy,opy,dpy,min_tilt,max_tilt,min_azimuth,max_azimuth,px_list,py_list);
  }
  if(0!=sep3dc_broadcast_headers(&data,0))
    return(sepwarn(NOT_MET,"trouble broadcasting headers\n"));

  if(impi!=0){
     MPI_Recv(&nlen,1,MPI_INT,impi-1, 9999,MPI_COMM_WORLD,&status);
     MPI_Recv(hfile,nlen,MPI_CHAR,impi-1, 3999,MPI_COMM_WORLD,&status);
  }
  if(impi!=nmpi-1){
     MPI_Send(&nlen,1,MPI_INT,impi+1,9999, MPI_COMM_WORLD);
     MPI_Send(hfile,nlen,MPI_CHAR,impi+1,3999, MPI_COMM_WORLD);
  }

  if(data.file_format[0]!='R') 
    seperr("expecting a regular dataset");

  ohx=data.o[0];   dhx=data.d[0]; nhx=data.n[0];
  orx=data.o[1];   drx=data.d[1]; nrx=data.n[1];
  ory=data.o[2];   dry=data.d[2]; nry=data.n[2];

  osx=orx-(ohx+(nhx-1)*dhx);
  esx=orx+(nrx-1)*drx-ohx; 
  dsx=dhx;
  nsx=(esx-osx)/dsx+1;
  plane_osx=osx;  plane_dsx=dsx; plane_nsx=nsx;
  plane_osy=ory;  plane_dsy=dry; plane_nsy=nry;
 
  getch("fw","d",&fw);
  getch("nw","d",&nw);
  
  ow =data.o[3]+data.d[3]*fw;    dw=data.d[3]; 
  if(sep_thread_num()!=0){
    auxout(my_tag);
    auxputhead(my_tag,"-----------Copied from input file--------");
    auxputhead(my_tag,"%s\n",hfile);
    auxclose(my_tag);
    
     
     
    i=1;sep_put_data_axis_par(my_tag,&i,&data.n[1],&data.o[1],&data.d[1],data.label[1]);
    i=2;sep_put_data_axis_par(my_tag,&i,&data.n[2],&data.o[2],&data.d[2],data.label[2]);
    i=3;o=0;d=1;sep_put_data_axis_par(my_tag,&i,&np,&o,&d,"p");
    i=4;sep_put_data_axis_par(my_tag,&i,&nw,&ow,&dw,data.label[3]);
       
    /*    
    i=1;sep_put_data_axis_par(my_tag,&i,&data.n[0],&data.o[0],&data.d[0],data.label[0]);
    i=2;sep_put_data_axis_par(my_tag,&i,&data.n[1],&data.o[1],&data.d[1],data.label[1]);
    i=3;sep_put_data_axis_par(my_tag,&i,&data.n[2],&data.o[2],&data.d[2],data.label[2]);
    i=4;sep_put_data_axis_par(my_tag,&i,&nw,&ow,&data.d[3],data.label[3]);
    */

    auxputch("npx","d",&npx,my_tag);
    auxputch("npy","d",&npy,my_tag);
    auxputch("dpx","f",&dpx,my_tag);
    auxputch("dpy","f",&dpy,my_tag);
    auxputch("opx","f",&opx,my_tag);
    auxputch("opy","f",&opy,my_tag);
    auxputch("fw","d",&fw,my_tag);
    auxputch("min_azimuth","f",&min_azimuth,my_tag);
    auxputch("min_tilt","f",&min_tilt,my_tag);
    auxputch("max_azimuth","f",&max_azimuth,my_tag);
    auxputch("max_tilt","f",&max_tilt,my_tag);
    auxputch ("plane_os_x","f",&plane_osx,my_tag);
    auxputch ("plane_ds_x","f",&plane_dsx,my_tag);
    auxputch ("plane_ns_x","d",&plane_nsx,my_tag);
    auxputch ("plane_os_y","f",&plane_osy,my_tag);
    auxputch ("plane_ds_y","f",&plane_dsy,my_tag);
    auxputch ("plane_ns_y","d",&plane_nsy,my_tag);
  }
  

  plane=allocate_c3(np,nry,nrx); receiver=allocate_c3(nry,nrx,nhx);
  if(0==getch("verb","d",&verb)) verb=0;
  if(0==getch("max_size","d",&max_size)) max_size=500;
  esize=sep3dc_get_esize(&data);
  e2=esize/4;
  max_size=max_size*1000000/esize;

  /*if(data.ndims!=4)
     seperr("expecting 4-D datasets");
  */

 /*    wait ?? */ 
  
  if(impi==0) {
    sseek_block("combo",fw,8*nhx*nry*nrx,0);
  }
  
  
  for(iw=0;iw<nw;iw++){
    w=ow+iw*dw; 
    if(impi==0) {
      if(8*nrx*nry*nhx!=sreed("combo",**receiver,8*nhx*nry*nrx))
        seperr("trouble reading from combo file");
    }
    if(0!=broadcast_data(impi,nmpi,(float*)(**receiver),2*data.n[0],data.n[1]*data.n[2]))
      return(sepwarn(NOT_MET,"trouble broadcasting data"));
    
    if(impi!=0){
      
      for(ip=0;ip<np;ip++){
        for(iry=0;iry<nry;iry++){
          for(irx=0;irx<nrx;irx++){
            plane[ip][iry][irx]=0.0;
          }
        }
      }
      for(iry=0;iry<nry;iry++){
        ry=iry*dry+ory;
        sy=ry;
        syw=(sy-plane_osy)*w;
        for(irx=0;irx<nrx;irx++){
          rx=irx*drx+orx;
          sxw0=(rx-ohx-plane_osx)*w;
          for(ip=0; ip < np; ip++){
            px=px_list[ip]; py=py_list[ip];
            tx0tyw=sxw0*px+syw*py;
            dtxw=dhx*px*w;
            costx0tyw=cos(tx0tyw); sintx0tyw=sin(tx0tyw);
            cosdtxw=cos(dtxw);     sindtxw=sin(dtxw);
            __real__ phase=costx0tyw; __imag__ phase=sintx0tyw;
            for(ihx=0;ihx<nhx;ihx++){
              plane[ip][iry][irx]+=receiver[iry][irx][ihx]*phase;
              cosolda=__real__ phase; sinolda=__imag__ phase;
              __real__ phase=cosolda*cosdtxw+sinolda*sindtxw;
              __imag__ phase=sinolda*cosdtxw-cosolda*sindtxw;
            } //ihx
          }//ip
        }//irx
      }//iry
      
     //debug for(iry=0;iry<nry;iry++) for(irx=0;irx<nrx;irx++) for(ihx=0;ihx<nhx;ihx++) receiver[iry][irx][ihx]=0.0;
      if (srite(my_tag,**plane,np*nry*nrx*8)!=np*nry*nrx*8) seperr("err write plane\n");
      //if (srite(my_tag,**receiver,nry*nrx*nhx*8)!=nry*nrx*nhx*8) seperr("err write pp\n");
    }
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
  if(impi!=nmpi-1)
    MPI_Send((data+done),block, MPI_FLOAT, impi+1,i, MPI_COMM_WORLD);
  done+=block;
}
return(0);
}
__complex__ float ***allocate_c3(int n3, int n2, int n1)
{
        return (__complex__ float***)alloc3(n1,n2,n3,sizeof(__complex__ float));
}
/* allocate a 3-d array */
void ***alloc3 (int n1, int n2, int n3, int size)
{
        int i3,i2;
        void ***p;

        if ((p=(void***)malloc(n3*sizeof(void**)))==NULL)
                return NULL;
        if ((p[0]=(void**)malloc(n3*n2*sizeof(void*)))==NULL) {
                free(p);
                return NULL;
        }
        if ((p[0][0]=(void*)malloc(n3*n2*n1*size))==NULL) {
                free(p[0]);
                free(p);
                return NULL;
        }

        for (i3=0; i3<n3; i3++) {
                p[i3] = p[0]+n2*i3;
                for (i2=0; i2<n2; i2++)
                        p[i3][i2] = (char*)p[0][0]+size*n1*(i2+n2*i3);
        }
        return p;
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

 int return_number_planes(int npx, float opx, float dpx, int npy, float opy, float dpy,
   float min_tilt, float max_tilt, float min_azimuth, float max_azimuth, int impi)
 {
  int iplist,ipx,ipy;
  float py,py2,px,px2,sintilt,tilt,azimuth,sinazimuth;
     
     
    iplist=0;
    for (ipy=0;ipy<npy;ipy++){
      py=ipy*dpy+opy;  py2=py*py;
      for(ipx=0;ipx<npx;ipx++){
        px=ipx*dpx+opx; px2=px*px;
        sintilt=sqrt(py2+px2)*1500.0;
        tilt=asin(sintilt)/3.1415926*180;
        if (tilt>=min_tilt && tilt <max_tilt){
          if (px==0 && py==0){
         
          }
          else{
            sinazimuth=py/sqrt(px2+py2);
            azimuth=asin(sinazimuth)/3.1415926*180;
            if (px < 0 )    azimuth=180-azimuth;
            if (azimuth <0) azimuth=360+azimuth; 
          }
          if ( azimuth >= min_azimuth &&  azimuth <max_azimuth){
            iplist++;
          }
        } 
      }
    }
    return(iplist);
}

void return_planes(int npx, float opx, float dpx, int npy, float opy, float dpy,
   float min_tilt, float max_tilt, float min_azimuth, float max_azimuth, float *pxl, float *pyl)
   {
     int iplist,ipx,ipy;
  float py,py2,px,px2,sintilt,tilt,azimuth,sinazimuth;
     
    iplist=0;
    for (ipy=0;ipy<npy;ipy++){
      py=ipy*dpy+opy;  py2=py*py;
      for(ipx=0;ipx<npx;ipx++){
        px=ipx*dpx+opx; px2=px*px;
        sintilt=sqrt(py2+px2)*1500.0;
        tilt=asin(sintilt)/3.1415926*180;
        if (tilt>=min_tilt && tilt <max_tilt){
          if (px==0 && py==0){
         
          }
          else{
            sinazimuth=py/sqrt(px2+py2);
            azimuth=asin(sinazimuth)/3.1415926*180;
            if (px < 0 )    azimuth=180-azimuth;
            if (azimuth <0) azimuth=360+azimuth; 
          }
          if ( azimuth >= min_azimuth &&  azimuth <max_azimuth){
           pxl[iplist]=px;  pyl[iplist]=py;
            iplist++;
          }
        } 
      }
    }   
   }
