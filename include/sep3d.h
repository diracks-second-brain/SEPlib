#ifndef _SEP_90_H
#define _SEP_90_H

/* Include file <sep90.h>  This file contains function definitions for all
 * the sep90 routines. It should be includded in all the c-files in
 * that library.
 *
 * I should probably end up adding this to include.dir/sepcube.h
 *
 */

#include <sepcube.h>
#include <stdio.h>

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
extern int sep_put_number_keys(char* , int* );
extern int sep_get_number_keys(char* , int* );
extern int sep_tag_is_pipe(char *tag);
extern void sep_3d_close(void);
extern void init_3d(void);


extern int sep_put_key(char* , char* , char* , char*, int* );
extern int sep_get_key_index(char* , char* , int* );
extern int sep_get_key_name(char* , int* , char* );
extern int sep_get_key_type(char* , int* , char* );
extern int sep_get_key_fmt(char* , int* , char* );
extern int sep_get_key_first_byte(char* , int* , int* );
extern int sep_get_header_bytes(char* , int* );
extern int sep_copy_grid(char* , char* );

extern int sep_put_header_format_tag(const char* , char* );
extern int sep_get_header_format_tag(const char* , char** );
extern int fget_header_format_tag(char* , char* );
extern int fget_grid_format_tag(char* , char* );
extern int sep_copy_hff(char* , char* );
extern int sep_copy_gff(char* , char* );
extern int sep_set_no_headers(char* );
extern int sep_set_regular_grid(char* );
extern int sep_set_no_grid(char* );

extern int sep_put_grid_format_tag(const char* , char* );
extern int sep_get_grid_format_tag(const char* , char** );

extern int sep_put_data_axis_par(const char*,int*,int*,float*,float*,char*);
extern int sep_get_data_axis_par(const char*,int*,int*,float*,float*,char*);
extern int sep_get_number_data_axes(const char*,int*);

extern int sep_put_header_axis_par(const char*,int*,int*,float*,float*,char* );
extern int sep_get_header_axis_par(const char*,int*,int*,float*,float*,char*);
extern int sep_get_number_header_axes(const char*,int*);

extern int sep_put_grid_axis_par(const char*,int*,int*,float*,float*,char* );
extern int sep_get_grid_axis_par(const char*,int*,int*,float*,float*,char*);
extern int sep_get_number_grid_axes(const char*,int*);

extern int sep_put_val_by_index(char* , int* , int* , int*, void*);
extern int sep_get_val_by_index(char* , int* , int* , int*, void*);
extern int sep_put_val_by_name(char* , int* , char* , int*, void*);
extern int sep_get_val_by_name(char* , int* , char* , int*, void*);
extern int sep_put_val_headers(char* , int* , int* , void*);
extern int sep_get_val_headers(char* , int* , int* , void*);
extern int sep_insert_val_by_index(char* , int* , int*, void*, void*);
extern int sep_extract_val_by_index(char* , int* , int*, void*, void*);

extern int sep_put_grid_window(char*, int*, int*, int*, int*, int*, int*);
extern int sep_get_grid_window(char*, int*, int*, int*, int*, int*, int*);

extern int sep_copy_data_pointer(char* , char* );
extern int sep_copy_header_keys(char* , char* );
extern int sep_reorder_data(char*,char*,int,int,int*);
extern int sep_reorder_data_fast(char*,char*,int,int,int*,int);

_XFUNCPROTOEND
#else
extern void sep_3d_close();
extern void init_3d();
extern int sep_put_number_keys();
extern int sep_get_number_keys();
extern int stdout_pipe();

extern int sep_put_key();
extern int sep_get_key_index();
extern int sep_get_key_name();
extern int sep_get_key_type();
extern int sep_get_key_fmt();
extern int sep_get_key_first_byte();
extern int sep_get_header_bytes();

extern int sep_put_header_format_tag();
extern int sep_get_header_format_tag();
extern int fget_header_format_tag();
extern int fget_grid_format_tag();
extern int sep_copy_hff();
extern int sep_copy_gff();
extern int sep_set_no_headers();
extern int sep_set_regular_grid();
extern int sep_set_no_grid();

extern int sep_put_data_axis_par();
extern int sep_get_data_axis_par();
extern int sep_get_number_data_axes();
extern int sep_put_header_axis_par();
extern int sep_get_header_axis_par();
extern int sep_get_number_header_axes();
extern int sep_put_grid_axis_par();
extern int sep_get_grid_axis_par();
extern int sep_get_number_grid_axes();


extern int sep_put_val_by_index();
extern int sep_get_val_by_index();
extern int sep_put_val_by_name();
extern int sep_get_val_by_name();
extern int sep_put_val_headers();
extern int sep_get_val_headers();
extern int sep_insert_val_by_index();
extern int sep_extract_val_by_index();

extern int sep_put_grid_window();
extern int sep_get_grid_window();

extern int sep_copy_data_pointer();
extern int sep_copy_header_keys();
extern int sep_reorder_data();
#endif

#endif
/*  $Id: sep3d.h,v 1.1.1.1 2004/03/25 06:37:22 cvs Exp $ */
/*  $Id: sep3d.h,v 1.1.1.1 2004/03/25 06:37:22 cvs Exp $ */
