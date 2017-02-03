/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2005 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: ciff.c,v 1.8 2005/07/24 17:03:18 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Canon CIFF/CRW routines                                            */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* The information coded here is derived from the CIFF specification  */
/* found at                                                           */
/*    http://xyrion.org/ciff/CIFFspecV1R04.pdf                        */

/* with additional information from Phil Harvey's website at:         */
/*    http://www.sno.phy.queensu.ca/~phil/exiftool/canon_raw.html     */
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
#include "ciff.h"
#include "canon_extern.h"
#include "maker_extern.h"


unsigned long
process_ciff(FILE *inptr,struct fileheader *header,unsigned long fileoffset_base,
                        unsigned long heaplength,struct image_summary *summary_entry,
                        char *parent_name,int level,int indent)
{
    struct ciff_header *ciffheader;
    unsigned long max_offset = 0UL;
    unsigned short byteorder = 0;
    unsigned long heap_start = 0UL;
    unsigned long offset_table_end = 0UL;

    if(inptr)
    {
        if(header)
        {
            if(header->ciff_header)
            {
                ciffheader = header->ciff_header;
                byteorder = ciffheader->byteorder;
                heap_start = fileoffset_base + ciffheader->headerlength;
                if(heaplength)
                    offset_table_end = fileoffset_base + heaplength;
                else if(fseek(inptr,0L,SEEK_END) == 0)
                    offset_table_end = ftell(inptr);
                if(ferror(inptr) == 0)
                {
                    max_offset = process_ciff_dir(inptr,heap_start,offset_table_end,
                                                    ciffheader->subtype,parent_name,
                                                    summary_entry,byteorder,
                                                    level,indent);
                    if(max_offset > 0L)
                        max_offset += 4;
                }
                else
                    fprintf(stderr,"%s: cannot read offset table\n",Progname);
            }
            else
                fprintf(stderr,"%s: null ciffheader to process_ciff()\n",Progname);
        }
        else
            fprintf(stderr,"%s: null fileheader to process_ciff()\n",Progname);
    }
    else
        fprintf(stderr,"%s: no open file pointer to read Print Image data\n",
                Progname);

    return(max_offset);
}

unsigned long
process_ciff_dir(FILE *inptr,unsigned long start_offset,unsigned long end_offset,
                char *subtype,char *dirname,struct image_summary * summary_entry,
                unsigned short byteorder,int level,int indent)
{
    struct ciff_direntry *entry;
    unsigned long dircount_loc;
    unsigned long max_dir_offset = 0L;
    unsigned long entry_offset,entry_offset_start;
    unsigned long next_entry_offset,dir_offset;
    unsigned short num_entries;
    char *tablename = CNULL;
    int i;
    int chpr = 0;

    dircount_loc = read_ulong(inptr,byteorder,end_offset - 4) + start_offset;
    num_entries = read_ushort(inptr,byteorder,dircount_loc);
    entry_offset = entry_offset_start = dircount_loc + 2;
    entry = NULL;

    max_dir_offset = start_offset;

    /* Show where this directory is in the heap                       */
    print_tag_address(SECTION|ENTRY,start_offset,indent,"@");
    if((PRINT_SECTION && dirname))
    {
        chpr += printf("<%s> HEAP, length %lu",dirname,end_offset - start_offset);
        chpr = newline(chpr);
    }
    else
    {
        if((PRINT_TAGINFO))
        {
            /* Always want to know WHICH heap offset                  */
            chpr += printf("%s.",dirname);
            chpr += printf("%-*.*s",CIFFTAGWIDTH,CIFFTAGWIDTH,"HeapOffset");
        }
        if((PRINT_VALUE))
        {
            if(PRINT_BOTH_OFFSET)
                chpr += printf(" = @%#lx=%lu",start_offset,start_offset);
            else if(PRINT_HEX_OFFSET)
                chpr += printf(" = @%#lx",start_offset);
            else 
                chpr += printf(" = @%lu",start_offset);
        }
        chpr = newline(chpr);
    }

    /* Now burrow down to the bottom, recursively. This will show the */
    /* lower level directories and entries first.                     */
    for(i = 0; i < num_entries; ++i)
    {
        entry = read_ciff_direntry(inptr,entry,byteorder,entry_offset);
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
        dir_offset =
            process_ciff_direntry(inptr,CIFF_INHEAP,byteorder,entry,start_offset,
                                            dirname,summary_entry,level,indent);
        entry_offset = next_entry_offset;
    }

    /* Next show the directory info; this will display higher level   */
    /* directories and their direct entry values as the recursive     */
    /* calls unwind back to the top.                                  */
    if(PRINT_SECTION)
    {
        if(level == 0)
        {
            if(subtype && (strncmp(subtype,"JPGM",4) == 0))
                tablename = splice(dirname,".","ImageProperties");
            else
                tablename = splice(dirname," ","offset table");
            print_tag_address(SECTION,dircount_loc,indent,"@");
            chpr += printf("<%s> %u entries",tablename,num_entries);
        }
        else
        {
            dirname = dirname ? dirname : QSTRING;
            print_tag_address(SECTION,dircount_loc,indent+MEDIUMINDENT,"@");
            chpr += printf("<%s DIRECTORY>, %u entries",dirname,num_entries);
            indent += MEDIUMINDENT;
        }
        chpr = newline(chpr);
    }
 
    entry_offset = entry_offset_start;
    max_dir_offset = start_offset;
    for(i = 0; i < num_entries; ++i)
    {
        entry = read_ciff_direntry(inptr,entry,byteorder,entry_offset);
        next_entry_offset = ftell(inptr);

        if((PRINT_SECTION)) 
        {
            print_tag_address(ENTRY,entry_offset,indent + MEDIUMINDENT,"@");
            print_ciff_taginfo(entry,start_offset);
        }
        if(PRINT_VALUE)
        {
            print_ciff_value(inptr,entry,byteorder,start_offset,entry_offset,
                                                    dirname,summary_entry
                                                    ,level,indent);
        }
        entry_offset = next_entry_offset;
        if(entry_offset > max_dir_offset)
            max_dir_offset = entry_offset;
    }
    /* Close off the directory; at the top, report the location of    */
    /* the offset table (or top level directory if JPGM) and loction  */
    /* where its location was found (at the end of the file).         */
    if(PRINT_SECTION)
    {
        if(level > 0)
        {
            print_tag_address(SECTION|ENTRY,max_dir_offset - 1,indent,"-");
            dirname = dirname ? dirname : QSTRING;
            chpr += printf("</%s DIRECTORY>\n",dirname);
            print_tag_address(SECTION|ENTRY,end_offset - 4,indent,"-");
            chpr += printf("%s DIRECTORY offset = %lu\n",dirname,dircount_loc);
            print_tag_address(SECTION|ENTRY,end_offset - 1,indent-MEDIUMINDENT,"-");
            chpr += printf("</%s>",dirname);
        }
        else
        {
            print_tag_address(SECTION|ENTRY,max_dir_offset - 1,indent,"-");
            /* tablename is dynamically allocated */
            tablename = tablename ? tablename : strdup(QSTRING);
            chpr += printf("</%s>\n",tablename);
            print_tag_address(SECTION|ENTRY,end_offset - 4,indent,"-");
            chpr += printf("%s location = @%lu",tablename,dircount_loc);
        }
    }
    else if(level == 0)
    {
        print_tag_address(SECTION|ENTRY,end_offset - 1,indent,"-");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES)) 
                chpr += printf("%s.",dirname);
            chpr += printf("OffsetTableOffset");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",dircount_loc);
    }
    chpr = newline(chpr);
    if(entry)
        free(entry);
    if(tablename)
        free(tablename);
    return(max_dir_offset);
}


/* Print the part of a CIFF entry describing the entry tag, including */
/* it's tag number, name and type. Only the items enabled in          */
/* "Print_options" are printed.                                       */

void
print_ciff_taginfo(struct ciff_direntry *entry,unsigned long fileoffset_base)
{
    char *nameoftag = NULL;
    int tagwidth = CIFFTAGWIDTH;
    unsigned short tag,format,location;
    int chpr = 0;

    if(entry && (PRINT_ENTRY))
    {
        tag = entry->type & CIFF_TYPEMASK;
        format = entry->type & CIFF_FORMATMASK;
        location = entry->type & CIFF_LOCATIONMASK;
        if(PRINT_BOTH_TAGNO)
            chpr += printf("<%#06x=%5u> ",tag,tag);
        else if(PRINT_DEC_TAGNO)
            chpr += printf("<%5u> ",tag);
        else if(PRINT_HEX_TAGNO)
            chpr += printf("<%#06x> ",tag);
        if((PRINT_TAGNAME))
        {
            nameoftag = cifftagname(tag);
            if(nameoftag)
            {
                chpr += printf("%-*.*s",tagwidth,tagwidth,nameoftag);
                free(nameoftag);
            }
        }
        if(PRINT_TYPE)
        {
            chpr += printf(" [%#06x ",format);
            switch(format)
            {
                case 0x0000: chpr += printf("BYTE  "); break;
                case 0x0800: chpr += printf("ASCII "); break;
                case 0x1000: chpr += printf("SHORT "); break;
                case 0x1800: chpr += printf("LONG  "); break;
                case 0x2000: chpr += printf("STRUCT"); break;
                case 0x2800: chpr += printf("SUBDIR"); break;
                case 0x3000: chpr += printf("SUBDIR"); break;
                case 0x3800:
                default:
                    chpr += printf("UNDEFINED");
                    break;
            }
            switch(location)
            {
                case 0x0000: chpr += printf(" INHEAP %10lu %10lu",entry->length,entry->offset); break;
                case 0x4000: chpr += printf(" INREC  %10lu %10lu",entry->length,entry->offset); break;
                case 0x8000: chpr += printf(" X8000  %10lu %10lu",entry->length,entry->offset); break;
                case 0xc000: chpr += printf(" XC000  %10lu %10lu",entry->length,entry->offset); break;
                default:
                    chpr += printf(" UNKNOWNLOC");
                    break;
            }
            chpr += printf("] ");
        }
        /* ###%%% should be done immediately before value is printed; */
        /* not here                                                   */
        if(PRINT_VALUE)
            chpr += printf(" = "); 
    }
    setcharsprinted(chpr);
}

/* Print the "value" portion of a directory entry. If the value is    */
/* contained fully within the entry, the direntry processor is needed */
/* to decode the tag value, otherwise, just print the address in the  */
/* heap where things have already been handled.                       */

void
print_ciff_value(FILE *inptr,struct ciff_direntry *entry, unsigned short byteorder,
                            unsigned long fileoffset_base,unsigned long entry_offset,
                            char *dirname,struct image_summary *summary_entry,
                            int level,int indent)
{
    unsigned short location;
    unsigned long max_dir_offset = 0;
    int chpr = 0;

    location = entry->type & CIFF_LOCATIONMASK;
    if(location == CIFF_INREC)
    {
        /* chpr += printf("INREC"); */
        max_dir_offset = process_ciff_direntry(inptr,CIFF_INREC,byteorder,entry,
                                        entry_offset,dirname,summary_entry,level,
                                        indent);
    }
    else if(PRINT_SECTION)
    {
        if(PRINT_BOTH_OFFSET)
            chpr += printf("@%#lx=%lu",entry->offset + fileoffset_base,
                    entry->offset + fileoffset_base);
        else if(PRINT_HEX_OFFSET)
            chpr += printf("@%#lx",entry->offset + fileoffset_base);
        else /* default to decimal offset if nothing is selected      */
            chpr += printf("@%lu",entry->offset + fileoffset_base);

        if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
            chpr += printf(" (%lu) ",entry->offset);
        chpr = newline(chpr);
    }
    setcharsprinted(chpr);
}

unsigned long
canon_colorspace(FILE *inptr,unsigned short byteorder,char *tagprefix,
                            unsigned long offset,unsigned long dirlength,int indent)
{
    unsigned long count,value;
    unsigned long end_offset;
    int i;
    int chpr = 0;

    if(PRINT_SECTION)
    {
        chpr += printf(", length %lu",dirlength);
        chpr = newline(chpr);
    }

    end_offset = offset + dirlength;
    count = (dirlength / sizeof(unsigned short));

    /* There should be just one (no idea why they didn't do it        */
    /* INRECORD), and there is no 2-byte count                        */
    for(i = 0; i < count; ++i)
    {
        value = read_ushort(inptr,byteorder,offset);
        print_tag_address(ENTRY,offset,indent,"@");
        if((PRINT_TAGINFO))
        {
            if(tagprefix && (PRINT_LONGNAMES))
                chpr = printf("%s.",tagprefix);
            chpr = printf("%-*.*s",CIFFTAGWIDTH,CIFFTAGWIDTH,"type");
        }
        if((PRINT_VALUE))
        {
            chpr += printf(" = %lu ",value);
            if(value == 1)
                chpr += printf(" = \"real-world\"");
            else if(value == 2)
                chpr += printf(" = \"document\"");
            else if(value == 0xfff)
                chpr += printf(" = \"uncalibrated\"");
            else
                chpr += printf(" = \"undefined\"");
        }
        chpr = newline(chpr);
        offset += 2;
    }
    setcharsprinted(chpr);
    return(end_offset);
}

/* Decode the "capture time"; print in the Exif "DateTimeOriginal"    */
/* format.                                                            */
unsigned long
canon_ct_to_datetime(FILE *inptr,unsigned short byteorder,char *tagname,unsigned long offset,
                                                unsigned long dirlength,int indent)
{
    unsigned long end_offset = 0UL;
    unsigned long count;
    unsigned long timezone;
    unsigned long zoneinfo;
    struct tm *ts_time;
    long timestamp;
    int chpr = 0;

    if(PRINT_SECTION)
    {
        chpr += printf(", length %lu",dirlength);
        chpr = newline(chpr);
    }

    count = dirlength / sizeof(unsigned long);
    end_offset = offset + dirlength;
    print_tag_address(ENTRY,offset,indent+MEDIUMINDENT,"@");
    tagname = tagname ? tagname : QSTRING;
    if(PRINT_TAGNAME)
        chpr += printf("%s",tagname);
    if(PRINT_VALUE)
    {
        chpr += printf(" = ");
        print_ulong(inptr,count,byteorder,offset);
    }
    timestamp = read_ulong(inptr,byteorder,offset);
    timezone = read_ulong(inptr,byteorder,HERE);
    zoneinfo = read_ulong(inptr,byteorder,HERE);
    if(PRINT_VALUE)
    {
        ts_time = gmtime(&timestamp);
        chpr += printf(" = %02d:%02d:%02d %02d:%02d:%02d GMT",ts_time->tm_year+1900,ts_time->tm_mon+1,
                ts_time->tm_mday,ts_time->tm_hour,ts_time->tm_min,ts_time->tm_sec);
        if((zoneinfo & 0x80000000) && timezone)
            chpr += printf(" + %lu",timezone);
    }
    chpr = newline(chpr);

    return(end_offset);
}

/* Print a "value" from a CIFF structure entry, according to the CIFF */
/* type recorded in the entry. Print in hex or decimal (or both)      */
/* according to the global Print_options variable.                    */

/* This is generally undefined data, usually with unknown tagname or  */
/* data interpretation. It may be large, so line-by-line output is    */
/* avoided in favor of the abbreviated 'print_TYPE()' routines, and   */
/* an opportunity is provided to hex/ascii dump the section .         */

unsigned long
print_ciffinheapdata(FILE *inptr,unsigned short byteorder,char *dirname,
            char *tagname,unsigned long offset,unsigned long dirlength,
            unsigned short data_format,int indent)
{
    unsigned long end_offset = 0UL;
    unsigned long dumplength;
    unsigned short count;
    int chpr = 0;

    if(inptr)
    {
        end_offset = offset + dirlength;
        switch(data_format)
        {
            case 0x0000: count = dirlength; break;
            case 0x0800: count = dirlength; break;
            case 0x1000: count = dirlength/2; break;
            case 0x1800: count = dirlength/4; break;
            default: count = dirlength; break;  /* handle as bytes?   */
        }
        dirname = dirname ? dirname : QSTRING;
        tagname = tagname ? tagname : QSTRING;
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION|ENTRY,offset, indent,"@");
            chpr += printf("<%s.%s> data, %u entries, length %lu",dirname,tagname,count,dirlength);
            chpr = newline(chpr);
        }
        if((PRINT_TAGINFO))
        {
            print_tag_address(ENTRY,offset,indent+MEDIUMINDENT,"@");
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",dirname);
            chpr += printf("%-*.*s",CIFFTAGWIDTH,CIFFTAGWIDTH,tagname);
        }
        if(PRINT_VALUE)
        {
            chpr += printf(" = ");
            switch(data_format)
            {
                case 0x1000:    /* DC_USHORT                      */
                            print_ushort(inptr,count,byteorder,offset);
                            break;
                case 0x1800:   /* DC_UINT32:                      */
                            print_ulong(inptr,count,byteorder,offset);
                            break;
                case 0x0800:     /* DC_ASCII:                     */
                            print_ascii(inptr,count,offset);
                            break;
                case 0x0000:     /* DC_BYTE:                      */
                            print_ubytes(inptr,count,offset);
                            break;
                default:
                            chpr += printf("(data format %#x)",data_format);
                            break;
            }
            if(Max_undefined > 0L)
            {
                if((Max_undefined == DUMPALL) ||
                        (Max_undefined > dirlength))
                    dumplength = dirlength;
                else 
                    dumplength = Max_undefined;
                chpr = newline(chpr);
                hexdump(inptr,offset,dirlength,dumplength,16,indent+MEDIUMINDENT,
                                                        MEDIUMINDENT);
                chpr = newline(chpr);
            }
        }
        if(PRINT_SECTION)
        {
            chpr = newline(chpr);
            print_tag_address(SECTION|ENTRY,end_offset - 1, indent,"-");
            dirname = dirname ? dirname : QSTRING;
            chpr += printf("</%s.%s>",dirname,tagname);
        }
        setcharsprinted(chpr);
    }
    return(end_offset);
}

/* Print a "value" from a CIFF directory entry when the value is      */
/* contained within the record, rather than INHEAP. The data is       */
/* printed according to the CIFF type recorded in the entry, in hex   */
/* or decimal (or both) according to the global Print_options         */
/* variable.                                                          */

/* This is generally undefined data, usually with unknown tagname or  */
/* data interpretation.                                               */

unsigned long
print_ciffinrecdata(FILE *inptr,unsigned short byteorder,char *tagname,
                                unsigned long offset,unsigned long dirlength,
                                unsigned short data_format,int indent)
{
    unsigned long end_offset = 0UL;
    unsigned short count;
    int chpr = 0;

    if(inptr)
    {
        end_offset = offset + dirlength;
        if((PRINT_VALUE))
        {
            count = dirlength;
            chpr = count;
            switch(data_format)
            {
                case 0x1000:    /* DC_USHORT                          */
                            print_ushort(inptr,count/2,byteorder,offset);
                            break;
                case 0x1800:    /* DC_UINT32:                         */
                            print_ulong(inptr,count/4,byteorder,offset);
                            break;
                case 0x0800:    /* DC_ASCII:                          */
                            print_ascii(inptr,count,offset); 
                            break;
                case 0x0000:    /* DC_BYTE:                           */
                            print_ubytes(inptr,count,offset);
                            break;
                default:
                            chpr += printf("(data format %#x)",data_format);
                            break;
            }
            setcharsprinted(chpr);
        }
    }
    return(end_offset);
}

unsigned long
process_ciff_direntry(FILE *inptr,unsigned short location,unsigned short byteorder,
                        struct ciff_direntry *entry, unsigned long fileoffset_base,
                        char *dirname,struct image_summary *summary_entry,int level,
                        int indent)
{
    unsigned long max_offset = 0;
    unsigned long endofdir;
    unsigned short tag,marker,shvalue;
    char *nameoftag = NULL;
    char *fullnameoftag = NULL;
    char *tagname,*makename;
    int entrywidth = CIFFTAGWIDTH;
    int status = 0;
    int chpr = 0;
    int parindent = 0;
    int make,model,tagwidth;
    int makelen;
    float fvalue;

    if(entry)
    {
        if((entry->type & CIFF_LOCATIONMASK) == location)
        {
            chpr = 0;
            max_offset = fileoffset_base;
            /* ###%%%
            if((entry->type & CIFF_LOCATIONMASK) == CIFF_INHEAP)
                chpr += printf("INHEAP");
            */
            tag = entry->type & CIFF_TYPEMASK;
            nameoftag = cifftagname(tag);
            fullnameoftag = splice(dirname,".",nameoftag);
            if(PRINT_LONGNAMES)
            {
                tagname = fullnameoftag;
                if(dirname)
                    entrywidth += strlen(dirname) + 1;
            }
            else
                tagname = nameoftag;
            
            switch(tag)
            {
                case CIFFTAG_COLORINFO2:    /* fall through           */
                case CIFFTAG_COLORINFO1:    /* INHEAPDATA             */
                    if((PRINT_SECTION))
                        indent += MEDIUMINDENT;
                    max_offset = print_ciffinheapdata(inptr,byteorder,dirname,
                            nameoftag,entry->offset + fileoffset_base,entry->length,
                            entry->type&CIFF_FORMATMASK,indent/* +MEDIUMINDENT*/);
                    break;
                case CIFFTAG_ROMOPERATIONMODE:  /* ASCII INREC        */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent+MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                    }
                    if(PRINT_VALUE)
                    {
                        print_ascii(inptr,8,fileoffset_base + 2);
                        max_offset = fileoffset_base + 2 + 8;
                    }
                    break;
                case CIFFTAG_RAWMAKEMODEL: /* ###%%% ASCII SPECIAL    */
                    print_tag_address(ENTRY,entry->offset + fileoffset_base,indent+MEDIUMINDENT,
                                                                                "@");
                    makename = (char *)read_bytes(inptr, entry->length,
                                                    entry->offset + fileoffset_base);
                    max_offset = ftell(inptr);
                    Make_name = strdup(makename);
                    makelen = strlen(makename);
                    Model_name = strdup(makename + makelen + 1);
                    dirname = dirname ? dirname : QSTRING;
                    if((LIST_MODE))
                    {
                        /* Make pseudo-tags and separate Make and     */
                        /* Model                                      */
                        if(PRINT_TAGNAME)
                            chpr += printf("%s.%-*.*s",dirname,CIFFTAGWIDTH,CIFFTAGWIDTH,"Make");
                        if(PRINT_VALUE)
                        {
                            chpr += printf(" = \"%s\"",Make_name);
                            if(Use_Make_name)
                                chpr += printf(" (Using: Make=%s)",Use_Make_name);
                        }
                        chpr = newline(chpr);
                        print_tag_address(ENTRY,entry->offset + fileoffset_base + strlen(Make_name) + 1,
                                                                            indent+MEDIUMINDENT,"@");
                        if(PRINT_TAGNAME)
                            chpr += printf("%s.%-*.*s",dirname,CIFFTAGWIDTH,CIFFTAGWIDTH,"Model");
                        if(PRINT_VALUE)
                        {
                            chpr += printf(" = \"%s\"",Model_name);
                            if(Use_Make_name)
                                chpr += printf(" (Using: Model=%s)",Use_Model_name);
                        }
                    }
                    else
                    {
                        if(PRINT_TAGNAME)
                        {
                            if((PRINT_SECTION))
                                chpr += printf("%s.%-*.*s",dirname,CIFFTAGWIDTH,CIFFTAGWIDTH,nameoftag);
                            else
                                chpr += printf("%-*.*s",CIFFTAGWIDTH,CIFFTAGWIDTH,nameoftag);
                        }
                        if(PRINT_VALUE)
                        {
                            chpr += printf(" = ");
                            if((PRINT_ASCII_IGNORE_LENGTH))
                                chpr += printf("%s\\0%s",Make_name,Model_name);
                            else
                                print_ascii(inptr,entry->length,entry->offset + fileoffset_base);
                            if(Use_Make_name)
                                chpr += printf(" Using: Make=%s",Use_Make_name);
                            if(Use_Model_name)
                                chpr += printf(" Using: Model=%s",Use_Model_name);
                        }
                    }
                    break;
                case CIFFTAG_FILEDESCRIPTION:   /* ASCII INHEAP       */
                case CIFFTAG_FIRMWAREVERSION:   /* ASCII INHEAP       */
                case CIFFTAG_OWNERNAME:         /* ASCII INHEAP       */
                case CIFFTAG_IMAGETYPE:         /* ASCII INHEAP       */
                case CIFFTAG_ORIGINALFILENAME:  /* ASCII INHEAP       */
                case CIFFTAG_THUMBNAILFILENAME: /* ASCII INHEAP       */
                    print_tag_address(ENTRY,entry->offset + fileoffset_base,indent+MEDIUMINDENT,"@");
                    if((PRINT_TAGINFO))
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                    if(PRINT_VALUE)
                    {
                        chpr += printf(" = ");
                        print_ascii(inptr,entry->length,entry->offset + fileoffset_base);
                        max_offset = ftell(inptr);
                    }
                    break;
                case CIFFTAG_BASEISO:       /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent+MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                    }
                    if(PRINT_VALUE)
                        chpr += printf("%lu",entry->length);
                    max_offset = ftell(inptr);
                    break;
                case CIFFTAG_FOCALLENGTH:     /* INREC                */
                    /* length mm, xy size in .001 inch ###%%% ???     */
                    if((PRINT_VALUE) && (PRINT_SECTION))
                    {
                        indent += 19;
                        chpr = printf("{");
                        chpr = newline(chpr);
                        parindent = charsprinted() - LARGEINDENT;
                        indent += MEDIUMINDENT;
                    }
                    if(PRINT_ENTRY)
                    {
                        entrywidth = strlen("TargetCompressionRatio");
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        parindent = charsprinted() - LARGEINDENT;
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,"Undefined"); 
                        }
                        if((PRINT_VALUE))
                        {
                            shvalue = read_ushort(inptr,byteorder,fileoffset_base + 2);
                            printf(" = %u",shvalue);
                            chpr = newline(chpr);
                        }
                        print_tag_address(ENTRY,fileoffset_base + 4,indent + MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,
                                                                        "FocalLength"); 
                        }
                        if((PRINT_VALUE))
                        {
                            shvalue = read_ushort(inptr,byteorder,fileoffset_base + 4);
                            chpr += printf(" = %u mm",shvalue);
                            chpr = newline(chpr);
                        }
                        print_tag_address(ENTRY,fileoffset_base + 6,indent + MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,
                                                                        "FocalPlaneXDim"); 
                        }
                        if((PRINT_VALUE))
                        {
                            shvalue = read_ushort(inptr,byteorder,fileoffset_base + 6);
                            chpr += printf(" = %u inches/1000",shvalue);
                            chpr = newline(chpr);
                        }
                        print_tag_address(ENTRY,fileoffset_base + 8,indent + MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,
                                                                        "FocalPlaneYDim"); 
                        }
                        if((PRINT_VALUE))
                        {
                            shvalue = read_ushort(inptr,byteorder,fileoffset_base + 8);
                            chpr += printf(" = %u inches/1000",shvalue);
                            if(PRINT_SECTION)
                            {
                                chpr = newline(chpr);
                                print_tag_address(ENTRY,fileoffset_base + 9,0,"@");
                                parindent -= charsprinted();
                                putindent(parindent);
                                chpr = printf("}");
                                indent -= MEDIUMINDENT;
                            }
                        }
                    }
                    break;
                case CIFFTAG_SHOTINFO: /* INHEAP subroutine           */
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(short)) - 1);
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_shotinfo(inptr,byteorder,tagname,
                                                entry->offset + fileoffset_base,
                                                entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_CAMERASETTINGS:    /* INHEAP subroutine */
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,
                                            (entry->length/sizeof(short)) - 1);
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_camera_settings(inptr,byteorder,
                                                    tagname,entry->offset + fileoffset_base,
                                                    entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                                                    indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_SENSORINFO:    /* INHEAP subroutine */
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(short)) - 1);
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_sensorinfo(inptr,byteorder,tagname,entry->offset + fileoffset_base,
                                                                entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_CUSTOMFUNCTIONS:   /* INHEAP subroutine */
                    if(Use_Make_name)
                        make = maker_number(Use_Make_name);
                    else
                        make = maker_number(Make_name);
                    if(Use_Model_name)
                        model = model_number(make,Use_Model_name,NULLSTRING);
                    else
                        model = model_number(make,Model_name,NULLSTRING);
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,
                                                    (entry->length/sizeof(short)) - 1);
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_customfunctions(inptr,byteorder,
                                            tagname,entry->offset + fileoffset_base,
                                            entry->length,model,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_PICTUREINFO:   /* INHEAP subroutine */
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(short)) - 1);
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_pictureinfo(inptr,byteorder,
                                                    tagname,entry->offset + fileoffset_base,
                                                    entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_WHITEBALANCETABLE: /* INHEAP subroutine */
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,
                                                (entry->length - sizeof(unsigned short))/sizeof(float));
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_whitebalancetable(inptr,byteorder,tagname,
                                    entry->offset + fileoffset_base,entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_IMAGESPEC:   /* INREC */
                    if((PRINT_VALUE) && (PRINT_SECTION))
                    {
                        indent += 19;
                        chpr = printf("{");
                        chpr = newline(chpr);
                        parindent = charsprinted();
                    }
                    if(PRINT_ENTRY)
                    {
                        entrywidth = strlen("TargetCompressionRatio");
                        dirname = dirname ? dirname : QSTRING;
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + LARGEINDENT,"@");
                        parindent = charsprinted() - LARGEINDENT;
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,"FileFormat"); 
                        }
                        if((PRINT_VALUE))
                        {
                            chpr += printf(" = ");
                            switch(entry->length)
                            {
                                case 0x10000: chpr += printf("JPEG quantized\n"); break;
                                case 0x10002: chpr += printf("JPEG non-quantized\n"); break;
                                case 0x10003: chpr += printf("JPEG nq picture/q text\n"); break;
                                case 0x20001: chpr += printf("CRW\n"); break;
                                default: chpr += printf("unknown\n"); break;
                            }
                        }
                        print_tag_address(ENTRY,fileoffset_base + 6,indent + LARGEINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,"TargetCompressionRatio"); 
                        }
                        if((PRINT_VALUE))
                        {
                            fvalue = to_float(entry->offset);
                            chpr += printf(" = %f",fvalue);
                            if(PRINT_SECTION)
                            {
                                chpr = newline(chpr);
                                print_tag_address(ENTRY,fileoffset_base + 9,0,"@");
                                parindent -= charsprinted();
                                putindent(parindent);
                                chpr = printf("}");
                            }
                        }
                    }
                    break;
                case CIFFTAG_SHUTTERRELEASEMETHOD:  /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                        if(PRINT_VALUE)
                            chpr += printf(" = ");
                    }
                    if(PRINT_VALUE)
                    {
                        print_ushort(inptr,4,byteorder,fileoffset_base + 2);
                        chpr += printf("%lu = ",entry->length & 0xffff);
                        if((entry->length & 0xffff) == 0)
                            chpr += printf("single-shot");
                        else if((entry->length & 0xffff) == 1)
                            chpr += printf("continuous");
                        else
                            chpr += printf("undefined");
                    }
                    break;
                case CIFFTAG_SHUTTERRELEASETIMING:  /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                        if(PRINT_VALUE)
                            chpr += printf(" = ");
                    }
                    if(PRINT_VALUE)
                    {
                        print_ushort(inptr,4,byteorder,fileoffset_base + 2);
                        chpr += printf(" = %lu = ",entry->length & 0xffff);
                        if((entry->length & 0xffff) == 0)
                            chpr += printf("shutter-priority");
                        else if((entry->length & 0xffff) == 1)
                            chpr += printf("aperture-priority");
                        else
                            chpr += printf("undefined");
                    }
                    break; 
                case CIFFTAG_SELFTIMERTIME: /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                    }
                    if(PRINT_VALUE)
                        chpr += printf(" = %lu msec ",entry->length & 0xffff);
                    break;
                case CIFFTAG_TARGETDISTANCESETTING: /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                    }
                    if(PRINT_VALUE)
                    {
                        float fvalue;

                        fvalue = to_float(entry->length);
                        chpr += printf("%f mm",fvalue);
                    }
                    break;
                case CIFFTAG_COLORSPACE:    /* INHEAP subroutine      */
                    if((PRINT_SECTION))
                    {
                        PUSHCOLOR(MAKER_COLOR);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent+MEDIUMINDENT,"@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(unsigned short)));
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_colorspace(inptr,byteorder,
                                                    tagname,entry->offset + fileoffset_base,
                                                    entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        indent -= MEDIUMINDENT;
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_TARGETIMAGETYPE:   /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                        if(PRINT_VALUE)
                            chpr += printf(" = ");
                    }
                    if(PRINT_VALUE)
                    {
                        print_ushort(inptr,4,byteorder,fileoffset_base + 2);
                        chpr += printf(" = %lu = ",entry->length & 0xffff);
                        if((entry->length & 0xffff) == 0)
                            chpr += printf("real-world");
                        else if((entry->length & 0xffff) == 1)
                            chpr += printf("document");
                        else
                            chpr += printf("undefined");
                    }
                    break; 
                case CIFFTAG_SERIALNUMBER:  /* INREC                  */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent + MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                        if(PRINT_VALUE)
                            chpr += printf(" = ");
                    }
                    if(PRINT_VALUE)
                    {
                        chpr += printf("%lu",entry->length);
                        print_canon_serialno(entry->length);
                    }
                    break;
                case CIFFTAG_CAPTUREDTIME: /* INHEAP subroutine */
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,indent+MEDIUMINDENT,
                                                                                    "@");
                        chpr += printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(unsigned long)));
                    }
                    max_offset = canon_ct_to_datetime(inptr,byteorder,
                                                tagname,entry->offset + fileoffset_base,
                                                entry->length,indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_EXPOSUREINFO: /* INHEAP, 3 floats; EC, Tv, Av */
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,entry->offset + fileoffset_base,indent+MEDIUMINDENT,
                                                                                "@");
                        chpr = printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(unsigned long)));
                        indent += MEDIUMINDENT;
                    }
                    max_offset = canon_exposureinfo(inptr,byteorder,tagname,
                                                    entry->offset + fileoffset_base,
                                                    entry->length,indent+MEDIUMINDENT,
                                                    summary_entry);
                    indent -= MEDIUMINDENT;
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_IMAGEINFO: /* INHEAP subroutine */
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,entry->offset + fileoffset_base,indent+MEDIUMINDENT,
                                                                                "@");
                        chpr = printf("<%s> data, %lu entries",fullnameoftag,(entry->length/sizeof(unsigned long)));
                    }
                    max_offset = canon_imageinfo(inptr,byteorder,tagname,
                                                    entry->offset + fileoffset_base,
                                                    entry->length,indent+MEDIUMINDENT,
                                                    summary_entry);
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,entry->offset + fileoffset_base + entry->length - 1,
                                            indent+MEDIUMINDENT,"-");
                        chpr += printf("</%s>",fullnameoftag);
                        POPCOLOR();
                    }
                    break;
                case CIFFTAG_FLASHINFO: /* INREC two floats, flash Guidenumber & Threshold */
                    if((PRINT_VALUE) && (PRINT_SECTION))
                    {
                        indent += 19;
                        chpr = printf("{");
                        chpr = newline(chpr);
                        parindent = charsprinted();
                    }
                    if(PRINT_ENTRY)
                    {
                        float fvalue;

                        entrywidth = strlen("FlashGuideNumber");
                        dirname = dirname ? dirname : QSTRING;
                        print_tag_address(ENTRY,fileoffset_base + 2,indent+MEDIUMINDENT,"@");
                        parindent = charsprinted() - MEDIUMINDENT;
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,"FlashGuideNumber"); 
                        }
                        if((PRINT_VALUE))
                        {
                            fvalue = to_float(entry->length);
                            chpr += printf(" = %f",fvalue);
                        }

                        chpr = newline(chpr);
                        print_tag_address(ENTRY,fileoffset_base + 6,indent+MEDIUMINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",fullnameoftag);
                            chpr += printf("%-*.*s",entrywidth,entrywidth,
                                                                        "FlashThreshold"); 
                        }
                        if((PRINT_VALUE))
                        {
                            fvalue = to_float(entry->offset);
                            chpr += printf(" = %f",fvalue);
                        }
                    }
                    if(PRINT_SECTION)
                    {
                        chpr = newline(chpr);
                        print_tag_address(ENTRY,fileoffset_base + 9,0,"@");
                        parindent -= charsprinted();
                        putindent(parindent);
                        chpr = printf("}");
                    }
                    break;
                case CIFFTAG_MEASUREDEV:    /* INREC */
                    if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                    {
                        print_tag_address(ENTRY,fileoffset_base + 2,indent+MEDIUMINDENT,"@");
                        chpr = printf("%-*.*s",entrywidth,entrywidth,tagname);
                    }
                    if(PRINT_VALUE)
                    {
                        float fvalue;

                        fvalue = to_float(entry->length);
                        chpr += printf("%f",fvalue);
                    }
                    break;
                case CIFFTAG_RAWIMAGEDATA:  /* INHEAP */
                    if(PRINT_SECTION)
                    {
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent,"@");
                        chpr += printf("Start of %s, length %lu\n",nameoftag,entry->length);
                        if((PRINT_VALUE))
                            dumpsection(inptr,entry->offset + fileoffset_base,entry->length,indent + SMALLINDENT);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent,"@");
                        chpr += printf("End of %s",nameoftag);
                    }
                    else
                    {
                        print_tag_address(ENTRY,entry->offset + fileoffset_base,
                                                    indent+MEDIUMINDENT,"@");
                        /* pseudo-tags                                */
                        if(PRINT_TAGINFO)
                            chpr += printf("%s.%-*.*s",dirname,CIFFTAGWIDTH,CIFFTAGWIDTH,nameoftag);
                        if((PRINT_ENTRY))
                        {
                            if((PRINT_VALUE))
                            {
                                if(PRINT_BOTH_OFFSET)
                                    chpr += printf(" = @%#lx=%lu",entry->offset + fileoffset_base,
                                            entry->offset + fileoffset_base);
                                else if(PRINT_HEX_OFFSET)
                                    chpr += printf(" = @%#lx",entry->offset + fileoffset_base);
                                else 
                                    chpr += printf(" = @%lu",entry->offset + fileoffset_base);

                                if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                                    chpr += printf(" (%lu)",entry->offset);
                            }
                            chpr = newline(chpr);
                            print_tag_address(ENTRY,0,indent+MEDIUMINDENT,"@");
                            if((PRINT_TAGINFO))
                            {
                                tagwidth = CIFFTAGWIDTH - strlen(nameoftag) - 1;
                                chpr += printf("%s.%s.%-*.*s",dirname,nameoftag,tagwidth,
                                                            tagwidth,"length");
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = %lu",entry->length);
                        }
                    }
                    max_offset = entry->offset + fileoffset_base + entry->length;
                    summary_entry->offset = entry->offset + fileoffset_base;
                    summary_entry->length = entry->length;
                    summary_entry->subfiletype = PRIMARY_TYPE;
                    summary_entry->imageformat = IMGFMT_CRW;
                    summary_entry->compression = is_compressed_crw(inptr,
                                                        entry->offset + fileoffset_base,
                                                        entry->length);
                    summary_entry->entry_lock = lock_number(summary_entry);

                    break;
                case CIFFTAG_JPEGIMAGE: /* INHEAP */
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                                                indent,"@");
                        chpr += printf("Start of %s length %lu\n",nameoftag,entry->length);
                    }
                    summary_entry = new_summary_entry(summary_entry,JPEG_SOI,JPEG_SOI);
                    if(summary_entry)
                        summary_entry->subfiletype = REDUCED_RES_TYPE;
                    marker = read_ushort(inptr,TIFF_MOTOROLA,entry->offset + fileoffset_base);
                    max_offset = process_jpeg_segments(inptr,entry->offset + fileoffset_base,
                                        marker, entry->length,summary_entry,fullnameoftag,
                                        "@",indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent,"@");
                        chpr += printf("End of %s",nameoftag);
                    }
                    else 
                    {
                        print_tag_address(ENTRY,entry->offset + fileoffset_base,
                                                    indent+MEDIUMINDENT,"@");
                        /* pseudo-tags                                */
                        if((PRINT_TAGINFO))
                            chpr += printf("%s.%-*.*s",dirname,CIFFTAGWIDTH,CIFFTAGWIDTH,nameoftag);
                        if((PRINT_ENTRY))
                        {
                            if((PRINT_VALUE))
                            {
                                if(PRINT_BOTH_OFFSET)
                                    chpr += printf(" = @%#lx/%lu",entry->offset + fileoffset_base,
                                            entry->offset + fileoffset_base);
                                else if(PRINT_HEX_OFFSET)
                                    chpr += printf(" = @%#lx",entry->offset + fileoffset_base);
                                else 
                                    chpr += printf(" = @%lu",entry->offset + fileoffset_base);

                                if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                                    chpr += printf(" (%lu)",entry->offset);
                            }
                            chpr = newline(chpr);
                            print_tag_address(ENTRY,0,indent+MEDIUMINDENT,"@");
                            if((PRINT_TAGINFO))
                            {
                                tagwidth = CIFFTAGWIDTH - strlen(nameoftag) - 1;
                                chpr += printf("%s.%s.%-*.*s",dirname,nameoftag,tagwidth,
                                                    tagwidth,"length");
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = %lu",entry->length);
                        }
                    }
                    print_jpeg_status();
                    break;
                case CIFFTAG_JPEGTHUMBNAIL: /* INHEAP */
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base,
                                            indent,"-");
                        chpr += printf("Start of %s length %lu\n",nameoftag,entry->length);
                    }
                    summary_entry = new_summary_entry(summary_entry,FILEFMT_CIFF,IMGFMT_JPEG);
                    if(summary_entry)
                        summary_entry->subfiletype = THUMBNAIL_TYPE;
                    marker = read_ushort(inptr,TIFF_MOTOROLA,entry->offset + fileoffset_base);
                    max_offset = process_jpeg_segments(inptr,entry->offset + fileoffset_base,marker,
                                                        entry->length,summary_entry,fullnameoftag,
                                                        "@",indent+MEDIUMINDENT);
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(SECTION|ENTRY,entry->offset + fileoffset_base + entry->length - 1,
                                            indent,"-");
                        chpr += printf("End of %s",nameoftag);
                    }
                    else
                    {
                        print_tag_address(ENTRY,entry->offset + fileoffset_base,
                                                    indent+MEDIUMINDENT,"@");
                        /* pseudo-tags                                */
                        if((PRINT_TAGINFO))
                            chpr += printf("%s.%-*.*s",dirname,CIFFTAGWIDTH,CIFFTAGWIDTH,nameoftag);
                        if((PRINT_ENTRY))
                        {
                            if((PRINT_VALUE))
                            {
                                if(PRINT_BOTH_OFFSET)
                                    chpr += printf(" = @%#lx/%lu",entry->offset + fileoffset_base,
                                            entry->offset + fileoffset_base);
                                else if(PRINT_HEX_OFFSET)
                                    chpr += printf(" = @%#lx",entry->offset + fileoffset_base);
                                else 
                                    chpr += printf(" = @%lu",entry->offset + fileoffset_base);

                                if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                                    chpr += printf(" (%lu)",entry->offset);
                            }
                                    chpr = newline(chpr);
                            print_tag_address(ENTRY,0,indent+MEDIUMINDENT,"@");
                            if((PRINT_TAGINFO))
                            {
                                tagwidth = CIFFTAGWIDTH - strlen(nameoftag) - 1;
                                chpr += printf("%s.%s.%-*.*s",dirname,nameoftag,tagwidth,
                                                tagwidth,"length");
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = %lu",entry->length);
                        }
                    }
                    print_jpeg_status();
                    break;
                case CIFFTAG_IMAGEDESCRIPTION:  /* SUBDIR             */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent+MEDIUMINDENT);
                    break;
                case CIFFTAG_CAMERAOBJECT:  /* SUBDIR                 */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent+MEDIUMINDENT);
                    break;
                case CIFFTAG_SHOOTINGRECORD:    /* SUBDIR             */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent+MEDIUMINDENT);
                    break;
                case CIFFTAG_MEASUREDINFO:  /* SUBDIR                 */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent+MEDIUMINDENT);
                    break;
                case CIFFTAG_CAMERASPECIFICATION:   /* SUBDIR         */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent+MEDIUMINDENT);
                    break;
                case CIFFTAG_IMAGEPROPS:    /* SUBDIR                 */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent);
                    break;
                case CIFFTAG_EXIFINFORMATION:   /* SUBDIR             */
                    endofdir = entry->offset+fileoffset_base+entry->length;
                    if((PRINT_SECTION))
                        tagname = fullnameoftag;
                    PUSHCOLOR(EXIF_COLOR);
                    max_offset = process_ciff_dir(inptr,entry->offset + fileoffset_base,
                                                    endofdir,CNULL,tagname,summary_entry,
                                                    byteorder,level+1,indent+MEDIUMINDENT);
                    POPCOLOR();
                    break;
#if 0
                case CIFFTAG_FILENUMBER: break;   /* no interp reqd, use default */
                case CIFFTAG_DECODERTABLE: break;  /* 4 values; unknown, use default */
                case CIFFTAG_COMPONENTVERSION: break; /* unknown, use default */
                case CIFFTAG_RELEASESETTING: break; /* spec lists but doesn't define */
                case CIFFTAG_0X1834: break; /* seen in ExifInformation; unknown, use default */  
                case CIFFTAG_0x183B: break; /* seen in ExifInformation; unknown, use default */
                case CIFFTAG_RECORDID: break;   /* no conversion reqd */
#endif
                case CIFFTAG_NULLRECORD:    /* fall thru to default   */
                case CIFFTAG_FREEBYTES:     /* fall thru to default   */
                default:
                    if(location == CIFF_INHEAP)
                    {
                        if((PRINT_SECTION))
                            indent += MEDIUMINDENT;
                        max_offset = print_ciffinheapdata(inptr,byteorder,dirname,
                                nameoftag,entry->offset + fileoffset_base,entry->length,
                                entry->type&CIFF_FORMATMASK,indent);
                    }
                    else if(location == CIFF_INREC)
                    {
                        if(!(PRINT_SECTION) && (PRINT_TAGINFO))
                        {
                            print_tag_address(ENTRY,fileoffset_base + 2,indent+MEDIUMINDENT,"@");
                            chpr += printf("%-*.*s",entrywidth,entrywidth,tagname); 
                            if(PRINT_VALUE)
                                chpr += printf(" = ");
                        }
                        max_offset = print_ciffinrecdata(inptr,byteorder,tagname,
                                    fileoffset_base + 2,8,entry->type&CIFF_FORMATMASK,
                                    indent+MEDIUMINDENT);
                    }
                    else
                        max_offset = 0;
                    break;
            }
            chpr = newline(chpr);
            if(fullnameoftag)
                free(fullnameoftag);
            if(nameoftag)
                free(nameoftag);
            fullnameoftag = nameoftag = CNULL;
        }
    }
    return(max_offset);
}

/* This checks for compression in "raw" CRW files in the manner       */
/* suggested by Dave Coffin's "dcraw" program. The ImageInfo data     */
/* doesn't seem to say.                                               */
/* A return value of 0 indicates uncompressed data. A return value of */
/* 1 indicates compressed.                                            */

int
is_compressed_crw(FILE *inptr,unsigned long start_offset,unsigned long imagelength)
{
    int is_compressed = 0;
    int maxcheck = 16384;   /* 0x4000                                 */
    int count = 0;
    int ch,lastch;

    if((inptr) && (fseek(inptr,start_offset + 512,SEEK_SET) != -1))
    {
        if(imagelength < maxcheck)
            maxcheck = imagelength;
        lastch = 0;
        while(((ch = fgetc(inptr)) != EOF) && (count++ < maxcheck))
        {
            if(lastch == 0xff)
            {
                is_compressed = 1;
                if(ch)
                {
                    is_compressed = 0;
                    break;
                }
            }
            lastch = ch;
        }
    }
    return(is_compressed);
}

/* Print CIFF header information if options permit. Return 0 if       */
/* information is valid to print, or -1 if the header is invalid      */

int
print_ciff_header(struct fileheader *header,unsigned long section_id)
{
    struct ciff_header *ciffheader = NULL;
    unsigned short byteorder = 0;
    int status = -1;
    int chpr = 0;

    if(header)
    {
        byteorder = header->file_marker;
        if(header->ciff_header)
        {
            ciffheader = header->ciff_header;
            if(header->probe_magic == PROBE_CIFFMAGIC)
            {
                if(Print_options & section_id)
                    chpr += printf("CIFF");
                switch(byteorder)
                {
                case TIFF_INTEL:
                case TIFF_MOTOROLA:
                    if(Print_options & section_id)
                    {
                        print_byteorder(byteorder,1);
                        chpr += printf(" \"%s%s\", version %#lx, heap offset = %#lx/%lu",
                                ciffheader->type,ciffheader->subtype,
                                ciffheader->version,ciffheader->headerlength,
                                ciffheader->headerlength);
                    }
                    status = 0;
                    break;
                default:
                    if(Print_options & section_id)
                        chpr += printf("INVALID CIFF BYTEORDER ");
                        print_byteorder(byteorder,1);
                        chpr += printf(" (CIFF type \"%s%s\")",ciffheader->type,
                                                        ciffheader->subtype);
                    break;
                }
            }
            else if(Print_options & section_id)
            {
                    chpr += printf("INVALID CIFF IDENTIFIER ");
                    print_magic(header->probe_magic,4);
                    chpr += printf(" byteorder ");
                    print_byteorder(byteorder,1);
            }
        }
        else
            fprintf(stderr,"%s: null ciffheader to print_ciff_header()\n",Progname);
    }
    else
        fprintf(stderr,"%s: null fileheader to print_ciff_header()\n",Progname);
    chpr = newline(chpr);
    return(status);
}

struct ciff_header *
read_ciffheader(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    static struct ciff_header header;
    struct ciff_header *headerptr = NULL;
    unsigned char *string;

    memset(&header,0,sizeof(struct ciff_header));
    if(inptr)
    {
        header.byteorder = read_ushort(inptr,byteorder,offset);
        header.headerlength = read_ulong(inptr,byteorder,HERE);
        string = read_bytes(inptr,4,HERE);
        if(string)
        {
            strncpy(header.type,(char *)string,4);
            string = read_bytes(inptr,4,HERE);
            if(string)
                strncpy(header.subtype,(char *)string,4);
        }
        header.version = read_ulong(inptr,byteorder,HERE);
        header.reserved1 = read_ulong(inptr,byteorder,HERE);
        header.reserved2 = read_ulong(inptr,byteorder,HERE);
        if((header.byteorder == byteorder) &&
                (strncmp(header.type,"HEAP",4) == 0) &&
                                    (header.headerlength > 0))
        {
            headerptr = &header;
        }
    }
    return(headerptr);
}


struct ciff_direntry *
read_ciff_direntry(FILE *inptr,struct ciff_direntry *entry,unsigned short byteorder,unsigned long offset)
{
    if(inptr)
    {
        if(entry == NULL)
            entry = (struct ciff_direntry *)malloc(sizeof(struct ciff_direntry));
        if(entry)
        {
            memset(entry,0,sizeof(struct ciff_direntry));
            entry->type = read_ushort(inptr,byteorder,offset);
            entry->length = read_ulong(inptr,byteorder,offset + 2);
            entry->offset = read_ulong(inptr,byteorder,offset + 6);
        }
    }
    return(entry);
}
