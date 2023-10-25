/*$

=head1 NAME

Su2sep - convert from su to sep3d


=head1 SYNOPSIS

Su2sep < in.su >out.H pars

=head1 INPUT PARAMETERS

=over 4

=item verb - int  

      [1000000] How often to print number of traces converted 


=back

=head1 DESCRIPTION

Converts a SU dataset to SEP3d

=head1 SEE ALSO

L<Sep2su>

=head1 CATEGORY

B<converters>

=cut
*/
#include<superset.h>
#ifdef SU_SUPPORT
#include "su.h"
#include "segy.h"

char *sdoc[] = {
"									     ",
" SU2SEP - Convert sufile to sepfile ",
"  su2sep < in.su >out.H pars",
" ",
" verb  -  integer  [1000000] How often to print number of traces converted",
NULL};

#include<superset.h>
segy tr;
int main(int argc, char **argv)
{
	int i,verb;
	/* hook up getpar */
	initpar(argc,argv);
	getch_add_string("suinput=1");
	initargs(argc, argv);
	verb=1000000; getch("verb","d",&verb);


#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Su2sep - convert from su to sep3d");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Su2sep < in.su >out.H pars");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    verb - int");\
 sep_add_doc_line("              [1000000] How often to print number of traces converted");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Converts a SU dataset to SEP3d");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Sep2su");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    converters");\
 sep_add_doc_line("");

   doc(SOURCE);

	requestdoc(0); i=0;
	if (!fgettr(stdin,(&tr))) err("can't get first trace");
	do { i++; 
puttr(&tr);
	if(i%verb==0) fprintf(stderr,"converted %d  traces \n",i);
	} while (fgettr(stdin,(&tr)));
	if(verb>0) fprintf(stderr,"converted %d  traces \n",i);
  finish_susep();
	return EXIT_SUCCESS;
}
#else
#include<sep3d.h>
int main( int argc, char **argv)
{
fprintf(stderr,"You did not configure with SU support \n");
}
#endif
