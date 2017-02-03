/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_fujifilm.c,v 1.13 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Fujitsu camera maker-specific routines (Fujifilm)                  */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Most of the information coded here is due to TsuruZoh Tachibanaya  */
/* at:                                                                */
/* http://www.ba.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "defs.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

extern struct camera_id fujifilm_model_id[];

/* Find the identifying number assigned to known Fujifilm camera      */
/* models. This number is used to dispatch print and interpret        */
/* routines approopriate to the current image.                        */

int
fujifilm_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &fujifilm_model_id[0]; model_id && model_id->name; ++model_id)
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


/* Dispatch a print routine for direct values in Fujifilm cameras,    */
/* based upon model                                                   */

void
print_fujifilm_makervalue(struct ifd_entry *entry_ptr,int make,int model,
                    char *prefix)
{
    int noteversion = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        noteversion = getnoteversion();
        switch(noteversion)
        {
            case 1:
                print_fujifilm1_makervalue(entry_ptr,make,model,prefix);
                fujifilm1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Fujifilm cameras. This routine is */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_fujifilm1_makervalue(struct ifd_entry *entry_ptr,int make,int model,
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


/* Dispatch a routine to decode and print offset values for Fujifilm  */
/* cameras.                                                           */

void
print_fujifilm_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char *prefix,
    int indent,int make,int model,int at_offset)
{
    int noteversion = 0;

    if(entry_ptr)
    {
        noteversion = getnoteversion();
        switch(noteversion)
        {
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                fileoffset_base,parent_name,prefix,indent,
                                make,model,at_offset);
                break;
        }
    }
}



/* Fujifilm-specific tagnames for makernotes.                              */

char *
maker_fujifilm_tagname(unsigned short tag,int model)
{
    char *tagname = (char *)0;
    int noteversion = 0;

    /* Should have to do this only once, and only for unrecognized    */
    /* models. If the model is recognized (or the user has forced a   */
    /* noteversion) noteversion will already be set.                  */
    if((noteversion = getnoteversion()) == 0)
    {
        /* Only one tagset has been observed so far; it seems         */
        /* reasonable to default to that tagset (rather than none).   */
        noteversion = 1;
        setnoteversion(1);
    }

    switch(noteversion)
    {
        case 1:
            tagname = maker_fujifilm1_tagname(tag,model);
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
maker_fujifilm1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0000: tagname = "Version"; break;
        case 0x0010: tagname = "UniqueSerial"; break; /* Exiv2 */
        case 0x1000: tagname = "Quality"; break;
        case 0x1001: tagname = "Sharpness"; break;
        case 0x1002: tagname = "WhiteBalance"; break;
        case 0x1003: tagname = "Color"; break;
        case 0x1004: tagname = "Tone"; break;
        case 0x1010: tagname = "FlashMode"; break;
        case 0x1011: tagname = "FlashStrength"; break;
        case 0x1020: tagname = "Macro"; break;
        case 0x1021: tagname = "FocusMode"; break;
        case 0x1030: tagname = "SlowSync"; break;
        case 0x1031: tagname = "PictureMode"; break;
        case 0x1100: tagname = "ContTake-Bracket"; break;
        case 0x1300: tagname = "BlurWarning"; break;
        case 0x1301: tagname = "FocusWarning"; break;
        case 0x1302: tagname = "AEWarning"; break;

        case 0x1401: tagname = "FilmMode"; break; /* Exiv2 */
        case 0x1402: tagname = "DynamicRangeSettings"; break; /* Exiv2 */
        case 0x1403: tagname = "DevelopmentDynamicRange"; break; /* Exiv2 */

        case 0x1404: tagname = "FocalLengthMin"; break;
        case 0x1405: tagname = "FocalLengthMax"; break;
        case 0x1406: tagname = "MaxApertureAtMinFocal"; break;
        case 0x1407: tagname = "MaxApertureAtMaxFocal"; break;
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}


/* Report the "meaning" of tag values found at offsets in a Fujifilm  */
/* MakerNote IFD entry (not at an offset).                            */

void
fujifilm1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x1001:    /* Sharpness */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Soft"); break;
                    case 2: chpr += printf("Soft"); break;
                    case 3: chpr += printf("Normal"); break;
                    case 4: chpr += printf("Hard"); break;
                    case 5: chpr += printf("Hard"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1002:    /* White Balance */
                print_startvalue();
                /* I think we're looking at a bit mask here, but I have   */
                /* no way to check; this is the way Tachibanaya           */
                /* characterizes it.                                      */
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 256: chpr += printf("Daylight"); break;
                    case 512: chpr += printf("Cloudy"); break;
                    case 768:   /* ???? */ printf("DaylightColor-Fluorescent"); break;
                    case 769:   /* ???? */ printf("DaywhiteColor-Fluorescent"); break;
                    case 770:   /* ???? */ printf("White-Fluorescent"); break;
                    case 1024: chpr += printf("Incandescent"); break;
                    case 3840: chpr += printf("Custom"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1003:    /* Color Saturation */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Standard"); break;
                    case 256: chpr += printf("High"); break;
                    case 512: chpr += printf("Low"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1004:    /* Tone */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Standard"); break;
                    case 256: chpr += printf("High"); break;
                    case 512: chpr += printf("Low"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1010:    /* Flash Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("On"); break;
                    case 2: chpr += printf("Off"); break;
                    case 3: chpr += printf("Redeye"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1020:    /* Macro Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1021:    /* Focusing Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Manual"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1030:    /* Slow Sync */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1031:    /* Picture Mode */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Auto"); break;
                    case 1: chpr += printf("Portrait"); break;
                    case 2: chpr += printf("Landscape"); break;
                    case 4: chpr += printf("Sports"); break;
                    case 5: chpr += printf("Night"); break;
                    case 6: chpr += printf("Program Normal"); break;
                    case 256: chpr += printf("Aperture Priority"); break;
                    case 512: chpr += printf("Shutter Priority"); break;
                    case 768: chpr += printf("Manual"); break;
                    default: printred("undefined"); }
                print_endvalue();
                break;
            case 0x1100:    /* Continuous Take/Bracket */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1300:    /* Blur Warning */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Ok"); break;
                    case 1: chpr += printf("Blurred"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1301:    /* Focus Warning */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Ok"); break;
                    case 1: chpr += printf("Out of focus"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x1302:    /* AE Warning */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Ok"); break;
                    case 1: chpr += printf("Overexposed"); break;
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
