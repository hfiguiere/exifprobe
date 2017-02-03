/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: private_tags.h,v 1.8 2005/07/15 17:41:41 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* "private" TIFF tags assigned by Adobe to vendors                   */
/* Most of these are taken from libtiff/tiff.h, viz.                  */
/* A few are taken from GeoTIFF                                       */

#ifndef PRIVATE_TAGS_INCLUDED
#define PRIVATE_TAGS_INCLUDED


#define TIFFTAG_XML                         0x02bc /* 700   */
#define TIFFTAG_REFPTS                      0x80B9 /* 32953 */
#define TIFFTAG_REGIONTACKPOINT             0x80BA /* 32954 */
#define TIFFTAG_REGIONWARPCORNERS           0x80BB /* 32955 */
#define TIFFTAG_REGIONAFFINE                0x80BC /* 32956 */
#define TIFFTAG_MATTEING                    0x80E3 /* 32995 */
#define TIFFTAG_DATATYPE                    0x80E4 /* 32996 */
#define TIFFTAG_IMAGEDEPTH                  0x80E5 /* 32997 */
#define TIFFTAG_TILEDEPTH                   0x80E6 /* 32998 */
#define TIFFTAG_PIXAR_IMAGEFULLWIDTH        0x8214 /* 33300 */
#define TIFFTAG_PIXAR_IMAGEFULLLENGTH       0x8215 /* 33301 */
#define TIFFTAG_PIXAR_TEXTUREFORMAT         0x8216 /* 33302 */
#define TIFFTAG_PIXAR_WRAPMODES             0x8217 /* 33303 */
#define TIFFTAG_PIXAR_FOVCOT                0x8218 /* 33304 */
#define TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN  0x8219 /* 33305 */
#define TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA  0x821A /* 33306 */
#define TIFFTAG_WRITERSERIALNUMBER          0x827D /* 33405 */
#define TIFFTAG_GEOPIXELSCALE               0x830e /* 33550 */
#define TIFFTAG_RICHTIFFIPTC                0x83bb /* 33723 */
#define TIFFTAG_INTERGRAPH_MATRIX           0x8480 /* 33920 */
#define TIFFTAG_GEOTIEPOINTS                0x8482 /* 33922 */
#define TIFFTAG_IT8SITE                     0x84E0 /* 34016 */
#define TIFFTAG_IT8COLORSEQUENCE            0x84E1 /* 34017 */
#define TIFFTAG_IT8HEADER                   0x84E2 /* 34018 */
#define TIFFTAG_IT8RASTERPADDING            0x84E3 /* 34019 */
#define TIFFTAG_IT8BITSPERRUNLENGTH         0x84E4 /* 34020 */
#define TIFFTAG_IT8BITSPEREXTENDEDRUNLENGTH 0x84E5 /* 34021 */
#define TIFFTAG_IT8COLORTABLE               0x84E6 /* 34022 */
#define TIFFTAG_IT8IMAGECOLORINDICATOR      0x84E7 /* 34023 */
#define TIFFTAG_IT8BKGCOLORINDICATOR        0x84E8 /* 34024 */
#define TIFFTAG_IT8IMAGECOLORVALUE          0x84E9 /* 34025 */
#define TIFFTAG_IT8BKGCOLORVALUE            0x84EA /* 34026 */
#define TIFFTAG_IT8PIXELINTENSITYRANGE      0x84EB /* 34027 */
#define TIFFTAG_IT8TRANSPARENCYINDICATOR    0x84EC /* 34028 */
#define TIFFTAG_IT8COLORCHARACTERIZATION    0x84ED /* 34029 */
#define TIFFTAG_FRAMECOUNT                  0x85B8 /* 34232 */
#define TIFFTAG_JPL_CARTO_IFD               0x85D7 /* 34263 */
#define TIFFTAG_GEOTRANSMATRIX              0x85D8 /* 34264 */
#define TIFFTAG_ICCPROFILE                  0x8773 /* 34675 */
#define TIFFTAG_PHOTOSHOP                   0x8649 /* 34377 */
#define TIFFTAG_GEOKEYDIRECTORY             0x87AF /* 34735 */
#define TIFFTAG_GEODOUBLEPARAMS             0x87B0 /* 34736 */
#define TIFFTAG_GEOASCIIPARAMS              0x87B1 /* 34737 */
#define TIFFTAG_JBIGOPTIONS                 0x87BE /* 34750 */
#define TIFFTAG_FAXRECVPARAMS               0x885C /* 34908 */
#define TIFFTAG_FAXSUBADDRESS               0x885D /* 34909 */
#define TIFFTAG_FAXRECVTIME                 0x885E /* 34910 */
#define TIFFTAG_FEDEX_EDR                   0x8871 /* 34929 */
#define TIFFTAG_STONITS                     0x923F /* 37439 */

/* Kodak APP3 Meta */
#define TIFFTAG_META_0xc353                 0xC353 /* 50003 */

#define TIFFTAG_PrintIM                     0xC4A5 /* 50341 */

/* Canon CR2 files */
#define TIFFTAG_CR2_0xc5d8                  0xC5D8 /* 50648 */
#define TIFFTAG_CR2_0xc5d9                  0xC5D9 /* 50649 */
#define TIFFTAG_CR2_0xc5e0                  0xC5E0 /* 50656 */
#define TIFFTAG_CR2_SLICE                   0xC640 /* 50752 */
#define TIFFTAG_CR2_SRAWTYPE                0xC6C5 /* 50885 */

/* Panasonic RAW / RW2 */
#define TIFFTAG_RW2_PANASONICRAWVERSION     0x0001 /* 00001 */
#define TIFFTAG_RW2_SENSORWIDTH             0x0002 /* 00002 */
#define TIFFTAG_RW2_SENSORHEIGHT            0x0003 /* 00003 */
#define TIFFTAG_RW2_SENSORTOPBORDER         0x0004 /* 00004 */
#define TIFFTAG_RW2_SENSORLEFTBORDER        0x0005 /* 00005 */
#define TIFFTAG_RW2_IMAGEHEIGHT             0x0006 /* 00006 */
#define TIFFTAG_RW2_IMAGEWIDTH              0x0007 /* 00007 */
#define TIFFTAG_RW2_REDBALANCE              0x0011 /* 00017 */
#define TIFFTAG_RW2_BLUEBALANCE             0x0012 /* 00018 */
#define TIFFTAG_RW2_ISO                     0x0017 /* 00023 */
#define TIFFTAG_RW2_WBREDLEVEL              0x0024 /* 00036 */
#define TIFFTAG_RW2_WBGREENLEVEL            0x0025 /* 00037 */
#define TIFFTAG_RW2_WBBLUELEVEL             0x0026 /* 00038 */
#define TIFFTAG_RW2_JPGFROMRAW              0x002E /* 00047 */

#define TIFFTAG_DCSHUESHIFTVALUES           0xFFFF /* 65535 */
#endif

