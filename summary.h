/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: summary.h,v 1.9 2005/07/24 21:15:04 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Definitions for image summarys                                     */

#ifndef SUMMARY_INCLUDED
#define SUMMARY_INCLUDED

/* FILE formats                                                       */
#define FILEFMT_TIFF                PROBE_TIFFMAGIC
#define FILEFMT_ORF1                PROBE_ORF1MAGIC
#define FILEFMT_ORF2                PROBE_ORF2MAGIC
#define FILEFMT_RW2                 PROBE_RW2MAGIC
#define FILEFMT_JPEG                PROBE_JPEGMAGIC
#define FILEFMT_JP2                 PROBE_JP2MAGIC
#define FILEFMT_CIFF                PROBE_CIFFMAGIC
#define FILEFMT_MRW                 PROBE_MRWMAGIC
#define FILEFMT_RAF                 PROBE_RAFMAGIC
#define FILEFMT_X3F                 PROBE_X3FMAGIC

/* FILE formats modifiers                                             */
#define FILESUBFMT_TIFF             0x00001
#define FILESUBFMT_TIFFOLD          0x00002
#define FILESUBFMT_TIFFEP           0x00004
#define FILESUBFMT_JPEG             0x00008
#define FILESUBFMT_EXIF             0x00010
#define FILESUBFMT_NEF              0x00020
#define FILESUBFMT_CR2              0x00040
#define FILESUBFMT_DNG              0x00080
/* ADD DCR, KDC? ###%%% */

#define FILESUBFMT_MNOTE            0x00100
#define FILESUBFMT_MNSUBIFD         0x00200

#define FILESUBFMT_CIFF             0x00400 /* may be used in APP0    */
#define FILESUBFMT_GEOTIFF          0x00800 /* not used yet           */

#define FILESUBFMT_JFIF             0x01000
#define FILESUBFMT_JFXX             0x02000
#define FILESUBFMT_FPIX             0x04000
#define FILESUBFMT_APPN             0x08000
/* ADD Meta(APP3)? ###%%% */

#define FILESUBFMT_TIFFUSED         0x003ff

/* IMAGE formats                                                      */
#define IMGFMT_NOIMAGE  0
#define IMGFMT_TIFF                 PROBE_TIFFMAGIC /* including ORF  */
#define IMGFMT_JPEG                 PROBE_JPEGMAGIC
#define IMGFMT_JPEG2000             PROBE_JP2MAGIC
#define IMGFMT_CRW                  PROBE_CIFFMAGIC
#define IMGFMT_MRW                  PROBE_MRWMAGIC
#define IMGFMT_RAF                  PROBE_RAFMAGIC
#define IMGFMT_X3F                  PROBE_X3FMAGIC
#define IMGFMT_MISMARKED            0x10000000  /* for CR2 "JPEG" RGB */

/* IMAGE format modifiers                                             */
/* TIFF Photometric values; first 16 bits if 17th bit                 */
#define IMGSUBFMT_CFA               0x18023     /* 32803 */
#define IMGSUBFMT_MINISWHITE        0x10000
#define IMGSUBFMT_MINISBLACK        0x10001
#define IMGSUBFMT_RGB               0x10002
#define IMGSUBFMT_PALETTE           0x10003
#define IMGSUBFMT_MASK              0x10004
#define IMGSUBFMT_SEPARATED         0x10005
#define IMGSUBFMT_YCBCR             0x10006
#define IMGSUBFMT_CIELAB            0x10008
#define IMGSUBFMT_ICCLAB            0x10009
#define IMGSUBFMT_ITULAB            0x10010
#define IMGSUBFMT_LOGL              0x1804c     /* 32844 */
#define IMGSUBFMT_LOGLUV            0x1804d     /* 32845 */
#define IMGSUBFMT_LINEARRAW         0x1884c     /* 38492 */
#define IMGSUBFMT_PMIMASK           0x0ffff

/* If this bit is set, the PMIMASKED value is the                     */
/* PhotoMetricInterpretation                                          */
#define IMGSUBFMT_VALUE_IS_PMI      0x10000

/* If PMI bit isn't set                                               */
#define IMGSUBFMT_JPEGTABLES        0x015b
#define IMGSUBFMT_HUFF_COMPRESSED_RGB   0x20001
#define IMGSUBFMT_JPEG_COMPRESSED_RGB   0x20002
#define IMGSUBFMT_X3F_UNSPECIFIED       0x20003
#define IMGSUBFMT_X3F_COMPRESSED        0x20004

#if 0
#define IMGSUBFMT_CMY               0x1000
#define IMGSUBFMT_GREYSCALE         0x00e1
#define IMGSUBFMT_BILEVEL           0x0010
#endif

#define IMGSUBFMT_TRUNCATED         0x10000000 
#define IMGSUBFMT_JPEG_EARLY_EOI    0x20000000
#define IMGSUBFMT_NO_JPEG           0x40000000  /* no SOI             */
#define IMGSUBFMT_JPEGFAILED        0x80000000  /* failed after SOI   */
#define IMGSUBFMT_ERRORMASK         0xf0000000

#define MAXSAMPLE                   16  /* size of bps array          */

struct image_summary {
    struct image_summary *prev_entry;
    struct image_summary *next_entry;
    int entry_lock;     /* If set, entry is used, add a new one       */

    /* per file                                                       */
    int fileformat;         /* identifies image FILE format           */
    int filesubformat;      /* file format modifier bits              */
    int filesubformatAPPN[16];   /* APPn sections present in file     */
    unsigned long primary_width; /* size of the primary image         */
    unsigned long primary_height;

    /* per image                                                      */
    unsigned long pixel_width;
    unsigned long pixel_height;
    unsigned long imageformat;    /* IMAGE: JPEG, CFA, JP2, TIFF,...  */
    unsigned long imagesubformat; /* image format modifiers           */
    unsigned long offset;         /* file offset of start of image    */
    unsigned long length;         /* image length in bytes            */

    /* These are used to process individually jpeg-compressed tiles   */
    unsigned long offset_loc;  /* location of offset table            */
    unsigned long length_loc;  /* location of bytecount table         */
    unsigned long chunkhow;    /* strips or tiles                     */
    unsigned long chunktype;   /* short or long                       */
    int noffsets;              /* number of strips, tiles             */

    int subfiletype;        /* PRIMARY, THUMBNAIL, etc.               */
    int compression;        /* interpret according to imageformat     */

    /* This stuff is experimental; not used everywhere                */
    int spp;                /* samples per pixel                      */
    int bps[MAXSAMPLE];     /* one for each conponent                 */
    int sample_size;        /* assume all are the same size           */
    float compress_percent; /* calulate in scan_summary()             */

    /* datatype describes the data structure from which the current   */
    /* image is described. For some file types this may be just the   */
    /* file type, for TIFF it will be the type of IFD (e.g. TIFF_IFD, */
    /* SubIFD, Exif, Interop, etc.                                    */
    int datatype; /* TIFF, EXIF, Interop, GPS, SubIFD...   */

    /* Used only for TIFF                                             */
    int ifdnum;         /* parent idfnum if subifd                    */
    int subifdnum;
};

#endif	/* SUMMARY INCLUDED */
