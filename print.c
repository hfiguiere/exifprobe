/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: print.c,v 1.51 2005/07/24 17:16:59 alex Exp $";
#endif

/* This file contains the primary "print" routines for TIFF/EXIF      */
/* data, file offsets, etc.                                           */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "maker_extern.h"
#include "ciff_extern.h"
#include "jp2_extern.h"
#include "mrw_extern.h"

/* Print the "header" information from the beginning of a file which  */
/* identifies the image type and/or image characteristics. Return 0   */
/* if all goes well, -1 if no header is passed or the header magic    */
/* number is invlid.                                                  */

int
print_header(struct fileheader *header,unsigned long section_id)
{
    int status = -1;
    int chpr = 0;

    if(header)
    {
        switch(header->probe_magic)
        {
        case JPEG_SOI:
            if(Print_options & section_id)
                chpr += printf("JPEG SOI %#06x",header->file_marker);
            status = 0;
            chpr = newline(chpr);
            break;
        case ORF1_MAGIC:
            status = print_tiff_header(ORF1_MAGIC,header->file_marker,section_id);
            /* no newline                                             */
            break;
        case ORF2_MAGIC:
            status = print_tiff_header(ORF2_MAGIC,header->file_marker,section_id);
            /* no newline                                             */
            break;
        case RW2_MAGIC:
            status = print_tiff_header(RW2_MAGIC,header->file_marker,section_id);
            /* no newline                                             */
            break;
        case TIFF_MAGIC:
            status = print_tiff_header(TIFF_MAGIC,header->file_marker,section_id);
            /* no newline                                             */
            break;
        case PROBE_CIFFMAGIC:
            status = print_ciff_header(header,section_id);
            chpr = newline(chpr);
            break;
        case PROBE_JP2MAGIC:
            status = print_jp2_header(header,section_id);
            chpr = newline(chpr);
            break;
        case PROBE_MRWMAGIC:
            status = print_mrw_header(header,section_id);
            chpr = newline(chpr);
            break;
        case PROBE_RAFMAGIC:
            chpr = printf("FUJIFILMCCD-RAW ");
            chpr = newline(chpr);
            break;
        default:
            if(Print_options & section_id)
                chpr += printf("UNKNOWN HEADER MAGIC %#06lx",header->probe_magic);
            chpr = newline(chpr);
            break;
        }
    }
    else
        fprintf(stderr,"%s: null fileheader to print_header()\n",Progname);
    return(status);
}


/* Print TIFF header information if options permit. Return 0 if       */
/* information is valid to print or does not need to be printed for   */
/* the identified section id. Return -1 if the magic number or        */
/* byteorder are invalid.                                             */

int
print_tiff_header(unsigned short magic,unsigned short byteorder,unsigned long section_id)
{
    char *name = CNULL;
    int status = -1;
    int chpr = 0;

    if(Print_options & section_id)
    {
        switch(magic)
        {
            case TIFF_MAGIC: name = "TIFF"; break;
            case ORF1_MAGIC: name = "ORF1"; break;
            case ORF2_MAGIC: name = "ORF2"; break;
            case RW2_MAGIC:  name = "RW2"; break;
            default: break;
        }

        if(name)
        {
            chpr += printf("%s",name);
            switch(byteorder)
            {
                case TIFF_INTEL:
                case TIFF_MOTOROLA:
                    print_byteorder(byteorder,1);
                    print_magic((unsigned long)magic,2);
                    status = 0;
                    break;
                default:
                    chpr += printf("INVALID %s BYTEORDER ",name);
                    print_byteorder(byteorder,1);
                    print_magic((unsigned long)magic,2);
                    break;
            }
        }
        else
        {
            chpr += printf("INVALID TIFF IDENTIFIER, byteorder ");
            print_byteorder(byteorder,1);
            print_magic((unsigned long)magic,2);
        }
    }
    else
        status = 0;

    setcharsprinted(chpr);
    return(status);
}

/* Print the file type as indicated by the "header" information at    */
/* the start of the file.                                             */

int
print_filetype(unsigned long magic,unsigned short marker)
{
    int status = -1;
    int chpr = 0;

    switch(magic)
    {
    case JPEG_SOI:
        chpr += printf("JPEG");
        status = 0;
        break;
    case ORF1_MAGIC:
        chpr += printf("ORF1");
        switch(marker)
        {
            case TIFF_INTEL:
                chpr += printf(":II");
                break;
            case TIFF_MOTOROLA:
                chpr += printf(":MM");
                break;
            default:
                break;
        }
        status = 0;
        break;
    case ORF2_MAGIC:
        chpr += printf("ORF2");
        switch(marker)
        {
            case TIFF_INTEL:
                chpr += printf(":II");
                break;
            case TIFF_MOTOROLA:
                chpr += printf(":MM");
                break;
            default:
                break;
        }
        status = 0;
        break;
    case RW2_MAGIC:
        chpr += printf("RW2");
        switch(marker)
        {
            case TIFF_INTEL:
                chpr += printf(":II");
                break;
            case TIFF_MOTOROLA:
                chpr += printf(":MM");
                break;
            default:
                break;
        }
        status = 0;
        break;
    case TIFF_MAGIC:
        chpr += printf("TIFF");
        switch(marker)
        {
            case TIFF_INTEL:
                chpr += printf(":II");
                break;
            case TIFF_MOTOROLA:
                chpr += printf(":MM");
                break;
            default:
                break;
        }
        status = 0;
        break;
    case PROBE_CIFFMAGIC:
        chpr += printf("CIFF");
        switch(marker)
        {
            case TIFF_INTEL:
                chpr += printf(":II");
                break;
            case TIFF_MOTOROLA:
                chpr += printf(":MM");
                break;
            default:
                break;
        }
        status = 0;
        break;
    case PROBE_JP2MAGIC:
        chpr += printf("JP2");
        status = 0;
        break;
    case PROBE_MRWMAGIC:
        chpr += printf("MRW");
        status = 0;
        break;
    case PROBE_RAFMAGIC:
        chpr += printf("RAF");
        status = 0;
        break;
    case PROBE_X3FMAGIC:
        chpr += printf("X3F");
        status = 0;
        break;
    default:
        chpr += printf("UNKNOWN");
        break;
    }
    setcharsprinted(chpr);
    return(status);
}

/* Print an address in the file (offset from beginning of file), in   */
/* hex, decimal, or both, offset from left margin by 'indent' spaces. */
/* "Addresses" are usually prefixed by an "@", but may be prefixed by */
/* a string passed as argument (this is normally used to pass a '-'   */
/* prefix to mark the end of a segment, or something to flag an       */
/* out-of-order segment).                                             */

/* If this routine is called with all PRINT_ADDRESS bits off, it     */
/* produces only the indent.                                          */

void
print_tag_address(unsigned long section_id,unsigned long address,int indent,char *prefix)
{
    int chpr = 0;

    if(Print_options & section_id)
    {
        print_filename();
        if(PRINT_INDENT_BEFORE_ADDRESS)
            putindent(indent);
        if(!prefix)
            prefix = "@";
        if(address == HERE)
        {
            /* special case; bland address field                      */
            if(PRINT_BOTH_ADDRESS)
                chpr += printf("%*s:  ",(ADDRWIDTH * 2) + 3,prefix);
            else if(PRINT_HEX_ADDRESS)
                chpr += printf("%*s:  ",ADDRWIDTH + 1,prefix);
            else if(PRINT_DEC_ADDRESS)
                chpr += printf("%*s:  ",ADDRWIDTH,prefix);
        }
        else
        {
            if(PRINT_BOTH_ADDRESS)
                chpr += printf("%s%#0*lx=%-*lu:  ",prefix,ADDRWIDTH+1,address,ADDRWIDTH,address);
            else if(PRINT_HEX_ADDRESS)
                chpr += printf("%s%#0*lx:  ",prefix,ADDRWIDTH+1,address);
            else if(PRINT_DEC_ADDRESS)
                chpr += printf("%s%-*lu:  ",prefix,ADDRWIDTH,address);
        }
        if(PRINT_INDENT_AFTER_ADDRESS)
            putindent(indent);
        setcharsprinted(chpr);
    }
}

/* Print the option-selected items in a TIFF IFD entry, including the */
/* tag number, tag name, tag type, and offset or value. If the        */
/* "value" doesn't fit in 4 bytes, print_offset_value() will be       */
/* called and may defer printing to be handled later (by the caller's */
/* second pass) or may print the offset, preceded by an '@' sign if   */
/* appropriate. If the value does fit in the 4 bytes provided in the  */
/* entry, print it according to its TIFF type, intercepting tags      */
/* which require special handling of value.                           */

unsigned long
print_entry(FILE *inptr,unsigned short byteorder,struct ifd_entry *entry_ptr,
            unsigned long fileoffset_base,struct image_summary *summary_entry,
            char *parent_name,int ifdtype,int ifdnum,int subifdnum,int indent)
{
    int value_is_offset = 0;
    unsigned long endoffset = 0UL;
    int chpr = 0;

    value_is_offset = is_offset(entry_ptr);
    if((PRINT_ENTRY))
    {
        print_taginfo(entry_ptr,parent_name,indent,ifdtype,ifdnum,subifdnum);
        if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
            ferror(inptr) || feof(inptr))
        {
            clearerr(inptr);
            printred("# INVALID ENTRY");
            endoffset = ftell(inptr);
        }
        else if(entry_ptr->count == 0)
        {
            if(PRINT_VALUE)
                chpr += printf(" EMPTY (value=%#lx)",entry_ptr->value);
            endoffset = ftell(inptr);
        }
        else if(value_is_offset)
        {
            /* Offsets read from the file are printed relative to the */
            /* beginning of the file (with fileoffset_base added) so  */
            /* that the values, printed later, will be easy to find.  */
            if(PRINT_BOTH_OFFSET)
                chpr += printf("@%#lx=%lu",entry_ptr->value + fileoffset_base,
                        entry_ptr->value + fileoffset_base);
            else if(PRINT_HEX_OFFSET)
                chpr += printf("@%#lx",entry_ptr->value + fileoffset_base);
            else if(PRINT_DEC_OFFSET)
                chpr += printf("@%lu",entry_ptr->value + fileoffset_base);

            if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                chpr += printf(" (%lu) ",entry_ptr->value);

            /* Print "inline" in REPORT & LIST modes                      */
            if(!(PRINT_VALUE_AT_OFFSET))
            {
                setcharsprinted(chpr);
                chpr = 0;
                endoffset = print_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,parent_name,
                                            ifdtype,indent,0);
            }
        }
        else if(PRINT_VALUE)
        {
            /* Intercept some EXIF values which print_value() would   */
            /* not handle properly, possibly because they are ascii   */
            /* items tagged as UNDEFINED. A few items given as LONG   */
            /* type are actually offsets, and are printed as such     */
            /* here, with fileoffset_base added.                      */
            switch(entry_ptr->tag)
            {
                case EXIFTAG_ExifVersion:
                case EXIFTAG_FlashPixVersion:
                case INTEROPTAG_Version:
                case INTEROPTAG_Index:
                    /* ASCII                                          */
                    /* ###%%% show_string()?                          */
                    chpr += printf("%s\'%s\'",PREFIX,(char *)&entry_ptr->value);
                    break;
                case EXIFTAG_ExifIFDPointer:
                case EXIFTAG_GPSInfoIFDPointer:
                case EXIFTAG_Interoperability:
                case TIFFTAG_JPEGInterchangeFormat:
                    /* OFFSETS                                        */
                    if(PRINT_BOTH_OFFSET)
                    {
                        chpr += printf("@%#lx=%lu",entry_ptr->value + fileoffset_base,
                                entry_ptr->value + fileoffset_base);
                    }
                    else if(PRINT_HEX_OFFSET)
                        chpr += printf("@%#lx",entry_ptr->value + fileoffset_base);
                    else if((PRINT_DEC_OFFSET) || is_a_long_offset(entry_ptr)) 
                        chpr += printf("@%lu",entry_ptr->value + fileoffset_base);

                    if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                        chpr += printf(" (%lu)",entry_ptr->value);
                    break;
                default:
                    /* VALUES                                         */
                    if(entry_ptr->tag == TIFFTAG_StripOffsets)
                        print_value(entry_ptr,"@");
                    else
                        print_value(entry_ptr,PREFIX);
                    /* This one is worth flagging...                  */
                    if((entry_ptr->tag == TIFFTAG_StripOffsets) &&
                                                (entry_ptr->value == 0))
                    {
                            printred(" (BAD START OFFSET)");
                    }
                    interpret_value(entry_ptr,summary_entry);
                    break;
            }
        }
        chpr = newline(chpr);
    }
    return(endoffset);
}


/* Some items are marked as LONG, but are intended to be interpreted  */
/* as offsets.                                                        */

int
is_a_long_offset(struct ifd_entry *entry_ptr)
{
    int is_an_offset = 0;

    if(entry_ptr)
    {
        switch(entry_ptr->tag)
        {
            case EXIFTAG_ExifIFDPointer:
            case EXIFTAG_GPSInfoIFDPointer:
            case EXIFTAG_Interoperability:
            case TIFFTAG_JPEGInterchangeFormat:
                ++is_an_offset;
                break;
            default:
                break;
        }
    }
    return(is_an_offset);
}

/* Repeat the tag identifier for values printed at an offset. This is */
/* called only when printing offset values at the offset (rather than */
/* "inline"), and repeats only one of the identifiers (preferably     */
/* tagname).                                                          */

void
print_tagid(struct ifd_entry *entry_ptr,int indent,int ifdtype)
{
    char *nameoftag;
    int chpr = 0;

    if(entry_ptr && (PRINT_ENTRY))
    {
        if(PRINT_TAGNAME || !(PRINT_TAGNO))
        {
            switch(ifdtype)
            {
                case GPS_IFD:
                    nameoftag = gps_tagname(entry_ptr->tag);
                    break;
                case INTEROP_IFD:
                    nameoftag = interop_tagname(entry_ptr->tag);
                    break;
                case EXIF_IFD:
                case TIFF_IFD:
                case TIFF_SUBIFD:
                default:
                    nameoftag = tagname(entry_ptr->tag);
                    break;
            }
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,nameoftag);
        }
        else if(PRINT_HEX_TAGNO)
            chpr += printf("<%#06x>",entry_ptr->tag & 0xffff);
        else if(PRINT_DEC_TAGNO)
            chpr += printf("<%5u>",entry_ptr->tag & 0xffff);
    }
    setcharsprinted(chpr);
}


/* Print the part of an IFD entry describing the entry tag, including */
/* it's tag number, name and type. Only the items enabled in          */
/* "Print_options" are printed.                                       */

/* When the LONGNAMES option bit is set prefix the tag name with a    */
/* section identifier (parent_name).                                  */

void
print_taginfo(struct ifd_entry *entry_ptr,char *parent_name,
                    int indent,int ifdtype,int ifdnum,int subifdnum)
{
    char *nameoftag;
    char *nameof_value_type;
    int tagwidth = TAGWIDTH;
    int chpr = 0;

    if(entry_ptr && (PRINT_ENTRY))
    {
        if(PRINT_BOTH_TAGNO)
            chpr += printf("<%#06x=%5u> ",entry_ptr->tag & 0xffff,entry_ptr->tag & 0xffff);
        else if(PRINT_DEC_TAGNO)
            chpr += printf("<%5u> ",entry_ptr->tag & 0xffff);
        else if(PRINT_HEX_TAGNO)
            chpr += printf("<%#06x> ",entry_ptr->tag & 0xffff);
        if((PRINT_TAGNAME))
        {
            switch(ifdtype)
            {
                case GPS_IFD:
                    nameoftag = gps_tagname(entry_ptr->tag);
                    break;
                case INTEROP_IFD:
                    nameoftag = interop_tagname(entry_ptr->tag);
                    break;
                case EXIF_IFD:
                case TIFF_IFD:
                case TIFF_SUBIFD:
                    nameoftag = tagname(entry_ptr->tag);
                    break;
                default:
                    nameoftag = tagname(entry_ptr->tag);
                    break;
            }
            if((PRINT_LONGNAMES))
                printf("%s.",parent_name);
            chpr += printf("%-*.*s",tagwidth,tagwidth,nameoftag ? nameoftag : "NULL");
        }
        if(PRINT_TYPE)
        {
            nameof_value_type = value_type_name(entry_ptr->value_type);
            chpr += printf(" [%-2u=%-9.9s %5lu] ",entry_ptr->value_type,
                    nameof_value_type,entry_ptr->count);
        }
        if(PRINT_VALUE)
            chpr += printf(" = "); 
    }
    setcharsprinted(chpr);
}

/* Print a "value" from an ifd structure entry, according to the TIFF */
/* type recorded in the entry. Print in hex or decimal (or both)      */
/* according to the global Print_options variable.                    */

void
print_value(struct ifd_entry *entry_ptr,char *prefix)
{
    unsigned long byte;
    unsigned short ushort_val;
    unsigned long i;
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        if(prefix)
            chpr += printf("%s",prefix);
        switch(entry_ptr->value_type)
        {
            case SBYTE:
                byte = entry_ptr->value;
                for(i = 0; i < entry_ptr->count; ++i)
                {
                    if(PRINT_BOTH_VALUE)
                        chpr += printf("%#x=%d",(unsigned int)((char)byte & 0xff),
                                    (int)((char)byte & 0xff));
                    else if(PRINT_HEX_VALUE)
                        chpr += printf("%#x",(unsigned int)((char)byte & 0xff));
                    else if(PRINT_DEC_VALUE)
                        chpr += printf("%d",(int)((char)byte & 0xff));
                    byte >>= 8;
                }
                break;
            case SHORT:
                ushort_val = entry_ptr->value ;
                if(PRINT_BOTH_VALUE)
                    chpr += printf("%#x=%u",ushort_val,ushort_val);
                else if(PRINT_HEX_VALUE)
                    chpr += printf("%#x",ushort_val);
                else if(PRINT_DEC_VALUE)
                    chpr += printf("%u",ushort_val);
                if(entry_ptr->count > 1)
                {
                    (void)putchar(','); ++chpr;
                    ushort_val = (entry_ptr->value & 0xffff0000) >> 16;
                    if(PRINT_BOTH_VALUE)
                        chpr += printf("%#x=%u",ushort_val,ushort_val);
                    else if(PRINT_HEX_VALUE)
                        chpr += printf("%#x",ushort_val);
                    else if(PRINT_DEC_VALUE)
                        chpr += printf("%u",ushort_val);
                }
                break;
            case SSHORT:
                ushort_val = entry_ptr->value & 0xffff;
                if(PRINT_BOTH_VALUE)
                    chpr += printf("%#x=%d",(unsigned int)ushort_val,
                                (int)ushort_val);
                else if(PRINT_HEX_VALUE)
                    chpr += printf("%#x",(unsigned int)ushort_val);
                else if(PRINT_DEC_VALUE)
                    chpr += printf("%d",(int)ushort_val);
                if(entry_ptr->count > 1)
                {
                    (void)putchar(',');
                    ushort_val = (entry_ptr->value & 0xffff0000) >> 16;
                    if(PRINT_BOTH_VALUE)
                        chpr += printf("%#x=%d",(unsigned int)ushort_val,
                                    (int)ushort_val);
                    else if(PRINT_HEX_VALUE)
                        chpr += printf("%#x",(unsigned int)ushort_val);
                    else if(PRINT_DEC_VALUE)
                        chpr += printf("%d",(int)ushort_val);
                }
                break;
            case LONG:
                if(PRINT_BOTH_VALUE)
                    chpr += printf("0x%08lx=%lu",entry_ptr->value,
                                entry_ptr->value);
                else if(PRINT_HEX_VALUE)
                    chpr += printf("0x%08lx",entry_ptr->value);
                else if(PRINT_DEC_VALUE)
                    chpr += printf("%lu",entry_ptr->value);
                break;
            case SLONG:
                if(PRINT_BOTH_VALUE)
                    chpr += printf("%#lx=%ld",(unsigned long)entry_ptr->value,
                                (long)entry_ptr->value);
                else if(PRINT_HEX_VALUE)
                    chpr += printf("%#lx",(unsigned long)entry_ptr->value);
                else if(PRINT_DEC_VALUE)
                    chpr += printf("%ld",(long)entry_ptr->value);
                break;
            case ASCII:
                byte = entry_ptr->value;
                if(entry_ptr->count <= 4)
                {
                    chpr += printf("\'");
                    for(i = 0; i < entry_ptr->count; ++i)
                    {
                        if(isprint(byte & 0xff))
                            (void)putchar(byte & 0xff);
                        else
                            chpr += printf("\\%03ld",byte & 0xff);
                        byte >>= 8;
                        ++chpr;
                    }
                    chpr += printf("\'");
                }
                break;
            case FLOAT:
                chpr += printf("%g",(float)entry_ptr->value);
                break;
            case DOUBLE: /* double is not possible in a direct ifd entry  */
                chpr += printf("# DOUBLE TYPE IS INVALID HERE");
                break;
            case UNDEFINED: /* fall through; handle these as BYTE         */
            case BYTE:
        default:
                byte = entry_ptr->value;
                for(i = 0; i < entry_ptr->count; ++i)
                {
                    if(i)
                        (void)putchar(',');
                    if(PRINT_BOTH_VALUE)
                        chpr += printf("%#lx=%lu",byte & 0xff,byte & 0xff);
                    else if(PRINT_HEX_VALUE)
                        chpr += printf("%#lx",byte & 0xff);
                    else if(PRINT_DEC_VALUE)
                        chpr += printf("%lu",byte & 0xff);
                    byte >>= 8;
                }
                /* UNDEFINED stuff may be ascii                   */
                byte = entry_ptr->value;
                if((entry_ptr->value_type == UNDEFINED) &&
                    (entry_ptr->count == 4) &&
                        isprint(byte & 0xff))   
                        
                {
                    chpr += printf(" = \'");
                    for(i = 0; i < entry_ptr->count; ++i)
                    {
                        if(isprint(byte & 0xff))
                            (void)putchar(byte & 0xff);
                        else
                            (void)putchar('.');
                        byte >>= 8;
                        ++chpr;
                    }
                    (void)putchar('\'');
                    ++chpr;
                }
                break;
        }
    }
    setcharsprinted(chpr);
}

/* Goto the offset given by an IFD entry whose value couldn't be      */
/* squeezed into the 4 bytes of the entry itself, and read the value  */
/* found at that offset, printing according to the TIFF type found in */
/* the entry. UNDEFINED entry types must/may be special-cased, else   */
/* their presence and size will be noted and the data skipped or      */
/* hexdumped.                                                         */

/* Returns the file pointer offset at the end of processing, which    */
/* may be 0 if the seek or read fails, or if the entry does not have  */
/* offset data, or the data is outside the current IFD (e.g.          */
/* JPEGTables).                                                       */

unsigned long
print_offset_value(FILE *inptr,unsigned short byteorder,
                struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
                char *parent_name,int ifdtype,int indent,
                int at_offset)
{
    unsigned long endoffset = 0L;
    unsigned long value_offset,dumplength,total;
    char *nameoftag,*data;
    int chpr = 0;

    if(inptr && entry_ptr)
    {
        value_offset = fileoffset_base + entry_ptr->value;
        endoffset = value_offset + 
            (value_type_size(entry_ptr->value_type) * entry_ptr->count);

        if(PRINT_VALUE)
        {
            if((at_offset && (PRINT_VALUE_AT_OFFSET)) ||
                    ((PRINT_OFFSET) && (entry_ptr->value_type != UNDEFINED)))
                chpr += printf(" = ");
            switch(entry_ptr->value_type)
            {
                case UNDEFINED: 
                    if(!at_offset && (PRINT_VALUE))
                    {
                        if(!(PRINT_OFFSET))
                            chpr += printf("@%lu:%lu",value_offset,entry_ptr->count);
                        else
                            chpr += printf(":%lu", entry_ptr->count);
                    }
                    switch(entry_ptr->tag)
                    {
                        case EXIFTAG_CFAPattern:
                            if(!at_offset && (PRINT_VALUE))
                                chpr += printf(" = ");
                            nameoftag = tagname(entry_ptr->tag);
                            setcharsprinted(chpr);
                            chpr = 0;
                            print_cfapattern(inptr,entry_ptr->count,
                                            byteorder,value_offset,
                                            parent_name,nameoftag);
                            break;
                        case EXIFTAG_UserComment:
                            if(!at_offset && (PRINT_VALUE))
                                chpr += printf(" = ");
                            setcharsprinted(chpr);
                            chpr = 0;
                            print_user_comment(inptr,entry_ptr->count,
                                                value_offset,byteorder);
                            break;
                        case TIFFTAG_META_0xc353:
                            /* This should probably be handled        */
                            /* through the 'private IFD' mechanism,   */
                            /* but do it here for now.                */
                            if(!at_offset && (PRINT_VALUE))
                                chpr += printf(" = ");
                            /* The first 8 bytes should say "UNICODE" */
                            if((data = (char *)read_bytes(inptr,8UL,value_offset)) && (strncmp(data,"UNICODE",8) == 0))
                            {
                                if(!(LIST_MODE))
                                    chpr += printf("UNICODE: ");
                                setcharsprinted(chpr);
                                chpr = 0;
                                print_unicode(inptr,entry_ptr->count - 8,HERE,byteorder);
                            }
                            else
                            {
                                chpr = newline(chpr);
                                dumplength = entry_ptr->count;
                                hexdump(inptr,value_offset,dumplength,dumplength,16,0,SUBINDENT);
                            }
                            break;
                        case TIFFTAG_PrintIM:
                        case EXIFTAG_MakerNote:
                        case TIFFTAG_JPEGTables:
                            break;
                        /* TIFFEP_CFAPATTERN is not quite the same as */
                        /* EXIF_CFAPattern: it will be handled in     */
                        /* interpret.c                                */
                        case TIFFEPTAG_CFAPattern:
                        case TIFFEPTAG_SpatialFrequencyResponse:
                        case EXIFTAG_SpatialFrequencyResponse:
                        case EXIFTAG_SpectralSensitivity:
                        case EXIFTAG_OECF:
                            /* fall through, until some routines are  */
                            /* written for these.                     */
                        default:
                            /* Could make a pseudo-tag for 'count' in */
                            /* LIST mode...                           */
                            if(at_offset)
                            {
                                if(Max_undefined == 0L)
                                    chpr += printf(" (not dumped, use -U)");
                                else
                                {
                                    /* Even in LIST mode              */
                                    if((Max_undefined == DUMPALL) ||
                                            (Max_undefined > entry_ptr->count))
                                        dumplength = entry_ptr->count;
                                    else 
                                        dumplength = Max_undefined;
                                    chpr = newline(1);
                                    hexdump(inptr,value_offset,entry_ptr->count,
                                                dumplength,12,indent,SUBINDENT);
                                }
                            }
                            /* make certain we're at the end          */
                            fseek(inptr,(long)(value_offset + entry_ptr->count),SEEK_SET);
                            break;
                    }
                    if(!at_offset && (PRINT_VALUE))
                        chpr += printf("    # UNDEFINED");
                    break;
                case BYTE:
                    if(entry_ptr->tag == DNGTAG_OriginalRawFileName)
                        print_ascii(inptr,entry_ptr->count,value_offset);
                    else
                        print_ubytes(inptr,entry_ptr->count,value_offset);
                    break;
                case SBYTE:
                    print_sbytes(inptr,entry_ptr->count,value_offset);
                    break;
                case ASCII:
                    print_ascii(inptr,entry_ptr->count,value_offset);
                    if(Debug & NOTE_DEBUG)
                    {
                        if(entry_ptr->tag == TIFFTAG_Make)
                            chpr += printf("  [%d]",maker_number(Make_name));
                        else if(entry_ptr->tag == TIFFTAG_Model)
                            chpr += printf("  [%d]",model_number(maker_number(Make_name),
                                                    Model_name,CNULL));
                    }
                    break;
                case SHORT:
                    if(entry_ptr->tag == TIFFTAG_StripOffsets)
                        chpr += printf("@");
                    print_ushort(inptr,entry_ptr->count,byteorder,value_offset);
                    if(entry_ptr->tag == TIFFTAG_StripByteCounts)
                    {
                        total = sum_strip_bytecounts(inptr,byteorder,
                                    entry_ptr->value + fileoffset_base,
                                    entry_ptr->count,entry_ptr->value_type);
                        chpr += printf(" = %lu",total);
                    }
                    break;
                case SSHORT:
                    print_sshort(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case LONG:
                    if(entry_ptr->tag == TIFFTAG_StripOffsets)
                        chpr += printf("@");
                    print_ulong(inptr,entry_ptr->count,byteorder,value_offset);
                    if(entry_ptr->tag == TIFFTAG_StripByteCounts)
                    {
                        total = sum_strip_bytecounts(inptr,byteorder,
                                    entry_ptr->value + fileoffset_base,
                                    entry_ptr->count,entry_ptr->value_type);
                        chpr += printf(" = %lu",total);
                    }
                    break;
                case SLONG:
                    print_slong(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case RATIONAL:
                    print_urational(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SRATIONAL:
                    print_srational(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case FLOAT:
                    print_float(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case DOUBLE:
                    print_double(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                default:
                    chpr += printf(" INVALID TYPE %#x",entry_ptr->value_type);
                    break;
            }
            setcharsprinted(chpr);
            chpr = 0;
            interpret_offset_value(inptr,entry_ptr,byteorder,fileoffset_base);
            chpr = newline(chpr);
        }
    }
    else
    {
        chpr += printf(" invalid call to print_offset_value() ");
        if(inptr == (FILE *)0)
            chpr += printf(" - no input file pointer ");
        if(entry_ptr == (struct ifd_entry *)0)
            chpr += printf(" - null entry pointer");
        chpr = newline(chpr);
    }
    return(endoffset);
}

/* Read 'count' unsigned bytes starting at 'offset' and print their   */
/* values in hex. If there are a lot, just print the first few, an    */
/* elipsis to admit that something is left out, then print the count  */
/* and the last value and its offset.                                 */

void
print_ubytes(FILE *inptr,unsigned long count,unsigned long offset)
{
    int i,j;
    unsigned short byte;
    int chpr = 0;

    if(count)
    {
        byte = read_ubyte(inptr,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        chpr += printf("%02x",(unsigned int)byte & 0xff);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i,offset += 2L)
        {
            byte = read_ubyte(inptr,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%02x",(unsigned int)byte & 0xff);
        }
    }
    else
    {
#define MAX 10UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            byte = read_ubyte(inptr,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%02x",(unsigned int)byte & 0xff);
        }
        if((count > j) && (ferror(inptr) == 0))
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - 1);
            byte = read_ubyte(inptr,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%02x (%lu)",(unsigned int)byte & 0xff,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
}

/* Read 'count' signed bytes starting at 'offset' and print their     */
/* values in decimale. If there are a lot, just print the first few,  */
/* an elipsis to admit that something is left out, then print the     */
/* last value and its offset.                                         */

void
print_sbytes(FILE *inptr,unsigned long count,unsigned long offset)
{
    int i,j;
    short byte;
    int chpr = 0;

    if(count)
    {
        byte = read_ubyte(inptr,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        chpr += printf("%d",byte);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i,offset += 2L)
        {
            byte = read_ubyte(inptr,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%d",byte);
        }
    }
    else
    {
#define MAX 10UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            byte = read_ubyte(inptr,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%d",byte);
        }
        if((count > j) && (ferror(inptr) == 0))
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - 1);
            byte = read_ubyte(inptr,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%d (%lu)",byte,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

/* Read 'count' unsigned short integers starting at 'offset' and      */
/* print their values. If there are a lot, just print the first few,  */
/* an elipsis to admit that something is left out, then print the     */
/* last value and it's offset.                                        */
void
print_ushort(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    unsigned short value;
    int chpr = 0;

    if(count)
    {
        value = read_ushort(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        if(PRINT_BOTH_VALUE)
            chpr += printf("%#x/%u",value,value);
        else if(PRINT_HEX_VALUE)
            chpr += printf("%#x",value);
        else if(PRINT_DEC_VALUE)
            chpr += printf("%u",value);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i,offset += 2L)
        {
            value = read_ushort(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_BOTH_VALUE)
                chpr += printf(",%#x/%u",value,value);
            else if(PRINT_HEX_VALUE)
                chpr += printf(",%#x",value);
            else if(PRINT_DEC_VALUE)
                chpr += printf(",%u",value);
        }
    }
    else
    {
#define MAX 5UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i,offset += 2L)
        {
            value = read_ushort(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_BOTH_VALUE)
                chpr += printf(",%#x/%u",value,value);
            else if(PRINT_HEX_VALUE)
                chpr += printf(",%#x",value);
            else if(PRINT_DEC_VALUE)
                chpr += printf(",%u",value);
        }
        if((count > j) && (ferror(inptr) == 0))
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 2L;
            value = read_ushort(inptr,byteorder,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_BOTH_VALUE)
                chpr += printf(",%#x/%u (%lu)",value,value,count);
            else if(PRINT_HEX_VALUE)
                chpr += printf(",%#x (%lu)",value,count);
            else if(PRINT_DEC_VALUE)
                chpr += printf(",%u (%lu)",value,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

/* Read 'count' signed short integers starting at 'offset' and print  */
/* their values. If there are a lot, just print the first 5, an       */
/* elipsis to admit that something is left out, then print the last   */
/* value and it's offset.                                             */

void
print_sshort(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    short value;
    int chpr = 0;

    if(count)
    {
        value = (short)read_ushort(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        chpr += printf("%d",value);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i,offset += 2L)
        {
            value = (short)read_ushort(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%d",value);
        }
    }
    else
    {
#define MAX 5UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i,offset += 2L)
        {
            value = (short)read_ushort(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%d",value);
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 2L;
            value = (short)read_ushort(inptr,byteorder,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%d (%lu)",value,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

/* Read 'count' unsigned long integers starting at 'offset' and print */
/* their values. If there are a lot, just print the first 3, an       */
/* elipsis to admit that something is left out, then print the last   */
/* value and it's offset.                                             */

void
print_ulong(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    unsigned long value;
    int chpr = 0;

    if(count)
    {
        value = read_ulong(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        if(PRINT_BOTH_VALUE)
            chpr += printf("%#lx/%lu",value,value);
        else if(PRINT_HEX_VALUE)
            chpr += printf("%#lx",value);
        else if(PRINT_DEC_VALUE)
            chpr += printf("%lu",value);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i,offset += 4L)
        {
            value = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_BOTH_VALUE)
                chpr += printf(",%#lx/%lu",value,value);
            else if(PRINT_HEX_VALUE)
                chpr += printf(",%#lx",value);
            else if(PRINT_DEC_VALUE)
                chpr += printf(",%lu",value);
        }
    }
    else
    {
#define MAX 3UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i,offset += 4L)
        {
            value = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_BOTH_VALUE)
                chpr += printf(",%#lx/%lu",value,value);
            else if(PRINT_HEX_VALUE)
                chpr += printf(",%#lx",value);
            else if(PRINT_DEC_VALUE)
                chpr += printf(",%lu",value);
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 4L;
            value = read_ulong(inptr,byteorder,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_BOTH_VALUE)
                chpr += printf(",%#lx/%lu (%lu)",value,value,count);
            else if(PRINT_HEX_VALUE)
                chpr += printf(",%#lx (%lu)",value,count);
            else if(PRINT_DEC_VALUE)
                chpr += printf(",%lu (%lu)",value,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

/* Read 'count' signed long integers starting at 'offset' and print   */
/* their values. If there are a lot, just print the first 3, an       */
/* elipsis to admit that something is left out, then print the last   */
/* value and it's offset.                                             */

void
print_slong(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    long value;
    int chpr = 0;

    if(count)
    {
        value = (long)read_ulong(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        chpr += printf("%ld",value);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i,offset += 4L)
        {
            value = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%ld",value);
        }
    }
    else
    {
#define MAX 3UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            value = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%ld",value);
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 4L;
            value = (long)read_ulong(inptr,byteorder,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%ld (%lu)",value,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

/* Read 'count' unsigned rational values starting at 'offset' and     */
/* print their values. If there are a lot, just print the first 3, an */
/* elipsis to admit that something is left out, then print the last   */
/* value and it's offset.                                             */

void
print_urational(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    unsigned long num,denom;
    int chpr = 0;

    if(count)
    {
        num = read_ulong(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        denom = read_ulong(inptr,byteorder,HERE);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        if(PRINT_RAW_VALUES)
            chpr += printf("%lu/%lu = ",num,denom);
        if(num == 0UL)
            chpr += printf("0");
        else
            chpr += printf("%g",(double)((double)num/(double)denom));
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i)
        {
            num = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            denom = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_RAW_VALUES)
                chpr += printf("%lu/%lu = ",num,denom);
            if(num == 0UL)
                chpr += printf(",0");
            else
                chpr += printf(",%g",(double)((double)num/(double)denom));
        }
    }
    else
    {
#define MAX 3UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            num = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            denom = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_RAW_VALUES)
                chpr += printf("%lu/%lu = ",num,denom);
            if(num == 0UL)
                chpr += printf(",0");
            else
                chpr += printf(",%g",(double)((double)num/(double)denom));
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 8UL;
            fseek(inptr,offset,SEEK_SET);
            num = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            denom = read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_RAW_VALUES)
                chpr += printf("%lu/%lu = ",num,denom);
            else
            if(num == 0UL)
                chpr += printf(",0 (%lu)",count);
            else
                chpr += printf(",%g (%lu)",(double)((double)num/(double)denom),count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

void
print_srational(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    long num,denom;
    int chpr = 0;

    if(count)
    {
        num = (long)read_ulong(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        denom = (long)read_ulong(inptr,byteorder,HERE);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        if(PRINT_RAW_VALUES)
            chpr += printf("%ld/%ld = ",num,denom);
        if(num == 0UL)
            chpr += printf("0");
        else
            chpr += printf("%g",(double)((double)num/(double)denom));
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i)
        {
            num = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            denom = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_RAW_VALUES)
                chpr += printf("%ld/%ld = ",num,denom);
            if(num == 0UL)
                chpr += printf(",0");
            else
                chpr += printf(",%g",(double)((double)num/(double)denom));
        }
    }
    else
    {
#define MAX 3UL
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            num = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            denom = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_RAW_VALUES)
                chpr += printf("%ld/%ld = ",num,denom);
            if(num == 0UL)
                chpr += printf(",0");
            else
                chpr += printf(",%g",(double)((double)num/(double)denom));
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 8UL;
            fseek(inptr,offset,SEEK_SET);
            num = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            denom = (long)read_ulong(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            if(PRINT_RAW_VALUES)
                chpr += printf("%ld/%ld = ",num,denom);
            if(num == 0UL)
                chpr += printf(",0 (%lu)",count);
            else
                chpr += printf(",%g (%lu)",(double)((double)num/(double)denom),count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

void
print_float(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    double data;
    int chpr = 0;

    if(count)
    {
        data = read_float(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
                setcharsprinted(chpr);
            return;
        }
        chpr += printf("%g",data);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i)
        {
            data = read_float(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%g",data);
        }
    }
    else
    {
#define MAX 3L
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            data = read_float(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%g",data);
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 4L;
            data = read_float(inptr,byteorder,offset);
            chpr += printf(",%g (%lu)",data,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}


void
print_double(FILE *inptr,unsigned long count,unsigned short byteorder,
                unsigned long offset)
{
    int i,j;
    double data;
    int chpr = 0;

    if(count)
    {
        data = read_double(inptr,byteorder,offset);
        if(feof(inptr) || ferror(inptr))
        {
            clearerr(inptr);
            setcharsprinted(chpr);
            return;
        }
        chpr += printf("%g",data);
    }
    if(PRINT_MULTIVAL_ALL)
    {
        for(i = 1; i < count; ++i)
        {
            data = read_double(inptr,byteorder,HERE);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%g",data);
        }
    }
    else
    {
#define MAX 3L
        j = count > MAX + 2 ? MAX : count;
        for(i = 1; i < j; ++i)
        {
            data = read_double(inptr,byteorder,HERE);
            chpr += printf(",%g",data);
        }
        if(count > j)
        {
            chpr += printf(" ... ");
            offset += (unsigned long)(count - j) * 8UL;
            data = read_double(inptr,byteorder,offset);
            if(feof(inptr) || ferror(inptr))
            {
                clearerr(inptr);
                setcharsprinted(chpr);
                return;
            }
            chpr += printf(",%g (%lu)",data,count);
            if(PRINT_ADDRESS)
                chpr += printf(" -> @%lu",offset);
        }
#undef MAX
    }
    setcharsprinted(chpr);
}

/* Print up to 'count' bytes read from the image file, starting at    */
/* 'offset', as an ascii string, expanding nulls and unprintable      */
/* characters.                                                        */

/* If PRINT_ASCII_IGNORE_LENGTH is set in Print_options, quit         */
/* printing (but not reading) if a null byte is encountered.          */

int
print_ascii(FILE *inptr,unsigned long count,unsigned long offset)
{
    int remaining;
    int nread;
    int ch,lastch;
    int chpr = 0;
    int indent = 0;

    remaining = count;
    nread = 0;
    if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
    {
        chpr += printf(" seek failed to offset %lu to print ascii",offset);
        chpr = newline(chpr);
    }
    else
    {
        indent = charsprinted();
        lastch = 'A';
        (void)putchar('\'');
        chpr++;
        while((remaining > 0) && !feof(inptr))
        {
            ch = fgetc(inptr);
            ++nread;
            --remaining;
            if(ch && isprint(ch))
            {
                if(isdigit(ch) && (lastch == '\0'))
                    chpr += printf("00");
                (void)putchar(ch);
                chpr++;
            }
            else if(ch == '\0')
            {
                if((PRINT_ASCII_IGNORE_LENGTH))
                {
                    /* read it but don't print any more               */
                    while((remaining-- > 0) && !feof(inptr))
                    {
                        ch = fgetc(inptr);
                        ++nread;
                    }
                    break;
                }
                chpr += printf("\\0");
            }
            else if((ch == '\n') && !(LIST_MODE))
            {
                chpr = newline(chpr);
                putindent(indent);
            }
            else if((ch == '\011') && !(LIST_MODE))
            {
                putchar(ch);
                ++ch;
            }
            else
                chpr += printf("\\%03o",(unsigned int)(ch & 0xff));
            lastch = ch;
        }
        (void)putchar('\'');
        chpr++;
        if(ferror(inptr))
            clearerr(inptr);
    }
    setcharsprinted(chpr);
    return(nread);
}

/* Write 16 bit 'unicode' data. The data is read as unsigned short    */
/* characters and normally printed via putchar() (unsigned char). If  */
/* the user specifically requests PRINT_UNICODE, blast out both       */
/* bytes.                                                             */
/* Any checks for UNICODE should be done by the caller.               */

int
print_unicode(FILE *inptr,unsigned long count,unsigned long offset,unsigned short byteorder)
{
    int remaining;
    int nread;
    int ch = 0;
    int chpr = 0;

    nread = 0;
    if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
    {
        chpr += printf(" seek failed to offset %lu to print unicode",offset);
        chpr = newline(chpr);
    }

    remaining = count / 2;
    putchar('\''); ++chpr;
    while((remaining > 0) && !feof(inptr))
    {
        if(ch == 0)
            chpr += printf("\\0");
        else
            ch = read_ushort(inptr,byteorder,HERE);
        ++nread;
        ++chpr;
        --remaining;
        /* Write both bytes iff requested by user                     */
        if((PRINT_UNICODE))
            (void)fwrite((char *)&ch,2,1,stdout);
        else
            putchar(ch);
    }
    putchar('\''); ++chpr;
    if(ferror(inptr))
        clearerr(inptr);

    setcharsprinted(chpr);
    return(nread);
}

/* Print an ascii string, expanding nulls and unprintable characters. */
/* Print up to a null byte, or up to 'maxchars', whichever occurs     */
/* first.                                                             */

/* ###%%% IGNORE_LENGTH?                                              */
int
show_string(char *string,int maxchars,int noquotes)
{
    int lastch;
    int chpr = 0;
    int indent = 0;
    int printed = 0;
    char *ch;

    if(string)
    {
        indent = charsprinted();
        lastch = 'A';
        if(!noquotes)
        {
            (void)putchar('\'');
            chpr++;
        }
        ch = string;
        while(printed < maxchars)
        {
            if(*ch == '\0')
            {
                printf("\\0");
                break;
            }
            else if(isprint(*ch))
            {
                if(isdigit(*ch) && (lastch == '\0'))
                    chpr += printf("00");
                (void)putchar(*ch);
                chpr++;
            }
            else if(*ch == '\n')
            {
                chpr = newline(chpr);
                putindent(indent);
            }
            else if(*ch == '\011')
            {
                putchar(*ch);
                ++ch;
            }
            else
                chpr += printf("\\%03o",(unsigned int)(*ch & 0xff));
            lastch = *ch;
            ++printed;
            ++ch;
        }
        if(!noquotes)
        {
            (void)putchar('\'');
            chpr++;
        }
    }
    setcharsprinted(chpr);
    return(printed);
}


/* Print the UserComment field from an EXIF segment, which is         */
/* expected at 'offset' in the file and should be 'count' bytes long. */
/* The field begins with an 8 byte "character code", followed by      */
/* 'count - 8' bytes of actual comment. The length is printed as      */
/* "%d+8" to give the user the usable length of the comment, but      */
/* remind that the 8 bytes is there. An "undefined" field is printed  */
/* as ascii, but checked to see if it is filled with nulls or spaces. */
/* Non-ascii comment types are simply hexdumped.                      */

void
print_user_comment(FILE *inptr,unsigned long count,unsigned long offset,
                                                unsigned short byteorder)
{
    char *data;
    int i;
    int chpr = 0;

    /* The first 8 bytes identify the character code                  */

    if(PRINT_ENTRY)
    {
        if(PRINT_SECTION)
            chpr += printf("length %lu+8: (CC=",count - 8);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            chpr += printf("seek failed to offset %lu to process user comment)",offset);
            chpr = newline(chpr);
        }
        else if((data = (char *)read_bytes(inptr,8UL,HERE)))
        {
            if(offset == HERE)
                offset = ftell(inptr);  /* need to return after check     */
            if((data[0] == 0) || (strncmp(data,"ASCII",5) == 0))
            {
                if(PRINT_SECTION)
                {
                    if(data[0] == 0)
                        chpr += printf("undefined");
                    else
                    {
                        for(i = 0; i < 8; ++i)
                        {
                            if(data[i] && isascii(data[i]))
                            {
                                (void)putchar(data[i]);
                                chpr++;
                            }
                        }
                    }
                    (void)putchar(')');
                    chpr++;
                    chpr += printf(" = ");
                }
                i = 8;
                if(PRINT_SECTION)
                {
                    /* check for all nulls                                   */
                    for( ; i < count; ++i)
                    {
                        if(fgetc(inptr) != '\0')
                            break;
                    }
                }
                if(i == count)
                    chpr += printf(" (%lu nulls)",count - 8);
                else
                {
                    /* check for all spaces                               */
                    if(fseek(inptr,offset + 8,SEEK_SET))
                    {
                        chpr += printf(" re-seek to comment offset %lu failed ",offset + 8);
                        chpr = newline(chpr);
                        why(stdout);
                    }
                    else
                    {
                        i = 8;
                        for( ; i < count; ++i)
                        {
                            if(fgetc(inptr) != ' ')
                                break;
                        }
                        if(i >= count)
                            chpr += printf("(%d spaces)",i - 8);
                        else
                        {
                            if(fseek(inptr,offset + i,SEEK_SET))
                            {
                                chpr += printf(" re-seek to comment offset %lu failed ",offset + 8);
                                why(stdout);
                            }
                            else
                            {
                                chpr++;
                                /* note that the above loop strips leading spaces */
                                if((i > 20) && (PRINT_SECTION))
                                    chpr += printf("(%d spaces)",i - 8);
                                print_ascii(inptr,count - i,offset + i);
                            }
                        }
                    }
                }
                if((LIST_MODE))
                {
                    chpr += printf(" # CC='");
                    if(data[0])
                    {
                        for(i = 0; i < 8; ++i)
                        {
                            if(data[i] && isascii(data[i]))
                            {
                                (void)putchar(data[i]);
                                chpr++;
                            }
                        }
                        putchar('\''); ++chpr;
                    }
                    else
                        chpr += printf("undefined'");
                }
            }
            else if(strncmp(data,"UNICODE",5) == 0)
            {
                if((PRINT_SECTION))
                {
                    for(i = 0; i < 8; ++i)
                    {
                        if(data[i] && isascii(data[i]))
                        {
                            (void)putchar(data[i]);
                            chpr++;
                        }
                    }
                }
                print_unicode(inptr,count - 8,HERE,byteorder);
                if((LIST_MODE))
                {
                    chpr += printf(" # CC='");
                    if(data[0])
                    {
                        for(i = 0; i < 8; ++i)
                        {
                            if(data[i] && isascii(data[i]))
                            {
                                (void)putchar(data[i]);
                                chpr++;
                            }
                        }
                        putchar('\''); ++chpr;
                    }
                    else
                        chpr += printf("undefined'");
                }
            }
            else
            {
                chpr = newline(chpr);
                hexdump(inptr,offset,count,count,16,0,SUBINDENT);
                chpr = newline(chpr);
            }
        }
        setcharsprinted(chpr);
    }
}

#define COLORSTACKSIZE  16
static int colorstack[COLORSTACKSIZE];
static int colorstack_index = -1;

void
pushcolor(int color)
{
    if(USE_COLOR)
    {
        if(colorstack_index < 0)
        {
            colorstack_index = 0;
            colorstack[0] = END_COLOR;
        }
        else if(colorstack_index >= (COLORSTACKSIZE - 1))
            colorstack_index = 0;

        printf("\033[%dm",color);
        colorstack[++colorstack_index] = color;
    }
}

void
popcolor()
{
    if(USE_COLOR)
    {
        if(colorstack_index > 0)
            colorstack[colorstack_index--] = END_COLOR;
        printf("\033[0m");
        if(colorstack_index >= 0)
            printf("\033[%dm",colorstack[colorstack_index]);
    }
}

void
printred(char *msg)
{
    int chpr = 0;

    PUSHCOLOR(RED);
    chpr = printf("%s",msg);
    POPCOLOR();
    setcharsprinted(chpr);
}

/* Print the Color Filter Array pattern in a human-readable form.     */
/* The array pattern is printed on a single line, as simply as        */
/* possible. This could be annoying for arrays larger than 2x2.       */

/* This prints the color characters...                                */
void
putcolorchar(unsigned short color)
{
    switch(color & 0xff)
    {
        case 0: putchar('R'); break;
        case 1: putchar('G'); break;
        case 2: putchar('B'); break;
        case 3: putchar('C'); break;
        case 4: putchar('M'); break;
        case 5: putchar('Y'); break;
        case 6: putchar('W'); break;
        default: putchar('?'); break;
    }
    setcharsprinted(1);
}

/* ...and this does the grunt work.                                   */

void
print_cfapattern(FILE *inptr,unsigned long count,unsigned short byteorder,
                    unsigned long offset,char *parent_name,char *nameoftag)
{
    unsigned short horiz_repeat;
    unsigned short hr = 0;
    unsigned short vert_repeat;
    unsigned short vr = 0;
    unsigned short color;
    unsigned long cfaoffset,hoffset;
    int h,v,show;
    int chpr = 0;
    int curindent = 0;

#define MAX_REPEAT  8
#define MAX_COLOR   6
    if((PRINT_SECTION))
        chpr += printf("length %lu: ",count);
    if(offset == HERE)
        cfaoffset = ftell(inptr);
    else
        cfaoffset = offset;
    horiz_repeat = read_ushort(inptr,byteorder,cfaoffset);
    vert_repeat = read_ushort(inptr,byteorder,HERE);

    if((horiz_repeat == 0) || (vert_repeat == 0) ||
        (horiz_repeat > MAX_REPEAT) || (vert_repeat > MAX_REPEAT))
    {
        /* This section checks for possible byteorder reversal in the */
        /* repeat counts.                                             */
        if(byteorder == TIFF_INTEL)
        {
            hr = read_ushort(inptr,TIFF_MOTOROLA,cfaoffset);
            vr = read_ushort(inptr,TIFF_MOTOROLA,HERE);
        }
        else
        {
            hr = read_ushort(inptr,TIFF_INTEL,HERE);
            vr = read_ushort(inptr,TIFF_INTEL,HERE);
        }
        if(PRINT_SECTION)
        {
            curindent = charsprinted() + chpr;
            chpr += printf("H=%#x/%u, V=%#x/%u ",horiz_repeat,vert_repeat,
                                                 horiz_repeat,vert_repeat);
            PUSHCOLOR(RED);
            chpr += printf(" (INVALID or BYTE SWAPPED!)*");
            POPCOLOR();
            chpr = newline(chpr);
        }
        else
        {
            PUSHCOLOR(RED);
            chpr += printf("%ux%u",horiz_repeat,vert_repeat);
            chpr += printf(" # WARNING: repeat counts INVALID or BYTE SWAPPED");
            POPCOLOR();
        }
        if((hr > 0) && (vr > 0) &&
            ((hr < MAX_REPEAT) || (vr < MAX_REPEAT)))
        {
            horiz_repeat = hr;
            vert_repeat = vr;
            if(PRINT_SECTION)
            {
                print_tag_address(ENTRY|APP_ENTRY,HERE,0,"*");
                curindent -= charsprinted();
                putindent(curindent);
            }
            else if(PRINT_TAGINFO)
            {
                chpr = newline(chpr);
                print_tag_address(ENTRY|APP_ENTRY,HERE,MEDIUMINDENT,"*");
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,nameoftag);
                if((PRINT_VALUE))
                    chpr += printf(" = ");
            }
        }
    }
    if(PRINT_SECTION)
    {
        chpr += printf("H=%#x/%u, V=%#x/%u ",horiz_repeat,vert_repeat,
                                             horiz_repeat,vert_repeat);
        /* Some sanity checks                                             */
        if((horiz_repeat == 0) || (vert_repeat == 0) ||
            (horiz_repeat > MAX_REPEAT) || (vert_repeat > MAX_REPEAT))
        {
            PUSHCOLOR(RED);
            chpr += printf(" INVALID REPEAT COUNT");
            POPCOLOR();
        }
        else
        {
            chpr += printf(" = ");
            hoffset = ftell(inptr);
            for(v = 0; v < vert_repeat; ++v)
            {
                putchar('[');
                ++chpr;
                /* show 2 repetitions */
                for(show = 0; show < 2; ++show)
                {
                    for(h = 0; h < horiz_repeat; ++h)
                    {
                        color = read_ubyte(inptr,hoffset+h);
                        if(ferror(inptr))
                            break;
                        if(color <= MAX_COLOR)
                        {
                            putcolorchar(color & 0xff);
                            putchar(',');
                            ++chpr;
                        }
                        else
                        {
                            putchar('?');
                            ++chpr;
                            setcharsprinted(chpr);
                            return;
                        }
                    }
                }
                chpr += printf(" ...],");
                hoffset = ftell(inptr);
            }
            chpr += printf(" ...");
            if(hr)
                printred(" (BYTE SWAPPED) ");
        }
    }
    else
    {
        chpr += printf("%dx%d",horiz_repeat,vert_repeat);
        if((horiz_repeat == 0) || (vert_repeat == 0) ||
            (horiz_repeat > MAX_REPEAT) || (vert_repeat > MAX_REPEAT))
        {
            PUSHCOLOR(RED);
            chpr += printf("# INVALID REPEAT COUNT");
            POPCOLOR();
        }
        else
        {
            print_startvalue();
            hoffset = ftell(inptr);
            for(v = 0; v < vert_repeat; ++v)
            {
                for(h = 0; h < horiz_repeat; ++h)
                {
                    color = read_ubyte(inptr,hoffset+h);
                    if(ferror(inptr))
                        break;
                    if(color < MAX_COLOR)
                    {
                        putcolorchar(color & 0xff);
                        if((h + 1) < horiz_repeat)
                            putchar(',');
                        ++chpr;
                    }
                    else
                    {
                        putchar('?');
                        ++chpr;
                        setcharsprinted(chpr);
                        return;
                    }
                }
                hoffset = ftell(inptr);
                if((v + 1) < vert_repeat)
                    putchar(':');
            }
            print_endvalue();
            if(hr)
                printred(" # BYTE SWAPPED");
        }
    }
    if(ferror(inptr))
        clearerr(inptr);
    setcharsprinted(chpr);
}

/* Dump a bunch of bytes in traditional hex/ascii format. The offset  */
/* and size of the chunk to dump are passed, with a separate argument */
/* for the number of bytes to print, which is constrained to be <=    */
/* the size. The per-line width of the output is also passed          */
/* (sometimes this program uses 16, sometimes 12 if the chunk might   */
/* be in IFD format).                                                 */

/* This function checks to assure that the buffer in read_bytes() is  */
/* not overrun (read_bytes() can't report truncation) and loops on    */
/* the data if necessary.                                             */

/* If the length of data to be printed is less than the actual size,  */
/* the last line is annotated with "ect..." to indicate that there is */
/* more. On the other hand, if 'max_to_print' expires before the last */
/* line is full, both fields are padded with spaces.                  */

/* NOTE: this routine never prints a final newline (intentionally)    */

void
hexdump(FILE *inptr,unsigned long offset,unsigned long size,
        unsigned long max_to_print,int width,int indent,
        int subindent)
{
    char *data,*next;
    int bytes = 0;
    int printed = 0;
    int etc = 0;
    unsigned long remaining,readlength;
    int i;
    int chpr = 0;

    if(inptr)
    {
        if((size > 0L) && (max_to_print > 0L))
        {
            if(size > MAXBUFLEN)
                readlength = MAXBUFLEN;
            else
                readlength = size;

            /* Always read in multiples of "width" if doing multiple  */
            /* reads.                                                 */
            if(size > readlength)
                readlength = (readlength / width) * width;
            if(size < max_to_print)
                max_to_print = size;

            if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
            {
                chpr += printf(" seek failed to offset %lu to print hex/ascii dump",offset);
                chpr = newline(chpr);
                why(stdout);
                return;
            }
            while((printed < max_to_print) && (data = (char *)read_bytes(inptr,readlength,HERE)))
            {
                if(feof(inptr) || ferror(inptr))
                {
                    clearerr(inptr);
                    setcharsprinted(chpr);
                    break;
                }
                if(max_to_print < readlength)
                {
                    readlength = max_to_print;
                    ++etc;
                }

                bytes = 0;
                while(bytes < readlength)
                {
                    next = data;
                    remaining = readlength - bytes;

                    /* start address of each line                     */
                    print_tag_address(ENTRY|APP_ENTRY,offset + printed,
                                        indent + SUBINDENT," ");
                    extraindent(subindent); 

                    /* This is the hex part                           */
                    for(i = 0; i < width; ++i)
                    {
                        if(i && ((i % 4) == 0))
                            putchar(' ');
                        if(i < remaining)
                            chpr += printf("%02x ",(unsigned int)(*data++ & 0xff));
                        else
                            chpr += printf("   ");
                    }

                    /* Now do it again for the ascii side             */
                    chpr += printf(" |");
                    remaining = readlength - bytes;
                    for(i = 0; i < width; ++i,++next,++bytes,++printed)
                    {
                        if(i >= remaining)      /* pad at the end     */
                            putchar(' ');
                        else if(isprint(*next))
                            putchar(*next);
                        else
                            putchar('.');  /* can't win 'em all  */
                        ++chpr;
                    }
                    if(printed < max_to_print)
                        chpr += printf("|\n");  /* every line except the last */
                    else
                        chpr += printf("|");
                }
                if((max_to_print - printed) < readlength)
                    readlength = max_to_print - printed;
            }
            if(etc)     /* didn't print the whole chunk...            */
                chpr += printf(" etc...");
        }
    }
    else
        printred(" INTERNAL ERROR: no input file to hexdump()\n");
    setcharsprinted(chpr);
}

/* indent the following text by printing 'indent' spaces              */
void
putindent(int spaces)
{
    int spaces_left;

    if(spaces > MAXINDENT)
        spaces = MAXINDENT;
    spaces_left = spaces;
    while(spaces_left-- > 0)
        (void)putchar(' ');
    setcharsprinted(spaces);
}

/* conditionally indent the following text                            */

void
extraindent(int spaces)
{
    if(PRINT_INDENT)
        putindent(spaces);
}

#define START_VALUE " = \'"
void
print_startvalue()
{
    printf("%s",START_VALUE);
    setcharsprinted(sizeof(START_VALUE));
}

#define END_VALUE   "\'"
void
print_endvalue()
{
    printf("%s",END_VALUE);
    setcharsprinted(sizeof(END_VALUE));
}

/* Print the TIFF byteorder. If verbose, print as "(hex=ascii)",      */
/* otherwise just print the 2 bytes as a string, with octal escapes   */
/* for unprintable bytes.                                             */

void
print_byteorder(unsigned short byteorder,int verbose)
{
    int chpr = 0;

    if(verbose == 1)
        chpr += printf("(");
    else if(verbose == 2)
        chpr += printf("\'");
    (void)show_string((char *)&byteorder,2,1);

    /* Check for a legitimate TIFF byteorder                          */
    if(verbose == 1)
    {
        switch(byteorder)
        {
            case TIFF_INTEL:
            case TIFF_MOTOROLA:
                chpr += printf("=%#06x)",byteorder);
                break;
            default:
                chpr +=printf("=%#06x=unknown)",byteorder);
                break;
        }
    }
    else if(verbose == 2)
        chpr += printf("\'");
    setcharsprinted(chpr);
}

/* Print a "magic number". The 'size' argument should be 2 for        */
/* unsigned short magic numbers, 4 for unsigned long.                 */

void
print_magic(unsigned long magic,int size)
{
    int chpr = 0;

    chpr += printf(" magic=%#06lx=",magic);
    show_string((char *)&magic,size,0);
    setcharsprinted(chpr);
}

/* Check and clear the status recorded by jpeg_status() from          */
/* process_jpeg_segments() and report anomalies. Output is preceded   */
/* by shell comment markers and normally appears at the end of a      */
/* line.                                                              */

void
print_jpeg_status()
{
    int status = 0;

    /* get and clear the status                                       */
    status = jpeg_status(status);

    if((PRINT_SEGMENT))
    {
        if(status == JPEG_NO_SOI)
            printred(" ### (JPEG_SOI NOT FOUND)");
        if(status == JPEG_HAD_ERROR)
            printred(" ### (INVALID JPEG MARKER)");
        else if(status == JPEG_NO_EOI)
            printred(" ### (JPEG_EOI NOT FOUND)");
        else if(status == JPEG_EARLY_EOI)
            printred(" (JPEG_EOI FOUND EARLY)");
    }
}

void
display_end_of_ifd(unsigned long max_ifd_offset,int ifdtype,int ifdnum,int subifdnum,
                                                int indent,char *ifdname,char *prefix)
{
    int chpr = 0;
    print_tag_address(SECTION,max_ifd_offset - 1,indent,prefix);
    switch(ifdtype)
    {
        case TIFF_IFD:
            if(PRINT_SECTION)
                chpr += printf("</IFD %d>",ifdnum);
            break;
        case TIFF_SUBIFD:
            if(PRINT_SECTION)
                chpr += printf("</SubIFD %d of IFD %d>",subifdnum,ifdnum);
            break;
        case INTEROP_IFD:
            if(PRINT_SECTION)
                chpr += printf("</Interoperability SubIFD>");
            break;
        case GPS_IFD:
            if(PRINT_SECTION)
                chpr += printf("</GPS SubIFD>");
            break;
        case EXIF_IFD:
            if(PRINT_SECTION)
                chpr += printf("</EXIF IFD>");
            break;
        case MAKER_SUBIFD:
            if(PRINT_SECTION)
                chpr += printf("</%s>",ifdname);
            break;
        default:
            if(PRINT_SECTION)
                chpr += printf("</UNKNOWN IFD TYPE %d>",ifdtype);
            break;
    }
    chpr = newline(chpr);
}



/* Provide a short hex/ascii dump of interesting sections. The        */
/* default dumplength can be increased via the -U option. This allows */
/* examination of the tables and CFA sections.                        */

void
dumpsection(FILE *inptr,unsigned long offset,unsigned long length,int indent)
{
    unsigned long dumplength;

    if(inptr)
    {
        if(Max_imgdump > 0L)
            dumplength = Max_imgdump;
        else
            dumplength = DEF_DUMPLENGTH;
        if(dumplength > length)
            dumplength = length;
        if(dumplength)
        {
            hexdump(inptr,offset,length,dumplength,16,indent,0);
            (void)newline(1);
        }
    }
}

/* ================= PRIVATE IFDs ====================                */

/* process_private_ifd() reads an ifd section which is defined        */
/* "privately" (e.g. by a camera maker for use in makernotes) and for */
/* which tag numbers are thus defined without reference to the TIFF   */
/* spec. Any interpretation of tag numbers, values, etc. must be      */
/* switched through routines which recognize the make of the camera   */
/* or other device which produced the file, and might potentially     */
/* need to recognize a "note version" as well.                        */

/* The routine currently assumes a single IFD (no IFD chaining),      */
/* which may be be short-sighted considering the inventive nature of  */
/* camera producers. Please stand by...                               */

/* There is a mild assumption that these IFDs will be contained in    */
/* makernotes, since the TIFF, JPEG and Exif specs do not provide for */
/* such things outside the maker blob.                                */

unsigned long
process_private_ifd(FILE *inptr,unsigned short byteorder,
                    unsigned long subifd_offset,unsigned long fileoffset_base,
                    unsigned long max_offset,unsigned short subifd_ident,
                    struct image_summary *summary_entry,char *parent_name,
                    char *prefix,int make,int model,int indent)
{
    struct ifd_entry *entry_ptr;
    unsigned long max_value_offset = 0L;
    unsigned long next_ifd_offset,current_offset,max_subifd_offset;
    unsigned long start_entry_offset,entry_offset,value_offset;
    int entry_num,num_entries,value_is_offset,use_second_pass;
    int chpr = 0;

    if(inptr == (FILE *)0)
    {
        fprintf(stderr,"%s: no open file pointer to read makernote %s subifd\n",
                Progname,parent_name);
        return(0L);
    }
    if(summary_entry)
        summary_entry->filesubformat |= FILESUBFMT_MNSUBIFD;

    /* If the section appears to be beyond the end of the parent ifd, */
    /* mark the start address to call attention.                      */
    if(max_offset && ((subifd_offset + fileoffset_base) > max_offset))
        prefix = "+";

    max_subifd_offset = max_offset;
    print_tag_address(SECTION,subifd_offset + fileoffset_base,indent,prefix);
    if(PRINT_SECTION)
        chpr += printf("<%s SubIFD> (in MakerNote)",parent_name);

    num_entries = read_ushort(inptr,byteorder,subifd_offset + fileoffset_base);
    if(ferror(inptr) || feof(inptr))
        goto blewit;
    current_offset = entry_offset = start_entry_offset = ftell(inptr);
    if(PRINT_SECTION)
    {
        chpr += printf(" %d entries ",num_entries);
        chpr += printf("starting at file offset %#lx=%lu",
                                            current_offset,current_offset);
            chpr = newline(chpr);
    }
    use_second_pass = value_is_offset = 0;

    /* The direct entries                                             */
    for(entry_num = 0; entry_num < num_entries; ++entry_num)
    {
        entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
        if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
            ferror(inptr) || feof(inptr))
        {
            int invalid_entry = 0;
            unsigned long limit_offset = 0L;

            print_tag_address(ENTRY,entry_offset,indent,prefix);
            chpr += printf(" INVALID MAKERNOTE SUBIFD ENTRY (%lu)",entry_ptr->value);
            chpr = newline(chpr);
            clearerr(inptr);
            current_offset = ftell(inptr);
            if(max_offset > 0)
                limit_offset = max_offset;
            else
            {
                if(fseek(inptr,0L,SEEK_END) != -1)
                {
                    limit_offset = ftell(inptr);
                    fseek(inptr,current_offset,SEEK_SET);
                }
            }
            /* If there's an error on input, or we can't check    */
            /* for absurd num_entries, give up.                   */
            if(!ferror(inptr) && (limit_offset > 0))
            {
                /* If the number of entries would read past the   */
                /* size of the IFD, or past EOF, give up          */
                if((entry_offset + (12 * num_entries)) < limit_offset)
                {
                    /* Limit the number of consecutive failures.  */
                    /* An apparently valid entry resets the count */
                    /* to 0.                                      */
                    if(invalid_entry++ < MAX_INVALID_ENTRIES)
                    {
                        entry_offset = current_offset;
                        continue;
                    }
                }
            }
            chpr = newline(chpr);
            goto blewit;
        }
        current_offset = ftell(inptr);

        print_tag_address(ENTRY,entry_offset,indent,prefix);
        value_offset = print_private_entry(inptr,byteorder,entry_ptr,fileoffset_base,
                                            subifd_ident,summary_entry,parent_name,
                                            prefix,make,model,SUBINDENT);
        if(value_offset == 0UL)
            value_offset = current_offset;
        if(value_offset > max_value_offset)
            max_value_offset = value_offset;
        if((PRINT_VALUE_AT_OFFSET) && (is_offset(entry_ptr)))
            ++use_second_pass;
        entry_offset = current_offset;
    }

    next_ifd_offset = read_ulong(inptr,byteorder,current_offset);

    /* offsets found in the next pass should be within the bounds of  */
    /* the parent ifd. The following helps to detect chunks that are  */
    /* written "out of place".                                        */

    /* There should be no next_ifd_offset (it should be 0)            */
    /* ###%%% prepare for chains?                                     */
    if(next_ifd_offset > 0L)
    {
        /* ###%%% report unwillingness to handle chained IFDs         */
        if((PRINT_ADDRESS) && (PRINT_VALUE) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(ENTRY,current_offset,indent,prefix);
            extraindent(SUBINDENT);
            chpr += printf("**** next IFD offset %lu  ",next_ifd_offset);
            next_ifd_offset += fileoffset_base;
            if(next_ifd_offset < ftell(inptr))
            {
                printred("BAD NEXT IFD OFFSET");
                next_ifd_offset = 0L;
            }
            else
                chpr += printf("(+ %lu = %#lx/%lu)",fileoffset_base,
                                        next_ifd_offset,next_ifd_offset);
            chpr = newline(chpr);
        }
        else
            next_ifd_offset += fileoffset_base;
        /* This should never happen                                   */
        max_subifd_offset = next_ifd_offset;
    }
    else
    {
        if((PRINT_ADDRESS) && (PRINT_VALUE) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(ENTRY,current_offset,indent,prefix);
            extraindent(SUBINDENT);
            chpr += printf("**** next IFD offset 0");
            chpr = newline(chpr);
        }
    }

    if(ferror(inptr) || feof(inptr))
    {
        chpr += printf(" READ NEXT IFD OFFSET FAILED ");
        chpr = newline(chpr);
        why(stdout);
        clearerr(inptr);    /* keep going...                          */
    }
    else
        current_offset = ftell(inptr);

    value_offset = current_offset;

    /* Second pass, to evaluate entries which are stored indirectly.  */
    /* This occurs when the value requires more than 4 bytes, AND for */
    /* certain LONG values which are intended to be used as offsets   */
    if(use_second_pass)
    {
        if((PRINT_VALUE) && (PRINT_ADDRESS) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(ENTRY,value_offset,indent,prefix);
            extraindent(SUBINDENT);
            chpr += printf("============= VALUES, %s IFD ============",parent_name);
            chpr = newline(chpr);
        }

        entry_offset = start_entry_offset;
        for(entry_num = 0; entry_num < num_entries; ++entry_num)
        {
            entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
            if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                ferror(inptr) || feof(inptr))
            {
                /* If the first pass made it through invalid entries, */
                /* this pass should just ignore them and quietly      */
                /* continue.                                          */
                clearerr(inptr);
                entry_offset = current_offset = ftell(inptr);
                continue;
            }
            current_offset = entry_offset = ftell(inptr);
            if(is_offset(entry_ptr) ||
                        private_value_is_offset(make,subifd_ident,entry_ptr->tag))
            {
                value_offset = print_offset_pe_value(inptr,byteorder,
                                            entry_ptr,fileoffset_base,
                                            subifd_ident,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,1);
            }
            if(value_offset > max_value_offset)
                max_value_offset = value_offset;
        }
    }
    else if(current_offset > max_value_offset)
        max_value_offset = current_offset;

    if(max_offset && (max_value_offset > max_offset))
    {
        max_value_offset = max_offset;
        print_tag_address(SECTION,max_value_offset - 1,indent,">");
    }
    else
        print_tag_address(SECTION,max_value_offset - 1,indent,"-");
    if(PRINT_SECTION)
    {
        chpr += printf("</%s SubIFD> (in MakerNote)",parent_name);
        chpr = newline(chpr);
    }

    return(max_value_offset);
blewit:
    clearerr(inptr);
    current_offset = ftell(inptr);
    print_tag_address(SECTION,current_offset - 1,indent,"-");
    if(PRINT_SECTION)
    {
        chpr += printf("</EXIF IFD>");
        chpr = newline(chpr);
    }
    return(0L);
}

unsigned long
print_private_entry(FILE *inptr,unsigned short byteorder,struct ifd_entry *entry_ptr,
                            unsigned long fileoffset_base, unsigned short subifd_ident,
                            struct image_summary *summary_entry, char *parent_name,
                            char *prefix,int make,int model,int indent)
{
    int value_is_offset = 0;
    unsigned long endoffset = 0UL;
    int chpr = 0;

    if((PRINT_ENTRY))
    {
        value_is_offset = is_offset(entry_ptr);
        if(value_is_offset == 0)
            value_is_offset = private_value_is_offset(make,subifd_ident,entry_ptr->tag);
        print_private_taginfo(entry_ptr,subifd_ident,parent_name,make,indent);
        if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
            ferror(inptr) || feof(inptr))
        {
            clearerr(inptr);
            printred("# INVALID ENTRY");
            endoffset = ftell(inptr);
        }
        else if(entry_ptr->count == 0)
        {
            if(PRINT_VALUE)
                chpr += printf(" EMPTY (value=%#lx)",entry_ptr->value);
            endoffset = ftell(inptr);
        }
        else if(value_is_offset)
        {
            if(value_is_offset > 0)
            {
                /* Offsets read from the file are printed relative to the */
                /* beginning of the file (with fileoffset_base added) so  */
                /* that the values, printed later, will be easy to find.  */
                if(PRINT_BOTH_OFFSET)
                    chpr += printf("@%#lx=%lu",entry_ptr->value + fileoffset_base,
                            entry_ptr->value + fileoffset_base);
                else if(PRINT_HEX_OFFSET)
                    chpr += printf("@%#lx",entry_ptr->value + fileoffset_base);
                else if(PRINT_DEC_OFFSET)
                    chpr += printf("@%lu",entry_ptr->value + fileoffset_base);

                if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                    chpr += printf(" (%lu) ",entry_ptr->value);
            }
            else if((value_is_offset < 0) && (PRINT_VALUE))
            {
                print_value(entry_ptr,PREFIX);
                interpret_pe_value(entry_ptr,subifd_ident,make,prefix);
            }

            /* Print "inline" in REPORT & LIST modes                      */
            if(!(PRINT_VALUE_AT_OFFSET))
            {
                setcharsprinted(chpr);
                chpr = 0;
                endoffset = print_offset_pe_value(inptr,byteorder,
                                            entry_ptr,fileoffset_base,
                                            subifd_ident,summary_entry,
                                            parent_name,"@",indent + MEDIUMINDENT,
                                            make,model,0);
            }
        }
        else if(PRINT_VALUE)
        {
            print_value(entry_ptr,PREFIX);
            interpret_pe_value(entry_ptr,subifd_ident,make,prefix);
        }
        chpr = newline(chpr);
    }
    return(endoffset);
}

int
private_value_is_offset(int make,unsigned short subifd_ident,unsigned short tag)
{
    int is_offset = 0;

    switch(make)
    {
        case MAKER_OLYMPUS:
            is_offset = olympus_private_value_is_offset(subifd_ident,tag);
            break;
        default:
            break;
    }
    return(is_offset);
}

void
print_private_taginfo(struct ifd_entry *entry_ptr,unsigned short subifd_ident,
                                        char *parent_name,int make,int indent)
{
    char *nameoftag = CNULL;
    char *nameof_value_type = CNULL;
    int tagwidth = TAGWIDTH;
    int chpr = 0;

    if(entry_ptr && (PRINT_ENTRY))
    {
        extraindent(indent);
        if(PRINT_BOTH_TAGNO)
            chpr += printf("<%#06x=%5u> ",entry_ptr->tag & 0xffff,entry_ptr->tag & 0xffff);
        else if(PRINT_DEC_TAGNO)
            chpr += printf("<%5u> ",entry_ptr->tag & 0xffff);
        else if(PRINT_HEX_TAGNO)
            chpr += printf("<%#06x> ",entry_ptr->tag & 0xffff);
        if((PRINT_TAGNAME))
        {
            nameoftag = private_tagname(make,entry_ptr->tag,subifd_ident);
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",tagwidth,tagwidth,nameoftag ? nameoftag : "NULL");
        }
        if(PRINT_TYPE)
        {
            nameof_value_type = value_type_name(entry_ptr->value_type);
            chpr += printf(" [%-2u=%-9.9s %5lu] ",entry_ptr->value_type,
                                    nameof_value_type,entry_ptr->count);
        }
        if(PRINT_VALUE)
            chpr += printf(" = "); 
    }
    setcharsprinted(chpr);
}

void
print_private_tagid(struct ifd_entry *entry_ptr,unsigned short subifd_ident,
                                                        int make,int indent)
{
    char *nameoftag;
    int chpr = 0;

    if(entry_ptr && (PRINT_ENTRY))
    {
        extraindent(indent);
        if(PRINT_TAGNAME || !(PRINT_TAGNO))
        {
            nameoftag = private_tagname(make,entry_ptr->tag,subifd_ident);
            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,nameoftag);
        }
        else if(PRINT_HEX_TAGNO)
            chpr += printf("<%#06x>",entry_ptr->tag & 0xffff);
        else if(PRINT_DEC_TAGNO)
            chpr += printf("<%5u>",entry_ptr->tag & 0xffff);
        /* ###%%% should be done just befor value is printed, not     */
        /* here                                                       */
        if(PRINT_VALUE)
            chpr += printf(" = ");
    }
    setcharsprinted(chpr);
}

char *
private_tagname(int make,unsigned short tag,unsigned short subifd_ident)
{
    static char unknown_buf[16];
    char *tagname = CNULL;

    switch(make)
    {
        case MAKER_OLYMPUS:
            tagname = olympus_private_tagname(tag,subifd_ident);
            break;
        default:
            break;
    }
    if(tagname == CNULL)
    {
        unknown_buf[0] = '\0';
        if(snprintf(unknown_buf,11,"TAG_%#06x",(int)tag) > 11)
        {
            printf(" bad tag %#x not converted\n",tag);
            why(stdout);
            tagname = "<BADTAG>";
        }
        else
            tagname = unknown_buf;
    }
    return(tagname);
}

/* Interpret private entry values                                     */

void
interpret_pe_value(struct ifd_entry *entry_ptr,unsigned short subifd_ident,
                                                    int make,char *prefix)
{
    if(entry_ptr)
    {
        switch(make)
        {
            case MAKER_OLYMPUS:
                olympus_interpret_pe_value(entry_ptr,subifd_ident,prefix);
                break;
            default:
                break;
        }
    }
}

unsigned long
print_offset_pe_value(FILE *inptr,unsigned short byteorder,struct ifd_entry *entry_ptr,
                    unsigned long fileoffset_base,unsigned short subifd_ident,
                    struct image_summary *summary_entry,char *parent_name,char*prefix,
                    int indent,int make,int model,int at_offset)
{
    unsigned long endoffset = 0L;

    if(inptr && entry_ptr)
    {
        endoffset = fileoffset_base + entry_ptr->value + 
            (value_type_size(entry_ptr->value_type) * entry_ptr->count);

        switch(make)
        {
            case MAKER_OLYMPUS:
                olympus_offset_pe_value(inptr,byteorder,entry_ptr,fileoffset_base,
                                                        subifd_ident,summary_entry,
                                                        parent_name,prefix,indent,
                                                        make,model,at_offset);
                break;
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,subifd_ident,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
        }
    }
    else
    {
        printred(" invalid call to print_offset_pe_makervalue ");
        if(inptr == (FILE *)0)
            printred(" no input file pointer ");
        if(entry_ptr == (struct ifd_entry *)0)
            printred(" null entry pointer");
    }
    if(ferror(inptr) || feof(inptr))
        clearerr(inptr);
    (void)newline(0);
    return(endoffset);
}

void
print_private_offset_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    unsigned short subifd_ident,char *parent_name, char *prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long dumplength;
    int chpr = 0;

    if((value_type_size(entry_ptr->value_type) * entry_ptr->count) > 4)
    {
        value_offset = fileoffset_base + entry_ptr->value;
        if(PRINT_VALUE)
        {
            if(at_offset)
            {
                print_tag_address(ENTRY,value_offset,indent,prefix);
                print_private_tagid(entry_ptr,subifd_ident,make,SUBINDENT);
            }
            else
            {
                if((PRINT_OFFSET) && (entry_ptr->value_type != UNDEFINED))
                    chpr += printf(" = ");
            }
            switch(entry_ptr->value_type)
            {
                case UNDEFINED: 
                    /* Could make a pseudo-tag for 'count' in LIST    */
                    /* mode...                                        */
                    if((PRINT_SECTION))
                        chpr += printf("length %-9lu # UNDEFINED",entry_ptr->count);
                    else
                    {
                        if(!(PRINT_OFFSET))
                            chpr += printf("@%lu",value_offset);
                        chpr += printf(":%lu     # UNDEFINED",entry_ptr->count);
                    }
                    if(Max_undefined == 0)
                    {
                        if((PRINT_SECTION))
                            printred(" (not dumped, use -U)");
                    }
                    else
                    {
                        /* Even in LIST mode                          */
                        if((Max_undefined == DUMPALL)
                            || (Max_undefined > entry_ptr->count))
                                dumplength = entry_ptr->count;
                        else 
                            dumplength = Max_undefined;
                        chpr = newline(1);
                        hexdump(inptr,entry_ptr->value + fileoffset_base,
                                    entry_ptr->count,dumplength,12,
                                    indent,SUBINDENT);
                    }
                    /* make certain we're at the end                  */
                    fseek(inptr,(long)(entry_ptr->value + fileoffset_base + entry_ptr->count),SEEK_SET);
                    break;
                case ASCII:
                    print_ascii(inptr,entry_ptr->count,value_offset); 
                    break;
                case BYTE:
                    print_ubytes(inptr,entry_ptr->count,value_offset);
                    break;
                case SBYTE:
                    print_sbytes(inptr,entry_ptr->count,value_offset);
                    break;
                case SHORT:
                    print_ushort(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SSHORT:
                    print_sshort(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case LONG:
                    print_ulong(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SLONG:
                    print_slong(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case RATIONAL:
                    print_urational(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SRATIONAL:
                    print_srational(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case FLOAT:
                    print_float(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case DOUBLE:
                    print_double(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                default:
                    chpr = printf(" INVALID TYPE %#x",entry_ptr->value_type);
                    break;
            }
        }
    }
    if(ferror(inptr) || feof(inptr))
        clearerr(inptr);
    setcharsprinted(chpr);
}

/* ================= END PRIVATE IFDS ==================              */
