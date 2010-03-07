/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_tagnames.c,v 1.14 2005/07/24 21:32:11 alex Exp $";
#endif

/* This file contains routines to produce tag names for tags found in */
/* EXIF image MakerNotes on a make/model basis. The "master" routine, */
/* maker_tagname() dispatches to maker-specific routines, which may   */
/* then examine model magic number (maker.h). Those maker-specific    */
/* routines are arranged to check first for model-specific tags; if   */
/* the model-specific routine does not exist, or does not name the    */
/* current tag, the caller (maker-specific routine) may examine a     */
/* "generic" list of tags for that maker. Finally, if the             */
/* maker-specific routine finds nothing, the top-level routine        */
/* (maker_tagname()) produces and returns a string which is the hex   */
/* value of the tag, constructed in static storage.                   */

/* Note that only "known" tags are decoded; tags marked "unknnown" in */
/* the various references used are unlisted in the maker_specific     */
/* routines, so that the tag number will print.                       */

#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "maker.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "summary.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

/* The "master" tagname routine; dispatches to maker-specific         */
/* routines. If no maker-specific routine is supplied, a name (hex    */
/* value of the tag) will be constructed in static storage. This      */
/* routine intends to always return a non-null pointer;               */

char *
maker_tagname(unsigned short tag,int make,int model)
{
    static char unknown_buf[16];
    char *tagname = NULL;

    switch(make)
    {
        case MAKER_AGFA:
            tagname = maker_agfa_tagname(tag,model);
            break;
        case MAKER_ASAHI:
        case MAKER_PENTAX:
            tagname = maker_asahi_tagname(tag,model);
            break;
        case MAKER_CANON:
            tagname = maker_canon_tagname(tag,model);
            break;
        case MAKER_CASIO:
            tagname = maker_casio_tagname(tag,model);
            break;
        case MAKER_EPSON:
            tagname = maker_epson_tagname(tag,model);
            break;
        case MAKER_FUJIFILM:
            tagname = maker_fujifilm_tagname(tag,model);
            break;
        case MAKER_KYOCERA:
            tagname = maker_kyocera_tagname(tag,model);
            break;
        case MAKER_LEICA:
            tagname = maker_leica_tagname(tag,model);
            break;
        case MAKER_MINOLTA:
            tagname = maker_minolta_tagname(tag,model);
            break;
        case MAKER_NIKON:
            tagname = maker_nikon_tagname(tag,model);
            break;
        case MAKER_OLYMPUS:
            tagname = maker_olympus_tagname(tag,model);
            break;
        case MAKER_PANASONIC:
            tagname = maker_panasonic_tagname(tag,model);
            break;
        case MAKER_SANYO:
            tagname = maker_sanyo_tagname(tag,model);
            break;
        case MAKER_SIGMA:
            tagname = maker_sigma_tagname(tag,model);
            break;
        case MAKER_TOSHIBA:
            tagname = maker_toshiba_tagname(tag,model);
            break;
        case MAKER_SONY:
        case MAKER_KODAK:
        case MAKER_KONICA:
        case MAKER_HP:
        case MAKER_RICOH:
        case MAKER_TRAVELER:
        default:
            break;
    }

    if(tagname == NULL)
    {
        memset(unknown_buf,'\0',16);
        /* "regular" tags use lower case; use upper case for          */
        /* MakerNote tags                                             */
        if(snprintf(unknown_buf,11,"TAG_%#06X",(int)tag) > 11)
        {
            printf(" bad tag %#x not converted\n",tag);
            why(stdout);
            tagname = "Maker_BADTAG";
        }
        else
            tagname = unknown_buf;
    }
    return(tagname);
}
