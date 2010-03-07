/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: epson_datadefs.h,v 1.1 2005/05/25 15:38:07 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef EPSON_DATADEFS_INCLUDED
#define EPSON_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id epson_model_id[] = {
    { "PhotoPC 850Z", 13, EPSON_850Z, 1, 1 },
    { "PhotoPC 3000Z", 14, EPSON_3000Z, 1, 1 },
    { "PhotoPC 3100Z", 14, EPSON_3100Z, 1, 1 },
    { "L-500V ", 8, EPSON_L500V, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* EPSON_DATADEFS_INCLUDED */
