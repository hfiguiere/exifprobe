/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_minolta.c,v 1.28 2005/07/24 21:57:28 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* minolta camera maker-specific routines                             */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Most of the information used here is due to Dalibor Jelinek at:    */
/*     http://www.dalibor.cz/minolta/makernote.htm                    */
/*                                                                    */
/* The F100 and "X" models *appear* to use the same MakerNote tagset  */
/* as used for Sanyo models, as decoded by John Hawkins               */
/*     http://www.exif.org/makernotes/SanyoMakerNote.html             */
/*                                                                    */
/* NOTE that this is guesswork. Test images I have found for these    */
/* models appear to decode properly using the tag numbers from        */
/* Hawkinn's Sanyo data.  Send mail if you can confirm or deny.       */
/*                                                                    */
/* The EX model MakerNote format is indecipherable.                   */
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
#include "maker_extern.h"
#include "extern.h"

extern struct camera_id minolta_model_id[];

/* Find the identifying number assigned to known Minolta camera       */
/* models, and record the noteversion and tagset assigned to that     */
/* model. The noteversion is used to dispatch print and interpret     */
/* routines approopriate to the current image.                        */

int
minolta_model_number(char *model,char *software)
{
    struct camera_id *model_id;
    int number = NO_MODEL;

    for(model_id = &minolta_model_id[0]; model_id && model_id->name; ++model_id)
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


/* Dispatch a print routine based upon the noteversion which has been */
/* assigned.                                                          */

void
print_minolta_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                                                            char *prefix)
{
    int noteversion = 0;

    noteversion = getnoteversion();

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(noteversion)
        {
            case 1:
                print_minolta1_makervalue(entry_ptr,make,model,prefix);
                minolta1_interpret_value(entry_ptr,model);
                break;
            default:
                print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Maker-specific print routine for minolta cameras. This routine is  */
/* responsible for picking off any direct entry tags which are        */
/* peculiar and might not be handled properly by print_value()        */
/* (usually UNDEFINED values which fit in the 4-byte entry value). If */
/* there are no such entries, this function simply calls              */
/* print_value().                                                     */

void
print_minolta1_makervalue(struct ifd_entry *entry_ptr,int make, int model,
                                                            char *prefix)
{
    int chpr = 0;
    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag) 
        {
                case 0x0040:    /* ImageByteSize                      */
                    chpr += printf("%lu ",entry_ptr->value);
                    if(entry_ptr->count > 1)
                    {
                        printred("# BAD COUNT");
                        setcharsprinted(chpr);
                    }
                    break;
                default:
                    print_value(entry_ptr,PREFIX);
                break;
        }
    }
}

/* Dispatch a routine to decode and print offset values for minolta   */
/* cameras.                                                           */

void
print_minolta_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    int noteversion = 0;


    if(entry_ptr)
    {
        noteversion = getnoteversion();
        switch(noteversion)
        {
            case 1:
                minolta1_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                minolta1_interpret_offset_makervalue(inptr,byteorder,entry_ptr,
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

/* Maker-specific routine to print UNDEFINED values found at offsets  */
/* in minolta makernotes. One of these may be supplied for each model */
/* with unique UNDEFINED tags.                                        */

void
minolta1_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    static unsigned long jpegthumbnailoffset = 0UL;
    unsigned long value_offset;
    unsigned long dumplength;
    unsigned long count;
    unsigned long max_offset = 0;
    unsigned short marker;
    char *nameoftag,*tagprefix;
    char *fulldirname = NULL;
    int status = 0;
    int chpr = 0;

    if(entry_ptr)
    {
        nameoftag = maker_tagname(entry_ptr->tag,make,model); 
        fulldirname = splice(parent_name,".",nameoftag);
        value_offset = fileoffset_base + entry_ptr->value;
        count = entry_ptr->count;
        switch(entry_ptr->tag)
        {
            case 0x0001:    /* Camera Settings, old                   */
            case 0x0003:    /* Camera Settings, new                   */
                if(at_offset)
                {
                    if((PRINT_SECTION))
                    {
                        print_tag_address(SECTION,value_offset,indent,prefix);
                        chpr += printf("<%s> ",nameoftag);
                        chpr += printf("%lu entries",count / 4);
                    }
                }
                else if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",value_offset);
                }
                if((PRINT_LONGNAMES))
                    tagprefix = fulldirname;
                else
                    tagprefix = nameoftag;
                max_offset = minolta_camerasetting(inptr,byteorder,model,tagprefix,
                                    entry_ptr,value_offset,indent + MEDIUMINDENT);
                if(PRINT_SECTION)
                {
                    chpr = newline(0);
                    print_tag_address(SECTION,max_offset - 1,indent,prefix);
                    chpr += printf("</%s> ",nameoftag);
                }
                setcharsprinted(chpr);
                break;
            case 0x0040:    /* ImageByteSize                      */
                break;      /* should have a count of 1               */
            case 0x0088:    /* JpegThumbnailOffset                    */
                /* It's not magic, there's a trick to it. The offset  */
                /* length will normally be the next entry tag in the  */
                /* IFD, and this will work iff it is. Remember the    */
                /* offset and handle it when the length tag is seen.  */
                /* This is necessary because we do not save entries.  */
                jpegthumbnailoffset = value_offset;
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu",jpegthumbnailoffset);
                }
                break;
            case 0x0089:    /* JpegThumbnailLength                    */
                if(jpegthumbnailoffset == 0UL)
                {
                    printred("# WARNING: JpegThumbnailLength encountered with 0 offset");
                    break;  /* must have the offset.                  */
                }
                value_offset = jpegthumbnailoffset;
                count = entry_ptr->value;
                jpegthumbnailoffset = 0L;
                /* fall through                                       */
            case 0x0081:    /* Jpeg Thumbnail                         */
                /* Minolta seems to use EITHER 0x0088/0x0089 OR       */
                /* 0x0081, not both.                                  */
                if(at_offset && (PRINT_SECTION))
                {
                    print_tag_address(VALUE_AT_OFFSET,value_offset,indent,
                                                        prefix);
                    chpr += printf("# Start of JPEG Thumbnail from MakerNote");
                    chpr += printf(" length %lu",count); 
                }
                else if(!at_offset && (PRINT_VALUE) && (entry_ptr->tag == 0x0081))
                {
                    if(!(PRINT_OFFSET))
                        chpr += printf("@%lu:%lu",value_offset,count);
                    else
                        chpr = printf(":%lu",count);
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
            case 0x0e00: /* PrintIM                                   */
                if(!at_offset && (PRINT_VALUE))
                {
                    if(!(PRINT_OFFSET))
                    {
                        chpr += printf("@%lu:%lu",value_offset,count);
                        chpr = newline(chpr);
                    }
                }
                process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                    count,nameoftag,parent_name,prefix,indent);
                break;
            case 0x0f00: /* Data                                      */
                if(at_offset && (PRINT_ENTRY))
                {
                    print_tag_address(ENTRY,value_offset,indent,prefix);
                    print_makertagid(entry_ptr,23," : ",make,model);
                    chpr += printf(" length %-9lu", count);
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

/* Minolta-specific tagnames for makernotes.                          */
/* Minolta models sport at least 9 different MakerNote tagsets that I */
/* have seen. However, the various tagsets do not overlap (if a given */
/* tag number is used in more than 1 model, it is assigned the same   */
/* meaning everywhere). This makes it possible to use a single        */
/* routine for tagnames, values, interpretations, etc.                */

char *
maker_minolta_tagname(unsigned short tag,int model)
{
    char *tagname = (char *)0;
    int noteversion = 0;

    /* Should have to do this only once, and only for unrecognized    */
    /* models. If the model is recognized (or the user has forced a   */
    /* noteversion) noteversion will already be set.                  */
    if((noteversion = getnoteversion()) == 0)
    {
        noteversion = 1;
        setnoteversion(1);
    }

    switch(noteversion)
    {
        case 1:
            tagname = maker_minolta1_tagname(tag,model);
            break;
        default:
            break;
    }

    /* If no model-specific tag is found, check "generic" tags        */
    if(tagname == NULL)
    {
        switch(tag)
        {
            case 0x0e00: tagname = "PrintIM"; break;
            default: break;
        }
    }
    return(tagname);
}

/* Although several versions of Minolta MakerNotes have been seen in  */
/* the wild, there appears to be no duplication of tag numbers (i.e.  */
/* no tag number is used for different purposes in one note version   */
/* than in another vesion. This makes it possible to treat the notes  */
/* as a single version.                                               */

/* For now...                                                         */

char *
maker_minolta1_tagname(unsigned short tag,int model)
{
    char *tagname = CNULL;

    /* The Minolta MakerNote of TIFF format images have a JPEG        */
    /* thumbnail with a defective SOI marker (0x00f8). In fact,       */
    /* Minolta JPEG_SOI is often missing the high "0xff".             */

    if(tagname == NULL)
    {
        switch(tag)
        {
            case 0x0000: tagname = "Version"; break;
            case 0x0001: tagname = "CameraSettings"; break;
            case 0x0003: tagname = "CameraSettings"; break;
            case 0x0040: tagname = "ImageByteSize"; break;
            case 0x0081: tagname = "JpegThumbnail"; break;
            case 0x0088: tagname = "JpegThumbnailOffset"; break;
            case 0x0089: tagname = "JpegThumbnailLength"; break;
            case 0x0101: tagname = "ColorMode"; break;
            case 0x0102: tagname = "ImageQuality"; break;
            case 0x0103: if(model == MINOLTA_DIMAGE7Hi)
                              tagname = "ImageQuality";
                         else
                              tagname = "ImageSize";
                         break;
            /* The following tagname appears valid for at least the   */
            /* F200, thanks to Harald Puhl (h.puhl@ki-ag.com)         */
            case 0x0104: tagname = "FlashCompensation"; break;
            case 0x0200: tagname = "SpecialMode"; break;

    /* The following tag names appear valid for the F100 and "X" but  */
    /* do not appear to be used for any other Minolta model. They are */
    /* essentially the Sanyo tag set, used here based upon guesswork; */
    /* some Minolta and Sanyo cameras are said to use the same chip,  */
    /* and some F100 and "X" images I've seen contain almost exactly  */
    /* this set of tags (as described by John Hawkins for Sanyo       */
    /* DSC-MZ2 notes). The Quality, Macro, SpecialMode and PrintIM    */
    /* tags are reasonably certain to be correct (they "decode"       */
    /* properly). The remaining names are less certain, and no        */
    /* attempt is made to decode them in minolta1_interpret_value()/  */

            case 0x0201: tagname = "Quality"; break;
            case 0x0202: tagname = "Macro"; break;
            case 0x0204: tagname = "DigiZoom"; break;
            case 0x0207: tagname = "SoftwareRelease"; break;
            case 0x0208: tagname = "PictInfo"; break;
            case 0x0209: tagname = "CameraID"; break;
            case 0x020e: tagname = "SeqShotMethod"; break;
            case 0x020f: tagname = "WideRange"; break;
            case 0x0210: tagname = "ColorAdjMode"; break;
            case 0x0213: tagname = "QuickShot"; break;
            case 0x0214: tagname = "SelfTimer"; break;
            case 0x0216: tagname = "VoiceMemo"; break;
            case 0x0217: tagname = "RecordShutterRel"; break;
            case 0x0218: tagname = "FlickerReduce"; break;
            case 0x0219: tagname = "OpticalZoom"; break;
            case 0x021b: tagname = "DigitalZoom"; break;
            case 0x021d: tagname = "LightSourceSpecial"; break;
            case 0x021e: tagname = "Resaved"; break;
            case 0x021f: tagname = "SceneSelect"; break;
            case 0x0223: tagname = "ManualFocusDist"; break; /* ###%%% not seen */
            case 0x0224: tagname = "SeqShotIntvl"; break; /* ###%%% not seen */
            case 0x0225: tagname = "FlashMode"; break; /* ###%%% not seen */
            case 0x0e00: tagname = "PrintIM"; break;
            case 0x0f00: tagname = "Data"; break;
            default: break;
        }
    }
    setnotetagset(1);
    return(tagname);
}


void
minolta1_interpret_value(struct ifd_entry *entry_ptr,int model)
{
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
    {
        switch(entry_ptr->tag)
        {
            case 0x0101:    /* ColorMode                                  */
                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Natural"); break;
                    case 1: chpr += printf("Black/White*"); break;
                    case 2: chpr += printf("Vivid"); break;
                    case 3: chpr += printf("Solarize"); break;
                    case 4: chpr += printf("Adobe RGB"); break;
                    case 9: chpr += printf("unknown"); break;
                    case 132: chpr += printf("Adobe RGB Embedded Profile"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0102:    /* CameraSettings ImageQuality, 7, 7Hi, F300  */
minolta_7Hi_botch:
                print_startvalue();
                switch(entry_ptr->value)
                {
                    /* ###%%% jpeg quality/compression? check this        */
                    case 0: chpr += printf("Raw"); break;
                    case 1: chpr += printf("Uncompressed"); break; /* superfine, raw? tiff? */
                    case 2: chpr += printf("Fine(1/2)"); break;
                    case 3: chpr += printf("Normal(1/3)"); break;   /* ###%%% */
                    case 4: chpr += printf("Economy(1/4)"); break;  /* ###%%% */
                    case 5: chpr += printf("ExtraFine(1/1)"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0103:    /* CameraSettings ImageSize*, 7Hi, F300       */
                /* The 7Hi appears to get this wrong, printing the same   */
                /* value for this as for 0x0102; the F300 seems to do it  */
                /* right. The ImageSize entry in "Camera Settings"        */
                /* appears correct in both cases. For the 7Hi, it would   */
                /* be wrong to report this as image size, so just don't   */
                /* do it.                                                 */
                if(model == MINOLTA_DIMAGE7Hi)
                    goto minolta_7Hi_botch;

                print_startvalue();
                switch(entry_ptr->value)
                {
                    case 1: chpr += printf("1600x1200"); break;
                    case 2: chpr += printf("unknown"); break;   /* 1280x960? */
                    case 3: chpr += printf("640x480"); break;
                    case 4: chpr += printf("unknown"); break;
                    case 5: chpr += printf("2560x1920"); break;
                    case 6: chpr += printf("2272x1704"); break; 
                    case 7: chpr += printf("2048x1536"); break;
                    case 13: chpr += printf("3264x2448"); break;
                    default: printred("undefined"); break;
                }
                print_endvalue();
                break;
            case 0x0201:    /* Quality, X, F100                           */
                print_startvalue();
                    /* ###%%% jpeg quality/compression? check this, same as 0x0102?       */
                switch(entry_ptr->value)
                {
                    case 0: chpr += printf("Raw*"); break;
                    case 1: chpr += printf("Economy(1/8)"); break;
                    case 2: chpr += printf("Normal(1/4)"); break; 
                    case 3: chpr += printf("Fine(1/2)"); break;
                    case 4: chpr += printf("Uncompressed"); break;
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
minolta1_interpret_offset_makervalue(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base)
{
    unsigned long offset,value;
    int chpr = 0;

    if(entry_ptr && (PRINT_VALUE))
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

/* Interpret the "Camera Settings" tag data from Minolta MakerNotes.  */

#include <math.h>

unsigned long
minolta_camerasetting(FILE *inptr,unsigned short byteorder,int model,
    char *tagprefix,struct ifd_entry *entry_ptr,unsigned long offset,int indent)
{
    unsigned long value,end_offset;
    double fvalue;
    int i,corr_i,entries;
    signed char wcorr;
    int chpr = 0;

    end_offset = offset + entry_ptr->count;
    if((PRINT_SECTION))
        chpr += printf(" length %lu",entry_ptr->count);
    if((PRINT_ENTRY))
    {
        entries = entry_ptr->count / 4;
        for(i = 0; i < entries; ++i)
        {
            chpr = newline(chpr);
            value = read_ulong(inptr,TIFF_MOTOROLA,offset);
            print_tag_address(ENTRY,offset,indent,"@");
            if((PRINT_TAGINFO))
                chpr += printf("%s.",tagprefix);
            offset += 4;

            /* Very oddly new values are inserted after value 51. */
            corr_i=i;
            if((model != MINOLTA_DIMAGE7Hi) && (model != MINOLTA_DIMAGEA1))
            {
                if (i > 50)
                    corr_i++;
            }
            if((model != MINOLTA_DIMAGEA1) && (model != MINOLTA_DIMAGEA2) && (model != MINOLTA_DIMAGEA200))
            {
                if (i > 51)
                    corr_i++;
            }

            switch(corr_i)
            {
                case 0:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 1:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ExposureMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Programmed Auto"); break;
                            case 1: chpr += printf("Aperture Priority Auto"); break;
                            case 2: chpr += printf("Shuttter Priority Auto"); break;
                            case 3: chpr += printf("Manual"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 2:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FlashMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Fill-Flash"); break;
                            case 1: chpr += printf("Red Eye Reduction"); break;
                            case 2: chpr += printf("Rear Flash Sync"); break;
                            case 3: chpr += printf("Wireless"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 3:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"WhiteBalance");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#lx",value);
                        print_startvalue();
                        if (value & 0x00ff0000)
                        {
                            /* New style white balance values */
                            switch(value >> 24)
                            {
                                case 0: printf("Auto"); break;
                                case 1: printf("Daylight"); break;
                                case 2: printf("Cloudy"); break;
                                case 3: printf("Incandescent"); break;
                                case 5: printf("Fluorescent"); break;
                                case 6: printf("Shadow"); break;
                                case 7: printf("Memory1"); break;
                                case 8: printf("Memory2"); break;
                                case 9: printf("Memory3"); break;
                                default: printred("unknown"); break;
                            }
                            wcorr = ((value & 0x00ffffff) >> 16) - 0x80;
                            switch(wcorr)
                            {
                                case 0: break;
                                default: printf(" %+hd", wcorr); break;
                            }
                        }
                        else
                        {
                            /* Old style white balance values */
                            switch(value)
                            {
                                case 0: chpr += printf("Auto"); break;
                                case 1: chpr += printf("Daylight"); break;
                                case 2: chpr += printf("Cloudy"); break;
                                case 3: chpr += printf("Incandescent"); break;
                                case 5: chpr += printf("Custom"); break;
                                case 7: chpr += printf("Fluorescent"); break;
                                case 8: chpr += printf("Fluorescent2"); break;
                                case 11: chpr += printf("Custom2"); break;
                                case 12: chpr += printf("Custom3"); break;
                                default: printred("unknown"); break;
                            }
                        }
                        print_endvalue();
                    }
                    break;
                case 4:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ImageSize");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            /* ###%%% There should also be a          */
                            /* 2080x1560 size, at least for the A1    */
                            case 0: switch(model)
                                    {
                                        case MINOLTA_DIMAGEA2:
                                            chpr += printf("3264x2448");
                                            break;
                                        case MINOLTA_DIMAGES404:
                                        case MINOLTA_DIMAGES414:
                                            chpr += printf("2272x1704");
                                            break;
                                        case MINOLTA_DIMAGE5:
                                            chpr += printf("2048x1536");
                                            break;
                                        case MINOLTA_DIMAGEA1:
                                        case MINOLTA_DIMAGE7Hi:
                                        case MINOLTA_DIMAGE7i:
                                        default:
                                            chpr += printf("2560x1920");
                                            break;
                                    }
                                    break;
                            case 1: chpr += printf("1600x1200"); break;
                            case 2: chpr += printf("1280x960"); break;
                            case 3: chpr += printf("640x480"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 5:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ImageQuality");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("RAW"); break;
                            case 1: chpr += printf("Super Fine"); break;
                            case 2: chpr += printf("Fine"); break;
                            case 3: chpr += printf("Standard"); break;
                            case 4: chpr += printf("Economy"); break;
                            case 5: chpr += printf("Extra Fine"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 6:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"DriveMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Single"); break;
                            case 1: chpr += printf("Continuous"); break;
                            case 2: chpr += printf("Self Timer"); break;
                            case 4: chpr += printf("Bracketing"); break;
                            case 5: chpr += printf("Interval"); break;
                            case 6: chpr += printf("UHS Continuous"); break;
                            case 7: chpr += printf("HS Continuous"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 7:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"MeteringMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Multi-Segment"); break;
                            case 1: chpr += printf("Center Weighted"); break;
                            case 2: chpr += printf("Spot"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 8:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"CCDSentitivity");
                    if((PRINT_VALUE))
                    {
                        if(PRINT_RAW_VALUES)
                            chpr += printf(" = %lu",value);
                        fvalue = ((double)value/8.0) - 1.0;
                        chpr += printf(" = %.3g APEX",fvalue);
                        fvalue = pow(2.0,fvalue) * 3.125;
                        print_startvalue();
                        chpr += printf("%.3g ISO",fvalue);
                        print_endvalue();
                    }
                    break;
                case 9:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"ShutterSpeed");
                    if((PRINT_VALUE))
                    {
                        if(PRINT_RAW_VALUES)
                            chpr += printf(" = %lu",value);
                        if (value == 8)
                            fvalue = 30.0; /* Due to rounding error x=8 should be displayed as 30 sec. */
                        else
                        {
                            fvalue = ((double)value / 8.0) - 6.0;
                            chpr += printf(" = %.3g APEX",fvalue);
                            fvalue = pow(2.0,-fvalue);
                        }
                        print_startvalue();
                        chpr += printf("%.3g sec",fvalue);
                        print_endvalue();
                    }
                    break;
                case 10:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"Aperture");
                    if((PRINT_VALUE))
                    {
                        if(PRINT_RAW_VALUES)
                            chpr += printf(" = %lu",value);
                        fvalue = (double)value/8.0 - 1.0;
                        chpr += printf(" = %.3g APEX",fvalue);
                        fvalue = pow(2.0,fvalue/2.0);
                        print_startvalue();
                        chpr += printf("f%.2g",fvalue);
                        print_endvalue();
                    }
                    break;
                case 11:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"MacroMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("On"); break;
                            case 1: chpr += printf("Off"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 12:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"DigitalZoom");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Off"); break;
                            case 1: chpr += printf("Electronic"); break;
                            case 2: chpr += printf("2x"); break;
                            /* FIXME: http://www.dalibor.cz/minolta/makernote.htm says 2 */
                            case 0xffff: chpr += printf("2x"); break;
                            default: printred("Undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 13:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"ExposureCompensation");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        chpr += printf("%g EV",(double)value/3.0 - 2);
                        print_endvalue();
                    }
                    break;
                case 14:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"BracketStep");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("1/3"); break;
                            case 1: chpr += printf("2/3"); break;
                            case 2: chpr += printf("1"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 15:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 16:
                    if((PRINT_TAGINFO))
                        /* FIXME: Values don't fit on A1 */
                        chpr += printf("%02d_%-21.21s",i,"IntervalLength");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value + 1);
                        chpr += printf(" minutes");
                    }
                    break;
                case 17:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"IntervalNumber");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 18:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FocalLength");
                    if((PRINT_VALUE))
                    {
                        /* This value is not equal to FocalLength in public
                           EXIF as the public one is tweaked to get 7x zoom to
                           comply with marketing specification. The real zoom
                           is 6.51x. * 3.9333 is 35mm equivalent */
                        chpr += printf(" = %lu",value);
                        fvalue = (double)value/256.0;
                        print_startvalue();
                        chpr += printf("%g mm",fvalue);
                        print_endvalue();
                    }
                    break;
                case 19:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FocalDistance");
                    if((PRINT_VALUE))
                    {
                        if (value == 0)
                        {
                            chpr += printf(" = infinity");
                        }
                        else
                        {
                            chpr += printf(" = %lu",value);
                            chpr += printf(" mm");
                        }
                    }
                    break;
                case 20:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FlashFired");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        if(value == 0UL)
                            chpr += printf("No");
                        else
                            chpr += printf("Yes");
                        print_endvalue();
                    }
                    break;
                case 21:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Date");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#010lx",value);
                        print_startvalue();
                        chpr += printf("%02ld:%02ld:%02ld",(value & 0xffff0000) >> 16,
                                (value & 0xff00) >> 8,value & 0xff);
                        print_endvalue();
                    }
                    break;
                case 22:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Time");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %#010lx",value);
                        print_startvalue();
                        chpr += printf("%02ld:%02ld:%02ld",(value & 0xffff0000) >> 16,
                                (value & 0xff00) >> 8,value & 0xff);
                        print_endvalue();
                    }
                    break;
                case 23:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"MaxAperture");
                    if((PRINT_VALUE))
                    {
                        if(PRINT_RAW_VALUES)
                            chpr += printf(" = %lu",value);
                        fvalue = (double)value/8.0 - 1.0;
                        chpr += printf(" = %.3g APEX",fvalue);
                        fvalue = pow(2.0,fvalue/2.0);
                        print_startvalue();
                        chpr += printf("f%.2g",fvalue);
                        print_endvalue();
                    }
                    break;
                case 24:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 25:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 26:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FileNumberMemory");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        if(value == 0UL)
                            chpr += printf("Off");
                        else
                            chpr += printf("On");
                        print_endvalue();
                    }
                    break;
                case 27:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"LastFileNumber");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        /* 0 if FileNumberMemory is off */
                        if(value == 0)
                        {
                            print_startvalue();
                            chpr += printf("(FileNumberMemory Off)");
                            print_endvalue();
                        }
                    }
                    break;
                case 28:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"WhiteBalanceRed");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        chpr += printf("%.3f",value/256.0);
                        print_endvalue();
                    }
                    break;
                case 29:
                    /* Warning! Green white balance coefficient in    */
                    /* raw file may not be equal to one and in this   */
                    /* case WB coefficients stored in raw file will   */
                    /* not match those stored here.                   */
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"WhiteBalanceGreen");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        chpr += printf("%.3f",value/256.0);
                        print_endvalue();
                    }
                    break;
                case 30:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"WhiteBalanceBlue");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        chpr += printf("%.3f",value/256.0);
                        print_endvalue();
                    }
                    break;
                case 31:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Saturation");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        if ((model == MINOLTA_DIMAGEA1) || (model == MINOLTA_DIMAGEA2) || (model == MINOLTA_DIMAGEA200))
                            chpr += printf("%+ld",value-5);
                        else
                            chpr += printf("%+ld",value-3);

                        print_endvalue();
                    }
                    break;
                case 32:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Contrast");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        if ((model == MINOLTA_DIMAGEA1) || (model == MINOLTA_DIMAGEA2) || (model == MINOLTA_DIMAGEA200))
                            chpr += printf("%+ld",value-5);
                        else
                            chpr += printf("%+ld",value-3);

                        print_endvalue();
                    }
                    break;
                case 33:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Sharpness");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Hard"); break;
                            case 1: chpr += printf("Normal"); break;
                            case 2: chpr += printf("Soft"); break;
                            case 3: chpr += printf("None"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 34:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Subject Program");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("None"); break;
                            case 1: chpr += printf("Portrait"); break;
                            case 2: chpr += printf("Text"); break;
                            case 3: chpr += printf("Night Portrait"); break;
                            case 4: chpr += printf("Sunset"); break;
                            case 5: chpr += printf("Sports Action"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 35:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FlashCompensation");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        fvalue = ((double)value - 6.0) / 3.0;
                        print_startvalue();
                        chpr += printf("%g EV",fvalue);
                        print_endvalue();
                    }
                    break;
                case 36:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ISO Setting");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("100"); break;
                            case 1: chpr += printf("200"); break;
                            case 2: chpr += printf("200"); break;
                            case 3: chpr += printf("800"); break;
                            case 4: chpr += printf("Auto"); break;
                            case 5: chpr += printf("64"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 37:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"CameraModel");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("DiMAGE 7"); break;
                            case 1: chpr += printf("DiMAGE 5"); break;
                            case 2: chpr += printf("DiMAGE S304"); break;
                            case 3: chpr += printf("DiMAGE S404"); break;
                            case 4: chpr += printf("DiMAGE 7i"); break;
                            case 5: chpr += printf("DiMAGE 7Hi"); break;
                            case 6: chpr += printf("DiMAGE A1"); break;
                            case 7: chpr += printf("DiMAGE (A2 or S414)"); break;
                            /* A200 doesn't appear to have            */
                            /* CameraSettings                         */
                            default: printred("unknown"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 38:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"IntervalMode*");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        /* FIXME: verify it */
                        {
                            case 0: chpr += printf("Still Image"); break;
                            case 1: chpr += printf("Time Lapse"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 39:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FolderName");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Standard Form"); break;
                            case 1: chpr += printf("Data Form"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 40:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ColorMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Natural Color"); break;
                            case 1: chpr += printf("Black and White"); break;
                            case 2: chpr += printf("Vivid Color"); break;
                            case 3: chpr += printf("Solarization"); break;
                            case 4: chpr += printf("Adobe RGB"); break;
                            case 9: chpr += printf("unknown"); break;
                            case 132: chpr += printf("Adobe RGB Embedded Profile"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 41:
                    /* FIXME: assign values */
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ColorFilter");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        switch(model)
                        {
                            case MINOLTA_DIMAGEA2:
                            case MINOLTA_DIMAGEA1:
                                print_startvalue();
                                switch(value)
                                {
                                    case 0: chpr += printf("cool 100%%"); break;
                                    case 1: chpr += printf("cool 80%%"); break;
                                    case 2: chpr += printf("cool 60%%"); break;
                                    case 3: chpr += printf("cool 40%%"); break;
                                    case 4: chpr += printf("cool 20%%"); break;
                                    case 5: chpr += printf("no cool/warm adjustment"); break;
                                    case 6: chpr += printf("warm 20%%"); break;
                                    case 7: chpr += printf("warm 40%%"); break;
                                    case 8: chpr += printf("warm 60%%"); break;
                                    case 9: chpr += printf("warm 80%%"); break;
                                    case 10: chpr += printf("warm 100%%"); break;
                                    default: chpr += printf("undefined"); break;
                                }
                                print_endvalue();
                                break;
                            case MINOLTA_DIMAGE7Hi:
                            case MINOLTA_DIMAGE7i:
                                print_startvalue();
                                switch(value)
                                {
                                    case 0: chpr += printf("cool 100%%"); break;
                                    case 1: chpr += printf("cool 67%%"); break;
                                    case 2: chpr += printf("cool 33%%"); break;
                                    case 3: chpr += printf("no cool/warm adjustment"); break;
                                    case 4: chpr += printf("warm 33%%"); break;
                                    case 5: chpr += printf("warm 67%%"); break;
                                    case 6: chpr += printf("warm 100%%"); break;
                                    default: chpr += printf("undefined"); break;
                                }
                                print_endvalue();
                                break;
                            default:
                                /* Print nothing unless the model is  */
                                /* known                              */
                                break;
                        }
                    }
                    break;
                case 42:
                    /* FIXME: assign values */
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"BWFilter");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 43:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"InternalFlashFired");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("No"); break;
                            case 1: chpr += printf("Yes"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 44:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d_%-21.21s",i,"BrightnessValue");
                    if((PRINT_VALUE))
                    {
                        if(PRINT_RAW_VALUES)
                            chpr += printf(" = %lu",value);
                        fvalue = ((double)value/ 8.0) - 6.0;
                        chpr += printf(" = %g APEX",fvalue);
                        fvalue = pow(2.0,fvalue);
                        print_startvalue();
                        chpr += printf("%.3f foot lambert",fvalue);
                        print_endvalue();
                    }
                    break;
                case 45:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"SpotFocusPointX");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 46:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"SpotFocusPointY");
                    if((PRINT_VALUE))
                        chpr += printf(" = %lu",value);
                    break;
                case 47:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"WideFocusZone");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        /* FIXME: Different for A1 */
                        if(model == MINOLTA_DIMAGEA1)
                            chpr += printf("Unknown");
                        else
                        {
                            switch(value)
                            {
                                case 0: chpr += printf("no zone"); break;
                                case 1: chpr += printf("center zone (horizontal)"); break;
                                case 2: chpr += printf("center zone (vertical)"); break;
                                case 3: chpr += printf("left zone"); break;
                                case 4: chpr += printf("right zone"); break;
                                default: printred("undefined"); break;
                            }
                        }
                        print_endvalue();
                    }
                    break;
                case 48:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FocusMode");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Auto"); break;
                            case 1: chpr += printf("Manual"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 49:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"FocusArea");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Wide"); break;
                            case 1: chpr += printf("Spot"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 50:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"DECPosition");
                    if((PRINT_VALUE))
                    {
                        chpr += printf(" = %lu",value);
                        print_startvalue();
                        switch(value)
                        {
                            case 0: chpr += printf("Exposure"); break;
                            case 1: chpr += printf("Contrast"); break;
                            case 2: chpr += printf("Saturation"); break;
                            case 3: chpr += printf("Filter"); break;
                            default: printred("undefined"); break;
                        }
                        print_endvalue();
                    }
                    break;
                case 51:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"ColorProfileEmbedded");
                    if((PRINT_VALUE))
                    {
                            chpr += printf(" = %lu",value);
                            print_startvalue();
                            switch(value)
                            {
                                case 0: chpr += printf("No"); break;
                                case 1: chpr += printf("Yes"); break;
                                default: printred("undefined"); break;
                            }
                            print_endvalue();
                    }
                    break;
                case 52:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"AntiShake");
                    if((PRINT_VALUE))
                    {
                            chpr += printf(" = %lu",value);
                            print_startvalue();
                            switch(value)
                            {
                                case 0: chpr += printf("Off"); break;
                                case 1: chpr += printf("On"); break;
                                default: printred("undefined"); break;
                            }
                            print_endvalue();
                    }
                    break;
                case 53:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"DataImprint");
                    if((PRINT_VALUE))
                    {
                            chpr += printf(" = %lu",value);
                            print_startvalue();
                            switch(value)
                            {
                                case 0: chpr += printf("None"); break;
                                case 1: chpr += printf("YYYY/MM/DD"); break;
                                case 2: chpr += printf("MM/DD/HR.MIN"); break;
                                case 3: chpr += printf("Text"); break;
                                case 4: chpr += printf("Text + Id#"); break;
                                default: printred("undefined"); break;
                            }
                            print_endvalue();
                    }
                    break;
                default:
                    if((PRINT_TAGINFO))
                        chpr += printf("%02d:%-21.21s",i,"Unknown");
                    if((PRINT_VALUE))
                        chpr += printf(" = %#lx/%lu",value,value);
                    break;
            }
        }
    }
    setcharsprinted(chpr);
    return(end_offset);
}

int
maker_minolta_value_is_offset(struct ifd_entry *entry_ptr,int model)
{
    int is_offset = 0;

    if(entry_ptr)
    {
        switch(entry_ptr->tag)
        {
            case 0x0040: is_offset = -1; break; /* cancel */
            case 0x0088: is_offset = 1; break;
            case 0x0089: is_offset = -1; break;
            default: break;
        }
    }
    return(is_offset);
}

/* I have seen at least 9 Minolta versions, as shown below.           */
/* Fortunately, some of the variations are minor, and the tagsets do  */
/* not overlap, so it is possible to service all of them with just    */
/* one tagset and one set of processing routines. The version strings */
/* in the notes have no apparent meaning.                             */

/* tagsets: HEX
0,1,10,20,40,81,e00  MLT0
0,1,10,20,40,E00  mlt0
0,3,10,20,40,88,89,E00 MLT0
0,3,10,20,40,88,89,100-103 MLT0
0,4,10,18,20,40,88,89,100-107,10A-10D (MRW) MLT0
0,88,89,100-103,F00 MLY0
0,88,89,100-103,F00,18 (MRW) MLY0
0,200-21F,E00,F00   MLT0
0,81,200-21F,E00,F00 MLT0
*/
