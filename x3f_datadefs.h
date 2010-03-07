/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: x3f_datadefs.h,v 1.1 2005/06/09 02:20:32 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef X3F_DATADEFS_INCLUDED
#define X3F_DATADEFS_INCLUDED


struct x3f_header {
    unsigned long magic;
    unsigned short version_minor;
    unsigned short version_major;
    char unique_id[16];
    unsigned long mark_bits;
    unsigned long image_width;
    unsigned long image_height;
    unsigned long image_rotation;
    char white_balance[32];
    char extended_data_types[32];
    unsigned long extended_data_values[32];
};

struct x3f_direntry {
    unsigned long section_id;
    unsigned long data_offset;
    unsigned long data_length;
    unsigned long data_type;
};

struct x3f_imag_header {
    unsigned long section_id;
    unsigned short version_minor;
    unsigned short version_major;
    unsigned long image_type;
    unsigned long image_format;
    unsigned long image_width;
    unsigned long image_height;
    unsigned long image_rowsize;
};

struct x3f_prop_header {
    unsigned long section_id;
    unsigned short version_minor;
    unsigned short version_major;
    unsigned long num_entries;
    unsigned long char_format;
    unsigned long reserved;
    unsigned long entry_length;
};

struct x3f_camf_header {
    unsigned long section_id;
    unsigned short version_minor;
    unsigned short version_major;
    unsigned long item1;
    unsigned long item2;
    unsigned long item3;
    unsigned long item4;
    unsigned long item5;
};
    
#endif /* X3F_DATADEFS_INCLUDED */
