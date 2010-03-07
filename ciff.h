/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: ciff.h,v 1.1 2005/05/25 16:03:03 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CIFF_INCLUDED
#define CIFF_INCLUDED

/* CIFF type and locacation masks and types                           */
#define CIFF_TYPEMASK   0x3fff
#define CIFF_FORMATMASK   0x3800
#define CIFF_LOCATIONMASK   0xc000
#define CIFF_INHEAP         0x0000
#define CIFF_INREC          0x4000

#define CIFFTAGWIDTH        23 
#define CIFF_MAXLEVELS      16 

#include "ciff_datadefs.h"
#include "cifftags.h"
#include "ciff_extern.h"

#endif /* CIFF_INCLUDED */
