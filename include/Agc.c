/*$
=head1 NAME

Agc - Automatic gain control with first arrival detection

=head1 SYNOPSIS 

Agc window=200 detect=0 dwind=10 thresh=25. < in.H > out.H
 
=head1 INPUT  PARAMETERS

=over 4

=item window   - integer 

      [200]: length of the window in number of samples

=item dwind    - integer 

       [10]: length of the detection window in number of samples

=item detect   - integer  

        [0]:  detection off (Default value)    1:  detection on

=item thresh   - real     

      [25.]: threshold (see comments below)

=item n1,n2,n3,n4,n5,n6,n7 - integer  

      standard seplib parameters 

=back

=head1 DESCRIPTION 

Gain program with first arrival detection
 
=head1 COMMENTS

        Agc gains seismic traces by normalizing each sample by the
        power of a window of samples surrounding that point.  
      
        When the detect= is set to 1, agc is not applied on a trace
        until a first arrival is detected. The first arrival is  
        detected when a moving dwind exceeds an initial dwind by a 
        threshold factor.

=head1 EXAMPLE

	Agc < in.H window=200 detect=1 dwind=10 thresh=25. > out.H

	Applies agc on a trace based on a trace window of 200 samples. 
	Agc is only applied after the first arrival has been detected 
	using a detection window of 10 samples. The detection of the 
	first arrival is based on an amplitude change between consecutive 
	detection windows of 25%.

=head1 CATEGORY

B<seis/filter>

=cut

>*/

/*
 
  KEYWORDS agc gain scale : Tpow

  SEE ALSO

  WHERE
        ./cube/seis/Agc.c

*/ 
/*
Martin Karrenbach 11-3-92  changed alloc(sizeof()) and sreed/srite sizeof(xdr)
Stewart A. Levin 5/6/95 Mobil/SEP corrected erroneous "==" instead of "="
*/
static int window=200, dwind=10, detect=0;
float thresh=25.;

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Agc - Automatic gain control with first arrival detection");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Agc window=200 detect=0 dwind=10 thresh=25. < in.H > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT  PARAMETERS");\
 sep_add_doc_line("    window - integer");\
 sep_add_doc_line("              [200]: length of the window in number of samples");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dwind - integer");\
 sep_add_doc_line("               [10]: length of the detection window in number of samples");\
 sep_add_doc_line("");\
 sep_add_doc_line("    detect - integer");\
 sep_add_doc_line("                [0]:  detection off (Default value)    1:  detection on");\
 sep_add_doc_line("");\
 sep_add_doc_line("    thresh - real");\
 sep_add_doc_line("              [25.]: threshold (see comments below)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n1,n2,n3,n4,n5,n6,n7 - integer");\
 sep_add_doc_line("              standard seplib parameters ");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Gain program with first arrival detection");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("            Agc gains seismic traces by normalizing each sample by the");\
 sep_add_doc_line("            power of a window of samples surrounding that point.  ");\
 sep_add_doc_line("");\
 sep_add_doc_line("            When the detect= is set to 1, agc is not applied on a trace");\
 sep_add_doc_line("            until a first arrival is detected. The first arrival is  ");\
 sep_add_doc_line("            detected when a moving dwind exceeds an initial dwind by a ");\
 sep_add_doc_line("            threshold factor.");\
 sep_add_doc_line("");\
 sep_add_doc_line("EXAMPLE");\
 sep_add_doc_line("            Agc < in.H window=200 detect=1 dwind=10 thresh=25. > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("            Applies agc on a trace based on a trace window of 200 samples. ");\
 sep_add_doc_line("            Agc is only applied after the first arrival has been detected ");\
 sep_add_doc_line("            using a detection window of 10 samples. The detection of the ");\
 sep_add_doc_line("            first arrival is based on an amplitude change between consecutive ");\
 sep_add_doc_line("            detection windows of 25%.");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/filter");\
 sep_add_doc_line("");
#include <sep.main>
MAIN()
{
	int ix, nt, nx, n3, esize;
	float initaver, probe, l1norm, oldnorm;
	float *x, *y, *xp, *yp, *tail, *head, *start, *end;

        sep_begin_prog();
	/* fetch parameters */
	if (!fetch("n1","d",&nt))
		if (fetch("nt","d",&nt)==0) seperr ("n1= missing\n");
	if (!fetch("n2","d",&nx)) if (!fetch("nx","d",&nx)) nx = 0;
	if (!fetch("n3","d",&n3)) n3 = 1;
	if (nx == 0)	nx = ssize ("in") / (nt * 4);
	else nx *= n3;
	if (1==hetch("n4","d",&n3)) nx = nx*n3;
	if (1==hetch("n5","d",&n3)) nx = nx*n3;
	if (1==hetch("n6","d",&n3)) nx = nx*n3;
	if (1==hetch("n7","d",&n3)) nx = nx*n3;
      

	if (!fetch("esize","d",&esize)) esize = sizeof(float);
	getch("window","d",&window);
	getch("detect","d",&detect);
	getch("dwind","d",&dwind);
	getch("thresh","f",&thresh);
	putch ("#window","d",&window);
	if (detect)
		puthead ("dwind=%d thresh=%g\n",dwind,thresh);
	hclose();

	/* allocate storage */
	x = (float *) alloc (nt*sizeof(float));
	y = (float *) alloc (nt*sizeof(float));

	/* gain each trace by l1 norm of moving window */
	for (ix = 0; ix<nx; ix++) {

		if((nt*esize) != sreed ("in",x,nt*esize)) 
			seperr("unexpected EOF on input\n");
		xp = x;
		yp = y;

		/* detect first significant energy by comparing 
		   short moving window with an initial average */
		if (detect) {
		detects: for (end=y+nt; yp<end;) *yp++ = 0.;
			for (initaver = 0., end = x + dwind; xp<end;) {
				if (*xp>0.) initaver += *xp++;
				else initaver -= *xp++;
				}

			for (xp=x+dwind/2,yp=y+dwind/2,tail=x,head=x+dwind,end=x+nt-dwind/2, 
				probe=initaver; ((probe<=(initaver*thresh))&&(xp<end)); xp++,yp++)

				if (*head>0.) probe += *head++;
				else probe -= *head++;
				if (*tail>0.) probe -= *tail++;
				else probe += *tail++;

			if ((xp == end)||(probe == 0.)) {
				/*fprintf (stderr,"no energy detected in trace %d\n",ix);*/
				goto right;
			}
		}

		/* untaper beginning of trace */
		for (start=xp, l1norm=0., end=xp+window; xp<end;) {
			if (*xp>0.) l1norm += *xp++;
			else l1norm -= *xp++;
		}
		if (l1norm == 0) {
			xp = x;
			goto detects;
		}
		for (xp=start, end=start+window/2; xp<end;) *yp++ = *xp++/l1norm;

		/* core routine */
		for(tail=start,head=start+window,end=x+nt-window/2,oldnorm=l1norm;xp<end;)
		{
			if (*head>0.) l1norm += *head++;
			else l1norm -= *head++;
			if (*tail>0.) l1norm -= *tail++;
			else l1norm += *tail++;
			if (oldnorm == 0.) l1norm = oldnorm;
			oldnorm = l1norm;
			*yp++ = *xp++ / l1norm;
		}

		/* untaper end of trace */
		for (end=x+nt; xp<end;) *yp++ = *xp++/l1norm;
		right: srite ("out",y,nt*esize);
	}
  sep_end_prog();
  return EXIT_SUCCESS;
}
