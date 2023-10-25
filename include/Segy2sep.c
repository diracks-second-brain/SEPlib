/*$

=head1 NAME

Segy2sep - Convert from Segy to  SEPlib3d


=head1 SYNOPSIS

Segy2sep > stdout tape=   [pars]


=head1 INPUT PARAMETERS

=over 4


=item  tape    -file

       input tape device or seg-y filename (see notes)

=item  buff    -int

       [1]  for buffered device (9-track reel tape drive)
       =0 possibly useful for 8mm EXABYTE drives

=item  verb -int

       [0]  silent operation

=item vblock - int

       [50] ; echo every '50' traces

=item  hfile   -file

       [tape.header]  file to store ebcdic block (as ascii)

=item  bfile   -file

       [tape.binary]  file to store binary block
       specify full paths to ease conversion back to segy

=item  ns      -int

       [bh.hns]  number of samples (use if bhed ns wrong)

=item  trmin   -int

       [1]    first trace to read

=item  trmax   -int

       [INT_MAX]  last trace to read

=item  errmax  -int

       [0]  allowable number of consecutive tape IO errors

=item  nmem-int

       [100] how many traces to look for non zero header keys

=item  nkeep-  -int

       number of keys we will force to be transfered

=item  keep_   -char*

       name of the key to keep (size of nkeep)

=item  keeplist-char*

       name of keys to keep (separated by :)

=item  nextra_keys  -int

       number of extra keys not in SEG-Y standard

=item  extra_name_  -char*

       name of extra key

=item  extra_type_  -char*

       type of extra key (int, unsigned int, short, unsigned
       short, long, unsigned long, float, double)

=item  extra_offset -int*

       index of extra key (0-238)

=item  conv -int *

   [1] convert data to native format,0  assume data is in native format

=item  endian -int *

   [1] set =0 for little-endian machines(PC's,DEC,etc.)

=item  ignorelist-char*

       list, separated by :, of keys to ignore

=item  nignore -int

       number of keys to ignore when converting

=item  ignore_ -char*

       name of the key to ignore (size of nignore)

=item  ignorelist-char*

       list, separated by :, of keys to ignore

=item  dump_all     -int

       [0], 1-dump all keys

=item  only_list    -int

       [0], dump only keys listed by keep and extra

=item ns           -int

      number of samples in the dataset

=item format       -int

      [1]  Float data, 2 

     [ 5] IEEE floating point, 4 byte (32 bits)                            

=item ntraces     - int

       If you want to pipe out  provide the number
       of traces in the dataset, otherwise unpipable.

=item nsamp_max   - int
       
       By default each trace is assumed to be SU_NFLTS (1,000,000+)
       in size and a copy of that length is done for each trace.
       Set this to something more reasonalbe (2000) for a massive speed up.

=back

=head1 DESCRIPTION

Read in a segy tape

=head1 COMMENTS

   Notes: Traditionally tape=/dev/rmt0.   However, in the modern world
   tape device names are much less uniform.  The magic name can
   often be deduced by 'ls /dev'.  Likely man pages with the
   names of the tape devices are: 'mt', 'sd' 'st'.  Also
   try 'man -k scsi', ' man mt', etc.  Sometimes 'mt status'
   will tell the device name.

   For a seg-y diskfile use tape=filename.
   Remark: a seg-y file is not the same as an su file.
   A seg-y file consists of three parts: an ebcdic header,
   a binary reel header, and the traces.  The traces are (usually)
   in 32 bit IBM floating point format.  An SU file consists only
   of the trace portion written in the native binary floats.

     type:    sudoc segyread   for further information

  ARRAY(INVALID)=(/ns,delrt,d1,f1,f2,d1,d2/)



=head1 CATEGORY

B<converters>

=cut
>*/

/* Copyright (c) Colorado School of Mines, 1999.*/
/* All rights reserved.                       */

/* SEGYREAD: $Revision: 1.2 $ ; $Date: 2004/07/08 18:15:32 $     */

#include<superset.h>
#ifdef SU_SUPPORT
#include "su.h"
#include "segy.h"
#include "tapesegy.h"
#include "tapebhdr.h"
#include "bheader.h"
#include "header.h"
#include "hdr.h"
#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int array_decode(char list[4096],char array[100][128]);
void handle_additional();
_XFUNCPROTOEND
#else
int array_decode()
#endif

char  list[4096], array[100][128];
char *sdoc[] = {
"                      ",
" Segy2sep - Convert sufile to sepfile ",
"  Segy2sep tape=in.su >out.H pars",
" ",
" verb  -  integer  [1000000] How often to print number of traces converted",
NULL};



/*
 * Note:
 *      If you have a tape with multiple sequences of ebcdic header,
 *	binary header,traces, use the device that
 *	invokes the no-rewind option and issue multiple segyread
 *	commands (making an appropriate shell script if you
 *	want to save all the headers).	Consider using >> if
 *	you want a single trace file in the end.  Similar
 *	considerations apply for multiple reels of tapes,
 *	but use the standard rewind on end of file.
 *						
 * Note: For buff=1 (default) tape is accessed with 'read', for buff=0
 *	tape is accessed with fread. We suggest that you try buff=1
 *	even with EXABYTE tapes.				 
 * Caveat: may be slow on an 8mm streaming (EXABYTE) tapedrive
 * Warning: segyread or segywrite to 8mm tape is fragile. Allow sufficient
 *	time between successive reads and writes.
 * Warning: may return the error message "efclose: fclose failed"
 *	intermittently when segyreading/segywriting to 8mm (EXABYTE) tape
 *	even if actual segyread/segywrite is successful. However, this
 *	error message may be returned if your tape drive has a fixed 
 *	block size set.
 * Caution: When reading or writing SEG-Y tapes, the tape
 *	drive should be set to be able to read variable block length
 *	tape files.
 */

/* Credits:
 *	SEP: Einar Kjartansson
 *	CWP: Jack K. Cohen, Brian Sumner, Chris Liner
 *	   : John Stockwell (added 8mm tape stuff)
 *  SEP: RGC Converted back to SEPlib
 * conv parameter added by:
 *	Tony Kocurko
 *	Department of Earth Sciences
 *	Memorial University of Newfoundland
 *	St. John's, Newfoundland
 * read from stdin via tape=-  added by	Tony Kocurko
 * bhed format = 2,3 conversion by:
 *	Remco Romijn (Applied Geophysics, TU Delft)
 *	J.W. de Bruijn (Applied Geophysics, TU Delft)
 *--------------------------
 * Additional Notes:
 *	Brian's subroutine, ibm_to_float, which converts IBM floating
 *	point to IEEE floating point is NOT portable and must be
 *	altered for non-IEEE machines.	See the subroutine notes below.
 *
 *	A direct read by dd would suck up the entire tape; hence the
 *	dancing around with buffers and files.
 * 
 */
/**************** end self doc ***********************************/

/* Subroutine prototypes */
static void ibm_to_float(int from[], int to[], int n, int endian);
static void int_to_float(int *from, float *to, int n, int endian);
static void short_to_float(short from[], float to[], int n, int endian);
static void tapebhed_to_bhed(const tapebhed *tapebhptr, bhed *bhptr);
static void tapesegy_to_segy(const tapesegy *tapetrptr, segy *trptr);
static void swap_int_4_new(int *tni4);

/* Globals */
tapesegy tapetr;
tapebhed tapebh;
segy tr;
bhed bh;
int nsamp_max;

int
main(int argc, char **argv)
{
	char *tape;		/* name of raw tape device	*/
	char *bfile;		/* name of binary header file	*/
	char *hfile;		/* name of ascii header file	*/

	int tapefd=0;		/* file descriptor for tape	*/

	FILE *tapefp=NULL;	/* file pointer for tape	*/
	FILE *binaryfp;		/* file pointer for bfile	*/
	FILE *headerfp;		/* file pointer for hfile	*/
	FILE *pipefp;		/* file pointer for popen write */

	size_t nsegy;		/* size of whole trace in bytes		*/
	int i;			/* counter				*/
	int itr;		/* current trace number			*/
	int trmin;		/* first trace to read			*/
	int trmax;		/* last trace to read			*/
	int ns;			/* number of data samples		*/

	int over;		/* flag for bhed.float override		*/
	int format;		/* flag for to specify override format	*/
	cwp_Bool format_set = cwp_false;
				/* flag to see if new format is set	*/
	int conv;		/* flag for data conversion		*/
	int verbose;		/* echo every ...			*/
	int vblock;		/* ... vblock traces with verbose=1	*/
	int buff;		/* flag for buffered/unbuffered device	*/
	int endian;		/* flag for big=1 or little=0 endian	*/
	int errmax;		/* max consecutive tape io errors	*/
	int errcount = 0;	/* counter for tape io errors		*/
	cwp_Bool nsflag;	/* flag for error in tr.ns		*/
 
	char cmdbuf[BUFSIZ];	/* dd command buffer			*/
	char ebcbuf[EBCBYTES];	/* ebcdic data buffer			*/



	/* Initialize */
	initargs(argc, argv);
sep_add_doc_line("NAME");
sep_add_doc_line("    Segy2sep - Convert from Segy to SEPlib3d");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("    Segy2sep > stdout tape= [pars]");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    tape -file");
sep_add_doc_line("               input tape device or seg-y filename (see notes)");
sep_add_doc_line("");
sep_add_doc_line("    buff -int");
sep_add_doc_line("               [1]  for buffered device (9-track reel tape drive)");
sep_add_doc_line("               =0 possibly useful for 8mm EXABYTE drives");
sep_add_doc_line("");
sep_add_doc_line("    verb -int");
sep_add_doc_line("               [0]  silent operation");
sep_add_doc_line("");
sep_add_doc_line("    vblock - int");
sep_add_doc_line("               [50] ; echo every '50' traces");
sep_add_doc_line("");
sep_add_doc_line("    hfile -file");
sep_add_doc_line("               [tape.header]  file to store ebcdic block (as ascii)");
sep_add_doc_line("");
sep_add_doc_line("    bfile -file");
sep_add_doc_line("               [tape.binary]  file to store binary block");
sep_add_doc_line("               specify full paths to ease conversion back to segy");
sep_add_doc_line("");
sep_add_doc_line("    ns -int");
sep_add_doc_line("               [bh.hns]  number of samples (use if bhed ns wrong)");
sep_add_doc_line("");
sep_add_doc_line("    trmin -int");
sep_add_doc_line("               [1]    first trace to read");
sep_add_doc_line("");
sep_add_doc_line("    trmax -int");
sep_add_doc_line("               [INT_MAX]  last trace to read");
sep_add_doc_line("");
sep_add_doc_line("    errmax -int");
sep_add_doc_line("               [0]  allowable number of consecutive tape IO errors");
sep_add_doc_line("");
sep_add_doc_line("    nmem-int");
sep_add_doc_line("               [100] how many traces to look for non zero header keys");
sep_add_doc_line("");
sep_add_doc_line("    nkeep- -int");
sep_add_doc_line("               number of keys we will force to be transfered");
sep_add_doc_line("");
sep_add_doc_line("    keep_ -char*");
sep_add_doc_line("               name of the key to keep (size of nkeep)");
sep_add_doc_line("");
sep_add_doc_line("    keeplist-char*");
sep_add_doc_line("               name of keys to keep (separated by :)");
sep_add_doc_line("");
sep_add_doc_line("    nextra_keys -int");
sep_add_doc_line("               number of extra keys not in SEG-Y standard");
sep_add_doc_line("");
sep_add_doc_line("    extra_name_ -char*");
sep_add_doc_line("               name of extra key");
sep_add_doc_line("");
sep_add_doc_line("    extra_type_ -char*");
sep_add_doc_line("               type of extra key (int, unsigned int, short, unsigned");
sep_add_doc_line("               short, long, unsigned long, float, double)");
sep_add_doc_line("");
sep_add_doc_line("    extra_offset -int*");
sep_add_doc_line("               index of extra key (0-238)");
sep_add_doc_line("");
sep_add_doc_line("    conv -int *");
sep_add_doc_line("           [1] convert data to native format,0  assume data is in native format");
sep_add_doc_line("");
sep_add_doc_line("    endian -int *");
sep_add_doc_line("           [1] set =0 for little-endian machines(PC's,DEC,etc.)");
sep_add_doc_line("");
sep_add_doc_line("    ignorelist-char*");
sep_add_doc_line("               list, separated by :, of keys to ignore");
sep_add_doc_line("");
sep_add_doc_line("    nignore -int");
sep_add_doc_line("               number of keys to ignore when converting");
sep_add_doc_line("");
sep_add_doc_line("    ignore_ -char*");
sep_add_doc_line("               name of the key to ignore (size of nignore)");
sep_add_doc_line("");
sep_add_doc_line("    ignorelist-char*");
sep_add_doc_line("               list, separated by :, of keys to ignore");
sep_add_doc_line("");
sep_add_doc_line("    dump_all -int");
sep_add_doc_line("               [0], 1-dump all keys");
sep_add_doc_line("");
sep_add_doc_line("    only_list -int");
sep_add_doc_line("               [0], dump only keys listed by keep and extra");
sep_add_doc_line("");
sep_add_doc_line("    ns -int");
sep_add_doc_line("              number of samples in the dataset");
sep_add_doc_line("");
sep_add_doc_line("    format -int");
sep_add_doc_line("              [1]  Float data, 2 ");
sep_add_doc_line("");
sep_add_doc_line("             [ 5] IEEE floating point, 4 byte (32 bits)");
sep_add_doc_line("");
sep_add_doc_line("    ntraces - int");
sep_add_doc_line("               If you want to pipe out  provide the number");
sep_add_doc_line("               of traces in the dataset, otherwise unpipable.");
sep_add_doc_line("");
sep_add_doc_line("    nsamp_max - int");
sep_add_doc_line("               By default each trace is assumed to be SU_NFLTS (1,000,000+)");
sep_add_doc_line("               in size and a copy of that length is done for each trace.");
sep_add_doc_line("               Set this to something more reasonalbe (2000) for a massive speed up.");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("    Read in a segy tape");
sep_add_doc_line("");
sep_add_doc_line("COMMENTS");
sep_add_doc_line("       Notes: Traditionally tape=/dev/rmt0.   However, in the modern world");
sep_add_doc_line("       tape device names are much less uniform.  The magic name can");
sep_add_doc_line("       often be deduced by 'ls /dev'.  Likely man pages with the");
sep_add_doc_line("       names of the tape devices are: 'mt', 'sd' 'st'.  Also");
sep_add_doc_line("       try 'man -k scsi', ' man mt', etc.  Sometimes 'mt status'");
sep_add_doc_line("       will tell the device name.");
sep_add_doc_line("");
sep_add_doc_line("       For a seg-y diskfile use tape=filename.");
sep_add_doc_line("       Remark: a seg-y file is not the same as an su file.");
sep_add_doc_line("       A seg-y file consists of three parts: an ebcdic header,");
sep_add_doc_line("       a binary reel header, and the traces.  The traces are (usually)");
sep_add_doc_line("       in 32 bit IBM floating point format.  An SU file consists only");
sep_add_doc_line("       of the trace portion written in the native binary floats.");
sep_add_doc_line("");
sep_add_doc_line("         type:    sudoc segyread   for further information");
sep_add_doc_line("");
sep_add_doc_line("      ARRAY(INVALID)=(/ns,delrt,d1,f1,f2,d1,d2/)");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    converters");
sep_add_doc_line("");

   doc(SOURCE);
	requestdoc(1); /* stdin not used */
  

  handle_additional();
  
  fprintf(stderr,"CHECK SIZES %d %d %d\n",sizeof(long),sizeof(int),sizeof(long long));

	/* Make sure stdout is a file or pipe */
	switch(filestat(STDOUT)) {
	case TTY:
		seperr("stdout can't be tty");
	break;
	case DIRECTORY:
		seperr("stdout must be a file, not a directory");
	break;
	case BADFILETYPE:
		seperr("stdout is illegal filetype");
	break;
	default: /* Others OK */
	break;
	}

	/* Set filenames */
	MUSTGETPARSTRING("tape",  &tape);
	if (!getparstring("hfile", &hfile))	hfile = "header";
	if (!getparstring("bfile", &bfile))	bfile = "binary";

	
	/* Set parameters */
	if (!getparint("trmin", &trmin))	trmin = 1;
	if (!getparint("trmax", &trmax))	trmax = INT_MAX;
	if (!getparint("verbose", &verbose))	verbose = 0;
	if (!getparint("vblock", &vblock))	vblock = 50;
	if (!getparint("endian", &endian))	endian = 1;
	if (!getparint("errmax", &errmax))	errmax = 0;
	if (!getparint("nsamp_max", &nsamp_max))		nsamp_max = SU_NFLTS;
	if (!getparint("buff", &buff))		buff = 1;


	/* Override binary format value */
	if (!getparint("over", &over))		over = 0;
	if (getparint("format", &format))	format_set = cwp_true;
	if (((over!=0) && (format_set))) {
		bh.format = format;
		if ( !((format==1) || (format==2) || (format==3)) ) {
 	          fprintf(stderr,"Specified format=%d not supported\n", format);
		  fprintf(stderr,"Assuming IBM floating point format, instead\n");
		}
	}

	/* Override conversion of IBM floating point data? */
	if (!getparint("conv", &conv))		conv = 1;

	/* Open files - first the tape */
	if (buff) {
		if ( tape[0] == '-' && tape[1] == '\0' ) tapefd = 0;
		else tapefd = open(tape, O_RDONLY, 0444);
	} else {
		if ( tape[0] == '-' && tape[1] == '\0' ) tapefp = stdin;
		else tapefp = fopen(tape, "r");
	}

	/* - the ebcdic header file in ascii */
	headerfp = efopen(hfile, "w");
	if (verbose) fprintf(stderr,"header file opened successfully\n");

	/* - the binary data file */
	binaryfp = efopen(bfile, "w");
	if (verbose) fprintf(stderr,"binary file opened successfully\n");

	/* Read the ebcdic raw bytes from the tape into the buffer */
	if (buff) {
		if (-1 == (int) read(tapefd, ebcbuf, EBCBYTES)) {
			if (verbose)
				fprintf(stderr,"tape read error on ebcdic header\n");
			if (++errcount > errmax)
				seperr("exceeded maximum io errors");
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	} else {
		 (int) fread(ebcbuf, 1, EBCBYTES, tapefp);
		 if (ferror(tapefp)) {
			if (verbose)
				fprintf(stderr,"tape read error on ebcdic header\n");
			if (++errcount > errmax)
				seperr("exceeded maximum io errors");
			clearerr(tapefp);
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	}

	/* Open pipe to use dd to convert ascii to ebcdic */
	sprintf(cmdbuf, "dd ibs=3200 of=%s conv=ascii cbs=80 count=1", hfile);
	pipefp = epopen(cmdbuf, "w");

	/* Write ebcdic stream from buffer into pipe */
	efwrite(ebcbuf, EBCBYTES, 1, pipefp);


	/* Read binary header from tape to bh structure */
	if (buff) {
		if (-1 == read(tapefd, (char *) &tapebh, BNYBYTES)) {
			if (verbose)
				fprintf(stderr,"tape read error on binary header\n");
			if (++errcount > errmax)
				err("exceeded maximum io errors");
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	} else {
		 fread((char *) &tapebh, 1, BNYBYTES, tapefp);
		 if (ferror(tapefp)) {
			if (verbose)
				fprintf(stderr,"tape read error on binary header\n");
			if (++errcount > errmax)
				err("exceeded maximum io errors");
			clearerr(tapefp);
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	}

	/* Convert from bytes to ints/shorts */
	tapebhed_to_bhed(&tapebh, &bh);

	/* if little endian machine, swap bytes in binary header */
	if (endian==0) for (i = 0; i < BHED_NKEYS; ++i) swapbhval(&bh,i);

	/* Override binary format value */
	if (!getparint("over", &over))		over = 0;
	if (getparint("format", &format))	format_set = cwp_true;
	if (((over!=0) && (format_set)))	bh.format = format;
  
	switch (bh.format) {
	case 1:
		if (verbose) fprintf(stderr,"assuming IBM floating point input\n");
		break;
	case 2:
		if (verbose) fprintf(stderr,"assuming 4 byte integer input\n");
		break;
	case 3:
		if (verbose) fprintf(stderr,"assuming 2 byte integer input\n");
		break;
   default:
    (over) ? warn("ignoring bh.format ... continue") :
      err("format not SEGY standard (1, 2 or 3)");
  }


	/* Compute length of trace (can't use sizeof here!) */
	if (!getparint("ns", &ns))  ns = bh.hns; /* let user override */
	if (!ns) err("samples/trace not set in binary header");
	bh.hns = ns;
	switch (bh.format) {
	case 3:
		nsegy = ns*2 + SEGY_HDRBYTES;
		break;
	case 2:
	case 1:
	default:
		nsegy = ns*4 + SEGY_HDRBYTES;
	}

	/* Write binary header from bhed structure to binary file */
	efwrite( (char *) &bh,1, BNYBYTES, binaryfp);

	/* Close binary and header files now to allow pipe into segywrite */
	efclose(binaryfp);
	if (verbose) fprintf(stderr,"binary file closed successfully\n");
	efclose(headerfp);
	epclose(pipefp);
	if (verbose) fprintf(stderr,"header file closed successfully\n");


	/* Read the traces */
	nsflag = cwp_false;
	itr = 0;
	while (itr < trmax) {
		int nread;

		if (buff) {

			if (-1 == 
			   (nread = (int) read(tapefd, (char *) &tapetr, nsegy))){
				if (verbose)
				      fprintf(stderr,"tape read error on trace %d\n", itr);
				if (++errcount > errmax)
				      err("exceeded maximum io errors");
			} else { /* Reset counter on successful tape IO */
				errcount = 0;
			}
		} else {
			nread = (int) fread((char *) &tapetr, 1, nsegy, tapefp);
			if (ferror(tapefp)) {
				if (verbose)
				      fprintf(stderr,"tape read error on trace %d\n", itr);
				if (++errcount > errmax)
				      err("exceeded maximum io errors");
				clearerr(tapefp);
			} else { /* Reset counter on successful tape IO */
				errcount = 0;
			}
		}
			
	if (!nread) break; /* middle exit loop instead of mile-long while */
	
		/* Convert from bytes to ints/shorts */
		tapesegy_to_segy(&tapetr, &tr);
		/* If little endian machine, then swap bytes in trace header */
		if (endian==0)
			for (i = 0; i < SEGY_NKEYS; ++i) swaphval(&tr,i);
	
		/* Check tr.ns field */
		if (!nsflag && ns != tr.ns) {
			fprintf(stderr,"discrepant tr.ns = %d with tape/user ns = %d\n"
				"\t... first noted on trace %d\n",
				tr.ns, ns, itr + 1);
			nsflag = cwp_true;
		}

		/* Convert and write desired traces */
		if (++itr >= trmin) {
			/* Convert IBM floats to native floats */
			  if (conv) {
				switch (bh.format) {
				case 1:
			  /* Convert IBM floats to native floats */
					ibm_to_float((int *) tr.data,
						(int *) tr.data, ns, endian);
					break;
				case 2:

			  /* Convert 4 byte integers to native floats */
					int_to_float((int *) tr.data,
						(float *) tr.data, ns, endian);
					break;
				case 3:
			  /* Convert 2 byte integers to native floats */
					short_to_float((short *) tr.data,
						(float *) tr.data, ns, endian);
					break;
				}
			}

			/* handle no ibm conversion for little endian case */
			if (conv==0 && endian==0)
				for (i = 0; i < ns ; ++i)
					swap_float_4(&tr.data[i]);

			/* Write the trace to disk */
			tr.ns = ns;
			puttr(&tr);

			/* Echo under verbose option */
			if (verbose && itr % vblock == 0)
				fprintf(stderr," %d traces from tape\n", itr);
		}
	}



	/* Re-iterate error in case not seen during run */
	if (nsflag) fprintf(stderr,"discrepancy found in header and trace ns values\n"
		"the value (%d) was used to extract traces", ns);


	/* Clean up (binary & header files already closed above) */
	(buff) ? eclose(tapefd):
		 efclose(tapefp);
	if (verbose) fprintf(stderr,"tape closed successfully\n");


   finish_susep();
	return (0);
}

#ifdef _HPUX_SOURCE
static void ibm_to_float(int from[], int to[], int n, int endian)
{
	/* HP version of ibm_to_float */

    register int fconv, fmant, i, t, dummy;

	dummy = endian;

    for (i=0;i<n;++i) {
        fconv = from[i];

        /* next lines modified (M.J.Rutty 20/9/92) */
        /* if (fconv) { */
        /* fmant = 0x00ffffff & fconv; */

        fmant = 0x00ffffff & fconv;
        if (!fmant)
          fconv=0;
        else {
          /* end modifications */
            t = (int) ((0x7f000000 & fconv) >> 22) - 130;
            while (!(fmant & 0x00800000)) { --t; fmant <<= 1; }
            if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
            else if (t <= 0) fconv = 0;
            else fconv = (0x80000000 & fconv) |(t << 23)|(0x007fffff & fmant);
        }
        to[i] = fconv;
    }
    return;
}

#else /* use the regular ibm_to_float routine */

static void ibm_to_float(int from[], int to[], int n, int endian)
/***********************************************************************
ibm_to_float - convert between 32 bit IBM and IEEE floating numbers
************************************************************************
Input::
from		input vector
to		output vector, can be same as input vector
endian		byte order =0 little endian (DEC, PC's)
			    =1 other systems 
************************************************************************* 
Notes:
Up to 3 bits lost on IEEE -> IBM

Assumes sizeof(int) == 4

IBM -> IEEE may overflow or underflow, taken care of by 
substituting large number or zero

Only integer shifting and masking are used.
************************************************************************* 
Credits: CWP: Brian Sumner,  c.1985
*************************************************************************/
{
    register int fconv, fmant, i, t;

    for (i=0;i<n;++i) {

	fconv = from[i];

	/* if little endian, i.e. endian=0 do this */
	if (endian==0) fconv = (fconv<<24) | ((fconv>>24)&0xff) |
		((fconv&0xff00)<<8) | ((fconv&0xff0000)>>8);

	if (fconv) {
	    fmant = 0x00ffffff & fconv;
	    /* The next two lines were added by Toralf Foerster */
	    /* to trap non-IBM format data i.e. conv=0 data  */
	    if (fmant == 0)
		seperr(" data are not in IBM FLOAT Format !");	
	    t = (int) ((0x7f000000 & fconv) >> 22) - 130;
	    while (!(fmant & 0x00800000)) { --t; fmant <<= 1; }
	    if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
	    else if (t <= 0) fconv = 0;
	    else fconv = (0x80000000 & fconv) |(t << 23)|(0x007fffff & fmant);
	}
	to[i] = fconv;
    }
    return;
}
#endif

static void tapebhed_to_bhed(const tapebhed *tapebhptr, bhed *bhptr)
/****************************************************************************
tapebhed_to_bhed -- converts the seg-y standard 2 byte and 4 byte
	integer header fields to, respectively, the
	machine's short and int types. 
*****************************************************************************
Input:
tapbhed		pointer to array of 
*****************************************************************************
Notes:
The present implementation assumes that these types are actually the "right"
size (respectively 2 and 4 bytes), so this routine is only a placeholder for
the conversions that would be needed on a machine not using this convention.
*****************************************************************************
Author: CWP: Jack  K. Cohen, August 1994
****************************************************************************/

{
	register int i;
	Value val;
	
	/* convert binary header, field by field */
	for (i = 0; i < BHED_NKEYS; ++i) {
		gettapebhval(tapebhptr, i, &val);
		putbhval(bhptr, i, &val);
	}
}

static void tapesegy_to_segy(const tapesegy *tapetrptr, segy *trptr)
/****************************************************************************
tapesegy_to_segy -- converts the seg-y standard 2 byte and 4 byte
		    integer header fields to, respectively, the machine's
		    short and int types. 
*****************************************************************************
Input:
tapetrptr	pointer to trace in "tapesegy" (SEG-Y on tape) format

Output:
trptr		pointer to trace in "segy" (SEG-Y as in	 SU) format
*****************************************************************************
Notes:
Also copies float data byte by byte.  The present implementation assumes that
the integer types are actually the "right" size (respectively 2 and 4 bytes),
so this routine is only a placeholder for the conversions that would be needed
on a machine not using this convention.	 The float data is preserved as
four byte fields and is later converted to internal floats by ibm_to_float
(which, in turn, makes additonal assumptions).
*****************************************************************************
Author: CWP:Jack K. Cohen,  August 1994
****************************************************************************/
{
	register int i;
	Value val;
	
	/* convert header trace header fields */
	for (i = 0; i < SEGY_NKEYS; ++i) {
		gettapehval(tapetrptr, i, &val);
		puthval(trptr, i, &val);
	}

	/* copy the optional portion */
	memcpy((char *)&(trptr->otrav)+2, tapetrptr->unass, 60);

	/* copy data portion */
	memcpy(trptr->data, tapetrptr->data, 4*nsamp_max);
}

static void int_to_float(int from[], float to[], int n, int endian)
/****************************************************************************
Author:	J.W. de Bruijn, May 1995
****************************************************************************/
{
	register int i;
  
	if (endian == 0) {
		for (i = 0; i < n; ++i) {
			swap_int_4_new(&from[i]);
			to[i] = (float) from[i];
		}
	} else {
		for (i = 0; i < n; ++i) {
			to[i] = (float) from[i];
		}
	}
}

static void short_to_float(short from[], float to[], int n, int endian)
/****************************************************************************
short_to_float - type conversion for additional SEG-Y formats
*****************************************************************************
Author: Delft: J.W. de Bruijn, May 1995
Modified by: Baltic Sea Reasearch Institute: Toralf Foerster, March 1997 
****************************************************************************/
{
	register int i;
  
	if (endian == 0) {
		for (i = n-1; i >= 0 ; --i) {
			swap_short_2(&from[i]);
			to[i] = (float) from[i];
		}
	} else { 
		for (i = n-1; i >= 0 ; --i)
			to[i] = (float) from[i];
	}
}
void handle_additional(){
char list[4096],array[100][128],temp_ch[128],temp2_ch[128];
int ierr,i,nt;


getch_add_string("suinput=1");
if(1==getch("only_list","d",&ierr)){
  if(ierr==1){
    for(i=0; i < SU_NKEYS; i++){
			sprintf(temp_ch,"%s=skip",hdr[i].key);
      getch_add_string(temp_ch);
    }
  }
}
if(1==getch("dump_all","d",&ierr)){
  if(ierr==1){
    for(i=0; i < SU_NKEYS; i++){
			sprintf(temp_ch,"%s=%s",hdr[i].key,hdr[i].key);
      getch_add_string(temp_ch);
    }
  }
}
if(1==getch("nignore","d",&nt)){
  for(i=1; i <= nt; i++){
    sprintf(temp_ch,"ignore%d",i);
    if(0==getch(temp_ch,"s",temp2_ch))
      seperr("nignore=%d ignore%d not specified\n",nt,i);
    sprintf(temp_ch,"%s=skip",temp2_ch);
    getch_add_string(temp_ch);
  }
}

if(0!=getch("ignorelist","s",list)){
  nt=array_decode(list,array); 
  for(i=0; i <= nt; i++){
    sprintf(temp_ch,"%s=skip",array[i]);
    getch_add_string(temp_ch);
  }
}

if(1==getch("nkeep","d",&nt)){
  for(i=1; i <= nt; i++){
    sprintf(temp_ch,"keep%d",i);
    if(0==getch(temp_ch,"s",temp2_ch))
      seperr("nkeep=%d keep%d not specified\n",nt,i);
    sprintf(temp_ch,"%s=%s",temp2_ch,temp2_ch);
    getch_add_string(temp_ch);
  }
}
if(0!=getch("keeplist","s",list)){
  nt=array_decode(list,array); 
  for(i=0; i <= nt; i++){
    sprintf(temp_ch,"%s=%s",array[i],array[i]);
    getch_add_string(temp_ch);
  }
}

}

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int array_decode(char list[4096],char array[100][128])
_XFUNCPROTOEND
#else
int array_decode(list,array)
char  list[4096], array[100][128];
#endif
{
int i2,i1,tot;

i2=0; tot=0;i1=0;
while(list[i2] !='\0' && i2 < 4096){
    if(list[i2]!=':'){
      array[tot][i1]=list[i2];
      i1++;
    }
    else {
      array[tot][i1]='\0';
      i1=0; tot++;
    }
i2++;
}
array[tot][i1]='\0';
return(tot+1);
}

void swap_int_4_new(int *tni4)
/**************************************************************************
swap_int_4_new              swap a 4 byte integer
***************************************************************************/
{
 *tni4=(((*tni4>>24)&0xff) | ((*tni4&0xff)<<24) |
            ((*tni4>>8)&0xff00) | ((*tni4&0xff00)<<8));
}



#else
#include<sep3d.h>
int main( int argc, char **argv)
{
fprintf(stderr,"You did not configure with SU support \n");
}
#endif
