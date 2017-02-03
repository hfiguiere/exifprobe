/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_nikon.c,v 1.30 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Nikon camera maker-specific routines.                              */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Much of the information coded here is due to TsuruZoh Tachibanaya  */
/* at:                                                                */
/* http://www.ba.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html   */

/* and Max Lyons (E990 data) at:                                      */
/*   http://www.tawbaware.com/                                        */

/* Some information is taken from Eugene Crosser's "photopc" program: */
/*   http://photopc.sourceforge.net/                                  */
/* (as modified by John Bowman: http://www.math.ualberta.ca/imaging/) */

/* The presence of JPEG thumbnails in TIFF format makernotes,         */
/* extension of (some) E990 tags to the E995 and later models, the    */
/* extra TIFF header in D100s, and the PrintIM data in some models,   */
/* is due to examination (by earlier versions of this program) of     */
/* images from my Nikon 990, and sample images obtained               */
/* from:                                                              */
/*   http://www.imaging-resource.com/                                 */
/* and                                                                */
/*   http://www.dpreview.com/                                         */

/* April, 2005:                                                       */
/* Much of the new information for later model Nikons is taken from   */
/* information provided by a Phil Harvey (author of 'exiftool') at:   */
/*   http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/index.html */
/* and/or  Evan Hunter ("The PHP JPEG Metadata Toolkit"), at:         */
/*   http://www.ozhiker.com/electronics/pjmt/jpeg_info/nikon_mn.html  */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

extern struct camera_id nikon_model_id[];

/* Find the identifying number assigned to known Nikon camera models. */
/* This number is used to dispatch print and interpret routines       */
/* apporopriate to the current image. At the moment, the "software"   */
/* string is not used.                                                */

int
nikon_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &nikon_model_id[0]; model_id && model_id->name; ++model_id)
    {
        if(strncasecmp(model,model_id->name,model_id->namelen) == 0)
        {
            number = model_id->id;
            setnoteversion(model_id->noteversion);
            setnotetagset(model_id->notetagset);    /* info only      */
            break;
        }
    }
    return(number);
}

/* Determine and set note version from detected scheme.               */
/* Notes with ID scheme use tagset 1, offset routine 1, interp 1      */
/* Notes with Plain and ID/TIFFhdr use tagset 2, offset 2, interp 2   */

/* first tag is also distinctive (v2 has 0x1 (UNDEFINED count 4,      */
/* ascii version), v1 has 0x2 (ASCII count 6, "08.00") although TIFF  */
/* files will have a thumbnail (0x100) which displaces the first tag  */
/* (placed out of numerical sort order).                              */
/* For now, it does not seem necessary to check first tag.            */

int
set_nikon_noteversion()
{
    struct maker_scheme *scheme;
    int noteversion = 0;
    int tagset = 0;

    scheme = retrieve_makerscheme();
    if(scheme->note_version <= 0)
    {
        noteversion = -1;
        tagset = -1;
        switch(scheme->scheme_type)
        {
            case HAS_ID_SCHEME:
                tagset = 1;
                noteversion = 1;
                break;
            case PLAIN_IFD_SCHEME:
            case HAS_ID_PLUS_TIFFHDR_SCHEME:
                tagset = 2;
                noteversion = 2;
                break;
            case UNKNOWN_SCHEME:
                break;
            default: 
                break;
        }
        setnotetagset(tagset);
        setnoteversion(noteversion);
    }
    else
        noteversion = scheme->note_version;
    return(noteversion);
}

void
print_nikon_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1:
                print_value(entry_ptr,PREFIX);
                nikon1_interpret_value(entry_ptr);
                break;
            case 2:
                print_value(entry_ptr,PREFIX);
                nikon2_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

void
print_nikon_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    int noteversion = 0;
    char *nameoftag;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 1:
                nikon1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 2:
                nikon2_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                nameoftag = maker_tagname(entry_ptr->tag,make,model); 
                nikon2_interpret_offset_makervalue(inptr,byteorder,entry_ptr,
                                                fileoffset_base,parent_name,
                                                nameoftag,indent);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,parent_name,prefix,
                                        indent,make,model,at_offset);
                break;
        }
    }
}

void
nikon1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long max_offset = 0;
    unsigned long value_offset;
    unsigned long count;
    unsigned long dumplength;
    unsigned short marker;
    int status = 0;
    int chpr = 0;
    char *nameoftag;
    char *fulldirname = NULL;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;

        switch(entry_ptr->tag)
        {
            case 0x0100: /* Jpeg Thumbnail                            */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %lu",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%-9lu",value_offset,count);
                    else
                        chpr += printf(":%-9lu", count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf(" # UNDEFINED");
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                    count,summary_entry,fulldirname,
                                    prefix,indent+SMALLINDENT);
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(VALUE_AT_OFFSET,value_offset + count - 1,
                                                                        indent,"-");
                        chpr += printf("# End of JPEG Thumbnail from MakerNote");
                        chpr += printf(" length %ld",count); 
                        if((PRINT_ENTRY) && !(PRINT_VALUE))
                            chpr = newline(chpr);
                    }
                }
                print_jpeg_status();
                if(marker && summary_entry)
                { 
                    /* The new one is on the end of the chain         */
                    if((summary_entry = last_summary_entry(summary_entry)))
                    {
                        summary_entry->filesubformat |= FILESUBFMT_MNOTE;
                        summary_entry->datatype = MAKER_IFD;
                        summary_entry->subfiletype = THUMBNAIL_TYPE;
                    }
                }
                /* make certain we're at the end                      */
                clearerr(inptr);
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            case 0x0f00: /* Data                                      */
                if(at_offset && (PRINT_ENTRY))
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,25," : ",make,model);
                    chpr += printf("length %-9lu # UNDEFINED", count);
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    else
                        chpr += printf(":%lu", count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf(" # UNDEFINED");
                }
                if(Max_undefined == 0)
                {
                    if(chpr)
                        printred(" (not dumped, use -U)");
                }
                else
                {
                    if((Max_undefined == DUMPALL) || (Max_undefined > count))
                        dumplength = count;
                    else 
                        dumplength = Max_undefined;
                    chpr = newline(chpr);
                    hexdump(inptr,value_offset,count,dumplength,12,
                                        indent,SUBINDENT);
                    chpr = newline(1);
                }
                /* make certain we're at the end                      */
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

void
nikon2_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long max_offset = 0;
    unsigned long value_offset;
    unsigned long count;
    unsigned long dumplength;
    unsigned long orig_print_options;
    unsigned short marker;
    char *nameoftag;
    char *fulldirname = NULL;
    int status = 0;
    int chpr = 0;
    int ifdnum = -1;
    int subifdnum = -1;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;

        switch(entry_ptr->tag)
        {
            case 0x0011: /* "NikonPreviewImage"                       */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of Nikon Preview Image SubIFD from MakerNote");
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                chpr = newline(chpr);
                if(summary_entry)
                {
                    ifdnum = summary_entry->ifdnum;
                    subifdnum = summary_entry->subifdnum;
                }
                value_offset = process_tiff_ifd(inptr,byteorder,entry_ptr->value,
                                        fileoffset_base,0,summary_entry,fulldirname,
                                        MAKER_SUBIFD,ifdnum,subifdnum,
                                        indent+SMALLINDENT);
                if(at_offset && (PRINT_SECTION))
                {
                    chpr = newline(chpr);
                    if(value_offset == 0)
                        PUSHCOLOR(RED);
                    print_tag_address(VALUE_AT_OFFSET,value_offset - 1,
                                                indent,"-");
                    if(value_offset == 0)
                        POPCOLOR();
                    chpr += printf("# End of Nikon Preview Image SubIFD from MakerNote");
                    if(value_offset == 0)
                        printred(" (INVALID)");
                }
                if(summary_entry)
                { 
                    /* The new one is on the end of the chain         */
                    if((summary_entry = last_summary_entry(summary_entry)))
                    {
                        summary_entry->filesubformat |= FILESUBFMT_MNSUBIFD;
                        summary_entry->datatype = MAKER_SUBIFD;
                        summary_entry->subfiletype = REDUCED_RES_TYPE;
                    }
                }
                break;
            case 0x0100: /* Jpeg Thumbnail                            */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %lu",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    else
                        chpr += printf(":%lu", count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf(" # UNDEFINED");
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                    count,summary_entry,fulldirname,
                                    prefix,indent+SMALLINDENT);
#if 0
/* an alternate place to look */
                if((status = jpeg_status(status)) == JPEG_NO_SOI)
                {
                    marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset - fileoffset_base);
                    max_offset = process_jpeg_segments(inptr,value_offset - fileoffset_base,
                                            marker,count,summary_entry,
                                            prefix,indent+SMALLINDENT);
                    if(max_offset)
                        value_offset -= fileoffset_base;
                }
#endif
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(VALUE_AT_OFFSET,value_offset + count - 1,
                                                    indent,"-");
                        chpr += printf("# End of JPEG Thumbnail from MakerNote");
                        chpr += printf(" length %lu",count); 
                        if((PRINT_ENTRY) && !(PRINT_VALUE))
                            chpr = newline(chpr);
                    }
                }
                print_jpeg_status();
                if(marker && summary_entry)
                { 
                    /* The new one is on the end of the chain         */
                    if((summary_entry = last_summary_entry(summary_entry)))
                    {
                        summary_entry->filesubformat |= FILESUBFMT_MNOTE;
                        summary_entry->datatype = MAKER_IFD;
                        summary_entry->subfiletype = THUMBNAIL_TYPE;
                    }
                }
                /* make certain we're at the end unless 'inptr' bad   */
                clearerr(inptr);
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            case 0x0e00: /* PrintIM (Epson Print Image matching)      */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf("    # UNDEFINED");
                    chpr = newline(chpr);
                }
                process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                    count,nameoftag,parent_name,prefix,indent);
                break;
            case 0x0010: /* Data                                      */
                if(at_offset && (PRINT_ENTRY))
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,25," : ",make,model);
                    chpr += printf("length %-9lu # UNDEFINED", count);
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    else
                        chpr += printf(":%lu",count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf("    # UNDEFINED");
                }
                if(Max_undefined == 0)
                {
                    if(chpr)
                        printred(" (not dumped, use -U)");
                }
                else
                {
                    if((Max_undefined == DUMPALL) || (Max_undefined > count))
                        dumplength = count;
                    else 
                        dumplength = Max_undefined;
                    chpr = newline(chpr);
                    hexdump(inptr,value_offset,count,dumplength,12,
                                indent,SUBINDENT);
                    chpr = newline(1);
                }
                /* make certain we're at the end                      */
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            case 0x0097:    /* Color Balance ????                     */
                /* Most models show this as an UNDEFINED section; the */
                /* SQ provides it as ASCII, with the value an "array" */
                /* of ascii strings (including newlines). Can these   */
                /* both have the same function?                       */
                if(entry_ptr->value_type == UNDEFINED)
                {
#if 1
                    if(at_offset && (PRINT_ENTRY)) /* PRINT_VALUE?    */
                    {
                        print_tag_address(ENTRY,value_offset,indent,prefix);
                        print_makertagid(entry_ptr,MAKERTAGWIDTH," = ",make,model);
                    }
                    if(at_offset && (PRINT_VALUE) && (PRINT_VALUE_AT_OFFSET))
                        nikon_colorbalance(inptr,entry_ptr,byteorder,fileoffset_base,indent);
                    else if(!at_offset && (PRINT_VALUE))
                    {
                        if(!(PRINT_OFFSET))
                            chpr += printf("@%lu:%-9lu",value_offset,count);
                        else
                            chpr = printf("length %-9lu", entry_ptr->count);
                        if(!(PRINT_VALUE_AT_OFFSET))
                            chpr += printf(" # UNDEFINED");
                    }
#else
                    print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,fulldirname,prefix,
                                        indent,make,model,at_offset);
#endif
                }
                else
                {
                    /* This is an unpleasant hack to show all of the  */
                    /* null-separated strings, even in LIST mode,     */
                    /* where IGNORE_LENGTH is usually on.             */
                    orig_print_options = Print_options;
                    Print_options &=~(PRINT_ASCII_IGNORE_LENGTH);
                    print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,fulldirname,prefix,
                                        indent,make,model,at_offset);
                    Print_options = orig_print_options;
                }
                break;
            case 0x008c:    /* NEFDecodeTable1                        */
            case 0x0096:    /* NEFDecodeTable2                        */
                if((summary_entry) && (summary_entry->fileformat == FILEFMT_TIFF))
                    summary_entry->filesubformat |= FILESUBFMT_NEF;
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                    fileoffset_base,fulldirname,prefix,
                                    indent,make,model,at_offset);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                    fileoffset_base,fulldirname,prefix,
                                    indent,make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_nikon_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;
    int noteversion = 0;

    if((noteversion = getnoteversion()) == 0)
        noteversion = set_nikon_noteversion();

    switch(noteversion)
    {
        case 1:
            tagname = maker_nikon1_tagname(tag,model);
            break;

        case 2:
            tagname = maker_nikon2_tagname(tag,model);
            break;
        default:
            break;
    }
    return(tagname);
}


/* Tags for the older cameras.                                        */

char *
maker_nikon1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0003: tagname = "Quality"; break;
        case 0x0004: tagname = "ColorMode"; break;
        case 0x0005: tagname = "ImageAdjustment"; break;
        case 0x0006: tagname = "ISOSetting"; break;
        case 0x0007: tagname = "WhiteBalance"; break;
        case 0x0008: tagname = "Focus"; break;
        case 0x000a: tagname = "DigitalZoom"; break;
        case 0x000b: tagname = "LensConverter"; break;
        case 0x0100: tagname = "JpegThumbnail"; break;
        case 0x0f00: tagname = "Data"; break;
            default: break;
    }
    setnotetagset(1);
    return(tagname);
}


char *
maker_nikon2_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0001: tagname = "Version"; break;
        case 0x0002: tagname = "ISO"; break;
        case 0x0003: tagname = "ColorMode"; break;
        case 0x0004: tagname = "Quality"; break;
        case 0x0005: tagname = "WhiteBalance"; break;
        case 0x0006: tagname = "ImageSharpening"; break;
        case 0x0007: tagname = "FocusMode"; break;
        case 0x0008: tagname = "FlashSetting"; break;
        case 0x0009: tagname = "FlashType"; break; /* Harvey          */
#if 0
        case 0x000a: break; /* WhiteBal Bias ???                      */
        case 0x000d: break; /* WhiteBal coeff ???                     */
        case 0x000e: break; /* ISOSel? ???                            */
#endif
        case 0x000b: tagname = "WhiteBalanceBias"; break;
        case 0x000c: tagname = "WhiteBalance"; break;
        case 0x000f: tagname = "ISOSelection"; break;
        case 0x0010: tagname = "Data"; break;
        case 0x0011: tagname = "NikonPreviewImage"; break;
        case 0x0012: tagname = "FlashExposureCompensation*"; break;
        case 0x0013: tagname = "ISOSetting"; break;
        case 0x0016: tagname = "ImageBoundary"; break; 
        case 0x0018: tagname = "FlashExposureBracketValue*"; break; 
        case 0x0019: tagname = "ExposureBracketValue"; break; 

        case 0x0080: tagname = "ImageAdjustment"; break; 
        case 0x0081: tagname = "ToneCompensation"; break; 
        case 0x0082: tagname = "LensAdapter"; break;
        case 0x0083: tagname = "LensType"; break; 
        case 0x0084: tagname = "LensInfo"; break; 
        case 0x0085: tagname = "ManualFocusDistance"; break; 
        case 0x0086: tagname = "DigitalZoom"; break;
        case 0x0087: tagname = "FlashMode*"; break; 
        case 0x0088: tagname = "AutoFocusArea*"; break; 
        case 0x0089: tagname = "ShootingMode"; break;
        case 0x008c: tagname = "NEFDecodeTable1"; break;
        case 0x008d: tagname = "ColorMode"; break;
        case 0x008f: tagname = "SceneMode"; break;
        case 0x0090: tagname = "Lightsource"; break;
        case 0x0092: tagname = "HueAdjustment"; break;
        case 0x0094: tagname = "Saturation"; break;
        case 0x0095: tagname = "NoiseReduction"; break;
        case 0x0096: tagname = "NEFDecodeTable2"; break;
        case 0x0097: tagname = "ColorBalance**"; break;
        case 0x0099: tagname = "NEFThumbnailSize*"; break;
        case 0x00a0: tagname = "SerialNumber"; break; 
        case 0x00a7: tagname = "ShutterCount"; break; 
        case 0x00a9: tagname = "ImageOptimization"; break; 
        case 0x00aa: tagname = "Saturation"; break; 
        case 0x00ab: tagname = "VariProgram"; break; 

        case 0x0100: tagname = "JpegThumbnail"; break;
        case 0x0e00: tagname = "PrintIM"; break;
        case 0x0e0e: tagname = "NikonCaptureOffsets"; break;
        default: break;
    }
    setnotetagset(2);
    return(tagname);
}

void
nikon1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    switch(entry_ptr->tag)
    {
        case 0x0003:    /* Quality */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 1:
                    chpr += printf("VGA Basic - 640x480 (1/16)");
                    break;
                case 2:
                    chpr += printf("VGA Normal - 640x480 (1/8)");
                    break;
                case 3:
                    chpr += printf("VGA Fine - 640x480 (1/4)");
                    break;
                case 4:
                    chpr += printf("XGA Basic - 1024x768 (1/16)"); 
                    break;
                case 5:
                    chpr += printf("XGA Normal - 1024x768 (1/8)");
                    break;
                case 6:
                    chpr += printf("XGA Fine - 1024x768 (1/4)"); 
                    break;
                case 7:
                    chpr += printf("SXGA Basic*");
                    break;
                case 8:
                    chpr += printf("SXGA Normal*");
                    break;
                case 9:
                    chpr += printf("SXGA Fine*");
                    break;
                case 10:
                    chpr += printf("Basic - 1600x1200 (1/16)");
                    break;
                case 11:
                    chpr += printf("Normal - 1600x1200 (1/8)");
                    break;
                case 12:
                    chpr += printf("Fine - 1600x1200 (1/4)");
                    break;
                case 20:
                    chpr += printf("Hi - 1600x1200");
                    break;
                default:
                    printred("undefined");
                    break;
            }
            print_endvalue();
            break;
        case 0x0004:    /* Color Mode */
            print_startvalue();
            if(entry_ptr->value == 1)
                chpr += printf("color");
            else if(entry_ptr->value == 2)
                chpr += printf("monochrome");
            else
                printred("undefined");
            print_endvalue();
            break;
        case 0x0005:    /* Image Adjustment */
            print_startvalue();
            /* The data from Crosser's "photopc" conflicts with that  */
            /* from Tachibanaya for contrast and brightness (they're  */
            /* swapped). These are Tachibanaya's values. Somebody please  */
            /* tell me which is right.                                */
            switch(entry_ptr->value)
            {
                case 0:
                    chpr += printf("Standard");
                    break;
                case 1:
                    chpr += printf("Brightness+?"); /* ###%%% Contrast?    */
                    break;
                case 2:
                    chpr += printf("Brightness-?");  /* ###%%% Contrast?   */
                    break;
                case 3:
                    chpr += printf("Contrast+?"); /* ###%%% Brightness?    */
                    break;
                case 4:
                    chpr += printf("Contrast-?"); /* ###%%% Brightness?    */
                    break;
                case 5:
                    /* Is this right?                                 */
                    chpr += printf("Auto?");
                    break;
                default:
                    printred("undefined");
                    break;
            }
            print_endvalue();
            break;
        case 0x0006:    /* CCD Sensitivity (ISO) */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 0:
                    chpr += printf("80");
                    break;
                case 2:
                    chpr += printf("160");
                    break;
                case 4:
                    chpr += printf("320");
                    break;
                case 5:
                    chpr += printf("100");
                    break;
                default:
                    printred("undefined");
                    break;
            }
            print_endvalue();
            break;
        case 0x0007:    /* White Balance */
            print_startvalue();
            /* These values, taken from "photopc", conflict with      */
            /* values given by Tachibanaya                            */
            switch(entry_ptr->value)
            {
                case 0:
                    chpr += printf("Auto");
                    break;
                case 1:
                    chpr += printf("Preset");
                    break;
                case 2:
                    chpr += printf("Daylight");
                    break;
                case 3:
                    chpr += printf("Incandescent");
                    break;
                case 4:
                    chpr += printf("Fluorescent");
                    break;
                case 5:
                    chpr += printf("Cloudy");
                    break;
                case 6:
                    chpr += printf("Flash");
                    break;
                default:
                    printred("undefined");
                    break;
            }
            print_endvalue();
            break;
        case 0x000b:    /* Lens Converter */
            print_startvalue();
            if(entry_ptr->value == 0)
                chpr += printf("none");
            else if(entry_ptr->value == 1)
                chpr += printf("fisheye");
            else
                printred("undefined");
            print_endvalue();
            break;

        default:
            break;
    }
    setcharsprinted(chpr);
}

void 
nikon2_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;
    
    switch(entry_ptr->tag)
    {
        case 0x0002:    /* CCD Sensitivity (ISO) */
            print_startvalue();
            if(entry_ptr->value & 0xffff)
                chpr += printf("%u",(unsigned short)(entry_ptr->value & 0xffff));
            else if((entry_ptr->value >> 16) & 0xffff)
                chpr += printf("%u",(unsigned short)((entry_ptr->value >> 16) & 0xffff));
            else
                chpr += printf("Auto");
            print_endvalue();
            break;
        case 0x000b:
        case 0x000d:
        case 0x000e:
        case 0x0011:
        case 0x008a:
        case 0x008b:
        case 0x0092:
        case 0x0094:
            /*
            print_startvalue();
            printred("unknown");
            print_endvalue();
            */
            break;
        case 0x0087:    /* Flash Used                                 */
            print_startvalue();
            switch(entry_ptr->value)
            {
                case 0: chpr += printf("did not fire"); break;
                case 8: chpr += printf("fired, commander mode"); break;
                case 9: chpr += printf("fired, TTL mode"); break;
                default: printred("undefined"); break;
            }
            print_endvalue();
            break;
        case 0x0088:    /* Auto Focus Position                    */
#if 0
            print_startvalue();
            /* updated from PHP Metadata Toolkit & exiftool           */
            /* ###%%% don't think these are right:                    */
            /* The two sources conflict.                              */
            /* AFPosition can be tied to metering, which isn't        */
            /* mentioned... There are 4 bytes here that might have    */
            /* significance. Hold off on this. Asterisk the tag.      */
            switch(entry_ptr->value & 0xff)
            {
                case 0:
                    chpr += printf("Single area - ");
                    break;
                case 0x1:
                    chpr += printf("Dynamic area - ");
                    break;
                case 0x2:
                    chpr += printf("Closest subject - ");
                    break;
                default:
                    printred("undefined area -");
                    break;
            }
#endif
            /* It appears that this byte describes autofocus          */
            /* position.                                              */
            /* It is not clear what the other bytes are for.          */
            /* Possibilities are:                                     */
            /*     metering lock to the focus area                    */
            /*     auto vs manual focus area select or "off"          */
            /*     continuous AF vs single AF                         */
            /*     auto focus on closest subject                      */
            /*     actual area used                                   */
            /* ...what *is* "dynamic area"?                           */
            print_startvalue();
            switch((entry_ptr->value >> 8) & 0xff)
            {
                case 0:
                    chpr += printf("Center");
                    break;
                case 0x1:
                    chpr += printf("Top");
                    break;
                case 0x2:
                    chpr += printf("Bottom");
                    break;
                case 0x3:
                    chpr += printf("Left");
                    break;
                case 0x4:
                    chpr += printf("Right");
                    break;
                default:
                    printred("undefined");
                    break;
            }
            print_endvalue();
            break;
        case 0x0089:    /* Shooting Mode */
            print_startvalue();
            switch(entry_ptr->value & 0x3)
            {
                case 0: chpr += printf("single frame"); break;
                case 1: chpr += printf("continuous"); break;
                case 2: chpr += printf("timer"); break;
                case 3: chpr += printf("remote timer?"); break;
                case 4: chpr += printf("remote?"); break;
                default: chpr += printf("(undefined)"); break;
            }
            if(entry_ptr->value & 0x8)
                chpr += printf(", bracketing on");
            else
                chpr += printf(", bracketing off");
            if(entry_ptr->value & 0x20)
                chpr += printf(", WB bracketing on");
            else
                chpr += printf(", WB bracketing off");
            print_endvalue();
            break;
        default:
            break;
    }
    setcharsprinted(chpr);
}

void
nikon2_interpret_offset_makervalue(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
                    char *parent_name,char *nameoftag,int indent)
{
    unsigned long offset;
    int chpr = 0;
    char *fullnameoftag = CNULL;
    int entrywidth;
    double minfl,maxfl,minmaxap,maxmaxap;
    unsigned long num,denom;

    if(entry_ptr && (PRINT_VALUE))
    {
        fullnameoftag = splice(parent_name,".",nameoftag);
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            case 0x0084:
                /* Assume no read errors here, since it's been    */
                /* read once before; if there are, we'll hear     */
                /* about it (but do nothing).                     */
                num = read_ulong(inptr,byteorder,offset);
                denom = read_ulong(inptr,byteorder,HERE);
                if(num == 0UL)
                    minfl = 0.0;
                else 
                    minfl = (double)((double)num/(double)denom);
                num = read_ulong(inptr,byteorder,HERE);
                denom = read_ulong(inptr,byteorder,HERE);
                if(num == 0UL)
                    maxfl = 0.0;
                else 
                    maxfl = (double)((double)num/(double)denom);
                num = read_ulong(inptr,byteorder,HERE);
                denom = read_ulong(inptr,byteorder,HERE);
                if(num == 0UL)
                    minmaxap = 0.0;
                else 
                    minmaxap = (double)((double)num/(double)denom);
                num = read_ulong(inptr,byteorder,HERE);
                denom = read_ulong(inptr,byteorder,HERE);
                if(num == 0UL)
                    maxmaxap = 0.0;
                else 
                    maxmaxap = (double)((double)num/(double)denom);

                if((PRINT_SECTION))
                {
                    chpr = printf(" = {");
                    chpr = newline(chpr);
                    indent += MEDIUMINDENT + MAKERTAGWIDTH + 3;
                    print_tag_address(ENTRY,offset,indent,"@");
                }
                else
                {
                    chpr = newline(chpr);
                    print_tag_address(ENTRY,offset,indent,"@");
                }

                if(PRINT_ENTRY)
                {
                    entrywidth = sizeof("MaxApertureAtMinFocalLength");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",fullnameoftag);
                        chpr += printf("%-*.*s",entrywidth,entrywidth,"MinFocalLength"); 
                    }
                    if((PRINT_VALUE))
                    {
                        print_startvalue();
                        printf("%.0f mm",minfl);
                        print_endvalue();
                        chpr = newline(chpr);
                    }
                    print_tag_address(ENTRY,offset + 8,indent,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",fullnameoftag);
                        chpr += printf("%-*.*s",entrywidth,entrywidth,"MaxFocalLength"); 
                    }
                    if((PRINT_VALUE))
                    {
                        print_startvalue();
                        chpr += printf("%.0f mm",maxfl);
                        print_endvalue();
                        chpr = newline(chpr);
                    }
                    print_tag_address(ENTRY,offset + 16,indent,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",fullnameoftag);
                        chpr += printf("%-*.*s",entrywidth,entrywidth,
                                                "MaxApertureAtMinFocalLength"); 
                    }
                    if((PRINT_VALUE))
                    {
                        /* ###%%% APEX? */
                        chpr += printf(" = %3.2f APEX",minmaxap);
                        minmaxap = pow(2.0,minmaxap/2.0);
                        print_startvalue();
                        chpr += printf("f%.1f",minmaxap);
                        print_endvalue();
                        chpr = newline(chpr);
                    }
                    print_tag_address(ENTRY,offset + 24,indent,"@");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",fullnameoftag);
                        chpr += printf("%-*.*s",entrywidth,entrywidth,
                                                "MaxApertureAtMaxFocalLength"); 
                    }
                    if((PRINT_VALUE))
                    {
                        /* ###%%% APEX? */
                        chpr += printf(" = %3.2f APEX",maxmaxap);
                        maxmaxap = pow(2.0,maxmaxap/2.0);
                        print_startvalue();
                        chpr += printf("f%.1f",maxmaxap);
                        print_endvalue();
                        if(PRINT_SECTION)
                        {
                            chpr = newline(chpr);
                            print_tag_address(ENTRY,offset + 31,0,"@");
                            putindent(indent - MEDIUMINDENT);
                            chpr = printf("}");
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
    setcharsprinted(chpr);
}

/* ###%%% Phil Harvey's ExifTool pages seem to indicate that these are tables of
'rational16' values after an initial 4 character identifier, except that the values
for identifier 0100 are 'int16'.  I dunno; the values I see do not look reasonable
for color balance values (i.e. 'Infinity', '17.2143', ...).  There must be more
than he's saying.

Exiftool picks out two values at model-dependent offsets for red and blue balance,
but doesn't address the rest of the section.  Print those values here, then call
dumpsection so that users can look at the rest.
*/

void
nikon_colorbalance(FILE *inptr,struct ifd_entry *entry_ptr,unsigned short byteorder,
                                            unsigned long fileoffset_base,int indent)
{
    unsigned long offset;
    unsigned short numerator,denominator;
    double value;
    int chpr = 0;
    int unknown = 0;
    char *id;

    if(entry_ptr && (PRINT_VALUE))
    {
        offset = entry_ptr->value + fileoffset_base;
#if 1
        id = read_string(inptr,offset,4);
        chpr += printf("'ID:");
        show_string(id,4,1);
        if(strncmp(id,"0100",4) == 0)
            offset += 72;
        else if(strncmp(id,"0102",4) == 0)
            offset += 10;
        else if(strncmp(id,"0103",4) == 0)
            offset += 20;
        else
            ++unknown;
        if(!unknown)
        {
            if((PRINT_SECTION))
            {
                indent += MAKERTAGWIDTH + 3;
                chpr += printf("' = {");
                chpr = newline(chpr);
                print_tag_address(ENTRY,offset,indent + MEDIUMINDENT,"?");
                chpr += printf("RedBalance**  = ");
                numerator = read_ushort(inptr,byteorder,offset);
                offset += 2;
                if(strncmp(id,"0100",4))
                {
                    denominator = read_ushort(inptr,byteorder,offset);
                    offset += 2;
                }
                else
                    denominator = 256;
                value = (double)numerator / (double)denominator;
                chpr += printf("%.6f (%u/%u)",value,numerator,denominator);
                chpr = newline(chpr);

                print_tag_address(ENTRY,offset,indent + MEDIUMINDENT,"?");
                chpr += printf("BlueBalance** = ");
                numerator = read_ushort(inptr,byteorder,offset);
                offset += 2;
                if(strncmp(id,"0100",4))
                {
                    denominator = read_ushort(inptr,byteorder,offset);
                    offset += 2;
                }
                else
                    denominator = 256;
                value = (double)numerator / (double)denominator;
                chpr += printf("%.6f (%u/%u)",value,numerator,denominator);
                chpr = newline(chpr);
                print_tag_address(ENTRY,offset + entry_ptr->count - 1,indent,"?");
                chpr += printf("} = {");
                indent -= MAKERTAGWIDTH + 3;
            }
            chpr = newline(chpr);
        }
        else
        {
            chpr += printf(" = unknown = {");
            chpr = newline(chpr);
        }
        offset = entry_ptr->value + fileoffset_base;
        PUSHCOLOR(RED);
        dumpsection(inptr,offset,entry_ptr->count,indent);
        POPCOLOR();
        print_tag_address(ENTRY,offset + entry_ptr->count - 1,indent,"?");
        chpr += printf("}");
    }
#else
/* This just prints everything after the idstring as short rationals, */
/* in the same fashion as print_ushort();                             */
        count = entry_ptr->count / 4;   /* two unsigned shorts        */
        print_ascii(inptr,4,offset);
        count--;
        offset += 4;
        chpr = newline(chpr);
        for(i = 0; i < count; ++i)
        {
            int rowlength;
            if((count -i) >= 4)
                rowlength = 4;
            else
                rowlength = count - i;
            print_tag_address(ENTRY,offset,indent,"?");
            for(j = 0; j < rowlength; ++j,++i)
            {
                numerator = read_ushort(inptr,byteorder,offset);
                denominator = read_ushort(inptr,byteorder,HERE);
                offset += 4;
                if(j)
                    chpr += printf(", ");
                if(j == 2)
                    chpr += printf("    ");
                if(numerator == 0)
                    chpr += printf("0     ");
                else if(denominator == 0)
                    chpr += printf("Inf   ");
                else
                {
                    value = (double)numerator / (double)denominator;
                    chpr += printf("%.4f",value);
                }
            }
            chpr = newline(chpr);
        }
    }
    if(Max_undefined > 0)
    {
        unsigned long dumplength;

        offset = entry_ptr->value + fileoffset_base;
        if((Max_undefined == DUMPALL) || (Max_undefined > entry_ptr->count))
            dumplength = entry_ptr->count;
        else 
            dumplength = Max_undefined;
        chpr = newline(chpr);
        hexdump(inptr,offset,entry_ptr->count,dumplength,16,
                    4,SUBINDENT);
        chpr = newline(1);
    }
#endif
    setcharsprinted(chpr);
}

/* Mark items (usually LONG) which should be treated as offsets.      */

int
maker_nikon_value_is_offset(struct ifd_entry *entry_ptr,int model)
{
    int is_offset = 0;

    /* There is currently no need for a switch on noteversion         */
    if(entry_ptr)
    {
        switch(entry_ptr->tag)
        {
            case 0x0011: is_offset = 1; break; 
            case 0x0097: is_offset = 1; break; 
            case 0x0100: is_offset = 1; break; 
            default: break;
        }
    }
    return(is_offset);
}
