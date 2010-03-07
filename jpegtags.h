/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: jpegtags.h,v 1.3 2003/02/20 00:30:07 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Tags ("markers") defined in the JPEG (ITU) spec                    */

#ifndef JPEG_MARKERS_INCLUDED
#define JPEG_MARKERS_INCLUDED

#define JPEG_SOF_0  0xFFC0
#define JPEG_SOF_1  0xFFC1
#define JPEG_SOF_2  0xFFC2
#define JPEG_SOF_3  0xFFC3
#define JPEG_DHT    0xFFC4
#define JPEG_SOF_5  0xFFC5
#define JPEG_SOF_6  0xFFC6
#define JPEG_SOF_7  0xFFC7
#define JPEG_JPG    0xFFC8
#define JPEG_SOF_9  0xFFC9
#define JPEG_SOF_10 0xFFCA
#define JPEG_SOF_11 0xFFCB
#define JPEG_DAC    0xFFCC
#define JPEG_SOF_13 0xFFCD
#define JPEG_SOF_14 0xFFCE
#define JPEG_SOF_15 0xFFCF
#define JPEG_RST0   0xFFD0
#define JPEG_RST1   0xFFD1
#define JPEG_RST2   0xFFD2
#define JPEG_RST3   0xFFD3
#define JPEG_RST4   0xFFD4
#define JPEG_RST5   0xFFD5
#define JPEG_RST6   0xFFD6
#define JPEG_RST7   0xFFD7
#define JPEG_SOI    0xFFD8
#define JPEG_BADSOI 0x00D8  /* minolta                                */
#define JPEG_EOI    0xFFD9
#define JPEG_SOS    0xFFDA
#define JPEG_DQT    0xFFDB
#define JPEG_DNL    0xFFDC
#define JPEG_DRI    0xFFDD
#define JPEG_DHP    0xFFDE
#define JPEG_EXP    0xFFDF
#define JPEG_APP0   0xFFE0
#define JPEG_APP1   0xFFE1
#define JPEG_APP2   0xFFE2
#define JPEG_APP3   0xFFE3
#define JPEG_APP4   0xFFE4
#define JPEG_APP5   0xFFE5
#define JPEG_APP6   0xFFE6
#define JPEG_APP7   0xFFE7
#define JPEG_APP8   0xFFE8
#define JPEG_APP9   0xFFE9
#define JPEG_APP10  0xFFEA
#define JPEG_APP11  0xFFEB
#define JPEG_APP12  0xFFEC
#define JPEG_APP13  0xFFED
#define JPEG_APP14  0xFFEE
#define JPEG_APP15  0xFFEF
#define JPEG_JPG0   0xFFF0
#define JPEG_JPG1   0xFFF1
#define JPEG_JPG2   0xFFF2
#define JPEG_JPG3   0xFFF3
#define JPEG_JPG4   0xFFF4
#define JPEG_JPG5   0xFFF5
#define JPEG_JPG6   0xFFF6
#define JPEG_JPG7   0xFFF7
#define JPEG_JPG8   0xFFF8
#define JPEG_JPG9   0xFFF9
#define JPEG_JPG10  0xFFFA
#define JPEG_JPG11  0xFFFB
#define JPEG_JPG12  0xFFFC
#define JPEG_JPG13  0xFFFD
#define JPEG_COM    0xFFFE
#define JPEG_TEM    0xFF01
#define JPEG_RES    0xFF02

#endif
