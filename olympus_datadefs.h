/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: olympus_datadefs.h,v 1.4 2005/07/08 15:02:00 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef OLYMPUS_DATADEFS_INCLUDED
#define OLYMPUS_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id olympus_model_id[] = {
    { "C700UZ", 7, OLYMPUS_700UZ, 1, 1 },
    { "C70Z,C7000Z",12, OLYMPUS_C70ZC7000Z, 1, 1 },
    { "C960Z",5, OLYMPUS_960Z, 1, 1 },
    { "C2040Z", 7, OLYMPUS_2040Z, 1, 1 },
    { "C2500L ",7, OLYMPUS_C2500L, 1, 1 },
    { "C3030Z", 7, OLYMPUS_3030Z, 1, 1 },
    { "C3040Z", 7, OLYMPUS_3040Z, 1, 1 },
    { "C8080WZ",8, OLYMPUS_C8080WZ, 1, 1 },
    { "E-1 ",4, OLYMPUS_E_1, 1, 1 },
    { "E-300",5, OLYMPUS_E_300, 1, 1 },
    { "u40D,S500,uD500 ",4, OLYMPUS_u40D, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* OLYMPUS_DATADEFS_INCLUDED */
