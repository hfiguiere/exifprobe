/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_toshiba.c,v 1.4 2005/07/24 22:56:27 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Toshiba camera maker-specific routines                             */
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

extern struct camera_id toshiba_model_id[];

/* Find the identifying number assigned to known Toshiba camera       */
/* models. This number is used to dispatch print and interpret        */
/* routines approopriate to the current image.                        */

int
toshiba_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &toshiba_model_id[0]; model_id && model_id->name; ++model_id)
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


/* Dispatch a print routine for direct values in Toshiba cameras,     */
/* based upon model                                                   */

void
print_toshiba_makervalue(struct ifd_entry *entry_ptr,int make,int model,
                    char *prefix)
{
    int noteversion = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        noteversion = getnoteversion();
        switch(noteversion)
        {
            default:
                print_toshiba1_makervalue(entry_ptr,make,model,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Toshiba cameras. This routine is  */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_toshiba1_makervalue(struct ifd_entry *entry_ptr,int make,int model,
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


/* Dispatch a routine to decode and print offset values for Toshiba   */
/* cameras.                                                           */

void
print_toshiba_offset_makervalue(FILE *inptr,unsigned short byteorder,
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
            case 1:
                toshiba1_offset_makervalue(inptr,byteorder,entry_ptr,
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
/* in Toshiba makernotes.                                             */

void
toshiba1_offset_makervalue(FILE *inptr,unsigned short byteorder,
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
                                        fileoffset_base,fulldirname,prefix,
                                        indent,make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}


/* toshiba-specific tagnames for makernotes.                          */

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_toshiba_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

#if 0
    int noteversion = 0;

    if((noteversion = getnoteversion()) == 0)
    {
        noteversion = 1;
        setnoteversion(1);
    }

    switch(noteversion)
    {
        case 1:
            tagname = maker_toshiba1_tagname(tag,model);
        default:
            break;
    }
#endif

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
