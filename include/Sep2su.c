/*$

=head1 NAME

Sep2su - convert from sep3d to su


=head1 SYNOPSIS

Sep3d2su < in.H >out.su pars

=head1 INPUT PARAMETERS

=over 4

=item verb - int  [1000000] 

      How often to print number of traces converted


=back

=head1 DESCRIPTION

Converts a SEP3d dataset to SU

=head1 SEE ALSO

L<Su2sep>

=head1 CATEGORY

B<converters>

=cut
*/
#include<superset.h>
#ifdef SU_SUPPORT
#include "su.h"
#include "segy.h"

/*********************** self documentation ******************************/
char *sdoc[] = {
"									     ",
" SEP2SU - Convert sepfile to a sufile ",
"  sep2su < in.H >out.su pars",
" ",
" verb  -  integer  [0] How often to print number of traces converted",
NULL};

segy tr;
int main(int argc, char **argv)
{
	int i;
	int verb;

	/* hook up getpar */
	initpar(argc,argv);
	getch_add_string("suoutput=1");
	initargs(argc, argv);

	verb=0;
	getch("verb","d",&verb);

sep_add_doc_line("NAME");
sep_add_doc_line("    Sep2su - convert from sep3d to su");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("    Sep3d2su < in.H >out.su pars");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    verb - int [1000000]");
sep_add_doc_line("              How often to print number of traces converted");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("    Converts a SEP3d dataset to SU");
sep_add_doc_line("");
sep_add_doc_line("SEE ALSO");
sep_add_doc_line("    Su2sep");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    converters");
sep_add_doc_line("");

   doc(SOURCE);
	requestdoc(0);
	i=0;
	if (!gettr(&tr)) err("can't get first trace");
	do {
		i++;
		fputtr(stdout,(&tr));
		if(verb!=0){
			if(i%verb==0) fprintf(stderr,"converted %d  traces \n",i);
		}
	} while (gettr(&tr));
			if(verb>0) fprintf(stderr,"converted %d  traces \n",i);

  finish_susep();
	return EXIT_SUCCESS;
}
#else
#include <sep3d.h>
main(int argc, char **argv)
{
fprintf(stderr,"You did not compile with su support \n");
}
#endif
