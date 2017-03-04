/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: readfile.c,v 1.30 2005/07/24 18:15:28 alex Exp $";
#endif

/* This file contains routines for reading specific value types with  */
/* byte-swapping for integer values, specialized routines for         */
/* strings, a file header decoder for various imagefile types, and a  */
/* general IFD entry reader.                                          */

/* The code needs to know the native byteorder of the machine, which  */
/* is currently not autoconfigured. If not sure, look for a file like */
/* <sys/endian.h>. "MOTOROLA" byteorder is "big-endian", "INTEL"      */
/* byteorder is "little-endian". The code does not currently handle   */
/* "pdp11" byteorder.                                                 */


#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "byteorder.h"

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "ciff.h"
#include "x3f.h"

/* Read a short integer value in the expectation that it is a TIFF or */
/* JPEG header marker/tag. If a valid JPEG_SOI is found, we're done.  */
/* If the value is a TIFF indicator, but the TIFF magic marker is not */
/* found, the magic number is then checked for a valid ORF or CIFF    */
/* header. If the marker is not a recognized magic number/string,     */
/* start over and check for a JP2 header box, or MRW, RAF, or X3F     */
/* header.                                                            */

/* When appropriage, ciff, mrw, x3f or jp2 header structures are      */
/* filled in and the fileheader magic set to indictate which is       */
/* valid.                                                             */

/* The return is a fileheader structure containing an exifprobe       */
/* private "magic number" and appropriate information about the file  */
/* structure, if any.                                                 */


struct fileheader *
read_imageheader(FILE *inptr,unsigned long offset)
{
    static struct fileheader fileheader;
    struct ciff_header *ciffheader;
    struct jp2_header jp2header;
    struct mrw_header mrwheader;
    struct x3f_header *x3fheader;
    unsigned short marker,magic;
    unsigned char *type;
    unsigned long umagic,length;
    char *string;

    marker = 0;
    fileheader.probe_magic = PROBE_NOMAGIC;
    fileheader.file_marker = 0;
    fileheader.ciff_header = NULL;
    fileheader.x3f_header = NULL;
    memset(&fileheader.jp2_header,'\0',sizeof(struct jp2_header));
    memset(&fileheader.mrw_header,'\0',sizeof(struct mrw_header));
    if(inptr && !feof(inptr) && !ferror(inptr))
    {
        /* The order of bytes for this read does not matter for TIFF, */
        /* but must be high-byte first for JPEG.                      */
        marker = read_ushort(inptr,TIFF_MOTOROLA,offset);
        switch(marker)
        {
            case TIFF_MOTOROLA:
            case TIFF_INTEL:
                if((magic = read_ushort(inptr,marker,HERE)) == TIFF_MAGIC)
                {
                    fileheader.probe_magic = TIFF_MAGIC;
                    fileheader.file_marker = marker;
                }
                else if(magic == ORF1_MAGIC)
                {
                    fileheader.probe_magic = ORF1_MAGIC;
                    fileheader.file_marker = marker;
                }
                else if(magic == ORF2_MAGIC)
                {
                    fileheader.probe_magic = ORF2_MAGIC;
                    fileheader.file_marker = marker;
                }
                else if(magic == RW2_MAGIC)
                {
                    fileheader.probe_magic = RW2_MAGIC;
                    fileheader.file_marker = marker;
                }
                else if((ciffheader = read_ciffheader(inptr,marker,offset)))
                {
                    fileheader.file_marker = ciffheader->byteorder;
                    fileheader.probe_magic = PROBE_CIFFMAGIC;
                    fileheader.ciff_header = ciffheader;
                }
                else
                {
                    fileheader.probe_magic = PROBE_NOMAGIC;
                    fileheader.file_marker = 0;
                    printf(" invalid TIFF magic (%#06x)",magic);
                    printf(", apparent TIFF byteorder is ");
                    print_byteorder(marker,1);
                    (void)newline(0);
                }
                break;
            case JPEG_SOI:
                fileheader.probe_magic = JPEG_SOI;
                fileheader.file_marker = marker;
                break;
            default:
                fileheader.probe_magic = PROBE_NOMAGIC;
                fileheader.file_marker = 0;
                if(marker == 0)
                {
                    /* check for jpeg2000                             */
                    length = read_ulong(inptr,TIFF_MOTOROLA,offset);
                    if(length == 12)
                    {
                        type = read_bytes(inptr,4,HERE);
                        if(type && ((strncmp((char *)type,"jP\040\040  ",4) == 0) ||
                                    (strncmp((char *)type,"jP\032\032  ",4) == 0)))
                        {
                            umagic = read_ulong(inptr,TIFF_MOTOROLA,HERE);
                            if(umagic == 0x0d0a870a)
                            {
                                fileheader.probe_magic = PROBE_JP2MAGIC;
                                fileheader.file_marker = 0;
                                jp2header.length = length;
                                jp2header.type = to_ulong(type,TIFF_MOTOROLA);
                                if(!ferror(inptr) && !feof(inptr))
                                {
                                    jp2header.magic = umagic;
                                    fileheader.jp2_header = jp2header;
                                }
                            }
                        }
                    }
                }
                else if(marker == 0x004d)
                {
                    umagic = read_ulong(inptr,TIFF_MOTOROLA,offset);
                    if(umagic == 0x004d524d)
                    {
                        /* Minolta MRW */
                        mrwheader.mrw_magic = umagic;
                        mrwheader.mrw_dataoffset = read_ulong(inptr,TIFF_MOTOROLA,HERE);
                        if(!ferror(inptr) && !feof(inptr))
                        {
                            fileheader.probe_magic = PROBE_MRWMAGIC;
                            fileheader.mrw_header = mrwheader;
                        }
                    }
                }
                else if((marker == 0x5546) || (marker == 0x4655))
                {
                    /* Fujifile RAF */
                    string = read_string(inptr,offset,16);
                    if(string && (strncmp(string,"FUJIFILMCCD-RAW ",16) == 0))
                        fileheader.probe_magic = PROBE_RAFMAGIC;
                }
                else if(marker == 0x464f) /* "FO" */
                {
                    /* Sigma/Foveon X3F */
                    umagic = read_ulong(inptr,TIFF_INTEL,offset);
                    if(umagic == PROBE_X3FMAGIC)
                    {
                        x3fheader = read_x3fheader(inptr,offset);
                        if(x3fheader)
                        {   fileheader.x3f_header = x3fheader;
                            fileheader.probe_magic = PROBE_X3FMAGIC;
                        }
                    }
                }
                else
                {
                    /* Unknown format                                 */
                    fileheader.probe_magic = PROBE_NOMAGIC;
                    fileheader.file_marker = 0;
                    fileheader.ciff_header = NULL;
                    fileheader.x3f_header = NULL;
                }
                break;
        }
    }
    return(&fileheader);
}

/* Read a generic TIFF IFD entry at the given offset, swapping        */
/* byteorder of entry values as required.                             */

/* The return is a pointer to a static structure representing the     */
/* entry.                                                             */

struct ifd_entry *
read_ifd_entry(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    static struct ifd_entry entry;
    unsigned char *value;

    memset(&entry,0,sizeof(struct ifd_entry));
    if(inptr && !feof(inptr) && !ferror(inptr))
    {
        entry.tag = read_ushort(inptr,byteorder,offset);
        entry.value_type = read_ushort(inptr,byteorder,HERE);
        entry.count = read_ulong(inptr,byteorder,HERE);
        value = read_bytes(inptr,4,HERE);
        if(value)
        {
            if((value_type_size(entry.value_type) * entry.count) > 4)
                entry.value = to_ulong(value,byteorder);    /* offset */
            else    /* actual value(s)                                */
            {
                entry.value = 0L;
                switch(entry.value_type)
                {
                    case SSHORT: 
                    case SHORT:
                        if(entry.count == 1)
                            entry.value = to_ushort(value,byteorder);
                        else
                        {
                            entry.value = to_ushort(value,byteorder);
                            if(byteorder == TIFF_MOTOROLA)
                                entry.value |= (((unsigned long)to_ushort(value + 2,byteorder) & 0xffff) << 16);
                            else
                                entry.value = (to_ushort(value + 2,byteorder) & 0xffff) |
                                    ((entry.value & 0xffff) << 16);
                        }
                        break;
                    case BYTE:
                    case SBYTE:
                    case UNDEFINED:
                    case ASCII:
                        entry.value = *(unsigned long *)value;
                        break;
                    case LONG:
                    case SLONG:
                    default:
                        entry.value = to_ulong(value,byteorder);
                        break;
                }
            }
        }
        else
            entry.value = 0L;
    }
    return(&entry);
}

/* Read 2 bytes at the given offset and interpret them as an unsigned */
/* short integer value, swapping bytes as necessary.                  */

unsigned short
read_ushort(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    unsigned char rbuf[2];
    unsigned long curoffset;
    unsigned short value = 0;
    int chpr = 0;

    if(inptr)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            printred(" SEEK FAILED to read unsigned short at offset ");
            printf("%lu",offset);
        }
        else if(fread(rbuf,1,sizeof(unsigned short),inptr) == sizeof(unsigned short))
            value = to_ushort(rbuf,byteorder);
        else
        {
            printred(" FAILED to read unsigned short value at offset ");
            if(offset == HERE)
                offset = curoffset;
            chpr += printf("%lu",offset);
        }
        if(ferror(inptr))
        {
            chpr += printf(" (");
            setcharsprinted(chpr);
            chpr = 0;
            printred(strerror(errno));
            chpr += printf(")");
            chpr = newline(chpr);
        }
        else if(feof(inptr))
        {
            printred(" (EOF)");
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(value);
}

/* read a single byte at the given offset and return it as an         */
/* unsigned short value                                               */

unsigned short
read_ubyte(FILE *inptr,unsigned long offset)
{
    unsigned char rbuf[1];
    unsigned long curoffset;
    unsigned short value = 0;
    int chpr = 0;

    if(inptr)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            printred(" SEEK FAILED to read unsigned byte at offset ");
            chpr += printf("%lu",offset);
        }
        else if(fread(rbuf,1,1,inptr) == 1)
            value = rbuf[0] & 0xff;
        else
        {
            if(offset == HERE)
                offset = curoffset;
            printred(" FAILED to read unsigned byte at offset ");
            chpr += printf("%lu",offset);
        }
        if(ferror(inptr))
        {
            chpr += printf(" (");
            setcharsprinted(chpr);
            chpr = 0;
            printred(strerror(errno));
            chpr += printf(")");
            chpr = newline(chpr);
        }
        else if(feof(inptr))
        {
            printred(" (EOF)");
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(value);
}

/* Read 4 bytes at the given offset and interpret as an unsigned long */
/* integer, swapping bytes as necessary                               */

/* NOTE: the routine *reads* a 4 byte number, but returns it as an    */
/* unsigned long in the native machine size.                          */

unsigned long
read_ulong(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    unsigned char rbuf[4];
    unsigned long curoffset;
    unsigned long value = 0;
    int chpr = 0;

    if(inptr)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            printred(" SEEK FAILED to read unsigned 32bit integer at offset ");
            printf("%lu",offset);
        }
        else if(fread(rbuf,1,4,inptr) == 4)
            value = to_ulong(rbuf,byteorder);
        else
        {
            printred(" FAILED to read unsigned 32bit integer at offset ");
            if(offset == HERE)
                offset = curoffset;
            chpr += printf("%lu",offset);
        }
        if(ferror(inptr))
        {
            chpr += printf(" (");
            setcharsprinted(chpr);
            chpr = 0;
            printred(strerror(errno));
            chpr += printf(")");
            chpr = newline(chpr);
        }
        else if(feof(inptr))
        {
            printred(" (EOF)");
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(value);
}


float
read_float(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    float value = 0.0;
    unsigned long ulval;

    ulval = read_ulong(inptr,byteorder,offset);
    value = to_float(ulval);
    return(value);
}

/* Read 8 bytes at the given offset and interpret as a long long,     */
/* swapping bytes as necessary.                                       */

unsigned long long
read_ulong64(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    unsigned char rbuf[sizeof(double)];
    unsigned long curoffset;
    unsigned long long value = 0;
    int chpr = 0;

    if(inptr)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
            chpr += printf(" seek failed to offset %lu to read unsigned long\n",
                    offset);
        else if(fread(rbuf,1,sizeof(long long),inptr) == sizeof(long long))
            value = to_ulong64(rbuf,byteorder);
        else
        {
            printred(" FAILED to read unsigned 64bit integer at offset ");
            if(offset == HERE)
                offset = curoffset;
            chpr += printf("%lu\n",offset);
        }
    }
    setcharsprinted(chpr);
    return(value);
}

unsigned long long
to_ulong64(unsigned char *buf,unsigned short byteorder)
{
    unsigned long long value = 0;
    unsigned char swap[sizeof(long long)];
    unsigned char *p;
    int i,size;

    if(buf)
    {
        p = buf;
#ifdef NATIVE_BYTEORDER_BIGENDIAN
        if(byteorder == TIFF_INTEL)
#else
        if(byteorder == TIFF_MOTOROLA)
#endif
        {
            size = sizeof(long long);
            for(i = 0; i < size; ++i)
                swap[i] = buf[size - 1 - i];
            p = swap;
        }
        value = *(long long *)p;
    }
    return(value);
}



/* Read 8 bytes at the given offset and interpret as a double,        */
/* swapping bytes as necessary.                                       */

double
read_double(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    unsigned char rbuf[sizeof(double)];
    unsigned long curoffset;
    double value = 0.0;
    int chpr = 0;

    if(inptr)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
            chpr += printf(" seek failed to offset %lu to read unsigned long\n",
                    offset);
        else if(fread(rbuf,1,sizeof(double),inptr) == sizeof(double))
            value = to_double(rbuf,byteorder);
        else
        {
            if(offset == HERE)
                offset = curoffset;
            PUSHCOLOR(RED);
            chpr += printf(" failed to read double value at offset %lu\n",offset);
            POPCOLOR();
        }
    }
    setcharsprinted(chpr);
    return(value);
}

/* Read 'nbytes' of bytes at the given offset; no byte swapping.      */
/* The result is a chunk of bytes...not a string.                     */

unsigned char *
read_bytes(FILE *inptr,unsigned long nbytes,unsigned long offset)
{
    static unsigned char rbuf[MAXBUFLEN];
    unsigned long curoffset;
    unsigned char *value = NULL;
    int chpr = 0;

    memset(rbuf,0,MAXBUFLEN);
    if(inptr)
    {
        if(nbytes > MAXBUFLEN)
        {
            chpr += printf(" read %lu bytes would overrun buffer; truncating to %d\n",
                                                                nbytes,MAXBUFLEN);
            nbytes = MAXBUFLEN;
        }
        if(nbytes)
        {
            if(inptr) /* ###%%% redundant; FIXME                      */
            {
                clearerr(inptr);
                curoffset = ftell(inptr);
                if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
                {
                    printred(" SEEK FAILED to read unsigned bytes at offset ");
                    chpr += printf("%lu",offset);
                }
                else
                {
                    if(fread(rbuf,1,nbytes,inptr) == nbytes)
                        value = rbuf;
                    else
                    {
                        if(offset == HERE)
                            offset = curoffset;
                        PUSHCOLOR(RED);
                        chpr += printf(" FAILED to read %lu unsigned bytes at offset ",nbytes);
                        chpr += printf("%lu",offset);
                        POPCOLOR();
                    }
                }
                if(ferror(inptr))
                {
                    chpr += printf(" (");
                    setcharsprinted(chpr);
                    chpr = 0;
                    printred(strerror(errno));
                    chpr += printf(")");
                    chpr = newline(chpr);
                }
                else if(feof(inptr))
                {
                    printred(" (EOF)");
                    chpr = newline(chpr);
                }
            }
        }
    }
    return(value);
}

/* convert a couple of bytes to an unsigned short integer in native   */
/* byte order, swapping bytes if file byteorder differs.              */
/* If byteorder is not TIFF_INTEL or TIFF_MOTOROLA, the native byte   */
/* order is used.                                                     */

unsigned short
to_ushort(unsigned char *buf,unsigned short byteorder)
{
    unsigned short value = 0;

    if(buf)
    {
        value = *(unsigned short *)buf;
#ifdef NATIVE_BYTEORDER_BIGENDIAN
        if(byteorder == TIFF_INTEL)
            value = (buf[1] << 8) | buf[0];
#else
        if(byteorder == TIFF_MOTOROLA)
            value = (buf[0] << 8) | buf[1];
#endif
    }
    return(value);
}

/* convert a bunch of bytes to an unsigned long integer in native     */
/* byte order, swapping bytes if file byteorder differs.              */
/* If byteorder is not TIFF_INTEL or TIFF_MOTOROLA, the native byte   */
/* order is used.                                                     */

unsigned long
to_ulong(unsigned char *buf,unsigned short byteorder)
{
    u_int32_t value = 0;

    if(buf)
    {
        value = *(u_int32_t *)buf;
#ifdef NATIVE_BYTEORDER_BIGENDIAN
        if(byteorder == TIFF_INTEL)
#else
        if(byteorder == TIFF_MOTOROLA)
#endif
            value = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    }
    return((unsigned long)value);
}

/* convert a bunch of bytes to a 'float' value in native byte order,  */
/* swapping bytes if file byteorder differs. If byteorder is not      */
/* TIFF_INTEL or TIFF_MOTOROLA, the native byte order is used.        */

/* This is pretty primitive; it assumes that the native floating      */
/* point format is the same as the floating point format written to   */
/* the image file, probably IEEE as required by the TIFF6             */
/* specification. See 'libtiff' for some conversion routines.         */

float
to_float(unsigned long uval)
{
    float value = 0.0;
    unsigned long *uval_ptr = &uval;

    value = *(float *)uval_ptr;
    return(value);
}

/* convert a bunch of bytes to a 'double' value in native byte order, */
/* swapping bytes if file byteorder differs. If byteorder is not      */
/* TIFF_INTEL or TIFF_MOTOROLA, the native byte order is used.        */

/* This is pretty primitive; it assumes that the native floating      */
/* point format is the same as the floating point format written to   */
/* the image file, probably IEEE as required by the TIFF6             */
/* specification. See 'libtiff' for some conversion routines.         */

double
to_double(unsigned char *buf,unsigned short byteorder)
{
    double value = 0.0;
    unsigned char swap[sizeof(double)];
    unsigned char *p;
    int i,size;

    if(buf)
    {
        p = buf;
#ifdef NATIVE_BYTEORDER_BIGENDIAN
        if(byteorder == TIFF_INTEL)
#else
        if(byteorder == TIFF_MOTOROLA)
#endif
        {
            size = sizeof(double);
            for(i = 0; i < size; ++i)
                swap[i] = buf[size - 1 - i];
            p = swap;
        }
        value = *(double *)p;
    }
    return(value);
}

/* Read a few bytes at 'offset' which are expected to be a JPEG APPn  */
/* id string, where 'n' is given as an argument.                      */

char *
read_appstring(FILE *inptr,unsigned short apptype,unsigned long offset)
{
    unsigned long length;
    char *appstring = NULLSTRING;


    if(inptr)
    {
        switch(apptype)
        {
            case JPEG_APP0:
                length = 32;    /* should be 5; allow for garbage     */
                break;
            case JPEG_APP1:
            case JPEG_APP2:
                length = 32;    /* should be 5; allow for garbage     */
                break;
            default:
                length = 32; 
                break;
        }
        if(length)
            appstring = read_string(inptr,offset,length);
    }
    return(appstring);
}

/* Read an ascii string, up to a null byte or 'max_bytes', at         */
/* 'offset' relative to the beginning of the file.                    */
/* Returns the string in static storage.  Use it or lose it.          */

char *
read_string(FILE *inptr,unsigned long offset,unsigned long max_bytes)
{
    static char stringbuf[MAXBUFLEN];
    unsigned long curoffset;
    char *bufp;
    int nread = 0;
    int chpr = 0;

    bufp = stringbuf;
    *bufp = '\0';

    if(inptr)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            printred(" SEEK FAILED to read ascii string at offset ");
            chpr += printf("%lu",offset);
        }
        else if(max_bytes > 0)
        {
            if(max_bytes > MAXBUFLEN)
                max_bytes = MAXBUFLEN - 2;
            while((nread < max_bytes) &&
                    (*bufp = fgetc(inptr)) &&
                        !feof(inptr) && 
                            !ferror(inptr) &&
                                    isascii(*bufp++))
            {
                    ++nread;
            }
            *bufp = '\0';
            if(feof(inptr) || ferror(inptr))
            {
                if(offset == HERE)
                    offset = curoffset;
                PUSHCOLOR(RED);
                chpr += printf(" FAILED to read character at offset ");
                chpr += printf("%lu",offset);
                POPCOLOR();
                if(ferror(inptr))
                {
                    chpr += printf(" (");
                    setcharsprinted(chpr);
                    chpr = 0;
                    printred(strerror(errno));
                    chpr += printf(")");
                    chpr = newline(chpr);
                }
                else if(feof(inptr))
                {
                    printred(" (EOF)");
                    chpr = newline(chpr);
                }
            }
        }
    }
    setcharsprinted(chpr);
    return(stringbuf);
}

int
ateof(FILE *inptr)
{
    int atend = 0;
    if(feof(inptr))
        atend++;
    else if(fgetc(inptr) == -1)
        atend++;
    return(atend);
}

/* Get the size of the file open on the argument stream, by seeking   */
/* to the end and enquiring of the resulting offset. Return the file  */
/* offset to its original location. If anything goes wrong, return 0, */
/* indicating failure (or a zero length file).                        */

unsigned long
get_filesize(FILE *inptr)
{
    unsigned long size = 0L;
    long current_offset;

    if(inptr)
    {
        clearerr(inptr);
        /* move back here after getting the size                      */
        current_offset = ftell(inptr);
        if(fseek(inptr,0L,SEEK_END) == 0)
        {
            size = ftell(inptr);
            if(fseek(inptr,current_offset,SEEK_SET) != 0)
                clearerr(inptr);    /* silently                       */
        }
        else
            size = 0L;
    }
    return(size);
}
