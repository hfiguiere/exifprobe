/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: casio_datadefs.h,v 1.2 2005/06/21 14:53:24 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CASIO_DATADEFS_INCLUDED
#define CASIO_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id casio_model_id[] = {
    { "QV-2000UX", 9, CASIO_QV2000UX, 1, 1 },
    { "QV-3000EX", 9, CASIO_QV3000EX, 1, 1 },
    { "QV-4000  ", 9, CASIO_QV4000, 1, 1 },
    { "QV-8000SX", 9, CASIO_QV8000SX, 1, 1 },
    { "QV-R51 ", 7, CASIO_QVR51, 2, 2 },
    { "EX-P505", 7, CASIO_EX_P505, 2, 2 },
    { "EX-P600", 7, CASIO_EX_P600, 2, 2 },
    { "EX-P700", 7, CASIO_EX_P700, 2, 2 },
    { "EX-S100", 7, CASIO_EX_S100, 2, 2 },
    { "EX-Z55 ", 7, CASIO_EX_Z55, 2, 2 },
    { "GV-20", 5, CASIO_GV_20, 2, 2 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* CASIO_DATADEFS_INCLUDED */
