/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: datadefs.h,v 1.2 2005/06/09 03:00:25 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef DATADEFS_INCLUDED
#define DATADEFS_INCLUDED

struct ifd_entry {
    unsigned short tag;
    unsigned short value_type;
    unsigned long count;
    unsigned long value;
};


#include "ciff_datadefs.h"
#include "mrw_datadefs.h"
#include "jp2_datadefs.h"
#include "x3f_datadefs.h"

struct fileheader {
    unsigned short file_marker;     /* initial ushort from file       */
    unsigned long probe_magic;      /* magic number assigned (misc.h) */
    struct ciff_header *ciff_header;    /* iff found to be CIFF       */
    struct jp2_header jp2_header;       /* JPEG2000 JP2 format        */
    struct mrw_header mrw_header;       /* Minolta MRW "raw" format   */
    struct x3f_header *x3f_header;  /* Sigma/Fovenon X3F              */
};

#endif  /* DATADEFS_INCLUDED */
