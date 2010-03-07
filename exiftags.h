/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: exiftags.h,v 1.6 2005/06/26 23:49:34 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* All tags defined tags in EXIF spec not also defined in TIFF6 or    */
/* JPEG specs                                                         */

#ifndef EXIFTAGS_INCLUDED
#define EXIFTAGS_INCLUDED

#define EXIFTAG_ExposureTime                    0x829A  /* 33434 */
#define EXIFTAG_FNumber                         0x829D  /* 33437 */
#define EXIFTAG_ExifIFDPointer                  0x8769  /* 34665 */
#define EXIFTAG_ExposureProgram                 0x8822  /* 34850 */
#define EXIFTAG_SpectralSensitivity             0x8824  /* 34862 */
#define EXIFTAG_GPSInfoIFDPointer               0x8825  /* 34853 */
#define EXIFTAG_ISOSpeedRatings                 0x8827  /* 34855 */
#define EXIFTAG_OECF                            0x8828  /* 34856 */
#define EXIFTAG_ExifVersion                     0x9000  /* 36864 */
#define EXIFTAG_DateTimeOriginal                0x9003  /* 36867 */
#define EXIFTAG_DateTimeDigitized               0x9004  /* 36868 */
#define EXIFTAG_ComponentsConfiguration         0x9101  /* 37121 */
#define EXIFTAG_CompressedBitsPerPixel          0x9102  /* 37122 */
#define EXIFTAG_ShutterSpeedValue               0x9201  /* 37377 */
#define EXIFTAG_ApertureValue                   0x9202  /* 37378 */
#define EXIFTAG_BrightnessValue                 0x9203  /* 37379 */
#define EXIFTAG_ExposureBiasValue               0x9204  /* 37380 */
#define EXIFTAG_MaxApertureValue                0x9205  /* 37381 */
#define EXIFTAG_SubjectDistance                 0x9206  /* 37382 */
#define EXIFTAG_MeteringMode                    0x9207  /* 37383 */
#define EXIFTAG_LightSource                     0x9208  /* 37384 */
#define EXIFTAG_Flash                           0x9209  /* 37385 */
#define EXIFTAG_FocalLength                     0x920A  /* 37386 */
/* The next tag is called SubjectLocation in TIFF EP             */
#define EXIFTAG_SubjectArea                     0x9214  /* 37396 */
#define EXIFTAG_MakerNote                       0x927C  /* 37500 */
#define EXIFTAG_UserComment                     0x9286  /* 37510 */
#define EXIFTAG_SubSecTime                      0x9290  /* 37520 */
#define EXIFTAG_SubSecTimeOriginal              0x9291  /* 37521 */
#define EXIFTAG_SubSecTimeDigitized             0x9292  /* 37522 */
#define EXIFTAG_FlashPixVersion                 0xA000  /* 40960 */
#define EXIFTAG_ColorSpace                      0xA001  /* 40961 */
#define EXIFTAG_PixelXDimension                 0xA002  /* 40962 */
#define EXIFTAG_PixelYDimension                 0xA003  /* 40963 */
#define EXIFTAG_RelatedSoundFile                0xA004  /* 40964 */
#define EXIFTAG_Interoperability                0xA005  /* 40965 */
#define EXIFTAG_FlashEnergy                     0xA20B  /* 41483 */
#define EXIFTAG_SpatialFrequencyResponse        0xA20C  /* 41484 */
#define EXIFTAG_FocalPlaneXResolution           0xA20E  /* 41486 */
#define EXIFTAG_FocalPlaneYResolution           0xA20F  /* 41487 */
#define EXIFTAG_FocalPlaneResolutionUnit        0xA210  /* 41488 */
#define EXIFTAG_SubjectLocation                 0xA214  /* 41492 */
#define EXIFTAG_ExposureIndex                   0xA215  /* 41493 */
#define EXIFTAG_SensingMethod                   0xA217  /* 41495 */
#define EXIFTAG_FileSource                      0xA300  /* 41728 */
#define EXIFTAG_SceneType                       0xA301  /* 41729 */
#define EXIFTAG_CFAPattern                      0xA302  /* 41730 */
#define EXIFTAG_ExifCustomRendered              0xA401  /* 41985 */
#define EXIFTAG_ExifExposureMode                0xA402  /* 41986 */
#define EXIFTAG_ExifWhiteBalance                0xA403  /* 41987 */
#define EXIFTAG_ExifDigitalZoomRatio            0xA404  /* 41988 */
#define EXIFTAG_ExifFocalLengthIn35mmFilm       0xA405  /* 41989 */
#define EXIFTAG_ExifSceneCaptureType            0xA406  /* 41990 */
#define EXIFTAG_ExifGainControl                 0xA407  /* 41991 */
#define EXIFTAG_ExifContrast                    0xA408  /* 41992 */
#define EXIFTAG_ExifSaturation                  0xA409  /* 41993 */
#define EXIFTAG_ExifSharpness                   0xA40A  /* 41994 */
#define EXIFTAG_ExifDeviceSettingDescription    0xA40B  /* 41995 */
#define EXIFTAG_ExifSubjectDistanceRange        0xA40C  /* 41996 */
#define EXIFTAG_ExifImageUniqueId               0xA420  /* 42016 */
#define EXIFTAG_ExifGamma                       0xA500  /* 42240 */

#define INTEROPTAG_Index                        0x0001
#define INTEROPTAG_Version                      0x0002
#define INTEROPTAG_RelatedFileFormat            0x1000
#define INTEROPTAG_RelatedImageWidth            0x1001
#define INTEROPTAG_RelatedImageLength           0x1002

#endif
