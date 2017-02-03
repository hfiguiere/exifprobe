/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: print_maker.c,v 1.29 2005/07/24 17:27:09 alex Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "defs.h"
#include "summary.h"
#include "maker.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

extern struct camera_name Camera_make[];

#ifndef STRCASESTR
#define STRCASESTR fake_strcasestr
extern char *fake_strcasestr(char *,char *);
#endif

/* Print an IFD entry presumed to be from a MakerNote.                */

/* This differs from print_entry() in that it dispatches              */
/* maker_tagname() to get tag names specific to a specific make and   */
/* (possibly) model, and calls print_makervalue() to intercept any    */
/* tags which may require special treatment due to maker oddities.    */
/* The maker-specific routines dispatched by print_makervalue() will  */
/* also handle interpretation of the tag values if known.             */

unsigned long
print_makerentry(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
                    struct image_summary *summary_entry,char *parent_name,
                    int indent,int make,int model)
{
    int value_is_offset = 0;
    unsigned long endoffset = 0UL;
    int chpr = 0;

    value_is_offset = is_offset(entry_ptr);
    if(value_is_offset == 0)
        value_is_offset = maker_value_is_offset(entry_ptr,make,model);
    else
        value_is_offset += maker_value_is_offset(entry_ptr,make,model);

    if(PRINT_ENTRY)
        print_makertaginfo(entry_ptr,parent_name,indent,make,model);
    if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE))
    {
        if(PRINT_ENTRY)
            chpr = printf("# INVALID MAKERNOTE ENTRY (%#lx)",entry_ptr->value);
        endoffset = ftell(inptr);
    }
    else if(entry_ptr->count == 0)
    {
        if(PRINT_VALUE)
            chpr = printf("EMPTY (%lu)",entry_ptr->value);
        endoffset = ftell(inptr);
    }
    else if(value_is_offset)
    {
        if(value_is_offset > 0)
        {
            if(PRINT_BOTH_OFFSET)
                chpr += printf("@%#lx=%lu",entry_ptr->value + fileoffset_base,
                        entry_ptr->value + fileoffset_base);
            else if(PRINT_HEX_OFFSET)
                chpr += printf("@%#lx",entry_ptr->value + fileoffset_base);
            else if(PRINT_DEC_OFFSET)
                chpr += printf("@%lu",entry_ptr->value + fileoffset_base);

            if(fileoffset_base && (PRINT_ENTRY_RELOFFSET))
                chpr += printf(" (%lu)",entry_ptr->value);
        }
        else if((value_is_offset < 0) && (PRINT_VALUE))
            print_makervalue(entry_ptr,make,model,PREFIX);

        /* Print "inline" in REPORT & LIST modes                      */
        if(!(PRINT_VALUE_AT_OFFSET))
        {
            setcharsprinted(chpr);
            chpr = 0;
            endoffset = print_offset_makervalue(inptr,byteorder,
                                        entry_ptr,fileoffset_base,
                                        summary_entry,parent_name,
                                        "@",indent + SUBINDENT,
                                        make,model,0);
        }
    }
    else if(PRINT_VALUE)
        print_makervalue(entry_ptr,make,model,PREFIX);
    chpr = newline(chpr);
    return(endoffset);
}

/* Repeat the tag identifier for values printed at an offset. This is */
/* called only from the "*makervalue-at_offset()" routines when       */
/* printing offset values at the offset (rather than "inline"), and   */
/* repeats only one of the identifiers (preferably tagname) followed  */
/* by the value separator (if values are printed at all). If no tag   */
/* identifier is enabled in Print_options, the tag name will be       */
/* printed, as in the entry line printed by print_makertaginfo().     */

/* The caller will normally pass the 'sep' argument as ' = ' if an    */
/* actual value is to follow, or ': ' if a description (e.g. length   */
/* of UNDEFINED segment which is not expanded).                       */

void
print_makertagid(struct ifd_entry *entry_ptr,int width, char *sep,
                                                int make,int model)
{
    char *nameoftag;
    int chpr = 0;

    if(entry_ptr)
    {
        if((PRINT_TAGNAME))
        {
            nameoftag = maker_tagname(entry_ptr->tag,make,model);
            chpr = printf("%-*.*s",width,width,nameoftag);
        }
        else if(PRINT_HEX_TAGNO)
            chpr = printf("<%#06x>",entry_ptr->tag & 0xffff);
        else if(PRINT_DEC_TAGNO)
            chpr = printf("<%5u>",entry_ptr->tag & 0xffff);
        if(PRINT_VALUE)
            chpr += printf("%s",sep ? sep : " = ");
    }
    setcharsprinted(chpr);
}

/* Print the part of a MakerNote entry describing the entry tag,      */
/* including it's tag number, name and type. Only the items enabled   */
/* in "Print_options" are printed.                                    */

/* In LIST mode prefix the tag name with a section identifier, so     */
/* that the resulting names may be recognized by patterns or regular  */
/* expressions applied to the output (e.g. 'egrep') in                */
/* post-processing.                                                   */

void
print_makertaginfo(struct ifd_entry *entry_ptr,char *parent_name,
                                        int indent,int make,int model)
{
    char *nameoftag;
    char *nameof_value_type;
    int chpr = 0;

    if(entry_ptr)
    {
        if(PRINT_BOTH_TAGNO)
            chpr = printf("<%#06X=%5u> ",entry_ptr->tag & 0xffff,entry_ptr->tag & 0xffff);
        else if(PRINT_HEX_TAGNO)
            chpr = printf("<%#06X> ",entry_ptr->tag & 0xffff);
        else  if(PRINT_DEC_TAGNO)
            chpr = printf("<%5u> ",entry_ptr->tag & 0xffff);
        if((PRINT_TAGNAME))
        {
            nameoftag = maker_tagname(entry_ptr->tag,make,model);
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",parent_name);
            chpr += printf("%-*.*s",MAKERTAGWIDTH,MAKERTAGWIDTH,nameoftag);
        }
        if(PRINT_TYPE)
        {
            nameof_value_type = value_type_name(entry_ptr->value_type);
            chpr += printf(" [%2u=%-9.9s %5lu] ",entry_ptr->value_type,
                    nameof_value_type,entry_ptr->count);
        }
        if(Debug & NOTE_DEBUG)
        {
            chpr += printf(" %d:%d,%d ",model,getnoteversion(),getnotetagset());
        }
        if(PRINT_VALUE)
            chpr += printf(" = "); 
    }
    setcharsprinted(chpr);
}

/* Dispatch maker-specific routines to print (and possibly interpret) */
/* non-offset values in MakerNote IFDs. If the make is not known, the */
/* normal print_value() routine will be called, and the               */
/* maker-specific routines are free to do the same if the tag doesn't */
/* need special handling (such as a 4 byte UNDEFINED which should be  */
/* printed as ascii). The maker-specific routines can also call       */
/* routines to interpret the value (e.g "On", "Off", etc.) according  */
/* to the requirements of the make and model.                         */

void
print_makervalue(struct ifd_entry *entry_ptr,int make,int model,char *prefix)
{
    if(entry_ptr)
    {
        switch(make)
        {
            case MAKER_AGFA:
                print_agfa_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_ASAHI:   /* Asahi is Pentax                    */
            case MAKER_PENTAX:  /* Pentax is Asahi                    */
                print_asahi_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_CANON:
                print_canon_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_CASIO:
                print_casio_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_EPSON:
                print_epson_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_FUJIFILM:
                print_fujifilm_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_HP:
                print_value(entry_ptr,PREFIX);
                break;
            case MAKER_KODAK:
                print_value(entry_ptr,PREFIX);
                break;
            case MAKER_KONICA:
                print_konica_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_KYOCERA:
                print_kyocera_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_LEICA:
                print_leica_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_MINOLTA:
                print_minolta_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_NIKON:
                print_nikon_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_OLYMPUS:
                print_olympus_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_PANASONIC:
                print_panasonic_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_RICOH:
                print_value(entry_ptr,PREFIX);
                break;
            case MAKER_SANYO:
                print_sanyo_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_SIGMA:
                print_sigma_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_SONY:
                print_sony_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_TOSHIBA:
                print_toshiba_makervalue(entry_ptr,make,model,PREFIX);
                break;
            case MAKER_TRAVELER:
                print_value(entry_ptr,PREFIX);
                break;
            default: /* No maker_specific print routine available     */
                print_value(entry_ptr,PREFIX); /* use default`        */
                break;
        }
    }
}

/* Print values found at offsets in a MakerNote entry by dispatching  */
/* maker-specific routines which know how to interpret UNDEFINED      */
/* values for that maker. These routines may also intercept "defined" */
/* values which are handled strangely, such as a "long" value which   */
/* is really an offset, or a 4-byte UNDEFINED value which is really   */
/* an offset to an UNDEFINED blob. If a maker-specific routine is     */
/* called, it must handle all values for that maker (possibly by      */
/* calling the generic routine). The generic and maker-specific       */
/* routines are all 'void' functions, which should assure that the    */
/* file pointer is moved past the subject data. This routine          */
/* clears any IO errors (or EOF) and returns the calculated file      */
/* offset of the end of the section.                                  */

unsigned long
print_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,
    struct image_summary *summary_entry,char *parent_name,char*prefix,
    int indent,int make,int model,int at_offset)
{
    unsigned long endoffset = 0L;

    if(inptr && entry_ptr)
    {
        if((value_type_size(entry_ptr->value_type) * entry_ptr->count) <= 4UL)
            endoffset = 0;
        else
            endoffset = fileoffset_base + entry_ptr->value + 
                (value_type_size(entry_ptr->value_type) * entry_ptr->count);

        switch(make)
        {
            case MAKER_AGFA:
                print_agfa_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_ASAHI:   /* Pentax                             */
            case MAKER_PENTAX:  /* Pentax                             */
                print_asahi_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_CANON:
                print_canon_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,make,
                                            model,at_offset);
                break;
            case MAKER_CASIO:
                print_casio_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_EPSON:
                print_epson_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_FUJIFILM:
                print_fujifilm_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_HP:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_KODAK:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_KYOCERA:
                print_kyocera_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_KONICA:
                print_konica_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_LEICA:
                print_leica_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_MINOLTA:
                print_minolta_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_NIKON:
                print_nikon_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_OLYMPUS:
                print_olympus_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_PANASONIC:
                print_panasonic_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_RICOH:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_SANYO:
                print_sanyo_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
											parent_name,prefix,indent,make,
											model,at_offset);
                break;
            case MAKER_SIGMA:
                print_sigma_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_SONY:
                print_sony_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_TOSHIBA:
                print_toshiba_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,summary_entry,
                                            parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            case MAKER_TRAVELER:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
            default:
                print_generic_offset_makervalue(inptr,byteorder,entry_ptr,
                                            fileoffset_base,parent_name,prefix,indent,
                                            make,model,at_offset);
                break;
        }
    }
    else
    {
        printred(" invalid call to print_offset_makervalue ");
        if(inptr == (FILE *)0)
            printred(" no input file pointer ");
        if(entry_ptr == (struct ifd_entry *)0)
            printred(" null entry pointer");
    }
    if(ferror(inptr) || feof(inptr))
        clearerr(inptr);
    (void)newline(0);
    if(ftell(inptr) > endoffset)
        endoffset = ftell(inptr);
    return(endoffset);
}

/* Print the value found at an offset in a generic MakerNote, if no   */
/* maker-specific routine wants to deal with it. Similar to           */
/* print_offset_value(). The routine is expected to read or skip all  */
/* data associated with the entry.                                    */

void
print_generic_offset_makervalue(FILE *inptr,unsigned short byteorder,
    struct ifd_entry *entry_ptr,unsigned long fileoffset_base,char *parent_name,
    char *prefix,int indent,int make,int model,int at_offset)
{
    unsigned long value_offset;
    unsigned long dumplength;
    int chpr = 0;

    if((value_type_size(entry_ptr->value_type) * entry_ptr->count) > 4)
    {
        value_offset = fileoffset_base + entry_ptr->value;
        if(PRINT_VALUE)
        {
            if(at_offset)
            {
                print_tag_address(ENTRY,value_offset,indent,prefix);
                if(entry_ptr->value_type == UNDEFINED)
                    print_makertagid(entry_ptr,MAKERTAGWIDTH," : ",make,model);
                else
                    print_makertagid(entry_ptr,MAKERTAGWIDTH," = ",make,model);
            }
            else
            {
                if((PRINT_OFFSET) && (entry_ptr->value_type != UNDEFINED))
                    chpr += printf(" = ");
            }
            switch(entry_ptr->value_type)
            {
                case UNDEFINED: 
                    /* Could make a pseudo-tag for 'count' in LIST    */
                    /* mode...                                        */
                    if((PRINT_SECTION) && at_offset && (PRINT_VALUE_AT_OFFSET))
                        chpr += printf("length %-9lu # UNDEFINED",entry_ptr->count);
                    else
                    {
                        if(!(PRINT_OFFSET))
                            chpr += printf("@%lu",value_offset);
                        chpr += printf(":%-9lu # UNDEFINED",entry_ptr->count);
                    }
                    if(Max_undefined == 0)
                    {
                        if((PRINT_SECTION))
                            printred(" (not dumped, use -U)");
                    }
                    else
                    {
                        /* Even in LIST and REPORT modes              */
                        if((Max_undefined == DUMPALL)
                            || (Max_undefined > entry_ptr->count))
                                dumplength = entry_ptr->count;
                        else 
                            dumplength = Max_undefined;
                        chpr = newline(1);
                        hexdump(inptr,entry_ptr->value + fileoffset_base,
                                    entry_ptr->count,dumplength,12,
                                    indent,SUBINDENT);
                    }
                    /* make certain we're at the end                  */
                    fseek(inptr,entry_ptr->value + fileoffset_base + entry_ptr->count,SEEK_SET);
                    break;
                case ASCII:
                    print_ascii(inptr,entry_ptr->count,value_offset); 
                    break;
                case BYTE:
                    print_ubytes(inptr,entry_ptr->count,value_offset);
                    break;
                case SBYTE:
                    print_sbytes(inptr,entry_ptr->count,value_offset);
                    break;
                case SHORT:
                    print_ushort(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SSHORT:
                    print_sshort(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case LONG:
                    print_ulong(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SLONG:
                    print_slong(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case RATIONAL:
                    print_urational(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case SRATIONAL:
                    print_srational(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case FLOAT:
                    print_float(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                case DOUBLE:
                    print_double(inptr,entry_ptr->count,byteorder,value_offset);
                    break;
                default:
                    chpr = printf(" INVALID TYPE %#x",entry_ptr->value_type);
                    break;
            }
        }
    }
    if(ferror(inptr) || feof(inptr))
        clearerr(inptr);
    setcharsprinted(chpr);
}


void
print_scheme(FILE *inptr,struct maker_scheme *maker_scheme)
{
    int chpr = 0;

    PUSHCOLOR(HI_BLACK);
    chpr = newline(1);
    chpr += printf("MakerNote Scheme:");
    chpr += printf(" (at offset %lu, length %lu)",maker_scheme->note_fileoffset,
                maker_scheme->note_length);
    chpr = newline(chpr);
    chpr += printf("\tScheme Type = ");
    switch(maker_scheme->scheme_type)
    {
        case PLAIN_IFD_SCHEME:
            chpr += printf("\"Plain IFD\" (%#lx) at offset %lu",maker_scheme->scheme_type,
                        maker_scheme->note_fileoffset);
            chpr = newline(chpr);
            break;
        case HAS_ID_SCHEME:
            chpr += printf("\"ID\" (%#lx)\n",maker_scheme->scheme_type);
            chpr += printf("\tID = \"%s\";",maker_scheme->id[0] ? maker_scheme->id : "(NONE)");
            chpr += printf(" length = %d, at offset %lu\n",maker_scheme->id_length,
                        maker_scheme->note_fileoffset);
            chpr += printf("\tIFD is offset %d from start of section",maker_scheme->ifd_offset);
            chpr = newline(chpr);
            break;
        case HAS_ID_PLUS_TIFFHDR_SCHEME:
            chpr += printf("\"ID plus TIFF header\" (%#lx)\n",maker_scheme->scheme_type);
            chpr += printf("\tID = \"%s\";",maker_scheme->id[0] ? maker_scheme->id : "(NONE)");
            chpr += printf("\tlength = %d, at offset %lu\n",maker_scheme->id_length,
                        maker_scheme->note_fileoffset);
            chpr += printf("\tTIFF header is offset %d from start of section\n",maker_scheme->tiffhdr_offset);
            chpr += printf("\tIFD is offset %d from start of TIFF header",maker_scheme->ifd_offset);
            chpr = newline(chpr);
            break;
        case UNKNOWN_SCHEME:
        default:
            chpr += printf("\"UNKNOWN\" (%#lx)\n",maker_scheme->scheme_type);
            chpr += printf("\tID = \"%s\";",maker_scheme->id[0] ? maker_scheme->id : "(NONE)");
            chpr += printf(" length = %d, at offset %lu",maker_scheme->id_length,
                        maker_scheme->note_fileoffset);
            chpr = newline(chpr);
            break;
    }
    if(maker_scheme->note_byteorder &&
            (maker_scheme->note_byteorder != maker_scheme->tiff_byteorder))
        PUSHCOLOR(HI_RED);

    chpr += printf("\tNote byteorder = ");
    print_byteorder(maker_scheme->note_byteorder,2);
    if(maker_scheme->note_byteorder &&
            (maker_scheme->note_byteorder != maker_scheme->tiff_byteorder))
    {
        chpr += printf(" != ");
        POPCOLOR();
    }
    else
    {
        chpr = newline(chpr);
        chpr += printf("\t");
    }
    chpr += printf("Tiff byteorder = ");
    print_byteorder(maker_scheme->tiff_byteorder,2);
    chpr += newline(chpr);
    switch(maker_scheme->offsets_relative_to)
    {
        case START_OF_TIFFHEADER:
                    chpr += printf("\tOffsets relative to TIFF header at offset %lu",
                                maker_scheme->fileoffset_base);
                    chpr = newline(chpr);
                    break;
        case START_OF_NOTE:
                    PUSHCOLOR(HI_RED);
                    chpr += printf("\tOffsets relative to start of MakerNote");
                    POPCOLOR();
                    chpr += printf(" at offset %lu",maker_scheme->note_fileoffset);
                    chpr += newline(1);
                    break;
        case START_OF_ENTRY:
                    PUSHCOLOR(HI_RED);
                    chpr += printf("\tOffsets relative to start of entry");
                    POPCOLOR();
                    chpr += newline(1);
                    break;
        default: chpr += printf("(UNKNOWN)\n");
                    break;
    }
    chpr += printf("\tNumber of entries = %d",maker_scheme->num_entries);
    chpr = newline(chpr);
    if((maker_scheme->scheme_type != UNKNOWN_SCHEME) && maker_scheme->no_nextifd_offset)
    {
        putchar('\t'); ++chpr;
        PUSHCOLOR(HI_RED);
        chpr += printf("No next IFD offset");
        POPCOLOR();
        chpr += newline(1);
    }
    if(Make_name)
    {
        putchar('\t'); ++chpr;
        chpr += printf("Make = \"%s\"",Make_name);
        chpr += printf("  (maker id = %d)",maker_scheme->make);
        chpr = newline(1);
    }
    if(Model_name)
    {
        putchar('\t'); ++chpr;
        chpr += printf("Model = \"%s\"",Model_name);
        chpr += printf("  (model id = %d)",maker_scheme->model);
        chpr = newline(1);
    }
    if(Software_name)
    {
        putchar('\t'); ++chpr;
        chpr += printf("Software = \"%s\"",Software_name);
        chpr = newline(1);
    }
    putchar('\t'); ++chpr;
    chpr += printf("Note version = %d (tagset = %d)",maker_scheme->note_version,maker_scheme->tagset);

    POPCOLOR();
    chpr = newline(chpr);
    chpr = newline(1);
}

/* Print a list of camera makes known to the program. If a global     */
/* matching pattern has been set for names (by option) , limit the    */
/* output to camera names for which a case-insensitive substring      */
/* match occurs, otherwise list all makes. For each make printed,     */
/* print the list of recognized models. The Make and Model names (in  */
/* the camera_names.h and "maker"_datadefs.h files) are. or should    */
/* be, the "official" names used by the maker in the Make and Model   */
/* entries of and TIFF IFD or CIFF Make/Model entry.                  */

void
print_camera_makes()
{
    struct camera_name *maker_id;
    char *pattern,*model_pattern;
    int chpr = 0;

    chpr = newline(0);
    chpr += printf("Camera Makes:");
    chpr = newline(chpr);

    if(Camera_name_pattern)
    {
        pattern = Camera_name_pattern;
        if((model_pattern = strchr(pattern,'+')))
            *model_pattern++ = '\0';
        else
            model_pattern = NULL;

        /* 'Camera_make' is a global parameter                        */
        for(maker_id = Camera_make; maker_id && maker_id->name; ++maker_id)
        {
            if(pattern)
            {
                if(*pattern == '-')
                {
                    chpr += printf("\t%-32.32s [%d]",maker_id->name,maker_id->id);
                    chpr = newline(chpr);
                }
                else
                {
                    if((*pattern) && !STRCASESTR(maker_id->name,pattern))
                        continue;
                    chpr += printf("\t%-32.32s [%d]",maker_id->name,maker_id->id);
                    chpr = newline(chpr);
                    if(model_pattern)
                    {
                        if(*model_pattern)
                            print_camera_models(maker_id->model,model_pattern);
                        else
                            print_camera_models(maker_id->model,NULL);
                    }
                }
            }
        }
    }
}

/* Print the list of recognized models for given device make. If a    */
/* non-null pattern is passed as argument, limit the output to models */
/* for which a case-insensitive substring match to the pattern occurs */
/* in the name, otherwise print all names.                            */

void
print_camera_models(struct camera_id *model_names,char *pattern)
{
    struct camera_id *maker_id;
    int chpr = 0;

    chpr = newline(0);
    for(maker_id = model_names ; maker_id && maker_id->name; ++maker_id)
    {
        if(pattern)
        {
            if(STRCASESTR(maker_id->name,pattern))
            {
                chpr += printf("\t\t%-32.32s ",maker_id->name);
                chpr += printf("\t[%2d,%d]",maker_id->id,
                                maker_id->noteversion);
                chpr = newline(chpr);
            }
        }
        else
        {
            chpr += printf("\t\t%-32.32s ",maker_id->name);
            chpr += printf("\t[%2d,%d]",maker_id->id,
                                maker_id->noteversion);
            chpr = newline(chpr);
        }
    }
}

char *
fake_strcasestr(char *string,char *pattern)
{
    char *match = CNULL;
    int plen,slen;

    if(string && pattern)
    {
        plen = strlen(pattern);
        if(plen == 0)
            match = string;
        else
        {
            slen = strlen(string);
            while(plen <= slen)
            {
                if(strncasecmp(string,pattern,plen) == 0)
                {
                    match = string;
                    break;
                }
                ++string;
                --slen;
            }
        }
    }
    return(match);
}
