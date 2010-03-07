/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2005 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* All tags defined in CIFF spec                                      */

#ifndef CIFFTAGS_INCLUDED
#define CIFFTAGS_INCLUDED

#define CIFFTAG_NULLRECORD                   0x0000
#define CIFFTAG_FREEBYTES                    0x0001
#define CIFFTAG_COLORINFO1                   0x0032
#define CIFFTAG_FILEDESCRIPTION              0x0805  /* 0x000d */
#define CIFFTAG_RAWMAKEMODEL                 0x080a  /* 0x0007 */
#define CIFFTAG_FIRMWAREVERSION              0x080b
#define CIFFTAG_COMPONENTVERSION             0x080c
#define CIFFTAG_ROMOPERATIONMODE             0x080d  /* 0x000? */
#define CIFFTAG_OWNERNAME                    0x0810  /* 0x0009 */
#define CIFFTAG_IMAGETYPE                    0x0815  /* 0x0006 */
#define CIFFTAG_ORIGINALFILENAME             0x0816
#define CIFFTAG_THUMBNAILFILENAME            0x0817

#define CIFFTAG_TARGETIMAGETYPE              0x100a
#define CIFFTAG_SHUTTERRELEASEMETHOD         0x1010
#define CIFFTAG_SHUTTERRELEASETIMING         0x1011
#define CIFFTAG_RELEASESETTING               0x1016
#define CIFFTAG_BASEISO                      0x101c
#define CIFFTAG_0X1028                       0x1028  /* 0x0003 */
#define CIFFTAG_FOCALLENGTH                  0x1029  /* 0x0002 */
#define CIFFTAG_SHOTINFO                     0x102a  /* 0x0004 */
#define CIFFTAG_COLORINFO2                   0x102c
#define CIFFTAG_CAMERASETTINGS               0x102d  /* 0x0001 */
#define CIFFTAG_SENSORINFO                   0x1031
#define CIFFTAG_CUSTOMFUNCTIONS              0x1033  /* 0x000f */
#define CIFFTAG_PICTUREINFO                  0x1038  /* 0x0012 */
#define CIFFTAG_WHITEBALANCETABLE            0x10a9  /* 0x00a9 */
#define CIFFTAG_COLORSPACE                   0x10b4  /* 0x00b4 */

#define CIFFTAG_IMAGESPEC                    0x1803
#define CIFFTAG_RECORDID                     0x1804
#define CIFFTAG_SELFTIMERTIME                0x1806
#define CIFFTAG_TARGETDISTANCESETTING        0x1807
#define CIFFTAG_SERIALNUMBER                 0x180b  /* 0x000c */
#define CIFFTAG_CAPTUREDTIME                 0x180e
#define CIFFTAG_IMAGEINFO                    0x1810
#define CIFFTAG_FLASHINFO                    0x1813
#define CIFFTAG_MEASUREDEV                   0x1814
#define CIFFTAG_FILENUMBER                   0x1817  /* 0x0008 */
#define CIFFTAG_EXPOSUREINFO                 0x1818
#define CIFFTAG_0X1834                       0x1834  /* 0x0010 */
#define CIFFTAG_DECODERTABLE                 0x1835

#define CIFFTAG_RAWIMAGEDATA                 0x2005
#define CIFFTAG_JPEGIMAGE                    0x2007
#define CIFFTAG_JPEGTHUMBNAIL                0x2008

#define CIFFTAG_IMAGEDESCRIPTION             0x2804
#define CIFFTAG_CAMERAOBJECT                 0x2807
#define CIFFTAG_SHOOTINGRECORD               0x3002
#define CIFFTAG_MEASUREDINFO                 0x3003
#define CIFFTAG_CAMERASPECIFICATION          0x3004
#define CIFFTAG_IMAGEPROPS                   0x300a
#define CIFFTAG_EXIFINFORMATION              0x300b

#endif /* CIFFTAGS_INCLUDED */
