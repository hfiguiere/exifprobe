/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2005 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: raf.c,v 1.7 2005/07/24 20:33:55 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Fujifilm RAF routines                                              */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* This is *experimental* and incomplete. It is based upon            */
/* examination of only four (count 'em' FOUR) images which I have     */
/* found on the web. This is sufficient to find and print the JPEG    */
/* image contained within the file, and even extract it with          */
/* something like 'extract_section'. The names and even identities of */
/* the items describing the CFA array are arbitrary and possibly      */
/* wrong; feedback is welcome.                                        */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "raf.h"


/* Print the RAF header information, even though it's hard to tell    */
/* exactly where the header ends and data begins.                     */

int
print_raf_header(FILE *inptr,struct fileheader *fileheader,unsigned long section_id)
{
    int status = -1;
    int chpr = 0;

    if(Print_options & section_id)
    {
        /* ###%%% for now, the first 60 bytes are "header"            */
        if(fileheader && (fileheader->probe_magic == PROBE_RAFMAGIC))
        {
            print_ascii(inptr,28,0);
            chpr += printf(" Model = ");
            print_ascii(inptr,32,28);
            status = 0;
        }
        else
            chpr += printf(" NOT AN RAF HEADER");
    }
    chpr = newline(chpr);
    return(status);
}

/* Process a Fujifilm RAF file, starting right after the RAF header.  */
/* At this point, I *think* the location of the jpeg image is fixed;  */
/* the rest is unclear, but do the best we can.                       */

unsigned long
process_raf(FILE *inptr,unsigned long offset,
            struct image_summary *summary_entry,
            char *parent_name,int indent)
{
    unsigned long start_of_jpeg,jpeg_length,max_offset;
    unsigned long table1_offset,table2_offset;
    unsigned long table1_length,table2_length;
    unsigned long CFA_offset,CFA_length;
    unsigned long unknown2,CFA_primaryarraywidth,CFA_fullarraywidth;
    unsigned long Secondary_offset,Secondary_length;
    unsigned long unknown3,Secondary_arraywidth,Secondary_fullarraywidth;
    unsigned long unused_value;
    struct image_summary *tmp_summary_entry;
    int unused;
    char *fullname = CNULL;
    int chpr = 0;
    unsigned short tag;
    

    CFA_primaryarraywidth = Secondary_offset = table1_offset = table2_offset =  0UL;

    /* Record the primary for the image summary                       */
    if(((summary_entry == NULL) || summary_entry->entry_lock) ||
                        (summary_entry->imageformat != IMGFMT_NOIMAGE))
    {
        summary_entry = new_summary_entry(summary_entry,FILEFMT_RAF,IMGFMT_RAF);
    }
    if(summary_entry)
    {
        summary_entry->imageformat = IMGFMT_RAF;
        summary_entry->imagesubformat = IMGSUBFMT_CFA;
        summary_entry->entry_lock = lock_number(summary_entry);
    }
    chpr = newline(chpr);

    /* A short section of zeros; why?                                 */
    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,offset,indent,"@");
        chpr += printf("<Offset Directory>");
        chpr = newline(chpr);
    }

    /* Some sort of ID or version?                                    */
    print_tag_address(ENTRY,offset,indent + SMALLINDENT,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s.",parent_name);
        chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"HeaderVersion");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" = ");
        print_ascii(inptr,4,offset);
    }
    chpr = newline(chpr);
    offset += 4;

    /* A 20 bytes section of zeros. Unknown                     */
    if((PRINT_SECTION))
    {
        print_tag_address(ENTRY,offset,indent + SMALLINDENT,"@");
        if((PRINT_VALUE))
            print_ubytes(inptr,20,offset);
        chpr = newline(chpr);
    }

    /* A jpeg reduced resolution image, complete with EXIF            */
    start_of_jpeg = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC);
    print_tag_address(ENTRY,RAF_JPEGLOC,indent + SMALLINDENT,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s.",parent_name);
        chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"JpegImageOffset");
    }
    if((PRINT_VALUE))
        chpr += printf(" = @%lu",start_of_jpeg);
    chpr = newline(chpr);
    print_tag_address(ENTRY,RAF_JPEGLOC + 4,indent + SMALLINDENT,"@");
    jpeg_length = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 4);
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s.",parent_name);
        chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"JpegImageLength");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",jpeg_length);
    chpr = newline(chpr);

    /* An offset to what may be a white balance table                 */
    print_tag_address(ENTRY,RAF_JPEGLOC + 8,indent + SMALLINDENT,"@");
    table1_offset = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 8);
    table1_length = read_ulong(inptr,TIFF_MOTOROLA,HERE);
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s.",parent_name);
        chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"Table1Offset");
    }
    if((PRINT_VALUE))
        chpr += printf(" = @%lu",table1_offset);
    chpr = newline(chpr);

    /* The length of the table; probably also the number of rows in   */
    /* the CFA array                                                  */
    print_tag_address(ENTRY,RAF_JPEGLOC + 12,indent + SMALLINDENT,"@");
    table1_length = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 12);
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s.",parent_name);
        chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"Table1Length");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",table1_length);
    chpr = newline(chpr);

    /* If there is a second table, this must be an SR Super CCD with  */
    /* a secondary CFA table containing the data for the secondary    */
    /* photodiodes. The start offset for the secondary data is offset */
    /* one row (2944 bytes) from the start of the primary data,       */
    /* suggesting that the primary and secondary CFA arrays are       */
    /* interleaved by rows (of 1472 unsigned shorts)                  */

    table2_offset = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 36);
    table2_length = read_ulong(inptr,TIFF_MOTOROLA,HERE);
    if(table2_offset == 0)
    {
        print_tag_address(ENTRY,RAF_JPEGLOC + 16,indent + SMALLINDENT,"@");
        CFA_offset = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 16);
        CFA_length = read_ulong(inptr,TIFF_MOTOROLA,HERE);
        Secondary_length = 0;
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFAOffset");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",CFA_offset);
        chpr = newline(chpr);
        print_tag_address(ENTRY,RAF_JPEGLOC + 20,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFALength");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",CFA_length);
        chpr = newline(chpr);
        for(unused = 24; unused < 64; unused += 4)
        {
            print_tag_address(ENTRY,RAF_JPEGLOC + unused,indent + SMALLINDENT,"@");
            unused_value = read_ulong(inptr,TIFF_MOTOROLA,(unsigned long)(RAF_JPEGLOC + unused));
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%s%-2d%*.*s","unused",(unused - 24)/4 + 1,TAGWIDTH-8,TAGWIDTH-8," ");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %lu",unused_value);
            chpr = newline(chpr);
        }
    }
    else
    {
        /* No secondary; easy.                                        */
        CFA_offset = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 16);
        CFA_length = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 20);

        print_tag_address(ENTRY,RAF_JPEGLOC + 16,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFA_Offset");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",CFA_offset);
        chpr = newline(chpr);

        print_tag_address(ENTRY,RAF_JPEGLOC + 20,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFA_length");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",CFA_length);
        chpr = newline(chpr);

        print_tag_address(ENTRY,RAF_JPEGLOC + 24,indent + SMALLINDENT,"@");
        unknown2 = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 24);
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"unknown2");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",unknown2);
        chpr = newline(chpr);

        CFA_primaryarraywidth = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 28);
        print_tag_address(ENTRY,RAF_JPEGLOC + 28,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            /* Array width in 16 bit unsigned shorts                  */
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFAPrimaryArrayWidth");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",CFA_primaryarraywidth);
        chpr = newline(chpr);

        print_tag_address(ENTRY,RAF_JPEGLOC + 32,indent + SMALLINDENT,"@");
        CFA_fullarraywidth = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 32);
        if((PRINT_TAGINFO))
        {
            /* Array width in bytes                                   */
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFAFullArrayWidth");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",CFA_fullarraywidth);
        chpr = newline(chpr);

        print_tag_address(ENTRY,RAF_JPEGLOC + 36,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"Table2Offset");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",table2_offset);
        chpr = newline(chpr);
        print_tag_address(ENTRY,RAF_JPEGLOC + 40,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"Table2Length");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",table2_length);
        chpr = newline(chpr);

        /* It appears that the secondary CFA data is interlaced by    */
        /* row with the primary data.                                 */
        Secondary_offset = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 44);
        Secondary_length = read_ulong(inptr,TIFF_MOTOROLA,HERE);
        print_tag_address(ENTRY,RAF_JPEGLOC + 44,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFASecondaryOffset**");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",Secondary_offset);
        chpr = newline(chpr);
        print_tag_address(ENTRY,RAF_JPEGLOC + 48,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFASecondaryLength**");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",Secondary_length);
        chpr = newline(chpr);

        unknown3 = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 52);
        Secondary_arraywidth = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 56);
        Secondary_fullarraywidth = read_ulong(inptr,TIFF_MOTOROLA,RAF_JPEGLOC + 60);
        print_tag_address(ENTRY,RAF_JPEGLOC + 52,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"unknown3");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",unknown3);
        chpr = newline(chpr);
        print_tag_address(ENTRY,RAF_JPEGLOC + 56,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFASecondaryArrayWidth");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",Secondary_arraywidth);
        chpr = newline(chpr);
        print_tag_address(ENTRY,RAF_JPEGLOC + 60,indent + SMALLINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            /* I dunno what to call this...                           */
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"CFASecondaryFullArrayWidth");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",Secondary_fullarraywidth);
        chpr = newline(chpr);
    }
    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,RAF_JPEGLOC + 63,indent,"@");
        chpr += printf("</Offset Directory>");
        chpr = newline(chpr);
    }

    /* Finish the summary entry                                       */
    if(summary_entry)
    {
        if(summary_entry->length <= 0)
            summary_entry->length = CFA_length + Secondary_length;
        if(summary_entry->offset <= 0)
            summary_entry->offset = CFA_offset;
        /* Not really compression; just record the presence of the    */
        /* secondary                                                  */
        if(Secondary_offset)
            summary_entry->compression = 1;
        else
            summary_entry->compression = 0;

        /* These aren't really pixel sizes, since the actual number   */
        /* of columns is half the arraywidth, and the array will have */
        /* to be rotated 45 degrees. And then there is the secondary  */
        /* pixel data...                                              */
        /* It's what we know, and post-processing can do what it      */
        /* wants.                                                     */
        if(CFA_primaryarraywidth)
        {
            summary_entry->pixel_width = CFA_primaryarraywidth;
            summary_entry->pixel_height = table1_length;
            summary_entry->primary_width = CFA_primaryarraywidth;
            summary_entry->primary_height = table1_length;
        }
        else
        {
            /* No size provided; this is the best we can do.          */
            summary_entry->pixel_width = (CFA_length + Secondary_length)/table1_length;
            summary_entry->pixel_height = table1_length;
            summary_entry->primary_width = summary_entry->pixel_width;
            summary_entry->primary_height = table1_length;
        }
        summary_entry->subfiletype = PRIMARY_TYPE;
    }
    
    /* Now display the offset sections, starting with the jpeg image  */
    /* (but only if showing sections)                                 */
    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,start_of_jpeg,indent,"@");
        chpr += printf("#### Start of Jpeg Image, length %lu",jpeg_length);
        chpr = newline(chpr);
    }

    /* Is this really a jpeg section?                             */
    tag = read_ushort(inptr,TIFF_MOTOROLA,start_of_jpeg);
    if(tag == JPEG_SOI)
    {
        /* If so, process it regardless of output options, to pick up */
        /* information for the image summary                          */
        fullname = splice(parent_name,".","JPEG");
        max_offset = process_jpeg_segments(inptr,start_of_jpeg,JPEG_SOI,jpeg_length,
                                                summary_entry,fullname,"@",SMALLINDENT);
        print_jpeg_status();
        if(summary_entry)
        { 
            /* The jpeg summary entry should be next in the chain;    */
            /* there may be others following; mark the jpeg entry so  */
            /* that it will show up in the format summary.            */
            if((tmp_summary_entry = summary_entry->next_entry))
                tmp_summary_entry->filesubformat |= FILESUBFMT_JPEG;
        }
    }
    else
        dumpsection(inptr,start_of_jpeg,jpeg_length,indent + SMALLINDENT);

    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,start_of_jpeg + jpeg_length - 1,indent,"@");
        chpr += printf("#### End of Jpeg Image, length %lu",jpeg_length);
        chpr = newline(chpr);

        print_tag_address(SECTION,table1_offset,indent,"@");
        chpr += printf("<Table 1> length %lu",table1_length);
        chpr = newline(chpr);
        if((PRINT_VALUE))
            dumpsection(inptr,table1_offset,table1_length,indent + SMALLINDENT);
        print_tag_address(SECTION,table1_offset + table1_length - 1,indent,"@");
        chpr += printf("</Table 1>");
        chpr = newline(chpr);

        if(table2_offset)
        {
            print_tag_address(SECTION,table2_offset,indent,"@");
            chpr += printf("<Table 2> length %lu",table2_length);
            chpr = newline(chpr);
            if((PRINT_VALUE))
                dumpsection(inptr,table2_offset,table2_length,indent + SMALLINDENT);
            print_tag_address(SECTION,table2_offset + table2_length - 1,indent,"@");
            chpr += printf("</Table 2>");
            chpr = newline(chpr);
        }

        /* The full CFA, with secondary if present                    */
        print_tag_address(SECTION,CFA_offset,indent,"@");
        chpr += printf("<CFA Image> length %lu",CFA_length + Secondary_length);
        chpr = newline(chpr);
        if((PRINT_VALUE))
            dumpsection(inptr,CFA_offset,CFA_length + Secondary_length,indent + SMALLINDENT);
        print_tag_address(SECTION,CFA_offset + CFA_length + Secondary_length - 1,indent,"@");
        chpr += printf("</CFA Image>");
        chpr = newline(chpr);

    }
    
    setcharsprinted(chpr);
    return(get_filesize(inptr));
}

