/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: main.c,v 1.30 2005/07/24 15:58:05 alex Exp $";
#endif

/* This program reads and reports the structure of and auxilliary     */
/* data contained within images produced by digital cameras or        */
/* similar devices. The program understands several file formats,     */
/* including TIFF, JPEG, MRW, CIFF, JP2, RAF, and X3F, and will read  */
/* most TIFF-derived formats such as EXIF, ORF, CR2, NEF, K25, KDC,   */
/* DNG, and PEF. The RAF code is "preliminary".                       */

/* MakerNote sections found in Exif IFDs are reported and expanded if */
/* the note is in a semblance of TIFF IFD format (determined          */
/* automatically, without reference to device make or model).         */
/* Information which has been published on the internet is used to    */
/* interpret notes where possible.                                    */

/* The default output attempts to describe the structure of the file, */
/* showing location, size and content of auxilliary data and image    */
/* sections. Multiple images contained in a single file are noted     */
/* (and JPEG sub-image markers examined for useful data). A summary   */
/* of images found is printed at the end of output for each file,     */
/* including a report of "file format" composed of the major sections */
/* found, and the size of the file.                                   */

/* Many options are provided for detail control of output format. For */
/* convenience, two pre-set formats (in addition to the default       */
/* 'structural' format) provide a "report" format (which shows        */
/* "structure" of TIFF ifds, but with values printed inline), and a   */
/* "list" format which eliminates structural elements and presents    */
/* data in a human (or script) readable format for possible use in    */
/* gallery applications.                                              */


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "defs.h"
#include "summary.h"
#include "global.h"
#include "datadefs.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "ciff_extern.h"
#include "x3f_extern.h"
#include "jp2_extern.h"
#include "mrw_extern.h"
#include "maker_extern.h"

int
main(int argc,char **argv)
{
    char *file;
    char *name = CNULL;
    FILE *inptr;
    int status = -1;
    unsigned long max_offset = 0UL;
    unsigned long ifd_offset = 0UL;
    unsigned long dumplength = 0UL;
    struct fileheader *header = NULL;
    struct image_summary *summary_entry = NULL;
    unsigned long filesize = 0UL;
    int chpr = 0;


    Progname = *argv;

    /* Gather options from EXIFPROBE_OPTIONS first                    */
    env_options();

    /* Now the command line                                           */
    argv += process_options(argc,argv);

    while((file = *argv++))
    {
        inptr = fopen(file,"rb");
        if(inptr == FNULL)
        {
            fprintf(stderr,"%s: failed to open file %s\n",Progname,file);
            why(stderr);
            max_offset = 0L;
        }
        else
        {
            PUSHCOLOR(BLACK);
            setfilename(file);  /* for '-n' option                    */
            filesize = get_filesize(inptr);

            if(Start_offset > filesize)
            {
                PUSHCOLOR(RED);
                print_filename();
                chpr += printf(" start offset %lu > filesize %lu\n",
                                            Start_offset,filesize);
                POPCOLOR();
                continue;
            }
            else if(Start_offset)
            {
                if(fseek(inptr,Start_offset,SEEK_SET) == -1)
                {
                    fprintf(stderr,"%s: FAILED to seek to start offset %lu\n",
                                                        Progname,Start_offset);
                    why(stdout);
                    continue;
                }
                printf("# Start at %ld\n",ftell(inptr));
            }

            print_filename();   /* for '-n' option                    */
            if((LIST_MODE))
                chpr += printf("FileName = %s",file);
            else
                chpr += printf("File Name = %s",file);
            chpr = newline(chpr);

            header = read_imageheader(inptr,Start_offset);
            if(ferror(inptr))
                continue;
            print_filename();
            if((LIST_MODE))
                chpr += printf("FileType = ");
            else
                chpr += printf("File Type = ");
            status = print_filetype(header->probe_magic,header->file_marker);
            chpr = newline(chpr);

            print_filename();
            if((LIST_MODE))
                chpr += printf("FileSize = %lu",filesize);
            else
                chpr += printf("File Size = %lu",filesize);
            chpr = newline(chpr);

            if(status == 0)
            {
                switch(header->probe_magic)
                {
                    case ORF1_MAGIC:
                    case ORF2_MAGIC:
                    case RW2_MAGIC:
                    case TIFF_MAGIC:
                        ifd_offset = read_ulong(inptr,header->file_marker,HERE); 
                        if(ifd_offset > filesize)
                        {
                            hexdump(inptr,Start_offset,10,10,12,0,0);
                            chpr = newline(chpr);
                            if(Max_undefined > 0L)
                                dumplength = Max_undefined - 10;
                            else
                                dumplength = 60;
                            if(dumplength)
                            {
                                hexdump(inptr,Start_offset +  10,
                                            filesize - 10 - Start_offset,
                                                            dumplength,12,0,0);
                                chpr = newline(1);
                            }
                            status = -1;
                            break;
                        }
                        summary_entry = new_summary_entry(NULL,header->probe_magic,TIFF_IFD);
                        name = "TIFF";
                        switch(header->probe_magic)
                        {
                            case ORF1_MAGIC:
                                name = "ORF1";
                                break;
                            case ORF2_MAGIC:
                                name = "ORF2";
                                break;
                            case RW2_MAGIC:
                                name = "RW2";
                                break;
                            case TIFF_MAGIC:
                            default:
                                break;
                        }

                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,Start_offset,0,"@");
                            status = print_header(header,SECTION);
                            chpr += printf(" ifd offset = %#lx/%lu",ifd_offset,ifd_offset);
                            if(Start_offset)
                            {
                                chpr += printf(" (+ %lu = %#lx/%lu)",Start_offset,
                                                                Start_offset + ifd_offset,
                                                                Start_offset + ifd_offset);
                            }
                            chpr = newline(chpr);
                        }
                        max_offset = process_tiff_ifd(inptr,header->file_marker,
                                            ifd_offset,Start_offset,0L,summary_entry,name,
                                            TIFF_IFD,0,-1,0);
                        if(max_offset > 0L)
                            status = 0;
                        fflush(stdout);
                        break;
                    case JPEG_SOI:
                        summary_entry = new_summary_entry(NULL,FILEFMT_JPEG,IMGFMT_JPEG);
                        if(summary_entry)
                            summary_entry->subfiletype = PRIMARY_TYPE;
                        max_offset = process_jpeg_segments(inptr,Start_offset,JPEG_SOI,0L,summary_entry,
                                                                            "JPEG","@",0);
                        status = jpeg_status(0);
                        fflush(stdout);
                        break;
                    case PROBE_CIFFMAGIC:
                        summary_entry = new_summary_entry(NULL,FILEFMT_CIFF,IMGFMT_CRW);
                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,Start_offset,0,"@");
                            status = print_header(header,SECTION);
                        }
                        max_offset = process_ciff(inptr,header,Start_offset,0L,summary_entry,"CIFF",0,0);
                        if(max_offset > 0L)
                            status = 0;
                        fflush(stdout);
                        break;
                    case PROBE_JP2MAGIC:
                        summary_entry = new_summary_entry(NULL,FILEFMT_JP2,IMGFMT_JPEG2000);
                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,Start_offset,0,"@");
                            status = print_header(header,SECTION);
                        }
                        max_offset = process_jp2(inptr,Start_offset + 12L,summary_entry,"JP2",0L);
                        if(max_offset > 0L)
                            status = 0;
                        fflush(stdout);
                        break;
                    case PROBE_MRWMAGIC:
                        summary_entry = new_summary_entry(NULL,FILEFMT_MRW,IMGFMT_MRW);
                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,Start_offset,0,"@");
                            status = print_header(header,SECTION);
                        }
                        max_offset =
                            process_mrw(inptr,Start_offset + 8L,
                                    header->mrw_header.mrw_dataoffset,
                                    filesize - header->mrw_header.mrw_dataoffset,
                                    summary_entry,"MRW",0L);
                        if(max_offset > 0L)
                            status = 0;
                        fflush(stdout);
                        break;
                    case PROBE_RAFMAGIC:
                        summary_entry = new_summary_entry(NULL,FILEFMT_RAF,IMGFMT_RAF);
                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,Start_offset,0,"@");
                            status = print_raf_header(inptr,header,SECTION);
                        }
                        if(status == 0)
                        {
                            max_offset = process_raf(inptr,Start_offset + 60,summary_entry,"RAF",0L);
                            if(max_offset > 0L)
                                status = 0;
                        }
                        fflush(stdout);
                        break;
                    case PROBE_X3FMAGIC:
                        summary_entry = new_summary_entry(NULL,FILEFMT_X3F,IMGFMT_X3F);
                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,Start_offset,0,"@");
                            status = print_x3f_header(inptr,header,SECTION);
                        }
                        if(status == 0)
                        {
                            max_offset = process_x3f(inptr,header,Start_offset,summary_entry,
                                                                                "X3F",0,0);
                            if(max_offset > 0L)
                                status = 0;
                        }
                        break;
                    default:
                        status = -1;
                        break;
                }
                if(status == JPEG_EARLY_EOI)
                    chpr = newline(chpr);
                print_tag_address(SECTION,filesize - 1,0,"-");
                if(PRINT_SECTION)
                {
                    chpr += printf("END OF FILE");
                    if(status == JPEG_EARLY_EOI)
                    {
                        jpeg_status(status);
                        print_jpeg_status();
                    }
                }
                chpr = newline(chpr);
                /* Print a summary of image sections found            */
                print_summary(summary_entry);
            }
            else
            {
                /* Allow hex/ascii dump of unregcognized file         */
                /* formats. In fact, always dump a little bit.        */
                if(Max_undefined == DUMPALL)
                    dumplength = filesize;
                else if(Max_undefined > 0L)
                    dumplength = Max_undefined;
                else
                    dumplength = DEF_DUMPLENGTH;
                if(dumplength)
                {
                    hexdump(inptr,Start_offset,filesize,dumplength,16,0,0);
                    chpr = newline(chpr);
                }
            }
            print_fileformat(summary_entry);
            summary_entry = destroy_summary(summary_entry);
            chpr = newline(1);
            if(inptr && fclose(inptr))
            {
                fprintf(stderr,"%s: FAILED to close stream for file \"%s\"\n",Progname,file);
                why(stderr);
            }
        }
        clear_memory();     /* Make, Model, Software...               */
        chpr = newline(1);
    }
    if(PRINT_CAMERA_NAMES)
        print_camera_makes();
    return(status);
}
