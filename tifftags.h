/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: tifftags.h,v 1.2 2005/05/30 15:53:10 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* All tags defined in TIFF6 spec                                     */

#ifndef TIFFTAGS_INCLUDED
#define TIFFTAGS_INCLUDED

#define TIFFTAG_NewSubFileType                  0x00FE /* 254 */
#define TIFFTAG_OldSubFileType                  0x00FF /* 255 */
#define TIFFTAG_ImageWidth                      0x0100 /* 256 */
#define TIFFTAG_ImageLength                     0x0101 /* 257 */
#define TIFFTAG_BitsPerSample                   0x0102 /* 258 */
#define TIFFTAG_Compression                     0x0103 /* 259 */
#define TIFFTAG_PhotometricInterpretation       0x0106 /* 262 */
#define TIFFTAG_Thresholding                    0x0107 /* 263 */
#define TIFFTAG_CellWidth                       0x0108 /* 264 */
#define TIFFTAG_CellLength                      0x0109 /* 265 */
#define TIFFTAG_FillOrder                       0x010A /* 266 */
#define TIFFTAG_DocumentName                    0x010D /* 269 */
#define TIFFTAG_ImageDescription                0x010E /* 270 */
#define TIFFTAG_Make                            0x010F /* 271 */
#define TIFFTAG_Model                           0x0110 /* 272 */
#define TIFFTAG_StripOffsets                    0x0111 /* 273 */
#define TIFFTAG_Orientation                     0x0112 /* 274 */
#define TIFFTAG_SamplesPerPixel                 0x0115 /* 277 */
#define TIFFTAG_RowsPerStrip                    0x0116 /* 278 */
#define TIFFTAG_StripByteCounts                 0x0117 /* 279 */
#define TIFFTAG_MinSampleValue                  0x0118 /* 280 */
#define TIFFTAG_MaxSampleValue                  0x0119 /* 281 */
#define TIFFTAG_XResolution                     0x011A /* 282 */
#define TIFFTAG_YResolution                     0x011B /* 283 */
#define TIFFTAG_PlanarConfiguration             0x011C /* 284 */
#define TIFFTAG_PageName                        0x011D /* 285 */
#define TIFFTAG_XPosition                       0x011E /* 286 */
#define TIFFTAG_YPosition                       0x011F /* 287 */
#define TIFFTAG_FreeOffsets                     0x0120 /* 288 */
#define TIFFTAG_FreeByteCounts                  0x0121 /* 289 */
#define TIFFTAG_GrayResponseUnit                0x0122 /* 290 */
#define TIFFTAG_GrayResponseCurve               0x0123 /* 291 */
#define TIFFTAG_T4Options                       0x0124 /* 292 */
#define TIFFTAG_T6Options                       0x0125 /* 293 */
#define TIFFTAG_ResolutionUnit                  0x0128 /* 296 */
#define TIFFTAG_PageNumber                      0x0129 /* 297 */
#define TIFFTAG_TransferFunction                0x012D /* 301 */
#define TIFFTAG_Software                        0x0131 /* 305 */
#define TIFFTAG_DateTime                        0x0132 /* 306 */
#define TIFFTAG_Artist                          0x013B /* 315 */
#define TIFFTAG_HostComputer                    0x013C /* 316 */
#define TIFFTAG_Predictor                       0x013D /* 317 */
#define TIFFTAG_WhitePoint                      0x013E /* 318 */
#define TIFFTAG_PrimaryChromaticities           0x013F /* 319 */
#define TIFFTAG_ColorMap                        0x0140 /* 320 */
#define TIFFTAG_HalftoneHints                   0x0141 /* 321 */
#define TIFFTAG_TileWidth                       0x0142 /* 322 */
#define TIFFTAG_TileLength                      0x0143 /* 323 */
#define TIFFTAG_TileOffsets                     0x0144 /* 324 */
#define TIFFTAG_TileByteCounts                  0x0145 /* 325 */
#define TIFFTAG_SubIFDtag                       0x014A /* 330 */
#define TIFFTAG_InkSet                          0x014C /* 332 */
#define TIFFTAG_InkNames                        0x014D /* 333 */
#define TIFFTAG_NumberOfInks                    0x014E /* 334 */
#define TIFFTAG_DotRange                        0x0150 /* 336 */
#define TIFFTAG_TargetPrinter                   0x0151 /* 337 */
#define TIFFTAG_ExtraSamples                    0x0152 /* 338 */
#define TIFFTAG_SampleFOrmat                    0x0153 /* 339 */
#define TIFFTAG_SMinSampleValue                 0x0154 /* 340 */
#define TIFFTAG_SMaxSampleValue                 0x0155 /* 341 */
#define TIFFTAG_TransferRange                   0x0156 /* 342 */
#define TIFFTAG_JPEGTables                      0x015B /* 347 */
#define TIFFTAG_JpegProc                        0x0200 /* 512 */
#define TIFFTAG_JPEGInterchangeFormat           0x0201 /* 513 */
#define TIFFTAG_JPEGInterchangeFormatLength     0x0202 /* 514 */
#define TIFFTAG_JPEGRestartInterval             0x0203 /* 515 */
#define TIFFTAG_JPEGLosslessPredictor           0x0205 /* 517 */
#define TIFFTAG_JPEGPointTransforms             0x0206 /* 518 */
#define TIFFTAG_JPEGQTables                     0x0207 /* 519 */
#define TIFFTAG_JPEGDCTables                    0x0208 /* 520 */
#define TIFFTAG_JPEGACTables                    0x0209 /* 521 */
#define TIFFTAG_YCbCrCoefficients               0x0211 /* 529 */
#define TIFFTAG_YCbCrSubSampling                0x0212 /* 530 */
#define TIFFTAG_YcbCrPositioning                0x0213 /* 531 */
#define TIFFTAG_ReferenceBlackWhite             0x0214 /* 532 */
#define TIFFTAG_Copyright                       0x8298 /* 32432 */

#endif
