/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: leica_datadefs.h,v 1.2 2005/07/24 21:30:58 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef LEICA_DATADEFS_INCLUDED
#define LEICA_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id leica_model_id[] = {
    { "DIGILUX 2",10, LEICA_DIGILUX2, 0, 0 },
    { "digilux 4.3",12, LEICA_DIGILUX43, 1, 1 },
    { "R9 - Digital Back DMR",22,LEICA_R9_DB_DMR, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* LEICA_DATADEFS_INCLUDED */
