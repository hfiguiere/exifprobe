/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: misc.c,v 1.22 2005/07/24 16:01:17 alex Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "ciff.h"

/* Return the size in bytes of a TIFF type                            */

int
value_type_size(unsigned short value_type)
{
    int size = 0;

    switch(value_type)
    {
        case BYTE:
        case ASCII:
        case SBYTE:
        case UNDEFINED:
            size = 1;
            break;
        case SHORT:
        case SSHORT:
            size = 2;
            break;
        case LONG:
        case SLONG:
        case FLOAT:
            size = 4;
            break;
        case RATIONAL:
        case SRATIONAL:
        case DOUBLE:
            size = 8;
            break;
        default:
            break;
    }
    return(size);
}

/* Return a string naming the TIFF type of a TIFF entry               */

char *
value_type_name(unsigned short value_type)
{
    char *name;

    switch(value_type)
    {
        case BYTE: name = "BYTE"; break;
        case ASCII: name = "ASCII"; break;
        case SBYTE: name = "SBYTE"; break;
        case UNDEFINED: name = "UNDEFINED"; break;
        case SHORT: name = "SHORT"; break;
        case SSHORT: name = "SSHORT"; break;
        case LONG: name = "LONG"; break;
        case SLONG: name = "SLONG"; break;
        case FLOAT: name = "FLOAT"; break;
        case RATIONAL: name = "RATIONAL"; break;
        case SRATIONAL: name = "SRATIONAL"; break;
        case DOUBLE: name = "DOUBLE"; break;
        default: name = "INVALID"; break;
    }
    return(name);
}


/* caller must call free() */
/* return NULL on error / buffer overflow */
char *
strdup_value(struct ifd_entry *entry, FILE *inptr,
                                        unsigned long fileoffset_base)
{
    char *val = NULL;
    if (is_offset(entry))
    {
        char *buf = (char *)read_bytes(inptr,
                                       entry->count,
                                       fileoffset_base +
                                       entry->value);
        if (buf) {
            val = strndup(buf, entry->count);
        }
        return val;
    }
    else
    {
        char *buf = (char *)&(entry->value);
        val = (char*)calloc(entry->count + 1, sizeof(char));
        for (int i = 0; i < entry->count; i++) {
          val[i] = buf[i];
        }
        return val;
    }
}

/* Print a summary of all of the images found in the file. This will  */
/* include thumbnail images stored by the unpleasant but ubiquitous   */
/* JPEFThumbnailFormat method as well as images described by TIFF     */
/* IFDs and SubIFDS, APP0 thumbnails, CIFF subimages, etc.            */

/* Even formats such as MRW or JP2 which might be considered          */
/* single-image formats can harbor TIFF IFDs (in a TTW block or uuid  */
/* box) which can in turn describe thumbnails or reduced resolution   */
/* images, so almost any image file can harbor this kind of overhead. */

/* The summary attempts to report the image format, location, size,   */
/* and compression type of each block of image data found. For JPEG   */
/* subimages claimed by the file, missing or corrupted data may be    */
/* reported.                                                          */

/* A count of apparently legitimate images found is given at the end, */
/* as well as a count of missing images.                              */

void
print_summary(struct image_summary *summary_entry)
{
    int number_of_images = 0;
    int images_found = 0;
    int chpr = 0;
    char *plural;

    /* Pre-process the summary list; marks primary, reduced-res image */
    /* types and attempts to avoid kruft from imcomplete summary      */
    /* entries.                                                       */
    number_of_images = scan_summary(summary_entry);

    for( ; summary_entry; summary_entry = summary_entry->next_entry)
    {
        PUSHCOLOR(SUMMARY_COLOR);
        images_found += print_image_summary_entry(summary_entry,"@");
        POPCOLOR();
    }

    chpr = newline(chpr);
    plural = NULLSTRING;
    print_filename();
    if(LIST_MODE)
        chpr += printf("NumberOfImages = %d",number_of_images);
    else
        chpr += printf("Number of images = %d",number_of_images);
    if(number_of_images - images_found)
    {
        chpr = newline(chpr);
        print_filename();
        if(LIST_MODE)
            chpr += printf("ImagesNotFound = %d",number_of_images - images_found);
        else
            chpr += printf("Images not found = %d",number_of_images - images_found);
    }
    chpr = newline(chpr);
}

/* Print one entry of the image summary.                              */

int
print_image_summary_entry(struct image_summary *entry,char *prefix)
{
    int chpr = 0;
    int image_found = 0;

    if(entry && (entry->entry_lock > 0) && (entry->imageformat != IMGFMT_NOIMAGE))
    {
        /* Make shell comments out of these if addresses are not      */
        /* printed                                                    */
        if(!(PRINT_ADDRESS))
            chpr += printf("# ");
        else
            print_tag_address(SECTION|ENTRY|SEGMENT,entry->offset,0,"@");
        if(Debug & 0x4)
            chpr += printf("#%d: ",entry->entry_lock);
        chpr += printf("Start of");
        print_imageformat(entry);
        if((entry->imagesubformat & ~IMGSUBFMT_ERRORMASK) != IMGSUBFMT_JPEGTABLES)
        {
            print_imagesubformat(entry);
            print_imagecompression(entry); 
            print_imagesubtype(entry); 
            print_imagesize(entry);
            print_location(entry);
            ++image_found;
        }
        else
            chpr += printf(" length %lu",entry->length);

        if(entry->imagesubformat & IMGSUBFMT_NO_JPEG)
        {
            printred(" (NO SOI)");
            image_found = 0;
        }
        else if(entry->imagesubformat & IMGSUBFMT_JPEGFAILED)
            printred(" (CORRUPTED)");
        else if(entry->imagesubformat & IMGSUBFMT_TRUNCATED)
            printred(" (TRUNCATED)");
        if(entry->length > 0)
        {
            if((PRINT_ADDRESS) == 0)   /* print_tag_address didn't do it     */
                chpr += printf(" at offset %#lx/%lu",entry->offset,entry->offset);
            chpr = newline(chpr);

            if((PRINT_ADDRESS) == 0)
                chpr += printf("# ");
            else
                print_tag_address(SECTION|ENTRY|SEGMENT,
                                            entry->offset + entry->length - 1,
                                            0,"-");
            chpr += printf("  End of");
            print_imageformat(entry); 
            if(entry->imagesubformat != IMGSUBFMT_JPEGTABLES)
            {
                print_imagesubtype(entry);
                if((PRINT_ADDRESS) == 0)   /* print_tag_address didn't do it     */
                    chpr += printf(" image data at offset %#lx/%lu",
                                                    entry->offset + entry->length - 1,
                                                    entry->offset + entry->length - 1);
                else
                    chpr += printf(" image data");
            }
            if(entry->imageformat & IMGFMT_MISMARKED)
            {
                PUSHCOLOR(RED);
                chpr += printf(" *uncompressed data marked as JPEG");
                POPCOLOR();
            }
            chpr = newline(chpr);
        }
        else
        {
            printred(" (no image)");
            image_found = 0;
        }
    }
    
    chpr = newline(chpr);
    return(image_found);
}

/* Scan the summary entry chain in an attempt to identify primary and */
/* reduced-resolution images. TIFF/EP marks types in NewSubfileType,  */
/* but bare TIFF does not. Exif sometimes writes the primary image    */
/* size (which is recorded in the summary entry passed to it), but    */
/* many cameras write abbreviated Exif sections.                      */

/* This routine scans the image summary chain forward and back again, */
/* attempting to write any found primary image size to all entries    */
/* and/or deduce which entry represents the primary image if none     */
/* have been marked by TIFF/EP. This is easy when there are only one  */
/* or two images, but a bit more trouble when there are 4 or 5.       */
/* During the scan, the maximum image size encountered is recorded,   */
/* and may be used to deduce the primary. It is assumed that the      */
/* primary will be maximum size (in pixels); when there are ties, the */
/* image type and/or compression may be checked. Uncompressed, "raw"  */
/* or "lossless" compression at maximum image size is likely the      */
/* primary.                                                           */

/* ###%%% This routine is a work in progress; it will likely change   */
/* and improve as more is learned about the ways in which             */
/* manufacturers are mangling the specifications. The routine         */
/* currently performs a useful and necessary task, and some largely   */
/* useless work. It is heavily dependent upon the code 'out there'    */
/* gathering and recording the proper information.                    */

/* This is called by the print_summary() routine just before printing */
/* the summary. It returns the number of images which are represented */
/* by summary entries.                                                */

int
scan_summary(struct image_summary *summary_entry)
{
    struct image_summary *next_entry = NULL;
    struct image_summary *prev_entry = NULL;
    unsigned long long image_size = 0ULL;
    unsigned long long max_image_size = 0ULL;
    unsigned long marked_primary_image_length = 0ULL;
    unsigned long long marked_primary_image_size = 0ULL;
    unsigned long long primary_image_size = 0ULL;
    unsigned long long next_primary_size = 0ULL;
    unsigned long max_image_width = 0UL;
    unsigned long max_image_height = 0UL;
    unsigned long max_image_length = 0UL;
    unsigned long image_length = 0UL;
    int primary_found = 0;
    int number_of_images = 0;

    if(Debug & 0x4)
        (void)newline(1);

    /* The passed entry should be the head; scan forward merging data */
    while(summary_entry/*  && next_entry */)
    {
        if(summary_entry->entry_lock <= 0)
        {
            summary_entry = summary_entry->next_entry;
            continue;
        }
        if(summary_entry)
            next_entry = summary_entry->next_entry;
        if(Debug & 0x4)
        {
            printred("DEBUG:");
            printf(" %d: ffm=%#x, fsf=%#x",summary_entry->entry_lock,
                    summary_entry->fileformat,summary_entry->filesubformat);
            printf(", spp=%d, bps[0]=%d",summary_entry->spp,summary_entry->bps[0]);
        }

        /* 'image_size' in pixels                                     */
        image_size = summary_entry->pixel_width * summary_entry->pixel_height;

        /* image_length vs summary length: compressed? how much?      */
        /* ###%%% this is "experimental" and shows up only in Debug   */
        if(summary_entry->bps[0] && summary_entry->spp)
        {
            int i;
            int spp = summary_entry->spp;
            float total_bits,bytes_per_pixel;

            total_bits = 0;
            if (spp > MAXSAMPLE)
            {
                spp = MAXSAMPLE;
                if(Debug & 0x4)
                {
                    printf("CLAMPING spp\n");
                }
            }
            for(i = 0; i < spp; ++i)
                total_bits += summary_entry->bps[i];
            bytes_per_pixel = total_bits / 8.0;
            if(Debug & 0x4)
                printf(", bytesperpixel=%.1f",bytes_per_pixel);
            image_length = image_size * bytes_per_pixel;
            summary_entry->compress_percent = (float)summary_entry->length / (float)image_length;
            summary_entry->compress_percent *= 100.0;
        }
        if(Debug & 0x4)
            printf(" c=%lu/%lu=%.0f%%\n",summary_entry->length,image_length,
                                            summary_entry->compress_percent);

        if(summary_entry->subfiletype == PRIMARY_TYPE)
        {
            ++primary_found;    /* marked in a TIFF/EP IFD            */
            marked_primary_image_size = image_size;
            marked_primary_image_length = summary_entry->length;
            if(summary_entry->primary_height == 0UL)
                summary_entry->primary_height = summary_entry->pixel_height;
            if(summary_entry->primary_width == 0UL)
                summary_entry->primary_width = summary_entry->pixel_width;
            
        }
        /* maintain max values independently of primary               */
        if(image_size > max_image_size)
        {
            max_image_height = summary_entry->pixel_height;
            max_image_width = summary_entry->pixel_width;
            max_image_size = image_size;
        }

        /* The largest chunk of image data; could be very important   */
        if(summary_entry->length > max_image_length)
            max_image_length = summary_entry->length;

        /* If a primary size was found, e.g. in an exif ifd, carry    */
        /* it forward. If more than one was found, keep the largest.  */
        /* Then carry it back down to the head in the backward loop.  */
        /* Should wind up with all entries the same (and possibly     */
        /* zero).                                                     */
        if(summary_entry->primary_height || summary_entry->primary_width)
        {
            primary_image_size = summary_entry->primary_width * summary_entry->primary_height;
            if(next_entry && (next_entry->primary_height || next_entry->primary_width))
            {
                next_primary_size = next_entry->primary_width * next_entry->primary_height;
                if(next_primary_size > primary_image_size)
                {
                    summary_entry->primary_width = next_entry->primary_width;
                    summary_entry->primary_height = next_entry->primary_height;
                    primary_image_size = next_primary_size;
                }
                else
                {
                    next_entry->primary_width = summary_entry->primary_width;
                    next_entry->primary_height = summary_entry->primary_height;
                }
            }
            else if(next_entry)
            {
                next_entry->primary_width = summary_entry->primary_width;
                next_entry->primary_height = summary_entry->primary_height;
            }
        }
        /* In JPEG format files, it's almost certain that the primary */
        /* begins at the beginning of the file; "almost" only because */
        /* I've seen some strange things...                           */
        if((summary_entry->fileformat == FILEFMT_JPEG) &&
                            (summary_entry->subfiletype == UNKNOWN_TYPE))
        {
            if(summary_entry->offset == 0)
                summary_entry->subfiletype = POSSIBLE_PRIMARY_TYPE;
        }
        
        prev_entry = summary_entry;
        summary_entry = next_entry;
    }
    summary_entry = prev_entry;
    if((Debug & 0x4) && summary_entry)
    {
        printred("DEBUG:");
        printf(" Exif   Primary=%lux%lu = %llu\n",summary_entry->primary_width,summary_entry->primary_height,primary_image_size);
        printred("DEBUG:");
        printf("        Maxsize=%lux%lu = %llu\n",max_image_width,max_image_height,max_image_size);
        printred("DEBUG:");
        printf(" Marked Primary size = %llu",marked_primary_image_size);
        printf(" Marked Primary length=%lu, max length = %lu\n",marked_primary_image_length,max_image_length);
    }

    /* now walk back to the head                                      */
    /* If a marked primary has been found, the primary job is to mark */
    /* reduced resolution images which haven't been defined as        */
    /* thumbnails in the main code. If no marked primary, it is also  */
    /* necessary to deduce the identity of the primary image, using   */
    /* size, length and/or image and compression type.                */

    /* ###%%% at this point, after all the futzing around with        */
    /* primary_width, etc., it never gets used. max_* are used        */
    /* (and primary_image_size may not have been set if the last      */
    /* entry contained the values)                                    */

    if(summary_entry)
        prev_entry = summary_entry->prev_entry;
    while(summary_entry)
    {
        if(Debug & 0x4)
        {
            printred("DEBUG:");
            printf(" #%d se=%#-9x, pe=%#-9x",summary_entry->entry_lock,
                    (unsigned int)summary_entry,(unsigned int)prev_entry);
        }
        image_size = summary_entry->pixel_width * summary_entry->pixel_height;
        image_length = summary_entry->length;
        if(summary_entry->entry_lock <= 0)
        {
            summary_entry = prev_entry;
            if(summary_entry)
                prev_entry = summary_entry->prev_entry;
            continue;
        }
        if(Debug & 0x4)
        {
            printf(" ffm=%#x, fsf=%#-3x",summary_entry->fileformat,summary_entry->filesubformat);
            printf(", ifm=%#lx, isf=%#lx",summary_entry->imageformat,summary_entry->imagesubformat);
            printf(", offs=%lu, len=%lu",summary_entry->offset,summary_entry->length);
            printf(", pw=%lu, ph=%lu",summary_entry->pixel_width,summary_entry->pixel_height);
        }
        if(prev_entry && (prev_entry->subfiletype != PRIMARY_TYPE))
        {
            /* The primary size was carried up to the last entry in   */
            /* the forward scan; now copy it back down.               */
            /* ###%%% is this used?   ...no, never                    */
            prev_entry->primary_width = summary_entry->primary_width;
            prev_entry->primary_height = summary_entry->primary_height;
        }
        if(Debug & 0x4)
            printf(" sft=%d\n",summary_entry->subfiletype);
        if(primary_found >= 1)  /* should never be greater than 1     */
        {
            if(summary_entry->subfiletype == POSSIBLE_PRIMARY_TYPE)
                summary_entry->subfiletype = REDUCED_RES_TYPE;
        }
        else if(primary_found == 0)
        {
            if(summary_entry->subfiletype == POSSIBLE_PRIMARY_TYPE)
            {
                if((summary_entry->fileformat == FILEFMT_TIFF) ||
                    (summary_entry->fileformat == FILEFMT_ORF1) ||
                        (summary_entry->fileformat == FILEFMT_ORF2))
                {
                    switch(summary_entry->compression)
                    {
                        case 1:
                        case 34712:     /* JPEG 2000 */
                        case 34713:     /* NEF compressed */
                        case JPEG_SOF_3:
                        case JPEG_SOF_7:
                        case JPEG_SOF_11:
                        case JPEG_SOF_15:
                            if(image_size >= max_image_size)
                            {
                                summary_entry->subfiletype = PRIMARY_TYPE;
                                ++primary_found;
                            }
                            break;
                        default:
                            if((image_size < max_image_size) || (image_length < max_image_length))
                                summary_entry->subfiletype = REDUCED_RES_TYPE;
                            break;
                    }
                }
                else if(summary_entry->fileformat == FILEFMT_JP2)
                {
                        summary_entry->subfiletype = PRIMARY_TYPE;
                        ++primary_found;
                }
                else if(summary_entry->fileformat == FILEFMT_JPEG)
                {
                    if(summary_entry->offset == 0)
                    {
                        summary_entry->subfiletype = PRIMARY_TYPE;
                        ++primary_found;
                    }
                    else if(summary_entry->filesubformat & (FILESUBFMT_TIFF|FILESUBFMT_TIFFEP))
                    {
                        switch(summary_entry->compression)
                        {
                            case JPEG_SOF_3:
                            case JPEG_SOF_7:
                            case JPEG_SOF_11:
                            case JPEG_SOF_15:
                                /* lossless compression               */
                                if(image_size >= max_image_size)
                                {
                                    summary_entry->subfiletype = PRIMARY_TYPE;
                                    ++primary_found;
                                }
                                break;
                            default:
                                if((image_size < max_image_size) ||
                                                            (image_length < max_image_length))
                                    summary_entry->subfiletype = REDUCED_RES_TYPE;
                                break;
                        }
                    }
                }
                else if(summary_entry->imagesubformat == IMGSUBFMT_CFA)
                {
                    /* fileformat may be MRW; other fileformats?      */
                    if(image_size >= max_image_size)
                    {
                        summary_entry->subfiletype = PRIMARY_TYPE;
                        ++primary_found;
                    }
                    else
                        summary_entry->subfiletype = REDUCED_RES_TYPE;
                }
                else
                    summary_entry->subfiletype = REDUCED_RES_TYPE;
            }
        }

        if(primary_found)
        {
            if(summary_entry->subfiletype <= POSSIBLE_PRIMARY_TYPE)
                summary_entry->subfiletype = REDUCED_RES_TYPE;
        }
        else if(image_size >= max_image_size)
        {
            if(summary_entry->subfiletype == POSSIBLE_PRIMARY_TYPE)
            {
                summary_entry->subfiletype = PRIMARY_TYPE;
                ++primary_found;
            }
        }
        else if(summary_entry->subfiletype <= POSSIBLE_PRIMARY_TYPE)
            summary_entry->subfiletype = REDUCED_RES_TYPE;

        if(((summary_entry->imageformat) != IMGFMT_NOIMAGE) &&
                ((summary_entry->imagesubformat & ~IMGSUBFMT_ERRORMASK) != IMGSUBFMT_JPEGTABLES))
            ++number_of_images;
        if(Debug & 0x4)
        {
            printred("DEBUG:");
            printf(" #%d se=%#-9x, pe=%#-9x",summary_entry->entry_lock,
                    (unsigned int)summary_entry,(unsigned int)prev_entry);
            printf(" ffm=%#x, fsf=%#-3x",summary_entry->fileformat,summary_entry->filesubformat);
            printf(", ifm=%#lx, isf=%#lx",summary_entry->imageformat,summary_entry->imagesubformat);
            printf(", offs=%lu, len=%lu",summary_entry->offset,summary_entry->length);
            printf(", pw=%lu, ph=%lu",summary_entry->pixel_width,summary_entry->pixel_height);
            printf(" sft=%d\n",summary_entry->subfiletype);
        }
        summary_entry = prev_entry;
        if(summary_entry)
            prev_entry = summary_entry->prev_entry;
        else
            prev_entry = NULL;
    }
    if(Debug & 0x4)
        (void)newline(1);
    return(number_of_images);
}

#include "maker_datadefs.h"
extern struct maker_scheme *retrieve_makerscheme();

/* Print a "file format" as a listing of major sections used in the   */
/* file. It would be nice (perhaps) to print a single file format for */
/* TIFF-derived formats such as CR2, NEF, K25, DNG, etc. but it is    */
/* difficult to say, in the absence of specifications, what makes     */
/* e.g. a CR2 file a CR2 file.                                        */

/* Filename extensions and vendor names do not characterize the       */
/* internal format of the file, and may be applied (erroneously) to   */
/* any file.                                                          */

/* At present, the program keeps watch for certain tag numbers in     */
/* TIFF Ifds which seem to be unique to a particular format, and      */
/* writes a qualifier (e.g. [NEF]) for TIFF-derived types.            */

/* ORF files can be marked reliably because Olympus was clever enough */
/* to change the magic number in the header, a trivial but important  */
/* change which probably should have been done for DNG (and CR and    */
/* NEF). There are even separate magic numbers for the two variants   */
/* of ORF.                                                            */

/* File formats with specific magic numbers in the file header, such  */
/* as MRW, CIFF, and ORF (and TIFF) will show up at the beginning of  */
/* output as that file 'type'. This routine will characterize the     */
/* "file format" by the major sections found in the summary entries.  */
/* Identifiable derived formats (e.g. a NEF compression tag) will be  */
/* indicated. The presence of MakerNotes and subifds in MakerNotes    */
/* will be shown.                                                     */

void
print_fileformat(struct image_summary *entry)
{
    struct maker_scheme *scheme;
    char *sep,*camera_name;
    int chpr = 0;
    int filesubformat_shown = 0;
    int with_filesubformat = 0;
    int vendor_filesubformat = 0;
    int has_appn[16];
    int i;

    print_filename();
    if((PRINT_LONGNAMES))
        chpr += printf("FileFormat = ");
    else
        chpr += printf("File Format = ");

    if(entry)
    {
        memset(has_appn,0,sizeof(has_appn));
        print_filetype(entry->fileformat,0);
        if(entry->filesubformat & FILESUBFMT_TIFFEP)
            chpr += printf("EP");

        if(entry->fileformat == FILEFMT_TIFF)
            filesubformat_shown |= FILESUBFMT_TIFF;
        if(entry->filesubformat & FILESUBFMT_TIFFEP)
            filesubformat_shown |= FILESUBFMT_TIFFEP;

        sep = "/";
        for( ; entry; entry = entry->next_entry)
        {
            if((entry->filesubformat & (FILESUBFMT_TIFF|FILESUBFMT_TIFFOLD)) &&
                        !(filesubformat_shown & (FILESUBFMT_TIFF|FILESUBFMT_TIFFOLD)))
                chpr += printf("%sTIFF",sep);
            if((entry->filesubformat & FILESUBFMT_JPEG) &&
                        !(filesubformat_shown & FILESUBFMT_JPEG))
                chpr += printf("%sJPEG",sep);
            if((entry->filesubformat & FILESUBFMT_GEOTIFF) &&
                        !(filesubformat_shown & FILESUBFMT_GEOTIFF))
                chpr += printf("%sGEOTIFF",sep);
            if((entry->filesubformat & FILESUBFMT_TIFFEP) &&
                        !(filesubformat_shown & FILESUBFMT_TIFFEP))
                chpr += printf("%sEP",sep);
            if((entry->filesubformat & FILESUBFMT_APPN) &&
                        !(filesubformat_shown & FILESUBFMT_APPN))
            {
                if(entry->filesubformatAPPN[0])
                {
                    chpr += printf("%sAPP0",sep);
                    if(entry->filesubformat & FILESUBFMT_JFIF)
                        chpr += printf("%sJFIF",sep);
                    if(entry->filesubformat & FILESUBFMT_JFXX)
                        chpr += printf("%sJFXX",sep);
                }
                if(entry->filesubformatAPPN[1])
                {
                    chpr += printf("%sAPP1",sep);
                    if(entry->filesubformat & (FILESUBFMT_TIFF|FILESUBFMT_TIFFOLD))
                    {
                        entry->filesubformat &= ~(FILESUBFMT_TIFF|FILESUBFMT_TIFFOLD);
                        chpr += printf("%sTIFF",sep);
                    }
                    if(entry->filesubformat & FILESUBFMT_EXIF)
                    {
                        entry->filesubformat &= ~FILESUBFMT_EXIF;
                        chpr += printf("%sEXIF",sep);
                    }
                }
                if(entry->filesubformatAPPN[2])
                {
                    chpr += printf("%sAPP2",sep);
                    if(entry->filesubformat & FILESUBFMT_FPIX)
                    {
                        entry->filesubformat &= ~FILESUBFMT_FPIX;
                        chpr += printf("%sFPIX",sep);
                    }
                }
                for(i = 3; i < 16; ++i)
                {
                    if(entry->filesubformatAPPN[i])
                        has_appn[i] = entry->filesubformatAPPN[i];
                }
            }
            if((entry->filesubformat & FILESUBFMT_CIFF) &&
                        !(filesubformat_shown & FILESUBFMT_CIFF))
                chpr += printf("%sCIFF",sep);
            if((entry->filesubformat & FILESUBFMT_EXIF) &&
                        !(filesubformat_shown & FILESUBFMT_EXIF))
                chpr += printf("%sEXIF",sep);
            if((entry->filesubformat & FILESUBFMT_FPIX) &&
                        !(filesubformat_shown & FILESUBFMT_FPIX))
                chpr += printf("%sFPIX",sep);

            filesubformat_shown |= entry->filesubformat;

            if(entry->filesubformat & FILESUBFMT_NEF)
                vendor_filesubformat |= FILESUBFMT_NEF;
            if(entry->filesubformat & FILESUBFMT_CR2)
                vendor_filesubformat |= FILESUBFMT_CR2;
            if(entry->filesubformat & FILESUBFMT_DNG)
                vendor_filesubformat |= FILESUBFMT_DNG;
            if(entry->filesubformat & FILESUBFMT_MNOTE)
                with_filesubformat |= FILESUBFMT_MNOTE;
            if(entry->filesubformat & FILESUBFMT_MNSUBIFD)
                with_filesubformat |= FILESUBFMT_MNSUBIFD;
        }
        for(i = 3; i < 16; ++i)
        {
            if(has_appn[i])
                chpr += printf("%sAPP%d",sep,i);
        }
        if(vendor_filesubformat & FILESUBFMT_NEF)
                chpr += printf(" [NEF]");
        if(vendor_filesubformat & FILESUBFMT_CR2)
                chpr += printf(" [CR2]");
        if(vendor_filesubformat & FILESUBFMT_DNG)
                chpr += printf(" [DNG]");
        if(with_filesubformat)
            chpr += printf(" #");
        if(with_filesubformat & FILESUBFMT_MNOTE)
        {
            chpr += printf(" with MakerNote");
            scheme = retrieve_makerscheme();
            if(scheme->make)
            {
                camera_name = find_camera_name(scheme->make);
                if(scheme->scheme_type != UNKNOWN_SCHEME)
                    printf(" (%s [%d])",camera_name,scheme->note_version);
                else
                    printf(" (%s - unknown makernote format)",camera_name);
            }
            if(with_filesubformat & FILESUBFMT_MNSUBIFD)
                chpr += printf(" and MakerNote-SubIFD");
        }
    }
    else
        chpr += printf("unknown");
    chpr = newline(chpr);
}

/* Find the name of a device (usually a camera) identified by the     */
/* internal id number 'make'.                                         */

extern struct camera_name Camera_make[];

char *
find_camera_name(int make)
{
    struct camera_name *maker_id;
    char *camera_name = UNKNOWN_CAMERA_STRING;

    /* 'Camera_make' is a global parameter                            */
    for(maker_id = Camera_make; maker_id && maker_id->name; ++maker_id)
    {
        if(make == maker_id->id)
        {
            camera_name = maker_id->name;
            break;
        }
    }
    return(camera_name);
}

void
print_imageformat(struct image_summary *entry)
{
    int chpr = 0;

    if(entry)
    {
        switch(entry->imageformat & ~IMGFMT_MISMARKED)
        {
        case IMGFMT_TIFF:
            if(entry->fileformat == FILEFMT_ORF1)
            {
                chpr += printf(" ORF1 16bps");
                /* ###%%% this is the wrong place to do this          */
                entry->imagesubformat = IMGSUBFMT_CFA;
            }
            else if(entry->fileformat == FILEFMT_ORF2)
            {
                chpr += printf(" ORF2 12bps");
                entry->imagesubformat = IMGSUBFMT_CFA;
            }
            else if(entry->fileformat == FILEFMT_RW2)
            {
                chpr += printf(" RW2");
                entry->imagesubformat = IMGSUBFMT_CFA;
            }
            else
                chpr += printf(" TIFF");
            break;
        case IMGFMT_JPEG:
            if(entry->imagesubformat == IMGSUBFMT_JPEGTABLES)
                chpr += printf("JPEGTables");
            else
                chpr += printf(" JPEG");
            if(entry->imageformat & IMGFMT_MISMARKED)
                chpr += printf("*");
            break;
        case IMGFMT_JPEG2000:
            chpr += printf(" JPEG2000");
            break;
        case IMGFMT_CRW:
            chpr += printf(" CRW");
            break;
        case IMGFMT_MRW:
            chpr += printf(" MRW");
            chpr += printf(" %d/%d",entry->bps[0],entry->sample_size);
            break;
        case IMGFMT_RAF:
            chpr += printf(" RAF");
            break;
        case IMGFMT_X3F:
            chpr += printf(" X3F");
            break;
        default:
            chpr += printf(" UNKNOWN format");
            break;
        }
    }
    setcharsprinted(chpr);
}

void
print_tiff_compression(struct image_summary *entry)
{
    char *compression;
    int chpr = 0;

    compression = tiff_compression_string(entry->compression);
    chpr += printf(" %s",compression);
    setcharsprinted(chpr);
}

char *
tiff_compression_string(unsigned long compvalue)
{
    char *compression;

    compression = NULLSTRING;
    switch(compvalue)
    {
        /* will be 0 only if compression tag does not appear                     */
        case 0: compression = "unknown format"; break;
        case 1: compression = "uncompressed"; break;    /* TIFF6/TIFF_EP         */
        case 2: compression = "Modifed Huffman RLE compressed"; break; /* TIFF6  */
        case 3: compression = "T4 fax encoded"; break;  /* TIFF6                 */
        case 4: compression = "T6 fax encoded"; break;  /* TIFF6                 */
        case 5: compression = "LZW compressed"; break;  /* TIFF6                 */
        case 6: compression = "Exif/old JPEG"; break;   /* TIFF6 pre TechNote 2  */
                                                        /* required by Exif!     */
        case 7: compression = "JPEG"; break;   /* TIFF6 current/TIFF_EP/DNG      */
        case 32773: compression = "Packbits RLE compressed"; break; /* TIFF6     */

        /* These are all from "libtiff -- tiff.h"                     */
        case 8: compression="Adobe Deflate compressed"; break;
        case 32771: compression="NeXT 2-bit RLE compressed"; break;
        case 32809: compression="ThunderScan RLE compressed"; break;
        case 32895: compression="IT8 CT compressed with padding"; break;
        case 32896: compression="IT8 Linework compressed"; break;
        case 32897: compression="IT8 Monochromecompressed"; break;
        case 32898: compression="IT8 Binary line art compressed"; break;
        case 32908: compression="Pixar companded 10bit LZW compressed"; break;
        case 32909: compression="Pixar companded 11bit ZIP compressed"; break;
        case 32946: compression="Deflate compressed"; break;
        case 32947: compression="Kodak DCS encoded"; break;
        case 34661: compression="ISO JBIG compressed"; break;
        case 34676: compression="SGI Log Luminance RLE compressed"; break;
        case 34677: compression="SGI Log 24-bit compressed"; break;

        /* Noted in images                                            */
        case 32867: compression="Kodak KDC compressed"; break;

        case 34712: compression="Jpeg2000 compressed"; break;
        case 34713: compression="NEF compressed"; break;
        case 65000: compression="Kodak DCR compressed"; break;
        default: compression = "(UNKNOWN TIFF compression)"; break;
    }
    return(compression);
}

void
print_jpeg_compression(struct image_summary *entry)
{
    char *compression;
    int chpr = 0;

    compression = NULLSTRING;
    if(entry)
    {
        switch(entry->compression)
        {
            case JPEG_SOF_0: compression="baseline DCT compressed"; break;
            case JPEG_SOF_1: compression="extended seq DCT Huffman"; break;
            case JPEG_SOF_2: compression="progressive DCT Huffman"; break;
            case JPEG_SOF_3: compression="lossless seq Huffman"; break;
            case JPEG_SOF_5: compression="differential seq DCT Huffman"; break;
            case JPEG_SOF_6: compression="differential progressive DCT Huffman"; break;
            case JPEG_SOF_7: compression="differential lossless Huffman"; break;
            case JPEG_SOF_9: compression="extended seq DCT arithmetic"; break;
            case JPEG_SOF_10: compression="progressive seq arithmetic"; break;
            case JPEG_SOF_11: compression="lossless seq arithmetic"; break;
            case JPEG_SOF_13: compression="differential seq DCT arithmetic"; break;
            case JPEG_SOF_14: compression="differential progressive DCT arithmetic"; break;
            case JPEG_SOF_15: compression="differential lossless arithmetic"; break;
            default: compression = "(UNKNOWN JPEG compression)"; break;
        }
    }
    chpr += printf(" %s",compression);
    setcharsprinted(chpr);
}

void
print_jp2c_compression(struct image_summary *entry)
{
    char *transform;
    char *quantization;
    int chpr = 0;

    transform = quantization = NULLSTRING;
    if(entry)
    {
        if((entry->compression & 0xff) == 1)
            transform = "5/3 reversible";
        else
            transform = "9/7 irreversible";
        switch((entry->compression & 0xff00) >> 8)
        {
            case 0:
                quantization = "no quantization";
                break;
            case 1:
                quantization = "implicit quantization";
                break;
            case 2:
                quantization = "explicit quantization";
                break;
            default:
                quantization = "unknown quantization";
                break;
        }
    }
    chpr += printf(" (%s compression with %s)",transform,quantization);
    setcharsprinted(chpr);
}

void
print_crw_compression(struct image_summary *entry)
{
    char *compression;
    int chpr = 0;

    compression = NULLSTRING;
    if(entry)
    {
        switch(entry->compression)
        {
            case 0: compression = "uncompressed"; break;
            case 1: compression = "compressed"; break;
            default: break;
        }
    }
    chpr += printf(" %s",compression);
    setcharsprinted(chpr);
}

void
print_raf_compression(struct image_summary *entry)
{
    char *compression;
    int chpr = 0;

    compression = NULLSTRING;
    if(entry)
    {
        switch(entry->compression)
        {
            case 0: compression = "uncompressed"; break;
            case 1: compression = "uncompressed (with secondary)"; break;
            default: break;
        }
    }
    chpr += printf(" %s",compression);
    setcharsprinted(chpr);
}

void
print_x3f_compression(struct image_summary *entry)
{
    char *compression;
    int chpr = 0;

    compression = NULLSTRING;
    if(entry)
    {
        switch(entry->imagesubformat)
        {
            case IMGSUBFMT_HUFF_COMPRESSED_RGB: compression = "Huffman compressed"; break;
            case IMGSUBFMT_JPEG_COMPRESSED_RGB: compression = "Jpeg compressed"; break;
            case IMGSUBFMT_X3F_COMPRESSED: compression = "compressed"; break;
            case IMGSUBFMT_RGB: compression = "uncompressed"; break;
            default: break;
        }
    }
    chpr += printf(" %s",compression);
    setcharsprinted(chpr);
}



void
print_imagecompression(struct image_summary *entry)
{
    if(entry)
    {
        switch(entry->imageformat)
        {
        case IMGFMT_MRW:    /* gets it from a TIFF IFD                */
        case IMGFMT_TIFF:
            print_tiff_compression(entry);
            break;
        case IMGFMT_JPEG:
            print_jpeg_compression(entry);
            break;
        case IMGFMT_JPEG2000:
            print_jp2c_compression(entry);
            break;
        case IMGFMT_CRW:
            print_crw_compression(entry);
            break;
        case IMGFMT_RAF:
            print_raf_compression(entry);
            break;
        case IMGFMT_X3F:
            print_x3f_compression(entry);
            break;
        default:
            break;
        }
    }
}


void
print_imagesubformat(struct image_summary *entry)
{
    char *subformat;
    unsigned long imagesubformat = 0;
    int chpr = 0;

    subformat = NULLSTRING;
    if(entry)
    {
        imagesubformat = entry->imagesubformat & ~IMGSUBFMT_ERRORMASK;
        switch(imagesubformat)
        {
        case IMGSUBFMT_CFA:
            subformat = "CFA";
            break;
        case IMGSUBFMT_RGB:
            subformat = "RGB";
            break;
        case IMGSUBFMT_YCBCR:
            subformat = "YCBCR";
            break;
        case IMGSUBFMT_PALETTE:
            subformat = "PALETTE";
            break;
        case IMGSUBFMT_LINEARRAW:
            subformat = "LinearRaw";
            break;
        case IMGSUBFMT_JPEGTABLES:
            subformat = "JPEGTables";
            break;
        case IMGSUBFMT_HUFF_COMPRESSED_RGB:
            subformat = "RGB";
            break;
        case IMGSUBFMT_JPEG_COMPRESSED_RGB:
            subformat = "RGB";
            break;
        case IMGSUBFMT_X3F_COMPRESSED:
            subformat = "RAW";
            break;
        case IMGSUBFMT_X3F_UNSPECIFIED:
            subformat = "Unspecified";
            break;
        default:
            break;
        }
    }
    if(*subformat)
        chpr += printf(" %s",subformat);
    setcharsprinted(chpr);
}

void
print_imagesubtype(struct image_summary *entry)
{
    char *type_of_image = UNKNOWN_STRING;
    int chpr = 0;

    switch(entry->subfiletype)
    {
    case THUMBNAIL_TYPE:
        type_of_image = THUMBNAIL_STRING;
        break;
    case PRIMARY_TYPE:
        type_of_image = PRIMARY_STRING;
        break;
    case PAGE_TYPE:
        type_of_image = PAGE_STRING;
        break;
    case MASK_TYPE:
        type_of_image = MASK_STRING;
        break;
    case REDUCED_RES_TYPE:
        type_of_image = REDUCED_RES_STRING; /* if not marked THUMBNAIL */
        break;
    default:
        type_of_image = UNSPECIFIED_STRING;
        break;
    }
    if(*type_of_image)
        chpr += printf(" %s",type_of_image);
    setcharsprinted(chpr);
}

void
print_imagesize(struct image_summary *entry)
{
    int chpr = 0;
    char *maybeshort = NULLSTRING;

    if(entry)
    {
        if(entry->imagesubformat & IMGSUBFMT_JPEG_EARLY_EOI)
            maybeshort = "<= ";
        chpr += printf(" image [%ldx%ld",entry->pixel_width,entry->pixel_height);
        if(entry->subfiletype == PRIMARY_TYPE)
        {
            if((entry->primary_width > entry->pixel_width) ||
                       (entry->primary_height > entry->pixel_height)) 
            {
                chpr += printf("<=%ldx%ld",entry->primary_width,entry->primary_height);
            }
        }
        chpr += printf("] length %s%lu",maybeshort,entry->length);
    }
    setcharsprinted(chpr);
}

/* Indicate where the image was found; the information is contained   */
/* in the filesubformat bits and/or datatype recorded for this        */
/* particular entry.                                                  */

void
print_location(struct image_summary *entry)
{
    char *space = NULL;
    int chpr = 0;

    if(entry)
    {
        space = " (";
        if((entry->fileformat == FILEFMT_TIFF) || 
                    entry->filesubformat & FILESUBFMT_TIFFUSED ||
                                            entry->filesubformatAPPN[1])
        {
            if(entry->ifdnum >= 0)
                chpr += printf("%sIFD %d",space,entry->ifdnum),space = " ";
            if(entry->subifdnum >= 0)
                chpr += printf("%sSubIFD %d",space,entry->subifdnum),space = " ";
            /* could use filesubformat here?                          */
            if(entry->datatype == MAKER_IFD)
                chpr += printf("%sMakerNote",space),space = " ";
            if(entry->datatype == MAKER_SUBIFD)
                chpr += printf("%sMakerNote SubIFD",space),space = " ";
        }
        else if(entry->fileformat == FILEFMT_JPEG)
        {
            if(entry->filesubformatAPPN[0])
            {
                chpr += printf("%sAPP0",space),space = " ";
                if(entry->filesubformat & FILESUBFMT_JFXX)
                    chpr += printf("%sJFXX",space),space = " ";
            }
        }
        if(chpr)
            chpr += printf(")");
    }
}

/* Sum the strip or type bytecounts from a TIFF header to yield the   */
/* size of the image data. The bytecounts may be short or long, so    */
/* the value_type must be checked.                                    */

unsigned long
sum_strip_bytecounts(FILE *inptr,unsigned short byteorder,unsigned long offset,
    unsigned long count,unsigned short value_type)
{
    int i;
    unsigned long sum = 0L;

    if(count)
    {
        /* Some use SHORT, some use LONG. If anyone uses some other   */
        /* type, just ignore 'em.                                     */
        if(value_type == SHORT)
        {
            sum = read_ushort(inptr,byteorder,offset);
            for(i = 1; i < count; ++i)
                sum += read_ushort(inptr,byteorder,HERE);
        }
        else if(value_type == LONG)
        {
            sum = read_ulong(inptr,byteorder,offset);
            for(i = 1; i < count; ++i)
                sum += read_ulong(inptr,byteorder,HERE);
        }
    }
    return(sum);
}


void
why(FILE *outptr)
{
    fprintf(outptr,"\t because: %s\n",strerror(errno));
    errno = 0;
}

unsigned long
toggle(unsigned long options,unsigned long bits)
{
    if(options & bits)
        options &= ~bits;
    else
        options |= bits;
    return(options);
}


/* Clean up memory used to save Make_name, etc.                       */
void
clear_memory()
{
    if(Make_name)
        free(Make_name);
    if(Model_name)
        free(Model_name);
    if(Software_name)
        free(Software_name);
    Make_name = Model_name = Software_name = (char *)0;
}

/* splice two strings together with a separator between. The result   */
/* is placed in dynamic memory. Free it when done with it.            */

char *
splice(char *string1,char *sep,char *string2)
{
    char splicebuf[1024];
    char *result = NULL;

    if(string1 == NULL)
        sep = string1 = NULLSTRING;
    if(string2 == NULL)
        string2 = NULLSTRING;
    snprintf(splicebuf,1024,"%s%s%s",string1,sep,string2);
    result = strdup(splicebuf);
    return(result);
}

/* The following routines attempt to keep track of unterminated       */
/* output lines and see that newlines are output as necessary, and    */
/* never more than necessary.                                         */

/* This requires keeping track of the count of characters printed and */
/* calling either setcharsprinted(xxx) to register them, or           */
/* newline(xxx) to force a newline. "newline(0)" will print a newline */
/* only if Charsprinted is non-zero.                                  */

/* This will work properly only if all functions which produce output */
/* cooperate.                                                         */

static int Charsprinted = 0;

int
newline(int charsprinted)
{
    if((charsprinted != 0) || (Charsprinted != 0))
        putchar('\n');
    Charsprinted = 0;
    return(Charsprinted);
}

void
setcharsprinted(int charsprinted)
{
    Charsprinted += charsprinted;
}

int
charsprinted()
{
    return(Charsprinted);
}

/* Record the filename and optionally prepend the filename to each    */
/* output line. Useful e.g. when multiple files are being LISTed and  */
/* grepped (exifgrep).                                                */

static char *Current_filename = CNULL;

void
setfilename(char *filename)
{
    Current_filename = filename;
}

void
print_filename()
{
    int chpr = 0;

    if(((PRINT_FILENAMES)) && (Current_filename))
    {
        PUSHCOLOR(BLACK);
        chpr = printf("%s: ",Current_filename);
        POPCOLOR();
    }
    setcharsprinted(chpr);
}
