/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: interpret.c,v 1.18 2005/07/24 21:26:04 alex Exp $";
#endif

#include <stdio.h>

#include "defs.h"
#include "datadefs.h"
#include "summary.h"
#include "maker.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "ciff.h"

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* This file contains the functions which interpret and print         */
/* "meaningful" descriptions for values found in TIFF and EXIF ifds.  */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */



static void print_aperture(FILE *,unsigned short,unsigned long);
static void print_brightness(FILE *,unsigned short,unsigned long);
static void print_shutterspeed(FILE *,unsigned short,unsigned long);

static void print_capturetype(unsigned long);
static void print_colorspace(unsigned long);
static void print_componentsconfig(unsigned long);
static void print_compression(unsigned long);
static void print_contrast(unsigned long);
static void print_custom(unsigned long);
static void print_epfpresunit(unsigned long);
static void print_epcfapattern(unsigned long,unsigned long);
static void print_offset_epcfapattern(FILE *,struct ifd_entry *,
                                                    unsigned long);
static void print_exposuremode(unsigned long);
static void print_exposureprogram(unsigned long);
static void print_filesource(unsigned long source);
static void print_flash(unsigned long flash);
static void print_gaincontrol(unsigned long);
static void print_jpegproc(unsigned long);
static void print_lightsource(unsigned long);
static void print_meteringmode(unsigned long);
static void print_newsubfiletype(unsigned long);
static void print_oldsubfiletype(unsigned long);
static void print_orientation(unsigned long);
static void print_photometric(unsigned long,int);
static void print_planarconfig(unsigned long);
static void print_resunit(unsigned long);
static void print_saturation(unsigned long);
static void print_scenetype(unsigned long type);
static void print_sensingmethod(unsigned long);
static void print_sharpness(unsigned long);
static void print_subjectdistancerange(unsigned long);
static void print_whitebalance(unsigned long);
static void print_ycbcrpositioning(unsigned long);

static void print_35mmfocallength();

void
interpret_value(struct ifd_entry *entry_ptr,struct image_summary *summary_entry)
{
    switch(entry_ptr->tag)
    {
        case TIFFTAG_Compression:
            print_compression(entry_ptr->value);
            break;
        case TIFFTAG_PhotometricInterpretation:
            if (summary_entry)
            {
                print_photometric(entry_ptr->value,summary_entry->fileformat);
            }
            break;
        case TIFFTAG_Orientation:
            print_orientation(entry_ptr->value);
            break;
        case TIFFTAG_ResolutionUnit:
        case EXIFTAG_FocalPlaneResolutionUnit:
            print_resunit(entry_ptr->value);
            break;
        case TIFFEPTAG_FocalPlaneResolutionUnit:
            print_epfpresunit(entry_ptr->value);
            break;
        case EXIFTAG_ColorSpace:
            print_colorspace(entry_ptr->value);
            break;
        case EXIFTAG_MeteringMode:
            print_meteringmode(entry_ptr->value);
            break;
        case TIFFEPTAG_CFAPattern:
            print_epcfapattern(entry_ptr->value,entry_ptr->count);
            break;
        case TIFFEPTAG_SensingMethod:
        case EXIFTAG_SensingMethod:
            print_sensingmethod(entry_ptr->value);
            break;
        case EXIFTAG_ExposureProgram:
            print_exposureprogram(entry_ptr->value);
            break;
        case EXIFTAG_ComponentsConfiguration:
            print_componentsconfig(entry_ptr->value);
            break;
        case TIFFTAG_OldSubFileType:
            print_oldsubfiletype(entry_ptr->value);
            break;
        case TIFFTAG_NewSubFileType:
            print_newsubfiletype(entry_ptr->value);
            break;
        case TIFFTAG_PlanarConfiguration:
            print_planarconfig(entry_ptr->value);
            break;
        case TIFFTAG_JpegProc:
            print_jpegproc(entry_ptr->value);
            break;
        case TIFFTAG_YcbCrPositioning:
            print_ycbcrpositioning(entry_ptr->value);
            break;
        case EXIFTAG_LightSource:
            print_lightsource(entry_ptr->value);
            break;
        case EXIFTAG_Flash:
            print_flash(entry_ptr->value);
            break;
        case  EXIFTAG_FileSource:
            print_filesource(entry_ptr->value);
            break;
        case  EXIFTAG_SceneType:
            print_scenetype(entry_ptr->value);
            break;
        case EXIFTAG_ExifCustomRendered:
            print_custom(entry_ptr->value);
            break;
        case EXIFTAG_ExifExposureMode:
            print_exposuremode(entry_ptr->value);
            break;
        case EXIFTAG_ExifWhiteBalance:
            print_whitebalance(entry_ptr->value);
            break;
        case EXIFTAG_ExifDigitalZoomRatio:
            /* Nothing to do                                          */
            break;
        case EXIFTAG_ExifFocalLengthIn35mmFilm:
            print_35mmfocallength();
            break;
        case EXIFTAG_ExifSceneCaptureType:
            print_capturetype(entry_ptr->value);
            break;
        case EXIFTAG_ExifGainControl:
            print_gaincontrol(entry_ptr->value);
            break;
        case EXIFTAG_ExifContrast:
            print_contrast(entry_ptr->value);
            break;
        case EXIFTAG_ExifSaturation:
            print_saturation(entry_ptr->value);
            break;
        case EXIFTAG_ExifSharpness:
            print_sharpness(entry_ptr->value);
            break;
        case EXIFTAG_ExifSubjectDistanceRange:
            print_subjectdistancerange(entry_ptr->value);
            break;
        case EXIFTAG_ExifImageUniqueId:
            /* Nothing to do                                          */
            break;
        default:
            break;
    }
}

void
interpret_offset_value(FILE *inptr,struct ifd_entry *entry_ptr,unsigned short byteorder,
                                                           unsigned long fileoffset_base)
{
    unsigned long offset;
    int chpr = 0;

    switch(entry_ptr->tag)
    {
        case TIFFEPTAG_CFAPattern:
            print_offset_epcfapattern(inptr,entry_ptr,fileoffset_base);
            break;
        case EXIFTAG_ExposureTime:
            setcharsprinted(printf(" sec"));
            break;
        case EXIFTAG_ShutterSpeedValue:
            chpr += printf(" APEX"); 
            offset = entry_ptr->value + fileoffset_base;
            print_shutterspeed(inptr,byteorder,offset);
            break;
        case EXIFTAG_ApertureValue:
        case EXIFTAG_MaxApertureValue:
        case EXIFTAG_FNumber:
            chpr += printf(" APEX");
            offset = entry_ptr->value + fileoffset_base;
            print_aperture(inptr,byteorder,offset);
            break;
        case EXIFTAG_BrightnessValue:
            chpr += printf(" APEX");
            offset = entry_ptr->value + fileoffset_base;
            print_brightness(inptr,byteorder,offset);
            break;
        case EXIFTAG_FocalLength:
            chpr += printf(" mm");
            break;
        case EXIFTAG_SubjectDistance:
            chpr += printf(" meters");
            break;
        case EXIFTAG_ExposureBiasValue:
            chpr += printf(" APEX");
            break;
    }
    setcharsprinted(chpr);
}

void
print_compression(unsigned long compvalue)
{
    char *compstr;
    int chpr = 0;

    if((compstr = tiff_compression_string(compvalue)))
    {
        print_startvalue();
        chpr += printf("%s",compstr);
        print_endvalue();
    }
    setcharsprinted(chpr);
}

void
print_photometric(unsigned long metric,int fileformat)
{
    int chpr = 0;

    switch(fileformat)
    {
        case FILEFMT_ORF1:
        case FILEFMT_ORF2:
            print_startvalue();
            switch(metric)
            {
                case 1: chpr += printf("CFA 16 bit"); break;
                case 2: chpr += printf("CFA 12 bit"); break;
                default: chpr += printf("undefined"); break;
            }
            print_endvalue();
            break;
        default:
            print_startvalue();
            switch(metric)
            {
                case 0: chpr += printf("MINISWHITE"); break;
                case 1: chpr += printf("MINISBLACK"); break;
                case 2: chpr += printf("RGB"); break;
                case 3: chpr += printf("Palette"); break;
                case 4: chpr += printf("Transparency Mask"); break;
                case 5: chpr += printf("Separated"); break;
                case 6: chpr += printf("YCbCr"); break;
                case 8: chpr += printf("CIELAB"); break;
                case 10: chpr += printf("ITULAB"); break;
                case 32803: chpr += printf("CFA"); break;    /* TIFFEP, DNG */
                case 32844: chpr += printf("CIE Log2(L)"); break;
                case 32845: chpr += printf("CIE Log2(L) (u',v')"); break;
                case 34892: chpr += printf("LinearRaw"); break;
                default: chpr += printf("undefined"); break;
            }
            print_endvalue();
            break;
    }
    setcharsprinted(chpr);
}

void
print_orientation(unsigned long value)
{
    int chpr = 0;

    print_startvalue();
    switch(value)
    {
        case 1: chpr += printf("0,0 is top left"); break;
        case 2: chpr += printf("0,0 is top right"); break;
        case 3: chpr += printf("0,0 is bottom right"); break;
        case 4: chpr += printf("0,0 is bottom left"); break;
        case 5: chpr += printf("0,0 is left top"); break;
        case 6: chpr += printf("0,0 is right top"); break;
        case 7: chpr += printf("0,0 is right bottom"); break;
        case 8: chpr += printf("0,0 is left bottom"); break;
        default: chpr += printf("undefined"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_resunit(unsigned long unit)
{
    int chpr = 0;

    print_startvalue();
    switch(unit)
    {
        case 1: chpr += printf("no units"); break;
        case 2: chpr += printf("pixels per inch"); break;
        case 3: chpr += printf("pixels per cm"); break;
        default: chpr += printf("undefined"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_epfpresunit(unsigned long unit)
{
    int chpr = 0;

    print_startvalue();
    switch(unit)
    {
        case 1: chpr += printf("no units"); break;
        case 2: chpr += printf("per meter"); break;
        case 3: chpr += printf("per cm"); break;
        case 4: chpr += printf("per mm"); break;
        case 5: chpr += printf("per um"); break;
        default: chpr += printf("undefined"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_epcfapattern(unsigned long value,unsigned long count)
{
    unsigned char *p;
    int i;
    int chpr = 0;

    p = (unsigned char *)&value;
    print_startvalue();
    for(i = 0; i < count; ++i)
    {
        if(i)
        {
            putchar(',');
            ++chpr;
        }
        putcolorchar((unsigned short)*p++ & 0xff);
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_offset_epcfapattern(FILE *inptr,struct ifd_entry *entry_ptr,
                                    unsigned long fileoffset_base)
{
    unsigned long offset;
    unsigned short p;
    int i;
    int chpr = 0;

    if(inptr)
    {
        offset = entry_ptr->value + fileoffset_base;
        print_startvalue();
        for(i = 0; i < entry_ptr->count; ++i)
        {
            p = read_ubyte(inptr,offset++);
            if(i)
            {
                putchar(',');
                ++chpr;
            }
            putcolorchar(p & 0xff);
        }
        print_endvalue();
    }
    setcharsprinted(chpr);
}

void
print_colorspace(unsigned long space)
{
    int chpr = 0;

    print_startvalue();
    if(space == 1)
        chpr += printf("sRGB");
    else if(space == 0xffff)
        chpr += printf("uncalibrated");
    else
        chpr += printf("reserved");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_meteringmode(unsigned long mode)
{
    int chpr = 0;

    print_startvalue();
    switch(mode)
    {
        case 0: chpr += printf("unknown"); break;
        case 1: chpr += printf("Average"); break;
        case 2: chpr += printf("Center Weighted Average"); break;
        case 3: chpr += printf("Spot"); break;
        case 4: chpr += printf("Multi Spot"); break;
        case 5: chpr += printf("Pattern"); break;
        case 6: chpr += printf("Partial"); break;
        default: if(mode < 255) chpr += printf("reserved");
            else chpr += printf("other"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}


void
print_sensingmethod(unsigned long method)
{
    int chpr = 0;

    print_startvalue();
    switch(method)
    {
        case 0: chpr += printf("Not defined"); break;
        case 1: chpr += printf("Monochrome area sensor"); break;
        case 2: chpr += printf("One-chip color area sensor"); break;
        case 3: chpr += printf("Two-chip color area sensor"); break;
        case 4: chpr += printf("Three-chip color area sensor"); break;
        case 5: chpr += printf("Color sequential area sensor"); break;
        case 7: chpr += printf("Trilinear sensor"); break;
        case 8: chpr += printf("Color sequential linear sensor"); break;
        default: chpr += printf("reserved"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_exposureprogram(unsigned long program)
{
    int chpr = 0;

    print_startvalue();
    switch(program)
    {
        case 0: chpr += printf("Not defined"); break;
        case 1: chpr += printf("Manual"); break;
        case 2: chpr += printf("Normal"); break;
        case 3: chpr += printf("Aperture Priority"); break;
        case 4: chpr += printf("Shutter Priority"); break;
        case 5: chpr += printf("Creative Program"); break;
        case 6: chpr += printf("Action Program"); break;
        case 7: chpr += printf("Portrait Mode"); break;
        case 8: chpr += printf("Landscape Mode"); break;
        default:
            if(program < 255)
                printf("reserved");
            break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_componentsconfig(unsigned long components)
{
    char *comp = (char *)&components;
    int i;
    int chpr = 0;

    print_startvalue();
    for(i = 0; i < 4; ++i)
    {
        switch(*comp++)
        {
        case 1: putchar('Y'); break;
        case 2: putchar('C'); putchar('b'); break;
        case 3: putchar('C'); putchar('r'); break;
        case 4: putchar('R'); break;
        case 5: putchar('G'); break;
        case 6: putchar('B'); break;
        case 0: break;
        default:
            putchar('.');
            break;
        }
        ++chpr;
    }
    print_endvalue();
    setcharsprinted(chpr);
}


void
print_newsubfiletype(unsigned long type)
{
    int chpr = 0;

    print_startvalue();
    switch(type)
    {
        case 0: chpr += printf("primary"); break;
        case 1: chpr += printf("thumbnail"); break;
        case 2: chpr += printf("page"); break;
        case 4: chpr += printf("mask"); break;
        default: chpr += printf("unknown"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}


void
print_oldsubfiletype(unsigned long type)
{
    int chpr = 0;

    print_startvalue();
    if(type == 1)
        chpr += printf("primary");
    else if(type == 2)
        chpr += printf("thumbnail");
    else
        chpr += printf("unknown");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_planarconfig(unsigned long config)
{
    int chpr = 0;

    print_startvalue();
    if(config == 1)
        chpr += printf("chunky/contig");
    else if(config == 2)
        chpr += printf("planar/separate");
    else
        chpr += printf("unknown");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_jpegproc(unsigned long process)
{
    int chpr = 0;

    print_startvalue();
    if(process == 1)
        chpr += printf("baseline");
    else if(process == 14)
        chpr += printf("lossless");
    else
        chpr += printf("unknown");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_ycbcrpositioning(unsigned long position)
{
    int chpr = 0;

    print_startvalue();
    if(position == 1)
        chpr += printf("centered");
    else if(position == 2)
        chpr += printf("co-sited");
    else
        chpr += printf("unknown");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_lightsource(unsigned long source)
{
    int chpr = 0;

    print_startvalue();
    switch(source)
    {
        case 0: chpr += printf("unknown"); break;
        case 1: chpr += printf("Daylight"); break;
        case 2: chpr += printf("Fluorescent"); break;
        case 3: chpr += printf("Tungsten"); break;
        case 4: chpr += printf("Flash"); break;
        case 9: chpr += printf("Fine weather"); break;
        case 10: chpr += printf("Cloudy weather"); break;
        case 11: chpr += printf("Shade"); break;
        case 12: chpr += printf("Daylight fluorescent"); break;
        case 13: chpr += printf("Day white fluorescent"); break;
        case 14: chpr += printf("Cool white fluorescent"); break;
        case 15: chpr += printf("Fluorescent"); break;
        case 17: chpr += printf("Standard light A"); break;
        case 18: chpr += printf("Standard light B"); break;
        case 19: chpr += printf("Standard light C"); break;
        case 20: chpr += printf("D65"); break;
        case 21: chpr += printf("D55"); break;
        case 22: chpr += printf("D75"); break;
        case 23: chpr += printf("D50"); break;
        case 24: chpr += printf("ISO studio tungsten"); break;
        case 25: chpr += printf("Other light source"); break;
        default:
                chpr += printf("reserved");
            break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_flash(unsigned long flash)
{
    unsigned long bits;
    int chpr = 0;

    print_startvalue();
    bits = flash & 0x1;
    if(bits != 0)
        chpr += printf("flash fired");
    else
        chpr += printf("no flash");    /* there will be more          */
    bits = (flash & 0x6) >> 1;
    switch(bits)    /* return detection                               */
    {
        case 2: chpr += printf(" - return not detected"); break;
        case 3: chpr += printf(" - return detected"); break;
        default: break;
    }
    bits = (flash & 0x18) >> 3; /* flash mode                         */
    switch(bits)
    {
        case 1: chpr += printf(" - compulsory"); break;
        case 2: chpr += printf(" - suppressed"); break;
        case 3: chpr += printf(" - auto"); break;
        default: break;
    }
    bits = (flash & 0x20) >> 5; /* flash function                     */
    if(bits != 0)
        chpr += printf(" - no flash function present");
    bits = (flash & 0x40) >> 6; /* red-eye                            */
    if(bits != 0)
        chpr += printf(" - no red-eye function present");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_filesource(unsigned long source)
{
    int chpr = 0;

    print_startvalue();
    switch(source)
    {
        case 0: chpr += printf("not specified"); break;
        case 1: chpr += printf("scanner - transparent"); break;
        case 2: chpr += printf("scanner - reflex"); break;
        case 3: chpr += printf("DSC"); break;
        default: chpr += printf("undefined");
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_scenetype(unsigned long type)
{
    int chpr = 0;

    print_startvalue();
    if(type == 1)
        chpr += printf("direct photo");
    else
        chpr += printf("not direct photo");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_custom(unsigned long rendered)
{
    int chpr = 0;

    print_startvalue();
    if(rendered == 0)
        chpr += printf("Normal");
    else if(rendered == 1)
        chpr += printf("Custom");
    else
        chpr += printf("reserved");
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_exposuremode(unsigned long mode)
{
    int chpr = 0;

    print_startvalue();
    switch(mode)
    {
        case 0: chpr += printf("Auto"); break;
        case 1: chpr += printf("Manual"); break;
        case 2: chpr += printf("Auto Bracket"); break;
        default: chpr += printf("???"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_whitebalance(unsigned long mode)
{
    int chpr = 0;

    print_startvalue();
    switch(mode)
    {
        case 0: chpr += printf("Auto"); break;
        case 1: chpr += printf("Manual"); break;
        default: chpr += printf("???"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_35mmfocallength()
{
    /* %%%### */
    printf("mm");
    setcharsprinted(2);
}

void
print_capturetype(unsigned long type)
{
    int chpr = 0;

    print_startvalue();
    switch(type)
    {
        case 0: chpr += printf("Standard"); break;
        case 1: chpr += printf("Landscape"); break;
        case 2: chpr += printf("Portrait"); break;
        case 3: chpr += printf("Night Scene"); break;
        default: chpr += printf("reserved"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_gaincontrol(unsigned long type)
{ 
    int chpr = 0;

    print_startvalue();
    switch(type)
    {
        case 0: chpr += printf("None"); break;
        case 1: chpr += printf("Low gain up"); break;
        case 2: chpr += printf("High gain up"); break;
        case 3: chpr += printf("Low gain down"); break;
        case 4: chpr += printf("Hign gain down"); break;
        default: chpr += printf("reserved"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_contrast(unsigned long contrast)
{
    int chpr = 0;

    print_startvalue();
    switch(contrast)
    {
        case 0: chpr += printf("Normal"); break;
        case 1: chpr += printf("Soft"); break;
        case 2: chpr += printf("Hard"); break;
        default: chpr += printf("???"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_saturation(unsigned long saturation)
{
    int chpr = 0;

    print_startvalue();
    switch(saturation)
    {
        case 0: chpr += printf("Normal"); break;
        case 1: chpr += printf("Low"); break;
        case 2: chpr += printf("High"); break;
        default: chpr += printf("???"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_sharpness(unsigned long sharpness)
{
    int chpr = 0;

    print_startvalue();
    switch(sharpness)
    {
        case 0: chpr += printf("Normal"); break;
        case 1: chpr += printf("Soft"); break;
        case 2: chpr += printf("Hard"); break;
        default: chpr += printf("???"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_subjectdistancerange(unsigned long range)
{
    int chpr = 0;

    print_startvalue();
    switch(range)
    {
        case 0: chpr += printf("???"); break;
        case 1: chpr += printf("Macro"); break;
        case 2: chpr += printf("Close view"); break;
        case 3: chpr += printf("Distant view"); break;
        default: chpr += printf("???"); break;
    }
    print_endvalue();
    setcharsprinted(chpr);
}

#include <math.h>

void
print_aperture(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    double aperture_value,fnumber;
    unsigned long num,denom;
    int chpr = 0;

    num = read_ulong(inptr,byteorder,offset);
    denom = read_ulong(inptr,byteorder,HERE);
    aperture_value = (double)((double)num/(double)denom);
    fnumber = pow(2.0,aperture_value/2.0);
    print_startvalue();
    chpr += printf("f%.1f",fnumber);
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_shutterspeed(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    double speed_value,exptime;
    long num,denom;
    int chpr = 0;

    num = read_ulong(inptr,byteorder,offset);
    denom = read_ulong(inptr,byteorder,HERE);
    speed_value = (double)((double)num/(double)denom);
    exptime = pow(2.0,-speed_value);
    print_startvalue();
    chpr += printf("%g sec",exptime);
    print_endvalue();
    setcharsprinted(chpr);
}

void
print_brightness(FILE *inptr,unsigned short byteorder,unsigned long offset)
{
    double brightness_value,brightness;
    long num,denom;
    int chpr = 0;

    num = read_ulong(inptr,byteorder,offset);
    denom = read_ulong(inptr,byteorder,HERE);
    brightness_value = (double)((double)num/(double)denom);
    brightness = pow(2.0,brightness_value);
    print_startvalue();
    chpr += printf("%.3g foot lambert",brightness);
    print_endvalue();
    setcharsprinted(chpr);
}
