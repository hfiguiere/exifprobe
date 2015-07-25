/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CANON_EXTERN_INCLUDED
#define CANON_EXTERN_INCLUDED

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Canon camera maker-specific external definitions                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

extern char *maker_canon_tagname(unsigned short,int);
extern char *maker_canon1_tagname(unsigned short,int);
extern int canon_model_number(char *,char *);
extern int print_canon_serialno(unsigned long);

extern void canon1_interpret_value(struct ifd_entry *);
extern void print_canon_makervalue(struct ifd_entry *,int,int,char *);
extern void print_canon1_makervalue(struct ifd_entry *,int,int,char *);
extern void print_canon_offset_makervalue(FILE *,unsigned short,
                                    struct ifd_entry *,unsigned long,
                                    struct image_summary *,char *,char *,
                                    int,int,int,int);
extern void canon1_offset_makervalue(FILE *,unsigned short,
                                    struct ifd_entry *,unsigned long,
                                    struct image_summary *,char *,char *,
                                    int,int,int,int);

extern unsigned long canon_camera_settings(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_shotinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_customfunctions(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int,int);
extern unsigned long canon_customfunctions_unk(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_customfunctionsD30(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_customfunctions10D(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_customfunctions20D(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_customfunctions1D(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_sensorinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_aspectinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_pictureinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_colorspace(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_whitebalancetable(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_imageinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int,
                                        struct image_summary *);
extern unsigned long canon_exposureinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int,
                                        struct image_summary *);
extern unsigned long canon_focusinfo(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern unsigned long canon_ct_to_datetime(FILE *,unsigned short,char *,
                                        unsigned long,unsigned long,int);
extern int maker_canon_value_is_offset(struct ifd_entry *,int);

#endif  /* CANON_EXTERN_INCLUDED */
