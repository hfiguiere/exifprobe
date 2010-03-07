/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: konica_datadefs.h,v 1.2 2005/06/03 14:04:22 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef KONICA_DATADEFS_INCLUDED
#define KONICA_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id konica_model_id[] = {
    { "Konica Digital Camera Q-M100", 29, KONICA_QM100, 1, 1 },
    { "Konica Digital Camera KD-400Z", 30, KONICA_KD400Z, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* KONICA_DATADEFS_INCLUDED */
