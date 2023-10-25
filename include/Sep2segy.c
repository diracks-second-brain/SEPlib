/*$

=head1 NAME

Sep2segy - Convert from SEPlib3d to  segy


=head1 SYNOPSIS

Sep2segy < stdout tape=   [pars]


=head1 INPUT PARAMETERS

=over 4

=item  tape    -file

       input tape device or seg-y filename (see notes)

=item verbose  - int
      
      [0] silent, 1= echo every vblock traces

=item vblock  - int

      [50]   echo every vblock traces whtn verbose option set

=item buff  -  int

   1		for buffered device (9-track reel tape drive)	

   0 possibly useful for 8mm EXABYTE drive	

=item  conv  -  int

	1 convert	=0 don't convert to IBM format			

=item create_reel  - int

  [0] whether (1) or not(0) to create header rather than read from disk  


=item  hfile - file

	hfile=header	ebcdic card image header file			

=item bfile  -  file

	bfile=binary	binary header file				

=item trmin  -  int

  1 first trace to write					

=item	trmax  -  int

  INT_MAX  last trace to write			       

=item endian - int

 [1] 0 for little-endian machines (PC's, DEC,etc...)

=item errmax - int

   0	allowable number of consecutive tape IO errors	


=item format - int

		override value of format in binary header file	

=item nsamp_max   - int

       By default each trace is assumed to be SU_NFLTS (1,000,000+)
       in size and a copy of that length is done for each trace.
       Set this to something more reasonalbe (2000) for a massive speed up.



=back

=head1 DESCRIPTION

Converts from SEP to segy formats


=head1 NOTES

 Note: The header files may be created with  'segyhdrs'.		

 Note: For buff=1 (default) tape is accessed with 'write', for buff=0	

	tape is accessed with fwrite. Try the default setting of buff=1 

 for all tape types.						

 Caveat: may be slow on an 8mm streaming (EXABYTE) tapedrive		

 Warning: segyread or segywrite to 8mm tape is fragile. Allow time	

	   between successive reads and writes.				

 Precaution: make sure tapedrive is set to read/write variable blocksize

	   tapefiles.							
									

=head1 CATEGORY

B<converters>

=cut
*/

/* Copyright (c) Colorado School of Mines, 1999.*/
/* All rights reserved.                       */

#include<superset.h>
/* SEGYWRITE: $Revision: 1.1.1.1 $ ; $Date: 2004/03/25 06:37:26 $    */
#ifdef SU_SUPPORT

#ifdef SUXDR    /* begin if SUXDR */
#include "su_xdr.h"

#else		/* else if not SUXDR */


#include "su.h"
#include "segy.h"
#include "tapebhdr.h"

#endif		/* end if  SUXDR */

#include "tapesegy.h"
#include "bheader.h"
#include<superset.h>
int nsamp_max;
void add_extra();


char *sdoc[] = {
"									",
" SEGYWRITE - write an SEG-Y tape					",
"									",
" segywrite <stdin tape=						",
"NULL"};


/*
 * Warning: may return the error message "efclose: fclose failed"
 *	 intermittently when segyreading/segywriting to 8mm EXABYTE tape,
 *	 even if actual segyread/segywrite is successful. However, this
 *	 may indicate that your tape drive has been set to a fixed block
 *	 size. Tape drives should be set to variable block size before reading
 *	 or writing tapes in the SEG-Y format.
 *
 * Credits:
 *	SEP: Einar Kjartansson
 *	CWP: Jack, Brian, Chris
 *	   : John Stockwell (added EXABYTE functionality)
 * Notes:
 *	Brian's subroutine, float_to_ibm, for converting IEEE floating
 *	point to IBM floating point is NOT portable and must be
 *	altered for non-IEEE machines.	See the subroutine notes below.
 *
 *	On machines where shorts are not 2 bytes and/or ints are not 
 *	4 bytes, routines to convert SEGY 16 bit and 32 bit integers 
 *	will be required.
 *
 *	The program, segyhdrs, can be used to make the ascii and binary
 *	files required by this code.
 */

/**************** end self doc ***********************************/

/* typedefs */
#ifdef SUXDR		/* begin if SUXDR */
#if defined(_CRAYMPP)	  /* begin if _CRAYMPP */
typedef short fourbyte;
#else			  /* else if SUXDR but not _CRAYMPP */
typedef int fourbyte;
#endif			  /* end if _CRAYMPP */
#endif			/* end if SUXDR */

/* subroutine prototypes */
#ifdef SUXDR		/* begin if  SUXDR */
static void float_to_ibm(fourbyte *from, fourbyte *to, int n, int endian);

#else			/* if not SUXDR */
static void float_to_ibm(int from[], int to[], int n, int endian);
static void bhed_to_tapebhed(const bhed *bhptr, tapebhed *tapebhptr); 
static void
	segy_to_tapesegy(const segy *trptr, tapesegy *tapetrptr, size_t nsegy); 

/*  globals */
tapesegy tapetr;
tapebhed tapebh;
#endif			/* end if SUXDR */

/* globals */
segy tr;
bhed bh;

int
main(int argc, char **argv)
{
	cwp_String tape;	/* name of raw tape device		*/
	cwp_String hfile;	/* name of ebcdic header file		*/
	cwp_String bfile;	/* name of binary header file		*/

#ifdef SUXDR	/* begin SUXDR */
	int j;			/* counter				*/
	FILE *headerfp;		/* file pointer to header file		*/
#else		/* else if not SUXDR */
	FILE *pipefp;		/* file pointer for popen read		*/
#endif		/* end if SUXDR */
	FILE *tapefp=NULL;	/* file pointer for tape		*/
	FILE *binaryfp;		/* file pointer for bfile		*/

	int tapefd=0;		/* file descriptor for tape buff=0	*/

	int i;			/* counter				*/
	int ns;			/* number of data samples		*/
	size_t nsegy;		/* size of whole trace in bytes		*/
	int itr;		/* current trace number			*/
	int trmax;		/* last trace to write			*/
	int trmin;		/* first trace to write			*/
	int verbose;		/* echo every ...			*/
	int vblock;		/* ... vblock traces with verbose=1	*/
	int buff;		/* buffered or unbuffered device	*/
	int endian;		/* =0 little endian; =1 big endian	*/
	int conv;		/* =1 IBM format =0 don't convert	*/
	int errmax;		/* max consecutive tape io errors	*/
	int errcount = 0;	/* counter for tape io errors		*/
	int format = 0;		/* tape format				*/
	cwp_Bool format_set = cwp_false; /* tape format			*/

#ifdef SUXDR	/* begin if SUXDR */
#if defined(CRAY) /* begin if defined CRAY */
#if defined(_CRAYMPP)  /* begin if defined _CRAYMPP */
	fourbyte imone = -1;	/* constant for Fortran linkage		*/
	fourbyte fns;		/* for Fortran CRAYMPP linkage		*/
#else		/* CRAY but not _CRAYMPP */	
	int ier;		/* CRAY ibmfloat error flag		*/
	fourbyte ione = -1;	/* constant for Fortran linkage		*/
#endif		/* end if _CRAYMPP */
#endif /* end if SUXDR and CRAY but not _CRAYMPP  */

	char ebcbuf[EBCBYTES+1];/* ebcdic data buffer			*/
	char bhbuf[BNYBYTES];	/* binary reel header buffer		*/
	char *trbuf;		/* output trace buffer			*/
	XDR bhed_xdr, bhbuf_xdr;/* for handling binary reel header	*/
	XDR trhd_xdr;
	unsigned int trstart;	/* "offset" of trhd stream buffer	*/

#else /* not Cray and not SUXDR */
	char cmdbuf[BUFSIZ];	/* dd command buffer			*/
	char ebcbuf[EBCBYTES];	/* ebcdic data buffer			*/
#endif		/* end if  SUXDR */
	
	/* Initialize */
	initargs(argc, argv);
sep_add_doc_line("NAME");
sep_add_doc_line("    Sep2segy - Convert from SEPlib3d to segy");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("    Sep2segy < stdout tape= [pars]");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    tape -file");
sep_add_doc_line("               input tape device or seg-y filename (see notes)");
sep_add_doc_line("");
sep_add_doc_line("    verbose - int");
sep_add_doc_line("              [0] silent, 1= echo every vblock traces");
sep_add_doc_line("");
sep_add_doc_line("    vblock - int");
sep_add_doc_line("              [50]   echo every vblock traces whtn verbose option set");
sep_add_doc_line("");
sep_add_doc_line("    buff - int");
sep_add_doc_line("           1            for buffered device (9-track reel tape drive)   ");
sep_add_doc_line("");
sep_add_doc_line("           0 possibly useful for 8mm EXABYTE drive");
sep_add_doc_line("");
sep_add_doc_line("    conv - int");
sep_add_doc_line("                1 convert       =0 don't convert to IBM format");
sep_add_doc_line("");
sep_add_doc_line("    create_reel - int");
sep_add_doc_line("          [0] whether (1) or not(0) to create header rather than read from disk");
sep_add_doc_line("");
sep_add_doc_line("    hfile - file");
sep_add_doc_line("                hfile=header    ebcdic card image header file");
sep_add_doc_line("");
sep_add_doc_line("    bfile - file");
sep_add_doc_line("                bfile=binary    binary header file");
sep_add_doc_line("");
sep_add_doc_line("    trmin - int");
sep_add_doc_line("          1 first trace to write");
sep_add_doc_line("");
sep_add_doc_line("    trmax - int");
sep_add_doc_line("          INT_MAX  last trace to write");
sep_add_doc_line("");
sep_add_doc_line("    endian - int");
sep_add_doc_line("         [1] 0 for little-endian machines (PC's, DEC,etc...)");
sep_add_doc_line("");
sep_add_doc_line("    errmax - int");
sep_add_doc_line("           0    allowable number of consecutive tape IO errors");
sep_add_doc_line("");
sep_add_doc_line("    format - int");
sep_add_doc_line("                        override value of format in binary header file");
sep_add_doc_line("");
sep_add_doc_line("    nsamp_max - int");
sep_add_doc_line("               By default each trace is assumed to be SU_NFLTS (1,000,000+)");
sep_add_doc_line("               in size and a copy of that length is done for each trace.");
sep_add_doc_line("               Set this to something more reasonalbe (2000) for a massive speed up.");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("    Converts from SEP to segy formats");
sep_add_doc_line("");
sep_add_doc_line("NOTES");
sep_add_doc_line("     Note: The header files may be created with  'segyhdrs'.                ");
sep_add_doc_line("");
sep_add_doc_line("     Note: For buff=1 (default) tape is accessed with 'write', for buff=0   ");
sep_add_doc_line("");
sep_add_doc_line("            tape is accessed with fwrite. Try the default setting of buff=1 ");
sep_add_doc_line("");
sep_add_doc_line("     for all tape types.                                            ");
sep_add_doc_line("");
sep_add_doc_line("     Caveat: may be slow on an 8mm streaming (EXABYTE) tapedrive            ");
sep_add_doc_line("");
sep_add_doc_line("     Warning: segyread or segywrite to 8mm tape is fragile. Allow time      ");
sep_add_doc_line("");
sep_add_doc_line("               between successive reads and writes.                         ");
sep_add_doc_line("");
sep_add_doc_line("     Precaution: make sure tapedrive is set to read/write variable blocksize");
sep_add_doc_line("");
sep_add_doc_line("               tapefiles.");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    converters");
sep_add_doc_line("");

   doc(SOURCE);
	requestdoc(1);
  add_extra();


	/* Get parameters */
	MUSTGETPARSTRING("tape", &tape);
	if (!getparstring("hfile", &hfile))	hfile = "header";
	if (!getparstring("bfile", &bfile))	bfile = "binary";
	if (!getparint	 ("trmin", &trmin))	trmin = 1;
	if (!getparint	 ("trmax", &trmax))	trmax = INT_MAX;
	if (!getparint	 ("verbose", &verbose)) verbose = 0;
	if (!getparint	 ("vblock", &vblock))	vblock = 50;
	if (!getparint	 ("buff", &buff))	buff = 1;
	if (!getparint	 ("conv", &conv))	conv = 1;
	if (!getparint	 ("endian", &endian))	endian = 1;
	if (!getparint	 ("errmax", &errmax))	errmax = 0;
  if (!getparint("nsamp_max", &nsamp_max))    nsamp_max = SU_NFLTS;
	if (getparint("format", &format))	format_set = cwp_true;
	
	/* Check parameters */
	if (trmin < 1 || trmax < 1 || trmax < trmin)
		err("bad trmin/trmax values, trmin = %d, trmax = %d",
			trmin, trmax);

	/* Get first trace early to be sure that binary file is ready */
	gettr(&tr);

	/* Open files - first the tape */
	if (buff) tapefd = eopen(tape, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	else tapefp = efopen(tape, "w");
	if (verbose) warn("tape opened successfully ");

#ifdef SUXDR	/* begin SUXDR */

        /* Open ascii header file */
	headerfp = efopen(hfile, "r");

        if (verbose) warn("header file opened successfully");

        /* - binary header file */
        binaryfp = efopen(bfile, "r");
	xdrstdio_create(&bhed_xdr,binaryfp,XDR_DECODE);
	xdrmem_create(&bhbuf_xdr,bhbuf,BNYBYTES,XDR_ENCODE);


        if (verbose) warn("binary file opened successfully");

        /* Read ascii header into buffer and blank newlines & nulls */
	memset(&(ebcbuf[0]),' ',EBCBYTES);
	for(i = 0; i<EBCBYTES; i += 80) {
		fgets(&(ebcbuf[i]),81, headerfp);
                j = (int) strlen(&(ebcbuf[i]));
		ebcbuf[i+j] = ' ';
		j--;
		if(ebcbuf[j] == '\n') ebcbuf[j] = ' ';
	}
	/* Convert to EBCDIC */
        zebc(&(ebcbuf[0]),&(ebcbuf[0]),EBCBYTES);

        efclose(headerfp);
        if (verbose) warn("header file closed successfully");
   }

        /* Write ebcdic stream to tape */
        if (buff) {
                if (EBCBYTES != write(tapefd, ebcbuf, EBCBYTES)) {
                        if (verbose)
                                warn("tape write error on ebcdic header");
                        if (++errcount > errmax)
                                err("exceeded maximum io errors");
                } else { /* Reset counter on successful tape IO */
                        errcount = 0;
                }
        } else {
                 fwrite(ebcbuf, 1, EBCBYTES, tapefp);
                 if (ferror(tapefp)) {
                        if (verbose)
                                warn("tape write error on ebcdic header");
                        if (++errcount > errmax)
                                err("exceeded maximum io errors");
                        clearerr(tapefp);
                } else { /* Reset counter on successful tape IO */
                        errcount = 0;
                }
        }

        /* Read binary file into bh structure */
	xdrbhdrsub(&bhed_xdr, &bh);

	/* update requisite field */
        if (format_set) bh.format = format;  /* manually set format */
        bh.ntrpr  = 1;  /* one trace per record */

#else  /* not SUXDR */

	/* - binary header file */
	binaryfp = efopen(bfile, "r");
	if (verbose) warn("binary file opened successfully");

	/* Open pipe to use dd to convert ascii to ebcdic */
	sprintf(cmdbuf, "dd if=%s conv=ebcdic cbs=80 obs=3200", hfile);
	pipefp = epopen(cmdbuf, "r");

	/* Read ebcdic stream from pipe into buffer */
	efread(ebcbuf, 1, EBCBYTES, pipefp);

	/* Write ebcdic stream to tape */
	if (buff) {
		if (EBCBYTES != write(tapefd, ebcbuf, EBCBYTES)) {
			if (verbose)
				warn("tape write error on ebcdic header");
			if (++errcount > errmax)
				err("exceeded maximum io errors");
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	} else {
		 fwrite(ebcbuf, 1, EBCBYTES, tapefp);
		 if (ferror(tapefp)) {
			if (verbose)
				warn("tape write error on ebcdic header");
			if (++errcount > errmax)
				err("exceeded maximum io errors");
			clearerr(tapefp);
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	}

	/* Read binary file into bh structure */
	efread((char *) &bh, 1, BNYBYTES, binaryfp);
        if (format_set) bh.format = format;  /* manually set format  */

	if (bh.ntrpr == 0) bh.ntrpr  = 1;	/* one trace per record */

#endif		/* end if SUXDR */

	/* Compute trace size (can't use HDRBYTES here!) */
	ns = bh.hns;
	if (!ns) err("bh.hns not set in binary header");
	nsegy = ns*4 + SEGY_HDRBYTES;

#ifdef SUXDR 	/* begin SUXDR */
	/* Convert from ints to shorts */
	xdrbhdrsub(&bhbuf_xdr, &bh);

        /* Write binary structure to tape */
        if (buff) {
                if (BNYBYTES != write(tapefd, bhbuf, BNYBYTES)) {
                        if (verbose)
                                warn("tape write error on binary header");
                        if (++errcount > errmax)
                                err("exceeded maximum io errors");
                } else { /* Reset counter on successful tape IO */
                        errcount = 0;
                }
        } else {
                 fwrite(bhbuf, 1, BNYBYTES, tapefp);
                 if (ferror(tapefp)) {
                        if (verbose)
                                warn("tape write error on binary header");
                        if (++errcount > errmax)
                                err("exceeded maximum io errors");
                        clearerr(tapefp);
                } else { /* Reset counter on successful tape IO */
                        errcount = 0;
                }
        }

#else /* not SUXDR */

	/* if little endian (endian=0) swap bytes of binary header */
	if (endian==0) for (i = 0; i < BHED_NKEYS; ++i) swapbhval(&bh,i);


	/* Convert from ints/shorts to bytes */
	bhed_to_tapebhed(&bh, &tapebh);


	/* Write binary structure to tape */
	if (buff) {
		if (BNYBYTES != write(tapefd, (char *) &tapebh, BNYBYTES)) {
			if (verbose)
				warn("tape write error on binary header");
			if (++errcount > errmax)
				err("exceeded maximum io errors");
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	} else {
		 fwrite((char *) &tapebh, 1, BNYBYTES, tapefp);
		 if (ferror(tapefp)) {
			if (verbose)
				warn("tape write error on binary header");
			if (++errcount > errmax)
				err("exceeded maximum io errors");
			clearerr(tapefp);
		} else { /* Reset counter on successful tape IO */
			errcount = 0;
		}
	}
#endif		/* end if SUXDR */


	/* Copy traces from stdin to tape */

#ifdef SUXDR	/* begin SUXDR */

	trbuf = (char *) alloc1(nsegy, sizeof(char));
	xdrmem_create(&trhd_xdr,trbuf,(unsigned int) nsegy,XDR_ENCODE);
	trstart = xdr_getpos(&trhd_xdr);

#endif 		/* end if SUXDR */

	itr = 0;
	do {

		/* Set/check trace header words */
		tr.tracr = ++itr;
		if (tr.ns != ns)
			err("conflict: tr.ns = %d, bh.ns = %d: trace %d",
					tr.ns, ns, itr);

		/* Convert and write desired traces */
		if (itr >= trmin) {

#ifdef SUXDR 	/* begin SUXDR */
         		/* convert trace header to SEGY standard */       
			if(FALSE == xdr_setpos(&trhd_xdr,trstart)) 
			    err("%s: trouble \"seeking\" start of trace",
				__FILE__);
			xdrhdrsub(&trhd_xdr,&tr);

                        /* Convert internal floats to IBM floats */
			if (conv) {
#if defined(CRAY)
#if defined(_CRAYMPP)
			    float_to_ibm((fourbyte *) (&(tr.data[0])),
					 (fourbyte *) (&(tr.data[0])),
					 ns, endian);
/* Stew's Fortran routine...
                            fns = ns;
                            IBMFLT(tr.data,tr.data,&fns,&imone);
*/
#else /* !_CRAYMPP */
			    USSCTI(tr.data,tr.data,&ione,&ns,&ier);
#endif /* _CRAYMPP */
#else /* !CRAY */
			    float_to_ibm((fourbyte *) (&(tr.data[0])),
					 (fourbyte *) (&(tr.data[0])),
					 ns, endian);
#endif /* !CRAY */
			    memcpy(trbuf+SEGY_HDRBYTES,(char *) tr.data,
				ns*4*sizeof(char));
			} else {
			    xdr_vector(&trhd_xdr,(char *) tr.data,
				ns, sizeof(float), xdr_float);
			}

                        /* Write the trace to tape */
                        if (buff) {
                            if (nsegy !=
                               write(tapefd, trbuf, nsegy)){
                                if (verbose)
                                    warn("tape write error on trace %d", itr);
                                if (++errcount > errmax)
                                    err("exceeded maximum io errors");
                            } else { /* Reset counter on successful tape IO */
                                errcount = 0;
                            }
                        } else {
                            fwrite(trbuf,sizeof(char),nsegy,tapefp);
                            if (ferror(tapefp)) {
                                if (verbose)
                                    warn("tape write error on trace %d", itr);
                                if (++errcount > errmax)
                                    err("exceeded maximum io errors");
                                    clearerr(tapefp);
                            } else { /* Reset counter on successful tape IO */
                                errcount = 0;
                            }
                        }

#else /* not SUXDR */
		
			/* Convert internal floats to IBM floats */
			if (conv)
				float_to_ibm((int *) tr.data, (int *) tr.data,
								ns, endian);

		       /* handle no ibm conversion for little endian case */
		       if (conv==0 && endian==0)
				for (i = 0; i < ns ; ++i)
					swap_float_4(&tr.data[i]);
			
			/* if little endian, swap bytes in header */
			if (endian==0)
			    for (i = 0; i < SEGY_NKEYS; ++i) swaphval(&tr,i);

			/* Convert from ints/shorts to bytes */
			segy_to_tapesegy(&tr, &tapetr, nsegy);

			/* Write the trace to tape */
			if (buff) {
			    if (nsegy !=
			       write(tapefd, (char *) &tapetr, nsegy)){
				if (verbose)
				    warn("tape write error on trace %d", itr);
				if (++errcount > errmax)
				    err("exceeded maximum io errors");
			    } else { /* Reset counter on successful tape IO */
				errcount = 0;
			    }
			} else {
			    fwrite((char *)&tapetr,1,nsegy,tapefp);
			    if (ferror(tapefp)) {
				if (verbose)
				    warn("tape write error on trace %d", itr);
				if (++errcount > errmax)
				    err("exceeded maximum io errors");
				    clearerr(tapefp);
			    } else { /* Reset counter on successful tape IO */
				errcount = 0;
			    }
			}

#endif		/* end if SUXDR */

			/* Echo under verbose option */
			if (verbose && itr % vblock == 0)
				warn(" %d traces written to tape", itr);
		}
	} while (gettr(&tr) && itr < trmax);


	/* Clean up */
	(buff) ?  eclose(tapefd) :
		  efclose(tapefp);
	if (verbose) warn("tape closed successfully");

#ifdef SUXDR	/* begin SUXDR */
	xdr_destroy(&trhd_xdr);
	xdr_destroy(&bhed_xdr);
	xdr_destroy(&bhbuf_xdr);
#endif 		/* end if SUXDR */

	efclose(binaryfp);
	if (verbose) warn("binary file closed successfully");

#ifndef SUXDR	/* begin not SUXDR */
	epclose(pipefp);
#endif		/* end if not SUXDR */

  finish_susep();
	return (0);
}

#ifdef SUXDR	/* begin SUXDR */

/* Assumes fourbyte == 4 byte integer */
static void float_to_ibm(fourbyte *from, fourbyte *to, int n, int endian)
/**********************************************************************
 float_to_ibm - convert between 32 bit IBM and IEEE floating numbers
*********************************************************************** 
Input:
from       input vector
n          number of floats in vectors
endian     =0 for little endian machine, =1 for big endian machines

Output:
to         output vector, can be same as input vector

*********************************************************************** 
Notes:
Up to 3 bits lost on IEEE -> IBM

IBM -> IEEE may overflow or underflow, taken care of by 
substituting large number or zero

Only integer shifting and masking are used.
*********************************************************************** 
Credits:     CWP: Brian Sumner
***********************************************************************/
{
    register fourbyte fconv, fmant, t;
    register int i;

    for (i=0;i<n;++i) {
        fconv = from[i];
        if (fconv) {
            fmant = (0x007fffff & fconv) | 0x00800000;
            t = (fourbyte) ((0x7f800000 & fconv) >> 23) - 126;
            while (t & 0x3) { ++t; fmant >>= 1; }
            fconv = (0x80000000 & fconv) | (((t>>2) + 64) << 24) | fmant;
        }
        if(endian==0)
                fconv = (fconv<<24) | ((fconv>>24)&0xff) |
                        ((fconv&0xff00)<<8) | ((fconv&0xff0000)>>8);

        to[i] = fconv;
    }
    return;
}

#else	/* not SUXDR */

#ifdef _HPUX_SOURCE
void float_to_ibm(int from[], int to[], int n, int endian)
{
    register int fconv, fmant, i, t, dummy;

	dummy = endian;

    for (i=0;i<n;++i) {
        fconv = from[i];
        if (fconv) {
            fmant = (0x007fffff & fconv) | 0x00800000;
            t = (int) ((0x7f800000 & fconv) >> 23) - 126;
            while (t & 0x3) { ++t; fmant >>= 1; }
            fconv = (0x80000000 & fconv) | (((t>>2) + 64) << 24) | fmant;
        }
        to[i] = fconv;
    }
    return;
}

#else

/* Assumes sizeof(int) == 4 */
static void float_to_ibm(int from[], int to[], int n, int endian)
/**********************************************************************
 float_to_ibm - convert between 32 bit IBM and IEEE floating numbers
*********************************************************************** 
Input:
from	   input vector
n	   number of floats in vectors
endian	   =0 for little endian machine, =1 for big endian machines

Output:
to	   output vector, can be same as input vector

*********************************************************************** 
Notes:
Up to 3 bits lost on IEEE -> IBM

IBM -> IEEE may overflow or underflow, taken care of by 
substituting large number or zero

Only integer shifting and masking are used.
*********************************************************************** 
Credits:     CWP: Brian Sumner
***********************************************************************/
{
    register int fconv, fmant, i, t;

    for (i=0;i<n;++i) {
	fconv = from[i];
	if (fconv) {
	    fmant = (0x007fffff & fconv) | 0x00800000;
	    t = (int) ((0x7f800000 & fconv) >> 23) - 126;
	    while (t & 0x3) { ++t; fmant >>= 1; }
	    fconv = (0x80000000 & fconv) | (((t>>2) + 64) << 24) | fmant;
	}
	if(endian==0)
		fconv = (fconv<<24) | ((fconv>>24)&0xff) |
			((fconv&0xff00)<<8) | ((fconv&0xff0000)>>8);

	to[i] = fconv;
    }
    return;
}

#endif 

static void bhed_to_tapebhed(const bhed *bhptr, tapebhed *tapebhptr)
/***************************************************************************
bhed_tape_bhed -- converts the binary tape header in the machine's short
and int types to, respectively, the seg-y standard 2 byte and 4 byte integer
types.
****************************************************************************
Input:
bhptr		pointer to binary header vector

Output:
tapebhptr	pointer to tape binary header vector
****************************************************************************
Notes:
The present implementation assumes that these types are actually the "right"
size (respectively 2 and 4 bytes), so this routine is only a placeholder for
the conversions that would be needed on a machine not using this convention.
****************************************************************************
Author: CWP: Jack K. Cohen  August 1994
***************************************************************************/
{
	register int i;
	Value val;
	
	/* convert the binary header field by field */
	for (i = 0; i < BHED_NKEYS; ++i) {
		getbhval(bhptr, i, &val);
		puttapebhval(tapebhptr, i, &val);
	}
}

static
void segy_to_tapesegy(const segy *trptr, tapesegy *tapetrptr, size_t nsegy)
/***************************************************************************
tapesegy_to_segy -- converts the integer header fields from, respectively,
		    the machine's short and int types to the  seg-y standard
		    2 byte and 4 byte types.
****************************************************************************
Input:
trptr		pointer to SU SEG-Y data vector		
nsegy		whole size of a SEG-Y trace in bytes

Output:
tapetrptr	pointer to tape SEG-Y data vector
****************************************************************************
Notes:
Also copies the float data byte by byte.  The present implementation assumes
that the integer types are actually the "right" size (respectively 2 and
4 bytes), so this routine is only a placeholder for the conversions that
would be needed on a machine not using this convention.	 The float data
is preserved as four byte fields and is later converted to internal floats
by float_to_ibm (which, in turn, makes additonal assumptions)
****************************************************************************
Author: CWP: Jack K. Cohen  August 1994
***************************************************************************/
{
	register int i;
	Value val;
	
	/* convert trace header, field by field */
	for (i = 0; i < SEGY_NKEYS; ++i) {
		gethval(trptr, i, &val);
		puttapehval(tapetrptr, i, &val);
	}
	
	/* copy optional portion */
	memcpy(tapetrptr->unass, (char *)&(trptr->otrav)+2, 60);

	/* copy data portion */
/*	memcpy(tapetrptr->data, trptr->data, 4*SU_NFLTS);*/
	memcpy(tapetrptr->data, trptr->data, 4*nsamp_max);

} 
void add_extra(){
int doit;
int n1,format;
char bfile[1024],hfile[1024];
char temp_ch[3200];
  FILE *binaryfp;   /* file pointer for bfile   */
  FILE *headerfp;   /* file pointer for hfile   */

  getch_add_string("suoutput=1");
  if(0==getch("create_reel","d",&doit)) doit=0;
  if(doit==1){
   if(0==getch("bfile","s",bfile))strcpy(bfile,"binary");
   if(0==getch("hfile","s",hfile))strcpy(hfile,"header");
  /* Open file for writing */
  binaryfp = efopen(bfile, "w");
  /* Create binary header; set all named fields */
  memset((void *) &bh, (int) '\0', BNYBYTES);
  if(hetch("n1","d",&n1)==0) seperr("trouble obtaining n1");
  bh.hns=n1;
  if(getch("format","d",&format)==0)format=1;
  bh.format=format;
  /* Write binary header from bh structure to designated file */
  efwrite( (char *) &bh, 1, BNYBYTES, binaryfp);
  /* Clean up */
  efclose(binaryfp);
  headerfp = efopen(hfile, "w");
  memset((void *) temp_ch, (int) '\0', EBCBYTES);
  sprintf(temp_ch,"Synthetic header made by Segy2sep\n");
  efwrite( (char *) temp_ch, 1, EBCBYTES, headerfp);
  efclose(headerfp);
  }



}
#endif		/* end if SUXDR */
#else
#include<sep3d.h>
int main( int argc, char **argv)
{
fprintf(stderr,"You did not configure with SU support \n");
}
#endif

