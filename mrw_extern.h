/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: mrw_extern.h,v 1.1 2005/05/25 00:53:56 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef MRW_EXTERN_INCLUDED
#define MRW_EXTERN_INCLUDED

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Minolta MRW-specific external definitions                           */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

extern int print_mrw_header(struct fileheader *,unsigned long);
extern unsigned long process_mrw(FILE *,unsigned long,unsigned long,
                                    unsigned long, struct image_summary *,
                                    char *,int);
extern unsigned long process_prd(FILE *,unsigned long,unsigned long,
                                    struct image_summary *,int);
extern unsigned long process_wbg(FILE *,unsigned long,unsigned long,
                                    struct image_summary *,int);
extern unsigned long process_rif(FILE *,unsigned long,unsigned long,
                                    struct image_summary *,int);

#endif  /* MRW_EXTERN_INCLUDED */
