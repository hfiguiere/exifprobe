/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_sigma.c,v 1.4 2005/07/24 22:56:27 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Sigma camera maker-specific routines.                              */
/* Tag information from;                                              */
/*   http://www.x3f.info/technotes/FileDocs/MakerNoteDoc.html         */
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

extern struct camera_id sigma_model_id[];

/* Find the id number assigned to known Sigma camera models.          */
/* This number is used to dispatch print and interpret routines       */
/* apporopriate to the current image.                                 */

int
sigma_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &sigma_model_id[0]; model_id && model_id->name; ++model_id)
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
print_sigma_makervalue(struct ifd_entry *entry_ptr,int make, int model,
					char *prefix)
{
    int noteversion = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        noteversion = getnoteversion();
        switch(noteversion)
        {
            case 1:
                print_sigma1_makervalue(entry_ptr,make,model,prefix);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for sigma cameras. This routine is    */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_sigma1_makervalue(struct ifd_entry *entry_ptr,int make, int model,
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

/* Dispatch a routine to decode and print offset values for sigma     */
/* cameras.                                                           */

void
print_sigma_offset_makervalue(FILE *inptr,unsigned short byteorder,
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
                sigma1_offset_makervalue(inptr,byteorder,entry_ptr,
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

/* Model-specific routine to print values found at an offset from the */
/* IFD entry in sigma makernotes.                                     */

void
sigma1_offset_makervalue(FILE *inptr,unsigned short byteorder,
	struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char *prefix,
    int indent,int make,int model,int at_offset)
{
	char *nameoftag;
    char *fulldirname = NULL;

	if(entry_ptr)
	{
		nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);

		switch(entry_ptr->tag)
		{
			default:
				print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
										fileoffset_base,fulldirname,prefix,indent,
										make,model,at_offset);
				break;
		}
        if(fulldirname)
            free(fulldirname);
	}
}

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_sigma_tagname(unsigned short tag,int model)
{
	char *tagname = CNULL;
    int noteversion = 0;

    if((noteversion = getnoteversion()) == 0)
    {
        /* Need more model samples before it is safe to assume a      */
        /* default                                                    */
        noteversion = 1;
        setnoteversion(1);
    }

	switch(noteversion)
	{
		case 1:
			tagname = maker_sigma1_tagname(tag,model);
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
maker_sigma1_tagname(unsigned short tag,int model)
{
	char *tagname = CNULL;

	switch(tag)
	{
        case 0x0002: tagname = "CameraSerialNumber"; break;
        case 0x0003: tagname = "DriveMode"; break;
        case 0x0004: tagname = "ResolutionMode"; break;
        case 0x0005: tagname = "AutofocusMode"; break;
        case 0x0006: tagname = "FocusSetting"; break;
        case 0x0007: tagname = "WhiteBalance"; break;
        case 0x0008: tagname = "ExposureMode"; break;
        case 0x0009: tagname = "MeteringMode"; break;
        case 0x000A: tagname = "LensFocalLengthRange"; break;
        case 0x000B: tagname = "ColorSpace"; break;
        case 0x000C: tagname = "Exposure"; break;
        case 0x000D: tagname = "Contrast"; break;
        case 0x000E: tagname = "Shadow"; break;
        case 0x000F: tagname = "Highlight"; break;
        case 0x0010: tagname = "Saturation"; break;
        case 0x0011: tagname = "Sharpness"; break;
        case 0x0012: tagname = "X3FillLight"; break;
        case 0x0014: tagname = "ColorAdjustment"; break;
        case 0x0015: tagname = "AdjustmentMode"; break;
        case 0x0016: tagname = "Quality"; break;
        case 0x0017: tagname = "Firmware"; break;
        case 0x0018: tagname = "Software"; break;
        case 0x0019: tagname = "AutoBracket"; break;
			default: break;
	}
	return(tagname);
}
