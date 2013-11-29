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

/* DNG 1.2.0 */

#define DNGTAG_ColorimetricReference        0xC6BF	/*	50879	*/
#define DNGTAG_CameraCalibrationSignature   0xC6F3	/*	50931	*/
#define DNGTAG_ProfileCalibrationSignature  0xC6F4	/*	50932	*/
#define DNGTAG_ExtraCameraProfiles          0xC6F5	/*	50933	*/
#define DNGTAG_AsShotProfileName            0xC6F6	/*	50934	*/
#define DNGTAG_NoiseReductionApplied        0xC6F7	/*	50935	*/
#define DNGTAG_ProfileName                  0xC6F8	/*	50936	*/
#define DNGTAG_ProfileHueSatMapDims         0xC6F9	/*	50937	*/
#define DNGTAG_ProfileHueSatMapData1        0xC6FA	/*	50938	*/
#define DNGTAG_ProfileHueSatMapData2        0xC6FB	/*	50939	*/
#define DNGTAG_ProfileToneCurve             0xC6FC	/*	50940	*/
#define DNGTAG_ProfileEmbedPolicy           0xC6FD	/*	50941	*/
#define DNGTAG_ProfileCopyright             0xC6FE	/*	50942	*/

#define DNGTAG_ForwardMatrix1               0xC714	/*	50964	*/
#define DNGTAG_ForwardMatrix2               0xC715	/*	50965	*/
#define DNGTAG_PreviewApplicationName       0xC716	/*	50966	*/
#define DNGTAG_PreviewApplicationVersion    0xC717	/*	50967	*/
#define DNGTAG_PreviewSettingsName          0xC718	/*	50968	*/
#define DNGTAG_PreviewPreviewSettingsDigest 0xC719	/*	50969	*/
#define DNGTAG_PreviewPreviewColorSpace     0xC71A	/*	50970	*/
#define DNGTAG_PreviewPreviewDateTime       0xC71B	/*	50971	*/
#define DNGTAG_RawImageDigest               0xC71C	/*	50972	*/
#define DNGTAG_OriginalRawFileDigest        0xC71D	/*	50973	*/
#define DNGTAG_SubTileBlockSize             0xC71E	/*	50974	*/
#define DNGTAG_RowInterleaveFactor          0xC71F	/*	50975	*/
#define DNGTAG_ProfileLookTableDims         0xC725	/*	50981	*/
#define DNGTAG_ProfileLookTableData         0xC726	/*	50982	*/


/* DNG 1.3.0 */

#define DNGTAG_OpcodeList1                  0xC740      /*      51008   */
#define DNGTAG_OpcodeList2                  0xC741      /*      51009   */
#define DNGTAG_OpcodeList3                  0xC74E      /*      51022   */
#define DNGTAG_NoiseProfile                 0xC761      /*      51041   */

/* Cinema DNG */

#define DNGTAG_TimeCodes                    0xC763      /*      51043   */
#define DNGTAG_FrameRate                    0xC764      /*      51044   */
#define DNGTAG_TStop                        0xC772      /*      51058   */
#define DNGTAG_ReelName                     0xC789      /*      51081   */
#define DNGTAG_CameraLabel                  0xC7A1      /*      51105   */

/* DNG 1.4.0 */

#define DNGTAG_OriginalDefaultFinalSize     0xC791      /*      51089   */
#define DNGTAG_OriginalBestQualityFinalSize 0xC792      /*      51090   */
#define DNGTAG_OriginalDefaultCropSize      0xC793      /*      51091   */
#define DNGTAG_ProfileHueSatMapEncoding     0xC7A3      /*      51107   */
#define DNGTAG_ProfileLookTableEncoding     0xC7A4      /*      51108   */
#define DNGTAG_BaselineExposureOffset       0xC7A5      /*      51109   */
#define DNGTAG_DefaultBlackRender           0xC7A6      /*      51110   */
#define DNGTAG_NewRawImageDigest            0xC7A7      /*      51111   */
#define DNGTAG_RawToPreviewGain             0xC7A8      /*      51112   */
#define DNGTAG_DefaultUserCrop              0xC7B5      /*      51125   */

#endif /* DNGTAGS_INCLUDED */
