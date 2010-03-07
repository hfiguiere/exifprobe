/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: tiffeptags.h,v 1.2 2003/01/30 20:47:31 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* All tags defined in TIFFEP spec not defined in TIFF6 or EXIF specs */

#ifndef TIFFEP_TAGS_INCLUDED
#define TIFFEP_TAGS_INCLUDED

#define TIFFEPTAG_CFARepeatPatternDim             0x828D /* 33421 */
#define TIFFEPTAG_CFAPattern                      0x828E /* 33422 */
#define TIFFEPTAG_BatteryLevel                    0x828F /* 33423 */
#define TIFFEPTAG_IPTC_NAA                        0x83BB /* 33723 */
#define TIFFEPTAG_InterColorProfile3              0x8773 /* 34675 */
#define TIFFEPTAG_Interlace                       0x8829 /* 34857 */
#define TIFFEPTAG_TimeZoneOffset                  0x882A /* 34858 */
#define TIFFEPTAG_SelfTimerMode                   0x882B /* 34859 */
#define TIFFEPTAG_FlashEnergy                     0x920B /* 37387 */
#define TIFFEPTAG_SpatialFrequencyResponse        0x920C /* 37388 */
#define TIFFEPTAG_Noise                           0x920D /* 37389 */
#define TIFFEPTAG_FocalPlaneXResolution           0x920E /* 37390 */
#define TIFFEPTAG_FocalPlaneYResolution           0x920F /* 37391 */
#define TIFFEPTAG_FocalPlaneResolutionUnit        0x9210 /* 37392 */
#define TIFFEPTAG_ImageNumber                     0x9211 /* 37393 */
#define TIFFEPTAG_SecurityClassification          0x9212 /* 37394 */
#define TIFFEPTAG_ImageHistory                    0x9213 /* 37395 */
#define TIFFEPTAG_ExposureIndex                   0x9215 /* 37397 */
#define TIFFEPTAG_TIFF_EPStandardID               0x9216 /* 37398 */
#define TIFFEPTAG_SensingMethod                   0x9217 /* 37399 */

#endif
