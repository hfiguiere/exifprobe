/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: fujifilm_datadefs.h,v 1.5 2005/06/25 15:12:52 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef FUJIFILM_DATADEFS_INCLUDED
#define FUJIFILM_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id fujifilm_model_id[] = {
    { "DX-10", 6, FUJI_DX10, 1, 1 },
    { "FUJIFILM FinePixA204", 20, FUJI_FP_A204, 1, 1 },
    { "FinePix40i", 11, FUJI_40i, 1, 1 },
    { "FinePix4900ZOOM",16, FUJI_4900ZOOM, 1, 1 },
    { "FinePix E550   ",16, FUJI_E550, 1, 1 },
    { "FinePix F601 ZOOM",18, FUJI_F601ZOOM, 1, 1 },
    { "FinePixS1Pro",13, FUJI_S1PRO, 1, 1 },
    { "FinePixS2Pro",13, FUJI_S2PRO, 1, 1 },
    { "FinePix S20Pro ",16, FUJI_S20PRO, 1, 1 },
    { "FinePix S5000 ",15, FUJI_S5000, 1, 1 },
    { "MX-1700ZOOM",12, FUJI_MX1700ZOOM, 1, 1 },
    { "SP-2000",12, FUJI_SP2000, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* FUJIFILM_DATADEFS_INCLUDED */
