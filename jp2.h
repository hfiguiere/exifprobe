/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: jp2.h,v 1.1 2005/05/24 19:06:11 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef JP2_INCLUDED
#define JP2_INCLUDED

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* JP2/JPEG2000-specific value definitions                           */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#define JP2_PRFL    0x7072666c
#define JP2_FTYP    0x66747970
#define JP2_JP2H    0x6a703268
#define JP2_JP2C    0x6a703263
#define JP2_JP2I    0x6a703269
#define JP2_XML     0x786d6c20
#define JP2_UUID    0x75756964
#define JP2_UINF    0x75696e66

/* JP2H sub-boxes                                                     */
#define JP2_ihdr    0x69686472
#define JP2_bpcc    0x62706363
#define JP2_colr    0x636f6c72
#define JP2_pclr    0x70636c72
#define JP2_cdef    0x63646566
#define JP2_res     0x72657320
#define JP2_resc    0x72657363
#define JP2_resd    0x72657364

/* uinf sub-boxes                                                     */
#define JP2_ulst    0x75637374
#define JP2_url     0x75726c20


/* magic values                                                       */
#define JP2_BR      0x6a703220      /* "brand"                        */

#define READSIZE    8192            /* used when reading text chunks  */

#include "jp2_datadefs.h"
#include "jp2tags.h"
#include "jp2_extern.h"

#endif  /* JP2_INCLUDED */
