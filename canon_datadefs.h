/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: canon_datadefs.h,v 1.2 2005/06/21 14:52:06 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CANON_DATADEFS_INCLUDED
#define CANON_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id canon_model_id[] = {
    { "Canon DIGITAL IXUS", 18, CANON_IXUS, 1, 1 },
    { "Canon EOS-1D", 13, CANON_EOS1D, 5, 1 },
    { "Canon EOS-1D Mark II", 20, CANON_EOS1DMARKII, 5, 1 },
    { "Canon EOS-1DS", 13, CANON_EOS1DS, 5, 1 },
    { "Canon EOS-1Ds Mark II", 21, CANON_EOS1DSMARKII, 5, 1 },
    { "Canon EOS 10D", 13, CANON_EOS10D, 3, 1 },
    { "Canon EOS 20D", 13, CANON_EOS20D, 4, 1 },
    { "Canon EOS D30", 13, CANON_EOSD30, 2, 1 },
    { "Canon EOS D60", 13, CANON_EOSD60, 1, 1 },
    { "Canon EOS 350", 13, CANON_EOS350, 1, 1 },
    { "Canon EOS DIGITAL REBEL", 23, CANON_EOSDIGITALREBEL, 1, 1 },
    { "Canon PowerShot A400", 21, CANON_POWERSHOT_A400, 1, 1 },
    { "Canon PowerShot A5", 19, CANON_POWERSHOT_A5, 1, 1 },
    { "Canon PowerShot G2", 18, CANON_POWERSHOT_G2, 1, 1 },
    { "Canon PowerShot Pro1", 20, CANON_POWERSHOT_PRO1, 1, 1 },
    { "Canon PowerShot S70", 19, CANON_POWERSHOT_S70, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* CANON_DATADEFS_INCLUDED */
