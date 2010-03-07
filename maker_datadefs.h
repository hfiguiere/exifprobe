/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: maker_datadefs.h,v 1.1 2005/05/25 15:38:08 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


#ifndef MAKER_DATADEFS_INCLUDED
#define MAKER_DATADEFS_INCLUDED

#include "maker.h"

struct maker_scheme {
    unsigned long scheme_type;
    unsigned long note_fileoffset;
    unsigned long note_length;
    char *id;                   /* Initial id string, if present      */
    int id_length;              /* length of ID string, if present    */
    int tiffhdr_offset;         /* from start of section, if present  */
    int ifd_offset;             /* from start of section or tiff hdr  */
    int num_entries;            /* read from the start of the IFD     */
    int offsets_relative_to;    /* tiffheader, start of note, entry   */
    int no_nextifd_offset;      /* some don't include nextifd_offset  */
    int make;                   /* the internal identifier for make   */
    int model;                  /* the internal identifier for model  */
    int note_version;        /* for makes with multiple note versions */
    int tagset;              /* usually same as model_version...      */
    unsigned long fileoffset_base; /* entry offsets relative to this  */
                                   /* if not entry-relative           */
    unsigned short tiff_byteorder; /* main file byte order            */
    unsigned short note_byteorder; /* may be different                */
    struct ifd_entry first_entry;
};

struct camera_id {
    char *name;
    int namelen;
    int id;
    int noteversion;
    int notetagset;
};

struct camera_name {
    char *name;
    int namelen;
    int id;
    struct camera_id *model;
};

#endif /* MAKER_DATADEFS_INCLUDED */
