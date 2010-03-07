/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: minolta_datadefs.h,v 1.4 2005/07/24 21:15:46 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef MINOLTA_DATADEFS_INCLUDED
#define MINOLTA_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id minolta_model_id[] = {
    { "DiMAGE 5", 9, MINOLTA_DIMAGE5, 1, 1 },
    { "DiMAGE 7", 9, MINOLTA_DIMAGE7, 1, 1 },
    { "DiMAGE 7Hi", 11, MINOLTA_DIMAGE7Hi, 1, 1 },
    { "DiMAGE 7i", 10, MINOLTA_DIMAGE7i, 1, 1 },
    { "DiMAGE A1", 10, MINOLTA_DIMAGEA1, 1, 1 },
    { "DiMAGE A2", 10, MINOLTA_DIMAGEA2, 1, 1 },
    { "DiMAGE A200", 12, MINOLTA_DIMAGEA200, 1, 1 },
    { "DiMAGE EX", 10, MINOLTA_DIMAGEEX, 1, 1 },
    { "DiMAGE F100", 12, MINOLTA_DIMAGEF100, 1, 1 },
    { "DiMAGE F200", 12, MINOLTA_DIMAGEF200, 1, 1 },
    { "DiMAGE F300", 12, MINOLTA_DIMAGEF300, 1, 1 },
    { "DiMAGE S404", 12, MINOLTA_DIMAGES404, 1, 1 },
    { "DiMAGE S414", 12, MINOLTA_DIMAGES414, 1, 1 },
    { "DiMAGE X", 9, MINOLTA_DIMAGEX, 1, 1 },
    { "DiMAGE X20", 11, MINOLTA_DIMAGEX20, 1, 1 },
    { "DYNAX 7D", 9, MINOLTA_DYNAX7D, 1, 1 },
    { "MAXXUM 7D", 10, MINOLTA_MAXXUM7D, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* MINOLTA_DATADEFS_INCLUDED */
