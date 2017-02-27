/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: maker_generic.c,v 1.40 2005/07/24 21:34:37 alex Exp $";
#endif

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* "Generic" routines for makernotes which use TIFF IFD structure     */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "defs.h"
#include "summary.h"
#include "maker.h"
#include "datadefs.h"
#include "maker_datadefs.h"
#include "camera_names.h"
#include "misc.h"
#include "tags.h"
#include "maker_extern.h"
#include "extern.h"

/* Read a makernote at the offset given in the passed maker_entry.    */

/* Checks for data with a TIFF IFD structure at the makernote offset, */
/* possibly offset by a maker name string and/or a new TIFF header    */
/* (e.g. some later Nikons). Notes which pass the test are scanned    */
/* and printed. If the note is in IFD format but the maker is         */
/* unknown, a "bare bones" entry will be printed, with tag numbers,   */
/* type, etc. If the maker is known, tag names, undefined values and  */
/* "interpretations" of the data may be printed. MakerNotes with      */
/* unknown/non-ifd structures could be dispatched to specialized      */
/* routines, but at this time I do not have information on any of     */
/* those. Unknown notes (or notes with invalid IFD entries) can be    */
/* dumped in hex/ascii by user option (-M). 'Undefined" data may be   */
/* similarly dumped by the -U option, if maker-specific routines      */
/* don't handle it.                                                   */

/* The return is the offset of the end of the note; if the note       */
/* cannot be read, return is 0L.                                      */

/* end_of_note: calculated from start offset and 'count' in parent    */
/*              entry                                                 */

/* max_value_offset: maximum offset reached by an entry value which   */
/* begins within the note, as defined by the count. Used...           */

/* max_end_offset: maximum offset reached by data referenced by any   */
/* entry, even if 'outside' the note                                  */

/* value_end_offset: maximum offset reached by the current entry's    */
/* offset value                                                       */

/* max_in_note_tagoffset: the starting offset of the last tag value   */
/* that begins within the note boundaries.                            */

/* min_out_of_note_tagoffset: the start offset of the first tag with  */
/* value which starts beyond the end of the note.                     */

/* The only rule is that thre are no rules...                         */

unsigned long
process_makernote(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *maker_entry,unsigned long fileoffset_base,
                    unsigned long max_offset,struct image_summary *summary_entry,
                    char *parent_name,int indent)
{
    struct ifd_entry *entry_ptr;
    struct maker_scheme *maker_scheme;
    unsigned long maker_offset = 0UL;
    unsigned long maker_count = 0UL;
    unsigned long max_value_offset = 0UL;
    unsigned long max_end_offset = 0UL;
    unsigned long value_end_offset = 0UL;
    unsigned long max_in_note_tagoffset = 0UL;
    unsigned long min_out_of_note_tagoffset = 0UL;
    unsigned long start_offset = 0UL;
    unsigned long dumplength = 0UL;
    unsigned long end_of_note = 0UL;
    unsigned long next_ifd_offset = 0UL;
    unsigned long current_offset;
    unsigned long start_entry_offset;
    unsigned long entry_offset,value_offset;
    char *prefix,*tprefix,*swname;
    char *nameoftag = CNULL;
    char *fulldirname = CNULL;
    int entry_num,num_entries,make,model;
    int tagindent = 0;
    int tagwidth = 0;
    int invalid = 0;
    int scheme_indent = 0;
    int value_is_offset = 0;
    int use_second_pass = 0;
    int chpr = 0;

    if(inptr && maker_entry)
    {
        PUSHCOLOR(MAKER_COLOR);
        if(summary_entry)
            summary_entry->filesubformat |= FILESUBFMT_MNOTE;
        /* Save these; they're in static storage in the maker_entry   */
        /* and will be overwritten when entry reading starts.         */
        maker_offset = maker_entry->value;
        maker_count = maker_entry->count;
        if(maker_count > 4UL)
            end_of_note = maker_offset + maker_count + fileoffset_base;
        else if(maker_count == 4UL)
        {
            /* special case, for certain Casio MakerNotes which use   */
            /* 4-byte UNDEFINED as though it were a LONG offset       */
            if(maker_entry->value_type == UNDEFINED)
                maker_offset = to_ulong((unsigned char *)&maker_entry->value,byteorder);
            /* This pretty much forces maker_scheme to PLAIN          */
            maker_count=0L;
        }
        else
            goto cant_dumpit;

        /* Clear any previous scheme structure                        */
        /* This must be done before model() number is called.         */
        clear_makerscheme();
        /* Get make and model from the global names saved from IFD0   */
        /* Override real names if -m, -l, -s                          */
        if(Use_Make_name)
            make = maker_number(Use_Make_name);
        else
            make = maker_number(Make_name);

        if(Use_Software_name)
            swname = Use_Software_name;
        else
            swname = Software_name;

        /* The camera-specific model number routine may fill in the   */
        /* note_version field in the scheme structure if the model is */
        /* recognized and has more than 1 note version. Unknown       */
        /* models will be tested later or defaulted                   */
        if(Use_Model_name)
            model = model_number(make,Use_Model_name,swname);
        else
            model = model_number(make,Model_name,swname);


        /* start_offset is where the note begins, relative to the     */
        /* start of the file.                                         */
        start_offset = start_entry_offset = maker_offset + fileoffset_base;
        nameoftag = tagname(maker_entry->tag);
        fulldirname = splice(parent_name,".",nameoftag);

        /* The MakerNote should be written within the values section  */
        /* of the Exif ifd, but sometimes is not. 'max_offset', if    */
        /* set, will be the end of the parent ifd (Exif). Mark it to  */
        /* indicate that the MakerNote is out of place.               */
        if(max_offset && (start_offset > max_offset))
            prefix = ">";
        else
            prefix = "@";
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,start_offset,indent,prefix);
            chpr = printf("<%s> length %lu, ",nameoftag,
                        maker_count ? maker_count: maker_entry->count);
        }
        else if(!(PRINT_VALUE_AT_OFFSET))
        {
            tagwidth = MAKERTAGWIDTH;
            if(!(PRINT_LONGNAMES))
                tagwidth -= strlen(nameoftag) + 1;
            /* When SECTIONS are not printed, provide these items as  */
            /* "pseudotags".                                          */
            if((PRINT_TAGINFO) || (PRINT_VALUE))
                print_tag_address(ENTRY,HERE,indent+SMALLINDENT,"*");
            if(PRINT_TAGINFO)
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%s.",nameoftag);
                chpr += printf("%-*.*s",tagwidth,tagwidth,"Offset");
            }
            if((PRINT_VALUE))
                chpr += printf(" = @%lu",start_offset);
            chpr = newline(chpr);
            if((PRINT_TAGINFO) || (PRINT_VALUE))
                print_tag_address(ENTRY,HERE,indent+SMALLINDENT,"*");
            if(PRINT_TAGINFO)
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%s.",nameoftag);
                chpr += printf("%-*.*s",tagwidth,tagwidth,"Length");
            }
            if((PRINT_VALUE))
                chpr += printf(" = %lu",maker_count ? maker_count: maker_entry->count);
            chpr = newline(chpr);
            if((PRINT_TAGINFO) || (PRINT_VALUE))
                print_tag_address(ENTRY,HERE,indent+SMALLINDENT,"*");
            if(PRINT_TAGINFO)
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%s.",nameoftag);
                chpr += printf("%-*.*s",tagwidth,tagwidth,"Scheme");
            }
        }

        /* Discover, if possible, the 'scheme' by which the MakerNote */
        /* is stored.                                                 */
        scheme_indent = charsprinted();
        maker_scheme = find_makerscheme(inptr,maker_offset,fileoffset_base,
                                            byteorder,maker_count,make,model);

        /* If scheme debugging is in effect, the type info below      */
        /* needs to be indented a bit, since the debug output splits  */
        /* the starting <MakerNote> line.                             */
        if(PRINT_SCHEME_DEBUG)
            putindent(scheme_indent + chpr + 2);
        /* Print a brief description of the scheme found. Enclose it  */
        /* in quotes if not in a SECTION line                         */
        if((PRINT_SECTION) || ((PRINT_VALUE) && !(PRINT_VALUE_AT_OFFSET)))
        {
            if(!(PRINT_SECTION))
                chpr += printf(" = '");
            switch(maker_scheme->scheme_type)
            {
                case HAS_ID_SCHEME:
                    if(maker_scheme->offsets_relative_to == START_OF_NOTE)
                        chpr += printf("Note-relative offset ");
                    else if(maker_scheme->offsets_relative_to == START_OF_ENTRY)
                        chpr += printf("Entry-relative offset ");
                    chpr += printf("ID");
                    break;
                case HAS_ID_PLUS_TIFFHDR_SCHEME:
                    chpr += printf("ID+TIFFheader");
                    break;
                case PLAIN_IFD_SCHEME:
                    if(maker_scheme->offsets_relative_to == START_OF_NOTE)
                        chpr += printf("Note-relative offset ");
                    else if(maker_scheme->offsets_relative_to == START_OF_ENTRY)
                        chpr += printf("Entry-relative offset ");
                    else
                        chpr += printf("Plain ");
                    chpr += printf("IFD");
                    if(maker_count == 0) /* some old Casio                */
                        chpr += printf(" (from UNDEFINED offset)");
                    break;
                /* New schemes can be added here.                         */
                case UNKNOWN_SCHEME:
                default:
                    /* dump a couple of lines, unless user says otherwise */
                    next_ifd_offset = 0UL;
                    chpr += printf("unknown");
                    if(PRINT_SECTION)
                        chpr += printf(" scheme");
                    if(PRINT_SECTION && !(PRINT_MAKERDUMP))
                    {
                        chpr = newline(1);
                        if(maker_count < DEF_SHORTDUMP)
                            dumplength = maker_count;
                        else
                            dumplength = DEF_SHORTDUMP;
                        hexdump(inptr,start_offset,maker_count,dumplength,12,
                                    indent,SUBINDENT);
                        if(maker_count > dumplength)
                        {
                            printred(" (use -M to see more)");
                            chpr = newline(1);
                        }
                    }
                    else if((PRINT_VALUE))
                            printf("'");
                    chpr = newline(chpr);
                    if(PRINT_MAKERNOTE_SCHEME)
                        print_scheme(inptr,maker_scheme);
                    goto dumpit;
                    break;
            }
            if(!(PRINT_SECTION))
                chpr = printf("'");
        }
        else
            chpr = newline(chpr);

        /* If printing a value in LIST mode, it's done; if in the     */
        /* middle of a section heading in SECTION mode, there's more  */
        /* heading to print.                                          */
        if(PRINT_SECTION)
            chpr += printf(" scheme,"); 
        else if((PRINT_VALUE))
            chpr = newline(chpr);
        num_entries = maker_scheme->num_entries;
        byteorder = maker_scheme->note_byteorder;
        if((maker_scheme->scheme_type == HAS_ID_PLUS_TIFFHDR_SCHEME) ||
                (maker_scheme->offsets_relative_to == START_OF_NOTE))
        {
            fileoffset_base = maker_scheme->fileoffset_base;
            start_entry_offset = fileoffset_base + maker_scheme->ifd_offset + 2;
        }
        else
            start_entry_offset = start_offset + maker_scheme->ifd_offset + 2;
        if(PRINT_SECTION)
        {
            chpr = printf(" %d entries starting at offset %#lx/%lu",num_entries,
                        start_entry_offset,start_entry_offset);
            chpr = newline(chpr);
        }
        if(PRINT_MAKERNOTE_SCHEME)
            print_scheme(inptr,maker_scheme);

        if((maker_scheme->id_length > 0) && (PRINT_ENTRY))
        {
            print_tag_address(ENTRY,start_offset, indent,prefix);
            extraindent(SUBINDENT);
            if((PRINT_LONGNAMES))
                chpr += printf("%s.%s.",parent_name,nameoftag);
            chpr = printf("%sId",nameoftag);
            if((PRINT_VALUE))
            {
                tagwidth = MAKERTAGWIDTH - strlen(nameoftag) - 2;
                putindent(tagwidth);
                chpr += printf(" = '%s'",maker_scheme->id);
            }
            chpr = newline(chpr);
        }

        if(((maker_scheme->scheme_type == HAS_ID_PLUS_TIFFHDR_SCHEME)) &&
                    (PRINT_SECTION))
        {
            print_tag_address(SECTION,fileoffset_base,indent + SMALLINDENT,prefix);
            if(print_tiff_header(TIFF_MAGIC,maker_scheme->note_byteorder,SECTION) == 0)
            {
                chpr = printf(" ifd offset = %#x/%d)",maker_scheme->ifd_offset,
                                                    maker_scheme->ifd_offset);
                if(fileoffset_base)
                {
                    chpr += printf(" (+ %lu = %#lx/%lu)",fileoffset_base,
                                            fileoffset_base + maker_scheme->ifd_offset,
                                            fileoffset_base + maker_scheme->ifd_offset);
                }
            }
            chpr = newline(chpr);
        }

        /* At this point, the note has been "verified" to be in TIFF  */
        /* IFD format, and can be read normally in that fashion.      */
        /* Maker-specific information is handled in the print_maker   */
        /* routines, according to the noteversion derived from the    */
        /* make and model numbers.                                    */

        /* Scheme validation is not entirely robust, however, so the  */
        /* code below bails out at the first sign of an invalid       */
        /* entry, and hexdumps the data if wanted.                    */

        indent += SMALLINDENT;
        current_offset = entry_offset = start_entry_offset;
        value_is_offset = 0;
        for(entry_num = 0; entry_num < num_entries; ++entry_num)
        {
            print_tag_address(ENTRY,entry_offset,indent,prefix);
            tagindent = charsprinted();  /* in case error msg needed   */
            entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
            if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                                                    ferror(inptr) || feof(inptr))
            {
                ++invalid;
            }
            else
                invalid = 0;
            if(maker_scheme->offsets_relative_to == START_OF_ENTRY)
                fileoffset_base = entry_offset;
            entry_offset = current_offset = ftell(inptr);
            value_offset = print_makerentry(inptr,byteorder,
                                            entry_ptr,fileoffset_base,
                                            summary_entry,fulldirname,
                                            indent,make,model);
            if(invalid > 5)
            {
                if((PRINT_SECTION))
                {
                    PUSHCOLOR(RED);
                    chpr += printf(" TOO MANY INVALID ENTRIES");
                    if((PRINT_MAKERDUMP))
                        chpr += printf(" (hexdump follows)");
                    else
                        chpr += printf(" (Use -M to hex/ascii dump)");
                    POPCOLOR();
                    chpr = newline(chpr);
                }
                else
                {
                    chpr = newline(chpr);
                    PUSHCOLOR(RED);
                    chpr += printf("# WARNING: TOO MANY INVALID MAKERNOTE ENTRIES");
                    chpr += printf(" (note processing aborted)");
                    POPCOLOR();
                    chpr = newline(chpr);
                }
                goto dumpit;
            }
            if(value_offset == 0UL)
                value_offset = entry_offset;
            if(end_of_note)
            {
                if((value_offset < end_of_note) && (value_offset > max_value_offset))
                    max_value_offset = value_offset;
            }
            else if(value_offset > max_value_offset) {
                max_value_offset = value_offset;
            }
            if((is_offset(entry_ptr) || maker_value_is_offset(entry_ptr,make,model)))
                ++use_second_pass;
        }

        /* Some makers write the next ifd offset as 0, as is normal   */
        /* for an IFD. Some write nothing.                            */
        if(maker_scheme->no_nextifd_offset)
        {
            next_ifd_offset = 0L;
            if((PRINT_SECTION) && (PRINT_ENTRY))
            {
                /* Since it isn't needed, just note it and move on.       */
                print_tag_address(SECTION,current_offset,indent,prefix);
                printred("no next IFD offset used");
                chpr = newline(1);
            }
        }
        else
        {
            next_ifd_offset = read_ulong(inptr,byteorder,current_offset);
            if(next_ifd_offset > 0L)
            {
                /* This should not happen, of course.                     */
                if((PRINT_ADDRESS) && (PRINT_VALUE) && (PRINT_VALUE_AT_OFFSET))
                {
                    print_tag_address(ENTRY,current_offset,indent,prefix);
                    chpr += printf("**** next IFD offset %lu  ",next_ifd_offset);
                    next_ifd_offset += fileoffset_base;
                    chpr += printf("(+ %lu = %#lx/%lu)",fileoffset_base,
                                next_ifd_offset,next_ifd_offset);
                    if(next_ifd_offset < ftell(inptr))
                    {
                        printred(" # BAD NEXT IFD OFFSET");
                        next_ifd_offset = 0L;
                    }
                    chpr = newline(chpr);
                }
                else
                    next_ifd_offset += fileoffset_base;
            }
            else
            {
                if((PRINT_ADDRESS) && (PRINT_VALUE) && (PRINT_VALUE_AT_OFFSET))
                {
                    print_tag_address(ENTRY,current_offset,indent,prefix);
                    chpr = printf("**** next IFD offset 0");
                    chpr = newline(chpr);
                }
            }
        }

        if(ferror(inptr))
        {
            chpr = printf(" READ NEXT IFD OFFSET FAILED ");
            why(stdout);
            clearerr(inptr);    /* keep going...                      */
            chpr = newline(1);
        }
        else
            current_offset = ftell(inptr);

        value_offset = current_offset;
        if((max_value_offset == 0) || (current_offset > max_value_offset))
            max_value_offset = current_offset;

        /* Make a second pass over the entries, to print values of    */
        /* entries whose values are indirect.                         */
        if(use_second_pass)
        {

            if((PRINT_VALUE) && (PRINT_ADDRESS) && (PRINT_VALUE_AT_OFFSET))
            {
                print_tag_address(ENTRY,value_offset,indent,prefix);
                chpr = printf("============= VALUES, %s ============",nameoftag);
                chpr = newline(chpr);
            }

            entry_offset = start_entry_offset;
            for(entry_num = 0; entry_num < num_entries; ++entry_num)
            {
                entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
                if(ferror(inptr))   /* why didn't this happen above?  */
                    goto dumpit;
                if(maker_scheme->offsets_relative_to == START_OF_ENTRY)
                    fileoffset_base = entry_offset;
                entry_offset = current_offset = ftell(inptr);
                if(is_offset(entry_ptr) || maker_value_is_offset(entry_ptr,make,model))
                {
                    /* If this is negative, it is assumed to be a     */
                    /* length value which is intended to trigger      */
                    /* processing of the previous entry, which is an  */
                    /* offset (usually of a thumbnail which is        */
                    /* specified by two tags)...so use the previous   */
                    /* value_offset. Don't violate this convention!   */
                    if(maker_value_is_offset(entry_ptr,make,model) >= 0)
                        value_offset = fileoffset_base + entry_ptr->value;
                    if(Debug & 0x10)
                        printf("SL:mvo=%lu,vo=%lu,veo=%lu, en=%lu ### prefix='%s'\n",
                                max_value_offset,value_offset,value_end_offset,end_of_note,prefix);
                    if((value_type_size(entry_ptr->value_type) * entry_ptr->count) <= 4UL)
                        value_end_offset = 0;
                    else
                        value_end_offset = value_offset +
                            (value_type_size(entry_ptr->value_type) * entry_ptr->count);
                    if(value_offset > end_of_note)
                    {
                        if(*prefix == '>')
                            tprefix = "+";
                        else
                            tprefix = ">";
                    }
                    else
                        tprefix = prefix;
                    if(Debug & 0x10)
                        printf("BP:mvo=%lu,vo=%lu,veo=%lu, meo=%lu, en=%lu ### prefix=%s\n",
                                            max_value_offset,value_offset,value_end_offset,
                                                        max_end_offset,end_of_note,prefix);
                    if((PRINT_VALUE_AT_OFFSET))
                    {
                        value_end_offset =
                            print_offset_makervalue(inptr,byteorder,entry_ptr,
                                        fileoffset_base,summary_entry,fulldirname,
                                                    tprefix,indent,make,model,1);
                    }
                    if(Debug & 0x10)
                        printf("AP:mvo=%lu,vo=%lu,veo=%lu, meo=%lu, en=%lu ### prefix=%s\n",
                                            max_value_offset,value_offset,value_end_offset,
                                                        max_end_offset,end_of_note,prefix);
                    /* If the value is a section which STARTS past    */
                    /* the announced end of the makernote it should   */
                    /* not be regarded as part of the makernote, and  */
                    /* should not contribute to the offset returned   */
                    /* from this routine.                             */
                    if(end_of_note)
                    {
                        if((value_offset < end_of_note) && (value_end_offset > max_value_offset))
                        {
                            max_value_offset = value_end_offset;
                            max_in_note_tagoffset = value_offset;
                        }
                    }
                    else if((value_end_offset > max_value_offset))
                            max_value_offset = value_end_offset;
                    /* Keep track of actual maximum; tattle later     */
                    if(value_end_offset > max_end_offset)
                        max_end_offset = value_end_offset;
                    if(end_of_note)
                    {
                        if((value_offset > end_of_note) && (min_out_of_note_tagoffset == 0))
                            min_out_of_note_tagoffset = value_offset;
                    }
                    if(Debug & 0x10)
                        printf("EL:mvo=%lu,vo=%lu,veo=%lu, meo=%lu, en=%lu ### prefix=%s\n",
                                            max_value_offset,value_offset,value_end_offset,
                                                        max_end_offset,end_of_note,prefix);
                }
                current_offset = ftell(inptr);
            }
        }
        else if(current_offset > max_value_offset)
            max_value_offset = current_offset;

        indent -= SMALLINDENT;
        /* If the note is incompletely specified, point that out. If  */
        /* IFD entries are not being printed this message is not      */
        /* wanted.                                                    */
        if((end_of_note && (max_value_offset < end_of_note)) &&
                                        (PRINT_SECTION) && (PRINT_ENTRY) &&
                                            !(LIST_MODE))
        {
            unsigned long printlength = 0;

            print_tag_address(ENTRY,max_value_offset - 1,indent + SMALLINDENT,
                                                                "*");
            chpr = printf("---- End of values before end of %s",nameoftag);
            chpr = newline(chpr);
            dumplength = end_of_note - max_value_offset;
            /* dump at least the first couple of rows, just to see... */
            if(PRINT_MAKERDUMP)
            {
                if((Max_makerdump == DUMPALL) || (Max_makerdump > dumplength))
                    printlength = dumplength;
                else if(Max_makerdump > 0L)
                    printlength = Max_makerdump;
                else if(Max_undefined > 0L)
                    printlength = Max_undefined > dumplength ? Max_undefined : dumplength;
            }
            else    /* always print a little bit in this case         */
            {
                if(dumplength > DEF_SHORTDUMP)
                    printlength = DEF_SHORTDUMP;
                else
                    printlength = dumplength;
            }
            hexdump(inptr,max_value_offset,dumplength,printlength,12,
                            indent,SUBINDENT);
            if(dumplength > printlength)
                printred(" (use -M to see more)");
            chpr = newline(1);
        }
        else if(maker_count == 0L)
        {
            end_of_note = max_value_offset - 1;
            maker_count = end_of_note - start_offset;
            if(!(PRINT_SECTION) && !(PRINT_VALUE_AT_OFFSET) && (PRINT_ENTRY))
            {
                tagwidth = MAKERTAGWIDTH;
                if(!(PRINT_LONGNAMES))
                    tagwidth -= strlen(nameoftag) - 1;
                /* rewrite this pseudotag, since the first one was    */
                /* wrong                                              */
                chpr = newline(chpr);
                if((PRINT_TAGINFO) || (PRINT_VALUE))
                    print_tag_address(ENTRY,HERE,indent+SMALLINDENT,prefix);
                if(PRINT_TAGINFO)
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",parent_name);
                    chpr += printf("%s.",nameoftag);
                    chpr += printf("%-*.*s",tagwidth,tagwidth,"Length");
                }
                if((PRINT_VALUE))
                    chpr += printf(" = %lu",maker_count);
            }
        }
        if(Debug & 0x10)
            printf("END: mvo=%lu, en=%lu, meo=%lu\n",max_value_offset,end_of_note,max_end_offset);
        if(end_of_note && (max_value_offset > (end_of_note + 1)) && max_in_note_tagoffset)
        {
            print_tag_address(SECTION,max_in_note_tagoffset,indent + SMALLINDENT,"!");
            printred("# WARNING: values extend past end of MakerNote");
            chpr = newline(chpr);
        }
        if(end_of_note && (max_end_offset > (end_of_note + 1)) && min_out_of_note_tagoffset)
        {
            print_tag_address(SECTION,min_out_of_note_tagoffset,indent + SMALLINDENT,"!");
            printred("# WARNING: values referenced outside of MakerNote");
            chpr = newline(chpr);
        }

        print_tag_address(SECTION,end_of_note - 1,indent,prefix);
        if(PRINT_SECTION)
            chpr = printf("</%s>",nameoftag);
        if(next_ifd_offset)
        {
            if((PRINT_SECTION))
                chpr += printf(" ");
            PUSHCOLOR(RED);
            chpr = printf("# WARNING! MakerNote has next ifd offset %lu",next_ifd_offset);
            POPCOLOR();
        }
        chpr = newline(chpr);
        POPCOLOR();
        if(fulldirname)
            free(fulldirname);
        fulldirname = CNULL;
    }
    /* The offset passed back to the caller may be higher than the    */
    /* offset written above for the end of the note, if values are    */
    /* referenced past advertised end of note in a chunk that began   */
    /* before end of note.                                            */
    if(max_value_offset > end_of_note)
        end_of_note = max_value_offset;
    return(end_of_note);

dumpit:
    if(PRINT_MAKERDUMP)
    {
        /* If no specific length is given for makernote data, limit   */
        /* the dump length to the length specified for undefined      */
        /* data.                                                      */
        if(Max_makerdump == DUMPALL)
            dumplength = maker_count;
        else if(Max_makerdump > 0L)
        {
            dumplength = Max_makerdump;
            /* In case the maker_count is botched, dump as much as    */
            /* asked                                                  */
            if(maker_count < dumplength)
                maker_count = dumplength;
        }
        else
            dumplength = Max_undefined;
        if(dumplength > 0L)
        {
            chpr = newline(chpr);
            hexdump(inptr,start_offset,maker_count,dumplength,12,
                    indent,SUBINDENT);
            chpr = newline(1);
        }
    }
    if(PRINT_SECTION)
    {
        print_tag_address(SECTION,end_of_note - 1,indent,"-");
        chpr = printf("</MakerNote>");
    }
    POPCOLOR();
    chpr = newline(chpr);
    clearerr(inptr);
    if(maker_count)
        return(end_of_note);
    return(0L);
cant_dumpit:
    clearerr(inptr);
    chpr = printf(" INVALID MakerNote offset %lu",maker_offset);
    chpr = newline(chpr);
    return(0L);
}

/* Return a unique number for a given maker, (assigned in "maker.h")  */
/* based upon a string, usually taken from a "Make" tag. This "magic  */
/* number" will be used to select routines to print tagnames and to   */
/* interpret data from MakerNotes for that maker.                     */

int
maker_number(char *make)
{
    struct camera_name *maker_id;
    int number = NO_MAKE;

    if(make)
    {
        /* 'Camera_make' is a global parameter                            */
        for(maker_id = Camera_make; maker_id && maker_id->name; ++maker_id)
        {
            if(strncasecmp(make,maker_id->name,maker_id->namelen) == 0)
            {
                number = maker_id->id;
                break;
            }
        }
    }
    return(number);
}

/* Call a maker-specific routine to assign an identifying number for  */
/* a specific model of a given maker's camera. The camera make is     */
/* identified by the number assigned by make_number(), and is used to */
/* dispatch the maker-specific routine, which is then free to pick an */
/* identifying number based upon the "Model" tag and possibly the     */
/* "Software" tag. The numbers are assigned in "maker.h". Model       */
/* numbers are used only to determine 'quirks' in some notes which    */
/* cannot reliably be discovered by examining the note scheme,        */

/* The softwarename field has proven unreliable for any purpose, is   */
/* not used anywhere in the program (other than to print it), and     */
/* could disappear from these argument lists. Some image edit         */
/* software will change this field, some won't; in any case,          */
/* maker-supplied versions do not appear to tell us much.             */

int
model_number(int make,char *modelstring,char *softwarename)
{
    int number = NO_MODEL;

    if(modelstring == NULL)
        modelstring = NULLSTRING;
    if(softwarename == NULL)
        softwarename = NULLSTRING;
    switch(make)
    {
        case MAKER_AGFA:
            number = agfa_model_number(modelstring,softwarename);
            break;
        case MAKER_ASAHI:   /* PENTAX                                 */
        case MAKER_PENTAX:  /* PENTAX                                 */
            number = asahi_model_number(modelstring,softwarename);
            break;
        case MAKER_CANON:
            number = canon_model_number(modelstring,softwarename);
            break;
        case MAKER_CASIO:
            number = casio_model_number(modelstring,softwarename);
            break;
        case MAKER_EPSON:
            number = epson_model_number(modelstring,softwarename);
            break;
        case MAKER_FUJIFILM:
            number = fujifilm_model_number(modelstring,softwarename);
            break;
        case MAKER_HP:      /* proprietary makernotes                 */
            number = hp_model_number(modelstring,softwarename);
            break;
        case MAKER_KODAK:   /* proprietary makernotes                 */
            number = kodak_model_number(modelstring,softwarename);
            break;
        case MAKER_KONICA:  /* proprietary notes: old Konica models   */
            number = konica_model_number(modelstring,softwarename);
            break;
        case MAKER_KYOCERA:
            number = kyocera_model_number(modelstring,softwarename);
            break;
        case MAKER_LEICA:
            number = leica_model_number(modelstring,softwarename);
            break;
        case MAKER_MINOLTA:
            number = minolta_model_number(modelstring,softwarename);
            break;
        case MAKER_NIKON:
            number = nikon_model_number(modelstring,softwarename);
            break;
        case MAKER_OLYMPUS:
            number = olympus_model_number(modelstring,softwarename);
            break;
        case MAKER_PANASONIC:
            number = panasonic_model_number(modelstring,softwarename);
            break;
        case MAKER_RICOH:    /* no or non-IFD makernote data          */
            number = ricoh_model_number(modelstring,softwarename);
            break;
        case MAKER_SANYO:
            number = sanyo_model_number(modelstring,softwarename);
            break;
        case MAKER_SIGMA:
            number = sigma_model_number(modelstring,softwarename);
            break;
        case MAKER_SONY:    /* no makernote data                      */
            /* DSC-V3 has makernote in IFD format, but nothing is     */
            /* known about meaning (there are no maker-specific       */
            /* routines here yet)                                     */
            number = sony_model_number(modelstring,softwarename);
            break;
        case MAKER_TOSHIBA:
            number = toshiba_model_number(modelstring,softwarename);
            break;
        case MAKER_TRAVELER:    /* proprietary makernote              */
            number = traveler_model_number(modelstring,softwarename);
            break;
        default:    /* NO_MODEL                                       */
            break;
    }
    return(number);
}

/* ********************************************************************/
/* Attempt to discover the format of a MakerNote. Some notes are in   */
/* normal TIFF IFD format, with tags assigned by the maker, and       */
/* values interpreted in maker-specific ways. Some makers begin with  */
/* a few bytes containing an ascii string identifying the maker, with */
/* a normal IFD following at a specific offset. One camera I've seen  */
/* begins with such an ascii id, followed by an actual TIFF header,   */
/* with the following IFD offsets then relative to that TIFF header.  */
/* This latter scheme seems the most sensible, since it allows image  */
/* editting software to move the MakerNote with little effort, and    */
/* allows standard TIFF machinery to read them.                       */

/* Within the three schemes above, all of which include the basic     */
/* format of a TIFF Image File Directory, makers have devised several */
/* ways to express their individuality.                               */

/* The most harmless is the length of the ID string, and the          */
/* resulting offset from the start of the MakerNote blob to the start */
/* of the actual IFD. Some makers prove their independence by writing */
/* "offset" entries in the IFD (required when a value won't fit in    */
/* the 12-byte entry) relative to the start of the MakerNote or even  */
/* the start of the current entry. The TIFF specification requires    */
/* that such offsets are relative to the beginning of the last        */
/* in-scope TIFF header.                                              */

/* At least one maker thumbs it's nose at the software world by       */
/* gratuitously changing the byte order used when writing the         */
/* MakerNote, compared to the remainder of the image file.            */

/* Some makers write MakerNotes, but in a purely private, proprietary */
/* format.  We cannot read those, except as a blob of useless junk.   */

/* This routine uses some simple heuristics to attempt to intuit the  */
/* format of a note, rather than a table of known make/model formats. */
/* If an Id string is found, an attempt is made to find a TIFF header */
/* or an apparently valid IFD. If found, the offset from the start of */
/* the blob is recorded, and if the byteorder is changed, that is     */
/* noted as well                                                      */

/* Offset to the start of the IFD and byteorder are sufficient to     */
/* read the directory entries (and print them) but the reference      */
/* point for offsets to indirect data is difficult to determine if it */
/* differs from the TIFF specification. Specific information,         */
/* according to camera make and model, is added for cameras which are */
/* known to write note- or entry-relative offsets. Some cameras also  */
/* fail to write zeros for the "next IFD offset" normally included at */
/* the end of a TIFF IFD, so that fact is noted as well.              */

/* If the note just doesn't look presentable, a (currently dummy)     */
/* routine can be used to check for known sports.                     */

/* The information recorded in the maker_scheme structure at this     */
/* point is sufficient to read and print the structure of the note    */
/* (except for UNKNOWN schemes, of course). Interpreting the          */
/* *meaning* of the tags and values is the responsibility of          */
/* maker-specific routines.                                           */
/* ********************************************************************/

/* ********************************************************************/
/* The maker_scheme structure may be of interest to maker-specific    */
/* routines, but is not passed around, and should not be modified     */
/* outside find_makerscheme. retrieve_makerscheme is provided to      */
/* allow a *copy* of the structure to be retrieved after it has been  */
/* filled.                                                            */
/* ********************************************************************/

/* Allow only find_makerscheme() to muck with the original scheme.    */
/* Spcific routines are provided to alter note_version and/or tagset  */
/* from anywhere in the program, although this is not currently       */
/* required.                                                          */

static struct maker_scheme maker_scheme;

/* Clear the structure and mark it clear for process_makernote()      */

void
clear_makerscheme()
{
    memset(&maker_scheme,'\0',sizeof(struct maker_scheme));
    maker_scheme.make = -1;
}

struct maker_scheme *
find_makerscheme(FILE *inptr,unsigned long note_offset,
                    unsigned long fileoffset_base,
                    unsigned short byteorder,
                    unsigned long maker_size,int make,int model)
{
    struct ifd_entry *ifdentry;
    int length_read,id_length,num_entries,start,maxchk_offset;
    int tiffhdr_offset = 0;
    unsigned long note_fileoffset = 0L;
    unsigned long num_entry_offset = 0L;
    unsigned long ifd_offset = 0L;
    unsigned long scheme_type = UNKNOWN_SCHEME;
    unsigned short alt_byteorder;
    unsigned short note_byteorder = byteorder;
    int chpr = 0;
    char *id;

    if(maker_scheme.make >= 0)
        clear_makerscheme();
    note_fileoffset = note_offset + fileoffset_base;

    id= (char *)read_bytes(inptr,MAX_IDLEN,note_fileoffset);
    length_read = id ? strlen(id) : 0;

    /* check that string is printable, adjust id_length if nexessary  */
    if(PRINT_SCHEME_DEBUG)
    {
        PUSHCOLOR(HI_RED);
        PUSHCOLOR(BG_WHITE);
        chpr = newline(1);
        chpr += printf("Scheme debug:");
        chpr = newline(chpr);
        chpr += printf("Initial id length = %d, ID=",length_read);
        chpr += print_ascii(inptr,length_read,note_fileoffset);
        chpr += printf(" at file offset %lu",note_fileoffset);
        chpr = newline(chpr);
    }

    /* Check that the string is printable, and truncate at the first  */
    /* non-printable found                                            */
    id = check_printable(id,MAX_IDLEN);
    id_length = strlen(id);
    if((PRINT_SCHEME_DEBUG) && (id_length != length_read))
    {
        chpr += printf("\tchecked id length = %d",id_length);
        chpr = newline(chpr);
    }
    switch(id_length)
    {
        case -1:    /* read failed                                    */
        case 0:     /* normal or unknown                              */
        case 1:
        case 2:
            /* ###% allow 1 and 2 to be checked?                      */
            scheme_type = PLAIN_IFD_SCHEME;
            note_byteorder = byteorder;
#if 0
/* This is unwise                                                     */
            if(PRINT_SCHEME_DEBUG)
            {
                chpr += printf(" force PLAIN IFD with no offset");
                chpr = newline(chpr);
                chpr += printf(" entry check offset 0: ");
            }
            num_entry_offset = note_fileoffset;
            num_entries = read_ushort(inptr,byteorder,num_entry_offset);
            if(((num_entries > 0) && (maker_size > 0L) &&
                (num_entries <= (maker_size / 12))) &&
                    (invalid_ifd_entry(inptr,num_entry_offset + 2,
                                                maker_size,num_entries,
                                                        byteorder) == 0))
            {
                if(PRINT_SCHEME_DEBUG)
                {
                    chpr += printf(" FOUND entry, num_entries=%d",num_entries);
                    chpr = newline(chpr);
                }
                note_byteorder = byteorder;
            }
            else
            {
                if(PRINT_SCHEME_DEBUG)
                {
                    chpr += printf(", num_entries=%d\t(INVALID)",num_entries);
                    chpr = newline(chpr);
                    chpr += printf(" alt byteorder entry check offset 0: ");
                }
                if(byteorder == TIFF_MOTOROLA)
                    alt_byteorder = TIFF_INTEL;
                else
                    alt_byteorder = TIFF_MOTOROLA;
                num_entries = read_ushort(inptr,alt_byteorder,num_entry_offset);
                if(((num_entries > 0) && (maker_size > 0L) &&
                    (num_entries <= (maker_size / 12))) &&
                    (invalid_ifd_entry(inptr,num_entry_offset + 2,maker_size,num_entries,alt_byteorder) == 0))
                {
                    if(PRINT_SCHEME_DEBUG)
                    {
                        chpr += printf(" num_entries=%d, VALID entry found, byteorder=",num_entries);
                        print_byteorder(alt_byteorder,1);
                        chpr = newline(chpr);
                    }
                    note_byteorder = alt_byteorder;
                }
                else if(PRINT_SCHEME_DEBUG)
                {
                    chpr += printf(" , num_entries=%d\t(INVALID)",num_entries);
                    chpr = newline(chpr);
                }
            }
#endif
            break;
        default:
            ifd_offset = 0L;
            start = ((id_length + 3) * 2) / 2;
            maxchk_offset = start + 8;
            /* look for TIFF header first.                            */
            /* At least one maker uses odd offsets, so we can't       */
            /* increment by two                                       */
            for( ; start < maxchk_offset; ++start)
            {
                int print_marker = 0;

                if(PRINT_SCHEME_DEBUG)
                {
                    chpr += printf(" header check offset %d",start);
                    print_marker++;
                }

                alt_byteorder = valid_tiffhdr(inptr,note_fileoffset + start,print_marker);
                if(alt_byteorder)
                {
                    scheme_type = HAS_ID_PLUS_TIFFHDR_SCHEME;
                    tiffhdr_offset = start;
                    fileoffset_base = note_fileoffset + tiffhdr_offset;
                    ifd_offset = read_ulong(inptr,alt_byteorder,HERE);
                    note_byteorder = alt_byteorder;
                    if(PRINT_SCHEME_DEBUG)
                    {
                        chpr += printf(" found TIFF header af file offset %lu",fileoffset_base);
                        chpr = newline(chpr);
                        chpr += printf(" note_fileoffset = %lu, tifhdr_offset = %u, ifd_offset = %lu",
                                note_fileoffset,tiffhdr_offset,ifd_offset);
                        chpr = newline(chpr);
                    }
                    break;
                }
                else if(PRINT_SCHEME_DEBUG)
                {
                    chpr += printf("\t (NO VALID MARKER)");
                    chpr = newline(chpr);
                }
            }

            if(ifd_offset == 0)     /* No TIFF header                 */
            {
                start = ((id_length + 1) * 2) / 2;
                for( ; start < maxchk_offset; ++start)
                {
                    if(PRINT_SCHEME_DEBUG)
                        chpr += printf(" entry check offset %d",start);
                    num_entry_offset = note_fileoffset + start;
                    num_entries = read_ushort(inptr,byteorder,num_entry_offset);
                    if(((num_entries > 0) && (maker_size > 0L) &&
                        (num_entries <= (maker_size / 12))) &&
                            (invalid_ifd_entry(inptr,num_entry_offset + 2,
                                                maker_size,num_entries,
                                                    byteorder) == 0))
                    {
                        if(PRINT_SCHEME_DEBUG)
                        {
                            chpr += printf(" FOUND entry, num_entries=%d",num_entries);
                            chpr = newline(chpr);
                        }
                        scheme_type = HAS_ID_SCHEME;
                        ifd_offset = start;
                        note_byteorder = byteorder;
                        break;
                    }
                    else if(PRINT_SCHEME_DEBUG)
                    {
                        chpr += printf(" , num_entries=%d\t(INVALID)",num_entries);
                        chpr = newline(chpr);
                    }
                }
            }

            if(ifd_offset == 0)     /* Try the other byte order       */
            {
                if(byteorder == TIFF_MOTOROLA)
                    alt_byteorder = TIFF_INTEL;
                else
                    alt_byteorder = TIFF_MOTOROLA;
                start = ((id_length + 3) * 2) / 2;
                for( ; start < maxchk_offset; ++start)
                {
                    if(PRINT_SCHEME_DEBUG)
                        chpr += printf(" alt byteorder entry check offset %d",start);
                    num_entry_offset = note_fileoffset + start;
                    num_entries = read_ushort(inptr,alt_byteorder,num_entry_offset);
                    if(((num_entries > 0) && (maker_size > 0L) &&
                        (num_entries <= (maker_size / 12))) &&
                        (invalid_ifd_entry(inptr,num_entry_offset + 2,maker_size,num_entries,alt_byteorder) == 0))
                    {
                        if(PRINT_SCHEME_DEBUG)
                        {
                            chpr += printf(" num_entries=%d, VALID entry found, byteorder=",num_entries);
                            print_byteorder(alt_byteorder,1);
                            chpr = newline(chpr);
                        }
                        note_byteorder = alt_byteorder;
                        scheme_type = HAS_ID_SCHEME;
                        ifd_offset = start;
                        break;
                    }
                    else if(PRINT_SCHEME_DEBUG)
                    {
                        chpr += printf(" , num_entries=%d\t(INVALID)",num_entries);
                        chpr = newline(chpr);
                    }
                }
            }
            break;
    }

    if(PRINT_SCHEME_DEBUG)
    {
        chpr = newline(chpr);
        chpr += printf(" reading num_entries at offset %lu, byteorder=",
                    note_fileoffset + tiffhdr_offset + ifd_offset);
        print_byteorder(note_byteorder,1);
        chpr = newline(chpr);
    }
    if(ifd_offset >= 0)
    {
        num_entries = read_ushort(inptr,note_byteorder,note_fileoffset + tiffhdr_offset + ifd_offset);
        /* read and save first entry (in scheme structure) as an aid  */
        /* to selecting version of note processor to use for unknown  */
        /* models                                                     */
        /* ### this probably shouldn't be done until the scheme has   */
        /* been validated.  It's not used much anyway.                */
        /* ###%%% better yet, read it here and pass it to             */
        /* invalid_ifd_entry() rather than reading it again there.    */
        ifdentry = read_ifd_entry(inptr,note_byteorder,HERE);
        memcpy(&(maker_scheme.first_entry),ifdentry,sizeof(struct ifd_entry));
    }
    else
        num_entries = 0;    /* read failed */

    if(PRINT_SCHEME_DEBUG)
    {
        chpr += printf(" num_entries=%d",num_entries);
        chpr = newline(chpr);
    }

    
    if(((maker_size > 0L) && (num_entries > (maker_size / 12))) ||
        invalid_ifd_entry(inptr,note_fileoffset + tiffhdr_offset + ifd_offset + 2,
            maker_size,num_entries,note_byteorder))
    {
        /* Record what we know                                        */
        maker_scheme.make = make;
        maker_scheme.model = model;
        maker_scheme.note_version = -1;
        maker_scheme.scheme_type = UNKNOWN_SCHEME;
        maker_scheme.note_fileoffset = note_fileoffset;
        maker_scheme.note_length = maker_size;
        maker_scheme.id = id;   /* ...could be one                    */
        maker_scheme.id_length = id_length;
        maker_scheme.tiffhdr_offset = 0L;
        maker_scheme.ifd_offset = 0L;
        maker_scheme.fileoffset_base = fileoffset_base;
        maker_scheme.num_entries = 0;
        maker_scheme.offsets_relative_to = 0;
        maker_scheme.no_nextifd_offset = 1;
        maker_scheme.tiff_byteorder = byteorder;
        maker_scheme.note_byteorder = 0;
        find_maker_model(&maker_scheme,make,model);
        /* If find_maker_model() is changed to succeed at anything,   */
        /* this will have to change...                                */
        if(PRINT_SCHEME_DEBUG)
        {
            PUSHCOLOR(GREEN);
            chpr = printf(" No valid scheme found ");
            POPCOLOR();
            POPCOLOR();
        }
    }
    else
    {
        if(PRINT_SCHEME_DEBUG)
        {
            PUSHCOLOR(GREEN);
            chpr = printf(" Found valid scheme ");
            POPCOLOR();
            POPCOLOR();
        }
        /* We seem to have a winner...                                */
        if(scheme_type == UNKNOWN_SCHEME)
            maker_scheme.scheme_type = PLAIN_IFD_SCHEME;
        else
            maker_scheme.scheme_type = scheme_type;
        maker_scheme.make = make;
        maker_scheme.model = model;
        /* note_version may already be set by maker_number()          */
        maker_scheme.note_fileoffset = note_fileoffset;
        maker_scheme.note_length = maker_size;
        maker_scheme.id = id;
        maker_scheme.id_length = id_length;
        maker_scheme.ifd_offset = ifd_offset;
        maker_scheme.tiffhdr_offset = tiffhdr_offset;
        maker_scheme.num_entries = num_entries;
        maker_scheme.no_nextifd_offset = 0;
        maker_scheme.offsets_relative_to = START_OF_TIFFHEADER;
        maker_scheme.fileoffset_base = fileoffset_base;

        /* The two entries allow print_scheme() to make it plain when */
        /* the note byte order gratuitously differs from the rest of  */
        /* the file.                                                  */
        maker_scheme.tiff_byteorder = byteorder;
        maker_scheme.note_byteorder = note_byteorder;

        /* Maker-specific adjustments                                 */
        switch(make)
        {
            case MAKER_AGFA:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_ASAHI: /* fall through */
            case MAKER_PENTAX:
                if(maker_scheme.scheme_type == PLAIN_IFD_SCHEME)
                    maker_scheme.offsets_relative_to = START_OF_ENTRY;
                if((maker_scheme.note_version == 0))
                    (void)set_asahi_noteversion();
                break;
            case MAKER_CANON:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_CASIO:
                if((maker_scheme.note_version == 0))
                    (void)set_casio_noteversion();
                break;
            case MAKER_EPSON:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_FUJIFILM:
                maker_scheme.offsets_relative_to = START_OF_NOTE;
                maker_scheme.fileoffset_base = note_fileoffset;
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_HP:      /* proprietary makernote              */
                if((maker_scheme.note_version == 0))
                    setnoteversion(-1);
                break;
            case MAKER_KODAK:   /* proprietary makernote              */
                if((maker_scheme.note_version == 0))
                    setnoteversion(-1);
                break;
            case MAKER_KONICA:  /* no makernote data                  */
                if((maker_scheme.note_version == 0))
                    setnoteversion(-1);
                break;
            case MAKER_LEICA:
                if(strncasecmp(id,"FUJIFILM",8) == 0)
                {
                    maker_scheme.offsets_relative_to = START_OF_NOTE;
                    maker_scheme.fileoffset_base = note_fileoffset;
                    /* maker_scheme.no_nextifd_offset = 1; */
                }
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_MINOLTA:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_KYOCERA:
                maker_scheme.offsets_relative_to = START_OF_ENTRY;
                maker_scheme.no_nextifd_offset = 1;
                break;
            case MAKER_NIKON:
                if((maker_scheme.note_version == 0))
                    (void)set_nikon_noteversion();
                break;
            case MAKER_OLYMPUS:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_PANASONIC:
                maker_scheme.no_nextifd_offset = 1;
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_TOSHIBA:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                maker_scheme.no_nextifd_offset = 1;
                break;
            case MAKER_RICOH:    /* no or non-IFD makernote data      */
                if((maker_scheme.note_version == 0))
                    setnoteversion(-1);
                break;
            case MAKER_SANYO:
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_SIGMA:
                if(model == SIGMA_SD9)
                    maker_scheme.no_nextifd_offset = 1;
                if((maker_scheme.note_version == 0))
                    setnoteversion(1);
                break;
            case MAKER_SONY:
                if((maker_scheme.note_version == 0))
                    setnoteversion(-1);
                maker_scheme.no_nextifd_offset = 1;
                break;
            case MAKER_TRAVELER:    /* proprietary makernote          */
                if((maker_scheme.note_version == 0))
                    setnoteversion(-1);
                break;
            default:
                break;
        }
    }
    if(PRINT_SCHEME_DEBUG)
    {
        POPCOLOR();
        chpr = newline(chpr);
    }
    return(&maker_scheme);
}

/* Look up a make and model in a table of known sports. This is       */
/* currently just a dummy, but might be used to handle non-ifd notes  */
/* by setting a new type here and revising the generic code to        */
/* dispatch a specialized routine. I have no information on such      */
/* notes now.                                                         */

void
find_maker_model(struct maker_scheme *maker_scheme,int make,int model)
{
        /* do a lookup here on make and model                         */
        /* This should probably return a scheme structure, or some    */
        /* indication of success or failure, but since it's not used  */
        /* now (except to initialize the structure)...                */
}

/* Check for a valid TIFF header at the argument offset               */

unsigned short
valid_tiffhdr(FILE *inptr,unsigned long offset,int print_marker)
{
    struct fileheader *header;
    unsigned short marker = 0;
    int chpr = 0;

    if(inptr && offset)
    {
        if(print_marker)
            chpr = printf(" read at offset %lu ",offset);
        header = read_imageheader(inptr,offset);
        if(header)
        {
            if(print_marker)
                chpr += printf(" %#x ",header->file_marker);
            switch(header->probe_magic)
            {
                case TIFF_MAGIC:
                    marker = header->file_marker;
                    break;
                default:
                    marker = 0;
                    break;
            }
        }
    }
    setcharsprinted(chpr);
    return(marker);
}

/* Check for an apparently valid TIFF IFD entry at the argumrnt       */
/* offset.                                                            */

int
invalid_ifd_entry(FILE *inptr,unsigned long offset,unsigned long segment_size,
                    int nentries,unsigned short byteorder)
{
    struct ifd_entry *entry_ptr;
    int invalid = 0;

    if(inptr && offset && byteorder)
    {
        /* Read an entry at the given offset and see if has a valid   */
        /* type, and non-zero count.                                  */
        entry_ptr = read_ifd_entry(inptr,byteorder,offset);
        if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
            ferror(inptr) || feof(inptr))
                ++invalid;
        else if(entry_ptr->count == 0)
                ++invalid;
        
        /* If there is more than one entry, and the first appears     */
        /* valid, check the second entry.                             */
        /* If there is only one entry, and it appears valid, make     */
        /* sure that it's size fits within the note. This size check  */
        /* can fail on multi-entry notes when moronic makers write a  */
        /* thumbnail offset, then write the thumbnail outside the     */
        /* note...so we don't check size for multi.                   */
        if(nentries > 1)
        {
            if(invalid == 0)    /* double check                           */
            {
                entry_ptr = read_ifd_entry(inptr,byteorder,HERE);
                if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                    ferror(inptr) || feof(inptr))
                        ++invalid;
                else if(entry_ptr->count == 0)
                        ++invalid;
            }
        }
        else if(entry_ptr->count > segment_size)
            ++invalid;
    }
    else
        ++invalid;
    return(invalid);
}

/* Call a maker-specific routine to identify LONG values which must   */
/* be interpreted as offsets.                                         */

int
maker_value_is_offset(struct ifd_entry *entry,int make,int model)
{
    int is_offset = 0;

    if(entry)
    {
        switch(make)
        {
            case MAKER_ASAHI:
            case MAKER_PENTAX:
                is_offset = maker_asahi_value_is_offset(entry,model);
                break;
            case MAKER_CANON:
                is_offset = maker_canon_value_is_offset(entry,model);
                break;
            case MAKER_CASIO:
                is_offset = maker_casio_value_is_offset(entry,model);
                break;
            case MAKER_NIKON:
                is_offset = maker_nikon_value_is_offset(entry,model);
                break;
            case MAKER_MINOLTA:
                is_offset = maker_minolta_value_is_offset(entry,model);
                break;
            default:
                break;
        }
    }
    return(is_offset);
}

/* Copy the argument string into static storage, stopping at any      */
/* unprintable byte which may be found.                               */
/* Return the null-terminated result string.                          */

char *
check_printable(char *id, int maxlen)
{
    static char buffer[MAX_IDLEN];
    char *idval,*bufval;
    int i = 0;

    memset(buffer,'\0',MAX_IDLEN);
    if(maxlen > MAX_IDLEN)
        maxlen = MAX_IDLEN;
    bufval = buffer;

    for(idval = id; idval && (i < maxlen - 1); ++idval,++bufval,++i)
        if(!isprint(*idval))
        {
            *bufval = '\0';
            break;
        }
        else
            *bufval = *idval;
    return(buffer);
}

/* Record the "version" of the makernote as recognized by exifprobe.  */
/* Note that the version number is arbitrary and does not correspond  */
/* to any manufacturer numbering scheme; it is used for internal      */
/* identification in order to identify or determine which routine is  */
/* to be used for each model.                                         */

int option_noteversion = 0;

/* This function may be called from the option processing code to     */
/* allow a user to override the noteversion which would otherwise be  */
/* set. This is primarily intended to permit trying an existing       */
/* noteversion on an unrecognized model.                              */

void
setoptionnoteversion(int note_vers)
{
    option_noteversion = note_vers;
}

void
setnoteversion(int note_vers)
{
    if(option_noteversion != 0)
        maker_scheme.note_version = option_noteversion;
    else
        maker_scheme.note_version = note_vers;
}

int
getnoteversion()
{
    if(option_noteversion != 0)
        maker_scheme.note_version = option_noteversion;
    return(maker_scheme.note_version);
}

/* Tagset number is usually the same as note version; however, some   */
/* models may have fewer tagsets than note versions. The two are set  */
/* separately. Tagset number is provided for info/debug only (it is   */
/* not used by the selection code);                                   */

void
setnotetagset(int tagset)
{
    maker_scheme.tagset = tagset;
}

int
getnotetagset()
{
    return(maker_scheme.tagset);
}

/* Provide maker-specific files access to a copy of the scheme.       */

struct maker_scheme *
retrieve_makerscheme()
{
    static struct maker_scheme maker_scheme_copy;

    /* structure assignment                                           */
    maker_scheme_copy = maker_scheme;
    return(&maker_scheme_copy);
}
