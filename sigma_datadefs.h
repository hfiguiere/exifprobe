/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: sigma_datadefs.h,v 1.2 2005/06/15 23:34:09 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef SIGMA_DATADEFS_INCLUDED
#define SIGMA_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id sigma_model_id[] = {
    { "SIGMA SD9", 10, SIGMA_SD9, 1, 1 },
    { "SIGMA SD10", 11, SIGMA_SD10, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
#endif  /* SIGMA_DATADEFS_INCLUDED */
