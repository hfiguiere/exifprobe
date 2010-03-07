/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: dngtags.h,v 1.2 2005/07/12 18:25:13 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef DNGTAGS_INCLUDED
#define DNGTAGS_INCLUDED


#define DNGTAG_DNGVersion			        0xC612	/*	50706	*/
#define DNGTAG_DNGBackwardVersion			0xC613	/*	50707	*/
#define DNGTAG_UniqueCameraModel			0xC614	/*	50708	*/
#define DNGTAG_LocalizedCameraModel			0xC615	/*	50709	*/
#define DNGTAG_CFAPlaneColor			    0xC616	/*	50710	*/
#define DNGTAG_CFALayout			        0xC617	/*	50711	*/
#define DNGTAG_LinearizationTable			0xC618	/*	50712	*/
#define DNGTAG_BlackLevelRepeatDim			0xC619	/*	50713	*/
#define DNGTAG_BlackLevel			        0xC61A	/*	50714	*/
#define DNGTAG_BlackLevelDeltaH			    0xC61B	/*	50715	*/
#define DNGTAG_BlackLevelDeltaV			    0xC61C	/*	50716	*/
#define DNGTAG_WhiteLevel			        0xC61D	/*	50717	*/
#define DNGTAG_DefaultScale			        0xC61E	/*	50718	*/
#define DNGTAG_DefaultCropOrigin			0xC61F	/*	50719	*/
#define DNGTAG_DefaultCropSize			    0xC620	/*	50720	*/
#define DNGTAG_ColorMatrix1			        0xC621	/*	50721	*/
#define DNGTAG_ColorMatrix2			        0xC622	/*	50722	*/
#define DNGTAG_CameraCalibration1			0xC623	/*	50723	*/
#define DNGTAG_CameraCalibration2			0xC624	/*	50724	*/
#define DNGTAG_ReductionMatrix1			    0xC625	/*	50725	*/
#define DNGTAG_ReductionMatrix2			    0xC626	/*	50726	*/
#define DNGTAG_AnalogBalance			    0xC627	/*	50727	*/
#define DNGTAG_AsShotNeutral			    0xC628	/*	50728	*/
#define DNGTAG_AsShotWhiteXY			    0xC629	/*	50729	*/
#define DNGTAG_BaselineExposure			    0xC62A	/*	50730	*/
#define DNGTAG_BaselineNoise			    0xC62B	/*	50731	*/
#define DNGTAG_BaselineSharpness			0xC62C	/*	50732	*/
#define DNGTAG_BayerGreenSplit			    0xC62D	/*	50733	*/
#define DNGTAG_LinearResponseLimit			0xC62E	/*	50734	*/
#define DNGTAG_CameraSerialNumber			0xC62F	/*	50735	*/
#define DNGTAG_LensInfo			            0xC630	/*	50736	*/
#define DNGTAG_ChromaBlurRadius			    0xC631	/*	50737	*/
#define DNGTAG_AntiAliasStrength			0xC632	/*	50738	*/
#define DNGTAG_ShadowScale      			0xC633	/*	50739	*/
#define DNGTAG_DNGPrivateData			    0xC634	/*	50740	*/
#define DNGTAG_MakerNoteSafety			    0xC635	/*	50741	*/
#define DNGTAG_CalibrationIlluminant1		0xC65A	/*	50778	*/
#define DNGTAG_CalibrationIlluminant2		0xC65B	/*	50779	*/
#define DNGTAG_BestQualityScale			    0xC65C	/*	50780	*/
#define DNGTAG_RawDataUniqueID 			    0xC65D	/*	50781	*/

#define DNGTAG_OriginalRawFileName          0xC68B	/*	50827	*/
#define DNGTAG_OriginalRawFileData          0xC68C	/*	50828	*/
#define DNGTAG_ActiveArea                   0xC68D	/*	50829	*/
#define DNGTAG_MaskedAreas                  0xC68E	/*	50830	*/
#define DNGTAG_AsShotICCProfile             0xC68F	/*	50831	*/
#define DNGTAG_AsShotPreProfileMatrix       0xC690	/*	50832	*/
#define DNGTAG_CurrentICCProfile            0xC691	/*	50833	*/
#define DNGTAG_CurrentPreProfileMatrix      0xC692	/*	50834	*/

#endif /* DNGTAGS_INCLUDED */
