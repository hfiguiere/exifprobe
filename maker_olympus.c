/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_olympus.c,v 1.25 2005/07/24 22:56:26 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Olympus camera maker-specific routines                             */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Most of the information coded here is due to TsuruZoh Tachibanaya  */
/* at:                                                                */
/* http://www.ba.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* April, 2005:                                                       */
/* Much of the new information for later models is taken from         */
/* information provided by a Phil Harvey (author of 'exiftool') at:   */
/* http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/index.html   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "defs.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "maker_extern.h"

extern struct camera_id olympus_model_id[];

/* Find the identifying number assigned to known Olympus camera       */
/* models. This number is used to dispatch print and interpret        */
/* routines approopriate to the current image.                        */

int
olympus_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &olympus_model_id[0]; model_id && model_id->name; ++model_id)
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

/* Dispatch a print routine based upon model                          */

void
print_olympus_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1:
                print_olympus1_makervalue(entry_ptr,make,model,PREFIX);
                olympus1_interpret_value(entry_ptr);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Model-specific print routine for Olympus cameras. This routine is  */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and will not be handled properly by print_value()         */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_olympus1_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                    char *prefix)
{
    if(entry_ptr)
    {
        switch(entry_ptr->tag) 
        {
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Dispatch a routine to decode and print offset values for Olympus   */
/* cameras.                                                           */
void
print_olympus_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 1:
                olympus1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                olympus1_interpret_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,parent_name,prefix,indent,
                                        make,model,at_offset);
                break;
        }
    }
}

/* Model-specific routine to print UNDEFINED values found at offsets  */
/* in OLYMPUS makernotes.                                             */

void
olympus1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset,max_offset;
    unsigned long dumplength;
    unsigned long count;
    unsigned short marker;
    char *nameoftag;
    char *fulldirname = CNULL;
    int status = 0;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        if((PRINT_LONGNAMES))
        {
            fulldirname = splice(parent_name,".",nameoftag);
            nameoftag = fulldirname;
        }
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            case 0x0209:    /* CameraId                               */
                if(at_offset)
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,MAKERTAGWIDTH," = ",make,model);
                }
                if((PRINT_VALUE))
                    print_ascii(inptr,entry_ptr->count,value_offset);
                break;
            case 0x0100: /* JPEG Thumbnail in TIFF MakerNote          */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %lu",count); 
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%-9lu",value_offset,entry_ptr->count);
                    else
                        chpr += printf(":%-9lu", entry_ptr->count);
                    if(!(PRINT_VALUE_AT_OFFSET))
                        chpr += printf(" # UNDEFINED");
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                setcharsprinted(chpr);
                chpr = 0;
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
                    print_makertagid(entry_ptr,MAKERTAGWIDTH," : ",make,model);
                    chpr += printf("length %-9lu # UNDEFINED", count);
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%-9lu",value_offset,entry_ptr->count);
                    else
                        chpr += printf(":%-9lu", entry_ptr->count);
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
            case 0x2010:    /* Private IFD's all                      */
            case 0x2020:
            case 0x2030:
            case 0x2040:
            case 0x2050:
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                chpr = newline(chpr);
                max_offset = entry_ptr->value + fileoffset_base + entry_ptr->count;
                max_offset = process_private_ifd(inptr,byteorder,entry_ptr->value,
                                        fileoffset_base,max_offset,entry_ptr->tag,
                                        summary_entry,nameoftag,prefix,
                                        make,model,indent);
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

/* Olympus-specific tagnames for makernotes.                          */

/* The tagname routine is the first place in the code path which      */
/* requires knowledge of the note version. If the version is not      */
/* given in the model table (e.g. the model is unknown), then switch  */
/* code in find_maker_scheme() should have set it. This routine       */
/* repeats the check for non-zero noteversion and is prepared to set  */
/* the noteversion first time through, but should never need to do    */
/* so. Noteversion should always be non-zero; it should be set to -1  */
/* if generic processing is required.                                 */

char *
maker_olympus_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;
    int noteversion = 0;

    if((noteversion = getnoteversion()) == 0)
    {
        /* Need more model samples before it is safe to assume a      */
        /* default                                                    */
        noteversion = 1;
        setnoteversion(1);
    }

    /* Check specific models first                                    */
    switch(noteversion)
    {
        case 1:
            tagname = maker_olympus1_tagname(tag,model);
            break;
        default:
            break;
    }
    return(tagname);
}

char *
maker_olympus1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0100: tagname = "JpegThumbnail"; break;
        case 0x0200: tagname = "SpecialMode"; break;
        case 0x0201: tagname = "CompressionMode"; break; /* "Quality"? */
        case 0x0202: tagname = "Macro"; break;
        case 0x0203: tagname = "BWMode"; break;
        case 0x0204: tagname = "DigitalZoom"; break;
        case 0x0205: tagname = "FocalplaneDiagonal"; break;
        case 0x0207: tagname = "SoftwareRelease"; break;
        case 0x0208: tagname = "PictureInfo"; break;
        case 0x0209: tagname = "CameraID"; break;
        case 0x020b: tagname = "ImageWidth"; break; /* ? */
        case 0x020c: tagname = "ImageHeight"; break; /* ? */
        case 0x0f00: tagname = "Data"; break;

        
        case 0x0300: tagname = "PrecaptureFrames"; break;
        case 0x0302: tagname = "OneTouchWB"; break;


        case 0x1004: tagname = "FlashMode"; break;
        case 0x1005: tagname = "FlashDevice"; break;
        case 0x1006: tagname = "Bracket"; break;
        case 0x100b: tagname = "FocusMode"; break;
        case 0x100c: tagname = "FocusDistance"; break;
        case 0x100d: tagname = "Zoom"; break;
        case 0x100e: tagname = "MacroFocus"; break;
        case 0x100f: tagname = "SharpnessFactor"; break;
        case 0x1011: tagname = "ColorMatrix"; break;
        case 0x1012: tagname = "BlackLevel"; break;
        case 0x1015: tagname = "WhiteBalance"; break;
        case 0x1017: tagname = "RedBalance"; break;
        case 0x1018: tagname = "BlueBalance"; break;
        case 0x101a: tagname = "SerialNumber"; break;
        case 0x1023: tagname = "FlashBias"; break;
        case 0x1029: tagname = "Contrast"; break;
        case 0x102a: tagname = "SharpnessFactor"; break;
        case 0x102b: tagname = "ColorControl"; break;
        case 0x102c: tagname = "ValidBits"; break;
        case 0x102d: tagname = "CoringFilter"; break;
        case 0x102e: tagname = "ImageWidth"; break;
        case 0x102f: tagname = "ImageHeight"; break;
        case 0x1034: tagname = "CompressionRatio"; break;
        case 0x1035: tagname = "PreviewImageValid"; break;
        case 0x1036: tagname = "PreviewImageOffset"; break;
        case 0x1037: tagname = "PreviewImageLength"; break;

        case 0x2010: tagname = "Equipment"; break; /* IFD */
        case 0x2020: tagname = "CameraSettings"; break; /* IFD */
        case 0x2030: tagname = "RawDevelopment"; break; /* IFD */
        case 0x2040: tagname = "ImageProcessing"; break; /* IFD */
        case 0x2050: tagname = "FocusInfo"; break; /* IFD */
        default: break;
    }
    setnotetagset(1);
    return(tagname);
}


void
olympus1_interpret_value(struct ifd_entry *entry_ptr)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0201:    /* Compression Mode */
                print_startvalue();
                /* This appears to report "compression" only, while the   */
                /* camera's "quality" setting combines compression and    */
                /* image size in incomprehensible ways.                   */
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("SQ"); break;
                    case 2: chpr += printf("HQ"); break;
                    case 3: chpr += printf("SHQ"); break;
                    case 4: chpr += printf("RAW1"); break;
                    case 6: chpr += printf("RAW2"); break;
                    case 33: chpr += printf("not compressed"); break; /* TIFF  */
                    case 34: /* not sure about this one ###%%%         */
                             chpr += printf("not compressed");
                             break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0202:    /* Macro */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Macro"); break;
                    case 2: chpr += printf("View"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            default:
                break;
        }
    }
    setcharsprinted(chpr);
}

void
olympus1_interpret_offset_makervalue(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset,value;
    int chpr = 0;

    if((entry_ptr) && (PRINT_VALUE))
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            case 0x0200:    /* SpecialMode                               */
                value = read_ulong(inptr,byteorder,offset);
                print_startvalue();
                switch(value)
                {
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("Unkown"); break;
                    case 2: chpr += printf("Fast"); break;
                    case 3: chpr += printf("Panorama,"); 
                            value = read_ulong(inptr,byteorder,HERE);
                            chpr += printf("#%lu,",value);
                            value = read_ulong(inptr,byteorder,HERE);
                            switch(value)
                            {
                                case 1: chpr += printf(" Left to Right"); break;
                                case 2: chpr += printf(" Right to Left"); break;
                                case 3: chpr += printf(" Bottom to Top"); break;
                                case 4: chpr += printf(" Top to Bottom"); break;
                                default: printred(" undefined"); break;
                            }
                            break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            default:
                break;
        }
    }
    setcharsprinted(chpr);
}

/* ================== OLYMPUS SUBIFD ========================         */

char *
olympus_private_tagname(unsigned short tag,unsigned short subifd_ident)
{
    char *tagname = CNULL;

    switch(subifd_ident)
    {
        case 0x2010:
            tagname = olympus_private_2010_tagname(tag);
            break;
        case 0x2020:
            tagname = olympus_private_2020_tagname(tag);
            break;
        case 0x2030:
            tagname = olympus_private_2030_tagname(tag);
            break;
        case 0x2040:
            tagname = olympus_private_2040_tagname(tag);
            break;
        case 0x2050:
            tagname = olympus_private_2050_tagname(tag);
            break;
        default:
            break;
    }
    return(tagname);
}

char *
olympus_private_2010_tagname(unsigned short tag)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0000: tagname = "EquipmentInfoVersion"; break;
        case 0x0100: tagname = "FirmwareVersion2"; break;
        case 0x0101: tagname = "SerialNumber"; break;
        case 0x0103: tagname = "FocalPlaneDiagonal"; break;
        case 0x0104: tagname = "BodyFirmwareVersion"; break;
        case 0x0201: tagname = "Lens"; break;
        case 0x0202: tagname = "LensSerialNumber"; break;
        case  0x0204: tagname = "LensFirmwareVersion"; break;
        case 0x0206: tagname = "MaxApertureAtMaxFocal"; break;
        case 0x0207: tagname = "MinFocalLength"; break;
        case 0x0208: tagname = "MaxFocalLength"; break;
        case 0x0301: tagname = "Extender"; break;
        case 0x0302: tagname = "ExtenderSerialNumber"; break;
        case 0x0304: tagname = "ExtenderFirmwareVersion"; break;
        case 0x1000: tagname = "FlashType"; break;
        case 0x1001: tagname = "FlashModel"; break;
        case 0x1003: tagname = "FlashSerialNumber"; break;
        case 0x1004: tagname = "FlashFirmwareVersion"; break;
        default:
            break;
    }
    return(tagname);
}

char *
olympus_private_2020_tagname(unsigned short tag)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0000: tagname = "CameraSettingsVersion"; break;
        case 0x0100: tagname = "PreviewImageValid"; break;
        case 0x0101: tagname = "PreviewImageOffset"; break;
        case 0x0102: tagname = "PreviewImageLength"; break;
        case 0x0200: tagname = "ExposureMode"; break;
        case 0x0202: tagname = "MeteringMode"; break;
        case 0x0300: tagname = "MacroMode"; break;
        case 0x0301: tagname = "FocusMode"; break;
        case 0x0302: tagname = "FocusProcess"; break;
        case 0x0303: tagname = "AFSearch"; break;
        case 0x0304: tagname = "AFAreas"; break;
        case 0x0400: tagname = "FlashMode"; break;
        case 0x0401: tagname = "FlashExposureCompensation"; break;
        case 0x0501: tagname = "WhiteBalanceTemperature"; break;
        case 0x0502: tagname = "WhiteBalanceBracket"; break; 
        case 0x0503: tagname = "CustomSaturation"; break;
        case 0x0504: tagname = "ModifiedSaturation"; break;
        case 0x0505: tagname = "ContrastSetting"; break;
        case 0x0506: tagname = "SharpnessSetting"; break;
        case 0x0507: tagname = "ColorSpace"; break;
        case 0x0509: tagname = "SceneMode"; break;
        case 0x050a: tagname = "NoiseReduction"; break;
        case 0x050b: tagname = "DistortionCorrection"; break;
        case 0x050c: tagname = "ShadingCompensation"; break;
        case 0x050d: tagname = "CompressionFactor"; break;
        case 0x050f: tagname = "Gradation"; break;
        case 0x0600: tagname = "Sequence"; break;
        case 0x0603: tagname = "ImageQuality2"; break;
        default:
            break;
    }
    return(tagname);
}

char *
olympus_private_2030_tagname(unsigned short tag)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0000: tagname = "RawDevVersion"; break;
        case 0x0100: tagname = "RawDevExposureBiasValue"; break;
        case 0x0101: tagname = "RawDevWhiteBalanceValue"; break;
        case 0x0102: tagname = "RawDevWBFineAdjustment"; break;
        case 0x0103: tagname = "RawDevGrayPoint"; break;
        case 0x0104: tagname = "RawDevSaturationEmphasis"; break;
        case 0x0105: tagname = "RawDevMemoryColorEmphasis"; break;
        case 0x0106: tagname = "RawDevContrastValue"; break;
        case 0x0107: tagname = "RawDevSharpnessValue"; break;
        case 0x0108: tagname = "RawDevColorSpace"; break;
        case 0x0109: tagname = "RawDevEngine"; break;
        case 0x010a: tagname = "RawDevNoiseReduction"; break;
        case 0x010b: tagname = "RawDevEditStatus"; break;
        case 0x010c: tagname = "RawDevSettings"; break;
        default:
            break;
    }

    return(tagname);
}

char *
olympus_private_2040_tagname(unsigned short tag)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0000: tagname = "ImageProcessingVersion"; break;
        case 0x0100: tagname = "RedBlueBias"; break;
        case 0x0200: tagname = "ColorMatrix"; break;
        case 0x0300: tagname = "SmoothingParameter1"; break;
        case 0x0310: tagname = "SmoothingParameter2"; break;
        case 0x0600: tagname = "SmoothingThresholds"; break;
        case 0x0610: tagname = "SmoothingThreshold2"; break;
        case 0x0611: tagname = "ValidBits"; break;
        case 0x0614: tagname = "ImageWidth2"; break;
        case 0x0615: tagname = "ImageHeight2"; break;
        case 0x1010: tagname = "NoiseFilter2"; break;
        case 0x1012: tagname = "ShadingCompensation2"; break;
        default:
            break;
    }

    return(tagname);
}

char *
olympus_private_2050_tagname(unsigned short tag)
{
    char *tagname = CNULL;

    switch(tag)
    {
        case 0x0000: tagname = "FocusInfoVersion"; break;
        case 0x0209: tagname = "AutoFocus?"; break; 
        case 0x0300: tagname = "ZoomPosition"; break;
        case 0x0305: tagname = "FocusDistance"; break; 
        case 0x1201: tagname = "ExternalFlash"; break;
        case 0x1208: tagname = "InternalFlash"; break;
        default:
            break;
    }

    return(tagname);
}

void
olympus_interpret_pe_value(struct ifd_entry *entry_ptr,unsigned short subifd_ident,
                                                                    char *prefix)
{
    if(entry_ptr)
    {
        switch(subifd_ident)
        {
            case 0x2010:
                olympus2010_interpret_value(entry_ptr,prefix);
                break;
            case 0x2020:
                olympus2020_interpret_value(entry_ptr,prefix);
                break;
            case 0x2030:
                olympus2030_interpret_value(entry_ptr,prefix);
                break;
            case 0x2040:
                olympus2040_interpret_value(entry_ptr,prefix);
                break;
            case 0x2050:
                olympus2050_interpret_value(entry_ptr,prefix);
                break;
            default:
                break;
        }
    }
}

void
olympus2010_interpret_value(struct ifd_entry *entry_ptr,char *prefix)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x1000: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("None"); break;
                    case 2: chpr += printf("Simple E-System"); break;
                    case 3: chpr += printf("E-System"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x1001: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("FL-20"); break;
                    case 2: chpr += printf("FL-50"); break;
                    case 3: chpr += printf("RF-11"); break;
                    case 4: chpr += printf("TF-22"); break;
                    case 5: chpr += printf("FL-36"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            default:
                break;
        }
    }
    setcharsprinted(chpr);
}


void
olympus2020_interpret_value(struct ifd_entry *entry_ptr,char *prefix)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0100: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("No"); break;
                    case 1: chpr += printf("yes"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0200: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("Manual"); break;
                    case 3: chpr += printf("Aperture-priority"); break;
                    case 4: chpr += printf("Shutter-priority"); break;
                    case 5: chpr += printf("Program AE"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0202: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 2: chpr += printf("Center Weighted"); break;
                    case 3: chpr += printf("Spot"); break;
                    case 5: chpr += printf("ESP"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0300: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0301: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Single AF"); break;
                    case 1: chpr += printf("Sequential Shooting AF"); break;
                    case 2: chpr += printf("Continuous AF"); break;
                    case 3: chpr += printf("Multi AF"); break;
                    case 10: chpr += printf("Manual"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0302: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("AF Off"); break;
                    case 1: chpr += printf("AF On"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0303: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Not Ready"); break;
                    case 1: chpr += printf("Ready"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0504: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("CM1 (Red Enhance)"); break;
                    case 2: chpr += printf("CM2 (Green Enhance)"); break;
                    case 3: chpr += printf("CM3 (Blue Enhance)"); break;
                    case 4: chpr += printf("CM4 (Skin Tones)"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0507: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("sRGB"); break;
                    case 1: chpr += printf("Adobe RGB"); break;
                    case 2: chpr += printf("Pro Photo RGB"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0509: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Standard"); break;
                    case 7: chpr += printf("Sport"); break;
                    case 8: chpr += printf("Portrait"); break;
                    case 9: chpr += printf("Landscape+Portrait"); break;
                    case 10: chpr += printf("Landscape"); break;
                    case 11: chpr += printf("Night schene"); break;
                    case 17: chpr += printf("Night+Portrait"); break;
                    case 19: chpr += printf("Fireworks"); break;
                    case 20: chpr += printf("Sunset"); break;
                    case 22: chpr += printf("Macro"); break;
                    case 25: chpr += printf("Documents"); break;
                    case 26: chpr += printf("Museum"); break;
                    case 28: chpr += printf("Beach&Snow"); break;
                    case 30: chpr += printf("Candle"); break;
                    case 39: chpr += printf("High Key"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x050a: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("Noise Reduction On"); break;
                    case 2: chpr += printf("Noise Filter On"); break;
                    case 3: chpr += printf("Noise Reduction + Noise Filter On"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x050b: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x050c: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Off"); break;
                    case 1: chpr += printf("On"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x050f: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case -1: chpr += printf("Low Key"); break;
                    case 0: chpr += printf("Normal"); break;
                    case 1: chpr += printf("High Key"); break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            case 0x0600: 
                chpr += printf("%#x,%#x",(unsigned short)entry_ptr->value & 0xffff,(unsigned short)((entry_ptr->value & 0xffff0000) >> 16));
                break;
            case 0x0603: 
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("SQ"); break;
                    case 2: chpr += printf("HQ"); break;
                    case 3: chpr += printf("SHQ"); break;
                    case 4: chpr += printf("RAW1"); break;
                    case 6: chpr += printf("RAW2"); break;
                    case 33: chpr += printf("not compressed"); break;
                    case 34: /* not sure about this one ###%%%         */
                             chpr += printf("not compressed");
                             break;
                    default: chpr += printf("unknown"); break;
                }
                print_endvalue();
                break;
            default:
                break;
        }
    }
    setcharsprinted(chpr);
}


void
olympus2030_interpret_value(struct ifd_entry *entry_ptr,char *prefix)
{
    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}


void
olympus2040_interpret_value(struct ifd_entry *entry_ptr,char *prefix)
{
    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

void
olympus2050_interpret_value(struct ifd_entry *entry_ptr,char *prefix)
{
    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

/* Pick the appropriate offset value processor for Olympus private    */
/* IFDs based upon noteversion.                                       */

void
olympus_offset_pe_value(FILE *inptr,unsigned short byteorder,
                struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
                unsigned short subifd_ident,struct image_summary *summary_entry,
                char *parent_name,char*prefix,
                int indent,int make,int model,int at_offset)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr)
    {
        switch(noteversion)
        {
            case 1:
                olympus1_offset_pe_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,subifd_ident,
                                            summary_entry,parent_name,prefix,
                                            indent,make,model,at_offset);
                olympus1_interpret_offset_pe_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,subifd_ident);
                break;
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                        fileoffset_base,subifd_ident,
                                        parent_name,prefix,indent,
                                        make,model,at_offset);
                break;
        }
    }
}

/* Pick the appropriate offset value processor for Olympus            */
/* noteversion 1 private IFDs, based upon the tag number of the IFD.  */

void
olympus1_offset_pe_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    unsigned short subifd_ident,struct image_summary *summary_entry,
    char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    if(entry_ptr)
    {
        switch(subifd_ident)
        {
            case 0x2010:
                olympus2010_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 0x2020:
                olympus2020_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 0x2030:
                olympus2030_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 0x2040:
                olympus2040_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case 0x2050:
                olympus2050_offset_value(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
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
}

/* Olympus 'Equipment' IFD from Makernote (0x2010)                    */

void
olympus2010_offset_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long count;
    char *nameoftag;
    char *fulldirname = CNULL;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        if((PRINT_LONGNAMES))
        {
            fulldirname = splice(parent_name,".",nameoftag);
            nameoftag = fulldirname;
        }
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            case 0x0100:
            case 0x0101:
            case 0x0102:
            case 0x0202:
            case 0x0301:
            case 0x0302:
            case 0x1003:
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                        fileoffset_base,0x2010,
                                        fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

/* Olympus 'CameraSettings' IFD from Makernote (0x2020)                    */

void
olympus2020_offset_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    static unsigned long jpegthumbnailoffset = 0UL;
    static unsigned long jpegthumbnailvalid = 1UL;
    unsigned long value_offset,max_offset;
    unsigned long count;
    unsigned short marker;
    char *nameoftag;
    char *fulldirname = CNULL;
    int status = 0;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        if((PRINT_LONGNAMES))
        {
            fulldirname = splice(parent_name,".",nameoftag);
            nameoftag = fulldirname;
        }
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            case 0x0100: /* JPEG Thumbnail valid                      */
                /* The default is 1 (valid), so this can turn off     */
                /* processing if present and 0. Better to attempt     */
                /* processing when invalid than miss a valid image.   */
                jpegthumbnailvalid = value_offset;
                break;
            case 0x0101: /* JPEG Thumbnail offset                     */
                jpegthumbnailoffset = value_offset;
                if(!(PRINT_VALUE_AT_OFFSET))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",jpegthumbnailoffset);
                }
                break;
            case 0x0102: /* JPEG Thumbnail length                     */
                if(jpegthumbnailvalid == 0)
                {
                    jpegthumbnailvalid = 0;
                    break;
                }
                if(jpegthumbnailoffset)
                {
                    count = entry_ptr->value;
                    value_offset = jpegthumbnailoffset;
                    jpegthumbnailoffset = 0UL;
                }
                else
                {
                    printred("# Warning: no thumbnail offset");
                    break;
                }
                indent += SMALLINDENT;
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote SubIFD");
                    chpr += printf(" length %ld",count); 
                }
                if((PRINT_SECTION) || (PRINT_SEGMENT))
                    chpr = newline(chpr);
                marker = read_ushort(inptr,TIFF_MOTOROLA,value_offset);
                max_offset = process_jpeg_segments(inptr,value_offset,marker,
                                            count,summary_entry,fulldirname,
                                                prefix,indent+MEDIUMINDENT);
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                        jpeg_status(status);
                        print_tag_address(VALUE_AT_OFFSET,value_offset + count - 1,
                                                                        indent,"-");
                        chpr += printf("# End of JPEG Thumbnail from MakerNote SubIFD");
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
                        summary_entry->datatype = MAKER_SUBIFD;
                        summary_entry->subfiletype = REDUCED_RES_TYPE;
                    }
                }
                /* make certain we're at the end                      */
                clearerr(inptr);
                fseek(inptr,value_offset + count,SEEK_SET);
                break;
            case 0x0304:
            case 0x0503:
            case 0x0505:
            case 0x0506:
            case 0x050f:
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                        fileoffset_base,0x2020,
                                        fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

/* Olympus 'RawDevelopment' IFD from Makernote (0x2030)                    */

void
olympus2030_offset_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long count;
    char *nameoftag;
    char *fulldirname = CNULL;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        if((PRINT_LONGNAMES))
        {
            fulldirname = splice(parent_name,".",nameoftag);
            nameoftag = fulldirname;
        }
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                        fileoffset_base,0x2030,
                                        fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

/* Olympus 'ImageProcessing' IFD from Makernote (0x2040)                    */

void
olympus2040_offset_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long count;
    char *nameoftag;
    char *fulldirname = CNULL;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        if((PRINT_LONGNAMES))
        {
            fulldirname = splice(parent_name,".",nameoftag);
            nameoftag = fulldirname;
        }
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                        fileoffset_base,0x2040,
                                        fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

/* Olympus 'FocusInfo' IFD from Makernote (0x2050)                    */

void
olympus2050_offset_value(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long count;
    char *nameoftag;
    char *fulldirname = CNULL;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        if((PRINT_LONGNAMES))
        {
            fulldirname = splice(parent_name,".",nameoftag);
            nameoftag = fulldirname;
        }
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            default:
                print_private_offset_value(inptr,byteorder,entry_ptr,
                                        fileoffset_base,0x2050,
                                        fulldirname,prefix,indent,
                                        make,model,at_offset);
                break;
        }
        if(fulldirname)
            free(fulldirname);
    }
    setcharsprinted(chpr);
}

void
olympus1_interpret_offset_pe_value(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
                    unsigned short subifd_ident)
{
    if(entry_ptr)
    {
        switch(subifd_ident)
        {
            case 0x2010:
                olympus2010_interpret_offset_value(inptr,byteorder,entry_ptr,fileoffset_base);
                break;
            case 0x2020:
                olympus2020_interpret_offset_value(inptr,byteorder,entry_ptr,fileoffset_base);
                break;
            case 0x2030:
                olympus2030_interpret_offset_value(inptr,byteorder,entry_ptr,fileoffset_base);
                break;
            case 0x2040:
                olympus2040_interpret_offset_value(inptr,byteorder,entry_ptr,fileoffset_base);
                break;
            case 0x2050:
                olympus2050_interpret_offset_value(inptr,byteorder,entry_ptr,fileoffset_base);
                break;
            default:
                break;
        }
    }
}

void
olympus2010_interpret_offset_value(FILE *inptr,unsigned short byteorder,
        struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset;

    if(entry_ptr)
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

void
olympus2020_interpret_offset_value(FILE *inptr,unsigned short byteorder,
        struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset;

    if(entry_ptr)
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

void
olympus2030_interpret_offset_value(FILE *inptr,unsigned short byteorder,
        struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset;

    if(entry_ptr)
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

void
olympus2040_interpret_offset_value(FILE *inptr,unsigned short byteorder,
        struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset;

    if(entry_ptr)
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

void
olympus2050_interpret_offset_value(FILE *inptr,unsigned short byteorder,
        struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset;

    if(entry_ptr)
    {
        offset = entry_ptr->value + fileoffset_base;
        switch(entry_ptr->tag)
        {
            default:
                break;
        }
    }
}

int
olympus_private_value_is_offset(unsigned short subifd_ident,unsigned short tag)
{
    int is_offset = 0;

    switch(subifd_ident)
    {
        case 0x2010: break;
        case 0x2020: is_offset = olympus2020_value_is_offset(tag); break;
        case 0x2030: break;
        case 0x2040: break;
        case 0x2050: break;
        default: break;
    }
    return(is_offset);
}

int
olympus2020_value_is_offset(unsigned short tag)
{
    int is_offset = 0;

    switch(tag)
    {
        case 0x0100: is_offset = -1; break;
        case 0x0101: is_offset = 1; break;
        case 0x0102: is_offset = -1; break;
        default: break;
    }
    return(is_offset);
}

