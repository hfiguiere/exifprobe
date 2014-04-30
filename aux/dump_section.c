/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* This program reads an arbitrary section of a file from             */
/* 'start_offset' to 'end_offset', printing the data as numerical     */
/* values in one or more of the requested formats.                    */


/* First argument is filename; section to be dumped may be specified  */
/* by a second argument "@start:length" or by second and third        */
/* arguments "start end".                                             */

#ifndef lint
static char *ModuleId = "@(#) $Id: dump_section.c,v 1.3 2005/07/24 22:57:36 alex Exp $";
#endif

#include <stdio.h>
/* #include <stdlib.h> */
#include <limits.h>
#include <string.h>
#include <values.h>  /* Needed for MAXINT, no matter what compiler    */
                     /* may say                                       */

#define END_OF_FILE MAXINT
#define NAMELENGTH  1024
#define TIFF_INTEL      0x4949
#define TIFF_MOTOROLA   0x4d4d
#define HERE    (unsigned long)0xffffffff

extern char *optarg;
char *Progname;
extern int errno;

extern unsigned short read_ushort(FILE *,unsigned short,unsigned long);
extern unsigned short to_ushort(unsigned char *,unsigned short);
extern unsigned long read_ulong(FILE *,unsigned short,unsigned long);
extern unsigned long to_ulong(unsigned char *,unsigned short);



main(int argc,char **argv)
{
    char *arg,*infilename,*colon;
    unsigned long offset = 0UL;
    unsigned long start_offset = 0UL;
    unsigned long end_offset = END_OF_FILE;
    int opt,i;
    FILE *ifp;
    unsigned long section_size = 0;
    unsigned long lvalue;
    unsigned short svalue;
    unsigned short byteorder = TIFF_INTEL;
    unsigned short snumerator,sdenominator;
    unsigned long lnumerator,ldenominator;
    double rvalue;
    int show_bytes = 0;
    int show_bytes_decimal = 0;
    int show_bytes_hex = 0;
    int show_short = 0;
    int show_short_decimal = 0;
    int show_short_hex = 0;
    int show_long = 0;
    int show_long_decimal = 0;
    int show_long_hex = 0;
    int show_rational = 0;
    int show_long_rational = 0;
    int show_short_rational = 0;

    Progname = *argv++;
    --argc;
    while(argc > 2 && *argv)
    {
        arg = *argv;
        if(arg && (*arg == '-'))
        {
            switch(*++arg)
            {
            case 'b': /* bytes */
                show_bytes = 1;
                if(*++arg == '\0')
                {
                    show_bytes_decimal = 1;
                    show_bytes_hex = 1;
                }
                if(strchr(arg,'d'))
                    show_bytes_decimal = 1;
                if(strchr(arg,'h'))
                    show_bytes_hex = 1;
                break;
            case 's': /* shorts */
                show_short = 1;
                if(*++arg == '\0')
                {
                    show_short_decimal = 1;
                    show_short_hex = 1;
                }
                if(strchr(arg,'d'))
                    show_short_decimal = 1;
                if(strchr(arg,'h'))
                    show_short_hex = 1;
                break;
            case 'l': /* longs */
                show_long = 1;
                if(*++arg == '\0')
                {
                    show_long_decimal = 1;
                    show_long_hex = 1;
                }
                if(strchr(arg,'d'))
                    show_long_decimal = 1;
                if(strchr(arg,'h'))
                    show_long_hex = 1;
                break;
            case 'r':
                show_rational = 1;
                if(*++arg == '\0')
                {
                    show_long_rational = 1;
                    show_short_rational = 1;
                }
                if(strchr(arg,'l'))
                    show_long_rational = 1;
                if(strchr(arg,'s'))
                    show_short_rational = 1;
                break;
            case 'i': /* Intel byteorder */
                byteorder = TIFF_INTEL;
                break;
            case 'm': /* Motorola byteorder */
                byteorder = TIFF_MOTOROLA;
                break;
                
            }
            --argc;
            ++argv;
        }
        else
            break;
    }

    if(argc >= 1)
    {
        infilename = *argv;
        arg = *++argv;
        if(arg && (*arg == '@'))
        {
            start_offset = strtoul(++arg,(char **)0,0);
            colon = strchr(arg,':');
            if(colon)
                end_offset = start_offset + strtoul(++colon,(char **)0,0) - 1;
        }
        else if(arg)
        {
            start_offset = strtoul(arg,(char **)0,0);
            if(*++argv)
            {
                arg = *argv;
                end_offset = strtoul(arg,(char **)0,0);
            }
        }
    }

    printf("input file %s\n",infilename);
    printf("start_offset = %lu, ",start_offset);
    section_size =  end_offset - start_offset + 1;
    if(end_offset == END_OF_FILE)
        printf("end_offset = end of file\n");
    else
        printf("end_offset = %ld, section length = %ld\n",end_offset,section_size);
    printf("byteorder = %2.2s\n",(char *)&byteorder);

    if((ifp = fopen(infilename,"r")) != (FILE *)0)
    {
        if(show_bytes)
        {
            if(show_bytes_decimal)
            {
                fseek(ifp,start_offset,SEEK_SET);
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned byte - decimal:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    svalue = fgetc(ifp);
                    printf("%#06lx/%-04lu: %-12u",offset,offset,svalue);
                    ++offset;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
            }
            if(show_bytes_hex)
            {
                printf("\n=============================================\nUnsigned byte - hex:\n");
                fseek(ifp,start_offset,SEEK_SET);
                offset = start_offset;
                i = 0;
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    svalue = fgetc(ifp);
                    printf("%#06lx/%-04lu: %-12#x",offset,offset,svalue);
                    ++offset;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
            }
        }
        if(show_short)
        {
            if(show_short_decimal)
            {
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned short - decimal:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    svalue = read_ushort(ifp,byteorder,offset);
                    printf("%#06lx/%-04lu: %-12u",offset,offset,svalue);
                    offset += 2;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
            }
            if(show_short_hex)
            {
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned short - hex:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    svalue = read_ushort(ifp,byteorder,offset);
                    printf("%#06lx/%-04lu: %-12#x",offset,offset,svalue);
                    offset += 2;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
            }
        }
        if(show_long)
        {
            if(show_long_decimal)
            {
                fseek(ifp,start_offset,SEEK_SET);
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned long - decimal:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    lvalue = read_ulong(ifp,byteorder,offset);
                    printf("%#06x/%-04lu: %-12lu",offset,offset,lvalue);
                    offset += 4;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
                putchar('\n');
            }
            if(show_long_hex)
            {
                fseek(ifp,start_offset,SEEK_SET);
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned long - hex:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    lvalue = read_ulong(ifp,byteorder,offset);
                    printf("%#06x/%-04lu: %-12#lx",offset,offset,lvalue);
                    offset += 4;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
                putchar('\n');
            }
        }

        if(show_rational)
        {
            if(show_long_rational)
            {
                fseek(ifp,start_offset,SEEK_SET);
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned long - rational:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    rvalue = 0.0;
                    lnumerator = read_ulong(ifp,byteorder,offset);
                    ldenominator = read_ulong(ifp,byteorder,HERE);
                    if(sdenominator)
                        rvalue = (double)lnumerator / (double)ldenominator;
                    printf("%#06x/%-04lu: %-12.8f",offset,offset,rvalue);
                    offset += 8;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
                putchar('\n');
            }
            if(show_short_rational)
            {
                fseek(ifp,start_offset,SEEK_SET);
                offset = start_offset;
                i = 0;
                printf("\n=============================================\nUnsigned short rational:\n");
                while((offset <= end_offset) && !feof(ifp) && !ferror(ifp))
                {
                    rvalue = 0.0;
                    snumerator = read_ushort(ifp,byteorder,offset);
                    sdenominator = read_ushort(ifp,byteorder,HERE);
                    if(sdenominator)
                        rvalue = (double)snumerator / (double)sdenominator;
                    printf("%#06x/%-04lu: %-12.8f",offset,offset,rvalue);
                    offset += 4;
                    if(++i == 4)
                    {
                        putchar('\n');
                        i = 0;
                    }
                }
                putchar('\n');
            }
        }
    }
    else
    {
        fprintf(stderr,"%s: could not open input file \"%s\"\n",Progname,infilename);
        perror("\tbecause");
        exit(3);
    }
    exit(0);
}

unsigned short
read_ushort(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    unsigned char rbuf[2];
    unsigned long curoffset;
    unsigned short value = 0;
    int chpr = 0;

    if(inptr /*&& !feof(inptr) && !ferror(inptr)*/)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            printf(" SEEK FAILED to read unsigned short at offset ");
            printf("%lu",offset);
        }
        else if(fread(rbuf,1,sizeof(unsigned short),inptr) == sizeof(unsigned short))
            value = to_ushort(rbuf,byteorder);
        else
        {
            printf(" FAILED to read unsigned short value at offset ");
            if(offset == HERE)
                offset = curoffset;
            chpr += printf("%lu",offset);
        }
        if(ferror(inptr))
        {
            chpr += printf(" (");
            printf(strerror(errno));
            chpr += printf(")");
            chpr = putchar('\n');
        }
        else if(feof(inptr))
        {
            printf(" (EOF)");
            chpr = putchar('\n');
        }
    }
    return(value);
}

unsigned long
read_ulong(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    unsigned char rbuf[4];
    unsigned long curoffset;
    unsigned long value = 0;
    int chpr = 0;

    if(inptr /* && !feof(inptr) && !ferror(inptr)*/)
    {
        clearerr(inptr);
        curoffset = ftell(inptr);
        if((offset != HERE) && (fseek(inptr,offset,SEEK_SET) == -1))
        {
            printf(" SEEK FAILED to read unsigned 32bit integer at offset ");
            printf("%lu",offset);
        }
        else if(fread(rbuf,1,4,inptr) == 4)
            value = to_ulong(rbuf,byteorder);
        else
        {
            printf(" FAILED to read unsigned 32bit integer at offset ");
            if(offset == HERE)
                offset = curoffset;
            chpr += printf("%lu",offset);
        }
        if(ferror(inptr))
        {
            chpr += printf(" (");
            printf(strerror(errno));
            chpr += printf(")");
            putchar('\n');
        }
        else if(feof(inptr))
        {
            printf(" (EOF)");
            chpr = putchar('\n');
        }
    }
    return(value);
}

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

unsigned long
to_ulong(unsigned char *buf,unsigned short byteorder)
{
    unsigned long value = 0;

    if(buf)
    {
        value = *(unsigned long *)buf;
#ifdef NATIVE_BYTEORDER_BIGENDIAN
        if(byteorder == TIFF_INTEL)
#else
        if(byteorder == TIFF_MOTOROLA)
#endif
            value = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    }
    return((unsigned long)value);
}

