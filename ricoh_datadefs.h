/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: ricoh_datadefs.h,v 1.1 2005/06/21 15:34:51 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef RICOH_DATADEFS_INCLUDED
#define RICOH_DATADEFS_INCLUDED

#include "maker_datadefs.h"

struct camera_id ricoh_model_id[] = {
    { "RDC-5300 ", 9, RICOH_RDC5300, 0, 0 },
    { "RDC-6000", 9, RICOH_RDC6000, 0, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif  /* RICOH_DATADEFS_INCLUDED */
