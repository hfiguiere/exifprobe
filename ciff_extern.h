/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: ciff_extern.h,v 1.3 2005/06/06 18:50:04 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CIFF_EXTERN_INCLUDED
#define CIFF_EXTERN_INCLUDED

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Canon CIFF-specific external definitions                           */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

extern char *cifftagname(unsigned short);
extern int print_ciff_header(struct fileheader *,unsigned long);

extern unsigned long process_ciff(FILE *,struct fileheader *,unsigned long,
                                    unsigned long,struct image_summary *,
                                    char *,int,int);
extern unsigned long process_ciff_dir(FILE *,unsigned long,unsigned long,
                                    char *,char *,struct image_summary *,
                                    unsigned short,int,int);
extern unsigned long process_ciff_direntry(FILE *,unsigned short,unsigned short,
                                        struct ciff_direntry *, unsigned long,
                                        char *,struct image_summary *,int,int);
extern struct ciff_direntry *read_ciff_direntry(FILE *, struct ciff_direntry *,
                                            unsigned short,unsigned long);
extern struct ciff_header *read_ciffheader(FILE *,unsigned short,unsigned long);

extern void print_ciff_value(FILE *,struct ciff_direntry *,unsigned short,
                                            unsigned long,unsigned long,
                                            char *,struct image_summary *,
                                            int,int);
extern void print_ciff_taginfo(struct ciff_direntry *,unsigned long);
extern int is_compressed_crw(FILE *,unsigned long,unsigned long);
extern unsigned long print_ciffinheapdata(FILE *,unsigned short,char *,char *,
                                                unsigned long, unsigned long,
                                                unsigned short,int);
extern unsigned long print_ciffinrecdata(FILE *,unsigned short,char *,
                                                unsigned long, unsigned long,
                                                unsigned short,int);

#endif  /* CIFF_EXTERN_INCLUDED */
