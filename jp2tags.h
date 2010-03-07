/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2005 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: jp2tags.h,v 1.1 2005/05/24 19:08:57 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* tags defined in JP2 spec or "jasper"                               */

#ifndef JP2TAGS_INCLUDED
#define JP2TAGS_INCLUDED

#define	JP2C_SOC    0xff4f /* Start of code stream                    */
#define	JP2C_SIZ    0xff51 /* Image and tile size                     */
#define	JP2C_COD    0xff52 /* Coding style default                    */
#define JP2C_COC    0xff53 /* Coding style component                  */
#define	JP2C_TLM    0xff55 /* Tile-part lengths, main header          */
#define	JP2C_PLM    0xff57 /* Packet length, main header              */
#define	JP2C_PLT    0xff58 /* Packet length, tile-part header         */
#define	JP2C_RGN    0xff5e /* Region of interest                      */
#define JP2C_QCD    0xff5c /* Quantization default                    */
#define JP2C_QCC    0xff5d /* Quantization component                  */
#define JP2C_POC    0xff5f /* Progression order default               */
#define	JP2C_PPM    0xff60 /* Packed packet headers, main header      */
#define	JP2C_PPT    0xff61 /* Packet packet headers, tile-part header */
#define	JP2C_CRG    0xff63 /* Component registration                  */
#define JP2C_COM    0xff64 /* Comment                                 */
#define	JP2C_SOT    0xff90 /* Start of tile-part                      */
#define	JP2C_SOP    0xff91 /* Start of packet                         */
#define	JP2C_EPH    0xff92 /* End of packet header                    */
#define	JP2C_SOD    0xff93 /* Start of data                           */
#define	JP2C_EOC    0xffd9 /* End of code stream                      */

#endif /* JP2TAGS_INCLUDED */
