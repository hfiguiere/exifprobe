/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFFJPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: tagnames.c,v 1.21 2005/07/24 19:40:09 alex Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "ciff.h"
#include "dngtags.h"

/* This function attempts to print a "canonical" name for every tag   */
/* in the TIFF6, TIFF_EP, EXIF, DNG, and JPEG (10918-1)               */
/* specifications, without intervention of arbitrary "defines"...what */
/* you see is what you get.                                           */

/* The 'case' values are given in hex here, with the decimal values   */
/* in leading comments, and the specifications in which the tag       */
/* appears given in trailing comments. There are additonal private    */
/* tags (not yet listed here) which may be found in the "tiff.h" file */
/* of the "libtiff" distribution by Sam Leffler (Silicon Graphics);   */

char *
tagname(unsigned short tag)
{
    static char unknown_buf[16];
    char *tagname;

    switch(tag)
    {
/*     1 */ case  0x01: tagname = "PanasonicRawVersion"; break; /* RW2 */
/*     2 */ case  0x02: tagname = "SensorWidth"; break; /* RW2 */
/*     3 */ case  0x03: tagname = "SensorHeight"; break; /* RW2 */
/*     4 */ case  0x04: tagname = "SensorTopBorder"; break; /* RW2 */
/*     5 */ case  0x05: tagname = "SensorLeftBorder"; break; /* RW2 */
/*     6 */ case  0x06: tagname = "ImageHeight"; break; /* RW2 */
/*     7 */ case  0x07: tagname = "ImageWidth"; break; /* RW2 */
/*    17 */ case  0x11: tagname = "RedBalance"; break; /* RW2 */
/*    18 */ case  0x12: tagname = "BlueBalance"; break; /* RW2 */
/*    23 */ case  0x17: tagname = "ISO"; break; /* RW2 */
/*    36 */ case  0x24: tagname = "WBRedLevel"; break; /* RW2 */
/*    37 */ case  0x25: tagname = "WBGreenLevel"; break; /* RW2 */
/*    38 */ case  0x26: tagname = "WBBlueLevel"; break; /* RW2 */
/*    47 */ case  0x2E: tagname = "JpgFromRaw"; break; /* RW2 */

/*   254 */ case  0xFE: tagname = "NewSubFileType"; break; /* TIFF6, TIFFEP */
/*   255 */ case  0xFF: tagname = "SubFileType"; break; /* TIFF6 */
/*   256 */ case 0x100: tagname = "ImageWidth"; break;   /* TIFF6 EXIF TIFFEP */
/*   257 */ case 0x101: tagname = "ImageLength"; break;  /* TIFF6 EXIF TIFFEP */
/*   258 */ case 0x102: tagname = "BitsPerSample"; break;   /* TIFF6 EXIF TIFFEP */
/*   259 */ case 0x103: tagname = "Compression"; break; /* TIFF6 EXIF TIFFEP */
/*   260 */
/*   261 */
/*   262 */ case 0x106: tagname = "PhotometricInterpretation"; break;   /* TIFF6 EXIF TIFFEP */
/*   263 */ case 0x107: tagname = "Thresholding"; break;    /* TIFF6 */
/*   264 */ case 0x108: tagname = "CellWiddth"; break;  /* TIFF6 */
/*   265 */ case 0x109: tagname = "CellLength`"; break; /* TIFF6 */
/*   266 */ case 0x10A: tagname = "FillOrder"; break;   /* TIFF6 */
/*   267 */
/*   268 */ 
/*   269 */ case 0x10D: tagname = "DocumentName"; break;    /* TIFF6 */
/*   270 */ case 0x10E: tagname = "ImageDescription"; break;    /* TIFF6 EXIF TIFFEP */
/*   271 */ case 0x10F: tagname = "Make"; break;    /* TIFF6 EXIF TIFFEP */
/*   272 */ case 0x110: tagname = "Model"; break;   /* TIFF6 EXIF TIFFEP */
/*   273 */ case 0x111: tagname = "StripOffsets"; break;    /* TIFF6 EXIF TIFFEP */
/*   274 */ case 0x112: tagname = "Orientation"; break; /* TIFF6 EXIF TIFFEP */
/*   275 */
/*   276 */
/*   277 */ case 0x115: tagname = "SamplesPerPixel"; break; /* TIFF6 EXIF TIFFEP */
/*   278 */ case 0x116: tagname = "RowsPerStrip"; break;    /* TIFF6 EXIF TIFFEP */
/*   279 */ case 0x117: tagname = "StripByteCounts"; break; /* TIFF6 EXIF TIFFEP */
/*   280 */ case 0x118: tagname = "MinSampleValue"; break;  /* TIFF6 */
/*   281 */ case 0x119: tagname = "MaxSampleValue"; break;  /* TIFF6 */
/*   282 */ case 0x11A: tagname = "XResolution"; break; /* TIFF6 EXIF TIFFEP */
/*   283 */ case 0x11B: tagname = "YResolution"; break; /* TIFF6 EXIF TIFFEP */
/*   284 */ case 0x11C: tagname = "PlanarConfiguration"; break; /* TIFF6 EXIF TIFFEP */
/*   285 */ case 0x11D: tagname = "PageName"; break;    /* TIFF6 */
/*   286 */ case 0x11E: tagname = "XPosition"; break;   /* TIFF6 */
/*   287 */ case 0x11F: tagname = "YPosition"; break;   /* TIFF6 */
/*   288 */ case 0x120: tagname = "FreeOffsets"; break; /* TIFF6 */
/*   289 */ case 0x121: tagname = "FreeByteCounts"; break;  /* TIFF6 */
/*   290 */ case 0x122: tagname = "GrayResponseUnit"; break;    /* TIFF6 */
/*   291 */ case 0x123: tagname = "GrayResponseCurve"; break;   /* TIFF6 */
/*   292 */ case 0x124: tagname = "T4Options"; break;   /* TIFF6 */
/*   293 */ case 0x125: tagname = "T6Options"; break;   /* TIFF6 */
/*   294 */
/*   295 */
/*   296 */ case 0x128: tagname = "ResolutionUnit"; break;  /* TIFF6 EXIF TIFFEP */
/*   297 */ case 0x129: tagname = "PageNumber"; break;  /* TIFF6 */
/*   298 */
/*   299 */
/*   300 */ case 0x12C: tagname = "ColorResponseUnit"; break;   /* libtiff */
/*   301 */ case 0x12D: tagname = "TransferFunction"; break;    /* TIFF6 EXIF */
/*   302 */
/*   303 */
/*   304 */
/*   305 */ case 0x131: tagname = "Software"; break;    /* TIFF6 EXIF TIFFEP */
/*   306 */ case 0x132: tagname = "DateTime"; break;    /* TIFF6 EXIF TIFFEP */
/*   307 - 314 */
/*   315 */ case 0x13B: tagname = "Artist"; break;  /* TIFF6 EXIF TIFFEP */
/*   316 */ case 0x13C: tagname = "HostComputer"; break;    /* TIFF6 */
/*   317 */ case 0x13D: tagname = "Predictor"; break;   /* TIFF6 */
/*   318 */ case 0x13E: tagname = "WhitePoint"; break;  /* TIFF6 EXIF */
/*   319 */ case 0x13F: tagname = "PrimaryChromaticities"; break;   /* TIFF6 EXIF */
/*   320 */ case 0x140: tagname = "ColorMap"; break;    /* TIFF6 */
/*   321 */ case 0x141: tagname = "HalftoneHints"; break;   /* TIFF6 */
/*   322 */ case 0x142: tagname = "TileWidth"; break;   /* TIFFEP TIFF6 */
/*   323 */ case 0x143: tagname = "TileLength"; break;  /* TIFFEP TIFF6 */
/*   324 */ case 0x144: tagname = "TileOffsets"; break; /* TIFFEP TIFF6 */
/*   325 */ case 0x145: tagname = "TileByteCounts"; break;  /* TIFFEP TIFF6 */
/*   326 */ case 0x146: tagname = "BadFaxLines"; break; /* libtiff */
/*   327 */ case 0x147: tagname = "CleanFaxData"; break;    /* libtiff */
/*   328 */ case 0x148: tagname = "ConsecutiveBadFaxLines"; break;  /* libtiff */
/*   329 */
/*   330 */ case 0x14A: tagname = "SubIFDs"; break;   /* TIFFEP */
/*   331 */
/*   332 */ case 0x14C: tagname = "InkSet"; break;  /* TIFF6 */
/*   333 */ case 0x14D: tagname = "InkNames"; break;    /* TIFF6 */
/*   334 */ case 0x14E: tagname = "NumberOfInks"; break;    /* TIFF6 */
/*   335 */
/*   336 */ case 0x150: tagname = "DotRange"; break;    /* TIFF6 */
/*   337 */ case 0x151: tagname = "TargetPrinter"; break;   /* TIFF6 */
/*   338 */ case 0x152: tagname = "ExtraSamples"; break;    /* TIFF6 */
/*   339 */ case 0x153: tagname = "SampleFOrmat"; break;    /* TIFF6 */
/*   340 */ case 0x154: tagname = "SMinSampleValue"; break; /* TIFF6 */
/*   341 */ case 0x155: tagname = "SMaxSampleValue"; break; /* TIFF6 */
/*   342 */ case 0x156: tagname = "TransferRange"; break;   /* TIFF6              */
/*   343 - 346 */
/*   347 */ case 0x15B: tagname = "JPEGTables"; break;  /* TIFFEP */
/*   348 - 511 */
/*   512 */ case 0x200: tagname = "JpegProc"; break;    /* TIFF6              */
/*   513 */ case 0x201: tagname = "JPEGInterchangeFormat"; break; /* TIFF6 EXIF */
/*   514 */ case 0x202: tagname = "JPEGInterchangeFormatLength"; break; /* TIFF6 EXIF */
/*   515 */ case 0x203: tagname = "JPEGRestartInterval"; break; /* TIFF6 */
/*   516 */
/*   517 */ case 0x205: tagname = "JPEGLosslessPredictor"; break; /* TIFF6 */
/*   518 */ case 0x206: tagname = "JPEGPointTransforms"; break; /* TIFF6 */
/*   519 */ case 0x207: tagname = "JPEGQTables"; break; /* TIFF6 */
/*   520 */ case 0x208: tagname = "JPEGDCTables"; break; /* TIFF6 */
/*   521 */ case 0x209: tagname = "JPEGACTables"; break; /* TIFF6 */
/*   522 - 528 */
/*   529 */ case 0x211: tagname = "YCbCrCoefficients"; break;   /* TIFF6 TIFFEP */
/*   530 */ case 0x212: tagname = "YCbCrSubSampling"; break;    /* TIFF6 TIFFEP */
/*   531 */ case 0x213: tagname = "YCbCrPositioning"; break;    /* TIFF6 TIFFEP */
/*   532 */ case 0x214: tagname = "ReferenceBlackWhite"; break; /* TIFF6 TIFFEP */

/*   700 */ case 0x2bc: tagname = "XMLPacket"; break; /* Adobe, libtiff  */

/* 32953 */ case 0x80B9: tagname = "Refpts"; break; /* libtiff */
/* 32954 */ case 0x80BA: tagname = "Regiontackpoint"; break;    /* libtiff */
/* 32955 */ case 0x80BB: tagname = "Regionwarpcorners"; break;  /* libtiff */
/* 32956 */ case 0x80BC: tagname = "Regionaffine"; break;   /* libtiff */
/* 32995 */ case 0x80E3: tagname = "Matteing"; break;   /* libtiff */
/* 32996 */ case 0x80E4: tagname = "Datatype"; break;   /* libtiff */
/* 32997 */ case 0x80E5: tagname = "Imagedepth"; break; /* libtiff */
/* 32998 */ case 0x80E6: tagname = "Tiledepth"; break;  /* libtiff */
/* 33300 */ case 0x8214: tagname = "Pixar_imagefullwidth"; break;   /* libtiff */
/* 33301 */ case 0x8215: tagname = "Pixar_imagefulllength"; break;  /* libtiff */
/* 33302 */ case 0x8216: tagname = "Pixar_textureformat"; break;    /* libtiff */
/* 33303 */ case 0x8217: tagname = "Pixar_wrapmodes"; break;    /* libtiff */
/* 33304 */ case 0x8218: tagname = "Pixar_fovcot"; break;   /* libtiff */
/* 33305 */ case 0x8219: tagname = "Pixar_matrix_worldtoscreen"; break; /* libtiff */
/* 33306 */ case 0x821A: tagname = "Pixar_matrix_worldtocamera"; break; /* libtiff */
/* 33405 */ case 0x827D: tagname = "Writerserialnumber"; break; /* libtiff */

/* 33421 */ case 0x828D: tagname = "EPCFARepeatPatternDim"; break;  /* TIFFEP */
/* 33422 */ case 0x828E: tagname = "EPCFAPattern"; break;   /* TIFFEP */
/* 33423 */ case 0x828F: tagname = "BatteryLevel"; break;   /* TIFFEP */

/* 33432 */ case 0x8298: tagname = "Copyright"; break;  /* TIFF6 EXIF TIFFEP */

/* 33434 */ case 0x829A: tagname = "ExposureTime"; break;   /* EXIF TIFFEP */

/* 33437 */ case 0x829D: tagname = "FNumber"; break;    /* EXIF TIFFEP */

/* 33550 */ case 0x830E: tagname = "GeoPixelScale"; break;  /* libgeotiff */

/* 33723 */ case 0x83BB: tagname = "RichTiffIPTC"; break;   /* TIFFEP */

/* 33920 */ case 0x8480: tagname = "IntergraphMatrix"; break;   /* libgeotiff */
/* 33922 */ case 0x8482: tagname = "GeoTiePoints"; break;   /* libgeotiff */

/* 34016 */ case 0x84E0: tagname = "IT8site"; break;    /* libtiff */
/* 34017 */ case 0x84E1: tagname = "IT8colorsequence"; break;   /* libtiff */
/* 34018 */ case 0x84E2: tagname = "IT8header"; break;  /* libtiff */
/* 34019 */ case 0x84E3: tagname = "IT8rasterpadding"; break;   /* libtiff */
/* 34020 */ case 0x84E4: tagname = "IT8bitsperrunlength"; break;    /* libtiff */
/* 34021 */ case 0x84E5: tagname = "IT8bitsperextendedrunlength"; break;    /* libtiff */
/* 34022 */ case 0x84E6: tagname = "IT8colortable"; break;  /* libtiff */
/* 34023 */ case 0x84E7: tagname = "IT8imagecolorindicator"; break; /* libtiff */
/* 34024 */ case 0x84E8: tagname = "IT8bkgcolorindicator"; break;   /* libtiff */
/* 34025 */ case 0x84E9: tagname = "IT8imagecolorvalue"; break; /* libtiff */
/* 34026 */ case 0x84EA: tagname = "IT8bkgcolorvalue"; break;   /* libtiff */
/* 34027 */ case 0x84EB: tagname = "IT8pixelintensityrange"; break; /* libtiff */
/* 34028 */ case 0x84EC: tagname = "IT8transparencyindicator"; break;   /* libtiff */
/* 34029 */ case 0x84ED: tagname = "IT8colorcharacterization"; break;   /* libtiff */
/* 34232 */ case 0x85B8: tagname = "Framecount"; break; /* libtiff */

/* 34263 */ case 0x85D7: tagname = "JPLCartoIFD"; break;    /* libgeotiff */
/* 34264 */ case 0x85D8: tagname = "GeoTransMatrix"; break; /* libgeotiff */

/* 34377 */ case 0x8649: tagname = "PhotoShop"; break;  /* libtiff */
/* 34750 */ case 0x87BE: tagname = "JBIGoptions"; break;    /* libtiff */
/* 34908 */ case 0x885C: tagname = "Faxrecvparams"; break;  /* libtiff */
/* 34909 */ case 0x885D: tagname = "Faxsubaddress"; break;  /* libtiff */
/* 34910 */ case 0x885E: tagname = "Faxrecvtime"; break;    /* libtiff */
/* 34929 */ case 0x8871: tagname = "Fedex_edr"; break;  /* libtiff */
/* 37439 */ case 0x923F: tagname = "StoNits"; break;    /* libtiff */

/* 34665 */ case 0x8769: tagname = "ExifIFDPointer"; break;   /* EXIF */
/* 34675 */ case 0x8773: tagname = "InterColorProfile3"; break; /* TIFFEP */

/* 34735 */ case 0x87AF: tagname = "GeoKeyDirectory"; break;    /* libgeotiff */
/* 34736 */ case 0x87B0: tagname = "GeoDoubleParams"; break;    /* libgeotiff */
/* 34737 */ case 0x87B1: tagname = "GeoAsciiParams"; break; /* libgeotiff */

/* 34850 */ case 0x8822: tagname = "ExposureProgram"; break;    /* EXIF TIFFEP */
/* 34862 */ case 0x8824: tagname = "SpectralSensitivity"; break;    /* EXIF TIFFEP */
/* 34853 */ case 0x8825: tagname = "GPSInfoIFDPointer"; break;    /* EXIF TIFFEP */
/* 34855 */ case 0x8827: tagname = "ISOSpeedRatings"; break;    /* EXIF TIFFEP */
/* 34856 */ case 0x8828: tagname = "OECF"; break;   /* EXIF TIFFEP */
/* 34857 */ case 0x8829: tagname = "Interlace"; break;  /* TIFFEP */
/* 34864 */ case 0x8830: tagname = "SensitivityType"; break; /* Exiv2 */
/* 34858 */ case 0x882A: tagname = "TimeZoneOffset"; break; /* TIFFEP */
/* 34859 */ case 0x882B: tagname = "SelfTimerMode"; break;  /* TIFFEP */
/* 36864 */ case 0x9000: tagname = "Version"; break;    /* EXIF */
/* 36867 */ case 0x9003: tagname = "DateTimeOriginal"; break;   /* EXIF TIFFEP */
/* 36868 */ case 0x9004: tagname = "DateTimeDigitized"; break;  /* EXIF */
/* 37121 */ case 0x9101: tagname = "ComponentsConfiguration"; break;    /* EXIF */
/* 37122 */ case 0x9102: tagname = "CompressedBitsPerPixel"; break; /* EXIF TIFFEP */

/* 37377 */ case 0x9201: tagname = "ShutterSpeedValue"; break;  /* EXIF TIFFEP */
/* 37378 */ case 0x9202: tagname = "ApertureValue"; break;  /* EXIF TIFFEP */
/* 37379 */ case 0x9203: tagname = "BrightnessValue"; break;    /* EXIF TIFFEP */
/* 37380 */ case 0x9204: tagname = "ExposureBiasValue"; break;  /* EXIF TIFFEP */
/* 37381 */ case 0x9205: tagname = "MaxApertureValue"; break;   /* EXIF TIFFEP */
/* 37382 */ case 0x9206: tagname = "SubjectDistance"; break;    /* EXIF TIFFEP */
/* 37383 */ case 0x9207: tagname = "MeteringMode"; break;   /* EXIF TIFFEP */
/* 37384 */ case 0x9208: tagname = "LightSource"; break;    /* EXIF TIFFEP */
/* 37385 */ case 0x9209: tagname = "Flash"; break;  /* EXIF TIFFEP */
/* 37386 */ case 0x920A: tagname = "FocalLength"; break;    /* EXIF TIFFEP */
/* 37387 */ case 0x920B: tagname = "EPFlashEnergy"; break;    /* TIFFEP */
/* 37388 */ case 0x920C: tagname = "EPSpatialFrequencyResponse"; break;   /* TIFFEP */
/* 37389 */ case 0x920D: tagname = "Noise"; break;  /* TIFFEP */
/* 37390 */ case 0x920E: tagname = "EPFocalPlaneXResolution"; break;  /* TIFFEP */
/* 37391 */ case 0x920F: tagname = "EPFocalPlaneYResolution"; break;  /* TIFFEP */
/* 37392 */ case 0x9210: tagname = "EPFocalPlaneResolutionUnit"; break;   /* TIFFEP */
/* 37393 */ case 0x9211: tagname = "ImageNumber"; break;    /* TIFFEP */
/* 37394 */ case 0x9212: tagname = "SecurityClassification"; break; /* TIFFEP */
/* 37395 */ case 0x9213: tagname = "ImageHistory"; break;   /* TIFFEP */
/* 37396 */ case 0x9214: tagname = "SubjectArea"; break;    /* TIFFEP EXIF2.2 */
/* 37397 */ case 0x9215: tagname = "EPExposureIndex"; break;  /* TIFFEP */
/* 37398 */ case 0x9216: tagname = "TIFFEPStandardID"; break;  /* TIFFEP */
/* 37399 */ case 0x9217: tagname = "EPSensingMethod"; break;  /* TIFFEP */
/* 37500 */ case 0x927C: tagname = "MakerNote"; break;  /* EXIF */
/* 37510 */ case 0x9286: tagname = "UserComment"; break;    /* EXIF */
/* 37520 */ case 0x9290: tagname = "SubSecTime"; break; /* EXIF */
/* 37521 */ case 0x9291: tagname = "SubSecTimeOriginal"; break; /* EXIF */
/* 37522 */ case 0x9292: tagname = "SubSecTimeDigitized"; break;    /* EXIF */
/* 40960 */ case 0xA000: tagname = "FlashPixVersion"; break;    /* EXIF */
/* 40961 */ case 0xA001: tagname = "ColorSpace"; break; /* EXIF */
/* 40962 */ case 0xA002: tagname = "PixelXDimension"; break;    /* EXIF */
/* 40963 */ case 0xA003: tagname = "PixelYDimension"; break;    /* EXIF */
/* 40964 */ case 0xA004: tagname = "RelatedSoundFile"; break;   /* EXIF */
/* 40965 */ case 0xA005: tagname = "Interoperability"; break;   /* EXIF */
/* 41483 */ case 0xA20B: tagname = "FlashEnergy"; break;    /* EXIF */
/* 41484 */ case 0xA20C: tagname = "SpatialFrequencyResponse"; break;   /* EXIF */
/* 41486 */ case 0xA20E: tagname = "FocalPlaneXResolution"; break;  /* EXIF */
/* 41487 */ case 0xA20F: tagname = "FocalPlaneYResolution"; break;  /* EXIF */
/* 41488 */ case 0xA210: tagname = "FocalPlaneResolutionUnit"; break;   /* EXIF */
/* 41492 */ case 0xA214: tagname = "SubjectLocation"; break;    /* EXIF */
/* 41493 */ case 0xA215: tagname = "ExposureIndex"; break;  /* EXIF */
/* 41495 */ case 0xA217: tagname = "SensingMethod"; break;  /* EXIF */
/* 41728 */ case 0xA300: tagname = "FileSource"; break; /* EXIF */
/* 41729 */ case 0xA301: tagname = "SceneType"; break;  /* EXIF */
/* 41730 */ case 0xA302: tagname = "CFAPattern"; break; /* EXIF */
/* 41985 */ case 0xA401: tagname = "CustomRendered"; break; /* EXIF */
/* 41986 */ case 0xA402: tagname = "ExposureMode"; break;   /* EXIF */
/* 41987 */ case 0xA403: tagname = "WhiteBalance"; break;   /* EXIF */
/* 41988 */ case 0xA404: tagname = "DigitalZoomRatio"; break;   /* EXIF */
/* 41989 */ case 0xA405: tagname = "FocalLengthIn35mmFilm"; break;  /* EXIF */
/* 41990 */ case 0xA406: tagname = "SceneCaptureType"; break;   /* EXIF */
/* 41991 */ case 0xA407: tagname = "GainControl"; break;    /* EXIF */
/* 41992 */ case 0xA408: tagname = "Contrast"; break;   /* EXIF */
/* 41993 */ case 0xA409: tagname = "Saturation"; break; /* EXIF */
/* 41994 */ case 0xA40A: tagname = "Sharpness"; break;  /* EXIF */
/* 41995 */ case 0xA40B: tagname = "DeviceSettingDescription"; break;   /* EXIF */
/* 41996 */ case 0xA40C: tagname = "SubjectDistanceRange"; break;   /* EXIF */
/* 42016 */ case 0xA420: tagname = "ImageUniqueId"; break;  /* EXIF */

/* 42034 */ case 0xA432: tagname = "LensSpecification"; break; /* Fuji? Exiv2 */
/* 42035 */ case 0xA433: tagname = "LensMake"; break; /* Fuji? Exiv2 */
/* 42036 */ case 0xA434: tagname = "LensModel"; break; /* Fuji? Exiv2 */
/* 42037 */ case 0xA435: tagname = "LensSerial"; break; /* Fuji? Exiv2 */

/* 42240 */ case 0xA500: tagname = "Gamma"; break;  /* EXIF 221 */

/* 50003 */ case 0xC353: tagname = "META_Model"; break; /* Kodak APP3 Meta */
/* 50341 */ case 0xC4A5: tagname = "PrintIM"; break; /* Epson PIM */

/* Don't know what these are; appear in Canon CR2 files               */
/* 50648 */ case 0xC5D8: tagname = "TAG_CR2c5d8"; break; /* Canon CR2 */
/* 50649 */ case 0xC5D9: tagname = "TAG_CR2c5d9"; break; /* Canon CR2 */
/* 50656 */ case 0xC5E0: tagname = "TAG_CR2c5e0"; break; /* Canon CR2 */
/* 50752 */ case 0xC640: tagname = "CR2Slice"; break; /* Canon CR2 */

/* 50706 */ case 0xC612: tagname = "DNGVersion"; break;                 /* DNG */
/* 50707 */ case 0xC613: tagname = "DNGBackwardVersion"; break;         /* DNG */
/* 50708 */ case 0xC614: tagname = "UniqueCameraModel"; break;          /* DNG */
/* 50709 */ case 0xC615: tagname = "LocalizedCameraModel"; break;       /* DNG */
/* 50710 */ case 0xC616: tagname = "CFAPlaneColor"; break;              /* DNG */
/* 50711 */ case 0xC617: tagname = "CFALayout"; break;                  /* DNG */
/* 50712 */ case 0xC618: tagname = "LinearizationTable"; break;         /* DNG */
/* 50713 */ case 0xC619: tagname = "BlackLevelRepeatDim"; break;        /* DNG */
/* 50714 */ case 0xC61A: tagname = "BlackLevel"; break;                 /* DNG */
/* 50715 */ case 0xC61B: tagname = "BlackLevelDeltaH"; break;           /* DNG */
/* 50716 */ case 0xC61C: tagname = "BlackLevelDeltaV"; break;           /* DNG */
/* 50717 */ case 0xC61D: tagname = "WhiteLevel"; break;                 /* DNG */
/* 50718 */ case 0xC61E: tagname = "DefaultScale"; break;               /* DNG */
/* 50719 */ case 0xC61F: tagname = "DefaultCropOrigin"; break;          /* DNG */
/* 50720 */ case 0xC620: tagname = "DefaultCropSize"; break;            /* DNG */
/* 50721 */ case 0xC621: tagname = "ColorMatrix1"; break;               /* DNG */
/* 50722 */ case 0xC622: tagname = "ColorMatrix2"; break;               /* DNG */
/* 50723 */ case 0xC623: tagname = "CameraCalibration1"; break;         /* DNG */
/* 50724 */ case 0xC624: tagname = "CameraCalibration2"; break;         /* DNG */
/* 50725 */ case 0xC625: tagname = "ReductionMatrix1"; break;           /* DNG */
/* 50726 */ case 0xC626: tagname = "ReductionMatrix2"; break;           /* DNG */
/* 50727 */ case 0xC627: tagname = "AnalogBalance"; break;              /* DNG */
/* 50728 */ case 0xC628: tagname = "AsShotNeutral"; break;              /* DNG */
/* 50729 */ case 0xC629: tagname = "AsShotWhiteXY"; break;              /* DNG */
/* 50730 */ case 0xC62A: tagname = "BaselineExposure"; break;           /* DNG */
/* 50731 */ case 0xC62B: tagname = "BaselineNoise"; break;              /* DNG */
/* 50732 */ case 0xC62C: tagname = "BaselineSharpness"; break;          /* DNG */
/* 50733 */ case 0xC62D: tagname = "BayerGreenSplit"; break;            /* DNG */
/* 50734 */ case 0xC62E: tagname = "LinearResponseLimit"; break;        /* DNG */
/* 50735 */ case 0xC62F: tagname = "CameraSerialNumber"; break;         /* DNG */
/* 50736 */ case 0xC630: tagname = "LensInfo"; break;                   /* DNG */
/* 50737 */ case 0xC631: tagname = "ChromaBlurRadius"; break;           /* DNG */
/* 50738 */ case 0xC632: tagname = "AntiAliasStrength"; break;          /* DNG */
/* 50739 */ case 0xC633: tagname = "ShadowScale"; break;                /* DNG */
/* 50740 */ case 0xC634: tagname = "DNGPrivateData"; break;             /* DNG */
/* 50741 */ case 0xC635: tagname = "MakerNoteSafety"; break;            /* DNG */
/* 50778 */ case 0xC65A: tagname = "CalibrationIlluminant1"; break;     /* DNG */
/* 50779 */ case 0xC65B: tagname = "CalibrationIlluminant2"; break;     /* DNG */
/* 50780 */ case 0xC65C: tagname = "BestQualityScale"; break;           /* DNG */
/* 50781 */ case 0xC65D: tagname = "RawDataUniqueID"; break;            /* DNG */

/* 50827 */ case 0xC68B: tagname = "OriginalRawFileName"; break;        /* DNG */
/* 50828 */ case 0xC68C: tagname = "OriginalRawFileData"; break;        /* DNG */
/* 50829 */ case 0xC68D: tagname = "ActiveArea"; break;                 /* DNG */
/* 50830 */ case 0xC68E: tagname = "MaskedAreas"; break;                /* DNG */
/* 50831 */ case 0xC68F: tagname = "AsShotICCProfile"; break;           /* DNG */
/* 50832 */ case 0xC690: tagname = "AsShotPreProfileMatrix"; break;     /* DNG */
/* 50833 */ case 0xC691: tagname = "CurrentICCProfile"; break;          /* DNG */
/* 50834 */ case 0xC692: tagname = "CurrentPreProfileMatrix"; break;    /* DNG */

/* 50879 */ case 0xC6BF: tagname = "ColorimetricReference"; break;      /* DNG 1.2 */

/* 50931 */ case 0xC6F3: tagname = "CameraCalibrationSignature"; break; /* DNG 1.2 */
/* 50932 */ case 0xC6F4: tagname = "ProfileCalibrationSignature"; break;/* DNG 1.2 */
/* 50933 */ case 0xC6F5: tagname = "ExtraCameraProfiles"; break;        /* DNG 1.2 */
/* 50934 */ case 0xC6F6: tagname = "AsShotProfileName"; break;          /* DNG 1.2 */
/* 50935 */ case 0xC6F7: tagname = "NoiseReductionApplied"; break;      /* DNG 1.2 */
/* 50936 */ case 0xC6F8: tagname = "ProfileName"; break;                /* DNG 1.2 */
/* 50937 */ case 0xC6F9: tagname = "ProfileHueSatMapDims"; break;       /* DNG 1.2 */
/* 50938 */ case 0xC6FA: tagname = "ProfileHueSatMapData1"; break;      /* DNG 1.2 */
/* 50939 */ case 0xC6FB: tagname = "ProfileHueSatMapData2"; break;      /* DNG 1.2 */
/* 50940 */ case 0xC6FC: tagname = "ProfileToneCurve"; break;           /* DNG 1.2 */
/* 50941 */ case 0xC6FD: tagname = "ProfileEmbedPolicy"; break;         /* DNG 1.2 */
/* 50942 */ case 0xC6FE: tagname = "ProfileCopyright"; break;           /* DNG 1.2 */

/* 50964 */ case 0xC714: tagname = "ForwardMatrix1"; break;             /* DNG 1.2 */
/* 50965 */ case 0xC715: tagname = "ForwardMatrix2"; break;             /* DNG 1.2 */
/* 50966 */ case 0xC716: tagname = "PreviewApplicationName"; break;     /* DNG 1.2 */
/* 50967 */ case 0xC717: tagname = "PreviewApplicationVersion"; break;  /* DNG 1.2 */
/* 50968 */ case 0xC718: tagname = "PreviewSettingsName"; break;        /* DNG 1.2 */
/* 50969 */ case 0xC719: tagname = "PreviewSettingsDigest"; break;      /* DNG 1.2 */
/* 50970 */ case 0xC71A: tagname = "PreviewColorSpace"; break;          /* DNG 1.2 */
/* 50971 */ case 0xC71B: tagname = "PreviewDateTime"; break;            /* DNG 1.2 */
/* 50972 */ case 0xC71C: tagname = "RawImageDigest"; break;             /* DNG 1.2 */
/* 50973 */ case 0xC71D: tagname = "OriginalRawFileDigest"; break;      /* DNG 1.2 */
/* 50974 */ case 0xC71E: tagname = "SubTileBlockSize"; break;           /* DNG 1.2 */
/* 50975 */ case 0xC71F: tagname = "RowInterleaveFactor"; break;        /* DNG 1.2 */

/* 50981 */ case 0xC725: tagname = "ProfileLookTableDims"; break;       /* DNG 1.2 */
/* 50982 */ case 0xC726: tagname = "ProfileLookTableData"; break;       /* DNG 1.2 */

/* 51008 */ case 0xC740: tagname = "OpcodeList1"; break;                /* DNG 1.3 */
/* 51009 */ case 0xC741: tagname = "OpcodeList2"; break;                /* DNG 1.3 */

/* 51022 */ case 0xC74E: tagname = "OpcodeList3"; break;                /* DNG 1.3 */

/* 51041 */ case 0xC761: tagname = "NoiseProfile"; break;               /* DNG 1.3 */

/* 51043 */ case DNGTAG_TimeCodes: tagname = "TimeCodes"; break;        /* Cinama DNG */
/* 51044 */ case DNGTAG_FrameRate: tagname = "FrameRate"; break;        /* Cinema DNG */
/* 51058 */ case DNGTAG_TStop: tagname = "TStop"; break;                /* Cinema DNG */
/* 51081 */ case DNGTAG_ReelName: tagname = "ReelName"; break;          /* Cinema DNG */
/* 51105 */ case DNGTAG_CameraLabel: tagname = "CameraLabel"; break;    /* Cinema DNG */

/* 51089 */ case DNGTAG_OriginalDefaultFinalSize: tagname = "OriginalDefaultFinalSize"; break; /* DNG 1.4.0 */
/* 51090 */ case DNGTAG_OriginalBestQualityFinalSize: tagname = "OriginalBestQualityFinalSize"; break; /* DNG 1.4.0 */
/* 51091 */ case DNGTAG_OriginalDefaultCropSize: tagname = "OriginalDefaultCropSize"; break; /* DNG 1.4.0 */
/* 51107 */ case DNGTAG_ProfileHueSatMapEncoding: tagname = "ProfileHueSatMapEncoding"; break; /* DNG 1.4.0 */
/* 51108 */ case DNGTAG_ProfileLookTableEncoding: tagname = "ProfileLookTableEncoding"; break; /* DNG 1.4.0 */
/* 51109 */ case DNGTAG_BaselineExposureOffset: tagname = "BaselineExposureOffset"; break; /* DNG 1.4.0 */
/* 51110 */ case DNGTAG_DefaultBlackRender: tagname = "DefaultBlackRender"; break; /* DNG 1.4.0 */
/* 51111 */ case DNGTAG_NewRawImageDigest: tagname = "NewRawImageDigest"; break; /* DNG 1.4.0 */
/* 51112 */ case DNGTAG_RawToPreviewGain: tagname = "RawToPreviewGain"; break; /* DNG 1.4.0 */
/* 51125 */ case DNGTAG_DefaultUserCrop: tagname = "DefaultUserCrop"; break; /* DNG 1.4.0 */


/* %%%%% */ case 0xffc0: tagname = "JPEG_SOF_0"; break; /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffc1: tagname = "JPEG_SOF_1"; break; /* JPEG */
/* %%%%% */ case 0xffc2: tagname = "JPEG_SOF_2"; break; /* JPEG */
/* %%%%% */ case 0xffc3: tagname = "JPEG_SOF_3"; break; /* JPEG */
/* %%%%% */ case 0xffc4: tagname = "JPEG_DHT"; break;   /* JPEG */ /* EXIF */
/* %%%%% */ case 0xffc5: tagname = "JPEG_SOF_5"; break; /* JPEG */
/* %%%%% */ case 0xffc6: tagname = "JPEG_SOF_6"; break; /* JPEG */
/* %%%%% */ case 0xffc7: tagname = "JPEG_SOF_7"; break; /* JPEG */
/* %%%%% */ case 0xffc8: tagname = "JPEG_JPG"; break;   /* JPEG */
/* %%%%% */ case 0xffc9: tagname = "JPEG_SOF_9"; break; /* JPEG */
/* %%%%% */ case 0xffca: tagname = "JPEG_SOF_10"; break;    /* JPEG */
/* %%%%% */ case 0xffcb: tagname = "JPEG_SOF_11"; break;    /* JPEG */
/* %%%%% */ case 0xffcd: tagname = "JPEG_SOF_13"; break;    /* JPEG */
/* %%%%% */ case 0xffce: tagname = "JPEG_SOF_14"; break;    /* JPEG */
/* %%%%% */ case 0xffcf: tagname = "JPEG_SOF_15"; break;    /* JPEG */
/* %%%%% */ case 0xffcc: tagname = "JPEG_DAC"; break;   /* JPEG */
/* %%%%% */ case 0xffd0: tagname = "JPEG_RST0"; break;  /* JPEG */
/* %%%%% */ case 0xffd1: tagname = "JPEG_RST1"; break;  /* JPEG */
/* %%%%% */ case 0xffd2: tagname = "JPEG_RST2"; break;  /* JPEG */
/* %%%%% */ case 0xffd3: tagname = "JPEG_RST3"; break;  /* JPEG */
/* %%%%% */ case 0xffd4: tagname = "JPEG_RST4"; break;  /* JPEG */
/* %%%%% */ case 0xffd5: tagname = "JPEG_RST5"; break;  /* JPEG */
/* %%%%% */ case 0xffd6: tagname = "JPEG_RST6"; break;  /* JPEG */
/* %%%%% */ case 0xffd7: tagname = "JPEG_RST7"; break;  /* JPEG */
/* %%%%% */ case 0xffd8: tagname = "JPEG_SOI"; break;   /* JPEG */  /* EXIF */
/* ????? */ case 0x00d8: tagname = "JPEG_BADSOI"; break; /* Minolta MakerNote thumbnails */
/* %%%%% */ case 0xffd9: tagname = "JPEG_EOI"; break;   /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffda: tagname = "JPEG_SOS"; break;   /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffdb: tagname = "JPEG_DQT"; break;   /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffdc: tagname = "JPEG_DNL"; break;   /* JPEG */
/* %%%%% */ case 0xffdd: tagname = "JPEG_DRI"; break;   /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffde: tagname = "JPEG_DHP"; break;   /* JPEG */
/* %%%%% */ case 0xffdf: tagname = "JPEG_EXP"; break;   /* JPEG */
/* %%%%% */ case 0xffe0: tagname = "JPEG_APP0"; break;  /* JPEG */
/* %%%%% */ case 0xffe1: tagname = "JPEG_APP1"; break;  /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffe2: tagname = "JPEG_APP2"; break;  /* JPEG */  /* EXIF */
/* %%%%% */ case 0xffe3: tagname = "JPEG_APP3"; break;  /* JPEG */
/* %%%%% */ case 0xffe4: tagname = "JPEG_APP4"; break;  /* JPEG */
/* %%%%% */ case 0xffe5: tagname = "JPEG_APP5"; break;  /* JPEG */
/* %%%%% */ case 0xffe6: tagname = "JPEG_APP6"; break;  /* JPEG */
/* %%%%% */ case 0xffe7: tagname = "JPEG_APP7"; break;  /* JPEG */
/* %%%%% */ case 0xffe8: tagname = "JPEG_APP8"; break;  /* JPEG */
/* %%%%% */ case 0xffe9: tagname = "JPEG_APP9"; break;  /* JPEG */
/* %%%%% */ case 0xffea: tagname = "JPEG_APP10"; break; /* JPEG */
/* %%%%% */ case 0xffeb: tagname = "JPEG_APP11"; break; /* JPEG */
/* %%%%% */ case 0xffec: tagname = "JPEG_APP12"; break; /* JPEG */
/* %%%%% */ case 0xffed: tagname = "JPEG_APP13"; break; /* JPEG */
/* %%%%% */ case 0xffee: tagname = "JPEG_APP14"; break; /* JPEG */
/* %%%%% */ case 0xffef: tagname = "JPEG_APP15"; break; /* JPEG */
/* %%%%% */ case 0xfff0: tagname = "JPEG_JPG0"; break;  /* JPEG */
/* %%%%% */ case 0xfff1: tagname = "JPEG_JPG1"; break;  /* JPEG */
/* %%%%% */ case 0xfff2: tagname = "JPEG_JPG2"; break;  /* JPEG */
/* %%%%% */ case 0xfff3: tagname = "JPEG_JPG3"; break;  /* JPEG */
/* %%%%% */ case 0xfff4: tagname = "JPEG_JPG4"; break;  /* JPEG */
/* %%%%% */ case 0xfff5: tagname = "JPEG_JPG5"; break;  /* JPEG */
/* %%%%% */ case 0xfff6: tagname = "JPEG_JPG6"; break;  /* JPEG */
/* %%%%% */ case 0xfff7: tagname = "JPEG_JPG7"; break;  /* JPEG */
/* %%%%% */ case 0xfff8: tagname = "JPEG_JPG8"; break;  /* JPEG */
/* %%%%% */ case 0xfff9: tagname = "JPEG_JPG9"; break;  /* JPEG */
/* %%%%% */ case 0xfffa: tagname = "JPEG_JPG10"; break; /* JPEG */
/* %%%%% */ case 0xfffb: tagname = "JPEG_JPG11"; break; /* JPEG */
/* %%%%% */ case 0xfffc: tagname = "JPEG_JPG12"; break; /* JPEG */
/* %%%%% */ case 0xfffd: tagname = "JPEG_JPG13"; break; /* JPEG */
/* %%%%% */ case 0xfffe: tagname = "JPEG_COM"; break;   /* JPEG */
/* %%%%% */ case 0xff01: tagname = "JPEG_TEM"; break;   /* JPEG */
/* %%%%% */ case 0xff02: tagname = "JPEG_RES"; break;   /* JPEG */

    default:
        if(snprintf(unknown_buf,11,"TAG_%#06x",(int)tag) > 11)
        {
            printf(" bad tag %#x not converted\n",tag);
            why(stdout);
            tagname = "<BADTAG>";
        }
        else
            tagname = unknown_buf;
        break;
    }
    return(tagname);
}

char *
interop_tagname(unsigned short tag)
{
    static char unknown_buf[16];
    char *tagname;

    switch(tag)
    {
        case 0x0001: tagname = "InteroperabilityIndex"; break;   /* EXIF */
        case 0x0002: tagname = "InteroperabilityVersion"; break; /* EXIF */
        case 0x1000: tagname = "RelatedImageFileFormat"; break;  /* EXIF */
        case 0x1001: tagname = "RelatedImageWidth"; break;       /* EXIF */
        case 0x1002: tagname = "RelatedImageLength"; break;      /* EXIF */
        default:
            if(snprintf(unknown_buf,11,"IOP_%#06x",(int)tag) > 11)
            {
                printf(" bad tag %#x not converted\n",tag);
                why(stdout);
                tagname = "<BADTAG>";
            }
            else
                tagname = unknown_buf;
            break;
    }
    return(tagname);
}


char *
gps_tagname(unsigned short tag)
{
    static char unknown_buf[16];
    char *tagname;

    switch(tag)
    {
        case 0x0000: tagname = "VersionID"; break;   /* EXIF, TIFFEP */
        case 0x0001: tagname = "LatitudeRef"; break; /* EXIF, TIFFEP */
        case 0x0002: tagname = "Latitude"; break;  /* EXIF, TIFFEP */
        case 0x0003: tagname = "LongitudeRef"; break;       /* EXIF, TIFFEP */
        case 0x0004: tagname = "Longitude"; break;      /* EXIF, TIFFEP */
        case 0x0005: tagname = "AltitudeRef"; break;      /* EXIF, TIFFEP */
        case 0x0006: tagname = "Altitude"; break;      /* EXIF, TIFFEP */
        case 0x0007: tagname = "TimeStamp"; break;      /* EXIF, TIFFEP */
        case 0x0008: tagname = "Satellites"; break;      /* EXIF, TIFFEP */
        case 0x0009: tagname = "Status"; break;      /* EXIF, TIFFEP */
        case 0x000A: tagname = "MeasureMode"; break;      /* EXIF, TIFFEP */
        case 0x000B: tagname = "DOP"; break;      /* EXIF, TIFFEP */
        case 0x000C: tagname = "SpeedRef"; break;      /* EXIF, TIFFEP */
        case 0x000D: tagname = "Speed"; break;      /* EXIF, TIFFEP */
        case 0x000E: tagname = "TrackRef"; break;      /* EXIF, TIFFEP */
        case 0x000F: tagname = "Track"; break;      /* EXIF, TIFFEP */
        case 0x0010: tagname = "DirectionRef"; break;      /* EXIF, TIFFEP */
        case 0x0011: tagname = "Direction"; break;      /* EXIF, TIFFEP */
        case 0x0012: tagname = "MapDatum"; break;      /* EXIF, TIFFEP */
        case 0x0013: tagname = "DestlatitudeRef"; break;      /* EXIF, TIFFEP */
        case 0x0014: tagname = "Destlatitude"; break;      /* EXIF, TIFFEP */
        case 0x0015: tagname = "DestLongitudeRef"; break;      /* EXIF, TIFFEP */
        case 0x0016: tagname = "DestLongitude"; break;      /* EXIF, TIFFEP */
        case 0x0017: tagname = "BearingRef"; break;      /* EXIF, TIFFEP */
        case 0x0018: tagname = "Bearing"; break;      /* EXIF, TIFFEP */
        case 0x0019: tagname = "DestDistanceRef"; break;      /* EXIF, TIFFEP */
        case 0x001A: tagname = "DestDistance"; break;      /* EXIF, TIFFEP */
        case 0x001B: tagname = "ProcessingMethod"; break;      /* EXIF */
        case 0x001C: tagname = "AreaInformation"; break;      /* EXIF */
        case 0x001D: tagname = "DateStamp"; break;      /* EXIF */
        case 0x001E: tagname = "Differential"; break;      /* EXIF */
        default:
            if(snprintf(unknown_buf,11,"GPS_%#06x",(int)tag) > 11)
            {
                printf(" bad tag %#x not converted\n",tag);
                why(stdout);
                tagname = "<BADTAG>";
            }
            else
                tagname = unknown_buf;
            break;
    }
    return(tagname);
}

char *
cifftagname(unsigned short tag)
{
    static char unknown_buf[16];
    char *tagname;

    switch(tag)
    {
        case 0x0000: tagname = "NullRecord"; break;
        case 0x0001: tagname = "FreeBytes"; break;
        case 0x0032: tagname = "ColorInfo1"; break;
        case 0x0805: tagname = "FileDescription"; break;
        case 0x080a: tagname = "RawMakeModel"; break;
        case 0x080b: tagname = "FirmwareVersion"; break;
        case 0x080c: tagname = "ComponentVersion"; break;
        case 0x080d: tagname = "RomOperationMode"; break;
        case 0x0810: tagname = "OwnerName"; break;
        case 0x0815: tagname = "ImageType"; break;
        case 0x0816: tagname = "OriginalFilename"; break;
        case 0x0817: tagname = "ThumbnailFilename"; break;
        case 0x100a: tagname = "TargetImageType"; break;
        case 0x1010: tagname = "ShutterReleaseMethod"; break;
        case 0x1011: tagname = "ShutterReleaseTiming"; break;
        case 0x1016: tagname = "ReleaseSetting"; break;
        case 0x101c: tagname = "BaseISO"; break;
        /* case 0x1028: tagname = "TAG_0x1028"; break;                */
        case 0x1029: tagname = "FocalLengthInfo"; break;
        case 0x102a: tagname = "ShotInfo"; break;
        case 0x102c: tagname = "ColorInfo2"; break;
        case 0x102d: tagname = "CameraSettings"; break;
        case 0x1031: tagname = "SensorInfo"; break;
        case 0x1033: tagname = "CustomFunctions"; break;
        case 0x1038: tagname = "PictureInfo"; break;
        /* case 0x1039: tagname = "TAG_0x1039"; break;                */
        /* case 0x1093: tagname = "TAG_0x1093"; break;                */
        /* case 0x10a8: tagname = "TAG_0x10a8"; break;                */
        case 0x10a9: tagname = "WhiteBalanceTable"; break;
        /* case 0x10aa: tagname = "TAG_0x10aa"; break;                */
        /* case 0x10ae: tagname = "TAG_0x10ae"; break;                */
        case 0x10b4: tagname = "ColorSpace"; break;
        /* case 0x10b5: tagname = "TAG_0x10b5"; break;                */
        /* case 0x10c0: tagname = "0X10c0"; break;                    */
        /* case 0x10c1: tagname = "0X10c1"; break;                    */
        case 0x1803: tagname = "ImageSpec"; break;
        case 0x1804: tagname = "RecordId"; break;
        case 0x1806: tagname = "SelfTimerTime"; break;
        case 0x1807: tagname = "TargetDistanceSetting"; break;
        case 0x180b: tagname = "SerialNumber"; break;
        case 0x180e: tagname = "CapturedTime"; break;
        case 0x1810: tagname = "ImageInfo"; break;
        case 0x1813: tagname = "FlashInfo"; break;
        case 0x1814: tagname = "MeasuredEv"; break;
        case 0x1817: tagname = "Filenumber"; break;
        case 0x1818: tagname = "ExposureInfo"; break;
        /* case 0x1834: tagname = "TAG_0x1834"; break;                */
        case 0x1835: tagname = "DecoderTable"; break;
        /* case 0x183b: tagname = "TAG_0X183b"; break;                */
        case 0x2005: tagname = "RawImageData"; break;
        case 0x2007: tagname = "JpegImage"; break;
        case 0x2008: tagname = "JpegThumbnail"; break;
        case 0x2804: tagname = "ImageDescription"; break;
        case 0x2807: tagname = "CameraObject"; break;
        case 0x3002: tagname = "ShootingRecord"; break;
        case 0x3003: tagname = "MeasuredInfo"; break;
        case 0x3004: tagname = "CameraSpecification"; break;
        case 0x300a: tagname = "ImageProperties"; break;
        case 0x300b: tagname = "ExifInformation"; break;
        default:
            if(snprintf(unknown_buf,11,"TAG_%#06x",(int)tag) > 11)
            {
                printf(" bad tag %#x not converted\n",tag);
                why(stdout);
                tagname = "<BADTAG>";
            }
            else
                tagname = unknown_buf;
            break;
    }
    tagname = strdup(tagname);
    return(tagname);
}

char *
jp2000tagname(unsigned short tag)
{
    static char unknown_buf[16];
    char *tagname;

    switch(tag)
    {
		case 0xFF4F: tagname = "JP2C_SOC"; break;
		case 0xFF90: tagname = "JP2C_SOT"; break;
		case 0xFF93: tagname = "JP2C_SOD"; break;
		case 0xFFd9: tagname = "JP2C_EOC"; break;
		case 0xFF51: tagname = "JP2C_SIZ"; break;
		case 0xFF52: tagname = "JP2C_COD"; break;
		case 0xFF53: tagname = "JP2C_COC"; break;
		case 0xFF5e: tagname = "JP2C_RGN"; break;
		case 0xFF5c: tagname = "JP2C_QCD"; break;
		case 0xFF5d: tagname = "JP2C_QCC"; break;
		case 0xFF5F: tagname = "JP2C_POC"; break;
		case 0xFF55: tagname = "JP2C_TLM"; break;
		case 0xFF57: tagname = "JP2C_PLM"; break;
		case 0xFF58: tagname = "JP2C_PLT"; break;
		case 0xFF60: tagname = "JP2C_PPM"; break;
		case 0xFF61: tagname = "JP2C_PPT"; break;
		case 0xFF91: tagname = "JP2C_SOP"; break;
		case 0xFF92: tagname = "JP2C_EPH"; break;
		case 0xFF63: tagname = "JP2C_CRG"; break;
		case 0xFF64: tagname = "JP2C_COM"; break;
        default:
            if(snprintf(unknown_buf,12,"JP2C_%#06x",(int)tag) > 12)
            {
                printf(" bad tag %#x not converted\n",tag);
                why(stdout);
                tagname = "<BADTAG>";
            }
            else
                tagname = unknown_buf;
            break;
    }
    tagname = strdup(tagname);
    return(tagname);
}
