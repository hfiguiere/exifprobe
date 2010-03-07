/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: kodak_datadefs.h,v 1.2 2005/07/12 04:57:53 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef KODAK_DATADEFS_INCLUDED
#define KODAK_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id kodak_model_id[] = {
    { "DC200 Zoom (V05.00)", 20, KODAK_DC200, 0, 0 },
    { "DC210 Zoom (V05.00)", 20, KODAK_DC210, 0, 0 },
    { "DCS720X", 7, KODAK_DCS720X, 0, 0 },
    { "DCS760C", 7, KODAK_DCS760C, 0, 0 },
    { "Kodak DC120 ZOOM Digital Camera", 32, KODAK_DK120, 0, 0 },
    { "Kodak DC240 ZOOM Digital Camera", 32, KODAK_DC240, 0, 0 },
    { "Kodak DC4800 ZOOM Digital Camera", 33, KODAK_DC4800, 0, 0 },
    { "KODAK DC25 DIGITAL CAMERA", 25, KODAK_DC25, 0, 0 },
    { "KODAK LS443 ZOOM DIGITAL CAMERA", 32, KODAK_LS443, 0, 0 },
    { "Kodak Digital Science DC50 Zoom Camera", 39, KODAK_DC50, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* KODAK_DATADEFS_INCLUDED */
