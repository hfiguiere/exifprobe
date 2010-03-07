/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: nikon_datadefs.h,v 1.3 2005/06/30 18:36:00 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef NIKON_DATADEFS_INCLUDED
#define NIKON_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id nikon_model_id[] = {
    { "E700", 5, NIKON_700, 1, 1 },
    { "E775", 5, NIKON_775, 2, 2 },
    { "E800", 5, NIKON_800, 1, 1 },
    { "E885", 5, NIKON_885, 2, 2 },
    { "E900", 5, NIKON_900, 1, 1 },
    { "E950", 5, NIKON_950, 1, 1 },
    { "E990", 5, NIKON_990, 2, 2 },
    { "E995", 5, NIKON_995, 2, 2 },
    { "E3700", 6, NIKON_3700, 2, 2 },
    { "E5000", 6, NIKON_5000, 2, 2 },
    { "E5200", 6, NIKON_5200, 2, 2 },
    { "E5700", 6, NIKON_5700, 2, 2 },
    { "E8700", 6, NIKON_8700, 2, 2 },
    { "E8800", 6, NIKON_8800, 2, 2 },
    { "NIKON D1 ",10, NIKON_D1, 2, 2 },
    { "NIKON D1X",10, NIKON_D1X, 2, 2 },
    { "NIKON D70", 10, NIKON_D70, 2, 2 },
    { "NIKON D100 ", 12, NIKON_D100, 2, 2 },
    { "NIKON D2H",10, NIKON_D2H, 2, 2 },
    { "NIKON D2X",10, NIKON_D2X, 2, 2 },
    { "SQ  ",5, NIKON_SQ, 2, 2 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* NIKON_DATADEFS_INCLUDED */
