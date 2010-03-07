/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: ciff_datadefs.h,v 1.1 2005/05/25 15:38:07 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CIFF_DATADEFS_INCLUDED
#define CIFF_DATADEFS_INCLUDED


struct ciff_header {
    unsigned short byteorder;
    unsigned long headerlength;
    char type[5];
    char subtype[5];
    unsigned long version;
    unsigned long reserved1;
    unsigned long reserved2;
};

struct ciff_direntry {
    unsigned short type;
    unsigned long length;
    unsigned long offset;
};

#endif /* CIFF_DATADEFS_INCLUDED */
