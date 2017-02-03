/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_epson.c,v 1.22 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Epson camera maker-specific routines                               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* This is currently a near-duplicate of the Olympus, based upon the  */
/* similarity of tag numbers, the appearance of sample images taken   */
/* from the Web, and reports that some Olympus, Epson, Minolta and    */
/* Sanyo models use similar firmware.                                 */
/* The "photopc" site at soucrceforge provides some insight:          */
/*    http://photopc.sourceforge.net/cameras.html                     */
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

extern struct camera_id epson_model_id[];

/* Find the identifying number assigned to known Epson camera models. */
/* This number is used to dispatch appropriate print and interpret    */
/* routines approopriate to the current image.                        */

int
epson_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &epson_model_id[0]; model_id && model_id->name; ++model_id)
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


/* Dispatch a print routine for direct IFD values in Epson            */
/* MakerNotes, based upon model                                       */

void
print_epson_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1 :
                print_epson1_makervalue(entry_ptr,make,model,prefix);
                epson1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Epson cameras. This routine is      */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_epson1_makervalue(struct ifd_entry *entry_ptr,int make, int model,
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

/* Dispatch a routine to decode and print offset values in MakerNotes */
/* from Epson cameras.                                                */

void
print_epson_offset_makervalue(FILE *inptr,unsigned short byteorder,
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
            case 1 :
                epson1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                epson1_interpret_offset_makervalue(inptr,byteorder,
                                            entry_ptr,fileoffset_base);
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
/* in epson makernotes.                                               */

void
epson1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset,count;
    unsigned long dumplength;
    unsigned long max_offset = 0;
    unsigned short marker;
    char *nameoftag;
    char *fulldirname = NULL;
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
            case 0x020a:    /* MakerNote version of UserComment   */
                if(at_offset)
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,23," = ",make,model);
                }
                if((PRINT_VALUE))
                {
                    if((PRINT_OFFSET) && !at_offset)
                            chpr += chpr += printf(" = ");
                    print_user_comment(inptr,entry_ptr->count,value_offset,byteorder);
                }
                break;
            case 0x0209:    /* CameraId                           */
                if(at_offset)
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,23," = ",make,model);
                }
                if((PRINT_VALUE))
                {
                    if((PRINT_OFFSET) && !at_offset)
                        chpr += chpr += printf(" = ");
                    print_ascii(inptr,entry_ptr->count,value_offset);
                }
                break;
            case 0x0301:
                /* This is an odd one. Overwrite the values for       */
                /* offset and length, then fall through and display   */
                /* the thumbnail.                                     */
                /* Why didn't they just do this as an UNDEFINED       */
                /* section?                                           */
                value_offset = read_ulong(inptr,byteorder,value_offset);
                count = read_ulong(inptr,byteorder,HERE);
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(SECTION,fileoffset_base + entry_ptr->value,
                                                            indent,prefix);
                    print_makertagid(entry_ptr,23," = ",make,model);
                    if((PRINT_VALUE))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    chpr = newline(chpr);
                }
                else if((LIST_MODE) && !at_offset)
                {
                    if((PRINT_VALUE))
                    {
                        if(!(PRINT_OFFSET))
                            chpr += printf("@%lu",fileoffset_base + entry_ptr->value);
                    }
                    chpr = newline(chpr);
                    /* In list mode, when SECTIONS are not printed,   */
                    /* provide these items as "pseudotags".           */
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",fulldirname);
                        chpr += printf("%-*.*s",9,9,"Offset");
                    }
                    if((PRINT_VALUE))
                    {
                        if((PRINT_OFFSET))
                        {
                            if(PRINT_BOTH_OFFSET)
                                chpr += printf(" = @%#lx=%lu",value_offset,value_offset);
                            else if(PRINT_HEX_OFFSET)
                                chpr += printf(" = @%#lx",value_offset);
                            else if(PRINT_DEC_OFFSET)
                                chpr += printf(" = @%lu",value_offset);
                        }
                        else
                            chpr += printf(" = @%lu",value_offset);
                    }

                    chpr = newline(chpr);
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",fulldirname);
                        chpr += printf("%-*.*s",9,9,"Length");
                    }
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",count);
                }
                /* fall through and process the thumbnail             */
            case 0x0100: /* Jpeg Thumbnail                            */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %ld",count); 
                }
                else if(!at_offset && (PRINT_VALUE) && (entry_ptr->tag == 0x0100))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,entry_ptr->count);
                    else
                        chpr = printf("length %lu", entry_ptr->count);
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
            case 0x0f00:    /* Data                               */
                if(at_offset && (PRINT_ENTRY))
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,23," : ",make,model);
                    chpr += printf("length %lu", count);
                    if(Max_undefined == 0)
                    {
                        if(chpr)
                            printred(" (not dumped, use -U)");
                    }
                    else
                    {
                        if((Max_undefined == DUMPALL) || (Max_undefined > count))
                            dumplength = count;
                        else 
                            dumplength = Max_undefined;
                        chpr = newline(chpr);
                        hexdump(inptr,value_offset,count,dumplength,12,
                                    indent,SUBINDENT);
                        chpr = newline(1);
                    }
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,entry_ptr->count);
                    else
                        chpr = printf("length %lu", entry_ptr->count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf(" # UNDEFINED");
                }
                /* make certain we're at the end                      */
                fseek(inptr,value_offset + count,SEEK_SET);
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

/* Epson-specific tagnames for makernotes.                            */

char *
maker_epson_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;
    int noteversion = 0;

    /* Should have to do this only once, and only for unrecognized    */
    /* models. If the model is recognized (or the user has forced a   */
    /* noteversion) noteversion will already be set.                  */
    if((noteversion = getnoteversion()) == 0)
    {
        noteversion = 1;
        setnoteversion(1);
    }

    switch(noteversion)
    {
        case 1 :
            tagname = maker_epson1_tagname(tag,model);
            break;
        default:
            break;
    }

    if(tagname == NULL)
    {
        switch(tag)
        {
            case 0x0e00: tagname = "PrintIM"; break;
            case 0x0100: tagname = "JpegThumbnail"; break;
            default: break;
        }
    }
    return(tagname);
}
    
char *
maker_epson1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    /* These are the Olympus tags; try them ###%%% ???                */
    /* Update: the SoftwareRelease tag on Olympus is 0x0207; here it  */
    /* is 0x020d.  One Epson model sports a hidden thumbnail (0x0301) */
    switch(tag)
    {
        case 0x0200: tagname = "SpecialMode"; break;
        case 0x0201: tagname = "CompressionMode"; break;
        case 0x0202: tagname = "Macro"; break;
        case 0x0204: tagname = "DigitalZoom"; break;
        case 0x0208: tagname = "PictureInfo"; break;
        case 0x0209: tagname = "CameraID"; break;
        case 0x020a: tagname = "MakerUserComment"; break;
        case 0x020b: tagname = "ImageWidth"; break;
        case 0x020c: tagname = "ImageHeight"; break;
        case 0x020d: tagname = "SoftwareRelease"; break;
        case 0x0301: tagname = "JpegThumbnailData"; break;
        case 0x0e00: tagname = "PrintIM"; break;
        case 0x0f00: tagname = "Data"; break;
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}


/* This arbitrarily assumes that the Epsons use firmware similar to   */
/* that of the Olympus. I have seen no information on the web from    */
/* Epson owners who have decoded notes from their cameras. The        */
/* following seems to be correct based upon Epson PhotoPC 3000Z       */
/* images downloaded from "dpreview" and "imaging resources".         */

void
epson1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0201:    /* Compression Mode */
                print_startvalue();
                /* 'Quality' is a mix of this and image size.             */
                /* Interestingly, 'HyPict' (2544x1904) interpolated       */
                /* images are marked '34' (same as uncompressed 'tiff')   */
                /* but are in JPEG format with CompressedBitsPerPixel=1.  */
                /* The value '33' is used by Olympus cameras, which       */
                /* appear to use similar software. Of course, the Minolta */
                /* F100 looks a lot like this too, but uses '4' for       */
                /* uncompressed.                                          */
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("SQ"); break;
                    case 2: chpr += printf("HQ"); break;
                    case 3: chpr += printf("SHQ"); break;
                    case 33: chpr += printf("not compressed"); break;
                    case 34: chpr += printf("not compressed"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0202:    /* Macro */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Macro"); break;
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

/* Report the "meaning" of tag values found at offsets in an Epson    */
/* MakerNote IFD entry (not at an offset).                            */

void
epson1_interpret_offset_makervalue(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset,value;
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            case 0x0200:    /* SpecialMode                            */
                value = read_ulong(inptr,byteorder,offset);
                print_startvalue();
                /* Don't know what the last 2 values might mean for   */
                /* "normal" or "fast".                                */
                switch(value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Unkown"); break;
                    case 2: chpr += printf("Fast"); break;
                    case 3: chpr += printf("Panorama,");
                            value = read_ulong(inptr,byteorder,HERE);
                            chpr += printf("#%lu,",value);
                            value = read_ulong(inptr,byteorder,HERE);
                            switch(value)
                            {
                                case 1: chpr += printf(" Left to Right"); break;
                                case 2: chpr += printf(" Right to Left"); break;
                                case 3: chpr += printf(" Bottom to Top"); break;
                                case 4: chpr += printf(" Top to Bottom"); break;
                                default: printred(" undefined"); break;
                            }
                            break;
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
