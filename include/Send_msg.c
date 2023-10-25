/*
=head1 NAME
 
 Send_msg
   
=head1 SYNOPSIS
 
  Send_msg ip=  port= msg=
 
=head1 PARAMTERS
 
=over 4
 
=item ip - char*
 
  IP address to send
 
=item port - int*
 
  Port number to send to
 
=item msg_type - char*
 
 Message to send 
 
=item msg_extra - char*
 
 Message to extra 
 
=cut
*/

#include<seplib.h>

#define HEAD "/dev/null"
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("     Send_msg");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("      Send_msg ip=  port= msg=");\
 sep_add_doc_line("");\
 sep_add_doc_line("PARAMTERS");\
 sep_add_doc_line("    ip - char*");\
 sep_add_doc_line("          IP address to send");\
 sep_add_doc_line("");\
 sep_add_doc_line("    port - int*");\
 sep_add_doc_line("          Port number to send to");\
 sep_add_doc_line("");\
 sep_add_doc_line("    msg_type - char*");\
 sep_add_doc_line("         Message to send ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    msg_extra - char*");\
 sep_add_doc_line("         Message to extra ");\
 sep_add_doc_line("");
int main(int argc, char **argv){
 char ip[1024],msg_type[1024],msg_extra[1024];
 char temp_ch[1024];
 int port[500],ierr,i;

   initpar(argc,argv);
   

 if(0==getch("ip","s",ip))
   seperr("ip2 required parameter");

 if(0==getch("msg_type","s",msg_type))
   seperr("msg_type required parameter");

 if(0==getch("msg_extra","s",msg_extra))
   strcpy(msg_extra,"none");;

 ierr=getch("port","d",port);
  if(ierr==0) seperr("port required parameter");

  
  strcpy(temp_ch,"sep.mach_label=none");
  getch_add_string(temp_ch);
  strcpy(temp_ch,"sep.jobid=none");
  getch_add_string(temp_ch);
  sprintf(temp_ch,"sep.master_ip=%s",ip);
  getch_add_string(temp_ch);
  getch_add_string("sep.jobid=none");
  for (i=0; i < ierr; i++){
    sprintf(temp_ch,"sep.master_port=%d",port[i]);
    getch_add_string(temp_ch);
    if(0!=sep_send_msg(msg_type,msg_extra))
      seperr("trouble sending message");
  }
  return(0);
}
