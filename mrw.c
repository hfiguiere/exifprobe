/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: mrw.c,v 1.11 2005/07/24 20:27:36 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Minolta MRW routines                                               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* The information coded here is derived from information reported    */
/* by Dalibor Jelinek at:                                             */
/*      http://www.dalibor.cz/minolta/raw_file_format.htm             */
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
#include "mrw.h"


/* Print the (rather simple) MRW header information.                  */

int
print_mrw_header(struct fileheader *fileheader,unsigned long section_id)
{
    struct mrw_header mrwheader;
    int status = -1;
    int chpr = 0;

    if(Print_options & section_id)
    {
        if(fileheader && (fileheader->probe_magic == PROBE_MRWMAGIC))
        {
            mrwheader = fileheader->mrw_header;
            if(mrwheader.mrw_magic == PROBE_MRWMAGIC)
            {
                chpr += printf("MRW magic %#lx, length %u, offset to image data %lu",
                                            mrwheader.mrw_magic,8,mrwheader.mrw_dataoffset + 8);
                chpr = newline(chpr);
                status = 0;
            }
            else 
                chpr += printf(" MRW HEADER MAGIC BAD");
        }
        else
            chpr += printf(" NOT AN MRW HEADER");
    }
    chpr = newline(chpr);
    return(status);
}

/* Process a Minolta (now Konica Minolta) MRW file, starting right    */
/* after the MRM header.                                              */

unsigned long
process_mrw(FILE *inptr,unsigned long offset,unsigned long data_offset,
            unsigned long data_length,struct image_summary *summary_entry,
            char *parent_name,int indent)
{
    char *blockname;
    char *fulldirname = CNULL;
    struct fileheader *header;
    unsigned long blocklength,blockid;
    unsigned long ifd_offset;
    unsigned long max_offset = 0UL;
    int chpr = 0;
    int status = 0;

    if(inptr && data_offset)
    {
        if((summary_entry == NULL) || summary_entry->entry_lock)
            summary_entry = new_summary_entry(summary_entry,0,IMGFMT_MRW);
        if(summary_entry)
        {
            /* This isn't strictly necessary, because we're going to  */
            /* do it again at the end, but allows the TIFF processor  */
            /* to avoid changing this stuff if it notices they're     */
            /* already set.                                           */
            if(summary_entry->length <= 0)
                summary_entry->length = data_length;
            if(summary_entry->offset <= 0)
                summary_entry->offset = data_offset;
            summary_entry->imageformat = IMGFMT_MRW;
            summary_entry->imagesubformat = IMGSUBFMT_CFA;
            /* Don't lock it here; the TIFF processor should use it   */
            /* (and lock it)                                          */
        }
        chpr = newline(chpr);
        while(offset < data_offset)
        {
            blockid = read_ulong(inptr,TIFF_MOTOROLA,offset);
            blockname = read_string(inptr,offset+1,3);
            blocklength = read_ulong(inptr,TIFF_MOTOROLA,HERE);
            print_tag_address(SECTION,offset,indent,"@");
            if(blockid != MRM_PAD)
                offset += 8;
            if((PRINT_SECTION))
            {
                chpr += printf("{%s} (%#lx) length %lu starting at offset %#lx/%lu",blockname,
                                                        blockid,blocklength,offset,offset);
                chpr = newline(chpr);
            }

            if(Debug)
            {
                chpr = newline(chpr);
                hexdump(inptr,offset,48,48,16,0,0);
                chpr = newline(1);
            }
            switch(blockid)
            {
                case MRM_PRD:
                    max_offset = process_prd(inptr,offset,blocklength,summary_entry,indent);
                    offset += blocklength;
                    break;
                case MRM_TTW:
                    header = read_imageheader(inptr,offset);
                    if(header && (header->probe_magic == PROBE_TIFFMAGIC))
                    {
                        ifd_offset = read_ulong(inptr,header->file_marker,HERE); 
                        if(PRINT_SECTION)
                        {
                            print_tag_address(SECTION,offset,indent + 4,"@");
                            status = print_header(header,SECTION);
                            chpr += printf(" ifd offset = %#lx/%lu)",ifd_offset,ifd_offset);
                            if(offset)
                            {
                                chpr += printf(" (+ %lu = %#lx/%lu)",offset,offset + ifd_offset,
                                                                            offset + ifd_offset);
                            }
                            chpr = newline(chpr);
                        }
                        fulldirname = splice(parent_name,".",blockname);
                        max_offset = process_tiff_ifd(inptr,header->file_marker,
                                            ifd_offset,offset,offset + blocklength,summary_entry,
                                            fulldirname,TIFF_IFD,0,-1,indent + 4);
                        if(max_offset > 0L)
                            status = 0;
                        max_offset = offset + blocklength;
                        fflush(stdout);
                        if(fulldirname)
                            free(fulldirname);
                        fulldirname = CNULL;
                    }
                    else
                        chpr += printf(" NOT A TIFF HEADER");
                    offset += blocklength;
                    break;
                case MRM_WBG:
                    max_offset = process_wbg(inptr,offset,blocklength,summary_entry,indent);
                    offset += blocklength;
                    break;
                case MRM_RIF:
                    max_offset = process_rif(inptr,offset,blocklength,summary_entry,indent);
                    offset += blocklength;
                    break;
                case MRM_PAD:
                    offset += blocklength;
                    max_offset = offset;
                    break;
                default:
                    offset += blocklength;
                    max_offset = offset - 8;
                    break;
            }
            chpr = newline(0);
            if((PRINT_SECTION))
            {
                print_tag_address(SECTION,max_offset - 1,indent,"@");
                chpr += printf("{/%s}",blockname ? blockname : "UNK");
            }
            chpr = newline(chpr);
        }
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,data_offset,indent,"@");
            chpr += printf("MRW (CFA) data length %lu",data_length);
            chpr = newline(chpr);
            dumpsection(inptr,data_offset,data_length,indent + SMALLINDENT);
            print_tag_address(SECTION,data_offset + data_length - 1,indent,"@");
            chpr += printf("End of MRW data");
        }
        chpr = newline(chpr);
        /* Fix up any damage done by the TIFF IFD...                  */
        /* This information supercedes anything the TIFF processor    */
        /* may have done...image processing software can trash the    */
        /* TIFF structure (and some do), but this is basic            */
        /* information from the MRW header.                           */
        if(summary_entry)
        {
            summary_entry->length = data_length;
            summary_entry->offset = data_offset;
            summary_entry->imageformat = IMGFMT_MRW;
            summary_entry->imagesubformat = IMGSUBFMT_CFA;
            /* The TIFF processor should have locked it, but make     */
            /* sure                                                   */
            summary_entry->entry_lock = lock_number(summary_entry);
        }
    }
    return(data_offset + data_length);
}

#define SHORTNAMEWIDTH  20
#define LONGNAMEWIDTH   24

unsigned long
process_prd(FILE *inptr,unsigned long offset,unsigned long blocklength,
            struct image_summary *summary_entry,int indent)
{
    unsigned short ccd_height,ccd_width;
    unsigned short image_height,image_width;
    unsigned short datasize,pixelsize,storage_method;
    unsigned short unknown1,unknown2,unknown3;
    char *version;
    int chpr = 0;

    blocklength += offset;


    chpr = newline(chpr);
    version = (char *)read_bytes(inptr,8UL,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Version");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %8.8s",version);
        chpr = newline(chpr);
    }
    offset += 8;
    ccd_height = read_ushort(inptr,TIFF_MOTOROLA,offset);
    ccd_width = read_ushort(inptr,TIFF_MOTOROLA,HERE);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((LIST_MODE))
        {
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.PRD.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ccdWidth");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %u",ccd_width);
            chpr = newline(chpr);
            print_tag_address(ENTRY,offset + 4,indent + 4,"@");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.PRD.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ccdHeight");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %u",ccd_height);
        }
        else
        {
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.PRD.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"CCDSize");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %ux%u",ccd_width,ccd_height);
        }
        chpr = newline(chpr);
    }
    offset += 4;
    image_height = read_ushort(inptr,TIFF_MOTOROLA,offset);
    image_width = read_ushort(inptr,TIFF_MOTOROLA,HERE);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((LIST_MODE))
        {
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.PRD.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ImageWidth");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %u",image_width);
            chpr = newline(chpr);
            print_tag_address(ENTRY,offset + 4,indent + 4,"@");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.PRD.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ImageHeight");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %u",image_height);
        }
        else
        {
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.PRD.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ImageSize");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %ux%u",image_width,image_height);
        }
        chpr = newline(chpr);
        /* This is also in the TIFF IFD0                              */
        if(summary_entry)
        {
            summary_entry->pixel_width = image_width;
            summary_entry->pixel_height = image_height;
        }
    }
    offset += 4;
    datasize = read_ubyte(inptr,offset);
    if(summary_entry)
    {
        summary_entry->sample_size = datasize;
        summary_entry->spp = 1;
    }
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"DataSize");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",datasize);
        chpr = newline(chpr);
    }
    ++offset;
    pixelsize = read_ubyte(inptr,offset);
    if(summary_entry)
        summary_entry->bps[0] = (int)pixelsize;
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"PixelSize");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",pixelsize);
        chpr = newline(chpr);
    }
    ++offset;
    storage_method = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"StorageMethod");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %#x/%u",storage_method,storage_method);
        chpr = newline(chpr);
    }
    ++offset;
    unknown1 = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"unknown1");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",unknown1);
        chpr = newline(chpr);
    }
    ++offset;
    unknown2 = read_ushort(inptr,TIFF_MOTOROLA,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"unknown2");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",unknown1);
        chpr = newline(chpr);
    }
    offset += 2;
    unknown3 = read_ushort(inptr,TIFF_MOTOROLA,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.PRD.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"unknown3");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",unknown1);
        chpr = newline(chpr);
    }
    offset += 2;
    return(offset);
}

/* The White Balance blob. Weird stuff.                               */

unsigned long
process_wbg(FILE *inptr,unsigned long offset,unsigned long blocklength,
            struct image_summary *summary_entry,int indent)
{
    unsigned short wbr_denom,wbg1_denom,wbg2_denom,wbb_denom;
    unsigned short wbr_num,wbg1_num,wbg2_num,wbb_num;
    double wbr,wbg1,wbg2,wbb;
    int chpr = 0;

    blocklength += offset;

    chpr = newline(chpr);
    wbr_denom = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbrDenominator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbr_denom);
        switch(wbr_denom)
        {
            case 0: wbr_denom = 64; break;
            case 1: wbr_denom = 128; break;
            case 2: wbr_denom = 256; break;
            case 3: wbr_denom = 512; break;
            case 4: wbr_denom = 1024; break;
            default: break;
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbr_denom);

        chpr = newline(chpr);
    }
    offset++;
    wbg1_denom = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Wbg1Denominator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbg1_denom);
        switch(wbg1_denom)
        {
            case 0: wbg1_denom = 64; break;
            case 1: wbg1_denom = 128; break;
            case 2: wbg1_denom = 256; break;
            case 3: wbg1_denom = 512; break;
            case 4: wbg1_denom = 1024; break;
            default: break;
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbg1_denom);

        chpr = newline(chpr);
    }
    offset++;
    wbg2_denom = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Wbg2Denominator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbg2_denom);
        switch(wbg2_denom)
        {
            case 0: wbg2_denom = 64; break;
            case 1: wbg2_denom = 128; break;
            case 2: wbg2_denom = 256; break;
            case 3: wbg2_denom = 512; break;
            case 4: wbg2_denom = 1024; break;
            default: break;
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbg2_denom);

        chpr = newline(chpr);
    }
    offset++;
    wbb_denom = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbbDenominator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbb_denom);
        switch(wbb_denom)
        {
            case 0: wbb_denom = 64; break;
            case 1: wbb_denom = 128; break;
            case 2: wbb_denom = 256; break;
            case 3: wbb_denom = 512; break;
            case 4: wbb_denom = 1024; break;
            default: break;
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbb_denom);

        chpr = newline(chpr);
    }
    offset++;
    wbr_num = read_ushort(inptr,TIFF_MOTOROLA,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbrNumerator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbr_num);

        if((PRINT_SECTION))
        {
            if(wbr_denom)
            {
                wbr = (double)wbr_num / (double)wbr_denom;
                chpr += printf(" => wbr = %.3f",wbr);
            }
            else
                chpr += printf(" => wbr = inf");
        }
        else
        {
            chpr = newline(chpr);
            print_tag_address(ENTRY,HERE,indent + 4,"*");
            if((PRINT_TAGINFO))
            {
                /* pseudo-tag for result whitebalance             */
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.WBG.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbRed");
            }
            if((PRINT_VALUE))
            {
                if(wbr_denom)
                {
                    wbr = (double)wbr_num / (double)wbr_denom;
                    chpr += printf(" = %.3f",wbr);
                }
                else
                    chpr += printf(" = inf");
            }
        }

        chpr = newline(chpr);
    }
    offset += 2;
    wbg1_num = read_ushort(inptr,TIFF_MOTOROLA,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Wbg1Numerator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbg1_num);
        if((PRINT_SECTION))
        {
            if(wbg1_denom)
            {
                wbg1 = (double)wbg1_num / (double)wbg1_denom;
                chpr += printf(" => wbg1 = %.3f",wbg1);
            }
            else
                chpr += printf(" => wbg1 = inf");
        }
        else
        {
            chpr = newline(chpr);
            print_tag_address(ENTRY,HERE,indent + 4,"*");
            if((PRINT_TAGINFO))
            {
                /* pseudo-tag for result whitebalance             */
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.WBG.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbGreen1");
            }
            if((PRINT_VALUE))
            {
                if(wbg1_denom)
                {
                    wbg1 = (double)wbg1_num / (double)wbg1_denom;
                    chpr += printf(" = %.3f",wbg1);
                }
                else
                    chpr += printf(" => wbg1 = inf");
            }
        }
        chpr = newline(chpr);
    }
    offset += 2;
    wbg2_num = read_ushort(inptr,TIFF_MOTOROLA,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Wbg2Numerator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbg2_num);
        if((PRINT_SECTION))
        {
            if(wbg2_denom)
            {
                wbg2 = (double)wbg2_num / (double)wbg2_denom;
                chpr += printf(" => wbg2 = %.3f",wbg2);
            }
            else
                chpr += printf(" => wbg2 = inf");
        }
        else
        {
            chpr = newline(chpr);
            print_tag_address(ENTRY,HERE,indent + 4,"*");
            if((PRINT_TAGINFO))
            {
                /* pseudo-tag for result whitebalance             */
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.WBG.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbGreen2");
            }
            if((PRINT_VALUE))
            {
                if(wbg2_denom)
                {
                    wbg2 = (double)wbg2_num / (double)wbg2_denom;
                    chpr += printf(" = %.3f",wbg2);
                }
                else
                    chpr += printf(" => wbg2 = inf");
            }
        }
        chpr = newline(chpr);
    }
    offset += 2;
    wbb_num = read_ushort(inptr,TIFF_MOTOROLA,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.WBG.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbbNumerator");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",wbb_num);
        if((PRINT_SECTION))
        {
            if(wbb_denom)
            {
                wbb = (double)wbb_num / (double)wbb_denom;
                chpr += printf(" => wbb = %.3f",wbb);
            }
            else
                chpr += printf(" => wbb = inf");
        }
        else
        {
            chpr = newline(chpr);
            print_tag_address(ENTRY,HERE,indent + 4,"*");
            if((PRINT_TAGINFO))
            {
                /* pseudo-tag for result whitebalance             */
                if((PRINT_LONGNAMES))
                    chpr += printf("%s","MRW.WBG.");
                chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WbBlue");
            }
            if((PRINT_VALUE))
            {
                if(wbb_denom)
                {
                    wbb = (double)wbb_num / (double)wbb_denom;
                    chpr += printf(" = %.3f",wbb);
                }
                else
                    chpr += printf(" => wbb = inf");
            }
        }
        chpr = newline(chpr);
    }
    offset += 2;
    return(offset);
}

/* Requested Image Format                                             */

unsigned long
process_rif(FILE *inptr,unsigned long offset,unsigned long blocklength,
            struct image_summary *summary_entry,int indent)
{
    unsigned long end_offset;
    unsigned short saturation,contrast;
    unsigned short sharpness,white_balance;
    unsigned short subject_program,iso;
    unsigned short color_mode,color_filter,bw_filter;
    unsigned short unknown;
    double fvalue;
    int chpr = 0;

    end_offset = offset + blocklength;

    chpr = newline(chpr);
    unknown = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"unknown");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",unknown);
        chpr = newline(chpr);
    }
    ++offset;
    saturation = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Saturation");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",saturation);
        chpr = newline(chpr);
    }
    ++offset;
    contrast = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Contrast");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",contrast);
        chpr = newline(chpr);
    }
    ++offset;
    sharpness = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"Sharpness");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",sharpness);
        chpr = newline(chpr);
    }
    ++offset;
    white_balance = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"WhiteBalance");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u/%#x",white_balance,white_balance);
        chpr = newline(chpr);
    }
    ++offset;
    subject_program = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"SubjectProgram");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u",subject_program);
        chpr = newline(chpr);
    }
    ++offset;
    iso = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"CCDSensitivity");
        }
        if((PRINT_VALUE))
        {
            if(PRINT_RAW_VALUES)
                chpr += printf(" = %u",iso);
            fvalue = ((double)iso/8.0) - 1.0;
            chpr += printf(" = %.3g APEX",fvalue);
            fvalue = pow(2.0,fvalue) * 3.125;
            chpr += printf(" = %.3g ISO",fvalue);
        }
        chpr = newline(chpr);
    }
    ++offset;
    color_mode = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ColorMode");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u/%#x",color_mode,color_mode);
        chpr = newline(chpr);
    }
    ++offset;
    color_filter = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"ColorFilter");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u/%#x",color_filter,color_filter);
        chpr = newline(chpr);
    }
    ++offset;
    bw_filter = read_ubyte(inptr,offset);
    if((PRINT_ENTRY))
    {
        print_tag_address(ENTRY,offset,indent + 4,"@");
        if((PRINT_TAGINFO))
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s","MRW.RIF.");
            chpr += printf("%-*.*s",SHORTNAMEWIDTH,SHORTNAMEWIDTH,"BWFilter");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %u/%#x",bw_filter,bw_filter);
        chpr = newline(chpr);
    }
    ++offset;
    /* Remaining values are unknown                                   */
    return(end_offset);
}
