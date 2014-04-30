/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2005 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: jp2000.c,v 1.10 2005/07/24 19:54:04 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* JP2/Jpeg2000 routines                                              */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* The information coded here is derived from the public version of   */
/* the Jpeg2000 specification (badly outdated) and from the jasper    */
/* code at                                                            */
/*     http://www.ece.uvic.ca/~mdadams/jasper/                        */

/* Some items, such as the ipr, xml, uuid, and uinf boxes, are not    */
/* exposed in the jasper implementation, and are implemented from the */
/* public version of the spec. A few test images containing xml and   */
/* uuid boxes have been found; the rest are thoroughly untested.      */

/* 64-bit addressing is not yet supported                             */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "misc.h"
#include "tags.h"

#include "jp2.h"
#include "jp2_extern.h"
#include "jpegtags.h"
#include "jp2tags.h"
#include "extern.h"

struct jp2box *
read_jp2box(FILE *inptr,unsigned long offset)
{
    static struct jp2box box;
    struct jp2box *newbox = NULL; 

    if(inptr)
    {
        box.boxoffset = offset;
        box.lbox = read_ulong(inptr,TIFF_MOTOROLA,offset);
        if(!feof(inptr) && !(ferror(inptr)))
        {
            box.tbox = read_ulong(inptr,TIFF_MOTOROLA,HERE);
            if(!feof(inptr) && !(ferror(inptr)))
            {
                if(box.lbox == 1)
                {
#if 0
                    box.boxlength = read_ulong64(inptr,TIFF_MOTOROLA,HERE);
                    box.dataoffset = 16;
#else
                    PUSHCOLOR(RED);
                    printf("%s: cannot read 64 bit sizes/offsets yet",Progname);
                    POPCOLOR();
                    newbox = NULL;
#endif
                }
                else
                {
                    box.boxlength = box.lbox;
                    box.dataoffset = 8;
                }
                newbox = &box;
            }
            else
                newbox = NULL;
        }
        else
            newbox = NULL;
    }
    return(newbox);
}

/* Print the content of the JP2 file signature box iff allowed by     */
/* print options.                                                     */

int
print_jp2_header(struct fileheader *fileheader,unsigned long section_id)
{
    struct jp2_header jp2header;
    int status = -1;
    int chpr = 0;

    if(Print_options & section_id)
    {
        if(fileheader && (fileheader->probe_magic == PROBE_JP2MAGIC))
        {
            jp2header = fileheader->jp2_header;
            if(jp2header.magic == PROBE_JP2MAGIC)
            {
                /* Ok, I'm convinced...                               */
                print_jp2type(jp2header.type,0);
                chpr += printf(" <%#lx> magic %#lx, length %lu",jp2header.type,
                                            jp2header.magic,jp2header.length);
                chpr = newline(chpr);
                status = 0;
            }
            else 
                chpr += printf(" NOT A JP2 HEADER");
        }
        else
            chpr += printf(" NOT A JP2 HEADER");
    }
    chpr = newline(chpr);
    return(status);
}


/* Print a jp2 box identifier, with its length and offset to the box  */
/* data, if SECTIONS are enabled in print options.                    */

void
print_jp2box(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxlength = 0UL;
    int chpr = 0;

    if(box)
    {
        if(box->boxlength == 0UL)
            box->boxlength = get_filesize(inptr) - box->boxoffset;
        else
            boxlength = box->boxlength;
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,box->boxoffset,indent,"@");
            print_jp2type(box->tbox,0);
            chpr += printf(" <%#lx>",box->tbox);
            chpr += printf(" length %lu",boxlength);
            /* Indicate that this is lastbox                          */
            if(box->boxlength == 0UL)
                chpr += printf(" (0)");
            chpr += printf(" data offset %lu",box->dataoffset);
        }
    }
    chpr = newline(chpr);
}

int
list_jp2box(FILE *inptr,struct jp2box *box,char *parent_name,int indent,int donl)
{
    unsigned long boxlength = 0UL;
    int chpr = 0;

    if(box)
    {
        if(box->boxlength == 0UL)
            boxlength = get_filesize(inptr) - box->boxoffset;
        else
            boxlength = box->boxlength;
        if((LIST_MODE))
        {
            print_tag_address(ENTRY,box->boxoffset,indent,"@");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    printf("%s",parent_name);
                print_jp2type(box->tbox,0);
            }
            if((PRINT_VALUE))
            {
                chpr += printf(" = @%lu:%-4lu",box->boxoffset,boxlength);
                /* Indicate that this is lastbox                      */
                if(box->boxlength == 0UL)
                    chpr += printf(":0");
            }
        }
    }
    if(donl)
        chpr = newline(chpr);
    return(chpr);
}

/* Report the type of a JP2 box. Types are normally printable ascii   */
/* (stored in 4 byte unsigned integers), but this routine is prepared */
/* to print garbled 'types', which may indicate that the program has  */
/* gone astray, or that the file is garbled. Unprintable bytes,       */
/* including newlines and such, are printed in escaped octal          */
/* notation.                                                          */

/* Box type names are enclosed in [].                                 */

void
print_jp2type(unsigned long type,int atend)
{
    int i;
    char *p;
    int chpr = 0;

    p = (char *)&type + 3;

    if(!(LIST_MODE))
    {
        putchar('[');
        ++chpr;
    }
    /* 'atend' declares that the output marks the last byte of the    */
    /* box; this is indicated by prepending a '/' to the box name     */
    if(atend)
    {
        putchar('/');
        ++chpr;
    }
    for(i = 0; i < 4; ++i,--p)
    {
        if(isascii(*p) && isprint(*p))
        {
            putchar(*p);
            ++chpr;
        }
        else if(*p)
            chpr += printf("\\%03u",*p & 0xff);
        else
            chpr += printf("\\0");
    }
    if(!(LIST_MODE))
    {
        putchar(']');
        ++chpr;
    }
    setcharsprinted(chpr);
}

/* The top level JP2 processor.                                       */

unsigned long
process_jp2(FILE *inptr,unsigned long offset,struct image_summary *summary_entry,
                                    char *parent_name,int indent)
{
    struct jp2box *box;
    unsigned long max_offset = 0L;
    unsigned long lastbox = 0L;
    unsigned long dumplength;

    while(!feof(inptr) && !lastbox)
    {
        box = read_jp2box(inptr,offset);
        if(box)
        {
            /* This routine is currently called only from main(),     */
            /* where the summary_entry will already have been created */

            /* nonetheless...                                         */
            if((summary_entry == NULL) || summary_entry->entry_lock)
                summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG2000);
            switch(box->tbox)
            {
                case JP2_FTYP:
                    max_offset = process_jp2_ftyp(inptr,box,summary_entry,indent);
                    break;
                case JP2_XML:
                    max_offset = process_jp2_xml(inptr,box,indent);
                    break;
                case JP2_JP2I:
                    max_offset = process_jp2_jp2i(inptr,box,indent);
                    break;
                case JP2_JP2H:
                    max_offset = process_jp2_jp2h(inptr,box,summary_entry,indent);
                    break;
                case JP2_UUID:
                    max_offset = process_jp2_uuid(inptr,box,summary_entry,indent);
                    break;
                case JP2_JP2C:
                    if(box->boxlength == 0)
                        ++lastbox;
                    max_offset = process_jp2_jp2c(inptr,box,summary_entry,indent);
                    break;
                case JP2_UINF:
                    max_offset = process_jp2_uinf(inptr,box,summary_entry,indent);
                    break;
                case JP2_PRFL:  /* apparently not in the final spec   */
                default:
                    if((box->boxlength == 0) || ateof(inptr))
                        ++lastbox;
                    if((LIST_MODE))
                        setcharsprinted(list_jp2box(inptr,box,"JP2.",indent,1));
                    else
                        print_jp2box(inptr,box,indent);
                    max_offset = box->boxoffset + box->boxlength;
                    if((PRINT_SECTION))
                    {
                        /* Dump unrecognized boxes.                   */
                        if(Max_undefined > 0)
                        {
                            if(Max_undefined == DUMPALL)
                                dumplength = box->boxlength;
                            else if(Max_undefined > box->boxlength)
                                    dumplength = box->boxlength;
                            else
                                dumplength = Max_undefined;
                        }
                        else
                        {
                            /* Always dump a little of undefined  */
                            /* boxes                              */
                            if(box->boxlength < 48)
                                dumplength = box->boxlength;
                            else
                                dumplength = 48;
                        }

                        (void)newline(0);
                        hexdump(inptr,box->boxoffset,dumplength,dumplength,
                                                            16,indent,SUBINDENT);
                        (void)newline(1);
                    }
                    if((PRINT_SECTION))
                    {
                        (void)newline(0);
                        print_tag_address(SECTION,max_offset - 1,indent,"@");
                        print_jp2type(box->tbox,1);
                    }
                    break;
            }
            (void)newline(0);
            offset = max_offset;
            if(ateof(inptr))
                ++lastbox;
        }
        else
            break;
    }
    return(max_offset);
}

unsigned long
process_jp2_ftyp(FILE *inptr,struct jp2box *box,struct image_summary *summary_entry,
                                                        int indent)
{
    unsigned long boxoffset,max_offset,dataoffset;
    unsigned long boxlength;
    unsigned long mjv,mnv,cl,ncl;
    int compat = 0;
    int chpr = 0;
    int i;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

        
    mjv = read_ulong(inptr,TIFF_MOTOROLA,dataoffset);
    dataoffset += 4;
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.ftyp.");
        chpr += printf("MajorVersion");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" = %#-10lx = ",mjv);
        print_jp2type(mjv,0);
        if(mjv != JP2_BR)
            printred( " (INVALID MAJOR VERSION");
    }
    chpr = newline(chpr);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    mnv = read_ulong(inptr,TIFF_MOTOROLA,dataoffset); 
    dataoffset += 4;
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.ftyp.");
        chpr += printf("MinorVersion");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" = %#-10lx = ",mnv);
        print_jp2type(mnv,0);
    }
    chpr = newline(chpr);
    ncl = box->boxlength - box->dataoffset - 8;
    ncl /= 4;
    for(i = 0; i < ncl; ++i)
    {
        print_tag_address(ENTRY,dataoffset,indent + 8,"=");
        cl = read_ulong(inptr,TIFF_MOTOROLA,dataoffset); 
        dataoffset += 4;
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.ftyp.");
            chpr += printf("Compat");
        }
        if((PRINT_VALUE))
        {
            chpr += printf(" = %#-10lx = ",cl);
            print_jp2type(cl,0);
        }
        chpr =  newline(chpr);
        if(cl == JP2_BR)
            compat++;
    }
    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(box->tbox,1);
    }
    chpr = newline(chpr);
    return(max_offset);
}

/* The xml box. The xml is printed if Print_options includes SECTION, */
/* but the text is not examined for information which might go in the */
/* image summary. Even though image information may be recorded in    */
/* the xml, the information need not be correct or pertain to the     */
/* current image (the original image may have been modified without   */
/* updating the xml.                                                  */

unsigned long
process_jp2_xml(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxoffset,max_offset,dataoffset;
    unsigned long readsize;
    unsigned long boxlength,size;
    int chpr = 0;

    print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = box->boxoffset + box->boxlength;
    size = boxlength - 8;

    if((PRINT_SECTION))
    {
        print_tag_address(VALUE,dataoffset,indent + 10,"=");
        if((PRINT_VALUE))
        {
            if((inptr && (fseek(inptr,dataoffset,SEEK_SET)) != -1))
            {
                while(size > 0ULL)
                {
                    if(size > READSIZE)
                        readsize = READSIZE;
                    else
                        readsize = size;
                    /* Print as ascii                                     */
                    setcharsprinted(chpr);
                    print_ascii(inptr,readsize,dataoffset);
                    dataoffset += readsize;
                    size -= readsize;
                }
            }
        }
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"=");
        print_jp2type(box->tbox,1);
    }
    else if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,1);
    chpr = newline(chpr);
    return(max_offset);
}


/* The "Intellectual Property Rights" box, which records information  */
/* which may have nothing to do with intellect, property, or any      */
/* logical concept of rights.                                         */

unsigned long
process_jp2_jp2i(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxoffset,max_offset,dataoffset;
    unsigned long readsize;
    unsigned long boxlength,size;
    int chpr = 0;

    print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,dataoffset,indent + 8,"=");
        size = boxlength - 8;
        if((inptr && (fseek(inptr,dataoffset,SEEK_SET)) != -1))
        {
            while(size > 0ULL)
            {
                if(size > READSIZE)
                    readsize = READSIZE;
                else
                    readsize = size;
                /* Print as ascii...                                  */
                setcharsprinted(chpr);
                print_ascii(inptr,readsize,dataoffset);
                dataoffset += readsize;
                size -= readsize;
            }
        }
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"=");
        print_jp2type(box->tbox,1);
    }
    else if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,1);
    chpr = newline(chpr);
    return(max_offset);
}

/* The header superbox; this box has lots of human-interest stuff     */

unsigned long
process_jp2_jp2h(FILE *inptr,struct jp2box *box,struct image_summary *summary_entry,
                                                        int indent)
{
    unsigned long lastbox = 0L;
    unsigned long boxoffset,max_offset,dataoffset,tbox;
    unsigned long boxlength;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;
    tbox = box->tbox;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;


    while(!feof(inptr) && !lastbox && (dataoffset < max_offset))
    {
        box = read_jp2box(inptr,dataoffset);
        if(box)
        {
            switch(box->tbox)
            {
                case JP2_ihdr:
                    dataoffset = process_jp2_ihdr(inptr,box,summary_entry,indent + SUBINDENT);
                    break;
                case JP2_colr:
                    dataoffset = process_jp2_colr(inptr,box,indent + SUBINDENT);
                    break;
                case JP2_res:
                    dataoffset = process_jp2_res(inptr,box,indent + SUBINDENT);
                    break;
                case JP2_bpcc:
                case JP2_pclr:
                case JP2_cdef:
                case JP2_resc:  /* handled in process_jp2_res().  */
                case JP2_resd:  /* handled in process_jp2_res().  */
                    /* These fields are reported but not expanded */
                    print_jp2box(inptr,box,indent + SUBINDENT);
                    if(boxlength)
                        dataoffset += boxlength;
                    else
                        ++lastbox;
                    chpr = newline(chpr);
                    break;
                default:
                    /* report the unknown box and try to continue. If */
                    /* the box is properly constructed, that may be   */
                    /* possible; most likely this is garbled data.    */
                    print_jp2box(inptr,box,indent + SUBINDENT);
                    if(ferror(inptr) || feof(inptr))
                    {
                        clearerr(inptr);
                        ++lastbox;
                    }
                    if(boxlength)
                        dataoffset += boxlength;
                    else
                        ++lastbox;
                    chpr = newline(chpr);
                    break;
            }
        }
    }
    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(tbox,1);
    }
    chpr = newline(chpr);
    return(max_offset);
}

/* The ihdr box, within the jp2h box, wherein we find the image size. */

unsigned long
process_jp2_ihdr(FILE *inptr,struct jp2box *box,struct image_summary *summary_entry,
                                                        int indent)
{
    unsigned long boxoffset,max_offset,dataoffset;
    unsigned long boxlength;
    unsigned long imgheight = 0UL;
    unsigned long imgwidth = 0UL;
    unsigned short nc = 0;
    unsigned short bpc = 0;
    unsigned short comp = 0;
    unsigned short hasUNK = 0;
    unsigned short hasIPR = 0;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.jp2h.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

    imgheight = read_ulong(inptr,TIFF_MOTOROLA,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("ImageHeight");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",imgheight);
    chpr = newline(chpr);
    dataoffset += 4;

    imgwidth = read_ulong(inptr,TIFF_MOTOROLA,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("ImageWidth");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",imgwidth);
    chpr = newline(chpr);
    dataoffset += 4;
    if(summary_entry && (imgheight > 0) &&
                            (summary_entry->pixel_height < imgheight))
    {
        summary_entry->pixel_height = imgheight;
    }
    if(summary_entry && (imgwidth > 0) &&
                            (summary_entry->pixel_width < imgwidth))
    {
        summary_entry->pixel_width = imgwidth;
    }

    nc = read_ushort(inptr,TIFF_MOTOROLA,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("NumberOfComponents");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",nc);
    chpr = newline(chpr);
    dataoffset += 2;

    bpc = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("BitsPerComponent");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",bpc);
    chpr = newline(chpr);
    dataoffset++;

    comp = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("Compression");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",comp);
    chpr = newline(chpr);
    /* This is always 7 according to spec, and tells us nothing;  */
    /* it will be over-written with the transform type from a COD */
    /* segement, if one is found. I imagine that "extensions" to  */
    /* the spec will quickly overwhelm this.                      */
    if((summary_entry) && (summary_entry->compression <= 0))
        summary_entry->compression = comp;
    dataoffset++;

    hasUNK = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("Colorspace");
    }
    if((PRINT_VALUE))
            chpr += printf(" = %#x = %sknown",hasUNK,hasUNK ? "un" : "");
    chpr = newline(chpr);
    dataoffset++;

    hasIPR = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.ihdr.");
        chpr += printf("IPRbox");
    }
        if((PRINT_VALUE))
            chpr += printf(" = %#x = %s",hasIPR,hasIPR ? "yes" : "no");
    chpr = newline(chpr);
    dataoffset++;

    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(box->tbox,1);
        chpr = newline(chpr);
    }
    return(max_offset);
}

unsigned long
process_jp2_uuid(FILE *inptr,struct jp2box *box,struct image_summary *summary_entry,
                                                        int indent)
{
    unsigned long boxoffset,max_offset,dataoffset;
    unsigned long boxlength,dumplength;
    unsigned long ifd_offset = 0UL;
    unsigned short idbyte,byte;
    struct fileheader *header = NULL;
    int chpr = 0;
    int i;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,0);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

    idbyte = byte = read_ubyte(inptr,dataoffset);
    print_tag_address(SECTION,dataoffset,indent + 4,"*");
    if((LIST_MODE))
    {
        if((PRINT_VALUE))
            chpr += printf(" # ID: ");
    }
    else
        chpr += printf("ID: ");
    if(!(LIST_MODE) || (PRINT_VALUE))
    {
        if(!feof(inptr) && !ferror(inptr))
        {
            chpr += printf("%02x",(unsigned int)byte & 0xff);
            for(i = 1; i < 16; ++i)
            {
                byte = read_ubyte(inptr,HERE);
                if(feof(inptr) || ferror(inptr))
                    break;
                chpr += printf(",%02x",(unsigned int)byte & 0xff);
            }
            chpr = newline(chpr);
        }
    }
    dataoffset += 16;

    /* No idea what the 16 "ID" bytes mean, or how to track           */
    /* registered types through the OID register. For now, a leading  */
    /* byte of 0x5 seems to signal a TIFF IFD. Dump a little bit of   */
    /* anything else. Probably need to check more of the ID...        */

    /* ...ok, now I've seen *two* ids that introduce TIFF sections... */
    if((idbyte != 5) && (idbyte != 0xb1))
    {
        if((PRINT_SECTION))
        {
            dumplength = max_offset - dataoffset;
            if(dumplength > 48)
                dumplength = 48;
            chpr = newline(chpr);
            hexdump(inptr,dataoffset,dumplength,dumplength,16,indent,SUBINDENT);
            chpr = newline(1);
        }
    }

    /* So far I've seen these with recognizable "magic"               */
    /* 96,a9,f1,f1,... MSIG - ??? what is this?                       */
    /* 2c,4c,01,00,... 8BIM Adobe PhotoShop - should handle this...   */
    /* 05,37,cd,ab,... TIFF - this we can do...                       */
    /* b1,4b,f8,bd.... also TIFF                                      */
    header = read_imageheader(inptr,dataoffset);
    if(header && (header->probe_magic == PROBE_TIFFMAGIC))
    {
        if((PRINT_SECTION))
        {
            chpr = newline(chpr);
            print_tag_address(SECTION,dataoffset,indent + 4,"@");
            print_header(header,SECTION);
            ifd_offset = read_ulong(inptr,header->file_marker,HERE); 
            chpr += printf(" ifd offset = %#lx/%lu",ifd_offset,ifd_offset);
            chpr += printf(" (+ %lu = %#lx/%lu)",dataoffset,
                                                    dataoffset + ifd_offset,
                                                    dataoffset + ifd_offset);
            chpr = newline(0);
        }
        dataoffset = process_tiff_ifd(inptr,header->file_marker,
                            8,dataoffset,max_offset,summary_entry,"JP2.uuid",
                            TIFF_IFD,0,-1,indent + 4);
    }
    dataoffset = boxoffset + boxlength;

    if((PRINT_SECTION))
    {
        chpr = newline(0);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(box->tbox,1);
        chpr = newline(chpr);
    }
    return(max_offset);
}

/* The uinf superbox, presumably containing a list of uuids and urls  */
/* pointing to data to describe them.                                 */

/* This is from the outdated public Jpeg2000 spec; jasper doesn't     */
/* appear to handle it. Haven't seen one in the wild yet.             */

unsigned long
process_jp2_uinf(FILE *inptr,struct jp2box *box,struct image_summary *summary_entry,
                                                        int indent)
{
    unsigned long boxoffset,max_offset,dataoffset,tbox;
    unsigned long boxlength;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;
    tbox = box->tbox;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

    box = read_jp2box(inptr,dataoffset);
    if(box)
    {
        if((box->tbox) == JP2_ulst)
            dataoffset = process_jp2_ulst(inptr,box,indent);
        else
        {
            print_jp2box(inptr,box,indent);
            chpr += printf(" NOT a ULST box!");
            dataoffset += box->boxlength;
        }
        box = read_jp2box(inptr,dataoffset);
        if(box)
            dataoffset = process_jp2_de(inptr,box,indent);
        else
        {
            print_jp2box(inptr,box,indent);
            chpr += printf(" NOT a DE box!");
            dataoffset += box->boxlength;
        }
    }

    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(box->tbox,1);
    }
    chpr = newline(chpr);
    return(max_offset);
}

/* The uuid list (ulst) box of the uinf superbox.                     */

unsigned long
process_jp2_ulst(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxoffset,max_offset,dataoffset,tbox;
    unsigned long boxlength;
    unsigned short nuuid,byte;
    int chpr = 0;
    int i,j;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.uinf.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;
    tbox = box->tbox;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

    nuuid = read_ushort(inptr,TIFF_MOTOROLA,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.uinf.ulst.");
        chpr += printf("NumberOfUuids");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",nuuid);
    chpr = newline(chpr);
    dataoffset += 2;

    /* Print the uuid list, in hex                                */
    for(i = 0; i < nuuid; ++i)
    {
        print_tag_address(SECTION|ENTRY,dataoffset,indent + 4,"*");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.uinf.ulst.");
            chpr += printf("ID%d = ",i);
        }
        if((PRINT_VALUE))
            chpr += printf(" = ");
        if(!feof(inptr) && !ferror(inptr))
        {
            for(j = 0; j < 16; ++i)
            {
                byte = read_ubyte(inptr,dataoffset++);
                if(feof(inptr) || ferror(inptr))
                    break;
                if((PRINT_VALUE))
                    chpr += printf(",%02x",(unsigned int)byte & 0xff);
            }
        }
        else
            break;
        chpr = newline(chpr);
    }
    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(box->tbox,1);
    }
    chpr = newline(chpr);
    return(max_offset);
}

/* The data entry URL box of the uinf superbox.                       */

unsigned long
process_jp2_de(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxoffset,max_offset,dataoffset,tbox;
    unsigned long boxlength;
    unsigned short vers;
    unsigned long flag;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.uinf.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    boxlength = box->boxlength;
    tbox = box->tbox;

    dataoffset = boxoffset + box->dataoffset;
    max_offset = boxoffset + boxlength;

    vers = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.uinf.de.");
        chpr += printf("Version");
    }
    if((PRINT_VALUE))
            chpr += printf(" = %u",vers);
    chpr = newline(chpr);
    dataoffset ++;
    flag = read_ulong(inptr,TIFF_MOTOROLA,dataoffset);
    flag = (flag >> 8) & 0xfff;
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.uinf.de.");
        chpr += printf("flags");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",flag);
    chpr = newline(chpr);
    dataoffset += 3;
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.uinf.de.");
        chpr += printf("Location");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" (length %lu) = ",max_offset - dataoffset);
        /* This should be UTF-8  ###%%%                           */
        setcharsprinted(chpr);
        print_ascii(inptr,max_offset - dataoffset,dataoffset);
    }
    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(tbox,1);
    }
    chpr = newline(chpr);
    return(max_offset);
}

/* The colorspace (colr) of the jp2h superbox.                        */

unsigned long
process_jp2_colr(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxoffset,dataoffset;
    unsigned long boxlength,proflength;
    unsigned short meth = 0;
    unsigned short prec = 0;
    unsigned short approx = 0;
    unsigned short csenum = 0;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.jp2h.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    dataoffset = boxoffset + box->dataoffset;
    boxlength = box->boxlength;

    meth = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.colr.");
        chpr += printf("Method");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",meth);
    chpr = newline(chpr);
    dataoffset ++;

    prec = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.colr.");
        chpr += printf("Precedence");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",prec);
    chpr = newline(chpr);
    dataoffset ++;

    approx = read_ubyte(inptr,dataoffset);
    print_tag_address(ENTRY,dataoffset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2h.colr.");
        chpr += printf("ColorSpaceAproximation");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",approx);
    chpr = newline(chpr);
    dataoffset ++;

    if(meth == 1)
    {
        csenum = read_ulong(inptr,TIFF_MOTOROLA,dataoffset);
        print_tag_address(ENTRY,dataoffset,indent + 8,"=");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.jp2h.colr.");
            chpr += printf("EnumeratedColorSpace");
        }
        if((PRINT_VALUE))
        {
            chpr += printf(" = %u",csenum);
            switch(csenum)
            {
                case 16: chpr += printf(" = sRGB"); break;
                case 17: chpr += printf(" = greyscale"); break;
                default: chpr += printf(" = undefined"); break;
            }
        }
        chpr = newline(chpr);
        dataoffset += 4;
    }
    else
    {
        print_tag_address(ENTRY,dataoffset,indent + 8,"=");
        proflength = boxoffset + boxlength - dataoffset;
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.jp2h.colr.");
            chpr += printf("ICCProfileLength");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",proflength);
        chpr = newline(chpr);
        /* ###%%% display this as soon as we have some ICC routines   */
        dataoffset = boxoffset + boxlength;
    }
    dataoffset = boxoffset + boxlength;
    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(box->tbox,1);
    }
    chpr = newline(chpr);
    return(dataoffset);
}

/* The resolution superbox (res ). May contain one or two sub-boxes.  */
/* This routine handles the sub-boxes directly.                       */

unsigned long
process_jp2_res(FILE *inptr,struct jp2box *box,int indent)
{
    unsigned long boxoffset,max_offset,resboxoffset;
    unsigned long dataoffset,resdataoffset;
    unsigned long resboxlength = 0UL;
    unsigned long boxlength;
    unsigned long tbox;
    unsigned short vrcn = 0;
    unsigned short vrcd = 0;
    unsigned short hrcn = 0;
    unsigned short hrcd = 0;
    unsigned short vrce = 0;
    unsigned short hrce = 0;
    double vrc = 0.0;
    double hrc = 0.0;
    char *rtype;
    char id;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.jp2h.",indent,1);
    else
        print_jp2box(inptr,box,indent);

    /* Save the parent box params.                                    */
    boxoffset = box->boxoffset;
    dataoffset = boxoffset + box->dataoffset;
    boxlength = box->boxlength;
    tbox = box->tbox;
    max_offset = boxoffset + boxlength;

    /* There may be one or two sub-boxes, resc, and resd.             */
    boxlength -= box->dataoffset;

    indent += SUBINDENT;
    if(boxlength >= 14ULL)  /* sub-box must be this long              */
    {
        box = read_jp2box(inptr,dataoffset);

        if((PRINT_LONGNAMES))
            chpr = list_jp2box(inptr,box,"JP2.jp2h.res.",indent,1);
        else
            print_jp2box(inptr,box,indent + SUBINDENT);
        resboxoffset = box->boxoffset;
        resdataoffset = resboxoffset + box->dataoffset;
        resboxlength = box->boxlength;
        boxlength -= resboxlength;

        id = box->tbox & 0xff;
        vrcn = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("vr%cn",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",vrcn);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        vrcd = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("vr%cd",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",vrcd);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        hrcn = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("hr%cn",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",hrcn);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        hrcd = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("hr%cd",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",hrcd);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        vrce = read_ubyte(inptr,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("vr%ce",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",vrce);
        }
        resdataoffset ++;

        if(id == 'c')
            rtype = "Capture";
        else if(id == 'd')
            rtype = "Display";
        else
            rtype = "Illegitimate";

        vrc = (double)vrcn * pow(10.0,(double)vrce) / (double)vrcd ;
        if((LIST_MODE))
            print_tag_address(ENTRY,HERE,indent + 8,"*");
        if((PRINT_TAGINFO))
        {
            if((LIST_MODE))
            {
                if((PRINT_LONGNAMES))
                {
                    chpr += printf("JP2.jp2h.res.");
                    print_jp2type(box->tbox,0);
                    chpr += printf(".");
                }
                chpr += printf("Vertical%sResolution",rtype);
            }
            else
                chpr += printf(" ==> Vertical %s Resolution",rtype);
        }
        if((PRINT_VALUE))
            chpr += printf(" = %3.2f samples/meter",vrc);
        chpr = newline(chpr);

        hrce = read_ubyte(inptr,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("hr%ce",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",hrce);
        }
        resdataoffset ++;

        hrc = (double)hrcn * pow(10.0,(double)hrce) / (double)hrcd ;
        if((LIST_MODE))
            print_tag_address(ENTRY,HERE,indent + 8,"*");
        if((PRINT_TAGINFO))
        {
            if((LIST_MODE))
            {
                if((PRINT_LONGNAMES))
                {
                    chpr += printf("JP2.jp2h.res.");
                    print_jp2type(box->tbox,0);
                    chpr += printf(".");
                }
                chpr += printf("Horizontal%sResolution",rtype);
            }
            else
                chpr += printf(" ==> Horizontal %s resolution",rtype);
        }
        if((PRINT_VALUE))
            chpr += printf(" = %3.2f samples/meter",vrc);
        chpr = newline(chpr);
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,resdataoffset - 1,indent + SUBINDENT,"=");
            print_jp2type(box->tbox,1);
            chpr = newline(chpr);
        }
    }
    chpr = newline(chpr);
    dataoffset += resboxlength;

    /* Sometimes both sub-boxes are present; do it again.             */
    if(boxlength >= 14ULL)
    {
        box = read_jp2box(inptr,dataoffset);

        if((PRINT_LONGNAMES))
            chpr = list_jp2box(inptr,box,"JP2.jp2h.res.",indent,1);
        else
            print_jp2box(inptr,box,indent + SUBINDENT);
        resboxoffset = box->boxoffset;
        resdataoffset = resboxoffset + box->dataoffset;
        resboxlength = box->boxlength;
        boxlength -= resboxlength;

        id = box->tbox & 0xff;
        vrcn = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("vr%cn",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",vrcn);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        vrcd = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("vr%cd",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",vrcd);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        hrcn = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("hr%cn",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",hrcn);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        hrcd = read_ushort(inptr,TIFF_MOTOROLA,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("hr%cd",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",hrcd);
        }
        chpr = newline(chpr);
        resdataoffset += 2;

        vrce = read_ubyte(inptr,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("vr%ce",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",vrce);
        }
        resdataoffset ++;

        if(id == 'c')
            rtype = "Capture";
        else if(id == 'd')
            rtype = "Display";
        else
            rtype = "Illegitimate";

        vrc = (double)vrcn * pow(10.0,(double)vrce) / (double)vrcd ;
        if((PRINT_TAGINFO))
        {
            if((LIST_MODE))
            {
                if((PRINT_LONGNAMES))
                {
                    chpr += printf("JP2.jp2h.res.");
                    print_jp2type(box->tbox,0);
                    chpr += printf(".");
                }
                chpr += printf("Vertical%sResolution",rtype);
            }
            else
                chpr += printf(" ==> Vertical %s Resolution",rtype);
        }
        if((PRINT_VALUE))
            chpr += printf(" = %3.2f samples/meter",vrc);
        chpr = newline(chpr);

        hrce = read_ubyte(inptr,resdataoffset);
        if(!(LIST_MODE))
        {
            print_tag_address(ENTRY,resdataoffset,indent + 8,"=");
            if((PRINT_TAGINFO))
                chpr += printf("hr%ce",id);
            if((PRINT_VALUE))
                chpr += printf(" = %u",hrce);
        }
        resdataoffset ++;

        hrc = (double)hrcn * pow(10.0,(double)hrce) / (double)hrcd ;
        if((PRINT_TAGINFO))
        {
            if((LIST_MODE))
            {
                if((PRINT_LONGNAMES))
                {
                    chpr += printf("JP2.jp2h.res.");
                    print_jp2type(box->tbox,0);
                    chpr += printf(".");
                }
                chpr += printf("Horizontal%sResolution",rtype);
            }
            else
                chpr += printf(" ==> Horizontal %s resolution",rtype);
        }
        if((PRINT_VALUE))
            chpr += printf(" = %3.2f samples/meter",vrc);
        chpr = newline(chpr);
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,resdataoffset - 1,indent + SUBINDENT,"@");
            print_jp2type(box->tbox,1);
            chpr = newline(chpr);
        }
    }
    indent -= SUBINDENT;

    if((PRINT_SECTION))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION|ENTRY,max_offset - 1,indent,"=");
        print_jp2type(tbox,1);
    }
    chpr = newline(chpr);
    return(max_offset);
}

/* Process the jp2 codestream box.                                    */

unsigned long
process_jp2_jp2c(FILE *inptr,struct jp2box *box,struct image_summary *summary_entry,
                                                        int indent)
{
    unsigned long boxoffset,dataoffset;
    unsigned long boxlength;
    unsigned long tbox;
    int chpr = 0;

    if((LIST_MODE))
        chpr = list_jp2box(inptr,box,"JP2.",indent,1);
    else
        print_jp2box(inptr,box,indent);
    boxoffset = box->boxoffset;
    dataoffset = boxoffset + box->dataoffset;
    boxlength = box->boxlength;
    tbox = box->tbox;

    dataoffset = process_jpeg2000_codestream(inptr,dataoffset,boxlength,
                                                summary_entry,indent + 8);
    chpr = newline(chpr);
    if((PRINT_SECTION))
    {
        print_tag_address(SECTION,dataoffset - 1,indent,"@");
        print_jp2type(tbox,1);
    }
    chpr = newline(chpr);
    return(dataoffset);
}

/* Process the jpeg2000 codestream itself                             */

unsigned long
process_jpeg2000_codestream(FILE *inptr,unsigned long marker_offset,
                                    unsigned long data_length,
                                    struct image_summary *summary_entry,
                                    int indent)
{
    unsigned long max_offset = 0L;
    unsigned long start_of_jp2c_data = marker_offset;
    unsigned long start_of_tile = 0;
    unsigned long tile_length = 0;
    unsigned short seg_length,tmp;
    unsigned short tilenum = 0;
    unsigned short part_index = 0;
    unsigned short nparts = 0;
    unsigned short regvalue = 0;
    unsigned short tag = 0;
    int chpr = 0;
    int tagindent,dataindent;
    char *name;

    if(inptr)
    {
        PUSHCOLOR(JPEG_COLOR);
        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
        tagindent = indent;
        while(tag != 0)
        {
            switch(tag)
            {
                case JP2C_SOC:  /* Start of code stream               */
                    start_of_jp2c_data = marker_offset;
                    if((summary_entry == NULL) || summary_entry->entry_lock)
                        summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG2000);
                    if(summary_entry)
                    {
                        if(summary_entry->length <= 0)
                            summary_entry->length = data_length;
                        if(summary_entry->offset <= 0)
                            summary_entry->offset = start_of_jp2c_data;
                        summary_entry->imageformat = IMGFMT_JPEG2000;
                        summary_entry->entry_lock = lock_number(summary_entry);
                    }
                    name = jp2000tagname(tag);
                    print_tag_address(SECTION|ENTRY,marker_offset,tagindent,"@");
                    dataindent = charsprinted();
                    if((LIST_MODE))
                    {
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("JP2.jp2c.");
                            chpr += printf("%s",name);
                        }
                        if((PRINT_VALUE))
                            chpr += printf(" = @%lu",marker_offset);
                        chpr = newline(chpr);
                    }
                    else
                        chpr += printf("<%#x=%s> Start of codestream",tag,name);
                    chpr = newline(chpr);
                    marker_offset += 2;
                    break;
                case JP2C_SOT:  /* Start of tile-part                 */
                    name = jp2000tagname(tag);
                    start_of_tile = marker_offset;
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    print_tag_address(SEGMENT,marker_offset,tagindent,"@");
                    if((PRINT_SEGMENT) && (PRINT_SECTION))
                    {
                        if((LIST_MODE))
                        {
                            if((PRINT_TAGINFO))
                            {
                                if((PRINT_LONGNAMES))
                                    chpr += printf("JP2.jp2c.");
                                chpr += printf("%s",name);
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = @%lu:%u",marker_offset,seg_length);
                            chpr = newline(chpr);
                        }
                        else
                            chpr += printf("<%#x=%s> length %u",tag,name,seg_length);
                    }

                    tilenum = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 4);
                    if((PRINT_SEGMENT) && (PRINT_ENTRY) && !(LIST_MODE))
                        chpr += printf(" tile number %u",tilenum);
                    tile_length = read_ulong(inptr,TIFF_MOTOROLA,marker_offset + 6);
                    if((PRINT_SEGMENT) && (PRINT_ENTRY) && !(LIST_MODE))
                        chpr += printf(" tile_length = %lu",tile_length);
                    part_index = read_ubyte(inptr,HERE);
                    if((PRINT_SEGMENT) && (PRINT_ENTRY) && !(LIST_MODE))
                        chpr += printf(" tile index = %u",part_index);
                    nparts = read_ubyte(inptr,HERE);
                    if((PRINT_SEGMENT) && (PRINT_ENTRY) && !(LIST_MODE))
                        chpr += printf(" [of %u tile parts]",nparts);
                    chpr = newline(chpr);

                    if(ferror(inptr))
                    {
                        tag = 0;
                        continue;
                    }
                    marker_offset += seg_length + 2;
                    break;
                case JP2C_EPH:  /* End of packet header               */
                case JP2C_SOD:  /* Start of data                      */
                    name = jp2000tagname(tag);
                    if((PRINT_SEGMENT) && (PRINT_SECTION))
                    {
                        print_tag_address(SEGMENT,marker_offset,tagindent,"@");
                        if((LIST_MODE))
                        {
                            if((PRINT_TAGINFO))
                            {
                                if((PRINT_LONGNAMES))
                                    chpr += printf("JP2.jp2c.");
                                chpr += printf("%s",name);
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = @%lu",marker_offset);
                            chpr = newline(chpr);
                        }
                        else
                            chpr += printf("<%#x=%s>",tag,name);
                        chpr = newline(chpr);
                    }
                    marker_offset = start_of_tile + tile_length;
                    break;
                case JP2C_COM:
                    name = jp2000tagname(tag);
                    print_tag_address(SECTION|ENTRY,marker_offset,tagindent,"@");
                    if((LIST_MODE))
                    {
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("JP2.jpc.");
                            chpr += printf("Comment");
                        }
                    }
                    else
                        chpr += printf("<%#x=%s>",tag,name);
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    tmp = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 4);

                    if(ferror(inptr) == 0)
                    {
                        if(!(LIST_MODE))
                            chpr += printf(" length %u, reg %u",seg_length,
                                                                    regvalue);
                        if((PRINT_VALUE))
                        {
                            chpr += printf(" = ");
                            setcharsprinted(chpr);
                            (void)print_ascii(inptr,seg_length - 6,marker_offset + 6);
                        }
                        chpr = newline(chpr);
                        marker_offset += seg_length + 2;
                    }
                    else
                    {
                        tag = 0;
                        clearerr(inptr);
                    }
                    break;
                case JP2C_SIZ:
                    marker_offset = process_jpc_siz(inptr,marker_offset,tag,
                                                    summary_entry,tagindent);
                    break;
                case JP2C_COD:
                    marker_offset = process_jpc_cod(inptr,marker_offset,tag,
                                                    summary_entry,tagindent);
                    break;
                case JP2C_QCD:
                    marker_offset = process_jpc_qcd(inptr,marker_offset,tag,
                                                    summary_entry,tagindent);
                    break;
                case JP2C_EOC:
                    name = jp2000tagname(tag);
                    print_tag_address(SECTION|ENTRY,marker_offset,tagindent,"@");
                    if((LIST_MODE))
                    {
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("JP2.jp2c.");
                            chpr += printf("%s",name);
                        }
                        if((PRINT_VALUE))
                            chpr += printf(" = @%lu",marker_offset);
                        chpr = newline(chpr);
                    }
                    else
                        chpr += printf("<%#x=%s> End of codestream",tag,name);
                    chpr = newline(chpr);
                    tag = 0;
                    continue;
                    break;
                /* All of these fall through to the default           */
                case JP2C_CRG:
                case JP2C_SOP:
                case JP2C_POC:
                case JP2C_QCC:
                case JP2C_TLM:  /* Tile-part lengths, main header     */
                case JP2C_PLM:  /* Packet length, main header         */
                case JP2C_PLT:  /* Packet length, tile-part header    */
                case JP2C_PPM:  /* Packed packet headers; main header */
                case JP2C_PPT:  /* Packed packet headers; part header */
                default:
                    name = jp2000tagname(tag);
                    print_tag_address(SEGMENT,marker_offset,tagindent,"@");
                    /* These are exclude from LIST mode unless        */
                    /* SEGMENTS are explicitly enabled                */
                    if((PRINT_SEGMENT))
                    {
                        if((LIST_MODE))
                        {
                            if((PRINT_TAGINFO))
                            {
                                if((PRINT_LONGNAMES))
                                    chpr += printf("JP2.jp2c.");
                                chpr += printf("%s",name);
                            }
                            if((PRINT_VALUE))
                                chpr += printf(" = @%lu",marker_offset);
                        }
                        else
                            chpr += printf("<%#x=%s>",tag,name);
                    }
                    if((tag & 0xff00) != 0xff00)
                    {
                        tag = 0;
                        continue;
                    }
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    if(ferror(inptr) == 0)
                    {
                        if((PRINT_SEGMENT))
                        {
                            if((LIST_MODE))
                                chpr += printf(":%u",seg_length);
                            else
                                chpr += printf(" length %u",seg_length);
                            chpr = newline(chpr);
                        }
                        marker_offset += seg_length + 2;
                    }
                    else
                    {
                        tag = 0;
                        clearerr(inptr);
                    }
                    break;
            }
            tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
            if(ferror(inptr) == 0)
            {
                max_offset = ftell(inptr);
            }
            else
            {
                tag = 0;
                clearerr(inptr);
            }
        }
        POPCOLOR();
        if((summary_entry) && (summary_entry->length <= 0))
                summary_entry->length = max_offset - start_of_jp2c_data;
    }
    setcharsprinted(chpr);
    return(max_offset);
}

/* SIZ header. Prints everything.                                     */

unsigned long
process_jpc_siz(FILE *inptr,unsigned long marker_offset,unsigned short tag,
                                    struct image_summary *summary_entry,
                                    int indent)
{
    unsigned long max_offset;
    unsigned long gridwidth;
    unsigned long gridheight;
    unsigned long Ximgoffset;
    unsigned long Yimgoffset;
    unsigned long tilewidth;
    unsigned long tileheight;
    unsigned long Xtileoffset;
    unsigned long Ytileoffset;
    unsigned short seg_length;
    unsigned short caps;
    unsigned short ncomps;
    unsigned short precision;
    unsigned short hsep;
    unsigned short vsep;
    int chpr = 0;
    int i;
    char *name,*uns;

    name = jp2000tagname(tag);
    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
    print_tag_address(SECTION|ENTRY,marker_offset,indent,"@");

    if((LIST_MODE))
    {
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.jp2c.");
            chpr += printf("%s",name);
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu:%u",marker_offset,seg_length + 2);
    }
    else
        chpr += printf("<%#x=%s> length %u",tag,name,seg_length);
    max_offset = marker_offset + seg_length + 2;
    marker_offset += 4;

    caps = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("Capabilities");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",caps);
    marker_offset += 2;
    gridwidth = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("GridWidth");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",gridwidth);
    marker_offset += 4;
    gridheight = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("GridHeight");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",gridheight);
    marker_offset += 4;
    Ximgoffset = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("XImageOffset");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",Ximgoffset);

    /* Image height and width are normally taken from the ihdr box;   */
    /* if this is a bare codestream encapsulated in e.g. a TIFF file, */
    /* compute image size from grid size and offset                   */
    if(summary_entry && (summary_entry->pixel_width <= 0) &&
                                                (gridwidth > Ximgoffset))
    {
        summary_entry->pixel_width = gridwidth - Ximgoffset;
    }

    marker_offset += 4;
    Yimgoffset = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("YImageOffset");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",Yimgoffset);

    /* Image height and width are normally taken from the ihdr box;   */
    /* if this is a bare codestream encapsulated in e.g. a TIFF file, */
    /* compute image size from grid size and offset                   */
    if(summary_entry && (summary_entry->pixel_height <= 0) &&
                                                (gridheight > Yimgoffset))
    {
        summary_entry->pixel_height = gridheight - Yimgoffset;
    }

    marker_offset += 4;
    tilewidth = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("TileWidth");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",tilewidth);
    marker_offset += 4;
    tileheight = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("TileHeight");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",tileheight);
    marker_offset += 4;
    Xtileoffset = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("Xtileoffset");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",Xtileoffset);
    marker_offset += 4;
    Ytileoffset = read_ulong(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("Ytileoffset");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %lu",Ytileoffset);
    marker_offset += 4;
    ncomps = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
    if(summary_entry)
        summary_entry->spp = ncomps;
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.siz.");
        chpr += printf("NumberOfComponents");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",ncomps);
    marker_offset += 2;
    for(i = 0; i < ncomps; ++i)
    {
        precision = read_ubyte(inptr,marker_offset);
        if(summary_entry)
            summary_entry->bps[i] = precision;
        chpr = newline(chpr);
        if(precision & 0x80)
            uns = NULLSTRING;
        else
            uns = "un";
        print_tag_address(ENTRY,marker_offset,indent + 8,"=");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.jp2c.siz.");
            chpr += printf("Component%dPrecision",i);
        }
        if((PRINT_VALUE))
            chpr += printf(" = %#x = %u bits %ssigned",precision,
                                        (precision & 0x7f) + 1,uns);
        marker_offset++;
        /* ###%%% print these as separate lines?                      */
        hsep = read_ubyte(inptr,marker_offset);
        if((PRINT_SEGMENT) && (PRINT_VALUE) && !(LIST_MODE))
            chpr += printf(", hsep = %u",hsep);
        marker_offset++;
        vsep = read_ubyte(inptr,marker_offset);
        if((PRINT_SEGMENT) && (PRINT_VALUE) && !(LIST_MODE))
            chpr += printf(", vsep = %u",vsep);
        marker_offset++;
    }
    if(!(LIST_MODE))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION|ENTRY,marker_offset - 1,indent,"@");
        chpr += printf("</%#x=%s> ",tag,name);
    }
    if(marker_offset > max_offset)
        printred(" READ PAST END OF BOX");
    chpr = newline(chpr);
    return(max_offset);
}


/* Coding style header. Prints pretty much everything                 */
/* The public spec is way out of date and utterly wrong on this one;  */
/* this is from jasper.                                               */

unsigned long
process_jpc_cod(FILE *inptr,unsigned long marker_offset,unsigned short tag,
                                    struct image_summary *summary_entry,
                                    int indent)
{
    unsigned long max_offset;
    unsigned short seg_length;
    unsigned short cstyle;
    unsigned short decomp;
    unsigned short progorder;
    unsigned short nlayers;
    unsigned short cbw;
    unsigned short cbh;
    unsigned short cbstyle;
    unsigned short transform;
    unsigned short qmfbid;
    unsigned short part_wh;
    char *name;
    int chpr = 0;
    int i;

    name = jp2000tagname(tag);
    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
    print_tag_address(SECTION|ENTRY,marker_offset,indent,"@");
    if((LIST_MODE))
    {
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.jp2c.");
            chpr += printf("%s",name);
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu:%u",marker_offset,seg_length + 2);
    }
    else
        chpr += printf("<%#x=%s> length %u",tag,name,seg_length);
    max_offset = marker_offset + seg_length + 2;
    marker_offset += 4;

    cstyle = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("codingStyle");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" = %#x",cstyle & 0xff);
        chpr += printf(" = entropy coder with");
        if((cstyle & 1) == 0)
            chpr += printf("out");
        chpr += printf(" partitions,");
        if(cstyle & 2)
            chpr += printf(" with SOP markers and");
        else
            chpr += printf(" without SOP markers and");
        if(cstyle & 4)
            chpr += printf(" with EPH markers");
        else
            chpr += printf(" without EPH markers");
    }
    marker_offset++;
    progorder = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("ProgressionOrder");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %#x",progorder & 0xff);
    marker_offset++;
    nlayers = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("NumberOfLayers");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %#x",nlayers & 0xff);
    marker_offset += 2;
    transform = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("MultiComponentTransform");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" = %#x",transform & 0xff);
        if(transform & 1)
            chpr += printf(" = 5/3 reversible");
        else
            chpr += printf(" = 9/7 irreversible");
    }
    /* Record transform in the first byte of                      */
    /* summary_entry->compression; quantization will be recorded  */
    /* in the second byte. This overwrites the useless            */
    /* "compression" indicator in ihdr.                           */
    if(summary_entry)
        summary_entry->compression = transform & 1;
    marker_offset++;
    decomp = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("DecompLevels");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %u",decomp & 0xff);
    marker_offset++;
    cbw = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("CodeBlockWidthExponent");
    }
    if((PRINT_VALUE))
    {
        if((LIST_MODE))
        {
            chpr += printf(" = %#x",(cbw & 0xff) + 2);
            chpr = newline(chpr);
            /* pseudotag                                              */
            print_tag_address(ENTRY,HERE,indent + 8,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("JP2.jp2c.cod.");
                chpr += printf("CodeBlockWidth");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %g",pow(2.0,(double)(cbw + 2)));
        }
        else
            chpr += printf(" = %#x + 2 # cbw -> %g",cbw & 0xff,
                                            pow(2.0,(double)(cbw + 2)));
    }
    marker_offset++;
    cbh = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("CodeBlockHeightExponent");
    }
    if((PRINT_VALUE))
    {
        if((LIST_MODE))
        {
            chpr += printf(" = %#x",(cbh & 0xff) + 2);
            chpr = newline(chpr);
            /* pseudotag                                              */
            print_tag_address(ENTRY,HERE,indent + 8,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("JP2.jp2c.cod.");
                chpr += printf("CodeBlockHeight");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %g",pow(2.0,(double)(cbh + 2)));
        }
        else
            chpr += printf(" = %#x + 2 # cbh -> %g",cbh & 0xff,
                                            pow(2.0,(double)(cbh + 2)));
    }
    marker_offset++;
    cbstyle = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("CodeBLockStyle");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %#x",cbstyle & 0xff);
    marker_offset++;
    qmfbid = read_ubyte(inptr,marker_offset);
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.cod.");
        chpr += printf("QMIFBankId");
    }
    if((PRINT_VALUE))
        chpr += printf(" = %#x",qmfbid & 0xff);
    marker_offset++;
    if(cstyle & 1)
    {
        for(i = 0; i <= decomp; ++i)
        {
            part_wh = read_ubyte(inptr,marker_offset++);
            chpr = newline(chpr);
            print_tag_address(ENTRY,marker_offset,indent + 8,"=");
            if((LIST_MODE))
            {
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("JP2.jp2c.cod.");
                    chpr += printf("Partition%dWidth",i);
                }
                if((PRINT_VALUE))
                    chpr += printf(" = %u",part_wh & 0xf);
                chpr = newline(chpr);

                print_tag_address(ENTRY,marker_offset,indent + 8,"=");
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("JP2.jp2c.cod.");
                    chpr += printf("Partition%dHeight",i);
                }
                if((PRINT_VALUE))
                    chpr += printf(" = %u",(part_wh >> 4) & 0xf);
            }
            else 
            {
                if((PRINT_TAGINFO))
                    chpr += printf("partition %d width",i);
                if((PRINT_VALUE))
                    chpr += printf(" = %u,",part_wh & 0xf);
                    
                if((PRINT_TAGINFO))
                    chpr += printf(" height = ");
                if((PRINT_VALUE))
                    chpr += printf(" = %u",(part_wh >> 4) & 0xf);
            }
        }
    }
    if(!(LIST_MODE))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION|ENTRY,marker_offset - 1,indent,"@");
        chpr += printf("</%#x=%s> ",tag,name);
    }
    if(marker_offset > max_offset)
            printred(" READ PAST END OF BOX");
    chpr = newline(chpr);

    return(max_offset);
}

/* Main quantization header. Prints just the quantization style.      */

unsigned long
process_jpc_qcd(FILE *inptr,unsigned long marker_offset,unsigned short tag,
                                    struct image_summary *summary_entry,
                                    int indent)
{
    unsigned long max_offset;
    unsigned short seg_length;
    unsigned short qstyle;
    unsigned short guardbits;
    unsigned short stepsize,rawstepsize;
    char *name;
    int chpr = 0;
    int i;

    name = jp2000tagname(tag);
    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset +2 );
    print_tag_address(SECTION|ENTRY,marker_offset,indent,"@");

    if((LIST_MODE))
    {
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("JP2.jp2c.");
            chpr += printf("%s",name);
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu:%u",marker_offset,seg_length + 2);
    }
    else
        chpr += printf("<%#x=%s> length %u",tag,name,seg_length);
    max_offset = marker_offset + seg_length + 2;
    marker_offset += 4;

    qstyle = read_ubyte(inptr,marker_offset);
    ++marker_offset;
    chpr = newline(chpr);
    print_tag_address(ENTRY,marker_offset,indent + 8,"=");
    if((PRINT_TAGINFO))
    {
        if((PRINT_LONGNAMES))
            chpr += printf("JP2.jp2c.qcd.");
        chpr += printf("QuantizationStyle");
    }
    if((PRINT_VALUE))
    {
        chpr += printf(" = %#x",qstyle & 0xff);
        chpr += printf(" = ");
        if((qstyle & 0x1f) == 0)
            chpr += printf("no quantization");
        else if((qstyle & 0x1f) == 1)
            chpr += printf("implicit quantization");
        else if((qstyle & 0x1f) == 2)
            chpr += printf("explicit quantization");
        guardbits = (qstyle >> 5) & 0x7;

        chpr += printf(", %u guard bit%s",guardbits,guardbits > 1 ? "s" : "");
    }
    /* Record quantization in the second byte of the summary      */
    /* compression                                                */
    if(summary_entry)
        summary_entry->compression |= (((qstyle & 3) << 8) & 0xff00);
    for(i = 0;marker_offset < (max_offset - 1); ++i)
    {
        chpr = newline(chpr);
        print_tag_address(ENTRY,marker_offset,indent + 8,"=");
        if((qstyle & 0x1f) == 0)
        {
            rawstepsize = read_ubyte(inptr,marker_offset);
            stepsize = rawstepsize >> 3;
            if(stepsize & ~0x1f)
            {
                PUSHCOLOR(RED);
                print_tag_address(ENTRY,marker_offset,indent + SUBINDENT,"@");
                chpr += printf("# WARNING: stepsize exponent %d out of range (%#x => %#x)",
                                                                    i,rawstepsize,stepsize);
                POPCOLOR();
                marker_offset++;
                continue;
                break;
            }
            stepsize <<= 11;
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("JP2.jp2c.");
                chpr += printf("qcd.Stepsize[%d]",i);
            }
            if((PRINT_VALUE))
                chpr += printf(" = %#x -> %u",rawstepsize,stepsize);
            marker_offset++;
        }
        else
        {
            stepsize = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
            marker_offset += 2;
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("JP2.jp2c.qcd.");
                chpr += printf("Stepsize[%d]",i);
            }
            if((PRINT_VALUE))
                chpr += printf(" = %u",stepsize);
        }
    }
    if(!(LIST_MODE))
    {
        chpr = newline(chpr);
        print_tag_address(SECTION|ENTRY,marker_offset - 1,indent,"@");
        chpr += printf("</%#x=%s> ",tag,name);
    }
    if(marker_offset > max_offset)
        printred(" READ PAST END OF BOX");
    chpr = newline(chpr);

    return(max_offset);
}
