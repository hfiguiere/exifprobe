/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: pentax_datadefs.h,v 1.2 2005/06/21 15:19:20 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PENTAX_DATADEFS_INCLUDED
#define PENTAX_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id pentax_model_id[] = {
    { "PENTAX Optio 330 ",17, ASAHI_PENTAX330, 1, 1 },
    { "PENTAX Optio330RS",18, ASAHI_PENTAX330RS, 2, 2 },
    { "PENTAX Optio 430 ",17, ASAHI_PENTAX430, 1, 1 },
    { "PENTAX Optio430RS",18, ASAHI_PENTAX430RS, 2, 2 },
    { "PENTAX Optio 550",17, PENTAX_OPTIO550, 3, 3 },
    { "PENTAX Optio 750Z",18, PENTAX_OPTIO750Z, 3, 3 },
    { "PENTAX Optio S ",15, PENTAX_OPTIOS, 3, 3 },
    { "PENTAX Optio S5i ",17, PENTAX_OPTIOS5i, 3, 3 },
    { "PENTAX *ist D ",14, PENTAX_STARIST_D, 3, 3 },
    { "PENTAX *ist DS ",15, PENTAX_STARIST_DS, 3, 3 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* PENTAX_DATADEFS_INCLUDED */

