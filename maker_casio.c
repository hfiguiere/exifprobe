/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_casio.c,v 1.19 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Casio camera maker-specific routines.                              */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Most of the information coded here is due to Eckhard Henkel        */
/* (eckhard.henkel@t-online.de) as shown by TsuruZoh Tachibanaya at:  */
/* http://www.ba.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html   */
/* (now found at:                                                     */
/*   http://landscapeimage.com/ThumbHTML/help/exif_file_format.html)  */
/* Additional information:                                            */
/*  http://www.dicasoft.de/casiomn.htm                                */

/* EP-600 info: (added Jan 05)                                        */
/* evan@ozhiker.com                                                   */
/*    http://www.ozhiker.com/electronics/pjmt/jpeg_info/casio_mn.html */
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


extern struct camera_id casio_model_id[];

int
casio_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &casio_model_id[0]; model_id && model_id->name; ++model_id)
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

/* Determine and set note version from detected scheme.               */
/* Notes with ID scheme use noteversion 2                             */
/* Notes with Plain use noteversion 1                                 */

int
set_casio_noteversion()
{
    struct maker_scheme *scheme;
    int noteversion = 0;
    int tagset = 0;

    scheme = retrieve_makerscheme();
    if(scheme->note_version <= 0)
    {
        noteversion = -1;
        tagset = -1;
        switch(scheme->scheme_type)
        {
            case HAS_ID_SCHEME:     /* "QVC" */
                tagset = 2;
                noteversion = 2;
                break;
            case PLAIN_IFD_SCHEME:
                tagset = 1;
                noteversion = 1;
                break;
            case UNKNOWN_SCHEME:
                break;
            default: 
                break;
        }
        setnotetagset(tagset);
        setnoteversion(noteversion);
    }
    else
        noteversion = scheme->note_version;
    return(noteversion);
}


/* Dispatch a Casio print routine based upon noteversion              */

void
print_casio_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 2:
                print_casio1_makervalue(entry_ptr,make,model,prefix);
                casio2_interpret_value(entry_ptr);
                break;
            case 1:
                print_casio1_makervalue(entry_ptr,make,model,prefix);
                casio1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Casio cameras. This routine is    */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_casio1_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag) 
        {
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}


/* Dispatch a routine to decode and print offset values for Casio     */
/* cameras.                                                           */

void
print_casio_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 1:
                casio1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 2:
                casio2_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,parent_name,prefix,
                                        indent,make,model,at_offset);
                break;
        }
    }
}

/* Model-specific routine to print UNDEFINED values found at offsets  */
/* in Casio makernotes.                                               */

void
casio1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    char *nameoftag;
    unsigned long value_offset,current_offset;
    unsigned long count;
    int chpr = 0;
    char *fulldirname = NULL;

    if(entry_ptr)
    {
        value_offset = fileoffset_base + entry_ptr->value;
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        count = entry_ptr->count;

        switch(entry_ptr->tag)
        {
            case 0x0e00: /* PrintIM (Epson Print Image matching)  */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                    {
                        chpr += printf("@%lu:%lu",value_offset,count);
                        chpr = newline(chpr);
                    }
                }
                current_offset = ftell(inptr);
                if(process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                    count,nameoftag,parent_name,prefix,indent) == 0)
                {
                    /* At least one QV4000 image uses a PIM offset    */
                    /* relative to the entry address, even though     */
                    /* other entries in the note are TIFF-relative;   */
                    /* check for a PIM entry at that location if the  */
                    /* previous one failed. GROSS HACK!               */
                    fileoffset_base = current_offset - 12;
                    PUSHCOLOR(HI_BLACK);
                    chpr += printf(" checking for entry-relative offset bug");
                    POPCOLOR();
                    if(process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                                    count,nameoftag,parent_name,
                                    prefix,indent))
                    {
                        if(!(PRINT_SECTION))
                        {
                            chpr = newline(chpr);
                            printred("# WARNING:");
                        }
                        PUSHCOLOR(HI_BLACK);
                        chpr += printf(" %s data found at offset %lu - ",nameoftag,
                                                entry_ptr->value + fileoffset_base);
                        printred("start offset is incorrect!");
                        POPCOLOR();
                        if(!(PRINT_SECTION))
                        {
                            chpr = newline(chpr);
                            print_filename();
                            if((PRINT_TAGINFO))
                            {
                                if((PRINT_LONGNAMES))
                                    chpr += printf("%s.",fulldirname);
                                chpr += printf("%-*.*s",MAKERTAGWIDTH,MAKERTAGWIDTH,nameoftag);
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = @%lu:%lu",entry_ptr->value + fileoffset_base,
                                                                                        count);
                        }
                    }
                }
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

void
casio2_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset,current_offset;
    unsigned long count;
    unsigned short marker;
    char *nameoftag;
    char *fulldirname = NULL;
    unsigned long max_offset = 0;
    int status = 0;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;

        switch(entry_ptr->tag)
        {
            case 0x0004:    /* JpegThumbnailOffset                    */
                /* When this tag is present, the 0x2000 tag is        */
                /* present as well. Both entries point to the same    */
                /* thumbnail, so just print values for length and     */
                /* offset tags, and display the thumbnail on the      */
                /* (self-contained) 0x2000 tag                        */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                break;
            case 0x2000:   /* JpegThumbnail                           */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(SECTION,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %ld",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,entry_ptr->count);
                    else
                        chpr += printf("length %lu", entry_ptr->count);
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
                                                    indent,prefix);
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
            case 0x0e00: /* PrintIM (Epson Print Image matching)  */
                nameoftag = maker_tagname(entry_ptr->tag,make,model); 
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                current_offset = ftell(inptr);
                if(process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                    entry_ptr->count,nameoftag,parent_name,prefix,indent) == 0)
                {
                    /* At least one QV4000 image uses a PIM offset    */
                    /* relative to the entry address, even though     */
                    /* other entries in the note are TIFF-relative;   */
                    /* check for a PIM entry at that location if the  */
                    /* previous one failed. GROSS HACK!               */
                    fileoffset_base = current_offset - 12;
                    PUSHCOLOR(HI_BLACK);
                    /* We've already seen an error message            */
                    chpr += printf(" checking for entry-relative offset bug");
                    POPCOLOR();
                    if(process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                                    entry_ptr->count,nameoftag,parent_name,
                                    prefix,indent))
                    {
                        if(!(PRINT_OFFSET))
                        {
                            chpr = newline(chpr);
                            chpr += printf("# WARNING:");
                        }
                        PUSHCOLOR(HI_BLACK);
                        chpr += printf(" PrintImage data found at offset %lu - ",
                                                    entry_ptr->value + fileoffset_base);
                        printred("start offset is incorrect!");
                        POPCOLOR();
                        if(!(PRINT_SECTION))
                        {
                            chpr = newline(chpr);
                            print_tag_address(VALUE_AT_OFFSET,entry_ptr->value + fileoffset_base,
                                                        indent,"-");
                            if((PRINT_TAGINFO))
                            {
                                if((PRINT_LONGNAMES))
                                    chpr += printf("%s.",fulldirname);
                                chpr += printf("%-*.*s",MAKERTAGWIDTH,MAKERTAGWIDTH,nameoftag);
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = @%lu",entry_ptr->value + fileoffset_base);
                        }
                    }
                }
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}


/* Casio-specific tagnames for makernotes.                            */

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_casio_tagname(unsigned short tag,int model)
{
    char *tagname = (char *)0;
    int noteversion = 0;

    if((noteversion = getnoteversion()) == 0)
    {
        noteversion = set_casio_noteversion();
        setnoteversion(noteversion);
    }

    /* Check noteversions first                                       */
    switch(noteversion)
    {
        case 1:
            tagname = maker_casio1_tagname(tag,model);
            break;
        case 2:
            tagname = maker_casio2_tagname(tag,model);
            break;
        default:
            break;
    }

    /* If no version-specific tag is found, check "generic" tags      */
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
maker_casio1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0001: tagname = "RecordingMode"; break;
        case 0x0002: tagname = "Quality"; break;
        case 0x0003: tagname = "FocusingMode"; break;
        case 0x0004: tagname = "FlashMode"; break;
        case 0x0005: tagname = "FlashIntensity"; break;
        case 0x0006: tagname = "ObjectDistance"; break;
        case 0x0007: tagname = "WhiteBalance"; break;
        case 0x000a: tagname = "DigitalZoom"; break;
        case 0x000b: tagname = "Sharpness"; break;
        case 0x000c: tagname = "Contrast"; break;
        case 0x000d: tagname = "Saturation"; break;
        case 0x0014: tagname = "CCDSensitivity"; break;
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}


char *
maker_casio2_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    /* This is the same as Asahi/Pentax note version 2                */
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

/* Report the "meaning" of tag values found directly in a Casio       */
/* MakerNote IFD entry (not at an offset).                            */

void
casio1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0001:    /* RecordingMode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Single Shutter"); break;
                    case 2: chpr += printf("Panorama"); break;
                    case 3: chpr += printf("Night Scene"); break;
                    case 4: chpr += printf("Portrait"); break;
                    case 5: chpr += printf("Landscape"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0002:    /* Quality */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Economy"); break;
                    case 2: chpr += printf("Normal"); break;
                    case 3: chpr += printf("Fine"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0003:    /* Focusing Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 2: chpr += printf("Macro"); break;
                    case 3: chpr += printf("Auto"); break;
                    case 4: chpr += printf("Manual"); break;
                    case 5: chpr += printf("Infinity"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0004:    /* Flash Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Auto"); break;
                    case 2: chpr += printf("On"); break;
                    case 3: chpr += printf("Off"); break;
                    case 4: chpr += printf("Redeye"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0005:    /* Flash Intensity */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 11: chpr += printf("Weak"); break;
                    case 13: chpr += printf("Normal"); break;
                    case 15: chpr += printf("Strong"); break;
                    default: printred("undefined"); }
                break;
            case 0x0006:    /* Object Distance */
                PUSHCOLOR(UNCERTAIN_COLOR);
                chpr += printf(" mm");
                POPCOLOR();
                break;
            case 0x0007:    /* White Balance */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Auto"); break;
                    case 2: chpr += printf("Tungsten"); break;
                    case 3: chpr += printf("Daylight"); break;
                    case 4: chpr += printf("Fluorescent"); break;
                    case 5: chpr += printf("Shade"); break;
                    case 129: chpr += printf("Manual"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x000a:    /* Digital Zoom */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0x10000: chpr += printf("Off"); break;
                    case 0x10001: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x000b:    /* Sharpness */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Soft"); break;
                    case 2: chpr += printf("Hard"); break;
                    default: printred("undefined"); break;
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
                    default: printred("undefined"); break;
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
                    default: printred("undefined"); break;
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
casio2_interpret_value(struct ifd_entry *entry_ptr)
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
                    case 1: chpr += printf("Fine"); break;
                    case 2: chpr += printf("Shade"); break;
                    case 3: chpr += printf("Tungsten"); break;
                    case 4: chpr += printf("Fluorescent"); break;
                    case 5: chpr += printf("Manual"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            /* rational? case 0x001D: tagname = "FocalLength"; break;  units .1mm   */
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
#if 0
Off???
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
#endif
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

/* Some makernotes record offsets as LONG values rather than simply   */
/* using UNDEFINED with a size; the same technique used for           */
/* the ExifIFDPointer itself, or JpegInterchangeFormat, etc.          */
/* This routine hardcodes such tags so that the print_makerentry()    */
/* routine will present them as offsets.                              */

int
maker_casio_value_is_offset(struct ifd_entry *entry_ptr,int model)
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
            default:
                break;
        }
    }
    return(is_offset);
}
