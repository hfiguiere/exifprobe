/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: sony_datadefs.h,v 1.2 2005/06/21 15:19:54 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef SONY_DATADEFS_INCLUDED
#define SONY_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id sony_model_id[] = {
    { "CYBERSHOT", 9, SONY_CYBERSHOT, 1, 0 },
    { "DIGITALMAVICA", 13, SONY_DIGITALMAVICA, 1, 0 },
    { "DSC-D700", 7, SONY_DSCD700, 1, 0 },
    { "DSC-V3", 6, SONY_DSCV3, 1, 0 },
    { "MAVICA", 6, SONY_MAVICA, 1, 0 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* SONY_DATADEFS_INCLUDED */
