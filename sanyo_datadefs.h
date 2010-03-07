/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: sanyo_datadefs.h,v 1.1 2005/05/25 15:38:08 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef SANYO_DATADEFS_INCLUDED
#define SANYO_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id sanyo_model_id[] = {
    /* ###%%% more models, SX215,  */
    { "SR6 ", 4, SANYO_SR6, 1, 1 },
    { "SX113", 5, SANYO_SX113, 1, 1 },
    { "SX215", 5, SANYO_SX215, 1, 1 },
    { 0, 0, 0, 0, 0 }
};
#endif  /* SANYO_DATADEFS_INCLUDED */
