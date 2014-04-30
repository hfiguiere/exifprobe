/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: findjpeg.c,v 1.1 2005/06/30 17:50:27 alex Exp $";
#endif

/* This program does a quick sweep through any file looking for JPEG  */
/* data streams bracketed by JPEG_SOI and JPEG_EOI, and attempts to   */
/* show start offset and length of segments found. It crudely         */
/* attempts to pair up SOI and EOI when embedded images are found     */
/* (e.g. in APP1/TIFF/EXIF files).                                    */

/* Output should be taken with a grain of salt; the tag numbers       */
/* representing JPEG tags may occur randomly in any binary format     */
/* file. The program also recognizes and reports a small subset of    */
/* additional jpeg markers which should occur in a jpeg stream, as an */
/* aid to recognizing real image streams.                             */

/* In particular, a lookout should be kept for spurious APPn markers, */
/* since the program does not look for the "header" information which */
/* validates the section.                                             */

/* The program will check a section of a file if given a second       */
/* argument of "@offset:length" or second and third arguments "offset */
/* length", which may be in decimal of hex formats.                   */

/* The first argument must be a valid filename                        */

/* Suspected jpeg streams are reported at the end as "@offset:length. */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define END_OF_FILE  0xffffffff

extern char *optarg;

main(int argc,char **argv)
{
    int opt;
    unsigned long start_offset = 0UL;
    unsigned long end_offset = END_OF_FILE;
    char *arg,*filename,*colon;
    int soi[16];
    int eoi[16];
    int soi_index = 0;
    int eoi_index = 0;
    int tagloc = 0;
    FILE *fp;
    int highbyte,lobyte;
    int i,j;
    int tabs = 0;
    int hadsoi = 0;

    memset(soi,0,16);
    memset(eoi,0,16);
    if(argc >= 2)
    {
        filename = *++argv;
        arg = *++argv;
        if(arg && (*arg == '@'))
        {
            start_offset = strtoul(++arg,(char **)0,0);
            colon = strchr(arg,':');
            if(colon)
                end_offset = start_offset + strtoul(++colon,(char **)0,0);
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
    printf("filename=%s\n",filename);
    printf("start_offset = %lu\n",start_offset);
    if(end_offset == END_OF_FILE)
        printf("end_offset = end of file\n");
    else
        printf("end_offset = %lu\n",end_offset);

    if((fp = fopen(filename,"r")) != (FILE *)0)
    {
        fseek(fp,start_offset,SEEK_SET);
        while(((highbyte = fgetc(fp)) != EOF) && (ftell(fp) <= end_offset))
        {
            if(highbyte != 0xff)
                continue;
            tagloc = ftell(fp) - 1;
            lobyte = fgetc(fp);
            {
                switch(lobyte)
                {
                    case 0xd8:
                        for(i = 0; i < tabs; ++i)
                            putchar('\t');
                        soi[soi_index] = tagloc;
                        printf("JPEG_SOI=%#x/%d (e%d,s%d)\n",soi[soi_index],soi[soi_index],eoi_index,soi_index);
                        /* must be a JPEG marker next                 */
                        if(((highbyte = fgetc(fp)) != EOF) && (ftell(fp) <= end_offset))
                        {
                            //printf(" %#02x ",highbyte);
                            if(highbyte != 0xff)
                            {
                                fseek(fp,-1L,SEEK_CUR);
                                continue;
                            }
                            if(((lobyte = fgetc(fp)) != EOF) && (ftell(fp) <= end_offset))
                            {
                                if((lobyte < 0xc0) || (lobyte > 0xfe))
                                    continue;
                                fseek(fp,-2L,SEEK_CUR);
                                if(tagloc == 0)
                                    soi_index = -1;
                                ++soi_index;
                                ++tabs;
                                ++hadsoi;
                                //printf(" %#02x, soi_index => %d\n",lobyte,soi_index);
                            }
                        }
                        continue;
                        break;
                    case 0xd9:
                        if(soi[eoi_index])
                        {
                            eoi[eoi_index] = tagloc;
                            for(i = 1; i < tabs; ++i)
                                putchar('\t');
                            printf("JPEG_EOI=%#x/%d (e%d,s%d)\n",eoi[eoi_index],eoi[eoi_index],eoi_index,soi_index);
                            ++eoi_index;
                            --tabs;
                        }
                        else
                        {
                            for(i = 0; i < tabs; ++i)
                                putchar('\t');
                            printf("JPEG_EOI=%#x/%d\n",tagloc,tagloc);
                        }
                        break;
                    case 0xc4:
                        for(i = 0; i < tabs; ++i)
                            putchar('\t');
                        printf("JPEG_DHT=%#04x/%d\n",tagloc,tagloc);
                        break;
                    case 0xc0:
                    case 0xc1:
                    case 0xc2:
                    case 0xc3:
                    case 0xc5:
                    case 0xc6:
                    case 0xc7:
                    case 0xc9:
                    case 0xca:
                    case 0xcb:
                    case 0xcd:
                    case 0xce:
                    case 0xcf:
                        for(i = 0; i < tabs; ++i)
                            putchar('\t');
                        printf("JPEG_SOF%d=%#04x/%d (e%d,s%d)\n",lobyte & 0xf,tagloc,tagloc,eoi_index,soi_index);
                        break;
                    case 0xda:
                        for(i = 0; i < tabs; ++i)
                            putchar('\t');
                        printf("JPEG_SOS=%#04x/%d (e%d,s%d)\n",tagloc,tagloc,eoi_index,soi_index);
                        break;
                    case 0xdb:
                        for(i = 0; i < tabs; ++i)
                            putchar('\t');
                        printf("JPEG_DQT=%#04x/%d (e%d,s%d)\n",tagloc,tagloc,eoi_index,soi_index);
                        break;
                    case 0xe1:
                    case 0xe0:
                    case 0xe2:
                    case 0xe3:
                    case 0xe4:
                    case 0xe5:
                    case 0xe6:
                    case 0xe7:
                    case 0xe8:
                    case 0xe9:
                    case 0xea:
                    case 0xeb:
                    case 0xec:
                    case 0xed:
                    case 0xee:
                    case 0xef:
                        for(i = 0; i < tabs; ++i)
                            putchar('\t');
                        printf("JPEG_APP%d =%#04x/%d (e%d,s%d)\n",lobyte & 0xf,tagloc,tagloc,eoi_index,soi_index);
                        break;
                    default:
                        /*
                        if((lobyte >= 0xc0) && (lobyte < 0xff))
                            printf("JPEG_%#04x\n",lobyte);
                        */
                        break;
                }
            }
        }
        i = soi_index - eoi_index;
        for(j = eoi_index - 1; i < soi_index; ++i,--j)
        {
            if(soi[i] == -1)
                soi[i] = 0;
            if((i >= 0) && (j>= 0))
                printf("%d:%d @%d:%d => %d\n",i,j,soi[i],(eoi[i] + 2) - soi[i],eoi[i] + 1);
        }
    }
    exit(0);
}
