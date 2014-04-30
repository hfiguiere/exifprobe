/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2005 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: x3f.c,v 1.7 2005/07/24 20:31:30 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Sigma/FOVEON x3f routines                                          */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* See http://www.sd9.org.uk/X3F_Format.pdf                           */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "x3f.h"
#include "maker_extern.h"

/* Read and record the X3F file header                                */

struct x3f_header *
read_x3fheader(FILE *inptr,unsigned long offset)
{
    static struct x3f_header header;
    struct x3f_header *headerptr = NULL;
    unsigned long xmagic;
    unsigned char *bytearray;

    memset(&header,0,sizeof(struct x3f_header));
    if(inptr)
    {
        xmagic = read_ulong(inptr,TIFF_INTEL,offset);
        if(xmagic == PROBE_X3FMAGIC)
        {
            header.magic = xmagic;
            header.version_major = read_ushort(inptr,TIFF_INTEL,HERE);
            header.version_minor = read_ushort(inptr,TIFF_INTEL,HERE);
            bytearray = read_bytes(inptr,16,HERE);
            if(bytearray)
                memcpy(header.unique_id,bytearray,16);
            header.mark_bits = read_ulong(inptr,TIFF_INTEL,HERE);
            header.image_width = read_ulong(inptr,TIFF_INTEL,HERE);
            header.image_height = read_ulong(inptr,TIFF_INTEL,HERE);
            header.image_rotation = read_ulong(inptr,TIFF_INTEL,HERE);
            if(header.version_minor >= 1)
            {
                /* this is a string, e.g. "AUTO\0"                    */
                bytearray = read_bytes(inptr,32,HERE);
                if(bytearray)
                    memcpy(header.white_balance,bytearray,32);
                bytearray = read_bytes(inptr,32,HERE);
                if(bytearray)
                    memcpy(header.extended_data_types,bytearray,32);
                bytearray = read_bytes(inptr,128,HERE);
                if(bytearray)
                    memcpy(header.extended_data_values,bytearray,128);
            }
            headerptr = &header;
        }
    }
    return(headerptr);
}


/* Print X3f header information if options permit. Return 0 if       */
/* information is valid to print, or -1 if the header is invalid      */

int
print_x3f_header(FILE *inptr,struct fileheader *header,unsigned long section_id)
{
    struct x3f_header *x3fheader = NULL;
    int status = -1;
    int chpr = 0;
    int i;

    if(header)
    {
        if(header->x3f_header)
        {
            x3fheader = header->x3f_header;
            if(header->probe_magic == PROBE_X3FMAGIC)
            {
                if(Print_options & section_id)
                {
                    chpr += printf("%4.4s (%#06lx) version %2u.%-2u, id ",
                                            (char *)&(x3fheader->magic),
                                                    x3fheader->magic,
                                                    x3fheader->version_major,
                                                    x3fheader->version_minor);
                    chpr += printf("%#02x ",x3fheader->unique_id[0] & 0xff);
                    for(i = 0; i < 16; ++i)
                        chpr += printf("%02x ",x3fheader->unique_id[i] & 0xff);
                    chpr = newline(chpr);
                    if(x3fheader->version_minor >= 1)
                    {
                        print_tag_address(ENTRY,24,MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("X3f.Header.");
                            chpr += printf("MarkBits");
                            if((PRINT_VALUE))
                            {
                                putindent(X3FTAGWIDTH - 8);
                                chpr += printf(" = ");
                            }
                        }
                        if((PRINT_VALUE))
                            chpr += printf("%#06lx",x3fheader->mark_bits);
                        chpr = newline(chpr);
                    }
                    print_tag_address(ENTRY,28,MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("X3f.Header.");
                        chpr += printf("ImageWidth");
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 10);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("%lu",x3fheader->image_width);
                    chpr = newline(chpr);
                    print_tag_address(ENTRY,32,MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("X3f.Header.");
                        chpr += printf("ImageHeight");
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 11);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("%lu",x3fheader->image_height);
                    chpr = newline(chpr);
                    print_tag_address(ENTRY,36,MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("X3f.Header.");
                        chpr += printf("ImageRotation");
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 13);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("%lu",x3fheader->image_rotation);
                    chpr = newline(chpr);
                    if(x3fheader->version_minor >= 1)
                    {
                        print_tag_address(ENTRY,40,MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("X3f.Header.");
                            chpr += printf("WhiteBalance");
                            if((PRINT_VALUE))
                            {
                                putindent(X3FTAGWIDTH - 12);
                                chpr += printf(" = ");
                            }
                        }
                        if((PRINT_VALUE))
                            show_string(x3fheader->white_balance,32,0);
                        chpr = newline(chpr);
                        print_tag_address(ENTRY,72,MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("X3f.Header.");
                            chpr += printf("ExtendedDataTypes");
                            if((PRINT_VALUE))
                            {
                                putindent(X3FTAGWIDTH - 17);
                                chpr += printf(" = ");
                            }
                        }
                        if((PRINT_VALUE))
                            print_ubytes(inptr,32,72);
                        chpr = newline(chpr);
                        print_tag_address(ENTRY,104,MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("X3f.Header.");
                            chpr += printf("ExtendedDataValues");
                            if((PRINT_VALUE))
                            {
                                putindent(X3FTAGWIDTH - 19);
                                chpr += printf(" = ");
                            }
                        }
                        if((PRINT_VALUE))
                            print_ulong(inptr,32,TIFF_INTEL,104);
                        chpr = newline(chpr);
                    }
                    print_tag_address(SECTION,104 + 128 - 1,0,"@");
                    chpr += printf("</END OF HEADER>");
                    chpr = newline(chpr);
                }
                status = 0;
            }
            else if(Print_options & section_id)
            {
                    chpr += printf("INVALID X3F IDENTIFIER ");
                    print_magic(header->probe_magic,4);
            }
        }
        else
            fprintf(stderr,"%s: null x3fheader to print_x3f_header()\n",Progname);
    }
    else
        fprintf(stderr,"%s: null fileheader to print_x3f_header()\n",Progname);
    chpr = newline(chpr);
    return(status);
}

/* Process the remainder of an X3F file, after the header has been    */
/* read and validated. Returns the maximum offset reached in          */
/* processing, which is normally end of file.                         */

unsigned long
process_x3f(FILE *inptr,struct fileheader *header,unsigned long fileoffset_base,
                                        struct image_summary *summary_entry,
                                        char *parent_name,int level,int indent)
{
    struct x3f_header *x3fheader;
    unsigned long max_offset = 0UL;
    unsigned short byteorder = 0;
    unsigned long directory_pointer = 0UL;

    if(inptr)
    {
        if(header)
        {
            if(header->x3f_header)
            {
                x3fheader = header->x3f_header;
                /* X3F is always little-endian; nonetheless, pass it  */
                /* as an argument to lower-level functions. Just set  */
                /* it here.                                           */
                byteorder = TIFF_INTEL;
                if(fseek(inptr,-4L,SEEK_END) == 0)
                    directory_pointer = ftell(inptr);
                if(ferror(inptr) == 0)
                {
                    max_offset = process_x3f_dir(inptr,byteorder,x3fheader,
                                                    directory_pointer,summary_entry,
                                                    parent_name,level,indent);
                    if(max_offset > 0L)
                        max_offset += 4;
                }
                else
                    fprintf(stderr,"%s: cannot read directory offset\n",Progname);
            }
            else
                fprintf(stderr,"%s: null x3fheader to process_x3f()\n",Progname);
        }
        else
            fprintf(stderr,"%s: null fileheader to process_x3f()\n",Progname);
    }
    else
        fprintf(stderr,"%s: no open file pointer to read Print Image data\n",
                                                                        Progname);

    return(max_offset);
}

/* Find the directory at the end of the file and process it. Report   */
/* the detail data first, then show the directory itself.             */

unsigned long
process_x3f_dir(FILE *inptr,unsigned short byteorder,struct x3f_header *x3fheader,
                                                unsigned long directory_pointer,
                                                struct image_summary *summary_entry,
                                                char *parent_name,int level,int indent)
{
    struct x3f_direntry *entry;
    unsigned long directory_offset = 0UL;
    unsigned long section_id;
    unsigned long max_dir_offset = 0L;
    unsigned long entry_offset,next_entry_offset;
    unsigned long num_entries;
    unsigned short version_major;
    unsigned short version_minor;
    int i;
    int chpr = 0;

    directory_offset = read_ulong(inptr,byteorder,directory_pointer);

    section_id = read_ulong(inptr,byteorder,directory_offset);
    version_minor = read_ushort(inptr,byteorder,HERE);
    version_major = read_ushort(inptr,byteorder,HERE);
    num_entries = read_ulong(inptr,byteorder,HERE);
    entry = NULL;

    entry_offset = ftell(inptr);

    /* Print the entries first, in order                              */
    for(i = 0; i < num_entries; ++i)
    {
        entry = read_x3f_direntry(inptr,byteorder,entry,entry_offset);
        if(ferror(inptr))
        {
            fprintf(stderr,"%s: error reading directory entry at %lu\n",
                                                Progname,entry_offset);
            break;
        }
        if(entry == NULL)
        {
            fprintf(stderr,"%s: null entry %d\n",Progname,i);
            break;
        }
        next_entry_offset = ftell(inptr);
        process_x3f_direntry(inptr,byteorder,entry,entry_offset,
                                            summary_entry,parent_name,i,indent);
        entry_offset = next_entry_offset;
    }
    chpr = newline(chpr);
    
    /* Now show the directory itself                                  */

    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,directory_offset,indent,"@");
        chpr += printf("<DIRECTORY> version %d.%d id \"%4.4s\", %lu entries",
                                            version_major,version_minor,
                                            (char *)&section_id,num_entries);
    }
    chpr = newline(chpr);
    entry_offset = directory_offset + 12;
    for(i = 0; i < num_entries; ++i)
    {
        entry = read_x3f_direntry(inptr,byteorder,entry,entry_offset);
        if(ferror(inptr))
        {
            fprintf(stderr,"%s: error reading directory entry at %lu\n",
                                                Progname,entry_offset);
            break;
        }
        if(entry == NULL)
        {
            fprintf(stderr,"%s: null entry %d\n",Progname,i);
            break;
        }
        next_entry_offset = ftell(inptr);
        print_x3f_direntry(inptr,entry,entry_offset,
                                    parent_name,i,indent);
        entry_offset = next_entry_offset;
    }
    max_dir_offset = ftell(inptr);
    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,max_dir_offset - 1,indent,"@");
        chpr += printf("</DIRECTORY>");
        chpr = newline(chpr);
    }

    print_tag_address(ENTRY,directory_pointer,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s.",parent_name);
        chpr += printf("%s","DirectoryOffset");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 15);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
    {
        if(PRINT_BOTH_OFFSET)
            chpr += printf("@%#lx=%lu",directory_offset,directory_offset);
        else if(PRINT_HEX_OFFSET)
            chpr += printf("@%#lx",directory_offset);
        else 
            chpr += printf("@%lu",directory_offset);
    }
    chpr = newline(chpr);
    
    if(entry)
        free(entry);
    return(max_dir_offset);
}

/* Process a directory entry, representing one section of the file.   */
/* An entry number is added to the section id, so that sections with  */
/* the same name (e.g. IMGA) can be identified, especially in LIST    */
/* mode.                                                              */

void
process_x3f_direntry(FILE *inptr,unsigned short byteorder,struct x3f_direntry *entry,
                            unsigned long offset,struct image_summary *summary_entry,
                            char *parent_name,int entry_num,int indent)
{
    struct x3f_imag_header *imag_header;
    struct x3f_prop_header *prop_header;
    struct x3f_camf_header *camf_header;
    unsigned long data_offset = 0UL;
    unsigned long data_length = 0UL;
    char *dirname = CNULL;
    int chpr = 0;

    if(entry)
    {
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,entry->data_offset,indent,"@");
            chpr += printf("<%4.4s:%d> (%#06lx)",(char *)&entry->data_type,entry_num,entry->data_type);
            chpr += printf(" length %lu",entry->data_length);
            chpr = newline(chpr);
        }
        switch(entry->data_type)
        {
            case X3F_IMAG:
                dirname = splice(parent_name,".","IMAG");
                imag_header = read_x3f_imag_header(inptr,byteorder,entry->data_offset);
                data_offset = display_x3f_imag_header(entry,imag_header,dirname,entry_num,
                                                                    indent + MEDIUMINDENT);
                data_length = entry->data_length - (data_offset - entry->data_offset);
                if((summary_entry == NULL) || summary_entry->entry_lock)
                    summary_entry = new_summary_entry(summary_entry,FILEFMT_X3F,IMGFMT_X3F);
                if(summary_entry)
                {
                    switch(imag_header->image_type)
                    {
                        case 2:
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                            break;
                        case 3:
                            /* observed; spec doesn't say             */
                            summary_entry->subfiletype = PRIMARY_TYPE;
                            break;
                        default:
                            /* assume this; spec doesn't say          */
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                            break;
                    }
                    switch(imag_header->image_format)
                    {
                        case 3:
                            summary_entry->imagesubformat = IMGSUBFMT_RGB;
                            break;
                        case 11:
                            summary_entry->imagesubformat = IMGSUBFMT_HUFF_COMPRESSED_RGB;
                            break;
                        case 18:
                            summary_entry->imagesubformat = IMGSUBFMT_JPEG_COMPRESSED_RGB;
                            break;
                        case 6:
                            summary_entry->imagesubformat = IMGSUBFMT_X3F_COMPRESSED;
                            break;
                        default:
                            summary_entry->imagesubformat = IMGSUBFMT_X3F_UNSPECIFIED;
                            break;
                    }
                    summary_entry->imageformat = IMGFMT_X3F;
                    summary_entry->pixel_width = imag_header->image_width;
                    summary_entry->pixel_height = imag_header->image_height;
                    summary_entry->offset = data_offset;
                    summary_entry->length = data_length;
                    summary_entry->entry_lock = lock_number(summary_entry);

                }
                if((PRINT_SECTION))
                {
                    print_tag_address(SECTION,data_offset,indent + MEDIUMINDENT,"@");
                    chpr += printf("<IMAG.Data> length %lu",data_length);
                    chpr = newline(chpr);
                    dumpsection(inptr,data_offset,data_length,indent + LARGEINDENT);
                    print_tag_address(SECTION,entry->data_offset + entry->data_length - 1,
                                                                indent + MEDIUMINDENT,"@");
                    chpr += printf("</IMAG.Data>");
                }
                else if((LIST_MODE))
                {
                    print_tag_address(ENTRY,data_offset,indent + MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s:%d.",dirname,entry_num);
                        else
                            chpr += printf("IMAG:%d.",entry_num);
                        chpr += printf("Data");
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 7);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("@%lu:%lu",data_offset,data_length);
                }
                chpr = newline(chpr);
                break;
            case X3F_IMA2:
                dirname = splice(parent_name,".","IMA2");
                imag_header = read_x3f_imag_header(inptr,byteorder,entry->data_offset);
                data_offset = display_x3f_imag_header(entry,imag_header,dirname,entry_num,
                                                                    indent + MEDIUMINDENT);
                data_length = entry->data_length - (data_offset - entry->data_offset);
                if((summary_entry == NULL) || summary_entry->entry_lock)
                    summary_entry = new_summary_entry(summary_entry,FILEFMT_X3F,IMGFMT_X3F);
                if(summary_entry)
                {
                    switch(imag_header->image_type)
                    {
                        case 2:
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                            break;
                        case 3:
                            /* observed; spec doesn't say             */
                            summary_entry->subfiletype = PRIMARY_TYPE;
                            break;
                        default:
                            /* assume this; spec doesn't say          */
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                            break;
                    }
                    switch(imag_header->image_format)
                    {
                        case 3:
                            summary_entry->imagesubformat = IMGSUBFMT_RGB;
                            break;
                        case 11:
                            summary_entry->imagesubformat = IMGSUBFMT_HUFF_COMPRESSED_RGB;
                            break;
                        case 18:
                            summary_entry->imagesubformat = IMGSUBFMT_JPEG_COMPRESSED_RGB;
                            break;
                        case 6:
                            /* No idea how "raw" this is, or how it's */
                            /* compressed/encoded                     */
                            summary_entry->imagesubformat = IMGSUBFMT_X3F_COMPRESSED;
                            break;
                        default:
                            summary_entry->imagesubformat = IMGSUBFMT_X3F_UNSPECIFIED;
                            break;
                    }
                    summary_entry->imageformat = IMGFMT_X3F;
                    summary_entry->pixel_width = imag_header->image_width;
                    summary_entry->pixel_height = imag_header->image_height;
                    summary_entry->offset = data_offset;
                    summary_entry->length = data_length;
                    summary_entry->entry_lock = lock_number(summary_entry);

                }
                if((PRINT_SECTION))
                {
                    print_tag_address(SECTION,data_offset,indent + MEDIUMINDENT,"@");
                    chpr += printf("<IMA2.Data> length %lu",data_length);
                    chpr = newline(chpr);
                    dumpsection(inptr,data_offset,data_length,indent + LARGEINDENT);
                    print_tag_address(SECTION,entry->data_offset + entry->data_length - 1,
                                                                indent + MEDIUMINDENT,"@");
                    chpr += printf("</IMA2.Data>");
                }
                else if((LIST_MODE))
                {
                    print_tag_address(ENTRY,data_offset,indent + MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s:%d.",dirname,entry_num);
                        else
                            chpr += printf("IMA2:%d.",entry_num);
                        chpr += printf("Data");
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 7);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("@%lu:%lu",data_offset,data_length);
                }
                chpr = newline(chpr);
                break;
            case X3F_PROP:
                dirname = splice(parent_name,".","PROP");
                prop_header = read_x3f_prop_header(inptr,byteorder,entry->data_offset);
                data_offset = display_x3f_prop_header(entry,prop_header,dirname,entry_num,
                                                                    indent + MEDIUMINDENT);
                data_length = entry->data_length - (data_offset - entry->data_offset);
                process_x3f_props(inptr,byteorder,data_offset,data_length,
                                                prop_header->char_format,dirname,
                                                prop_header->num_entries,
                                                indent + MEDIUMINDENT);
                break;
            case X3F_CAMF:
                dirname = splice(parent_name,".","CAMF");
                camf_header = read_x3f_camf_header(inptr,byteorder,entry->data_offset);
                data_offset = display_x3f_camf_header(entry,camf_header,dirname,entry_num,
                                                                    indent + MEDIUMINDENT);
                data_length = entry->data_length - (data_offset - entry->data_offset);
                if((PRINT_SECTION))
                {
                    print_tag_address(SECTION,data_offset,indent + MEDIUMINDENT,"@");
                    chpr += printf("<CAMF.Data> length %lu",data_length);
                    chpr = newline(chpr);
                    dumpsection(inptr,data_offset,data_length,indent + LARGEINDENT);
                    print_tag_address(SECTION,entry->data_offset + entry->data_length - 1,
                                                                indent + MEDIUMINDENT,"@");
                    chpr += printf("</CAMF.Data>");
                }
                else if((LIST_MODE))
                {
                    print_tag_address(ENTRY,data_offset,indent + MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s:%d.",dirname,entry_num);
                        else
                            chpr += printf("CAMF:%d.",entry_num);
                        chpr += printf("Data");
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 7);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("@%lu:%lu",data_offset,data_length);
                }
                chpr = newline(chpr);
                break;
            default:
                if((PRINT_SECTION))
                {
                    print_tag_address(SECTION,entry->data_offset,indent + MEDIUMINDENT,"@");
                    chpr += printf("<%4.4s> length %lu",(char *)&entry->section_id,entry->data_length);
                    chpr = newline(chpr);
                    dumpsection(inptr,data_offset,data_length,indent + LARGEINDENT);
                    print_tag_address(SECTION,entry->data_offset + entry->data_length - 1,
                                                                indent + MEDIUMINDENT,"@");
                    chpr += printf("</%4.4s>",(char *)&entry->section_id);
                }
                else if((LIST_MODE))
                {
                    print_tag_address(ENTRY,data_offset,indent,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s:%d.",dirname,entry_num);
                        else
                            chpr += printf("%4.4s:%d.",(char *)&entry->section_id,entry_num);
                        if((PRINT_VALUE))
                        {
                            putindent(X3FTAGWIDTH - 7);
                            chpr += printf(" = ");
                        }
                    }
                    if((PRINT_VALUE))
                        chpr += printf("@%lu:%lu",entry->data_offset,entry->data_length);
                }
                chpr = newline(chpr);
                break;
        }
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,entry->data_offset + entry->data_length - 1,indent,"@");
            chpr += printf("</%4.4s:%d>",(char *)&entry->data_type,entry_num);
            chpr += newline(chpr);
        }
        if(dirname)
            free(dirname);
    }
}

void 
print_x3f_direntry(FILE *inptr,struct x3f_direntry *entry,unsigned long offset,
                                        char *parent_name,int entry_num,int indent)
{
    int chpr = 0;

    if(entry)
    {
        print_tag_address(ENTRY,offset,indent + MEDIUMINDENT,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%4.4s:%-2d",(char *)&entry->data_type,entry_num);
            if((PRINT_VALUE))
                putindent(X3FTAGWIDTH - 7);
        }
        if((PRINT_VALUE))
        {
            chpr += printf(" = ");
            chpr += printf("@%lu:%lu",entry->data_offset,entry->data_length);
        }
        chpr += newline(chpr);
    }
}


struct x3f_direntry *
read_x3f_direntry(FILE *inptr,unsigned short byteorder,struct x3f_direntry *entry,
                                                            unsigned long offset)
{
    if(inptr)
    {
        if(entry == NULL)
            entry = (struct x3f_direntry *)malloc(sizeof(struct x3f_direntry));
        if(entry)
        {
            memset(entry,0,sizeof(struct x3f_direntry));
            entry->data_offset = read_ulong(inptr,byteorder,offset);
            entry->data_length = read_ulong(inptr,byteorder,HERE);
            entry->data_type = read_ulong(inptr,byteorder,HERE);
        }
    }
    else
        entry = (struct x3f_direntry *)0;
    return(entry);
}

struct x3f_imag_header *
read_x3f_imag_header(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    static struct x3f_imag_header header;
    struct x3f_imag_header *headerptr= (struct x3f_imag_header *)0;

    if(!feof(inptr) && !ferror(inptr))
    {
        header.section_id = read_ulong(inptr,byteorder,offset);
        header.version_minor = read_ushort(inptr,byteorder,HERE);
        header.version_major = read_ushort(inptr,byteorder,HERE);
        header.image_type = read_ulong(inptr,byteorder,HERE);
        header.image_format = read_ulong(inptr,byteorder,HERE);
        header.image_width = read_ulong(inptr,byteorder,HERE);
        header.image_height = read_ulong(inptr,byteorder,HERE);
        header.image_rowsize = read_ulong(inptr,byteorder,HERE);
            if(!feof(inptr) && !ferror(inptr))
                headerptr = &header;
    }
    return(headerptr);
}

struct x3f_camf_header *
read_x3f_camf_header(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    static struct x3f_camf_header header;
    struct x3f_camf_header *headerptr = (struct x3f_camf_header *)0;

    if(!feof(inptr) && !ferror(inptr))
    {
        header.section_id = read_ulong(inptr,byteorder,offset);
        header.version_minor = read_ushort(inptr,byteorder,HERE);
        header.version_major = read_ushort(inptr,byteorder,HERE);
        header.item1 = read_ulong(inptr,byteorder,HERE);
        header.item2 = read_ulong(inptr,byteorder,HERE);
        header.item3 = read_ulong(inptr,byteorder,HERE);
        header.item4 = read_ulong(inptr,byteorder,HERE);
        header.item5 = read_ulong(inptr,byteorder,HERE);
        if(!feof(inptr) && !ferror(inptr))
            headerptr = &header;
    }
    return(headerptr);
}

struct x3f_prop_header *
read_x3f_prop_header(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    static struct x3f_prop_header header;
    struct x3f_prop_header *headerptr = (struct x3f_prop_header *)0;

    if(!feof(inptr) && !ferror(inptr))
    {
        header.section_id = read_ulong(inptr,byteorder,offset);
        header.version_minor = read_ushort(inptr,byteorder,HERE);
        header.version_major = read_ushort(inptr,byteorder,HERE);
        header.num_entries = read_ulong(inptr,byteorder,HERE);
        header.char_format = read_ulong(inptr,byteorder,HERE);
        header.reserved = read_ulong(inptr,byteorder,HERE);
        header.entry_length = read_ulong(inptr,byteorder,HERE);
        if(!feof(inptr) && !ferror(inptr))
            headerptr = &header;
    }
    return(headerptr);
}

unsigned long
display_x3f_imag_header(struct x3f_direntry *direntry,struct x3f_imag_header *header,
                                                char *dirname,int entry_num,int indent)
{
    int chpr = 0;

    print_tag_address(ENTRY,direntry->data_offset,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","SectionId");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 9);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%4.4s",(char *)&header->section_id);
    chpr = newline(chpr);
    
    print_tag_address(ENTRY,direntry->data_offset + 4,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Version");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 7);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%u.%u",header->version_major,header->version_minor);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 8,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","ImageType");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 9);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
    {
        chpr += printf("%-2lu",header->image_type);
        switch(header->image_type)
        {
            case 2: chpr += printf(" = \"Preview\""); break;
            case 3: chpr += printf(" = \"Primary\""); break;
            default: break;
        }
    }
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 12,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","ImageFormat");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 11);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
    {
        chpr += printf("%-2lu",header->image_format);
        switch(header->image_format)
        {
            case 3: chpr += printf(" = \"uncompressed RGB\""); break;
            case 6: chpr += printf(" = \"compressed RAW\""); break;
            case 11: chpr += printf(" = \"Huffman compressed RGB\""); break;
            case 18: chpr += printf(" = \"JPEG compressed RGB\""); break;
            default: break;
        }
    }
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 16,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","ImageWidth");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 10);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->image_width);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 20,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","ImageHeight");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 11);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->image_height);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 24,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","RowSize");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 7);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
    {
        chpr += printf("%lu",header->image_rowsize);
        if(header->image_rowsize == 0)
            chpr += printf(" = \"variable length\"");
    }
    chpr = newline(chpr);

    return(direntry->data_offset + 28);
}

unsigned long
display_x3f_prop_header(struct x3f_direntry *direntry,struct x3f_prop_header *header,
                                                char *dirname,int entry_num,int indent)
{
    int chpr = 0;

    print_tag_address(ENTRY,direntry->data_offset,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","SectionId");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 9);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%4.4s",(char *)&header->section_id);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 4,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Version");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 7);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%u.%u",header->version_major,header->version_minor);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 8,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Entries");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 7);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->num_entries);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 12,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","CharFormat");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 10);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->char_format);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 16,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","RESERVED");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 8);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->reserved);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 20,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","SectionLength");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 13);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->entry_length);
    chpr = newline(chpr);

    return(direntry->data_offset + 24);
}

unsigned long
display_x3f_camf_header(struct x3f_direntry *direntry,struct x3f_camf_header *header,
                                                char *dirname,int entry_num,int indent)
{
    int chpr = 0;

    print_tag_address(ENTRY,direntry->data_offset,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","SectionId");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 9);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%4.4s",(char *)&header->section_id);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 4,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Version");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 7);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%u.%u",header->version_major,header->version_minor);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 8,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Item1");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 5);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->item1);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 12,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Item2");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 5);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->item2);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 16,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Item3");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 5);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%4.4s",(char *)&header->item3);
    chpr = newline(chpr);

    print_tag_address(ENTRY,direntry->data_offset + 20,indent,"@");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("%s:%d.Header.",dirname,entry_num);
        chpr += printf("%s","Item4");
        if((PRINT_VALUE))
        {
            putindent(X3FTAGWIDTH - 5);
            chpr += printf(" = ");
        }
    }
    if((PRINT_VALUE))
        chpr += printf("%lu",header->item4);
    chpr = newline(chpr);

    /* Not sure how long the header is... CAMF is not described in    */
    /* the public X3F spec.                                           */

    /* Assume the header is same length as PROP for the purposes of   */
    /* dumping the data section                                       */
    return(direntry->data_offset + 24);
}

/* Process the property section. After the header, the section starts */
/* with an index of offsets to each name and value. The actual names  */
/* and values follow.                                                 */


unsigned long
process_x3f_props(FILE *inptr,unsigned short byteorder,unsigned long data_offset,
                           unsigned long data_length,unsigned long char_format,
                           char *dirname,unsigned long num_entries,int indent)
{
    unsigned long max_offset = 0UL;
    unsigned long name_offset = 0UL;
    unsigned long value_offset = 0UL;
    unsigned long offset = 0UL;
    unsigned long property_offset = 0UL;
    unsigned long index_length = 0UL;
    unsigned long list_data_length = 0UL;
    int chpr = 0;
    int i;

    if(inptr)
    {
        clearerr(inptr);
        offset = data_offset;
        property_offset = data_offset + (num_entries * 8);
        index_length = num_entries * 8;
        list_data_length = data_length - index_length;

        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,data_offset,indent,"@");
            chpr += printf("<Property List> %ld entries, length %lu",num_entries,data_length);
            chpr = newline(chpr);
            print_tag_address(SECTION,data_offset,indent + SMALLINDENT,"@");
            chpr += printf("<Property List Index> %ld entries, length %lu",num_entries,index_length);
            chpr = newline(chpr);
        }
        /* Print the index of name/value offsets first                */
        for(i = 0; i < num_entries; ++i)
        {
            if(fseek(inptr,offset,SEEK_SET) == 0)
            {
                name_offset = read_ulong(inptr,byteorder,HERE);
                name_offset *= 2;
                if(feof(inptr) || ferror(inptr))
                {
                    PUSHCOLOR(RED);
                    chpr += printf(" FAILED to read Property List name %d at offset %lu",i,offset);
                    POPCOLOR();
                    break;
                }
                value_offset = read_ulong(inptr,byteorder,HERE);
                value_offset *= 2;
                if(feof(inptr) || ferror(inptr))
                {
                    PUSHCOLOR(RED);
                    chpr += printf(" FAILED to read Property List value %d at offset %lu",i,offset + 4);
                    POPCOLOR();
                    break;
                }

                /* Show the name/value offsets, even in LIST mode     */
                print_tag_address(ENTRY,offset,indent + MEDIUMINDENT,"@");
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.Index.",dirname);
                    chpr += printf("NameOffset.%-2d",i);
                    if((PRINT_VALUE))
                    {
                        putindent(X3FTAGWIDTH - 15);
                        chpr += printf(" = ");
                    }
                }
                if((PRINT_VALUE))
                {
                    chpr += printf("@%lu",name_offset + property_offset);
                    if((PRINT_RAW_VALUES))
                        chpr += printf(" (%lu)",name_offset);
                }
                chpr = newline(chpr);

                print_tag_address(ENTRY,offset + 4,indent + MEDIUMINDENT,"@");
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.Index.",dirname);
                    chpr += printf("ValueOffset.%-2d",i);
                    if((PRINT_VALUE))
                    {
                        putindent(X3FTAGWIDTH - 16);
                        chpr += printf(" = ");
                    }
                }
                if((PRINT_VALUE))
                {
                    chpr += printf("@%lu",value_offset + property_offset);
                    if((PRINT_RAW_VALUES))
                        chpr += printf(" (%lu)",value_offset);
                }
                chpr = newline(chpr);

            }
            else
            {
                PUSHCOLOR(RED);
                chpr += printf(" SEEK FAILED reading Property List item %d at offset %lu",i,offset);
                POPCOLOR();
                break;
            }
                
            offset += 8;
            max_offset = ftell(inptr);
        }
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,max_offset - 1,indent + SMALLINDENT,"@");
            chpr += printf("</Property List iIndex>");
            chpr = newline(chpr);
            print_tag_address(SECTION,property_offset,indent + SMALLINDENT,"@");
            chpr += printf("<Property List Data> length %lu",list_data_length);
            chpr = newline(chpr);
        }
        offset = data_offset;

        /* Read 'em again, and this time show the names and values    */
        for(i = 0; i < num_entries; ++i)
        {
            if(fseek(inptr,offset,SEEK_SET) == 0)
            {
                name_offset = read_ulong(inptr,byteorder,HERE);
                name_offset *= 2;
                if(feof(inptr) || ferror(inptr))
                {
                    PUSHCOLOR(RED);
                    chpr += printf(" FAILED to read Property List name %d at offset %lu",i,offset);
                    POPCOLOR();
                    break;
                }
                value_offset = read_ulong(inptr,byteorder,HERE);
                value_offset *= 2;
                if(feof(inptr) || ferror(inptr))
                {
                    PUSHCOLOR(RED);
                    chpr += printf(" FAILED to read Property List value %d at offset %lu",i,offset + 4);
                    POPCOLOR();
                    break;
                }
                name_offset += property_offset;
                value_offset += property_offset;
                print_property(inptr,byteorder,name_offset,value_offset,char_format,
                                                    dirname,i,indent + MEDIUMINDENT);
            }
            else
            {
                PUSHCOLOR(RED);
                chpr += printf(" SEEK FAILED reading Property List item %d at offset %lu",i,offset);
                POPCOLOR();
                break;
            }
                
            offset += 8;
            max_offset = ftell(inptr);
        }
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,max_offset - 1,indent + SMALLINDENT,"@");
            chpr += printf("</Property List Data>");
            chpr = newline(chpr);
        }
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,property_offset + list_data_length - 1,indent,"@");
            chpr += printf("</Property List>");
            chpr = newline(chpr);
        }
    }
    else
    {
        PUSHCOLOR(RED);
        fprintf(stderr,"%s: no file pointer to read X3F Properties List\n",Progname);
        POPCOLOR();
    }
    setcharsprinted(chpr);
    return(max_offset);
}

/* Displays property names and values, given their offsets. The       */
/* characters are encoded as 16 bit 'unicode'. This routine reads     */
/* each character as an unsigned short and echos it via putchar().    */

void
print_property(FILE *inptr,unsigned short byteorder,unsigned long name_offset,
                        unsigned long value_offset,unsigned long char_format,
                        char *dirname,int index,int indent)
{
    int chpr = 0;
    int i = 0;
    int ch;

    if(inptr)
    {
        clearerr(inptr);
        if(fseek(inptr,name_offset,SEEK_SET) == 0)
        {
            if((PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
            {
                print_tag_address(ENTRY,name_offset,indent + SMALLINDENT,"@");
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.Data.",dirname);
                    chpr += printf("Name.%-2d",index);
                    if((PRINT_VALUE))
                    {
                        putindent(X3FTAGWIDTH - 17);
                        chpr += printf(" = ");
                    }
                }
                if((PRINT_VALUE))
                {
                    while((ch = read_ushort(inptr,byteorder,HERE)))
                    {
                        if((PRINT_UNICODE))
                            fwrite((char *)&ch,2,1,stdout);
                        else
                            putchar(ch);
                        ++chpr;
                    }
                }
            }
            else
            {
                print_tag_address(ENTRY,name_offset,indent + SMALLINDENT,"@");
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.Data.",dirname);
                    i = 0;
                    while((ch = read_ushort(inptr,byteorder,HERE)))
                    {
                        if((PRINT_UNICODE))
                            fwrite((char *)&ch,2,1,stdout);
                        else
                            putchar(ch);
                        ++chpr;
                        ++i;
                    }
                    if((PRINT_VALUE))
                    {
                        putindent(X3FTAGWIDTH - i - 6);
                        chpr += printf(" = ");
                    }
                }
            }
        }
        if((PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
            chpr = newline(chpr);
        if(fseek(inptr,value_offset,SEEK_SET) == 0)
        {
            if((PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
            {
                print_tag_address(ENTRY,value_offset,indent + SMALLINDENT,"@");
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",dirname);
                    chpr += printf("Value.%-2d",index);
                    if((PRINT_VALUE))
                    {
                        putindent(X3FTAGWIDTH - 18);
                        chpr += printf(" = ");
                    }
                }
                if((PRINT_VALUE))
                {
                    while((ch = read_ushort(inptr,byteorder,HERE)))
                    {
                        if((PRINT_UNICODE))
                            fwrite((char *)&ch,2,1,stdout);
                        else
                            putchar(ch & 0xff);
                        ++chpr;
                    }
                }
            }
            else if((PRINT_VALUE))
            {
                while((ch = read_ushort(inptr,byteorder,HERE)))
                {
                    if((PRINT_UNICODE))
                        fwrite((char *)&ch,2,1,stdout);
                    else
                        putchar(ch & 0xff);
                    ++chpr;
                }
            }
        }
        chpr = newline(chpr);
    }
}
