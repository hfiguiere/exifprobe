/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: jp2_extern.h,v 1.3 2005/07/24 21:30:11 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef JP2_EXTERN_INCLUDED
#define JP2_EXTERN_INCLUDED

extern void print_jp2type(unsigned long,int);
extern void print_jp2box(FILE *,struct jp2box *,int);
extern int print_jp2_header(struct fileheader *,unsigned long);
extern int list_jp2box(FILE *,struct jp2box *,char *,int,int);
extern char *jp2000tagname(unsigned short);
extern struct jp2box *read_jp2box(FILE *,unsigned long);

extern unsigned long process_jp2(FILE *,unsigned long,struct image_summary *,
                                                            char *,int);
extern unsigned long process_jp2_ftyp(FILE *,struct jp2box *,
                                        struct image_summary *,int);
extern unsigned long process_jp2_jp2h(FILE *,struct jp2box *,
                                        struct image_summary *,int);
extern unsigned long process_jp2_ihdr(FILE *,struct jp2box *,
                                        struct image_summary *,int);
extern unsigned long process_jp2_uuid(FILE *,struct jp2box *,
                                        struct image_summary *,int);
extern unsigned long process_jp2_uinf(FILE *,struct jp2box *,
                                        struct image_summary *,int);
extern unsigned long process_jp2_jp2c(FILE *,struct jp2box *,
                                        struct image_summary *,int);

extern unsigned long process_jp2_colr(FILE *,struct jp2box *,int);
extern unsigned long process_jp2_res(FILE *,struct jp2box *,int);
extern unsigned long process_jp2_xml(FILE *,struct jp2box *,int);
extern unsigned long process_jp2_jp2i(FILE *,struct jp2box *,int);
extern unsigned long process_jp2_ulst(FILE *,struct jp2box *,int);
extern unsigned long process_jp2_de(FILE *,struct jp2box *,int);

extern unsigned long process_jpeg2000_codestream(FILE *,unsigned long,
                                                    unsigned long,
                                                    struct image_summary *,
                                                    int);

extern unsigned long process_jpc_siz(FILE *,unsigned long,unsigned short,
                                                    struct image_summary *,
                                                    int);
extern unsigned long process_jpc_cod(FILE *,unsigned long,unsigned short,
                                                    struct image_summary *,
                                                    int);
extern unsigned long process_jpc_qcd(FILE *,unsigned long,unsigned short,
                                                    struct image_summary *,
                                                    int);

#endif  /* JP2_EXTERN_INCLUDED */
