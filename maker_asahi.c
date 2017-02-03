/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_asahi.c,v 1.10 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Asahi/Pentax camera maker-specific routines                        */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* This information is from an image by Johannes Tschebisch at:       */
/* http://www.jojotsch.de/downloads/jojothumb/beispiele/              */
/*                             html_exif/bilder/bilder.html           */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* New info (Jan 05) from Evan Hunter:                                */
/* http://www.ozhiker.com/electronics/pjmt/jpeg_info/pentax_mn.html   */

/* April 05: tagset 3 largely due to Phil Harvey's pages at:          */
/* http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/index.html   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "defs.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

extern struct camera_id pentax_model_id[];

/* Find the identifying number assigned to known Asahi/Pentax camera  */
/* models. This number is used to dispatch print and interpret        */
/* routines approopriate to the current image.                        */

int
asahi_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &pentax_model_id[0]; model_id && model_id->name; ++model_id)
    {
        if(strncasecmp(model,model_id->name,model_id->namelen) == 0)
        {
            number = model_id->id;
            setnoteversion(model_id->noteversion);
            setnotetagset(model_id->notetagset);    /* info only      */
            break;
        }
    }
    return(number);
}

/* If an unknown model is seen, use the makernote scheme to select a  */
/* noteversion to use                                                 */

/* There are some assumptions involved in doing this, but there seems */
/* sufficient difference (and distinctive difference) between         */
/* Asahi/Pentax schemes to make this reasonable.                      */

/* v3 has first tag 0x01 (1 SHORT) ID scheme,  "PENTAX Corporation"   */
/* v2 has first tag 0x02 (2 SHORT) ID scheme,  "Asahi Optical"        */
    /* second tag 0x02 (2 SHORT)                                      */
/* v1 has first tag 0x01 (1 SHORT) ER scheme (no ID), "Asahi Optical" */
    /* second tag 0x02 (1 SHORT)                                      */

int
set_asahi_noteversion()
{
    struct maker_scheme *scheme;
    int noteversion = 0;
    int tagset = 0;

    scheme = retrieve_makerscheme();
    if(scheme->note_version <= 0)
    {
        switch(scheme->scheme_type)
        {
            case HAS_ID_SCHEME:
                if(scheme->first_entry.tag == 0x1)
                {
                    tagset = 3;
                    noteversion = 3;
                }
                else
                {
                    tagset = 2;
                    noteversion = 2;
                }
                break;
            case PLAIN_IFD_SCHEME:
                tagset = 1;
                noteversion = 1;
                break;
        }
        setnotetagset(tagset);
        setnoteversion(noteversion);
    }
    else
        noteversion = scheme->note_version;
    return(noteversion);
}

/* Dispatch print routines for direct values in Asahi/Pentax cameras, */
/* based upon previously set noteversion                              */

void
print_asahi_makervalue(struct ifd_entry *entry_ptr,int make,int model,
                                                        char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 3:
                print_asahi1_makervalue(entry_ptr,make,model,prefix);
                asahi3_interpret_value(entry_ptr);
                break;
            case 2:
                print_asahi1_makervalue(entry_ptr,make,model,prefix);
                asahi2_interpret_value(entry_ptr);
                break;
            case 1:
                print_asahi1_makervalue(entry_ptr,make,model,prefix);
                asahi1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Asahi/Pentax cameras. This        */
/* routine is responsible for picking off any direct entry tags which */
/* are peculiar and will not be handled properly by print_value()     */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_asahi1_makervalue(struct ifd_entry *entry_ptr,int make,int model,
                    char *prefix)
{
    if(entry_ptr)
    {
        switch(entry_ptr->tag) 
        {
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}


/* Dispatch a routine to decode and print offset values for Asahi/Pentax     */
/* cameras.                                                           */

void
print_asahi_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,
    char *prefix,int indent,int make,int model,int at_offset)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 3:
                asahi2_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 2:
                asahi1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 1:
                asahi1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}


/* Model-specific routine to print UNDEFINED values found at offsets  */
/* in Asahi/Pentax makernotes.                                        */

void
asahi1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char *prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset,max_offset;
    unsigned short marker;
    unsigned long count;
    char *nameoftag;
    char *fulldirname = CNULL;
    int status = 0;
    int chpr = 0;

    if(inptr && entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;

        switch(entry_ptr->tag)
        {
            case 0x0004:    /* JpegThumbnailOffset                    */
                /* These models provide the 0x2000 tag as well as the */
                /* 0x0003/0x0004 tags. Both entries point to the same */
                /* thumbnail, so just print values for length and     */
                /* offset tags, and display the thumbnail on the      */
                /* (self-contained) 0x2000 tag                        */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                break;
            case 0x2000: /* Jpeg Thumbnail as UNDEFINED section       */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %lu",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu    # UNDEFINED",value_offset,count);
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                    count,summary_entry,fulldirname,
                                    prefix,indent+SMALLINDENT);
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(VALUE_AT_OFFSET,value_offset + count - 1,
                                                    indent,"-");
                        chpr += printf("# End of JPEG Thumbnail from MakerNote");
                        if((PRINT_ENTRY) && !(PRINT_VALUE))
                            chpr = newline(chpr);
                    }
                }
                print_jpeg_status();
                if(marker && summary_entry)
                { 
                    /* The new one is on the end of the chain         */
                    if((summary_entry = last_summary_entry(summary_entry)))
                    {
                        summary_entry->filesubformat |= FILESUBFMT_MNOTE;
                        summary_entry->datatype = MAKER_IFD;
                        summary_entry->subfiletype = THUMBNAIL_TYPE;
                    }
                }
                /* make certain we're at the end                      */
                clearerr(inptr);
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            case 0x0e00: /* PrintIM (Epson Print Image matching)      */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                    {
                        chpr += printf("@%lu:%lu",value_offset,count);
                        chpr = newline(chpr);
                    }
                }
                process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                    count,nameoftag,parent_name,prefix,indent);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,fileoffset_base,
                                                    fulldirname,prefix,indent,
                                                    make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

void
asahi2_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,
    char *prefix,int indent,int make,int model,int at_offset)
{
    static unsigned long jpegthumbnaillength = 0UL;
    unsigned long value_offset,max_offset;
    unsigned short marker;
    unsigned long count;
    char *nameoftag;
    char *fulldirname = CNULL;
    int status = 0;
    int chpr = 0;

    if(inptr && entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;

        switch(entry_ptr->tag)
        {
            case 0x0003:    /* JpegThumbnailLength                    */
                /* Just record it                                     */
                jpegthumbnaillength = entry_ptr->value;
                break;
            case 0x0004:    /* JpegThumbnailOffset                    */
                if(jpegthumbnaillength)
                {
                    count = jpegthumbnaillength;
                    jpegthumbnaillength = 0;
                }
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %ld",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    else
                        chpr = printf("length %lu",count);
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                    count,summary_entry,fulldirname,
                                    prefix,indent+SMALLINDENT);
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(VALUE_AT_OFFSET,value_offset + count - 1,
                                                    indent,"-");
                        chpr += printf("# End of JPEG Thumbnail from MakerNote");
                        if((PRINT_ENTRY) && !(PRINT_VALUE))
                            chpr = newline(chpr);
                    }
                }
                print_jpeg_status();
                if(marker && summary_entry)
                { 
                    /* The new one is on the end of the chain         */
                    if((summary_entry = last_summary_entry(summary_entry)))
                    {
                        summary_entry->filesubformat |= FILESUBFMT_MNOTE;
                        summary_entry->datatype = MAKER_IFD;
                        summary_entry->subfiletype = THUMBNAIL_TYPE;
                    }
                }
                /* make certain we're at the end                      */
                clearerr(inptr);
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            case 0x0e00: /* PrintIM (Epson Print Image matching)      */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                    count,nameoftag,parent_name,prefix,indent);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base, fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
    }
    setcharsprinted(chpr);
}


/* Asahi/Pentax-specific tagnames for makernotes.                     */

char *
maker_asahi_tagname(unsigned short tag,int model)
{
    char *tagname = (char *)0;
    int noteversion = 0;

    /* Should have to do this only once, and only for unrecognized    */
    /* models. If the model is recognized (or the user has forced a   */
    /* noteversion) noteversion will already be set.                  */
    /* This should already have been done in find_maker_scheme(), but */
    /* there is no harm in checking noteversion after retrieving it.  */
    if(((noteversion = getnoteversion()) == 0))
        noteversion = set_asahi_noteversion();

    switch(noteversion)
    {
        case 1:
            tagname = maker_asahi1_tagname(tag,model);
            break;
        case 2:
            tagname = maker_asahi2_tagname(tag,model);
            break;
        case 3:
            tagname = maker_asahi3_tagname(tag,model);
            break;
        default:
            break;
    }

    /* If no model-specific tag is found, check "generic" tags        */
    if(tagname == NULL)
    {
        switch(tag)
        {
            case 0x0e00: tagname = "PrintIM"; break;
            default: break;
        }
    }
    return(tagname);
}


char *
maker_asahi1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0001: tagname = "CaptureMode"; break;
        case 0x0002: tagname = "Quality"; break;
        case 0x0003: tagname = "FocusMode"; break;
        case 0x0004: tagname = "FlashMode"; break;
        case 0x0007: tagname = "WhiteBalance"; break;
        case 0x000a: tagname = "DigitalZoom"; break;
        case 0x000b: tagname = "Sharpness"; break;
        case 0x000c: tagname = "Contrast"; break;
        case 0x000d: tagname = "Saturation"; break;
        case 0x0014: tagname = "ISOSpeedSetting"; break;
        case 0x0017: tagname = "Color"; break;
        case 0x1000: tagname = "TimeZone"; break;
        case 0x1001: tagname = "DaylightSavings"; break;
    }
    setnotetagset(1);
    return(tagname);
}

char *
maker_asahi2_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    /* This is the same as Casio note version 2                       */
    switch(tag)
    {
        case 0x0002: tagname = "JpegThumbnailDimensions"; break;
        case 0x0003: tagname = "JpegThumbnailLength"; break;
        case 0x0004: tagname = "JpegThumbnailOffset"; break;
        case 0x0008: tagname = "QualityMode"; break;
        case 0x0009: tagname = "ImageSize"; break;
        case 0x000D: tagname = "FocusMode"; break;
        case 0x0014: tagname = "IsoSensitivity"; break;
        case 0x0019: tagname = "WhiteBalance"; break;
        case 0x001D: tagname = "FocalLength"; break; /* units .1mm   */
        case 0x001F: tagname = "Saturation"; break;
        case 0x0020: tagname = "Contrast"; break;
        case 0x0021: tagname = "Sharpness"; break;
        case 0x2000: tagname = "JpegThumbnail"; break;
        case 0x2011: tagname = "WhiteBalanceBias"; break;
        case 0x2012: tagname = "WhiteBalance"; break;
        case 0x2022: tagname = "ObjectDistance"; break;
        case 0x2034: tagname = "FlashDistance"; break;
        case 0x3000: tagname = "RecordMode"; break;
        case 0x3001: tagname = "SelfTimer?"; break;
        case 0x3002: tagname = "Quality"; break;
        case 0x3003: tagname = "FocusMode"; break;
        case 0x3006: tagname = "TimeZone"; break;
        case 0x3007: tagname = "BestshotMode"; break;
        case 0x3014: tagname = "CCDSensitivity"; break;
        case 0x3015: tagname = "ColorMode"; break;
        case 0x3016: tagname = "Enhancement"; break;
        case 0x3017: tagname = "Filter"; break;
        default: break;
    }
    setnotetagset(2);
    return(tagname);
}

char *
maker_asahi3_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0001: tagname = "CaptureMode"; break;
        case 0x0002: tagname = "JpegThumbnailDimensions"; break;
        case 0x0003: tagname = "JpegThumbnailLength"; break;
        case 0x0004: tagname = "JpegThumbnailOffset"; break;
        case 0x0006: tagname = "Date"; break;
        case 0x0007: tagname = "Time"; break;
        case 0x0008: tagname = "QualityMode"; break;
        case 0x0009: tagname = "ImageSize"; break;
        case 0x000B: tagname = "PictureMode"; break;
        case 0x000D: tagname = "FocusMode"; break;
        case 0x0014: tagname = "IsoSensitivity"; break;
        case 0x0017: tagname = "MeteringMode"; break;
        case 0x0019: tagname = "WhiteBalance"; break;
        case 0x001D: tagname = "FocalLength"; break;
        case 0x001E: tagname = "DigitalZoom"; break;
        case 0x001F: tagname = "Saturation"; break;
        case 0x0020: tagname = "Contrast"; break;
        case 0x0021: tagname = "Sharpness"; break;
        case 0x0029: tagname = "FrameNumber"; break;
        case 0x0039: tagname = "RawImageSize"; break;
        case 0x003F: tagname = "LensType"; break;
        case 0x0200: tagname = "BlackPoint"; break;
        case 0x0201: tagname = "WhitePoint"; break;
        case 0x0402: tagname = "ToneCurve"; break;
        case 0x0403: tagname = "ToneCurves"; break;
        default: break;
    }
    setnotetagset(3);
    return(tagname);
}

/* Report the "meaning" of tag values found at offsets in an          */
/* Asahi/Pentax MakerNote IFD entry (not at an offset).               */

void
asahi1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0001:    /* CaptureMode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Night-scene"); break;
                    case 2: chpr += printf("Manual"); break;
                    case 4: chpr += printf("Multiple"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0002:    /* Quality */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Good"); break;
                    case 1: chpr += printf("Better"); break;
                    case 2: chpr += printf("Best"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0003:    /* Focus Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 2: chpr += printf("Custom"); break;
                    case 3: chpr += printf("Auto"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0004:    /* Flash Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Auto"); break;
                    case 2: chpr += printf("On"); break;
                    case 4: chpr += printf("Off"); break;
                    case 6: chpr += printf("Red-eye"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0007:    /* White Balance */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Daylight"); break;
                    case 2: chpr += printf("Shade"); break;
                    case 3: chpr += printf("Tungsten"); break;
                    case 4: chpr += printf("Fluorescent"); break;
                    case 5: chpr += printf("Manual"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x000a:    /* Digital Zoom */
                if(entry_ptr->value == 0)
                {
                    print_startvalue();
                    chpr += printf("Off");
                    print_endvalue();
                }
                break;
            case 0x000b:    /* Sharpness */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Soft"); break;
                    case 2: chpr += printf("Hard"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x000c:    /* Contrast */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Low"); break;
                    case 2: chpr += printf("High"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x000d:    /* Saturation */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Low"); break;
                    case 2: chpr += printf("High"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0014:    /* ISO Speed Setting */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 10: chpr += printf("ISO 100"); break;
                    case 16: chpr += printf("ISO 200"); break;
                    case 100: chpr += printf("ISO 100"); break;
                    case 200: chpr += printf("ISO 200"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0017:    /* Color */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Normal"); break;
                    case 2: chpr += printf("Black & White"); break;
                    case 3: chpr += printf("Sepia"); break;
                    default: break;
                }
                print_endvalue();
                break;
            default:
                break;
        }
    }
    setcharsprinted(chpr);
}

void
asahi2_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0002: /* ThumbnailDimensions" */
                chpr += printf(" pixels");
                break;
            case 0x0008: /* QualityMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Fine"); break;
                    case 2: chpr += printf("Super Fine"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0009: /* ImageSize" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("640x480"); break;
                    case 4: chpr += printf("1600x1200"); break;
                    case 5: chpr += printf("2048x1536"); break;
                    case 8: chpr += printf("2560x1920"); break;
                    case 20: chpr += printf("2288x1712"); break;
                    case 21: chpr += printf("2592x1944"); break;
                    case 22: chpr += printf("2304x1728"); break;
                    case 23: chpr += printf("2296x3056"); break;
                    case 36: chpr += printf("3008x2008"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x000D: /* FocusMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Normal"); break;
                    case 2: chpr += printf("Macro"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0014: /* IsoSensitivity" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 3: chpr += printf("50"); break;
                    case 4: chpr += printf("64"); break;
                    case 6: chpr += printf("100"); break;
                    case 9: chpr += printf("200"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0019: /* WhiteBalance" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Daylight"); break;
                    case 2: chpr += printf("Shade"); break;
                    case 3: chpr += printf("Tungsten"); break;
                    case 4: chpr += printf("Fluorescent"); break;
                    case 5: chpr += printf("Manual"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x001D: chpr += printf(" x .1 mm"); break; 
            case 0x001F: /* Saturation" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("-1"); break;
                    case 1: chpr += printf("Normal"); break;
                    case 2: chpr += printf("+1"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0020: /* Contrast" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("-1"); break;
                    case 1: chpr += printf("Normal"); break;
                    case 2: chpr += printf("+1"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0021: /* Sharpness" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("-1"); break;
                    case 1: chpr += printf("Normal"); break;
                    case 2: chpr += printf("+1"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x2012: /* WhiteBalance" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Manual"); break;
                    case 1: chpr += printf("Auto"); break;
                    case 4: chpr += printf("Flash"); break;
                    case 12: chpr += printf("Flash"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x2022: /* ObjectDistance" */
                chpr += printf("mm");
                break;
            case 0x2034: /* FlashDistance" */
#if 0
    Off?
                if(entry_ptr->value == 0)
                {
                    print_startvalue();
                    chpr += printf("Off");
                    print_endvalue();
                }
#endif
                break;
            case 0x3000: /* RecordMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 2: chpr += printf("Normal"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3001: /* SelfTimer?" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Off"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3002: /* Quality" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Economy"); break;
                    case 2: chpr += printf("Normal"); break;
                    case 3: chpr += printf("Fine"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3003: /* FocusMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Fixed"); break;
                    case 3: chpr += printf("Single-Area Auto"); break;
                    case 6: chpr += printf("Multi-Area Auto"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3007: /* BestshotMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3015: /* ColorMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off*"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3016: /* Enhancement" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x3017: /* Filter" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            default: break;
        }
    }
    setcharsprinted(chpr);
}

void
asahi3_interpret_value(struct ifd_entry *entry_ptr)
{
    int year,month,day;
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0001:    /* CaptureMode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Night-scene"); break;
                    case 2: chpr += printf("Manual"); break;
                    case 4: chpr += printf("Multiple"); break;
                    default: break;
                }
                print_endvalue();
                break;
            case 0x0002: /* ThumbnailDimensions" */
                chpr += printf(" pixels");
                break;
            case 0x0006:
                year = ((entry_ptr->value & 0xff) << 8) | ((entry_ptr->value & 0xff00) >> 8);
                month = ((entry_ptr->value & 0xff0000) >> 16) & 0x7f;
                day = ((entry_ptr->value & 0xff000000) >> 24) & 0x7f;
                chpr += printf(" = %4d:%02d:%02d",year,month,day);
                break;
            case 0x0008: /* QualityMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Good"); break;
                    case 1: chpr += printf("Better"); break;
                    case 2: chpr += printf("Best"); break;
                    case 3: chpr += printf("TIFF"); break;
                    case 4: chpr += printf("RAW"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0009: /* ImageSize" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("640x480"); break;
                    case 1: chpr += printf("Full"); break;
                    case 2: chpr += printf("1024x768"); break;
                    case 4: chpr += printf("1600x1200"); break;
                    case 5: chpr += printf("2048x1536"); break;
                    case 20: chpr += printf("2288x1712"); break;
                    case 21: chpr += printf("2592x1944"); break;
                    case 22: chpr += printf("2304x1728"); break;
                    case 36: chpr += printf("3008x2008"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x000D: /* FocusMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Macro(1)"); break;
                    case 2: chpr += printf("Macro(2)"); break;
                    case 3: chpr += printf("Infinity"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0014: /* IsoSensitivity" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 3: chpr += printf("50"); break;
                    case 4: chpr += printf("64"); break;
                    case 6: chpr += printf("100"); break;
                    case 9: chpr += printf("200"); break;
                    case 12: chpr += printf("400"); break;
                    case 15: chpr += printf("800"); break;
                    case 18: chpr += printf("1600"); break;
                    case 21: chpr += printf("3200"); break;
                    case 50: chpr += printf("50"); break;
                    case 100: chpr += printf("100"); break;
                    case 200: chpr += printf("200"); break;
                    case 400: chpr += printf("400"); break;
                    case 800: chpr += printf("800"); break;
                    case 1600: chpr += printf("1600"); break;
                    case 3200: chpr += printf("3200"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0017: /* MeteringMode" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Multi Segment"); break;
                    case 1: chpr += printf("Center Weighted"); break;
                    case 2: chpr += printf("Spot"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0019: /* WhiteBalance" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Daylight"); break;
                    case 2: chpr += printf("Shade"); break;
                    case 3: chpr += printf("Fluorescent"); break;
                    case 4: chpr += printf("Tungsten"); break;
                    case 5: chpr += printf("Manual"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x001D: chpr += printf(" x .1 mm");
                break; 

            case 0x001F: /* Saturation" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Low"); break;
                    case 2: chpr += printf("High"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0020: /* Contrast" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Low"); break;
                    case 0x10: chpr += printf("Low"); break;
                    case 2: chpr += printf("High"); break;
                    case 0x20: chpr += printf("High"); break;
                    case 3: chpr += printf("Medium Low"); break;
                    case 0x30: chpr += printf("Medium High"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0021: /* Sharpness" */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Soft"); break;
                    case 0x10: chpr += printf("Soft"); break;
                    case 2: chpr += printf("Hard"); break;
                    case 0x20: chpr += printf("Hard"); break;
                    case 3: chpr += printf("Medium Soft"); break;
                    case 0x30: chpr += printf("Medium Hard"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            default: break;
        }
    }
    setcharsprinted(chpr);
}

/* Some makernotes record offsets as LONG values rather than simply   */
/* using UNDEFINED with a size; the same technique used for           */
/* the ExifIFDPointer itself, or JpegInterchangeFormat, etc.          */
/* This routine hardcodes such tags so that the print_makerentry()    */
/* routine will present them as offsets.                              */

/* Asahi has the unfortunate characteristic that tag numbers are      */
/* re-used for different purposes in the second version, and one of   */
/* those tags is an offset masquerading as a long.                    */

int
maker_asahi_value_is_offset(struct ifd_entry *entry_ptr,int model)
{
    int is_offset = 0;
    int noteversion = 0;

    if(entry_ptr)
    {
        noteversion = getnoteversion();

        switch(noteversion)
        {
            case 2:
                switch(entry_ptr->tag)
                {
                    case 0x0004: is_offset = 1; break;
                    default: break;
                }
                break;
            case 3:
                /* A negative value here forces call of the maker offset  */
                /* value routine, so that the routine may record          */
                /* information it will need later; the value is not       */
                /* treated as an offset.                                  */
                switch(entry_ptr->tag)
                {
                    case 0x0003: is_offset = -1; break; 
                    case 0x0004: is_offset = 1; break;
                    default: break;
                }
                break;
            case 1:
            default:
                break;
        }
    }

    return(is_offset);
}

/*
    Tagset 1: 0x01-0x19,0xE00,0x1000,0x1001  330,430
    Tagset 2: 0x02-0x04,0x2000-0x2003,0x2011-0x2013,0x2021-0x2023,0x2031-0x2034,03000-0x3007,0x3011-0x3018
               (first 4 tags thumbnail, even jpeg, both methods)  330RS,430RS
    Tagset 3: 0x01-0x0029,0x0E00,0x002E,...,0x002A,0x002C,0x0031,0x0032  Optio550
    Tagset 3: 0x01-0x0029,....0x0049,0x0200-0x0216,0x03FF,0x0402  starist
                (CaptureMode, next 3 thumbnail, dim/length/offset), date & time (6,7)
 */              
