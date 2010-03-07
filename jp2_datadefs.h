/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: jp2_datadefs.h,v 1.1 2005/05/24 19:07:06 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef JP2_DATADEFS_INCLUDED
#define JP2_DATADEFS_INCLUDED
struct jp2_header {
    unsigned long length;
    unsigned long type;
    unsigned long magic;
};

struct jp2box {
    unsigned long lbox;
    unsigned long tbox;
    unsigned long dataoffset;   /* from boxoffset                     */
    unsigned long boxoffset;    /* ###%%% should be long long         */
    unsigned long boxlength;    /* ###%%% should be long long         */
};

#endif  /* JP2_DATADEFS_INCLUDED */
