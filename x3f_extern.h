/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: x3f_extern.h,v 1.2 2005/07/24 21:03:53 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef X3F_EXTERN_INCLUDED
#define X3F_EXTERN_INCLUDED

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Fujifilm X3F-specific external definitions                         */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

extern struct x3f_header *read_x3fheader(FILE *,unsigned long);
extern int print_x3f_header(FILE *,struct fileheader *,unsigned long);
extern unsigned long process_x3f(FILE *,struct fileheader *,unsigned long,
                                                    struct image_summary *,
                                                    char *,int,int);
extern unsigned long process_x3f_dir(FILE *,unsigned short,struct x3f_header *,
                                        unsigned long,struct image_summary *,
                                        char *,int,int);
extern void process_x3f_direntry(FILE *,unsigned short,struct x3f_direntry *,
                                        unsigned long,struct image_summary *,
                                        char *,int,int);
extern struct x3f_direntry *read_x3f_direntry(FILE *,unsigned short,
                                                    struct x3f_direntry *,
                                                    unsigned long offset);
extern void print_x3f_direntry(FILE *,struct x3f_direntry *,unsigned long,
                                                            char *,int,int);
extern struct x3f_imag_header *read_x3f_imag_header(FILE *,unsigned short,unsigned long);
extern struct x3f_prop_header *read_x3f_prop_header(FILE *,unsigned short,unsigned long);
extern struct x3f_camf_header *read_x3f_camf_header(FILE *,unsigned short,unsigned long);
extern unsigned long display_x3f_imag_header(struct x3f_direntry *,struct x3f_imag_header *,
                                                                            char *,int,int);
extern unsigned long display_x3f_prop_header(struct x3f_direntry *,struct x3f_prop_header *,
                                                                            char *,int,int);
extern unsigned long display_x3f_camf_header(struct x3f_direntry *,struct x3f_camf_header *,
                                                                            char *,int,int);
extern unsigned long process_x3f_props(FILE *,unsigned short,unsigned long,unsigned long,
                                                    unsigned long,char *,unsigned long,int);
extern void print_property(FILE *,unsigned short,unsigned long,unsigned long,unsigned long,
                                                                            char *,int,int);

#endif  /* X3F_EXTERN_INCLUDED */
