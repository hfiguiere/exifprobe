/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: misc.h,v 1.4 2005/06/09 02:21:30 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Miscellaneous definitions                                          */

#ifndef MISC_INCLUDED
#define MISC_INCLUDED

/* These magic numbers are defined by well-known specification        */
#define TIFF_MAGIC      0x2a
#define TIFF_INTEL      0x4949
#define TIFF_MOTOROLA   0x4d4d

#define ORF1_MAGIC      0x4f52   /* "RO" 16 bits/sample PMI=1 res=314 */
#define ORF2_MAGIC      0x5352   /* "RS" 12 bits/sample PMI=2 res=72  */

#define RW2_MAGIC       0x0055   /* "U\0" for Panasonic RAW and RW2 */

/* Some of these magic numbers are private to this program, used to   */
/* id file formats which have no unique magic number.                 */
#define PROBE_NOMAGIC       0x0000
#define PROBE_TIFFMAGIC     TIFF_MAGIC
#define PROBE_ORF1MAGIC     ORF1_MAGIC
#define PROBE_ORF2MAGIC     ORF2_MAGIC
#define PROBE_JPEGMAGIC     0xffd8  /* JPEG_SOI */
#define PROBE_CIFFMAGIC     0x1a
#define PROBE_JP2MAGIC      0x0d0a870a
#define PROBE_MRWMAGIC      0x004d524d  /* "MRM" (ANY byte order)     */
#define PROBE_RAFMAGIC      0x4655      /* "FU"JIFILM-CCDRAW  (MOT)   */
#define PROBE_X3FMAGIC      0x62564f46  /* "FOVb" (INTEL)             */
#define PROBE_RW2MAGIC      RW2_MAGIC


/* TIFF entry type id                                                 */
#define BYTE        1
#define ASCII       2
#define SHORT       3
#define LONG        4
#define RATIONAL    5
#define SBYTE       6
#define SSHORT      8
#define UNDEFINED   7
#define SLONG       9
#define FLOAT       11
#define SRATIONAL   10
#define DOUBLE      12

char *
strdup_value(struct ifd_entry *entry, FILE *inptr,
                                        unsigned long fileoffset_base);

#endif  /* MISC_INCLUDED */
