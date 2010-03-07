/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: panasonic_datadefs.h,v 1.2 2005/06/21 15:16:22 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PANASONIC_DATADEFS_INCLUDED
#define PANASONIC_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id panasonic_model_id[] = {
    { "DMC-FX7", 7, PANASONIC_DMCFX7, 1, 1 },
    { "DMC-FZ1", 7, PANASONIC_DMCFZ1, 1, 1 },
    { "DMC-FZ2", 7, PANASONIC_DMCFZ2, 1, 0 },
    { "DMC-FZ3", 7, PANASONIC_DMCFZ3, 1, 1 },
    { "DMC-FZ10", 8, PANASONIC_DMCFZ10, 1, 1 },
    { "DMC-FZ15", 8, PANASONIC_DMCFZ15, 1, 1 },
    { "DMC-FZ20", 8, PANASONIC_DMCFZ20, 1, 1 },
    { "DMC-LC5", 7, PANASONIC_DMCLC5, 1, 1 },
    { "DMC-LC33", 8, PANASONIC_DMCLC33, 1, 1 },
    { "DMC-LC40", 8, PANASONIC_DMCLC40, 1, 1 },
    { "DMC-LC43", 8, PANASONIC_DMCLC43, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* PANASONIC_DATADEFS_INCLUDED */
