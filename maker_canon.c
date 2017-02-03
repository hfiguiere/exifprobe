/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_canon.c,v 1.29 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Canon camera maker-specific routines                               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* The information coded here is derived from information provided by */
/* David Burren at:                                                   */
/*     http://www.burren.cx/david/canon.html                          */

/* April 05: new tag information and CustomFunctions for additional   */
/* models from Phil Harvey's exiftool pages at:                       */
/* http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/index.html   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "defs.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

extern struct camera_id canon_model_id[];


/* Find the identifying number assigned to known Canon camera models. */
/* This number is used to dispatch appropriate print and interpret    */
/* routines approopriate to the current image.                        */

int
canon_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &canon_model_id[0]; model_id && model_id->name; ++model_id)
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

/* Dispatch a print routine based upon previously set noteversion     */
/* Everything is (currently) mapped to one set of routines.           */

void
print_canon_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                print_canon1_makervalue(entry_ptr,make,model,prefix);
                canon1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Canon cameras. This routine is    */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_canon1_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                                                            char *prefix)
{
    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag) 
        {
            case 0x000C:    /* Camera serial number                   */
                print_value(entry_ptr,PREFIX);
                /* ###%%%                                             */
                /* perhaps this should be in canon1_interpret_value() */
                (void)print_canon_serialno(entry_ptr->value);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Canon serial numbers are encoded. It is not clear that this is     */
/* consistent across all models, but since the "raw" value is         */
/* displayed as well, it will do no harm to display this              */
/* unconditionally.                                                   */

int
print_canon_serialno(unsigned long value)
{
    unsigned long tmp;
    int chpr = 0;

    print_startvalue();
    tmp = (value & 0xffff0000) >> 16;
    chpr += printf("%04lX",value & 0xffff);
    tmp = value & 0xffff;
    chpr += printf("%05lu",value & 0xffff);
    print_endvalue();
    setcharsprinted(chpr);
    return(chpr);
}

/* Dispatch a routine to decode and print offset values for Canon     */
/* cameras. There is currently only one tagset, and 1 interpretation  */
/* of each tag, except that there are 4 different varieties of custom */
/* functions. The canon_customfunctions() routine will do the         */
/* selection on noteversion; everything is handled the same here.     */

void
print_canon_offset_makervalue(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
                    struct image_summary *summary_entry,char *parent_name,
                    char *prefix,int indent,int make,int model,
                    int at_offset)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            /* noteversion 1 assumes the single known tagset is       */
            /* correct, but imposes a "generic" routine for           */
            /* CustomFunctions (which presents values but interprets  */
            /* nothing) for unrecognized models or those for which no */
            /* information is available.                              */
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                canon1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,fileoffset_base,
                                                parent_name,prefix,indent,
                                                make,model,at_offset);
                break;
        }
    }
}

/* Model-specific routine to print UNDEFINED values found at offsets  */
/* in canon makernotes.                                               */

void
canon1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long count;
    unsigned long max_offset = 0;
    unsigned long filesize = 0UL;
    unsigned short marker;
    char *nameoftag,*tagprefix;
    char *fulldirname = NULL;
    int status = 0;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        filesize = get_filesize(inptr);

        switch(entry_ptr->tag)
        {
            case 0x0001:    /* Camera Settings                        */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count - 1);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count - 1);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_camera_settings(inptr,byteorder,tagprefix,value_offset,
                                        entry_ptr->count * sizeof(unsigned short),
                                                            indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                                            indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                break;
            case 0x0002:    /* Focus Info                             */
               if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_focusinfo(inptr,byteorder,tagprefix,value_offset,
                                    entry_ptr->count * sizeof(unsigned short),
                                                            indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                                            indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                break; 
            case 0x0004:    /* Shot Info                              */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count - 1);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count - 1);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_shotinfo(inptr,byteorder,tagprefix,value_offset,
                                    entry_ptr->count * sizeof(unsigned short),
                                                        indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                                            indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                break;
            case 0x0090:    /* Custom Functions                       */
            case 0x000F:    /* Custom Functions                       */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count - 1);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count - 1);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_customfunctions(inptr,byteorder,tagprefix,value_offset,
                                        entry_ptr->count * sizeof(unsigned short),
                                        model,indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                    indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                break;
            case 0x00E0:    /* Sensor Info                            */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count - 1);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count - 1);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_sensorinfo(inptr,byteorder,tagprefix,value_offset,
                                    entry_ptr->count * sizeof(unsigned short),
                                    indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                                            indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                break;
            case 0x0012:    /* Picture Info                           */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count - 1);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count - 1);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_pictureinfo(inptr,byteorder,tagprefix,value_offset,
                                    entry_ptr->count * sizeof(unsigned short),
                                    indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                        indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                break;
            case 0x0081:    /* JPEG image offset                      */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG image from MakerNote");
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                    0,summary_entry,fulldirname,
                                    prefix,indent+SMALLINDENT);
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if(max_offset == 0UL)
                            max_offset = filesize;
                        if(max_offset)
                        {
                            if((status = jpeg_status(0)) == JPEG_EARLY_EOI)
                                chpr = newline(chpr);
                            jpeg_status(status);
                            print_tag_address(VALUE_AT_OFFSET,max_offset - 1,indent,"-");
                            chpr += printf("# End of JPEG image from MakerNote");
                            chpr += printf(" length %lu",max_offset - value_offset); 
                        }
                        if(!(PRINT_VALUE) && (status == JPEG_NO_ERROR))
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
                        if((summary_entry->compression == JPEG_SOF_3) && (max_offset == 0))
                            summary_entry->length = filesize - value_offset;
                    }
                }
                clearerr(inptr);
                break;
            case 0x009a:    /* AspectInfo                      */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",entry_ptr->count - 1);
                    }
                }
                else if((PRINT_VALUE))
                    chpr += printf("\'%lu entries\'",entry_ptr->count - 1);
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                setcharsprinted(chpr);
                chpr = 0;
                (void)canon_aspectinfo(inptr,byteorder,tagprefix,value_offset,
                                    entry_ptr->count * sizeof(long),
                                    indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    print_tag_address(SECTION,value_offset + (entry_ptr->count * 2) - 1,
                                                            indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
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


/* Canon-specific tagnames for makernotes.                            */
/* There is currently only one tagset, but there are (at least) 4     */
/* different sets of custom functions.                                */

char *
maker_canon_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;
    int noteversion = 0;

    /* Should have to do this only once, and only for unrecognized    */
    /* models. If the model is recognized (or the user has forced a   */
    /* noteversion) noteversion will already be set.                  */
    if(((noteversion = getnoteversion()) == 0))
    {
        /* Assume the tagset is correct, but use the generic          */
        /* CustomFunctions routine if required (some models don't     */
        /* have custom functions)                                     */
        noteversion = 1;
        setnoteversion(1);
    }

    switch(noteversion)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            tagname = maker_canon1_tagname(tag,model);
            break;
        default:
            /* This essentially turns off tagname interpretation; it  */
            /* should happen only if the user has deliberately set a  */
            /* non-existent version (e.g. -1)                         */
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
maker_canon1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0001: tagname = "CameraSettings"; break;
        case 0x0002: tagname = "FocusInfo"; break;
        case 0x0004: tagname = "ShotInfo"; break;
        case 0x0006: tagname = "ImageType"; break;
        case 0x0007: tagname = "FirmwareVersion"; break;
        case 0x0008: tagname = "ImageNumber"; break;
        case 0x0009: tagname = "OwnerName"; break;
        case 0x000a: tagname = "ColorInfoD30*"; break;
        case 0x000c: tagname = "CameraSerialNo"; break;
        case 0x000d: tagname = "CameraInfo"; break;
        case 0x000e: tagname = "FileLength"; break;
        case 0x000f: tagname = "CustomFunctions"; break;
        case 0x0010: tagname = "UniqueModelId"; break; /* from dcraw */
        case 0x0012: tagname = "PictureInfo"; break;
        case 0x0013: tagname = "ThumbnailImageValidArea"; break;
        case 0x0081: tagname = "JpegImage"; break;
        case 0x0090: tagname = "CustomFunctions"; break;
        case 0x0093: tagname = "FileInfo**"; break;   /* exiftool shows wrong type */
        case 0x0095: tagname = "LensName"; break;
        case 0x0098: tagname = "CropInfo"; break;
        case 0x009a: tagname = "AspectInfo"; break;
        case 0x00a0: tagname = "ColorInfo**"; break;  /* exiftool - wrong # values */
        case 0x00a9: tagname = "WhiteBalanceTable"; break;
        case 0x00ae: tagname = "ColorTemperature"; break;
        case 0x00b6: tagname = "PreviewImageInfo"; break;
        case 0x00e0: tagname = "SensorInfo"; break;
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}

/* Report the "meaning" of tag values found directly in a Canon       */
/* MakerNote IFD entry (not at an offset).                            */

/* There are currently no direct entry tags for which the meaning is  */
/* known, so this routine is null, empty, void. There are observed    */
/* tags with unknown meaning, so this routine will be necessary.      */

/* I suspect that ImageNumber could be decomposed into a DCF folder   */
/* number and sequence number),                                       */

void
canon1_interpret_value(struct ifd_entry *entry_ptr)
{
    switch(entry_ptr->tag)
    {
        default: break;
    }
}

/* ###%%% FIXIT: none of the routines below honor the PRINT_TAGNAMES  */
/* or PRINT_VALUE options.                                            */

/* Interpret the "Camera Settings" tag data from Canon MakerNotes.    */

unsigned long
canon_camera_settings(FILE *inptr,unsigned short byteorder,char *tagprefix,
                    unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    struct fileheader *header = NULL;
    unsigned short length,value;
    char *sep;
    int i;
    int chpr = 0;


    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);
    offset += 2;
    count = (dirlength / sizeof(unsigned short)) - 1;

    if((PRINT_ENTRY))
    {
        for(i = 1; i <= count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            /* tagprefix has already been adjusted according to the   */
            /* setting of PRINT_LONGNAMES.                            */
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            switch(i)
            {
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"MacroMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 1: chpr += printf("macro"); break;
                            case 2: chpr += printf("normal"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"SelfTimer");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        if(value)
                            chpr += printf(".%u sec",value); /* ###%%% ?  */
                        else
                            chpr += printf("off");
                        print_endvalue();
                    }
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Quality");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 2: chpr += printf("normal"); break;
                            case 3: chpr += printf("fine"); break;
                            case 4: header = read_imageheader(inptr,0);
                                    switch(header->probe_magic)
                                    {
                                        case TIFF_MAGIC:
                                        case PROBE_JPEGMAGIC:
                                            chpr += printf("Lossless JPEG");
                                            break;
                                        case PROBE_CIFFMAGIC:
                                        default:
                                            chpr += printf("RAW");
                                            break;
                                    }
                                    break;
                            case 5: chpr += printf("super fine"); break;
                            default: printred("unknown"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FlashMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("flash did not fire"); break;
                            case 1: chpr += printf("auto"); break;
                            case 2: chpr += printf("on"); break;
                            case 3: chpr += printf("redeye"); break;
                            case 4: chpr += printf("slow sync"); break;
                            case 5: chpr += printf("auto + redeye"); break;
                            case 6: chpr += printf("on + redeye"); break;
                            case 16: chpr += printf("external flash"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ContinuousMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("single or timer"); break;
                            case 1: chpr += printf("continuous"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FocusMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("One Shot AF"); break;
                            case 1: chpr += printf("AI Servo AF"); break;
                            case 2: chpr += printf("AI Focus AF"); break;
                            case 3: chpr += printf("Manual Focus"); break;
                            case 4: chpr += printf("single"); break;
                            case 5: chpr += printf("Continuous"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ImageSize");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Large"); break;
                            case 1: chpr += printf("Medium"); break;
                            case 2: chpr += printf("Small"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"EasyShootMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Full Auto"); break;
                            case 1: chpr += printf("Manual"); break;
                            case 2: chpr += printf("Landscape"); break;
                            case 3: chpr += printf("Fast Shutter"); break;
                            case 4: chpr += printf("Slow Shutter"); break;
                            case 5: chpr += printf("Night"); break;
                            case 6: chpr += printf("B & W"); break;
                            case 7: chpr += printf("Sepia"); break;
                            case 8: chpr += printf("Portrait"); break;
                            case 9: chpr += printf("Sports"); break;
                            case 10: chpr += printf("Macro/CloseUp"); break;
                            case 11: chpr += printf("Pan Focus"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"DigitalZoom");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("none"); break;
                            case 1: chpr += printf("2x"); break;
                            case 2: chpr += printf("4x"); break;
                            default: printred("(2 * ZoomedResBase) / ZoomedResValue**"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Contrast");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Normal"); break;
                            case 1: chpr += printf("High"); break;
                            case 0xffff: chpr += printf("Low"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Saturation");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Normal"); break;
                            case 1: chpr += printf("High"); break;
                            case 0xffff: chpr += printf("Low"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Sharpness");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Normal"); break;
                            case 1: chpr += printf("High"); break;
                            case 0xffff: chpr += printf("Low"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 16:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ISO");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#x/%u",value,value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Use Exif ISOSpeedRating"); break;
                            case 15: chpr += printf("Auto"); break;
                            case 16: chpr += printf("50"); break;
                            case 17: chpr += printf("100"); break;
                            case 18: chpr += printf("200"); break;
                            case 19: chpr += printf("400"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 17:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"MeteringMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Default"); break;
                            case 1: chpr += printf("Spot"); break;
                            case 3: chpr += printf("Evaluative"); break;
                            case 4: chpr += printf("Partial"); break;
                            case 5: chpr += printf("Center-weighted"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 18:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FocusType");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Manual"); break;
                            case 1: chpr += printf("Auto1"); break;
                            case 2: chpr += printf("Auto2"); break;
                            case 3: chpr += printf("Macro"); break;
                            case 7: chpr += printf("Infinity"); break;
                            case 8: chpr += printf("Locked (pan mode)"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 19:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"AFPositionSelected");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#x",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0x2005: chpr += printf("Manual AF Point select"); break;
                            case 0x3000: chpr += printf("None (manual focus)"); break;
                            case 0x3001: chpr += printf("Auto-selected"); break;
                            case 0x3002: chpr += printf("Right"); break;
                            case 0x3003: chpr += printf("Center"); break;
                            case 0x3004: chpr += printf("Left"); break;
                            case 0x4001: chpr += printf("Auto AF Point select"); break;
                            default: printred("undefined");
                        }
                        print_endvalue();
                    }
                    break;
                case 20:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ExposureMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Easy Shooting"); break;
                            case 1: chpr += printf("Program AE"); break;
                            case 2: chpr += printf("Shutter priority AE"); break; 
                            case 3: chpr += printf("Aperture priority AE"); break; 
                            case 4: chpr += printf("Manual"); break;
                            case 5: chpr += printf("Depth of field AE"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 21:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 22:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"LensType");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    /* ###%% Phil Harvey gives a massive Lens type    */
                    /* table                                          */
                    break;
                case 23:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FocalLength_long");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u units",value);
                    break;
                case 24:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FocalLength_short");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u units",value);
                    break;
                case 25:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FocalUnits*");
                    if((PRINT_VALUE))
                    {
                        if(value > 1)
                            chpr += printf(" = %u units per mm*",value);
                        else
                            chpr += printf(" = %u unit per mm",value);
                    }
                    break;
                case 26:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 27:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 28:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FlashActivity");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#x",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("did not fire"); break;
                            case 1: chpr += printf("fired"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 29:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FlashDetails");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#x",value);
                        print_startvalue();
                        sep = "";
                        if(value & 0x0e)
                        {
                            chpr += printf("%sexternal E-TTL",sep);
                            sep = ",";
                        }
                        if(value & 0x0d)
                        {
                            chpr += printf("%sInternal flash**",sep);
                            sep = ",";
                        }
                        if(value & 0x0b)
                        {
                            chpr += printf("%sFP sync used",sep);
                            sep = ",";
                        }
                        if(value & 0x07)
                        {
                            chpr += printf("%s2nd-curtain sync used",sep);
                            sep = ",";
                        }
                        if(value & 0x04)
                        {
                            chpr += printf("%sFP sync enabled",sep);
                            sep = ",";
                        }
                        if(value & 0x03)
                        {
                            chpr += printf("%sOn",sep);
                            sep = ",";
                        }
                        print_endvalue();
                    }
                    break;
                case 30:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 31:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 32:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"FocusMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Single"); break;
                            case 1: chpr += printf("Continuous"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 36:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ZoomedResValue**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 37:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ZoomedResBase**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 42:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"ColorTone**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x/%u",value,value);
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-23.23s",i,"undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x/%u",value,value);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* Interpret the "ShotInfo" settings in a Cannon MakerNote or         */
/* CIFF file                                                          */

unsigned long
canon_shotinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;

        for(i = 1; i <= count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            switch(i)
            {
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"ISO");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 6: 
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"ExposureCompensation**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"WhiteBalance");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Auto"); break;
                            case 1: chpr += printf("Sunny"); break;
                            case 2: chpr += printf("Cloudy"); break;
                            case 3: chpr += printf("Tungsten"); break;
                            case 4: chpr += printf("Fluorescent"); break;
                            case 5: chpr += printf("Flash"); break;
                            case 6: chpr += printf("Custom"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"SequenceNumber");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"AFPositionUsed");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#x",value);
                        print_startvalue();
                        if(value & 0xf000)
                        {
                            switch(value & 0x7)
                            {
                                case 0: chpr += printf("none (MF)"); break;
                                case 1: chpr += printf("right"); break;
                                case 2: chpr += printf("center"); break;
                                case 3: chpr += printf("center+right"); break;
                                case 4: chpr += printf("left"); break;
                                case 5: chpr += printf("left+right"); break;
                                case 6: chpr += printf("left+center"); break;
                                case 7: chpr += printf("all"); break;
                                default: printred("undefined"); break;
                            }
                            chpr += printf(" of %d points",(value & 0xf000) >> 12);
                        }
                        else
                            chpr += printf("MF");
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"FlashBias");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#x",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0xffc0: chpr += printf("-2 EV"); break;
                            case 0xffcc: chpr += printf("-1.67 EV"); break;
                            case 0xffd0: chpr += printf("-1.50 EV"); break;
                            case 0xffd4: chpr += printf("-1.33 EV"); break;
                            case 0xffe0: chpr += printf("-1 EV"); break;
                            case 0xffec: chpr += printf("-0.67 EV"); break;
                            case 0xfff0: chpr += printf("-0.50 EV"); break;
                            case 0xfff4: chpr += printf("-0.33 EV"); break;
                            case 0x0000: chpr += printf("0 EV"); break;
                            case 0x000c: chpr += printf("+0.33 EV"); break;
                            case 0x0010: chpr += printf("+0.50 EV"); break;
                            case 0x0014: chpr += printf("+0.67 EV"); break;
                            case 0x0020: chpr += printf("+1 EV"); break;
                            case 0x002c: chpr += printf("+1.33 EV"); break;
                            case 0x0030: chpr += printf("+1.50 EV"); break;
                            case 0x0034: chpr += printf("+1.67 EV"); break;
                            case 0x0040: chpr += printf("+2 EV"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 16:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"AutoExposureBracketing**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 17:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"AEBracketValue**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 18:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
                case 19:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"FocusDistanceMax**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u mm",value);
                    break;
                case 20:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"FocusDistanceMin**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u mm",value);
                    break;
                case 21:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"ApertureValue**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 22:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"ExposureTime**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 24:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"BulbDuration**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 27:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"AutoRotate**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 29:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"SelfTimer2**");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-28.28s",i,"Undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* Interpret Canon Custom Functions                                   */
unsigned long
canon_customfunctions(FILE *inptr,unsigned short byteorder,
    char *tagprefix,unsigned long offset,
    unsigned long dirlength,int model,int indent)
{
    unsigned long max_offset;
    int noteversion = 0;

    noteversion = getnoteversion();

    switch(noteversion)
    {
    case 5:  /* 20 entries (or 21,22) */
        max_offset = canon_customfunctions1D(inptr,byteorder,tagprefix,offset,dirlength,indent);
        break;
    case 4:  /* 18 entries                                            */
        max_offset = canon_customfunctions20D(inptr,byteorder,tagprefix,offset,dirlength,indent);
        break;
    case 3:  /* 17 entries starting at 1*/
        max_offset = canon_customfunctions10D(inptr,byteorder,tagprefix,offset,dirlength,indent);
        break;
    case 2:  /* 16 entries                                            */
        max_offset = canon_customfunctionsD30(inptr,byteorder,tagprefix,offset,dirlength,indent);
        break;
    case 1:  /* unknown customfuctions                                */
    default:
        max_offset = canon_customfunctions_unk(inptr,byteorder,tagprefix,offset,dirlength,indent);
        break;
    }
    return(max_offset);
}

/* A "generic" version of customfunctions; just prints the data, with */
/* no interpretation.                                                 */

unsigned long
canon_customfunctions_unk(FILE *inptr,unsigned short byteorder,char *tagprefix,
                        unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i,cfn,cfv;
    int chpr = 0;

    length = read_ushort(inptr,byteorder,offset);
    end_offset = offset + dirlength;
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;
        for(i = 0; i < count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            chpr += printf("%s",tagprefix);
            offset += 2;
            cfn = (value & 0xff00) >> 8;
            cfv = value & 0xff;
            if(PRINT_SECTION)
                chpr += printf("Unk.%02d: = %#x/%u",cfn,cfv,cfv);
            else
                chpr += printf("Unk.%02d = %#x/%u",cfn,cfv,cfv);
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* CustomFunctions for Canon D30                                      */

unsigned long
canon_customfunctionsD30(FILE *inptr,unsigned short byteorder,char *tagprefix,
                        unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i,cfn,cfv;
    int chpr = 0;

    length = read_ushort(inptr,byteorder,offset);
    end_offset = offset + dirlength;
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;
        for(i = 0; i < count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            cfn = (value & 0xff00) >> 8;
            cfv = value & 0xff;
            switch(cfn)
            {
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d:%-30.30s",cfn,"LongExposureNoiseReduction");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Off"); break;
                            case 1: chpr += printf("On"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"ShutterAElockButtons");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF/AE lock"); break;
                            case 1: chpr += printf("AE lock/AF"); break;
                            case 2: chpr += printf("AF/AF lock"); break;
                            case 3: chpr += printf("AE+release/AE+AF"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"MirrorLockup");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disabled"); break;
                            case 1: chpr += printf("Enabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"TvAvAndExposureLevel");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1/2 stop"); break;
                            case 1: chpr += printf("1/3 stop"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"AFAssistLight");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("On (auto)"); break;
                            case 1: chpr += printf("Off"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"FlashSyncSpeedAv");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Auto"); break;
                            case 1: chpr += printf("D30/200 (fixed)"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"AEBSequenceAutoCancellation");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("0,-,+/Enabled"); break;
                            case 1: chpr += printf("0,-,+/Disabled"); break;
                            case 2: chpr += printf("-,0,+/Enabled"); break;
                            case 3: chpr += printf("-,0,+/Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"ShutterCurtainSync");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("D30-curtain sync"); break;
                            case 1: chpr += printf("2nd-curtain sync"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"LensAFStopButtonFnSwitch");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF Stop"); break;
                            case 1: chpr += printf("Operate AF"); break;
                            case 2: chpr += printf("Lock AE and start timer"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"AutoReductionOfFillFlash");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Enabled"); break;
                            case 1: chpr += printf("Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"MenuButtonReturnPosition");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Top"); break;
                            case 1: chpr += printf("Previous (volatile)"); break;
                            case 2: chpr += printf("Previous"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"SETButtonFuncWhenShooting");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Not assigned"); break;
                            case 1: chpr += printf("Change quality"); break;
                            case 2: chpr += printf("Change ISO speed"); break;
                            case 3: chpr += printf("PreviouSelect parameters"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"SensorCleaning");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disabled"); break;
                            case 1: chpr += printf("Enabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"SuperimposedDisplay*");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("On"); break;
                            case 1: chpr += printf("Off"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"ShutterReleaseNoCFCard*");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Yes"); break;
                            case 1: chpr += printf("No"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf("D30.%02d_%-30.30s",cfn,"undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x/%u",cfv,cfv);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* CustomFunctions for Canon 10D                                      */

unsigned long
canon_customfunctions10D(FILE *inptr,unsigned short byteorder,char *tagprefix,
                        unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i,cfn,cfv;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;

        for(i = 0; i < count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            cfn = (value & 0xff00) >> 8;
            cfv = value & 0xff;
            switch(cfn)
            {
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"Set Button Function");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Not assigned"); break;
                            case 1: chpr += printf("Change quality"); break;
                            case 2: chpr += printf("Change parameters"); break;
                            case 3: chpr += printf("Menu display"); break;
                            case 4: chpr += printf("Image replay"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"ShutterReleaseNoCFCard");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Yes"); break;
                            case 1: chpr += printf("No"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"FlashSyncSpeedAv");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Auto"); break;
                            case 1: chpr += printf("1/200 (fixed)"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"ShutterAElockButtons");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF/AE lock"); break;
                            case 1: chpr += printf("AE lock/AF"); break;
                            case 2: chpr += printf("AF/AF lock"); break;
                            case 3: chpr += printf("AE/AF"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"AFAssistLight");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Emits/Fires"); break;
                            case 1: chpr += printf("Does not emit/Fires"); break;
                            case 2: chpr += printf("Only ext flash emits/Fires"); break;
                            case 3: chpr += printf("Emits/Does not fire"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"ExposureLevelIncrements");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1/2 stop"); break;
                            case 1: chpr += printf("1/3 stop"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"AFPointRegistration");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Center"); break;
                            case 1: chpr += printf("Bottom"); break;
                            case 2: chpr += printf("Right"); break;
                            case 3: chpr += printf("Extreme Right"); break;
                            case 4: chpr += printf("Automatic"); break;
                            case 5: chpr += printf("Extreme Left"); break;
                            case 6: chpr += printf("Left"); break;
                            case 7: chpr += printf("Top"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"RawandJpegRecording");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("RAW+Small/Normal"); break;
                            case 1: chpr += printf("RAW+Small/Fine"); break;
                            case 2: chpr += printf("RAW+Medium/Normal"); break;
                            case 3: chpr += printf("RAW+Medium/Fine"); break;
                            case 4: chpr += printf("RAW+Large/Normal"); break;
                            case 5: chpr += printf("RAW+Large/Fine"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"AEBSequence");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("0,-,+/Enabled"); break;
                            case 1: chpr += printf("0,-,+/Disabled"); break;
                            case 2: chpr += printf("-,0,+/Enabled"); break;
                            case 3: chpr += printf("-,0,+/Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"SuperimposedDisplay");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("On"); break;
                            case 1: chpr += printf("Off"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"MenuButtonDisplayPosition");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Previous (volatile)"); break;
                            case 1: chpr += printf("Previous"); break;
                            case 2: chpr += printf("Top"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"MirrorLockup");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disabled"); break;
                            case 1: chpr += printf("Enabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"AssistButtonFunction");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Normal"); break;
                            case 1: chpr += printf("Select home position"); break;
                            case 2: chpr += printf("Select HP (while pressing)"); break;
                            case 3: chpr += printf("Ax+/-(AF point by QCD)"); break;
                            case 4: chpr += printf("FE lock"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"FillFlashAutoReduction");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Enabled"); break;
                            case 1: chpr += printf("Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"ShutterCurtainSync");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1st-curtain sync"); break;
                            case 1: chpr += printf("2nd-curtain sync"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 16:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"SafetyShiftInAvOrTv");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disable"); break;
                            case 1: chpr += printf("Enable"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 17:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"LensAFStopButton");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF Stop"); break;
                            case 1: chpr += printf("Operate AF"); break;
                            case 2: chpr += printf("Lock AE and start timer"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf("10D.%02d_%-30.30s",cfn,"undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x/%u",cfv,cfv);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* CustomFunctions for Canon 20D                                      */

unsigned long
canon_customfunctions20D(FILE *inptr,unsigned short byteorder,char *tagprefix,
                        unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i,cfn,cfv;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;

        for(i = 0; i < count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            cfn = (value & 0xff00) >> 8;
            cfv = value & 0xff;
            switch(cfn)
            {
                case 0:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"SetButtonFunctionWhenShooting");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("(no function)"); break;
                            case 1: chpr += printf("change quality"); break;
                            case 2: chpr += printf("change parameters"); break;
                            case 3: chpr += printf("menu display"); break;
                            case 4: chpr += printf("image replay"); break;
                            default: printred("undefined"); break;
                        }
                    }
                    print_endvalue();
                    break;
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"LongExposureNoiseReduction");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Off"); break;
                            case 1: chpr += printf("On"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"FlashSyncSpeedAv");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Auto"); break;
                            case 1: chpr += printf("1/200 (fixed)"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"ShutterAElockButtons");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF/AE lock"); break;
                            case 1: chpr += printf("AE lock/AF"); break;
                            case 2: chpr += printf("AF/AF lock"); break;
                            case 3: chpr += printf("AE+release/AE+AF"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"AFAssistLight");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Emits"); break;
                            case 1: chpr += printf("Does not emit"); break;
                            case 2: chpr += printf("Only ext flash emits"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"ExposureLevelIncrements");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1/2 stop"); break;
                            case 1: chpr += printf("1/3 stop"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"FlashFiring");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Fires"); break;
                            case 1: chpr += printf("Does not fire"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"ISOSpeedExpansion");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Off"); break;
                            case 1: chpr += printf("On"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"AEBSequence");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("0,-,+/Enabled"); break;
                            case 1: chpr += printf("0,-,+/Disabled"); break;
                            case 2: chpr += printf("-,0,+/Enabled"); break;
                            case 3: chpr += printf("-,0,+/Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"SuperimposedDisplay");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("On"); break;
                            case 1: chpr += printf("Off"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"MenuButtonDisplayPosition");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Top"); break;
                            case 1: chpr += printf("Previous (volatile)"); break;
                            case 2: chpr += printf("Previous"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"MirrorLockup");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disabled"); break;
                            case 1: chpr += printf("Enabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"AFPointSelectionMethod");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Normal"); break;
                            case 1: chpr += printf("Multi-controller direct"); break;
                            case 2: chpr += printf("Quick control dial direct"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"ETTLII");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Evaluative"); break;
                            case 1: chpr += printf("Average"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"ShutterCurtainSync");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1st-curtain sync"); break;
                            case 1: chpr += printf("2nd-curtain sync"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"SafetyShiftInAvOrTv");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disable"); break;
                            case 1: chpr += printf("Enable"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 16:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"LensAFStopButton");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF Stop"); break;
                            case 1: chpr += printf("Operate AF"); break;
                            case 2: chpr += printf("Lock AE and start timer"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 17:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"AddOriginalDecisionData");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Off"); break;
                            case 1: chpr += printf("On"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf("20D.%02d_%-30.30s",cfn,"undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x/%u",cfv,cfv);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* CustomFunctions for Canon 1D                                       */

unsigned long
canon_customfunctions1D(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i,cfn,cfv;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;

        for(i = 0; i < count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            chpr += printf("%s",tagprefix);
            offset += 2;
            cfn = (value & 0xff00) >> 8;
            cfv = value & 0xff;
            switch(cfn)
            {
                case 0:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"FocusingScreen");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Ec-N,R"); break;
                            case 1: chpr += printf("Ec-A,B,C,CII,CIII,D,H,I,L"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"FinderDisplayDuringExposure");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Off"); break;
                            case 1: chpr += printf("On"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"ShutterReleaseNoCFCard");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Yes"); break;
                            case 1: chpr += printf("No"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"ISOSpeedExpansion");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("No"); break;
                            case 1: chpr += printf("Yes"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"ShutterAElockButtons");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF/AE lock"); break;
                            case 1: chpr += printf("AE lock/AF"); break;
                            case 2: chpr += printf("AF/AF lock"); break;
                            case 3: chpr += printf("AE+release/AE+AF"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"ManualTv");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Tv:Main/Av:Control"); break;
                            case 1: chpr += printf("Tv:Control/Av:Main"); break;
                            case 2: chpr += printf("Tv:Main/Av:Main w/o lens"); break;
                            case 3: chpr += printf("Tv:Control/Av:Main w/o lens"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"ExposureLevelIncrements");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1/3-Stop set, 1/3-Stop comp"); break;
                            case 1: chpr += printf("1-Stop set, 1/3-Stop comp"); break;
                            case 2: chpr += printf("1/2-Stop set, 1/2-Stop comp"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"USMLensElectronicMF");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Turns on after one-shot AF"); break;
                            case 1: chpr += printf("Turns off after one-shot AF"); break;
                            case 2: chpr += printf("Always off"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"LCDPanels");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Remaining shots/File no"); break;
                            case 1: chpr += printf("ISO/Remaining shots"); break;
                            case 2: chpr += printf("ISO/File no"); break;
                            case 3: chpr += printf("Shots in folder/Remaining shots"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"AEBSequenceAutoCancellation");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("0,-,+/Enabled"); break;
                            case 1: chpr += printf("0,-,+/Disabled"); break;
                            case 2: chpr += printf("-,0,+/Enabled"); break;
                            case 3: chpr += printf("-,0,+/Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"AFPointIllumination");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("On"); break;
                            case 1: chpr += printf("Off"); break;
                            case 2: chpr += printf("On without dimming"); break;
                            case 3: chpr += printf("Brighter"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"AFPointSelection");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("H:AF+Main, V:AF+Command"); break;
                            case 1: chpr += printf("H:Comp+Main, V:AF+Comp+Command"); break;
                            case 2: chpr += printf("H:Command only, V:Assist+Main"); break;
                            case 3: chpr += printf("H:FEL_Main, V:FEL+Command"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"MirrorLockup");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disabled"); break;
                            case 1: chpr += printf("Enabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"AFPointSpotMetering");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("45/Center of Point"); break;
                            case 1: chpr += printf("11/Active AF point"); break;
                            case 2: chpr += printf("11/Center AF point"); break;
                            case 3: chpr += printf("9/Active af pOINT"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"FillFlashAutoReduction");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Enabled"); break;
                            case 1: chpr += printf("Disabled"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"ShutterCurtainSync");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("1st-curtain sync"); break;
                            case 1: chpr += printf("2nd-curtain sync"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 16:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"SafetyShiftIn");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Disable"); break;
                            case 1: chpr += printf("Enable"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 17:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"AF Point Activation Area");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Single AF point"); break;
                            case 1: chpr += printf("Expanded (7 AF points)"); break;
                            case 2: chpr += printf("Auto expanded (max 13)"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 18:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"SwitchToRegisteredAFPoint");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF + assist"); break;
                            case 1: chpr += printf("Assist"); break;
                            case 2: chpr += printf("Only while pressing Assist"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 19:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"Lens AF Stop Button");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("AF Stop"); break;
                            case 1: chpr += printf("AF Start"); break;
                            case 2: chpr += printf("AE Lock whuile metering"); break;
                            case 3: chpr += printf("AF Point: M > Auto, Auto > Center"); break;
                            case 4: chpr += printf("AF Mode: oneshot/servo"); break;
                            case 5: chpr += printf("IS Start"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 20:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"AIServoTrackingSensitivity");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %u",cfv);
                        print_startvalue();
                        switch(cfv)
                        {
                            case 0: chpr += printf("Standard"); break;
                            case 1: chpr += printf("Slow"); break;
                            case 2: chpr += printf("Moderately slow"); break;
                            case 3: chpr += printf("Moderately fast"); break;
                            case 4: chpr += printf("Fast"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf("1D.%02d_%-30.30s",cfn,"undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x/%u",cfv,cfv);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

unsigned long
canon_sensorinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                        unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length,value;
    int i;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;

        for(i = 1; i <= count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            switch(i)
            {
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"SensorWidth");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"SensorHeight");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"SensorLeftBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"SensorTopBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"SensorRightBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"SensorBottomBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"BlackMaskLeftBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"BlackMaskTopBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"BlackMaskRightBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"BlackMaskBottomBorder");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"Undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}


unsigned long
canon_aspectinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                        unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned long value;
    int i;
    int chpr = 0;

    end_offset = offset + dirlength;
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        count = (dirlength / sizeof(long));

        for(i = 0; i <= count; ++i)
        {
            value = read_ulong(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 4;
            switch(i)
            {
                case 0:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"AspectRatio");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#lx",value);

                        print_startvalue();
                        switch(value)
                        {
                        case 0:
                            chpr += printf("3:2");
                            break;
                        case 1:
                            chpr += printf("1:1");
                            break;
                        case 2:
                            chpr += printf("4:3");
                            break;
                        case 7:
                            chpr += printf("16:9");
                            break;
                        case 8:
                            chpr += printf("4:5");
                            break;
                        default:
                            printred("undefined");
                            break;
                        }
                        print_endvalue();

                    break;
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"CroppedImageWidth");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"CroppedImageHeight");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"CroppedImageLeft");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"CroppedImageRight");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-26.26s",i,"Undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}


/* not a directory; an array of shorts used as rationals              */
unsigned long
canon_whitebalancetable(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short length;
    unsigned short num,denom;
    float value;
    int i;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
        chpr = newline(chpr);
    }

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength - 2) / sizeof(float);

        for(i = 0; i < count; ++i)
        {
            num = read_ushort(inptr,byteorder,offset);
            denom = read_ushort(inptr,byteorder,offset+2);
            value = (float)num / (float)denom;
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            switch(i)
            {
                case 0:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceAuto");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 1:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceAuto");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 2:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceDaylight");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 3:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceDaylight");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 4:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceCloudy");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 5:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceCloudy");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 6:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceTungsten");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 7:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceTungsten");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 8:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceFluorescent");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 9:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceFluorescent");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 10:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceFlash");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 11:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceFlash");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 12:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceCustom");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 13:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceCustom");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 14:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceB&W");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 15:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceB&W");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 16:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"RedBalanceShade");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                case 17:   
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"BlueBalanceShade");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"Undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %f",value);
                    break;
            }
            offset += 4;
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* pictureinfo appears to be treated as a directory, except that the  */
/* 'length' which should appear at the beginning of a ciff-style      */
/* directory often does not agree with the length passed from the     */
/* parent.                                                            */

unsigned long
canon_pictureinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short value,length;
    int i;
    int chpr = 0;

    end_offset = offset + dirlength;
    length = read_ushort(inptr,byteorder,offset);
    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        if(length != dirlength)
        {
            chpr = newline(chpr);
            PUSHCOLOR(RED);
            print_tag_address(SECTION,offset,indent,"@");
            chpr += printf("# WARNING: length %u from segment does not match length %lu from directory",
                        length,dirlength);
            POPCOLOR();
        }
    }
    chpr = newline(chpr);

    if((PRINT_ENTRY))
    {
        offset += 2;
        count = (dirlength / sizeof(unsigned short)) - 1;

        for(i = 1; i <= count; ++i)
        {
            value = read_ushort(inptr,byteorder,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
            offset += 2;
            switch(i)
            {
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"ImageWidth");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"ImageHeight");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"ImageWidthAsShot");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"ImageHeightAsShot");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                case 22:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"AFPointsUsed");
                    if((PRINT_VALUE))
                        chpr += printf(" = %u",value);
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf(".%02d_%-22.22s",i,"Undefined");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#x%u",value,value);
                    break;
            }
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* Currently identified only in CIFF files                            */

unsigned long
canon_imageinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent,
                            struct image_summary *summary_entry)
{
    unsigned long count,value;
    unsigned long end_offset;
    float fvalue;
    int i;
    int chpr = 0;

    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        chpr = newline(chpr);
    }

    /* NOTE that imageinfo is not treated as a directory; there is no */
    /* 2-byte count at the beginning.                                 */

    end_offset = offset + dirlength;
    count = (dirlength / sizeof(unsigned long));

    /* Must always do the processing, in order to pick up the image   */
    /* width and height for the summary, even if nothing is to be     */
    /* printed                                                        */
    for(i = 0; i < count; ++i)
    {
        value = read_ulong(inptr,byteorder,offset);
        print_tag_address(ENTRY,offset,indent + MEDIUMINDENT,"@");
        if((PRINT_ENTRY))
        {
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
        }
        offset += 4;
        switch(i)
        {
            case 0:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"ImageWidth");
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",value);
                if(summary_entry)
                    summary_entry->pixel_width = value;
                break;
            case 1:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"ImageHeight");
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",value);
                if(summary_entry)
                    summary_entry->pixel_height = value;
                break;
            case 2:
                fvalue = to_float(value);
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"PixelAspectRatio");
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",value);
                break;
            case 3:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"Rotation");
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",value);
                break;
            case 4:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"ComponentBitDepth");
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",value);
                break;
            case 5:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"ColorBitDepth");
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",value);
                break;
            case 6:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"ColorBW");
                if((PRINT_VALUE))
                {
                    chpr += printf(" = %#lx/%lu",value,value);
                    if((value & 0xff) == 1)
                         chpr += printf(" = color");
                    else
                         chpr += printf(" = greyscale");
                    if(((value & 0xff00) >> 8) == 1)
                         chpr += printf(" = \'pixel ratio postproc reqd\'");
                }
                break;
            default:
                if((PRINT_TAGINFO))
                     chpr += printf(".%02d_%-22.22s",i,"Undefined");
                if((PRINT_VALUE))
                    chpr += printf(" = %#lx/%lu",value,value);
                break;
        }
        chpr = newline(chpr);
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* Currently identified only in CIFF files                            */

unsigned long
canon_exposureinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent,
                            struct image_summary *summary_entry)
{
    unsigned long count,value;
    unsigned long end_offset;
    float fvalue;
    int i;
    int chpr = 0;

    if(PRINT_SECTION)
    {
        chpr += printf(" length %lu",dirlength);
        chpr = newline(chpr);
    }

    /* NOTE that exposureinfo is not treated as a directory; there is */
    /* no 2-byte count at the beginning.                              */

    end_offset = offset + dirlength;
    count = (dirlength / sizeof(unsigned long));

    for(i = 0; i < count; ++i)
    {
        value = read_ulong(inptr,byteorder,offset);
        print_tag_address(ENTRY,offset,indent,"@");
        if((PRINT_ENTRY))
        {
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
        }
        offset += 4;
        switch(i)
        {
            case 0:
                fvalue = to_float(value);
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"ExposureCompensation");
                if((PRINT_VALUE))
                    chpr += printf(" = %f",fvalue);
                break;
            case 1:
                fvalue = to_float(value);
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"Tv");
                if((PRINT_VALUE))
                    chpr += printf(" = %f",fvalue);
                break;
            case 2:
                fvalue = to_float(value);
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"Av");
                if((PRINT_VALUE))
                    chpr += printf(" = %f",fvalue);
                break;
            default:
                fvalue = to_float(value);
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"Undefined");
                if((PRINT_VALUE))
                    chpr += printf(" = %f",fvalue);
                break;
        }
        chpr = newline(chpr);
    }
    setcharsprinted(chpr);
    return(end_offset);
}


unsigned long
canon_focusinfo(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count;
    unsigned long end_offset;
    unsigned short value;
    int i;
    int chpr = 0;

    if(PRINT_SECTION)
        chpr += printf(" length %lu",dirlength);
    chpr = newline(chpr);

    /* NOTE that focallength is not treated as a directory; there is  */
    /* no 2-byte count at the beginning.                              */

    end_offset = offset + dirlength;
    count = (dirlength / sizeof(unsigned short));

    for(i = 0; i < count; ++i)
    {
        value = read_ushort(inptr,byteorder,offset);
        print_tag_address(ENTRY,offset,indent,"@");
        if((PRINT_ENTRY))
        {
            if((PRINT_TAGINFO))
                chpr += printf("%s",tagprefix);
        }
        offset += 2;
        switch(i)
        {
            case 0:
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"unknown");
                if((PRINT_VALUE))
                    chpr += printf(" = %u",value);
                break;
            case 1:
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"FocalLength");
                if((PRINT_VALUE))
                    chpr += printf(" = %u",value);
                break;
            case 2:
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"FocalPlaneXSize");
                if((PRINT_VALUE))
                    chpr += printf(" = %u",value);
                break;
            case 3:
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"FocalPlaneYSize");
                if((PRINT_VALUE))
                    chpr += printf(" = %u",value);
                break;
            default:
                if((PRINT_TAGINFO))
                    chpr += printf(".%02d_%-22.22s",i,"Undefined");
                if((PRINT_VALUE))
                    chpr += printf(" = %#x%u",value,value);
                break;
        }
        chpr = newline(chpr);
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* Some makernotes record offsets as LONG values rather than simply   */
/* using UNDEFINED with a size; the same technique used for           */
/* the ExifIFDPointer itself, or JpegInterchangeFormat, etc.          */
/* This routine hardcodes such tags so that the print_makerentry()    */
/* routine will present them as offsets.                              */

int
maker_canon_value_is_offset(struct ifd_entry *entry_ptr,int model)
{
    int is_offset = 0;
    int noteversion = 0;

    if(entry_ptr)
    {
        noteversion = getnoteversion();
        switch(noteversion)
        {
            default:
                switch(entry_ptr->tag)
                {
                    case 0x0081: is_offset = 1; break;
                    default: break;
                }
                break;
        }
    }
    return(is_offset);
}

/*
<0xc5d9=50649> = 2 compression=6 CR2 RGB (Ifd 2)

<0xc5d8=50648> = 0 compression=6  primary? (biggest)        CR2 Ifd 3 
<0xc5e0=50656> = 1                                          CR2 Ifd 3 
<0xc640-50752> = offset to 3 shorts (EOS 350D)              CR2 Ifd 3
<0xc640-50752> = offset to 3 shorts (EOS-1Ds Mark II)       CR2 Ifd 3  2,1680,1748 (e0=3)
*/
