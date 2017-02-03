/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_sanyo.c,v 1.23 2005/07/24 22:56:27 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* sanyo camera maker-specific routines                             */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* The information coded here is largely due to John Hawkins as shown */
/* at:                                                                */
/*     http://www.exif.org/makernotes/SanyoMakerNote.html             */

/* (John even lists the JPEG thumbnail in TIFF makernotes and the     */
/* Epson PrintIM tag!)                                                */
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

extern struct camera_id sanyo_model_id[];

/* Find the identifying number assigned to known Sanyo camera models. */
/* This number is used to dispatch print and interpret routines       */
/* approopriate to the current image.                                 */

int
sanyo_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &sanyo_model_id[0]; model_id && model_id->name; ++model_id)
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

/* Dispatch a print routine based upon model                          */

void
print_sanyo_makervalue(struct ifd_entry *entry_ptr,int make,int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1:
                print_sanyo1_makervalue(entry_ptr,make,model,prefix);
                sanyo1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for sanyo cameras. This routine is      */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_sanyo1_makervalue(struct ifd_entry *entry_ptr,int make,int model,
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


/* Dispatch a routine to decode and print offset values for sanyo     */
/* cameras.                                                           */
void
print_sanyo_offset_makervalue(FILE *inptr,unsigned short byteorder,
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
                sanyo1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                sanyo1_interpret_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,parent_name,prefix,indent,
                                        make,model,at_offset);
                break;
        }
    }
}

/* Model-specific routine to print UNDEFINED values found at offsets  */
/* in sanyo makernotes. One of these may be supplied for each model   */
/* with unique UNDEFINED tags.                                        */

void
sanyo1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long dumplength;
    unsigned long count;
    unsigned short marker;
    unsigned long max_offset = 0;
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
            case 0x0100:    /* Jpeg Thumbnail in TIFF MakerNote       */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    extraindent(4);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %ld",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,entry_ptr->count);
                    else
                        chpr += printf(":%lu", entry_ptr->count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf(" # UNDEFINED");
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                    count,summary_entry,fulldirname,
                                    prefix,indent+(SUBINDENT * 2));
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(VALUE_AT_OFFSET,value_offset + count - 1,
                                                    indent,"-");
                        extraindent(SUBINDENT);
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
            case 0x0f00: /* Data                                      */
                if(at_offset && (PRINT_ENTRY))
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,23," : ",make,model);
                    chpr += printf("length %-9lu # UNDEFINED", count);
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
                        chpr += printf(":%lu", entry_ptr->count);
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


/* sanyo-specific tagnames for makernotes.                            */

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_sanyo_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;
    int noteversion = 0;

    if(((noteversion = getnoteversion()) == 0))
    {
        /* Need more model samples before it is safe to assume a      */
        /* default                                                    */
        noteversion = 1;
        setnoteversion(1);
    }

    switch(noteversion)
    {
        case 1:
            tagname = maker_sanyo1_tagname(tag,model);
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
maker_sanyo1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0100: tagname = "JpegThumbnail"; break;
        case 0x0200: tagname = "SpecialMode"; break;
        case 0x0201: tagname = "Quality"; break;
        case 0x0202: tagname = "Macro"; break;
        case 0x0204: tagname = "DigiZoom"; break;
        case 0x0207: tagname = "SoftwareRelease"; break;
        case 0x0208: tagname = "PictureInfo"; break;
        case 0x0209: tagname = "CameraID"; break;
        case 0x020e: tagname = "SeqShotMethod"; break;
        case 0x020f: tagname = "WideRange"; break;
        case 0x0210: tagname = "ColorAdjMode"; break;
        case 0x0213: tagname = "QuickShot"; break;
        case 0x0214: tagname = "SelfTImer"; break;
        case 0x0216: tagname = "VoiceMemo"; break;
        case 0x0217: tagname = "RecordShutterRel"; break;
        case 0x0218: tagname = "FlickerReduce"; break;
        case 0x0219: tagname = "OpticalZoom"; break;
        case 0x021b: tagname = "DigitalZoom"; break;
        case 0x021d: tagname = "LightSourceSpecial"; break;
        case 0x021e: tagname = "Resaved"; break;
        case 0x021f: tagname = "SceneSelect"; break;
        case 0x0223: tagname = "ManualFocalDist"; break;
        case 0x0224: tagname = "SeqShotIntvl"; break;
        case 0x0225: tagname = "FlashMode"; break;
        case 0x0e00: tagname = "PrintIM"; break;
        case 0x0f00: tagname = "Data"; break;
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}


void
sanyo1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0201:    /* Quality */
                print_startvalue();
                /* ###%%% is this related to image size, e.g. 640x480?    */
                /* This tag is the same tag number as used by the         */
                /* Olympus, Epson, and Minolta F100, except that "super   */
                /* high" + "super fine" would give a (short) value of 34, */
                /* which is the value used for uncompressed tiff in the   */
                /* Epson at 2048x1536.                                    */
                switch(entry_ptr->value & 0xff)
                {
                    case 0: chpr += printf("very low res,"); break;
                    case 1: chpr += printf("low res,"); break;
                    case 2: chpr += printf("medium low res,");    /* 640 x 480 */ break;
                    case 3: chpr += printf("medium res,"); break;
                    case 4: chpr += printf("medium high res,"); break;
                    case 5: chpr += printf("high res,"); break;
                    case 6: chpr += printf("very high res,"); break;
                    case 7: chpr += printf("super high"); break;
                    default: printred("undefined"); break;
                }
                /* ###%%% is this related to compression, e.g TIFF vs     */
                /* JPEG ???                                               */
                switch((entry_ptr->value >> 8) & 0xff)
                {
                    case 0: chpr += printf(" normal"); break;
                    case 1: chpr += printf(" fine"); break;
                    case 2: chpr += printf(" super fine"); break;
                }
                print_endvalue();
                break;
            case 0x0202:    /* Macro */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Macro"); break;
                    case 2: chpr += printf("View"); break;
                    case 3: chpr += printf("Manual"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x020e:    /* Sequential Shot Method */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("none"); break;
                    case 1: chpr += printf("Standard"); break;
                    case 2: chpr += printf("Best"); break;
                    case 3: chpr += printf("Exposure Adjust"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x020f:    /* Wide Range */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0210:    /* Color Adjustment Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0213:    /* Quick Shot */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0214:    /* Self Timer  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0216:    /* Voice Memo  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0217:    /* Record Shutter Release  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Held"); break;
                    case 1: chpr += printf("Start/Stop"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0218:    /* Flicker Reduce  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0219:    /* Optical Zoom  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Disabled"); break;
                    case 1: chpr += printf("Enabled"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x021b:    /* Digital Zoom  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Disabled"); break;
                    case 1: chpr += printf("Enabled"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x021d:    /* Light Source Special  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x021e:    /* Resaved  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("No"); break;
                    case 1: chpr += printf("Yes"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x021f:    /* Scene Select  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("Sport"); break;
                    case 2: chpr += printf("TV?"); break;
                    case 3: chpr += printf("Night"); break;
                    case 4: chpr += printf("User 1"); break;
                    case 5: chpr += printf("User 2"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0224:    /* Sequential Shot Interval  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("5 frames/sec"); break;
                    case 1: chpr += printf("10 frames/sec"); break;
                    case 2: chpr += printf("15 frames/sec?"); break;
                    case 3: chpr += printf("20 frames/sec"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0225:    /* Flash Mode  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Force"); break;
                    case 2: chpr += printf("Disabled"); break;
                    case 3: chpr += printf("Redeye"); break;
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
sanyo1_interpret_offset_makervalue(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset,value;
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            case 0x0200:    /* SpecialMode                               */
                value = read_ulong(inptr,byteorder,offset);
                print_startvalue();
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
