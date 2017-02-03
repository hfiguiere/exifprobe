/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_panasonic.c,v 1.5 2005/07/24 22:56:27 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* panasonic camera maker-specific routines                           */
/*   panasonic support contributed by Tom Hughes <tom@compton.nu>     */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* The information coded here is derived from experimentation with    */
/* a Panasonic DMC-FZ10 and from looking at a few images from other   */
/* cameras found on various web sites.                                */
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

extern struct camera_id panasonic_model_id[];

/* Find the identifying number assigned to known Panasonic camera     */
/* models. This number is used to dispatch print and interpret        */
/* routines approopriate to the current image.                        */

int
panasonic_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &panasonic_model_id[0]; model_id && model_id->name; ++model_id)
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


/* Dispatch a print routine for direct values in Panasonic cameras,   */
/* based upon model                                                   */

void
print_panasonic_makervalue(struct ifd_entry *entry_ptr,int make,int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1:
                print_panasonic1_makervalue(entry_ptr,make,model,prefix);
                panasonic1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Panasonic cameras. This routine   */
/* is responsible for picking off any direct entry tags which are     */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_panasonic1_makervalue(struct ifd_entry *entry_ptr,int make,int model,
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


/* Dispatch a routine to decode and print offset values for Panasonic */
/* cameras.                                                           */

void
print_panasonic_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char *prefix,
    int indent,int make,int model,int at_offset)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 1:
                panasonic1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);

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
/* in Panasonic makernotes.                                           */

void
panasonic1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char *prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset,count;
    char *nameoftag;
    char *fulldirname = NULL;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model);
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
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
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,parent_name,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}


/* Panasonic-specific tagnames for makernotes.                        */

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_panasonic_tagname(unsigned short tag,int model)
{
    char *tagname = (char *)0;
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
        case 1:
            tagname = maker_panasonic1_tagname(tag,model);
            break;
        default:
            break;
    }
    return(tagname);
}

char *
maker_panasonic1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0001: tagname = "Quality"; break;
        case 0x0002: tagname = "FirmwareVersion"; break;
        case 0x0003: tagname = "WhiteBalance"; break; 
        case 0x0007: tagname = "FocusMode"; break;
        case 0x000f: tagname = "SpotMode"; break;
        case 0x001a: tagname = "StabilizerMode"; break;
        case 0x001c: tagname = "Macro"; break;
        case 0x001f: tagname = "PictureMode"; break;
        case 0x0020: tagname = "Audio"; break;
        case 0x0023: tagname = "WhiteBalanceAdjust"; break;
        case 0x0024: tagname = "FlashStrength"; break;
        case 0x0028: tagname = "ColorMode"; break;
        case 0x0029: tagname = "TimeSincePowerOn"; break;
        case 0x002c: tagname = "Contrast"; break;
        case 0x002d: tagname = "NoiseReduction"; break;
        case 0x0e00: tagname = "PrintIM"; break;
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}


/* Report the "meaning" of tag values found at offsets in a Panasonic */
/* MakerNote IFD entry (not at an offset).                            */

void
panasonic1_interpret_value(struct ifd_entry *entry_ptr)
{
    switch(entry_ptr->tag)
    {
        case 0x0001:    /* Quality */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 2: printf("Fine"); break;
                case 3: printf("Standard"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x0003:    /* White Balance */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1: printf("Auto"); break;
                case 2: printf("Daylight"); break;
                case 3: printf("Cloudy"); break;
                case 4: printf("Halogen"); break;
                case 5: printf("Manual"); break;
                case 8: printf("Flash"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x0007:    /* Focus Mode */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1: printf("Auto"); break;
                case 2: printf("Manual"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x000f:    /* Spot Mode */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1: printf("Spot"); break;
                case 16: printf("1 Area"); break;
                case 256: printf("9 Area/On"); break;
                case 4096: printf("3 Area/Off"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x001a:    /* Stabilizer Mode */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 2: printf("Mode 1"); break;
                case 3: printf("Off"); break;
                case 4: printf("Mode 2"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x001c:    /* Macro */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1: printf("On"); break;
                case 2: printf("Off"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x001f:    /* Picture Mode */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 2: printf("Portrait"); break;
                case 3: printf("Scenery"); break;
                case 4: printf("Sports"); break;
                case 5: printf("Night Portrait"); break;
                case 6: printf("Program"); break;
                case 7: printf("Aperture Priority"); break;
                case 8: printf("Shutter Priority"); break;
                case 9: printf("Macro"); break;
                case 11: printf("Manual"); break;
                case 13: printf("Panning"); break;
                case 18: printf("Fireworks"); break;
                case 19: printf("Party"); break;
                case 20: printf("Snow"); break;
                case 21: printf("Night Scenery"); break;
                default: printred("undefined");
            }
            print_endvalue();
            break;
        case 0x0020:    /* Audio */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1: printf("Yes"); break;
                case 2: printf("No"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x0023:    /* White Balance Adjust */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 0xfffb: printf("-5"); break;
                case 0xfffc: printf("-4"); break;
                case 0xfffd: printf("-3"); break;
                case 0xfffe: printf("-2"); break;
                case 0xffff: printf("-1"); break;
                case 0x0000: printf("0"); break;
                case 0x0001: printf("+1"); break;
                case 0x0002: printf("+2"); break;
                case 0x0003: printf("+3"); break;
                case 0x0004: printf("+4"); break;
                case 0x0005: printf("+5"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x0024:    /* Flash Strength */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 0xfffa: printf("-2 EV"); break;
                case 0xfffb: printf("-1.67 EV"); break;
                case 0xfffc: printf("-1.33 EV"); break;
                case 0xfffd: printf("-1 EV"); break;
                case 0xfffe: printf("-0.67 EV"); break;
                case 0xffff: printf("-0.33 EV"); break;
                case 0x0000: printf("0 EV"); break;
                case 0x0001: printf("+0.33 EV"); break;
                case 0x0002: printf("+0.67 EV"); break;
                case 0x0003: printf("+1 EV"); break;
                case 0x0004: printf("+1.33 EV"); break;
                case 0x0005: printf("+1.67 EV"); break;
                case 0x0006: printf("+2 EV"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x0028:    /* Color Mode */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1: printf("Off"); break;
                case 2: printf("Warm"); break;
                case 3: printf("Cool"); break;
                case 4: printf("Black/White"); break;
                case 5: printf("Sepia"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x002c:    /* Contrast */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 0: printf("Standard"); break;
                case 1: printf("Low"); break;
                case 2: printf("High"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x002d:    /* Noise Reduction */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 0: printf("Standard"); break;
                case 1: printf("Low"); break;
                case 2: printf("High"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        default:
            break;
    }
}
