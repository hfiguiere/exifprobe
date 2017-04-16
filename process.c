/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002, 2005 by Duane H. Hesser. All rights reserved.  */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: process.c,v 1.47 2005/07/25 22:05:10 alex Exp $";
#endif

/* Process segment types such as TIFF IFD, JPEG APP segments, and     */
/* JPEG basic segments.                                               */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "defs.h"
#include "summary.h"
#include "maker.h"
#include "datadefs.h"
#include "misc.h"
#include "tags.h"
#include "extern.h"
#include "ciff.h"

/* Read and decode a TIFF IFD, attempting to describe the location    */
/* and structure of the data as it is read. Entry values which live   */
/* at an offset from the entry are displayed following the last entry */
/* in the IFD (actually after the "next ifd offset") if               */
/* VALUE_AT_OFFSET is set in Print_options, so that most IFDS will    */
/* have a VALUES section following the entry lines, in just the       */
/* manner that the file is laid out by spec. Tag names for offset     */
/* values are repeated when the value is printed. If VALUE_AT_OFFSET  */
/* is not set, the value is printed immediately, unless the tag       */
/* represents a subsection, in which case the offset of the           */
/* subsection is printed as the tag value, and the subsection handled */
/* in a second pass, after all entries are done.                      */

/* Returns a file offset which represents the "highest" position of   */
/* the file pointer read by this routine and its subroutines, which   */
/* may be used by the caller to read the remainder of the file. This  */
/* number (less one) is printed as the offset of the end of the IFD.  */

/* At the same time, the function notices non-zero values of "next    */
/* ifd offset" which may be contained in this ifd, and passes it as   */
/* "max_offset" to routines which read subifds, on the assumption     */
/* that this ifd should not write into the next ifd. That value is    */
/* used to mark segments which appear to be out of order in the file. */
/* If the routine detects by this means that *this* ifd is outside    */
/* its natural bounds, and cannot be contained within its parent, it  */
/* returns 0, rather than the maximum value offset, so that the       */
/* parent routine will not improperly alter its maximum offset.       */

/* The arguments are the byteorder to be used when reading integer    */
/* data (usually taken from the TIFF header), the offset of the start */
/* of the directory, relative to the TIFF header, the offset of the   */
/* TIFF header from the beginning of the file, the sequence number of */
/* the IFD, and formatting values (indent and address width) to be    */
/* used when printing.                                                */

/* The routine also attempts to record the location, size, image type */
/* and compression method used for actual image data contained within */
/* or referenced by a tiff IFD. This information is printed at the    */
/* end, as a summary of images found in the file.                     */

unsigned long
process_tiff_ifd(FILE *inptr,unsigned short byteorder,unsigned long ifd_offset,
                    unsigned long fileoffset_base,unsigned long max_offset,
                    struct image_summary *summary_entry,char *parent_name,
                    int ifdtype,int ifdnum,int subifdnum,int indent)
{
    struct ifd_entry *entry_ptr = NULL;
    struct image_summary *tmp_summary_entry;
    unsigned long cur_ifd_offset,next_ifd_offset,current_offset;
    unsigned long start_entry_offset,entry_offset,value_offset;
    unsigned long value_start_offset = 0UL;
    unsigned long offset_limit = 0UL;
    unsigned long max_value_offset = 0UL;
    unsigned long max_ifd_offset = 0UL;
    unsigned long thn_offset = 0UL;     /* JPegInterchangeFormat      */
    unsigned long thn_length = 0UL;     /* JpegInterChangeFormat      */
    unsigned long tmp_length = 0L;
    unsigned long alt_length = 0L;
    unsigned long limit_offset = 0L;
    unsigned long filesize = 0UL;
    unsigned short marker,alt_byteorder = 0;
    int invalid_entry = 0;
    int value_is_offset = 0;
    int use_second_pass = 0;
    int status = 0;
    int chpr = 0;
    int entry_num,num_entries,i;
    char dirnamebuf[16];
    char *ifdname,*prefix,*dirname,*nameoftag,*tprefix;
    char *fulldirname = CNULL;
    char *listname = CNULL;

    if(inptr == (FILE *)0)
    {
        fprintf(stderr,"%s: no open file pointer to read TIFF IFD\n",
                Progname);
        return(0L);
    }
    if(Debug & OUT_DEBUG)
    {
        printf("PS=%d,",(Print_options & PRINT_SECTION) > 0);
        printf("PV=%d,",(Print_options & PRINT_VALUE) > 0);
        printf("PO=%d,",(Print_options & PRINT_OFFSET) > 0);
        printf("PVAO=%d\n",(Print_options & PRINT_VALUE_AT_OFFSET) > 0);
    }

    filesize = get_filesize(inptr);
    next_ifd_offset = ifd_offset + fileoffset_base;
    while(next_ifd_offset)
    {
        clearerr(inptr);
        /* max_offset, if set, is the maximum offset which the parent */
        /* is willing to claim. max_ifd_offset is the maximum offset  */
        /* reached by this ifd, which may be constrained by the       */
        /* "next_ifd_offset" of chained ifds.                         */
        cur_ifd_offset = next_ifd_offset;
        if(max_offset && (cur_ifd_offset > max_offset))
            prefix = ">";   /* outside the parent                     */
        else
            prefix = "@";

        print_tag_address(SECTION,cur_ifd_offset,indent,prefix);
        switch(ifdtype)
        {
            case TIFF_IFD:
                if(PRINT_SECTION)
                    chpr += printf("<IFD %d>",ifdnum);
                ifdname = "IFD";
                sprintf(dirnamebuf,"Ifd%d",ifdnum);
                dirname = dirnamebuf;
                break;
            case TIFF_SUBIFD:
                if(PRINT_SECTION)
                    chpr += printf("<SubIFD %d of IFD %d>",subifdnum,ifdnum);
                ifdname = "SubIFD";
                sprintf(dirnamebuf,"SubIfd%d",subifdnum);
                dirname = dirnamebuf;
                break;
            case INTEROP_IFD:
                if(PRINT_SECTION)
                    chpr += printf("<Interoperability SubIFD>");
                ifdname = "Interoperability SubIFD";
                dirname = "Interop";
                break;
            case GPS_IFD:
                if(PRINT_SECTION)
                    chpr += printf("<GPS SubIFD>");
                ifdname = "GPS SubIFD";
                dirname = "Gps";
                break;
            case EXIF_IFD:  /* This shouldn't happen                  */
                if(PRINT_SECTION)
                    chpr += printf("<EXIF IFD>");   
                ifdname = "EXIF IFD";
                dirname = "Exif";
                break;
            case MAKER_SUBIFD:
                ifdname = "MakerNote SubIFD";
                if(PRINT_SECTION)
                    chpr += printf("<%s>",ifdname);
                dirname = CNULL;
                break;
            default:
                if(PRINT_SECTION)
                    chpr += printf("<UNKNOWN IFD TYPE %d>",ifdtype);    /* SECTION */
                ifdname = "UNKNOWN IFD TYPE";
                dirname = CNULL;
                break;
        }
        if(dirname)
            listname = fulldirname = splice(parent_name,".",dirname);
        else
            listname = parent_name;

        num_entries = read_ushort(inptr,byteorder,cur_ifd_offset);
        max_value_offset = max_ifd_offset = 0L;
        if(ferror(inptr) || feof(inptr))
        {
            chpr = newline(chpr);
            printred("#========= WARNING: FAILED to read number of entries for IFD at offset ");
            chpr += printf("%lu",cur_ifd_offset);
            if(ferror(inptr))
                why(stdout);
            chpr = newline(chpr);
            goto blewit;
        }

        /* The file pointer is now at the start of the IFD entries    */
        current_offset = entry_offset = start_entry_offset = ftell(inptr);
        if(PRINT_SECTION)
        {
            chpr += printf(" %d entries ",num_entries);
            chpr += printf("starting at file offset %#lx=%lu",
                                            start_entry_offset,start_entry_offset);
            chpr = newline(chpr);
        }

        if((summary_entry == NULL) || summary_entry->entry_lock)
            summary_entry = new_summary_entry(summary_entry,0,ifdtype);
        else
            summary_entry = last_summary_entry(summary_entry);
        if(summary_entry)
        {
            summary_entry->datatype = ifdtype;
            summary_entry->imageformat = IMGFMT_NOIMAGE;
            summary_entry->filesubformat |= FILESUBFMT_TIFF;
            if((ifdtype == TIFF_IFD) || (ifdtype == TIFF_SUBIFD) || (ifdtype == MAKER_SUBIFD))
            {
                summary_entry->ifdnum = ifdnum;
                summary_entry->subifdnum = subifdnum;
            }
        }
        use_second_pass = value_is_offset = 0;

        indent += SMALLINDENT;
        for(entry_num = 0; entry_num < num_entries; ++entry_num)
        {
            entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);

            if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                ferror(inptr) || feof(inptr))
            {
                print_tag_address(ENTRY,entry_offset,indent,prefix);
                print_taginfo(entry_ptr,listname,SMALLINDENT,ifdtype,ifdnum,subifdnum);
                if((PRINT_ENTRY))
                    printred(" INVALID ENTRY");
                chpr = newline(chpr);

                /* If there are a few invalid entries in an otherwise */
                /* valid IFD, the invalid entries should be reported. */
                /* An invalid IFD, with 50,000 entries or so, all     */
                /* invalid, should be nipped in the bud.              */

                clearerr(inptr);
                current_offset = ftell(inptr);
                if(max_offset > 0)
                    limit_offset = max_offset;
                else
                {
                    if(fseek(inptr,0L,SEEK_END) != -1)
                    {
                        limit_offset = ftell(inptr);
                        fseek(inptr,current_offset,SEEK_SET);
                    }
                }
                /* If there's an error on input, or we can't check    */
                /* for absurd num_entries, give up.                   */
                if(!ferror(inptr) && !feof(inptr) && (limit_offset > 0))
                {
                    /* If the number of entries would read past the   */
                    /* size of the IFD, or past EOF, give up          */
                    if((ifd_offset + (12 * num_entries)) < limit_offset)
                    {
                        /* Limit the number of consecutive failures.  */
                        /* An apparently valid entry resets the count */
                        /* to 0.                                      */
                        if(invalid_entry++ < MAX_INVALID_ENTRIES)
                        {
                            entry_offset = current_offset;
                            clearerr(inptr);
                            continue;
                        }
                    }
                }
                chpr = newline(chpr);
                goto blewit;
            }
            invalid_entry = 0;
            current_offset = ftell(inptr);

            switch(entry_ptr->tag)
            {
                case TIFFTAG_OldSubFileType:   /* old, deprecated        */
                    if(summary_entry)
                    {
                        summary_entry->filesubformat |= FILESUBFMT_TIFFEP;
                        if(entry_ptr->value == 1)
                            summary_entry->subfiletype = PRIMARY_TYPE;
                        else if(entry_ptr->value == 2)
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                        else if(entry_ptr->value == 3)
                            summary_entry->subfiletype = PAGE_TYPE;
                    }
                    break;
                case TIFFTAG_NewSubFileType:
                    if(summary_entry)
                    {
                        summary_entry->filesubformat |= FILESUBFMT_TIFFEP;
                        if(entry_ptr->value == 0)
                            summary_entry->subfiletype = PRIMARY_TYPE;
                        else if(entry_ptr->value == 1)
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                        else if(entry_ptr->value == 2)
                            summary_entry->subfiletype = PAGE_TYPE;
                        else if(entry_ptr->value == 3)
                            summary_entry->subfiletype = MASK_TYPE;
                    }
                    break;
                case INTEROPTAG_RelatedImageWidth: /* ###%%% ???      */
                case TIFFTAG_ImageWidth:
                    if(summary_entry)
                        summary_entry->pixel_width = entry_ptr->value;
                    break;
                case INTEROPTAG_RelatedImageLength: /* ###%%% ???     */
                case TIFFTAG_ImageLength:
                    if(summary_entry)
                        summary_entry->pixel_height = entry_ptr->value;
                    break;
                case TIFFTAG_Compression:
                    if(summary_entry)
                    {
                        summary_entry->compression = entry_ptr->value;
                        if((entry_ptr->value == 7) || (entry_ptr->value == 6))
                        {
                            if(entry_ptr->value == 6)
                                summary_entry->filesubformat |= FILESUBFMT_TIFFOLD;
                            /* Some of this may have to be undone     */
                            /* later, if we find out they lied        */
                            summary_entry->entry_lock = lock_number(summary_entry);
                            if(summary_entry->imageformat == 0)
                                summary_entry->imageformat = IMGFMT_JPEG;
                        }
                        else
                        {
                            summary_entry->entry_lock = lock_number(summary_entry);

                            if(summary_entry->imageformat == 0)
                                summary_entry->imageformat = IMGFMT_TIFF;
                            if(summary_entry->compression == 34713)
                                summary_entry->filesubformat |= FILESUBFMT_NEF;
                            /* If there is no subfiletype tag, this   */
                            /* could be the primary, or it could be a */
                            /* thumbnail                              */
                            if(summary_entry->subfiletype <= POSSIBLE_PRIMARY_TYPE)
                                summary_entry->subfiletype = POSSIBLE_PRIMARY_TYPE;
                        }
                    }
                    break;
                case TIFFTAG_PhotometricInterpretation:
                    /* It appears that the PMI values used in Olympus */
                    /* ORF files (1 or 2 for the primary image) do    */
                    /* not correspond to legitimate TIFF PMI values.  */
                    /* Record them "as is" here, but they shouldn't   */
                    /* be *interpreted* in the same way as TIFF files */
                    if(summary_entry)
                        summary_entry->imagesubformat = entry_ptr->value | IMGSUBFMT_VALUE_IS_PMI;
                    break;
                case TIFFTAG_JPEGInterchangeFormat:
                    /* actually an offset...                          */
                    /* This method of JPEG-in-TIFF is discouraged by  */
                    /* the TIFF spec (after Technote 2), for good and */
                    /* sufficient reason>                             */
                    thn_offset = entry_ptr->value + fileoffset_base;
                    break;
                case TIFFTAG_JPEGInterchangeFormatLength:
                    thn_length = entry_ptr->value;
                    break;
                case TIFFTAG_StripOffsets:
                case TIFFTAG_TileOffsets:
                    /* If there are multiple strips, this will be     */
                    /* wrong, but will be overwritten in the second   */
                    /* pass.                                          */
                    if(summary_entry)
                    {
                        summary_entry->offset = entry_ptr->value;
                        if((summary_entry->compression != 6) &&
                                        (summary_entry->compression != 7))
                        {
                            summary_entry->entry_lock = lock_number(summary_entry);
                        }
                    }
                    break;
                case TIFFTAG_StripByteCounts:
                case TIFFTAG_TileByteCounts:
                    /* This may also be overwritten in the second     */
                    /* pass                                           */
                    if(summary_entry)
                    {
                        summary_entry->length = entry_ptr->value;
                        if((summary_entry->compression != 6) &&
                                        (summary_entry->compression != 7))
                        {
                            summary_entry->entry_lock = lock_number(summary_entry);
                        }
                    }
                    break;
                case TIFFEPTAG_TIFF_EPStandardID:
                    if(summary_entry)
                        summary_entry->filesubformat |= FILESUBFMT_TIFFEP;
                    break;
                case TIFFTAG_Make:
                case TIFFTAG_Model:
                case TIFFTAG_Software:
                    {
                        char *makename,*modelname,*swname;

                        /* These items will be read and printed in    */
                        /* the second pass, if one is made. This      */
                        /* grabs them and records them in globals,    */
                        /* which may be used later by the makernote   */
                        /* code.                                      */
                        switch(entry_ptr->tag)
                        {
                            case TIFFTAG_Make:
                                if(Make_name == NULL)
                                {
                                    Make_name = strdup_value(entry_ptr,
                                                             inptr,
                                                             fileoffset_base);
                                }
                                break;
                            case TIFFTAG_Model:
                                if(Model_name == NULL)
                                {
                                    Model_name = strdup_value(entry_ptr,
                                                              inptr,
                                                              fileoffset_base);
                                }
                                break;
                            case TIFFTAG_Software:
                                if(Software_name == NULL)
                                {
                                    Software_name = strdup_value(entry_ptr,
                                                                 inptr,
                                                                 fileoffset_base);
                                }
                                break;
                        }
                    }
                    break;
                    case TIFFTAG_BitsPerSample:
                        if(summary_entry)
                        {
                            /* Do not override information from       */
                            /* earlier sections; the tiff section in  */
                            /* e.g. MRW sections can lie if the image */
                            /* has been improperly handled by         */
                            /* other-party software.                  */
                            if(summary_entry->bps[0] == 0)
                            {
                                if(entry_ptr->count < 3)
                                {
                                    summary_entry->bps[0] = (unsigned short)entry_ptr->value & 0xffff;
                                    if(entry_ptr->count == 2)
                                        summary_entry->bps[1] =
                                            (unsigned short)((entry_ptr->value & 0xffff0000) >> 16);
                                }
                                else
                                {
                                    summary_entry->bps[0] = read_ushort(inptr,byteorder,entry_ptr->value);
                                    for(i = 1; (i < entry_ptr->count) && (i < MAXSAMPLE); ++i)
                                        summary_entry->bps[i] = read_ushort(inptr,byteorder,HERE);
                                }
                            }
                        }
                        break;
                    case TIFFTAG_SamplesPerPixel:
                        if((summary_entry) && (summary_entry->spp == 0))
                            summary_entry->spp = entry_ptr->value;
                        break;
                    case DNGTAG_DNGVersion:    /* A DNG-specific tag */
                        if(summary_entry)
                            summary_entry->filesubformat |= FILESUBFMT_DNG;
                        break;
                    case TIFFTAG_CR2_0xc5d9:    /* A CR2-specific tag */
                        if(summary_entry)
                        {
                            summary_entry->filesubformat |= FILESUBFMT_CR2;
                            if((summary_entry->compression == 6) &&
                                            (summary_entry->imagesubformat == IMGSUBFMT_RGB))
                            {
                                summary_entry->subfiletype = REDUCED_RES_TYPE;
                            }
                        }
                        break;
                    case TIFFTAG_CR2_0xc5d8:
                    case TIFFTAG_CR2_0xc5e0:
                    case TIFFTAG_CR2_SLICE:
                        /* These tags appear (so far) only in the     */
                        /* weird IFD containing the lossless jpeg     */
                        /* primary. It is possible to check for other */
                        /* things, e.g. just compression, strip       */
                        /* offset and bytecount (1 each), but for now */
                        /* just assume that any of these tags means   */
                        /* CR2. The compression at this point will be */
                        /* 6, but will be re-marked JPEG_SOF_3 when   */
                        /* the image is scanned.                      */
                        if(summary_entry)
                        {
                            summary_entry->filesubformat |= FILESUBFMT_CR2;
                            summary_entry->subfiletype = PRIMARY_TYPE;
                        }
                        break;
                case TIFFTAG_SubIFDtag:
                case EXIFTAG_ExifIFDPointer:
                case EXIFTAG_GPSInfoIFDPointer:
                case EXIFTAG_Interoperability: 
                case TIFFTAG_PrintIM:
                    use_second_pass++;
                    break;
                default:
                    break;
            }
            print_tag_address(ENTRY,entry_offset,indent,prefix);
            value_offset = print_entry(inptr,byteorder,entry_ptr,fileoffset_base,
                                                summary_entry,listname,ifdtype,
                                                ifdnum, subifdnum, SMALLINDENT);

            /* Keep track of how far into the file reading progresses */
            if(value_offset == 0UL)
                value_offset = current_offset;
            if(value_offset > max_value_offset)
                max_value_offset = value_offset;
            if((is_offset(entry_ptr)))
                ++use_second_pass;
            entry_offset = current_offset;
        }

        next_ifd_offset = read_ulong(inptr,byteorder,current_offset);
        if(next_ifd_offset > 0UL)
        {
            if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
            {
                print_tag_address(SECTION,current_offset,indent,prefix);
                chpr += printf("**** next IFD offset %lu",next_ifd_offset);
                next_ifd_offset += fileoffset_base;
                chpr += printf("(+ %lu = %#lx/%lu)",fileoffset_base,
                            next_ifd_offset,next_ifd_offset);
            }
            else
                next_ifd_offset += fileoffset_base;

            /* Corrupt file will cause infinite loops. So we abort.
             * It is possible this can be worked around better.
             * See Issue #9 https://github.com/hfiguiere/exifprobe/issues/9
             */
            if(next_ifd_offset < ftell(inptr)) {
              printred("\nReading IFD backwards. INVALID FILE. ABORTING.\n");
              exit(1);
            } else if (next_ifd_offset > filesize) {
              printred("\nReading IFD past EOF. INVALID FILE. ABORTING.\n");
              exit(1);
            }
            /* We should be able to tolerate these */
            if ((ifdtype != TIFF_IFD) && (ifdtype != TIFF_SUBIFD))
            {
                if(PRINT_SECTION)
                    printred(" INVALID NEXT IFD OFFSET ");
                else
                {
                    chpr = newline(chpr);
                    printred("# ========= WARNING: INVALID NEXT IFD OFFSET ");
                    chpr += printf("%ld in ",next_ifd_offset);
                    /* Until someone creates a subifd in a subifd,    */
                    /* this is enough.                                */
                    if(ifdtype == TIFF_IFD)
                        chpr += printf("IFD %d =========",ifdnum);
                    else if(ifdtype == TIFF_SUBIFD)
                        chpr += printf("SUBIFD %d of IFD %d =========",subifdnum,ifdnum);
                    else
                        chpr += printf("%s =========",ifdname);
                }
                next_ifd_offset = 0L;
            }
            /* This is the limit of offsets which should be part of   */
            /* this ifd; a limit may have been passed in from the     */
            /* parent, if it had a length of this section. This will  */
            /* replace that number unconditionally, although it may   */
            /* be that one should be checked against the other...     */
            /* This can be checked against max_value_offset, and      */
            /* value_offset in the second pass.                       */
            max_offset = next_ifd_offset;
            chpr = newline(chpr);
        }
        else
        {
            if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
            {
                print_tag_address(SECTION,current_offset,indent,prefix);
                chpr += printf("**** next IFD offset 0");
                chpr = newline(chpr);
            }
        }

        if(max_value_offset < current_offset)
            max_ifd_offset = current_offset;    
        else
            max_ifd_offset = max_value_offset;

        if(Debug & END_OF_SECTION_DEBUG)
            printf("mo=%lu, mvo=%lu, mio=%lu, nio=%lu, ol=%lu\n",max_offset,max_value_offset,max_ifd_offset,next_ifd_offset,offset_limit);

        /* If we made it through the first pass, we should be able to */
        /* get some info from the second pass, so don't let a failure */
        /* here stop us. It will fail later.                          */
        if(ferror(inptr) || feof(inptr))
        {
            chpr += printf("#========= WARNING: READ NEXT IFD OFFSET FAILED =========");
            chpr = newline(chpr);
            why(stdout);
            clearerr(inptr);    /* keep going...                      */
        }
        else
            current_offset = ftell(inptr);

        value_offset = current_offset;
        if(use_second_pass)
        {
            if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
            {
                print_tag_address(SECTION,value_offset,indent,prefix);
                chpr += printf("============= VALUES, ");

                if(ifdtype == TIFF_IFD)
                    chpr += printf("IFD %d",ifdnum);
                else if(ifdtype == TIFF_SUBIFD)
                    chpr += printf("SubIFD %d of IFD %d",subifdnum,ifdnum);
                else if(ifdtype == INTEROP_IFD)
                    chpr += printf("Interoperability SubIFD");
                else if(ifdtype == EXIF_IFD)    /* shouldn't happen   */
                    chpr += printf("EXIF IFD");
                else if(ifdtype == MAKER_SUBIFD)
                    chpr += printf("%s",ifdname);
                else
                    chpr += printf("UNKNOWN IFD TYPE %d",ifdtype);
                chpr += printf(" ============");
                chpr = newline(chpr);
            }

            /* Second pass, to evaluate entries which are stored      */
            /* indirectly (the value requires more than 4 bytes).     */

            /* This time we have to explicitly seek to each entry,    */
            /* since the value processing may send the file pointer   */
            /* off to exotic places.                                  */
            entry_offset = start_entry_offset;
            for(entry_num = 0; entry_num < num_entries; ++entry_num)
            {
                entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
                if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                    ferror(inptr) || feof(inptr))
                {
                    /* If the first pass made it through invalid      */
                    /* entries, this pass should just ignore them and */
                    /* quietly continue.                              */
                    clearerr(inptr);
                    entry_offset = current_offset = ftell(inptr);
                    continue;
                }
                entry_offset = current_offset = ftell(inptr);
                value_is_offset = is_offset(entry_ptr);
                switch(entry_ptr->tag)
                {
                    case TIFFTAG_SubIFDtag:
                        value_offset = process_subifd(inptr,byteorder,
                                            entry_ptr, fileoffset_base,0L,
                                            summary_entry,listname,
                                            ifdnum,++subifdnum,TIFF_SUBIFD,
                                            indent);
                        /* subifds are not part of the IFD; throw     */
                        /* away this value for now.                   */
                        value_offset = 0;
                        break;
                    case EXIFTAG_ExifIFDPointer:
                        value_start_offset = entry_ptr->value + fileoffset_base;
                        value_offset = process_exif_ifd(inptr,byteorder,
                                            entry_ptr->value,fileoffset_base,
                                            next_ifd_offset,summary_entry,listname,
                                            ifdnum,indent);
                        if(max_offset && (value_offset > max_offset))
                            offset_limit = value_offset;
                        break;
                    case EXIFTAG_GPSInfoIFDPointer:
                        value_offset = process_gps_ifd(inptr,byteorder,
                                            entry_ptr->value,fileoffset_base,
                                            next_ifd_offset,summary_entry,listname,
                                            ifdnum,indent);
                        break;
                    case EXIFTAG_Interoperability: 
                        /* This doesn't belong in a TIFF IFD, but     */
                        /* be prepared.                               */
                        /* Also we make sure we are not calling on    */
                        /* the same ifd offset                        */
                        PUSHCOLOR(INTEROP_COLOR);
                        if (entry_ptr->value != ifd_offset) {
                            value_offset = process_tiff_ifd(inptr,byteorder,
                                                entry_ptr->value,fileoffset_base,
                                                next_ifd_offset,summary_entry,
                                                listname,INTEROP_IFD,ifdnum,0,
                                                indent);
                        }
                        value_offset = 0;
                        POPCOLOR();
                        break;
                    case TIFFTAG_StripOffsets:
                    case TIFFTAG_TileOffsets:
                        /* Update the summary values set in the first */
                        /* pass                                       */
                        /* Bad strip offsets seem to be one of the    */
                        /* more common errors, especially when images */
                        /* have been processed by editing software.   */
                        /* Offsets greater than filesize are easy to  */
                        /* detect, so check for them, and test to see */
                        /* if they're just written with the wrong     */
                        /* byteorder.                                 */
                        
                        /* ###%%% need to record number of offsets    */
                        /* and location of offsets for multi-tile     */
                        /* (multi-strip?) images jpeg-compressed by   */
                        /* tile (seen in DNG)                         */

                       if((value_is_offset) && summary_entry)
                        {
                            alt_byteorder = byteorder;
                            summary_entry->chunkhow = entry_ptr->tag;
                            summary_entry->chunktype = entry_ptr->value_type;
                            /* ###%%% combine short and long cases; just switch read_xxx() */
                            if(entry_ptr->value_type == LONG)
                            {
                                unsigned long tmp_offset;

                                tmp_offset = read_ulong(inptr,byteorder,entry_ptr->value + fileoffset_base);
                                summary_entry->offset = tmp_offset;
                                summary_entry->offset_loc = entry_ptr->value +fileoffset_base;
                                summary_entry->noffsets = entry_ptr->count;
                                if(tmp_offset > filesize)
                                {
                                    chpr = newline(chpr);
                                    PUSHCOLOR(HI_RED);
                                    print_filename();
                                    chpr += printf("# WARNING: initial stripoffset (%#lx/%lu) > filesize...wrong byteorder? ",
                                                    tmp_offset,tmp_offset);
                                    print_byteorder(byteorder,1);
                                    chpr = newline(chpr);
                                    if(byteorder == TIFF_MOTOROLA)
                                        alt_byteorder = TIFF_INTEL;
                                    else
                                        alt_byteorder = TIFF_MOTOROLA;
                                    tmp_offset = read_ulong(inptr,alt_byteorder,
                                            entry_ptr->value + fileoffset_base);
                                    print_filename();
                                    chpr += printf("# WARNING: initial stripoffset using alt byteorder [");
                                    print_byteorder(alt_byteorder,1);
                                    chpr += printf("] is %#lx/%lu ",
                                                    tmp_offset,tmp_offset);
                                    chpr = newline(chpr);
                                    if(tmp_offset < filesize)
                                        summary_entry->offset = tmp_offset;
                                    POPCOLOR();
                                }
                            }
                            else
                            {
                                unsigned short tmp_offset;

                                tmp_offset = read_ushort(inptr,byteorder,entry_ptr->value + fileoffset_base);
                                summary_entry->offset = tmp_offset;
                                summary_entry->offset_loc = entry_ptr->value +fileoffset_base;
                                summary_entry->noffsets = entry_ptr->count;
                                if((unsigned long)tmp_offset > filesize)
                                {
                                    chpr = newline(chpr);
                                    PUSHCOLOR(HI_RED);
                                    print_filename();
                                    chpr += printf("# WARNING: initial stripoffset (%#x/%u) > filesize...wrong byteorder? ",
                                                    tmp_offset,tmp_offset);
                                    print_byteorder(byteorder,1);
                                    chpr = newline(chpr);
                                    if(byteorder == TIFF_MOTOROLA)
                                        alt_byteorder = TIFF_INTEL;
                                    else
                                        alt_byteorder = TIFF_MOTOROLA;
                                    tmp_offset = read_ushort(inptr,alt_byteorder,
                                                entry_ptr->value + fileoffset_base);
                                    print_filename();
                                    chpr += printf("# WARNING: initial stripoffset using alt byteorder  [");
                                    print_byteorder(alt_byteorder,1);
                                    chpr += printf("] is %#x/%u ",
                                                    tmp_offset,tmp_offset);
                                    chpr = newline(chpr);
                                    if(tmp_offset < filesize)
                                        summary_entry->offset = tmp_offset;
                                    POPCOLOR();
                                }
                            }
                        }
                        if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && value_is_offset))
                        {
                            print_tag_address(ENTRY,fileoffset_base + entry_ptr->value,
                                                indent,prefix);
                            print_tagid(entry_ptr,SMALLINDENT,ifdtype);
                            value_offset = 
                                print_offset_value(inptr,byteorder,entry_ptr,
                                                fileoffset_base,listname,
                                                ifdtype,indent,1);
                        }
                        break;
                    case TIFFTAG_StripByteCounts:
                    case TIFFTAG_TileByteCounts:
                        if((value_is_offset) && summary_entry)
                        {
                            tmp_length =
                                sum_strip_bytecounts(inptr,byteorder,
                                            entry_ptr->value + fileoffset_base,
                                            entry_ptr->count,entry_ptr->value_type);

                            summary_entry->length = tmp_length;
                            summary_entry->length_loc = entry_ptr->value + fileoffset_base;
                            if(summary_entry->noffsets != entry_ptr->count)
                                printred(" # Warning: number of bytecounts != number of offsets");
                            if(tmp_length > filesize)
                            {
                                chpr = newline(chpr);
                                PUSHCOLOR(HI_RED);
                                    print_filename();
                                chpr += printf("# WARNING: total stripbytecount (%#lx/%lu) > filesize (%lu) ... wrong byteorder? ",
                                                tmp_length,tmp_length,filesize);
                                print_byteorder(byteorder,1);
                                chpr = newline(chpr);
                                if(byteorder == TIFF_MOTOROLA)
                                    alt_byteorder = TIFF_INTEL;
                                else
                                    alt_byteorder = TIFF_MOTOROLA;
                                alt_length =
                                    sum_strip_bytecounts(inptr,alt_byteorder,
                                                entry_ptr->value + fileoffset_base,
                                                entry_ptr->count,entry_ptr->value_type);
                                print_filename();
                                chpr += printf("# WARNING: total stripbytecount using alt byteorder [");
                                print_byteorder(alt_byteorder,1);
                                chpr += printf("] is %#lx/%lu ",
                                                alt_length,alt_length);
                                chpr = newline(chpr);
                                if(alt_length < filesize)
                                    summary_entry->length = alt_length;
                                else
                                {
                                    print_filename();
                                    chpr += printf("# WARNING: file may be truncated");
                                    chpr = newline(chpr);
                                }
                                POPCOLOR();
                            }
                        }
                        if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && value_is_offset))
                        {
                            print_tag_address(ENTRY,fileoffset_base + entry_ptr->value,
                                                indent,prefix);
                            print_tagid(entry_ptr,SMALLINDENT,ifdtype);
                            value_offset =
                                print_offset_value(inptr,byteorder,entry_ptr,
                                                fileoffset_base,listname,
                                                ifdtype,indent,1);
                            /* ###%%% find a way to print total       */
                            /* ByteCount, at least in LIST mode       */
                        }
                        break;
                    case TIFFTAG_JPEGTables:
                        nameoftag = tagname(entry_ptr->tag);
                        if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && value_is_offset))
                        {
                            if(PRINT_SECTION)
                            {
                                chpr = newline(chpr);
                                print_tag_address(ENTRY,fileoffset_base + entry_ptr->value,
                                                    indent,prefix);
                                chpr += printf("# Start of %s length %lu",nameoftag,
                                                            entry_ptr->count);
                                chpr = newline(chpr);
                            }
                        }
                        marker = read_ushort(inptr,TIFF_MOTOROLA,fileoffset_base + entry_ptr->value);
                        /* Need a new entry; this IFD's entry may be  */
                        /* used for an image, but may not be locked   */
                        /* yet.                                       */
                        tmp_summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG);
                        value_offset = process_jpeg_segments(inptr,
                                                    fileoffset_base + entry_ptr->value,
                                                    marker,entry_ptr->count,
                                                    tmp_summary_entry,listname,
                                                    prefix,indent+SMALLINDENT);
                        if(tmp_summary_entry)
                            tmp_summary_entry->imagesubformat = TIFFTAG_JPEGTables;
                        if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && value_is_offset))
                        {
                            if((PRINT_SECTION))
                            {
                                /* ###%% check for early EOI;         */
                                if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                                    chpr = newline(chpr);
                                jpeg_status(status);
                                print_tag_address(ENTRY,fileoffset_base + entry_ptr->value + entry_ptr->count + 1,
                                                    indent,prefix);
                                chpr += printf("# End of %s",nameoftag);
                                print_jpeg_status();
                                chpr = newline(chpr);
                            }
                        }
                        break;
                    case TIFFTAG_PrintIM:
                        if(next_ifd_offset && (cur_ifd_offset > next_ifd_offset))
                            prefix = "+";
                        else
                            prefix = "@";
                        process_pim(inptr,byteorder,entry_ptr->value,fileoffset_base,
                                    entry_ptr->count,tagname(entry_ptr->tag),
                                    listname,prefix,indent);
                        chpr = newline(0);
                        break;
                    default:
                        if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && value_is_offset))
                        {
                            print_tag_address(ENTRY,fileoffset_base + entry_ptr->value,
                                                    indent,prefix);
                            print_tagid(entry_ptr,SMALLINDENT,ifdtype);
                            value_offset =
                                print_offset_value(inptr,byteorder,entry_ptr,
                                                fileoffset_base,listname,
                                                ifdtype,indent,1);
                        }
                        break;
                }
                if(value_offset == 0L) /* bad entry; try the next one */
                    clearerr(inptr);
                else if(value_offset > filesize)
                    goto blewit;
                else if(value_offset > max_value_offset)
                    max_value_offset = value_offset;
            }
        }
        else if(current_offset > max_value_offset)
            max_value_offset = current_offset;
        max_ifd_offset = max_value_offset;

        indent -= SMALLINDENT;

        /* max_ifd_offset is the maximum offset we've read so far;    */
        /* max_offset, if set, is the highest offset of this ifd,     */
        /* according to either the parent or the recently-read        */
        /* next_ifd_offset.                                           */

        if(max_offset && (max_ifd_offset > max_offset))
            prefix = "?";
        if(Debug & END_OF_SECTION_DEBUG)
            printf("mo=%lu, mvo=%lu, mio=%lu, nio=%lu, ol=%lu, prefix=%s\n",
                                max_offset,max_value_offset,max_ifd_offset,
                                next_ifd_offset,offset_limit,prefix);

        /* ================== show jpeg thumbnail =================== */
        if(thn_offset || thn_length)
        {
            int startindent;

            if(Debug & END_OF_SECTION_DEBUG)
                printf("mo=%lu, ol=%lu, thno=%lu\n",max_offset,offset_limit,thn_offset);
            /* This will be set only if a JpegInterchangeFormat tag   */
            /* has been seen, so this must be a JPEG thumbnail. The   */
            /* Exif spec indicates that the data should be written    */
            /* within the values section of the IFD, so scan and      */
            /* report here. If it appears to be written outside the   */
            /* bounds of the IFD, mark it. "max_offset" is the        */
            /* parent's idea (if any) of the extent of the IFD.       */
            /* "offset_limit" is the beginning of the Exif section,   */
            /* if present.                                            */

            if(max_offset && (thn_offset > max_offset))
                tprefix = ">";
            else if(offset_limit && (thn_offset > offset_limit))
                tprefix = "+";
            else
                tprefix = prefix;
            startindent = charsprinted();
            print_tag_address(SECTION,thn_offset,indent + SMALLINDENT,tprefix);
            if(PRINT_SECTION)
            {
                if(thn_length)
                    chpr += printf("#### Start of JPEG thumbnail data for IFD %d",ifdnum);
                else
                {
                    PUSHCOLOR(RED);
                    chpr += printf("#### ZERO LENGTH JPEG thumbnail for IFD %d",ifdnum);
                    POPCOLOR();
                }
                if((ifdtype == TIFF_SUBIFD) || (subifdnum >= 0))
                    chpr += printf(" SubIFD %d",subifdnum);
                if(ifdtype == MAKER_IFD)
                    chpr += printf(" %s",ifdname);
                else if(ifdtype == MAKER_SUBIFD)
                    chpr += printf(" %s",ifdname);
                if(thn_length)
                    chpr += printf(", length %lu ####",thn_length);
                else
                {
                    if(summary_entry && (summary_entry->imageformat == IMGFMT_JPEG)
                        && ((summary_entry->compression == 6) || (summary_entry->compression == 7)))
                    {
                        summary_entry->offset = thn_offset;
                    }
                }
                chpr = newline(chpr);
            } 

            if(thn_length)
            {
                unsigned short tmp_marker;

                marker = read_ushort(inptr,TIFF_MOTOROLA,thn_offset);
                if(marker != JPEG_SOI)
                {
                    if((marker & 0xff) != 0xd) /* possibly BAD_SOI    */
                    {
                        tmp_marker = read_ushort(inptr,TIFF_MOTOROLA,thn_offset - fileoffset_base);
                        if(tmp_marker == JPEG_SOI)
                        {
                            chpr = newline(chpr);
                            print_filename();
                            putchar('#');
                            if((PRINT_SECTION))
                                putindent(startindent - 1);
                            PUSHCOLOR(HI_RED);
                            chpr += printf(" WARNING: JPEGInterChangeFormat offset is botched");
                            chpr = newline(chpr);
                            if((PRINT_SECTION))
                            {
                                print_filename();
                                putchar('#');
                                putindent(startindent - 1);
                                chpr += printf(" WARNING: (possibly) correct offset follows:");
                                chpr = newline(chpr);
                            }
                            marker = tmp_marker;
                            thn_offset -= fileoffset_base;
                            print_tag_address(SECTION,thn_offset,indent + SMALLINDENT,tprefix);
                            if((LIST_MODE))
                            {
                                /* mock up the entry; the last read   */
                                /* entry is finished, so it can be    */
                                /* overwritten                        */
                                entry_ptr->tag = TIFFTAG_JPEGInterchangeFormat;
                                entry_ptr->value_type = LONG;
                                entry_ptr->count = 1;
                                entry_ptr->value = thn_offset - fileoffset_base;
                                value_offset = print_entry(inptr,byteorder,entry_ptr,fileoffset_base,
                                                            summary_entry,parent_name,ifdtype,
                                                            ifdnum, subifdnum,MEDIUMINDENT);
                            }
                            else if((PRINT_SECTION))
                                chpr += printf("#### Start of JPEG thumbnail data for IFD %d",ifdnum);
                            POPCOLOR();
                            chpr = newline(chpr);
                        }
                    }
                }
                /* Is this the image described by this ifd, or an     */
                /* add-on?  ...or both...                             */
                if(summary_entry &&
                    ((summary_entry->compression == 7) || (summary_entry->compression == 6)))
                {
                    /* If there's a length set in the summary, assume */
                    /* there's already an image associated with this  */
                    /* entry.                                         */
                    if((summary_entry->length == 0UL))
                    {
                        /* Othersise, make sure the jpeg processor    */
                        /* uses the current summary entry             */
                        summary_entry->entry_lock = 0;
                        summary_entry->imageformat = IMGFMT_NOIMAGE;
                        if(Debug & SCAN_DEBUG)
                        {
                            chpr = newline(chpr);
                            printred("DEBUG:");
                            chpr += printf(" UNLOCK **** ");
                            chpr += printf(" offset=%lu, length=%lu ",summary_entry->offset,
                                                                    summary_entry->length);
                            chpr = newline(chpr);
                        }
                    }
                }

                value_offset = 0;
                if(marker)
                    value_offset = process_jpeg_segments(inptr,thn_offset,marker,
                                                thn_length,summary_entry,parent_name,
                                                tprefix,indent+SMALLINDENT);

                /* Even if the jpeg was bad, there will be a summary  */
                /* entry for it.  It may not be the current one.      */
                tmp_summary_entry = last_summary_entry(summary_entry);
                tmp_summary_entry->ifdnum = ifdnum;
                tmp_summary_entry->subifdnum = subifdnum;
                tmp_summary_entry->datatype = ifdtype;
                    
                if(PRINT_SECTION)
                {
                    if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                        chpr = newline(chpr);
                    jpeg_status(status);
                    print_tag_address(SECTION,thn_offset + thn_length - 1,indent + SMALLINDENT,tprefix);
                    chpr += printf("#### End of JPEG thumbnail data for IFD %d",ifdnum);
                    if((ifdtype == TIFF_SUBIFD) || (subifdnum >= 0))
                        chpr += printf(" SubIFD %d",subifdnum);
                    if(ifdtype == MAKER_IFD)
                        chpr += printf(" %s",ifdname);
                    else if(ifdtype == MAKER_SUBIFD)
                        chpr += printf(" %s",ifdname);
                    chpr += printf(", length %lu ####",thn_length);
                    print_jpeg_status();
                    chpr = newline(chpr);
                }

                if(Debug & END_OF_SECTION_DEBUG)
                    printf("vo=%lu, mo=%lu, mvo=%lu, mio=%lu, nio=%lu, ol=%lu\n",value_offset,
                            max_offset,max_value_offset,max_ifd_offset,next_ifd_offset,offset_limit);
                if((value_offset == 0L) || (value_offset > (thn_offset + thn_length)))
                    value_offset = thn_offset + thn_length;
                if(value_offset > max_value_offset)
                    max_value_offset = value_offset;
            }
            /* done with this thumbnail                               */
            thn_offset = thn_length = 0;
            if((max_value_offset > max_ifd_offset) && 
                                    ((offset_limit == 0) ||
                                        (max_value_offset < offset_limit)))
            {
                max_ifd_offset = max_value_offset;
            }
        }
        /* ================== end show jpeg thumbnail =============== */

        /* The TIFF spec does not specify, directly, where an IFD     */
        /* ends. If there is a next IFD, the start of that IFD        */
        /* (next_ifd_offset) gives an upper bound for the current     */
        /* IFD, but for the last IFD, guessing is necessary. The IFD  */
        /* should end when the last offset value given in the         */
        /* directory entries is written; MANY camera files which use  */
        /* this format write items, particularly thumbnails, at       */
        /* arbitrary offsets, and the TIFF spec itself allows offsets */
        /* to point anywhere in the file. The Exif spec declares that */
        /* "thumbnails" should be written within the values section   */
        /* of the IFD, but (oddly) not the "primary". Add subifds,    */
        /* disagreements between TIFF and TIFFEP about which IFD      */
        /* should describe the primary, multiple "thumbnails" of      */
        /* various sizes, etc. and it is very difficult to think of   */
        /* an IFD as a "container" (as it should be) or to decide     */
        /* where it ends.                                             */

        /* The thumbnails written above should be a part of the IFD,  */
        /* according to the Exif spec and common sense. All other     */
        /* offset values for the IFD have been written, so it is time */
        /* to declare that the IFD ends. If there is information      */
        /* giving the maximum size of this IFD, items outside the     */
        /* bounds have already been marked so.                        */

        if(Debug & END_OF_SECTION_DEBUG)
            printf("vo=%lu, mo=%lu, mvo=%lu, mio=%lu, nio=%lu, ol=%lu\n",value_offset,
                    max_offset,max_value_offset,max_ifd_offset,next_ifd_offset,offset_limit);

        /* ========= show end of ifd ===========                      */
        if((prefix && (*prefix == '>')) || (max_offset == 0) ||
                                                    (max_ifd_offset < max_offset))
        {
            display_end_of_ifd(max_ifd_offset,ifdtype,ifdnum,subifdnum,indent,
                                                            ifdname,prefix);
        }
        else
            display_end_of_ifd(max_offset,ifdtype,ifdnum,subifdnum,indent,
                                                            ifdname,prefix);

        /* =  ================= show ifd image ========  =========== */
        /* ###%%% not showing offset and length in LIST MODE */
        if(summary_entry &&
                    ((summary_entry->fileformat == TIFF_MAGIC) ||
                        (summary_entry->fileformat == ORF1_MAGIC) ||
                            (summary_entry->fileformat == ORF2_MAGIC)) &&
                            ((ifdtype == TIFF_IFD) || (ifdtype == TIFF_SUBIFD)) &&
                                (summary_entry->length > 0))
        {
            if(Debug & END_OF_SECTION_DEBUG)
                printf("mo=%lu, ol=%lu, seo=%lu\n",max_offset,offset_limit,summary_entry->offset);
            /* These are being displayed "inline" with the IFD that   */
            /* describes them; try to mark them as shown out of       */
            /* place.                                                 */
            if(max_offset && (summary_entry->offset > max_offset))
                tprefix = ">";
            else if(offset_limit && (summary_entry->offset > offset_limit))
                tprefix = "+";
            else
                tprefix = prefix;
            /* Scan jpeg images and check for mismarked compression   */
            /* types                                                  */
            switch(summary_entry->compression)
            {
            case 6:
                if(summary_entry->imagesubformat & IMGSUBFMT_RGB)
                {
                    /* Perhaps this should be done unconditionally... */
                    max_value_offset = summary_entry->offset + summary_entry->length;
                    if(summary_entry->filesubformat & FILESUBFMT_CR2)
                    {
                        /* Actually uncompressed RGB                  */
                        summary_entry->compression = 1;
                        summary_entry->imageformat = IMGFMT_JPEG | IMGFMT_MISMARKED;
                    }
                    print_tag_address(SECTION,summary_entry->offset,indent,tprefix);
                    if(PRINT_SECTION)
                    {
                        chpr += printf("<=-=-=> Start of RGB data for IFD %d,",
                                                            summary_entry->ifdnum);
                        if(summary_entry->datatype == TIFF_SUBIFD)
                            chpr += printf(" SubIFD %d,",summary_entry->subifdnum);
                        chpr += printf(" data length %lu",summary_entry->length);
                        chpr = newline(chpr);

                        chpr = newline(chpr);
                        if((PRINT_VALUE))
                            dumpsection(inptr,summary_entry->offset,summary_entry->length,
                                                                    indent + SMALLINDENT);
                        print_tag_address(SECTION,max_value_offset - 1,indent,"-");
                        chpr += printf("</=-=-=> End of RGB data");
                        if((max_value_offset - 1) > filesize)
                        {
                            PUSHCOLOR(RED);
                            chpr += printf(" (TRUNCATED at %lu)",filesize);
                            POPCOLOR();
                            summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                        }
                        chpr = newline(chpr);
                    }
                    else if((PRINT_ENTRY))
                    {
                        /* ###%%% pseudo-tags fpr LIST Mode here */
                        print_tag_address(ENTRY,HERE,indent,"*");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",listname);
                            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"RGBImageData");
                        }
                        if((PRINT_VALUE))
                            chpr += printf(" = @%lu:%-9lu",summary_entry->offset,summary_entry->length);
                        chpr = newline(chpr);
                    }
                    /* done                                           */
                    break;

                }
                /* fall through */
            case 7:
                print_tag_address(SECTION,summary_entry->offset,indent,tprefix);
                if(PRINT_SECTION)
                {
                    chpr += printf("<=====> Start of JPEG data for IFD %d,",summary_entry->ifdnum);
                    if(summary_entry->datatype == TIFF_SUBIFD)
                        chpr += printf(" SubIFD %d,",summary_entry->subifdnum);
                    chpr += printf(" data length %lu",summary_entry->length);
                    chpr = newline(chpr);
                }
                else if((PRINT_ENTRY))
                {
                    /* ###%%% pseudo-tags fpr LIST Mode here */
                    print_tag_address(ENTRY,HERE,indent,"*");
                    if((PRINT_TAGINFO))
                    {
                        if((PRINT_LONGNAMES))
                            chpr += printf("%s.",listname);
                        chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"JpegImageData");
                    }
                    if((PRINT_VALUE))
                        chpr += printf(" = @%lu:%-9lu",summary_entry->offset,summary_entry->length);
                    chpr = newline(chpr);
                }

                /* If this IFD describes a JPEG image, the    */
                /* entry must go unlocked to the jpeg         */
                /* processor, which will lock it. This may    */
                /* cause problems if the IFD *also* contains  */
                /* JpegInterchangeFormat tags. It happens.    */
                if(Debug & SCAN_DEBUG)
                {
                    chpr = newline(chpr);
                    printred("DEBUG:");
                    chpr += printf(" UNLOCK **** ");
                    chpr += printf(" offset=%lu, length=%lu ",summary_entry->offset,
                                                            summary_entry->length);
                    chpr = newline(chpr);
                }
                summary_entry->entry_lock = 0;
                summary_entry->imageformat = IMGFMT_NOIMAGE;
                value_offset = 0;

                /* If compression == 7, PMI is CFA, and there are     */
                /* multiple tile offsets, each tile may be separately */
                /* compressed.                                        */
                marker = read_ushort(inptr,TIFF_MOTOROLA,summary_entry->offset);
                if((PRINT_SECTION) && (summary_entry->compression == 7) &&
                        (summary_entry->chunkhow == TIFFTAG_TileOffsets) &&
                                (summary_entry->noffsets > 1))
                {
                    unsigned long tmp_offset;
                    unsigned long offset_loc;
                    unsigned long length_loc;

                    /* Individually compressed tiles Show the first   */
                    /* and last, and just check for JPEG_SOI for      */
                    /* those in the middle. There should be an option */
                    /* to display them all and/or dump the section    */
                    /* (although '-O' can be used for that).          */
                    offset_loc = summary_entry->offset_loc;
                    length_loc = summary_entry->length_loc;
                    if(marker)
                    {
                        tmp_length = read_ulong(inptr,byteorder,length_loc);
                        value_offset = process_jpeg_segments(inptr,summary_entry->offset,marker,
                                            tmp_length,summary_entry,parent_name,
                                            tprefix,indent+MEDIUMINDENT);
                    }
                    for(i = 1; i < summary_entry->noffsets; ++i)
                    {
                        if(summary_entry->chunktype == LONG)
                        {
                            offset_loc += 4;
                            length_loc += 4;
                            tmp_offset = read_ulong(inptr,byteorder,offset_loc);
                            tmp_length = read_ulong(inptr,byteorder,length_loc);
                        }
                        else
                        {
                            offset_loc += 2;
                            length_loc += 2;
                            tmp_offset = read_ushort(inptr,byteorder,offset_loc);
                            tmp_length = read_ushort(inptr,byteorder,length_loc);
                        }
                        marker = read_ushort(inptr,TIFF_MOTOROLA,tmp_offset);
                        if(feof(inptr) || ferror(inptr))
                        {
                            value_offset = summary_entry->offset + summary_entry->length;
                            clearerr(inptr);
                            break;
                        }
                        if((i + 1) == summary_entry->noffsets)
                        {
                            /* Note that this one goes into the       */
                            /* summary                                */
                            chpr = newline(chpr);
                            value_offset = process_jpeg_segments(inptr,tmp_offset,marker,
                                                        tmp_length,summary_entry,parent_name,
                                                        tprefix,indent+MEDIUMINDENT);
                        }
                        else if(marker == JPEG_SOI)
                        {
                            /* This checks that there is at least an  */
                            /* SOI where it is supposed to be, but    */
                            /* avoids putting every tile in the       */
                            /* summary, which would be very boring.   */
                            if(i != 1)
                                chpr += printf(",");
                            if((i == 1) || ((i % 10) == 0))
                            {
                                chpr = newline(chpr);
                                print_tag_address(SECTION,HERE,indent + MEDIUMINDENT,"JPEG TILES");
                            }
                            chpr += printf("[%d]",i + 1);
                        }
                        else
                        {
                            ;
                        }   
                        if(feof(inptr) || ferror(inptr))
                        {
                            clearerr(inptr);
                            value_offset = summary_entry->offset + summary_entry->length;
                            break;
                        }
                    }
                    
                }
                else
                {
                    if(marker)
                        value_offset = process_jpeg_segments(inptr,summary_entry->offset,marker,
                                            summary_entry->length,summary_entry,parent_name,
                                            tprefix,indent+MEDIUMINDENT);
                }
                if(PRINT_SECTION)
                {
                    if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                        chpr = newline(chpr);
                    jpeg_status(status);
                    print_tag_address(SECTION,summary_entry->offset + summary_entry->length - 1,
                                        indent,tprefix);
                    chpr += printf("</=====> End of JPEG data ====");
                    if(value_offset > (summary_entry->offset + summary_entry->length))
                        value_offset = summary_entry->offset + summary_entry->length;
                    print_jpeg_status();
                    if((value_offset - 1) > filesize)
                    {
                        PUSHCOLOR(RED);
                        chpr += printf(" (TRUNCATED at %lu)",filesize);
                        POPCOLOR();
                        summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                    }
                    chpr = newline(chpr);
                }
                if(value_offset == 0L)
                    clearerr(inptr);
                else if(value_offset > max_value_offset)
                    max_value_offset = value_offset;
                if((max_value_offset - 1) > filesize)
                    summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                break;
            case 0:     /* this shouldn't happen...                   */
            default:
                if((summary_entry) && (summary_entry->imageformat != IMGFMT_JPEG))
                {
                    max_value_offset = summary_entry->offset + summary_entry->length;
                    if(PRINT_SECTION)
                    {
                        print_tag_address(SECTION,summary_entry->offset,indent,tprefix);
                        chpr += printf("<=-=-=> Start of ");
                        print_imageformat(summary_entry);
                        print_imagesubformat(summary_entry);
                        print_imagecompression(summary_entry);
                        chpr += printf(" image data for IFD %d,",
                                                            summary_entry->ifdnum);
                        if(summary_entry->datatype == TIFF_SUBIFD)
                            chpr += printf(" SubIFD %d,",summary_entry->subifdnum);
                        chpr += printf(" data length %lu",summary_entry->length);
                        chpr = newline(chpr);

                        chpr = newline(chpr);
                        if((PRINT_VALUE))
                            dumpsection(inptr,summary_entry->offset,summary_entry->length,
                                                                    indent + SMALLINDENT);
                        print_tag_address(SECTION,max_value_offset - 1,indent,tprefix);
                        chpr += printf("</=-=-=> End of image data");
                        if((max_value_offset - 1) > filesize)
                        {
                            PUSHCOLOR(RED);
                            chpr += printf(" (TRUNCATED at %lu)",filesize);
                            POPCOLOR();
                            summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                        }
                        chpr = newline(chpr);
                    }
                    else if((PRINT_ENTRY))
                    {
                        /* ###%%% pseudo-tags fpr LIST Mode here */
                        print_tag_address(ENTRY,HERE,indent,"*");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",listname);
                            chpr += printf("%-*.*s",TAGWIDTH,TAGWIDTH,"ImageData");
                        }
                        if((PRINT_VALUE))
                            chpr += printf(" = @%lu:%-9lu",summary_entry->offset,summary_entry->length);
                        if((max_value_offset - 1) > filesize)
                        {
                            PUSHCOLOR(RED);
                            chpr += printf(" # TRUNCATED at %lu",filesize);
                            POPCOLOR();
                            summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                        }
                        chpr = newline(chpr);
                    }
                    else if((max_value_offset -1) > filesize)
                            summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                }
                break;
            }
            if((max_value_offset > max_ifd_offset) && 
                    ((offset_limit == 0) || (max_value_offset < offset_limit)))
                max_ifd_offset = max_value_offset;
        }
        /* =========== end show ifd image ============                */

        /* ###%%% temporary debug                                     */
        if(Debug & END_OF_SECTION_DEBUG)
            printf("mo=%lu, mvo=%lu, mio=%lu, nio=%lu, ol=%lu\n",
                    max_offset,max_value_offset,max_ifd_offset,next_ifd_offset,offset_limit);

        /* lock the summary entry for this ifd, even if it doesn't    */
        /* describe an actual image                                   */
        if(summary_entry && (summary_entry->entry_lock  <= 0))
            summary_entry->entry_lock = lock_number(summary_entry);

        /* Prepare for the next ifd                                   */
        if(ifdtype == TIFF_IFD)
        {
            ++ifdnum;
            subifdnum = -1;
        }
        else if(ifdtype == TIFF_SUBIFD)
            ++subifdnum;
        if(fulldirname)
            free(fulldirname);
        fulldirname = CNULL;

        /* If there is a next ifd, "max_offset" must be reset.        */
        if(next_ifd_offset)
            max_offset = 0;
    }
    newline(chpr);

    return(max_ifd_offset);
blewit:
    clearerr(inptr);
    if(PRINT_SECTION)
    {
        /* Indicate the end of the IFD                                    */
        indent -= SMALLINDENT;
        current_offset = ftell(inptr);
        if((prefix && (*prefix == '>')) || (max_offset == 0) ||
                                                    (max_ifd_offset < max_offset))
        {
            display_end_of_ifd(max_ifd_offset,ifdtype,ifdnum,subifdnum,indent,
                                                            ifdname,prefix);
        }
        else
            display_end_of_ifd(max_offset,ifdtype,ifdnum,subifdnum,indent,
                                                            ifdname,prefix);
    }
    if(fulldirname)
        free(fulldirname);
    return(0L);
}

/* Create a summary entry and link it into a list of image            */
/* descriptions to be printed at the end of the file output.          */

struct image_summary *
new_summary_entry(struct image_summary *prev_entry,int fileformat,int datatype)
{
    struct image_summary *new = NULL;
    int chpr = 0;

    /* Make entries only for types which describe image data, i.e.    */
    /* not EXIF, Interop, etc.                                        */
    switch(datatype)
    {
        case EXIF_IFD:
        case INTEROP_IFD:
        case GPS_IFD:
            break;
        default:
            if(Debug & SCAN_DEBUG)
            {
                chpr = newline(1);
                printred("DEBUG: NEW SUMMARY ");
            }
            new = (struct image_summary *)malloc(sizeof(struct image_summary));
            if(new)
            {
                if(Debug & SCAN_DEBUG)
                    printf("%#lx\n",(unsigned long)new);
                memset(new,0,sizeof(struct image_summary));
                new->datatype = datatype;
                new->ifdnum = -1;
                new->subifdnum = -1;
                new->length = 0;
                new->offset = 0;
                new->subfiletype = -1;
                memset(new->filesubformatAPPN,0,16);
                if(prev_entry)
                {
                    if(Debug & SCAN_DEBUG)
                    {
                        printred("DEBUG: PREV ENTRY ");
                        chpr += printf("%#x, next=%#x, offset=%ld, lock=%d",
                                        (unsigned int)prev_entry,
                                        (unsigned int)prev_entry->next_entry,
                                        prev_entry->offset,prev_entry->entry_lock);
                        chpr = newline(chpr);
                    }
                    /* make sure to attach at the end of the chain        */
                    while(prev_entry->next_entry)
                    {
                        prev_entry = prev_entry->next_entry;
                        if(Debug & SCAN_DEBUG)
                        {
                            printred(" PREV ENTRY ");
                            chpr += printf("%#x, next=%#x, offset=%ld, lock=%d",
                                            (unsigned int)prev_entry,
                                            (unsigned int)prev_entry->next_entry,
                                            prev_entry->offset,prev_entry->entry_lock);
                            chpr = newline(chpr);
                        }
                    }
                    new->prev_entry = prev_entry;
                    if(Debug & SCAN_DEBUG)
                    {
                        printred("DEBUG: FOUND PREV ENTRY ");
                        chpr += printf("%#x, next=%#x, offset=%ld, lock=%d",
                                            (unsigned int)prev_entry,
                                            (unsigned int)prev_entry->next_entry,
                                            prev_entry->offset,prev_entry->entry_lock);
                        chpr = newline(chpr);
                    }
                    if(prev_entry->fileformat)
                        new->fileformat = prev_entry->fileformat;
                    prev_entry->next_entry = new;
                    if(Debug & SCAN_DEBUG)
                    {
                        printred("DEBUG: NEW ENTRY ");
                        chpr += printf(" next=%#x, prev offset=%d, prev lock=%d",
                                            (unsigned int)prev_entry->next_entry,
                                            (unsigned int)prev_entry->offset,prev_entry->entry_lock);
                        chpr = newline(chpr);
                    }
                }
                else
                    new->fileformat = fileformat;
            }
            else
            {
                printred("Memory allocation for image summary failed");
                if(prev_entry)
                    chpr += printf(" summary will be incomplete");
                else
                    chpr += printf(" no summary will be printed");
                newline(chpr);
            }
            if(Debug & SCAN_DEBUG)
                chpr = newline(1);
            break;
    }
    return(new);
}

/* Move to the end of the summary entry chain passed as argument      */
struct image_summary *
last_summary_entry(struct image_summary *summary_entry)
{
    if(summary_entry)
    {
        while(summary_entry->next_entry)
            summary_entry = summary_entry->next_entry;
    }
    return(summary_entry);
}

/* Lock an entry with a sequence number, primarily so entries can be  */
/* tracked while debugging. A locked entry is "in use', requiring     */
/* creation of a new entry for a new image                            */

int
lock_number(struct image_summary *summary_entry)
{
    int number = 0;
    struct image_summary *prev_entry,*last_entry;

    if(summary_entry)
    {
        if(summary_entry->entry_lock)
            number = summary_entry->entry_lock;  /* don't change it */
        else if((last_entry = last_summary_entry(summary_entry)))
        {
            /* Find the highest existing lock number; go to the end   */
            /* and scan back to the head                              */
            if(last_entry->entry_lock > 0)
                number = last_entry->entry_lock + 1;
            prev_entry = last_entry->prev_entry;
            while(prev_entry)
            {
                if(prev_entry->entry_lock >= number)
                    number = prev_entry->entry_lock + 1;
                prev_entry = prev_entry->prev_entry;
            }
            if(number <= 0)
                number = 1;
        }
        else
            number = 1;
    }
    if(Debug & SCAN_DEBUG)
    {
        (void)newline(1);
        printred("DEBUG: LOCK SUMMARY");
        printf(" %#x ==> %d",(unsigned int)summary_entry,number);
        (void)newline(1);
        (void)newline(1);
    }
    return(number);
}
               

struct image_summary *
destroy_summary(struct image_summary *summary_entry)
{
    struct image_summary *prev_entry = NULL;

    /* start at the end and work back to the head                     */
    if(Debug & SCAN_DEBUG)
        printred("DEBUG: DESTROY SUMMARY ");
    if(summary_entry)
    {
        while(summary_entry->next_entry)
            summary_entry = summary_entry->next_entry;
        while(summary_entry)
        {
            prev_entry = summary_entry->prev_entry;
            free(summary_entry);
            summary_entry = prev_entry;
        }
    }
    return(prev_entry);
}


/* Read, decode, and print an EXIF IFD at 'exif_offset', noticing     */
/* MakerNotes and "Interoperability" IFDs as they go by. The offset   */
/* of each entry and offset value is recorded and printed, and the    */
/* offset of the last byte read is reported as the end of the IFD. If */
/* all goes well, the offset at which the caller should next read     */
/* (just past the end of the offset values) is returned. If an        */
/* unrecoverable error occurs, 0UL is returned.                       */

/* This looks remoarkably like process_tiff_ifd(), except that it     */
/* expects EXIF tags, and doesn't expect further IFDs to be chained   */
/* from it. If it does find a chained IFD, it processes it as a       */
/* normal IFD (not another EXIF IFD). It should probably just warn    */
/* and ignore it.                                                     */

unsigned long
process_exif_ifd(FILE *inptr,unsigned short byteorder,
                    unsigned long exif_offset,unsigned long fileoffset_base,
                    unsigned long max_offset,struct image_summary *summary_entry,
                    char *parent_name,int ifdnum,int indent)
{
    struct ifd_entry *entry_ptr;
    unsigned long max_value_offset = 0L;
    unsigned long next_ifd_offset,current_offset,max_exif_offset;
    unsigned long start_entry_offset,entry_offset,value_offset;
    unsigned long value_start_offset = 0UL;
    unsigned long next_max_offset = 0UL;
    int entry_num,num_entries,value_is_offset,use_second_pass;
    int chpr = 0;
    char *prefix;
    char *fulldirname = CNULL;

    if(inptr == (FILE *)0)
    {
        fprintf(stderr,"%s: no open file pointer to read EXIF IFD\n",
                Progname);
        return(0L);
    }

    PUSHCOLOR(EXIF_COLOR);
    /* If the exif segment appears to be beyond the end of the parent */
    /* ifd, mark the start address to call attention.                 */
    if(max_offset && (exif_offset > max_offset))
        prefix = "+";
    else
        prefix = "@";
    if(summary_entry)
        summary_entry->filesubformat |= FILESUBFMT_EXIF;
    max_exif_offset = max_offset;
    print_tag_address(SECTION,exif_offset + fileoffset_base,indent,prefix);
    if(PRINT_SECTION)
        chpr += printf("<EXIF IFD> (in IFD %d)",ifdnum);

    num_entries = read_ushort(inptr,byteorder,exif_offset + fileoffset_base);
    if(ferror(inptr) || feof(inptr))
        goto blewit;
    current_offset = entry_offset = start_entry_offset = ftell(inptr);
    if(PRINT_SECTION)
    {
        chpr += printf(" %d entries ",num_entries);
        chpr += printf("starting at file offset %#lx=%lu",
                                            current_offset,current_offset);
            chpr = newline(chpr);
    }
    use_second_pass = value_is_offset = 0;
    fulldirname = splice(parent_name,".","Exif");

    indent += SMALLINDENT;
    /* The direct entries                                             */
    for(entry_num = 0; entry_num < num_entries; ++entry_num)
    {
        entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
        if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
            ferror(inptr) || feof(inptr))
        {
            int invalid_entry = 0;
            unsigned long limit_offset = 0L;

            print_tag_address(ENTRY,entry_offset,indent,prefix);
            chpr += printf(" INVALID EXIF ENTRY (%lu)",entry_ptr->value);
            chpr = newline(chpr);
            clearerr(inptr);
            current_offset = ftell(inptr);
            if(max_offset > 0)
                limit_offset = max_offset;
            else
            {
                if(fseek(inptr,0L,SEEK_END) != -1)
                {
                    limit_offset = ftell(inptr);
                    fseek(inptr,current_offset,SEEK_SET);
                }
            }
            /* If there's an error on input, or we can't check    */
            /* for absurd num_entries, give up.                   */
            if(!ferror(inptr) && !feof(inptr) && (limit_offset > 0))
            {
                /* If the number of entries would read past the   */
                /* size of the IFD, or past EOF, give up          */
                if((entry_offset + (12 * num_entries)) < limit_offset)
                {
                    /* Limit the number of consecutive failures.  */
                    /* An apparently valid entry resets the count */
                    /* to 0.                                      */
                    if(invalid_entry++ < MAX_INVALID_ENTRIES)
                    {
                        entry_offset = current_offset;
                        continue;
                    }
                }
            }
            chpr = newline(chpr);
            goto blewit;
        }
        current_offset = ftell(inptr);
        switch(entry_ptr->tag)
        {
            case EXIFTAG_PixelXDimension:
                if(summary_entry)
                    summary_entry->primary_width = entry_ptr->value;
                break;
            case EXIFTAG_PixelYDimension:
                if(summary_entry)
                    summary_entry->primary_height = entry_ptr->value;
                break;
            case EXIFTAG_Interoperability: 
            case EXIFTAG_MakerNote: 
                use_second_pass++;
                break;
            default:
                break;
        }
     
        print_tag_address(ENTRY,entry_offset,indent,prefix);
        value_offset = print_entry(inptr,byteorder,entry_ptr,fileoffset_base,
                                        summary_entry,fulldirname,EXIF_IFD,
                                        ifdnum,-1,SMALLINDENT);
        if(value_offset == 0UL)
            value_offset = current_offset;
        if(value_offset > max_value_offset)
            max_value_offset = value_offset;
        if((PRINT_VALUE_AT_OFFSET) && (is_offset(entry_ptr)))
            ++use_second_pass;
        entry_offset = current_offset;
    }

    next_ifd_offset = read_ulong(inptr,byteorder,current_offset);

    /* There should be no next_ifd_offset for the Exif Ifd.           */
    /* I am not confident, however, that someone won't eventually     */
    /* chain Exif ifds, so just handle them, and let the error        */
    /* messages fly.                                                  */
    if(next_ifd_offset > 0L)
    {
        if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(SECTION,current_offset,indent + SMALLINDENT,prefix);
            chpr += printf("**** next IFD offset %lu  ",next_ifd_offset);
            next_ifd_offset += fileoffset_base;
            if(next_ifd_offset < ftell(inptr))
            {
                printred("BAD NEXT IFD OFFSET");
                next_ifd_offset = 0L;
            }
            else
                chpr += printf("(+ %lu = %#lx/%lu)",fileoffset_base,
                                        next_ifd_offset,next_ifd_offset);
            chpr = newline(chpr);
        }
        else
            next_ifd_offset += fileoffset_base;
        /* This should never happen                                   */
        max_exif_offset = next_ifd_offset;
    }
    else
    {
        if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(SECTION,current_offset,indent,prefix);
            chpr += printf("**** next IFD offset 0");
            chpr = newline(chpr);
        }
    }

    if(ferror(inptr) || feof(inptr))
    {
        chpr += printf(" READ NEXT IFD OFFSET FAILED ");
        chpr = newline(chpr);
        why(stdout);
        clearerr(inptr);    /* keep going...                          */
    }
    else
        current_offset = ftell(inptr);

    value_offset = current_offset;

    /* Second pass, to evaluate entries which are stored indirectly.  */
    /* This occurs when the value requires more than 4 bytes, AND for */
    /* certain LONG values which are intended to be used as offsets   */
    /* (and should have their own type defined in the spec),          */
    if(use_second_pass)
    {
        if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(SECTION,value_offset,indent,prefix);
            chpr += printf("============= VALUES, EXIF IFD ============");
            chpr = newline(chpr);
        }

        entry_offset = start_entry_offset;
        for(entry_num = 0; entry_num < num_entries; ++entry_num)
        {
            entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
            if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                ferror(inptr) || feof(inptr))
            {
                /* If the first pass made it through invalid entries, */
                /* this pass should just ignore them and quietly      */
                /* continue.                                          */
                entry_offset = current_offset = ftell(inptr);
                continue;
            }
            current_offset = entry_offset = ftell(inptr);
            switch(entry_ptr->tag)
            {
                case EXIFTAG_Interoperability: 
                    PUSHCOLOR(INTEROP_COLOR);
                    value_offset = process_tiff_ifd(inptr,byteorder,
                                            entry_ptr->value,fileoffset_base,
                                            max_exif_offset,summary_entry,
                                            fulldirname,INTEROP_IFD,ifdnum,0,
                                            indent);
                    POPCOLOR();
                    break;
                case EXIFTAG_MakerNote: 
        /* ###%%% temporary debug                                     */
                    if(Debug & END_OF_SECTION_DEBUG)
                        printf("mo=%lu, mvo=%lu, ",
                                max_offset,max_value_offset);
                    value_start_offset = entry_ptr->value + fileoffset_base;
                    if(max_offset && (value_start_offset > max_offset) &&
                            (entry_ptr->count > 4))
                    {
                        next_max_offset = value_start_offset;
                    }
                    if(Debug & END_OF_SECTION_DEBUG)
                        printf("vso=%lu, nmo=%lu\n",value_start_offset,next_max_offset);
                    value_offset = process_makernote(inptr,byteorder,
                                            entry_ptr,fileoffset_base,max_exif_offset,
                                                    summary_entry,fulldirname,indent);
                    if(Debug & END_OF_SECTION_DEBUG)
                        printf("mo=%lu, mvo=%lu, ",max_offset,max_value_offset);
                    if(Debug & END_OF_SECTION_DEBUG)
                        printf("MNvo=%lu, mo=%lu\n",value_offset,max_offset);
                    break;
                default:
                    if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && is_offset(entry_ptr)))
                    {
                        print_tag_address(ENTRY,fileoffset_base + entry_ptr->value,
                                            indent,prefix);
                        print_tagid(entry_ptr,SMALLINDENT,EXIF_IFD);
                        value_offset =
                                    print_offset_value(inptr,byteorder,entry_ptr,
                                                    fileoffset_base,fulldirname,
                                                    EXIF_IFD,indent,1);
                    }
                    break;
            }
            if(value_offset > max_value_offset)
                max_value_offset = value_offset;
            if(max_offset && (value_offset > max_offset))
            {
                next_max_offset = value_start_offset;
                if(Debug & END_OF_SECTION_DEBUG)
                    printf("nmo=%lu\n",next_max_offset);
            }
        }
    }
    else if(current_offset > max_value_offset)
        max_value_offset = current_offset;

    indent -= SMALLINDENT;
    if(max_offset && (max_value_offset > max_offset))
        print_tag_address(SECTION,max_offset - 1,indent,"<");
    else
        print_tag_address(SECTION,max_value_offset - 1,indent,"-");
    if(PRINT_SECTION)
    {
        chpr += printf("</EXIF IFD>");
        chpr = newline(chpr);
    }
    POPCOLOR();

    /* This shouldn't happen.                                         */
    if(next_ifd_offset)
    {
        printred("#========= WARNING! CHAINING NEXT IFD FROM EXIF IFD =========");
        chpr = newline(1);
        value_offset = process_tiff_ifd(inptr,byteorder,next_ifd_offset,
                                fileoffset_base,max_offset,summary_entry,fulldirname,
                                EXIF_IFD,++ifdnum,-1,indent);
        if(value_offset > max_value_offset)
            max_value_offset = value_offset;
    }
    if(fulldirname)
        free(fulldirname);
    if(next_max_offset)
        max_value_offset = next_max_offset;
    if(Debug & END_OF_SECTION_DEBUG)
        printf("exmvo=%lu\n",max_value_offset);
    return(max_value_offset);
blewit:
    clearerr(inptr);
    current_offset = ftell(inptr);
    print_tag_address(SECTION,current_offset - 1,indent,"-");
    if(PRINT_SECTION)
    {
        chpr += printf("</EXIF IFD>");
        chpr = newline(chpr);
    }
    if(fulldirname)
        free(fulldirname);
    POPCOLOR();
    return(0L);
}

/* Read an ifd entry describing one or more subifd offsets, and       */
/* process the IFDs found at those offsets as TIFF image file         */
/* directories. This is used for TIFF6/TIFFEP SubIFDtags. The return  */
/* is 0L if something went wrong.                                     */

unsigned long
process_subifd(FILE *inptr,unsigned short byteorder,
                    struct ifd_entry *subfile_entry,
                    unsigned long fileoffset_base,unsigned long max_offset,
                    struct image_summary *summary_entry,char *parent_name,
                    int ifdnum,int subifdnum,int ifdtype,int indent)
{
    unsigned long entry_offset;
    unsigned long subifd_offset;
    unsigned long max_value_offset = 0L;
    int chpr = 0;
    int nsubifds,num;

    if(inptr && subfile_entry)
    {
        entry_offset = subfile_entry->value + fileoffset_base;
        nsubifds = subfile_entry->count;

        if(subifdnum >= 0)
        {
            if(nsubifds > 1)
            {
                if((PRINT_VALUE_AT_OFFSET))
                {
                    if(max_offset && (entry_offset > max_offset))
                        print_tag_address(SECTION,entry_offset,indent,"+");
                    else
                        print_tag_address(SECTION,entry_offset,indent,"@");
                    if(PRINT_SECTION)
                    {
                        chpr += printf("%s - %lu offsets: [max parent offset = %lu]",
                                tagname(subfile_entry->tag),subfile_entry->count,
                                max_offset);
                    }
                    for(num = 0; num < nsubifds; ++num)
                    {
                        subifd_offset = read_ulong(inptr,byteorder,entry_offset)
                                                            + fileoffset_base;
                        entry_offset = ftell(inptr); /* where to get next one */
                        if((PRINT_SECTION))
                            chpr += printf(" %lu",subifd_offset);
                        if(ferror(inptr) || feof(inptr))
                            break;
                        /* goto blewit;                               */
                    }
                    if(PRINT_SECTION)
                        chpr = newline(chpr);
                }

                entry_offset = subfile_entry->value + fileoffset_base;
                for(num = 0; num < nsubifds; ++num)
                {
                    subifd_offset = read_ulong(inptr,byteorder,entry_offset) +
                                                        fileoffset_base;
                    /* where to get next one                          */
                    entry_offset = ftell(inptr);
                    /* The next subifd offset suggests a maximum for  */
                    /* this subifd; at least until some nitwit writes */
                    /* the second one first. Doesn't help for the     */
                    /* last one, though.                              */
                    if(num < (nsubifds - 1))
                        max_offset = read_ulong(inptr,byteorder,HERE);
                    else
                        max_offset = 0;
                    if(ferror(inptr) || feof(inptr))
                        goto blewit;
                    max_value_offset = process_tiff_ifd(inptr,byteorder,
                                            subifd_offset,fileoffset_base,
                                            max_offset,summary_entry,parent_name,
                                            ifdtype,ifdnum,subifdnum + num,
                                            indent);
                    /* ###%%% don't we check for errors here???           */
                }
            }
            else
            {
                max_value_offset = process_tiff_ifd(inptr,byteorder,
                                            subfile_entry->value,fileoffset_base,
                                            max_offset,summary_entry,parent_name,
                                            ifdtype,ifdnum,subifdnum,
                                            indent);
            }
        }
        else
        {
            printred("Negative subifd number");
            chpr += printf(" %d, ifdtype %d not processed for IFD %d",subifdnum,ifdtype,ifdnum);
            chpr = newline(chpr);
            goto blewit;
        }
    }
    else
    {
        fprintf(stderr,"%s: no open file pointer to read EXIF\n",Progname);
        max_value_offset = 0L;
    }
    setcharsprinted(chpr);
    return(max_value_offset);
blewit:
    /* we're going to keep trying, because the caller's next offset   */
    /* may be ok.                                                     */
    clearerr(inptr);
    return(0L);
}

/* Just a bare bones GPS routine; no interpretation is done.          */

unsigned long
process_gps_ifd(FILE *inptr,unsigned short byteorder,
                    unsigned long gps_offset,unsigned long fileoffset_base,
                    unsigned long max_offset,struct image_summary *summary_entry,
                    char *parent_name,int ifdnum,int indent)
{
    struct ifd_entry *entry_ptr;
    unsigned long max_value_offset = 0L;
    unsigned long next_ifd_offset,current_offset,max_gps_offset;
    unsigned long start_entry_offset,entry_offset,value_offset;
    int entry_num,num_entries,value_is_offset,use_second_pass;
    int chpr = 0;
    char *fulldirname = CNULL;

    if(inptr == (FILE *)0)
    {
        fprintf(stderr,"%s: no open file pointer to read GPS IFD\n",
                Progname);
        return(0L);
    }

    PUSHCOLOR(GPS_COLOR);
    /* If the gps segment appears to be beyond the start of the       */
    /* parent ifd, mark the start address funny.                      */
    if(max_offset && (gps_offset > max_offset))
        print_tag_address(SECTION,gps_offset + fileoffset_base,indent,"^");
    else
        print_tag_address(SECTION,gps_offset + fileoffset_base,indent,"@");
    if(PRINT_SECTION)
        chpr += printf("<GPS IFD> (in IFD %d)",ifdnum);

    num_entries = read_ushort(inptr,byteorder,gps_offset + fileoffset_base);
    if(ferror(inptr) || feof(inptr))
        goto blewit;
    current_offset = entry_offset = start_entry_offset = ftell(inptr);
    if(PRINT_SECTION)
    {
        chpr += printf(" %d entries ",num_entries);
        chpr += printf("starting at file offset %#lx=%lu",
                                        current_offset,current_offset);
        chpr = newline(chpr);
    }
    use_second_pass = value_is_offset = 0;
    fulldirname = splice(parent_name,".","Gps");

    indent += SMALLINDENT;
    /* The direct entries                                             */
    for(entry_num = 0; entry_num < num_entries; ++entry_num)
    {
        print_tag_address(ENTRY,entry_offset,indent,"@");
        entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
        if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
            ferror(inptr) || feof(inptr))
        {
            print_tag_address(ENTRY,entry_offset,indent,"@");
            chpr += printf(" INVALID (%lu)",entry_ptr->value);
            chpr = newline(chpr);
            /* ###%%% replace with check code from process_tiff_ifd() */
            goto blewit;
        }
        entry_offset = current_offset = ftell(inptr);
        value_offset = print_entry(inptr,byteorder,entry_ptr,fileoffset_base,
                                            summary_entry,fulldirname,GPS_IFD,
                                            ifdnum,-1,SMALLINDENT);
        if(value_offset == 0UL)
            value_offset = entry_offset;
        if(value_offset > max_value_offset)
            max_value_offset = value_offset;
        if((PRINT_VALUE_AT_OFFSET) && (is_offset(entry_ptr)))
            ++use_second_pass;
    }

    next_ifd_offset = read_ulong(inptr,byteorder,current_offset);

    /* offsets found in the next pass should be within the bounds of  */
    /* the ifd. The following helps to detect chunks that are written */
    /* "out of place".                                                */
    if(next_ifd_offset > 0L)
    {
        if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(SECTION,current_offset,indent,"@");
            chpr += printf("**** next IFD offset %lu  ",next_ifd_offset);
            next_ifd_offset += fileoffset_base;
            if(next_ifd_offset < ftell(inptr))
            {
                printred("BAD NEXT IFD OFFSET");
                next_ifd_offset = 0L;
            }
            else
                chpr += printf("(+ %lu = %#lx/%lu)",fileoffset_base,
                            next_ifd_offset,next_ifd_offset);
            chpr = newline(chpr);
        }
        else
            next_ifd_offset += fileoffset_base;
        /* This should never happen                                   */
        max_gps_offset = next_ifd_offset;
        if(max_offset && (max_gps_offset > max_offset))
            max_gps_offset = max_offset;
    }
    else
    {
        max_gps_offset = max_offset;    
        if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(SECTION,current_offset,indent,"@");
            chpr += printf("**** next IFD offset 0");
            chpr = newline(chpr);
        }
    }

    if(ferror(inptr) || feof(inptr))
    {
        chpr += printf(" READ NEXT IFD OFFSET FAILED ");
        chpr = newline(chpr);
        why(stdout);
        clearerr(inptr);    /* keep going...                          */
    }
    else
        current_offset = ftell(inptr);

    value_offset = current_offset;

    /* Make a second pass to print offset values.                     */
    if(use_second_pass)
    {
        if((PRINT_VALUE) && (PRINT_SECTION) && (PRINT_VALUE_AT_OFFSET))
        {
            print_tag_address(SECTION,value_offset,indent,"@");
            chpr += printf("============= VALUES, GPS IFD ============");
            chpr = newline(chpr);
        }

        /* Second pass, to evaluate entries which are stored          */
        /* indirectly (the value requires more than 4 bytes).         */
        entry_offset = start_entry_offset;
        for(entry_num = 0; entry_num < num_entries; ++entry_num)
        {
            entry_ptr = read_ifd_entry(inptr,byteorder,entry_offset);
            if((entry_ptr->value_type == 0) || (entry_ptr->value_type > DOUBLE) ||
                ferror(inptr) || feof(inptr))
            {
                print_tag_address(ENTRY,entry_offset,indent,"@");
                chpr += printf(" INVALID (%lu)",entry_ptr->value);
                chpr = newline(chpr);
                goto blewit;
            }
            current_offset = entry_offset = ftell(inptr);
            switch(entry_ptr->tag)
            {
                default:
                    if((PRINT_VALUE) && ((PRINT_VALUE_AT_OFFSET) && is_offset(entry_ptr)))
                    {
                        print_tag_address(ENTRY,fileoffset_base + entry_ptr->value,
                                            indent,"@");
                        print_tagid(entry_ptr,SMALLINDENT,GPS_IFD);
                        value_offset =
                                    print_offset_value(inptr,byteorder,entry_ptr,
                                                    fileoffset_base,fulldirname,
                                                    GPS_IFD,indent,1);
                    }
                    break;
            }
            if(value_offset > max_value_offset)
                max_value_offset = value_offset;
        }
    }
    else if(current_offset > max_value_offset)
        max_value_offset = current_offset;

    indent -= SMALLINDENT;
    if(max_offset && (gps_offset > max_offset))
    {
        print_tag_address(SECTION,max_value_offset - 1,indent,">");
        max_value_offset = 0L;  /* let the caller know.               */
    }
    else
        print_tag_address(SECTION,max_value_offset - 1,indent,"-");
    if(PRINT_SECTION)
    {
        chpr += printf("</GPS IFD>");
        chpr = newline(chpr);
    }
    POPCOLOR();
    if(next_ifd_offset)
    {
        printred("#========= WARNING! CHAINING NEXT IFD FROM GPS IFD =========");
        chpr = newline(1);
        value_offset = process_tiff_ifd(inptr,byteorder,next_ifd_offset,
                                fileoffset_base,max_offset,NULL,fulldirname,
                                GPS_IFD,-1,-1,indent);
        if(value_offset > max_value_offset)
            max_value_offset = value_offset;
    }
    if(fulldirname)
        free(fulldirname);
    return(max_value_offset);
blewit:
    clearerr(inptr);
    current_offset = ftell(inptr);
    print_tag_address(SECTION,current_offset - 1,indent,"-");
    if(PRINT_SECTION)
    {
        chpr += printf("</GPS IFD>");
        chpr = newline(chpr);
    }
    if(fulldirname)
        free(fulldirname);
    POPCOLOR();
    return(0L);
}

/* Process JPEG the marker segments between JPEG SOI and EOI.         */
/* For the moment, that means just printing them (offset, id, length) */
/* and then moving to the next tag.                                   */

/* The first marker has been read, it's identity and offset are       */
/* passed in as arguments. No assumptions are made about the current  */
/* file pointer; the first read always begins at the passed           */
/* marker_offset.                                                     */

/* APP section are noticed, and APP0, APP1, and APP12 section         */
/* processed appropriately. Other APP section are reported without    */
/* expansion of the section, but may be hex/ascii dumped by option.   */

extern unsigned long forward_scan_for_eoi(FILE *,unsigned long,
                            unsigned long,char *,int);

unsigned long
process_jpeg_segments(FILE *inptr,unsigned long marker_offset,unsigned short tag,
                        unsigned long data_length,struct image_summary *summary_entry,
                        char *parent_name,char *prefix,int indent)
{
    static unsigned long img_pixels = 0L;
    unsigned long max_offset = 0L;
    unsigned long dumplength = 0L;
    unsigned long start_of_jpeg_data;
    unsigned long found_eoi_offset,after_sos,end_of_section,eof;
    unsigned short seg_length,tmp;
    unsigned short img_height = 0;
    unsigned short img_width = 0;
    unsigned short bad_soi = 0;
    int chpr = 0;
    int had_soi = 0;
    int status = 0;
    int i;
    char *name;
    static unsigned long max_eoi = 0UL;

    if(inptr)
    {
        start_of_jpeg_data = marker_offset;
        if(start_of_jpeg_data == 0UL)
            max_eoi = 0UL;
        if((summary_entry == NULL) ||
                summary_entry->entry_lock ||
                        (summary_entry->imageformat != IMGFMT_NOIMAGE))
        {
            summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG);
        }
        if(summary_entry)
        {
            if(summary_entry->length <= 0)
                summary_entry->length = data_length;
            if(summary_entry->offset <= 0)
                summary_entry->offset = start_of_jpeg_data;
            summary_entry->imageformat = IMGFMT_JPEG;
            summary_entry->entry_lock = lock_number(summary_entry);
            if(tag == 0)
            {
                clearerr(inptr);
                (void)jpeg_status(JPEG_NO_SOI);
                if(summary_entry)
                    summary_entry->imagesubformat |= IMGSUBFMT_NO_JPEG;
            }
        }
        PUSHCOLOR(JPEG_COLOR);
        while(tag != 0)
        {
            switch(tag)
            {
                case JPEG_SOS:  /* image data                         */
                    name = tagname(tag);
                    if(PRINT_SEGMENT)
                    {
                        print_tag_address(SEGMENT,marker_offset,indent + SMALLINDENT,prefix);
                        chpr += printf("<%s>",name);
                    }
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    if(!ferror(inptr) && !feof(inptr))
                    {
                        unsigned char ns,c,t,td,ta,ss,se,a,ah,al;
                        int i;

                        /* number of components in scan               */
                        ns = read_ubyte(inptr,HERE);
                        if(PRINT_SEGMENT)
                        {
                            chpr += printf(" length %u",seg_length);
                            chpr += printf("  start of JPEG data, %d components %lu pixels",
                                                                      ns & 0xff,img_pixels);
                            chpr = newline(chpr);
                        }
                        if(Debug & JPEG_MARKER_DEBUG)
                        {
                            for(i = 0; i < ns; ++i)
                            {
                                /* ###%%% */
                                putindent(6 + (2 * ADDRWIDTH) + indent + 16);
                                /* component selector                 */
                                c = read_ubyte(inptr,HERE); 
                                t = read_ubyte(inptr,HERE);
                                td = t & 0xf;
                                ta = ((t & 0xf0) >> 4) & 0xf;
                                if((ferror(inptr) == 0) && (feof(inptr) == 0) && (PRINT_SEGMENT))
                                {
                                    chpr += printf("Cs%d=%d, Td%d=%#04x, Ta%d=%#04x",i,c,i,td,i,ta);
                                    chpr = newline(chpr);
                                }
                            }
                            ss = read_ubyte(inptr,HERE); 
                            se = read_ubyte(inptr,HERE); 
                            a = read_ubyte(inptr,HERE);
                            ah = a & 0xf;
                            al = ((a & 0xf0) >> 4) & 0xf;
                            putindent(6 + (2 * ADDRWIDTH) + indent + 16);
                            chpr += printf("Ss=%u, Se=%u, Ah=%u, Al=%u",ss,se,ah,al);
                            chpr = newline(chpr);
                        }
                    }
                    else
                    {
                        tag = 0;
                        chpr += printf("ERROR reading length of SOS");
                        chpr = newline(chpr);
                        why(stdout);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                        continue;
                    }
                    if(data_length > 0L)
                    {   

                        after_sos = marker_offset + 2 + seg_length;
                        marker_offset = start_of_jpeg_data + data_length - 2L;
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                        if(ferror(inptr) || feof(inptr))
                        {
                            tag = 0;
                            printred("# WARNING: ERROR reading JPEG_EOI at end of section");
                            chpr = newline(chpr);
                            clearerr(inptr);
                            /* Skip the backward scan and check       */
                            /* forward from the end of the SOS        */
                            /* section, to see where the file is      */
                            /* truncated.                             */
                            status = JPEG_HAD_ERROR;
                            (void)jpeg_status(status);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
#if 0
/* Wrong if multiple compressed tiles/strips in passed length; status */
/* set to HAD_ERROR allows skippping backward scan and going direct   */
/* to forward scan. Should pass just tile/strip length and handle     */
/* multiples in parent.                                               */
                            continue;
#endif
                        }
                        if(tag == JPEG_EOI)
                        {
                            max_offset = ftell(inptr);
                            continue;
                        }
                        else    /* Try a little harder to find EOI    */
                        {
                            int i;

                            end_of_section = start_of_jpeg_data + data_length;
                            if(status == 0)
                            {
                                /* Try a quick scan backward from the end */
                                /* of the section, in case there's just a */
                                /* little padding.                        */
                                for(i = 1; i < 512; ++i)
                                {
                                    tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                                    if(Debug & JPEG_EOI_DEBUG)
                                        printf("DEBUG: %#lx/%ld: %#04x\n",marker_offset,marker_offset,tag);
                                    if(ferror(inptr) || feof(inptr))
                                        break;
                                    status = 0;
                                    if(tag == JPEG_EOI)
                                        break;
                                    --marker_offset;
                                }
                                if(ferror(inptr) || feof(inptr))
                                {
                                    tag = 0;
                                    printred("# WARNING: ERROR reading JPEG data looking for JPEG_EOI");
                                    chpr = newline(chpr);
                                    clearerr(inptr);
                                    (void)jpeg_status(JPEG_HAD_ERROR);
                                    if(summary_entry)
                                        summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                                    continue;
                                }
                                if(tag == JPEG_EOI)
                                {
                                    if(start_of_jpeg_data > 0UL)
                                    {
                                        if(feof(inptr) || ((end_of_section + 5) >= get_filesize(inptr)))
                                            max_eoi = start_of_jpeg_data;
                                    }
                                    max_offset = ftell(inptr);
                                    (void)jpeg_status(JPEG_EARLY_EOI);
                                    if(summary_entry)
                                        summary_entry->imagesubformat |= IMGSUBFMT_JPEG_EARLY_EOI;
                                    if(Debug & JPEG_EOI_DEBUG)
                                        printf("DEBUG: start_of_jpeg_data=%lu, max_eoi=%lu\n",start_of_jpeg_data,max_eoi);
                                    continue;
                                }
                            }
                            /* If the quick scan didn't work, try a   */
                            /* forward scan from the offset given by  */
                            /* the start of scan header.              */
                            found_eoi_offset = forward_scan_for_eoi(inptr,after_sos,
                                                            end_of_section,"!",indent);
                            if(start_of_jpeg_data > 0UL)
                            {
                                if(feof(inptr) || ((found_eoi_offset + 5) >= get_filesize(inptr)))
                                    max_eoi = start_of_jpeg_data;
                            }
                            if(Debug & JPEG_EOI_DEBUG)
                                printf("DEBUG: start_of_jpeg_data=%lu, max_eoi=%lu\n",start_of_jpeg_data,max_eoi);
                            if(found_eoi_offset)
                            {
                                max_offset = found_eoi_offset + 2;
                                (void)jpeg_status(JPEG_EARLY_EOI);
                                if(summary_entry)
                                    summary_entry->imagesubformat |= IMGSUBFMT_JPEG_EARLY_EOI;
                                marker_offset = max_offset - 2UL;
                                tag = JPEG_EOI;
                                continue;
                            }
                            else
                            {
                                if(ferror(inptr) || feof(inptr))
                                {
                                    tag = 0;
                                    clearerr(inptr);
                                    (void)jpeg_status(status);
                                    if(summary_entry)
                                        summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                                    continue;
                                }
                                else
                                {
                                    max_offset = 0;
                                    (void)fseek(inptr,start_of_jpeg_data + data_length - 2L,SEEK_SET);
                                    (void)jpeg_status(JPEG_NO_EOI);
                                }
                            }
                        }
                    }
                    else
                    {
                        /* See if there's a JPEG_EOI at (or somewhere */
                        /* near) EOF. If a previous subimage was      */
                        /* found at or very near eof, indicating a    */
                        /* thumbnail tacked on to the end of the      */
                        /* file, use the start of that image as the   */
                        /* limit for the search,                      */
                        if(Debug & JPEG_EOI_DEBUG)
                            printf("DEBUG: max_eoi=%lu\n",max_eoi);
                        if(fseek(inptr,0L,SEEK_END) == 0)
                        {
                            /* Values to use if a forward scan is     */
                            /* required.                              */
                            after_sos = marker_offset + 2 + seg_length;
                            eof = ftell(inptr);
                            if(max_eoi)
                            {
                                end_of_section = max_eoi - 1;
                                marker_offset = max_eoi - 2;
                            }
                            else
                            {
                                end_of_section = eof - 1;
                                marker_offset = eof - 2;
                            }
                            tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                            if(ferror(inptr) || feof(inptr))
                            {
                                tag = 0;
                                printred("# WARNING: ERROR reading JPEG data looking for JPEG_EOI");
                                chpr = newline(chpr);
                                clearerr(inptr);
                                (void)jpeg_status(JPEG_HAD_ERROR);
                                if(summary_entry)
                                    summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                                continue;
                            }
                            if(Debug & JPEG_EOI_DEBUG)
                                printf("DEBUG: %#lx/%ld: %#04x\n",marker_offset,marker_offset,tag);
                            if(tag != JPEG_EOI) /* try harder         */
                            {
                                for(i = 1; i < 512; ++i)
                                {
                                    --marker_offset;
                                    tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                                    if(ferror(inptr) || feof(inptr))
                                        break;
                                    if(Debug & JPEG_EOI_DEBUG)
                                        printf("DEBUG: %#lx/%ld: %#04x\n",marker_offset,marker_offset,tag);
                                    if(tag == JPEG_EOI)
                                        break;
                                }
                            }
                            if(ferror(inptr) || feof(inptr))
                            {
                                tag = 0;
                                printred("# WARNING: ERROR reading JPEG data looking for JPEG_EOI");
                                chpr = newline(chpr);
                                clearerr(inptr);
                                (void)jpeg_status(JPEG_HAD_ERROR);
                                if(summary_entry)
                                    summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                                continue;
                            }
                            if(tag == JPEG_EOI)
                            {
                                max_offset = ftell(inptr);
                                if(Debug & JPEG_EOI_DEBUG)
                                    printf("DEBUG: max_offset=%lu, eof=%lu, marker_offset=%lu\n",max_offset,eof,marker_offset);
                                if(marker_offset < (eof - 2))
                                {
                                    (void)jpeg_status(JPEG_EARLY_EOI);
                                    if(summary_entry)
                                    {
                                        summary_entry->imagesubformat |= IMGSUBFMT_JPEG_EARLY_EOI;
                                        if(summary_entry->length <= 0)
                                            summary_entry->length = eof - start_of_jpeg_data;
                                    }
                                }
                                continue;
                            }
                            else
                            {
                                found_eoi_offset = forward_scan_for_eoi(inptr,after_sos,end_of_section,
                                                                                            "!",indent);
                                if(found_eoi_offset)
                                {
                                    max_offset = found_eoi_offset + 2;
                                    marker_offset = found_eoi_offset;
                                    if(Debug & JPEG_EOI_DEBUG)
                                    {
                                        printf("DEBUG: max_offset=%lu, eof=%lu, marker_offset=%lu\n",max_offset,eof,marker_offset);
                                        printf("found_eof_offset=%lu, ftell=%lu\n",found_eoi_offset,ftell(inptr));
                                    }
                                    if(found_eoi_offset < (eof - 2))
                                    {
                                        (void)jpeg_status(JPEG_EARLY_EOI);
                                        if(summary_entry)
                                            summary_entry->imagesubformat |= IMGSUBFMT_JPEG_EARLY_EOI;
                                    }
                                    if((summary_entry) && (summary_entry->length <= 0))
                                        summary_entry->length = eof - start_of_jpeg_data;
                                    tag = JPEG_EOI;
                                    continue;
                                }
                                else
                                {
                                    if((summary_entry) && (summary_entry->length <= 0))
                                        summary_entry->length = eof - start_of_jpeg_data;
                                    print_tag_address(SEGMENT,eof - 1,indent + SMALLINDENT,prefix);
                                    printred(" # WARNING: at EOF: JPEG_EOI not found");
                                    if(!(LIST_MODE))
                                        chpr = newline(chpr);
                                    max_offset = 0;
                                    /* ###%%% this may not be needed, */
                                    /* since it's already been        */
                                    /* reported                       */
                                    (void)jpeg_status(JPEG_NO_EOI);
                                }
                            }
                        }
                        if(ferror(inptr) || feof(inptr))
                        {
                            (void)jpeg_status(JPEG_HAD_ERROR);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                        }
                    }
                    tag = 0;    /* break the loop                     */
                    break;
                case JPEG_DHT:  /* image data                         */
                    name = tagname(tag);
                    if(PRINT_SEGMENT)
                    {
                        print_tag_address(SEGMENT,marker_offset,indent + SMALLINDENT,prefix);
                        chpr += printf("<%s>",name);
                    }
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    if(!ferror(inptr) && !feof(inptr))
                    {
                        unsigned char t,tc,th,l;
                        int i;

                        t = read_ubyte(inptr,HERE);
                        tc = t & 0xf;
                        th = ((t & 0xf0) >> 4) & 0xf;
                        if(PRINT_SEGMENT)
                        {
                            chpr += printf(" length %u",seg_length);
                            chpr += printf(" table class = %u",tc & 0xff);
                            chpr += printf(" table id = %u",th & 0xff);
                            chpr = newline(chpr);
                        }
                        if(Debug & JPEG_MARKER_DEBUG)
                        {
                            for(i = 0; i < 16; ++i)
                            {
                                if((i % 5) == 0)
                                {
                                    chpr = newline(chpr);
                                    putindent(6 + (2 * ADDRWIDTH) + indent + 16);
                                }
                                /* component selector                 */
                                l = read_ubyte(inptr,HERE); 
                                if((ferror(inptr) == 0) && (feof(inptr) == 0) && (PRINT_SEGMENT))
                                {
                                    chpr += printf("l%-2d=%u",i,l);
                                    if(i < 15)
                                        chpr += printf(", ");
                                }
                            }
                            chpr = newline(chpr);
                        }

                        /* next tag                                       */
                        marker_offset += seg_length + 2;
                        if(marker_offset > 0L)
                            tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                        else
                            tag = 0;
                        if(ferror(inptr) || feof(inptr))
                        {
                            tag = 0;
                            clearerr(inptr);
                            (void)jpeg_status(JPEG_HAD_ERROR);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                            continue;
                        }
                    }
                    else
                    {
                        chpr += printf("ERROR reading length of DHT");
                        chpr = newline(chpr);
                        why(stdout);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                        return 0;
                    }
                    break;
                case JPEG_COM:
                    name = tagname(tag);
                    if(PRINT_SEGMENT)
                    {
                        print_tag_address(SEGMENT,marker_offset,indent + SMALLINDENT,prefix);
                        chpr += printf("<%s>",name);
                    }
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);

                    if(!ferror(inptr) && !feof(inptr))
                    {
                        if(PRINT_SEGMENT)
                        {
                            chpr += printf(" length %u: \'",seg_length);
                            (void)print_ascii(inptr,seg_length - 2,HERE);
                            chpr += printf("\'");
                            chpr = newline(chpr);
                        }
                        marker_offset += seg_length + 2;
                        /* next tag                                   */
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                        if(ferror(inptr) || feof(inptr))
                        {
                            tag = 0;
                            clearerr(inptr);
                            (void)jpeg_status(JPEG_HAD_ERROR);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                        }
                        else
                            max_offset = ftell(inptr);
                    }
                    else
                    {
                        tag = 0;
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    break;
                case JPEG_SOF_0:
                case JPEG_SOF_1:
                case JPEG_SOF_2:
                case JPEG_SOF_3:
                case JPEG_SOF_5:
                case JPEG_SOF_6:
                case JPEG_SOF_7:
                case JPEG_SOF_9:
                case JPEG_SOF_10:
                case JPEG_SOF_11:
                case JPEG_SOF_13:
                case JPEG_SOF_14:
                case JPEG_SOF_15:
                    name = tagname(tag);
                    /* If this is a TIFF format file, record the      */
                    /* actual compression type as indicated by the    */
                    /* SOF tag, for printing in the image summary at  */
                    /* the end.                                       */
                    if(summary_entry)
                        summary_entry->compression = tag;
                    if(PRINT_SEGMENT)
                    {
                        print_tag_address(SEGMENT,marker_offset,indent + SMALLINDENT,prefix);
                        chpr += printf("<%s>",name);
                    }
                    seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    if(!ferror(inptr) && !feof(inptr))
                    {
                        unsigned char nf = 0;
                        unsigned char c,h,v,t,p;
                        int i;
                        int iserror = 0;

                        if(PRINT_SEGMENT)
                            chpr += printf(" length %u",seg_length);
                        p = read_ubyte(inptr,HERE); /* precision    */
                        if(!iserror && !ferror(inptr) && !feof(inptr))
                            img_height = read_ushort(inptr,TIFF_MOTOROLA,HERE);
                        if(!iserror && !ferror(inptr) && !feof(inptr))
                            img_width = read_ushort(inptr,TIFF_MOTOROLA,HERE);
                        if(!iserror && !ferror(inptr) && !feof(inptr))
                            img_pixels = img_height * img_width;
                        if(!iserror && !ferror(inptr) && !feof(inptr))
                        {
                            nf = read_ubyte(inptr,HERE); /* components */
                            if (nf > MAXSAMPLE)
                            {
                                // See https://github.com/hfiguiere/exifprobe/issues/2
                                // we shouldn't need to clamp nf value.
                                nf = MAXSAMPLE;
                                iserror = 1;
                            }
                        }
                        if(!iserror && !ferror(inptr) && !feof(inptr))
                        {
                            max_offset = ftell(inptr);
                            if(PRINT_SEGMENT)
                                chpr += printf(", %d bits/sample, components=%d, width=%d, height=%d",
                                                            p,(int)nf,(int)img_width,(int)img_height);
                            if(summary_entry)
                            {
                                summary_entry->spp = nf;
                                for(i = 0; i < nf; i++)
                                    summary_entry->bps[i] = p;
                            }
                        }
                        if(iserror || ferror(inptr) || feof(inptr))
                        {
                            tag = 0;
                            clearerr(inptr);
                            (void)jpeg_status(JPEG_HAD_ERROR);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                            continue;
                        }
                        if(PRINT_SEGMENT)
                            chpr = newline(chpr);
                        if(Debug & JPEG_MARKER_DEBUG)
                        {
                            for(i = 0; i < nf; ++i)
                            {
                                putindent(6 + (2 * ADDRWIDTH) + indent + 16);
                                c = read_ubyte(inptr,HERE); /* component id */
                                tmp = read_ubyte(inptr,HERE); /* H | V    */
                                h = tmp & 0xf;
                                v = ((tmp & 0xf0) >> 4) & 0xf;
                                t = read_ubyte(inptr,HERE); /* Q tbl sel    */
                                if((ferror(inptr) == 0) && (PRINT_SEGMENT))
                                {
                                    chpr += printf("C%d=%d, H%d=%d, V%d=%d, T%d=%#04x",i,c,i,h,i,v,i,t);
                                    chpr = newline(chpr);
                                }
                            }
                        }
                        if(summary_entry && (img_height > 0) &&
                                                (summary_entry->pixel_height < img_height))
                        {
                            summary_entry->pixel_height = img_height;
                        }
                        if(summary_entry && (img_width > 0) &&
                                                (summary_entry->pixel_width < img_width))
                        {
                            summary_entry->pixel_width = img_width;
                        }
                        switch(tag)
                        {
                            /* lossless types are likely primary;     */
                            /* mark them for scan_summary()           */
                            case JPEG_SOF_3:
                            case JPEG_SOF_7:
                            case JPEG_SOF_11:
                            case JPEG_SOF_15:
                                summary_entry->subfiletype = POSSIBLE_PRIMARY_TYPE;
                                break;
                            default:
                                break;
                        }
                    }

                    if(!ferror(inptr) && !feof(inptr))
                    {
                        max_offset = ftell(inptr);
                        marker_offset += seg_length + 2;
                        /* next tag                                   */
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                        if(ferror(inptr) || feof(inptr))
                        {
                            tag = 0;
                            clearerr(inptr);
                            (void)jpeg_status(JPEG_HAD_ERROR);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                        }
                        else
                            max_offset = ftell(inptr);
                    }
                    else
                    {
                        tag = 0;
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    break;
                    
                case JPEG_BADSOI:   /* minolta                        */
                    if(bad_soi == 0)
                        bad_soi = tag;
                    /* fall through                                   */
                case JPEG_SOI:
                    name = tagname(tag);
                    if((PRINT_SECTION) || (PRINT_SEGMENT))
                    {
                        print_tag_address(SECTION|SEGMENT,marker_offset,indent,prefix);
                        if(bad_soi)
                        {
                            printred("<JPEG_BADSOI>");
                            chpr += printf(" (%#06x)",bad_soi);
                        }
                        else
                            chpr += printf("<%s>",name);
                        chpr = newline(chpr);
                    }
                    start_of_jpeg_data = marker_offset;
                    if(bad_soi == 0)
                        ++had_soi;  
                    /* next tag                                       */
                    tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                    if(!ferror(inptr) && !feof(inptr))
                    {
                        marker_offset += 2;
                        max_offset = ftell(inptr);
                    }
                    else
                    {
                        tag = 0;
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    break;
                case JPEG_EOI:
                    name = tagname(tag);
                    max_offset = ftell(inptr);
                    if((PRINT_SECTION) || (PRINT_SEGMENT))
                    {
                        print_tag_address(SECTION|SEGMENT,marker_offset,indent,prefix);
                        if((status = jpeg_status(0)) == JPEG_EARLY_EOI)
                        {
                            PUSHCOLOR(RED);
                            chpr += printf("<%s> JPEG length %lu",name,max_offset - start_of_jpeg_data);
                            POPCOLOR();
                        }
                        else
                            chpr += printf("<%s> JPEG length %lu",name,max_offset - start_of_jpeg_data);
                        /* re-set the cleared status                  */
                        jpeg_status(status);
                        if((PRINT_SECTION) && (status != JPEG_EARLY_EOI))
                            chpr = newline(chpr);
                    }
                    if((summary_entry) && (summary_entry->length <= 0))
                    {
                        /* Use specified length, if any, even if      */
                        /* early EOI                                  */
                        if(data_length > 0)
                        {
                            summary_entry->length = data_length;
                            max_offset = start_of_jpeg_data + data_length;
                        }
                        else
                            summary_entry->length = max_offset - start_of_jpeg_data;
                    }
                    tag = 0;  /* break the loop                       */
                    break;
                case JPEG_APP0: 
                    marker_offset = process_app0(inptr,marker_offset,tag,summary_entry,
                                                            parent_name,indent + SMALLINDENT);
                    /* next tag                                       */
                    if(marker_offset > 0L)
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                    else
                        tag = 0;
                    if(ferror(inptr) || feof(inptr))
                    {
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    else
                        max_offset = ftell(inptr);
                    break;
                case JPEG_APP1:
                    marker_offset = process_app1(inptr,marker_offset,tag,summary_entry,
                                                            parent_name,indent + SMALLINDENT);
                    /* next tag                                       */
                    if(marker_offset > 0L)
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                    else
                        tag = 0;
                    if(ferror(inptr) || feof(inptr))
                    {
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    break;
                case JPEG_APP3:
                    marker_offset = process_app3(inptr,marker_offset,tag,summary_entry,
                                                            parent_name,indent + SMALLINDENT);
                    /* next tag                                       */
                    if(marker_offset > 0L)
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                    else
                        tag = 0;
                    if(ferror(inptr) || feof(inptr))
                    {
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    break;
                case JPEG_APP12:
                    marker_offset = process_app12(inptr,marker_offset,tag,
                                            summary_entry,parent_name,indent + SMALLINDENT);
                    /* next tag                                       */
                    if(marker_offset > 0L)
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                    else
                        tag = 0;
                    if(ferror(inptr) || feof(inptr))
                    {
                        clearerr(inptr);
                        (void)jpeg_status(JPEG_HAD_ERROR);
                        if(summary_entry)
                            summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                    }
                    else
                        max_offset = ftell(inptr);
                    break;
                case JPEG_APP2:
                case JPEG_APP4:
                case JPEG_APP5:
                case JPEG_APP6:
                case JPEG_APP7:
                case JPEG_APP8:
                case JPEG_APP9:
                case JPEG_APP10:
                case JPEG_APP11:
                case JPEG_APP13:
                case JPEG_APP14:
                case JPEG_APP15:
                    marker_offset = process_appn(inptr,marker_offset,tag,
                                            summary_entry,parent_name,indent + SMALLINDENT);
                    /* next tag                                       */
                    if(marker_offset > 0L)
                        tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                    else
                        tag = 0;
                    if(ferror(inptr) || feof(inptr))
                    {
                        tag = 0;
                        clearerr(inptr);
                    }
                    else
                        max_offset = ftell(inptr);
                    break;
                default:
                    /* Minolta sometimes writes garbage in the high   */
                    /* byte of SOI tags; assume that 0xd8 in the low  */
                    /* byte is a bad SOI.                             */
                    if(((tag & 0xff00) != 0xff00) && ((tag & 0x00ff) == 0xd8))
                    {
                        /* Only if no SOI yet                         */
                        if((had_soi == 0) && (bad_soi == 0))
                        {
                            bad_soi = tag;
                            tag &= 0x00ff;  /* This will match        */
                                            /* JPEG_BADSOI above.     */
                            continue;
                        }
                        else if(bad_soi)
                            had_soi = 0;
                        /* ###%%% this should be handled better;      */
                        /* don't set had_soi on a bad_soi but set it  */
                        /* after a legitimate marker is found NEXT;   */
                        /* I.e. we have a legitimate jpeg only if a   */
                        /* good marker is found, after which an error */
                        /* indicates corrupted jpeg data, otherwise   */
                        /* an error should indicate no jpeg data      */
                        /* found.                                     */
                    }
                    name = tagname(tag);
                    if(PRINT_SEGMENT)
                    {
                        print_tag_address(SEGMENT,marker_offset,indent + SMALLINDENT,prefix);
                        chpr += printf("<%s>",name);
                    }
                    if((tag & 0xff00) != 0xff00)
                    {
                        if(PRINT_SEGMENT)
                        {
                            printred(" INVALID JPEG TAG");
                            chpr = newline(chpr);
                        }
                        tag = 0;
                        if(had_soi)
                            (void)jpeg_status(JPEG_HAD_ERROR);
                        else
                            (void)jpeg_status(JPEG_NO_SOI);
                        if(summary_entry)
                        {
                            /* If this is the first tag, we have no   */
                            /* reason to believe that this is really  */
                            /* jpeg data. If not, data is corrupted   */
                            if(had_soi)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                            else
                            {
                                summary_entry->imagesubformat |= IMGSUBFMT_NO_JPEG;
                                /* ###%%% hmmm...                     */
                                /* summary_entry->compression = 0;    */
                            }
                        }
                        else
                            printred("JPEG: NO SUMMARY ENTRY ");
                        clearerr(inptr);
                        if(Max_imgdump > 0)
                        {
                            /* If the length of the data is unknown,  */
                            /* limit the dump to something reasonable */
                            if(Max_imgdump == DUMPALL)
                            {
                                if(data_length > 0)
                                    dumplength = data_length;
                                else
                                    data_length = dumplength = 1024;
                            }
                            else if(Max_imgdump > data_length)
                            {
                                if(data_length > 0)
                                    dumplength = data_length;
                                else
                                    data_length = dumplength = 1024;
                            }
                            else
                                dumplength = Max_imgdump;
                            chpr = newline(0);
                            hexdump(inptr,start_of_jpeg_data,data_length,
                                        dumplength,16,indent,SMALLINDENT);
                            chpr = newline(1);
                        }
                        max_offset = start_of_jpeg_data + data_length;
                    }
                    else
                    {
                        seg_length = read_ushort(inptr,TIFF_MOTOROLA,marker_offset + 2);
                        if(!ferror(inptr) && !feof(inptr))
                        {
                            if(PRINT_SEGMENT)
                            {
                                chpr += printf(" length %u",seg_length);
                                chpr = newline(chpr);
                            }
                            marker_offset = marker_offset + 2 + seg_length;
                            tag = read_ushort(inptr,TIFF_MOTOROLA,marker_offset);
                            if(ferror(inptr) || feof(inptr))
                            {
                                tag = 0;
                                clearerr(inptr);
                                (void)jpeg_status(JPEG_HAD_ERROR);
                                if(summary_entry)
                                    summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                            }
                            else
                                max_offset = ftell(inptr);
                        }
                        else
                        {
                            tag = 0;
                            clearerr(inptr);
                            (void)jpeg_status(JPEG_HAD_ERROR);
                            if(summary_entry)
                                summary_entry->imagesubformat |= IMGSUBFMT_JPEGFAILED;
                        }
                    }
                    break;
            }
        }
        POPCOLOR();
    }
    setcharsprinted(chpr);
    return(max_offset);
}

/* set and report errors in jpeg files "out of band".                 */

int
jpeg_status(int setstatus)
{
    static int status = 0;
    int retvalue;

    retvalue = status;
    status = setstatus;
    return(retvalue);
}

#define JPEG_HIBYTE 0xff
#define EOI_LOBYTE  0xd9

unsigned long
forward_scan_for_eoi(FILE *inptr,unsigned long start_of_data,
                            unsigned long end_of_section,char *prefix,int indent)
{
    int highbyte,lobyte;
    unsigned long eoi_offset = 0UL;
    unsigned long tagloc;
    int chpr = 0;

    if(inptr && (fseek(inptr,start_of_data,SEEK_SET) != -1))
    {
        if(Debug & JPEG_EOI_DEBUG)
            printf("DEBUG: start scan at %lu\n",start_of_data);
        while(((highbyte = fgetc(inptr)) != EOF) && (highbyte != EOF) && (ftell(inptr) < end_of_section))
        {
            if(highbyte != JPEG_HIBYTE)
                continue;
            tagloc = ftell(inptr) - 1;
            lobyte = fgetc(inptr);
            if(lobyte == EOI_LOBYTE)
            {
                eoi_offset = tagloc;
                if(Debug & JPEG_EOI_DEBUG)
                {
                    if(!(LIST_MODE) && (PRINT_SEGMENT))
                    {
                        print_tag_address(SEGMENT,eoi_offset,indent + SMALLINDENT,prefix);
                        PUSHCOLOR(RED);
                        putindent(2);
                        chpr += printf("possible JPEG_EOI found at %lu",eoi_offset);
                    }
                    POPCOLOR();
                    chpr = newline(chpr);
                }
                break;
            }
        }
    }
    if(Debug & JPEG_EOI_DEBUG)
        printf("DEBUG: end scan at %lu\n",ftell(inptr));
    return(eoi_offset);
}


/* JFIF format files                                                  */

unsigned long
process_app0(FILE *inptr,unsigned long app0_offset,unsigned short tag,
                struct image_summary *summary_entry,char *parent_name,
                int indent)
{
    unsigned long max_offset = 0L;
    unsigned long data_offset,dumplength;
    unsigned long filesize = 0UL;
    char *app_string,*name;
    unsigned short app_length = 0L;
    int status = 0;
    int chpr = 0;
    unsigned short marker,xt,yt,tmp;
    struct fileheader *header = NULL;
    char *fulldirname = CNULL;
    char *subdirname = CNULL;

    if(inptr)
    {
        filesize = get_filesize(inptr);
        if(summary_entry)
        {
            summary_entry->filesubformat |= FILESUBFMT_APPN;
            summary_entry->filesubformatAPPN[0] = 1;
        }
        name = tagname(tag);
        app_length = read_ushort(inptr,TIFF_MOTOROLA,app0_offset + 2);
        if((PRINT_SECTION))
            print_tag_address(SECTION,app0_offset,indent,"@");
        data_offset = app0_offset + 4L;
        app_string = read_appstring(inptr,JPEG_APP0,data_offset);
        app_string = app_string ? app_string : QSTRING;
        if(PRINT_SECTION)
            chpr += printf("<%s> %#x length %u, ",name,tag,app_length);
        else if((PRINT_ENTRY))
        {
            /* ###%%% pseudo-tags fpr LIST Mode here */
            print_tag_address(ENTRY,HERE,indent,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%-14.14s","APP0");
            }
            if((PRINT_VALUE))
                chpr += printf(" = @%lu:%-9u",app0_offset,app_length);
            chpr = newline(chpr);
        }

        fulldirname = splice(parent_name,".","APP0");
        if((strncmp(app_string,"JFIF",4) == 0) && (app_string[4] == '\0'))
        {
            if(summary_entry)
                summary_entry->filesubformat |= FILESUBFMT_JFIF;
            data_offset += 5;
            if((PRINT_SECTION))
            {
                chpr += printf("\'%s\'",app_string);
                chpr = newline(chpr);
            }
            tmp = read_ushort(inptr,TIFF_MOTOROLA,data_offset);
            if((PRINT_APPENTRY) && (PRINT_ENTRY))
            {
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if(PRINT_SECTION)
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","Version");
                }
                if((PRINT_VALUE))
                    chpr += printf("= %d.%d",(int)((tmp >> 8) & 0xff),(int)(tmp & 0xff));
                chpr = newline(chpr);
            }
            data_offset = ftell(inptr);
            tmp = read_ubyte(inptr,data_offset);
            if((PRINT_APPENTRY) && (PRINT_ENTRY))
            {
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if(PRINT_SECTION)
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","Units");
                }
                if((PRINT_VALUE))
                    chpr += printf("= %s",tmp? tmp == 1 ? "'dots/inch'" : "'dots/cm'" : "'aspect ratio'");
                chpr = newline(chpr);
            }
            data_offset = ftell(inptr);
            tmp = read_ushort(inptr,TIFF_MOTOROLA,data_offset);
            if((PRINT_APPENTRY) && (PRINT_ENTRY))
            {
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if(PRINT_SECTION)
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","Xdensity");
                }
                if((PRINT_VALUE))
                    chpr += printf("= %u",tmp);
                chpr = newline(chpr);
            }
            data_offset = ftell(inptr);
            tmp = read_ushort(inptr,TIFF_MOTOROLA,data_offset);
            if((PRINT_APPENTRY) && (PRINT_ENTRY))
            {
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if(PRINT_SECTION)
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","Ydensity");
                }
                if((PRINT_VALUE))
                    chpr += printf("= %u",tmp);
                chpr = newline(chpr);
            }
            data_offset = ftell(inptr);
            xt = read_ubyte(inptr,data_offset);
            if((PRINT_APPENTRY) && (PRINT_ENTRY))
            {
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if(PRINT_SECTION)
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","XThumbnail");
                }
                if((PRINT_VALUE))
                    chpr += printf("= %d",(int)xt);
                chpr = newline(chpr);
            }
            data_offset = ftell(inptr);
            yt = read_ubyte(inptr,data_offset);
            if((PRINT_APPENTRY) && (PRINT_ENTRY))
            {
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if(PRINT_SECTION)
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if((PRINT_LONGNAMES))
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","YThumbnail");
                }
                if((PRINT_VALUE))
                    chpr += printf("= %d",(int)yt);
                chpr = newline(chpr);
            }
            if(xt && yt)
            {
                data_offset = ftell(inptr);
                print_tag_address(APP_ENTRY,data_offset,indent,"@");
                if((PRINT_APPENTRY) && (PRINT_SECTION))
                    extraindent(SMALLINDENT);
                if((PRINT_TAGINFO))
                {
                    if(PRINT_LONGNAMES)
                        chpr += printf("%s.",fulldirname);
                    chpr += printf("%-14.14s","Thumbnail");
                }
                if((PRINT_VALUE))
                {
                    chpr += printf(" = '%u RGB bytes (%u) pixels'",3 * xt * yt,xt * yt);
                    chpr = newline(chpr);
                }
            }
            max_offset = data_offset + 1 + (3 * xt * yt);
        }
        else if((strncmp(app_string,"JFXX",4) == 0) && (app_string[4] == '\0'))
        {
            if(summary_entry)
                summary_entry->filesubformat |= FILESUBFMT_JFXX;
            data_offset += 5;
            tmp = read_ubyte(inptr,data_offset);
            if((PRINT_SECTION))
            {
                chpr += printf("\'%s\'",app_string);
                chpr = newline(chpr);
                print_tag_address(SECTION,data_offset,indent,"@");
                if((PRINT_SECTION))
                {
                    extraindent(SMALLINDENT);
                    chpr += printf(" extension code %#x",tmp);
                }
            }
            else
                chpr = newline(chpr);
            ++data_offset;
            switch(tmp)
            {
            case 0x10:  if((PRINT_SECTION))
                        {
                            chpr += printf(" - JPEG thumbnail");
                            chpr = newline(chpr);
                        }
                        marker = read_ushort(inptr,TIFF_MOTOROLA,HERE);
                        summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG);
                        if(summary_entry)
                        {
                            summary_entry->filesubformat |= FILESUBFMT_JFXX;
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                        }
                        if((LIST_MODE))
                        {
                            if((PRINT_SEGMENT))
                                chpr = newline(chpr);
                            print_tag_address(ENTRY,data_offset,indent + SMALLINDENT,"@");
                            if((PRINT_TAGINFO))
                            {
                                if(PRINT_LONGNAMES)
                                    chpr += printf("%s.JFXX.",fulldirname);
                                chpr += printf("%-14.14s","JpegThumbnail");
                            }
                            if((PRINT_VALUE))
                            {
                                max_offset = app0_offset + app_length + 2;
                                chpr += printf(" = @%lu:%lu",data_offset,max_offset - data_offset);
                                chpr = newline(chpr);
                            }
                        }
                        max_offset = process_jpeg_segments(inptr,data_offset,marker,
                                            app_length - 8,summary_entry,parent_name,
                                            "@",indent + SMALLINDENT);
                        if((max_offset - 1) > filesize)
                        {
                            PUSHCOLOR(RED);
                            chpr += printf(" (TRUNCATED at %lu)",filesize);
                            POPCOLOR();
                            summary_entry->imagesubformat |= IMGSUBFMT_TRUNCATED;
                        }
                        max_offset = app0_offset + app_length + 2;
                        if((PRINT_SECTION))
                        {
                            if((status = jpeg_status(0) == JPEG_EARLY_EOI))
                                chpr = newline(chpr);
                            jpeg_status(status);
                            print_tag_address(SECTION,max_offset - 1,indent + SMALLINDENT,"@");
                            chpr += printf("#### End of JPEG thumbnail data for APP0");
                            chpr += printf(", length %lu ####",max_offset - data_offset);
                            print_jpeg_status();
                            chpr = newline(chpr);
                        }
                        break;
            case 0x11:  if((PRINT_APPENTRY) && (PRINT_SECTION))
                            chpr += printf(" - thumbnail stored using 1 byte/pixel"); 
                        xt = read_ubyte(inptr,data_offset++);
                        yt = read_ubyte(inptr,data_offset++);
                        /* The RGB palette is 768 bytes               */
                        max_offset = data_offset + 768 + (xt * yt) ;
                        summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG);
                        if(summary_entry)
                        {
                            summary_entry->filesubformat |= FILESUBFMT_JFXX;
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                            summary_entry->imagesubformat = IMGSUBFMT_PALETTE;
                            summary_entry->offset = data_offset;
                            summary_entry->length = xt * yt + 768;
                            summary_entry->pixel_width = xt;
                            summary_entry->pixel_height = yt;
                            summary_entry->compression = 1;
                            summary_entry->spp = 1;
                            summary_entry->sample_size = 8;
                            summary_entry->datatype = JPEG_APP0;
                            summary_entry->entry_lock = lock_number(summary_entry);
                        }
                        break;
            case 0x13:  if((PRINT_APPENTRY) && (PRINT_SECTION))
                            chpr += printf(" - thumbnail stored using 3 bytes/pixel");
                        xt = read_ubyte(inptr,data_offset++);
                        yt = read_ubyte(inptr,data_offset++);
                        max_offset = data_offset + (3 * xt * yt) ;
                        summary_entry = new_summary_entry(summary_entry,0,IMGFMT_JPEG);
                        if(summary_entry)
                        {
                            summary_entry->filesubformat |= FILESUBFMT_JFXX;
                            summary_entry->subfiletype = REDUCED_RES_TYPE;
                            summary_entry->imagesubformat = IMGSUBFMT_RGB;
                            summary_entry->offset = data_offset;
                            summary_entry->length = xt * yt * 3;
                            summary_entry->pixel_width = xt;
                            summary_entry->pixel_height = yt;
                            summary_entry->compression = 1;
                            summary_entry->spp = 3;
                            summary_entry->sample_size = 8;
                            summary_entry->datatype = JPEG_APP0;
                            summary_entry->entry_lock = lock_number(summary_entry);
                        }
                        break;
            default:    
                        if((PRINT_APPENTRY) && (PRINT_SECTION))
                            chpr += printf(" - UNKNOWN JFXX extension");
                        max_offset = data_offset;
                        break;
            }
            chpr = newline(chpr);
        }
        else if(((strncmp(app_string,"II",2) == 0) || (strncmp(app_string,"MM",2) == 0)) &&
                    (app_string[2] == 0x1a))    /* maybe CIFF     */
        {
            if((header = read_imageheader(inptr,data_offset)))
            {
                if(summary_entry)
                    summary_entry->filesubformat |= FILESUBFMT_CIFF;
                if((PRINT_APPENTRY) && (PRINT_SECTION))
                    chpr = newline(chpr);
                if((PRINT_SECTION))
                {
                    print_tag_address(SECTION,data_offset,indent+SMALLINDENT,"@");
                    print_header(header,SECTION);
                }
                if(header->probe_magic == PROBE_CIFFMAGIC)
                {
                    subdirname = splice(fulldirname,".","CIFF");
                    if(!(PRINT_SECTION))
                    {
                        print_tag_address(ENTRY|VALUE,data_offset,indent+SMALLINDENT,"@");
                        if((PRINT_TAGINFO))
                        {
                            if((PRINT_LONGNAMES))
                                chpr += printf("%s.",subdirname);
                            chpr += printf("%-*.*s",CIFFTAGWIDTH,CIFFTAGWIDTH,"HeaderOffset");
                        }
                        if((PRINT_VALUE))
                        {
                            if(PRINT_BOTH_OFFSET)
                                chpr += printf(" = @%#lx=%lu",data_offset,data_offset);
                            else if(PRINT_HEX_OFFSET)
                                chpr += printf(" = @%#lx",data_offset);
                            else 
                                chpr += printf(" = @%lu",data_offset);
                        }
                        chpr = newline(chpr);
                    }
                    max_offset = process_ciff(inptr,header,data_offset,app_length - 2,
                                        summary_entry,subdirname,0,indent+SMALLINDENT);
                    if(subdirname)
                        free(subdirname);
                    subdirname = CNULL;
                }
                else
                    goto appdump;
            }
            else
                goto appdump;
        }
        else
            goto appdump;

        if(fulldirname)
            free(fulldirname);
    /* If there's data unaccounted for, allow the data that isn't */
        /* accounted for to be dumped                                 */
        if(max_offset < (app0_offset + app_length + 2))
        {
            print_tag_address(SEGMENT,max_offset,indent,"@");
            extraindent(SMALLINDENT);
            dumplength = app0_offset + app_length + 1 - max_offset;
            chpr += printf("---- End of data before end of APP0 (%lu bytes)",dumplength);
            chpr = newline(chpr);
            /* dump at least the first couple of rows, just to see... */
            if(PRINT_APPNDUMP)
            {
                if((Max_appdump == DUMPALL) || (Max_appdump > (app_length + 2)))
                    dumplength = app_length - 2;
                else if(Max_appdump > 0L)
                    dumplength = Max_appdump;
                else if(Max_undefined > 0L)
                    dumplength = Max_undefined;
                else
                    dumplength = app_length - 2;
            }
            else
                dumplength = dumplength > 48 ? 48 : dumplength;
            if(dumplength)
            {
                hexdump(inptr,app0_offset + 4,app_length - 2,dumplength,16,
                            indent + SMALLINDENT,SMALLINDENT);
                chpr = newline(1);
            }
        }
        /* ###%%% if max_offset >, warn */
            
        /* close off the section.                                     */
        if((PRINT_SECTION))
        {
            print_tag_address(SECTION,app0_offset + app_length + 1,indent,"@");
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }

    return(app0_offset + app_length + 2);
appdump:
    /* Something went haywire...dump starting at the marker           */
    /* At the moment, this happens only for imaginative APP0 markers  */
    if(app_length > 0L)
    {
        if(PRINT_APPNDUMP)
        {
            if((Max_appdump == DUMPALL) || (Max_appdump > (app_length + 2)))
                dumplength = app_length + 2;
            else if(Max_appdump > 0L)
                dumplength = Max_appdump;
            else
                dumplength = Max_undefined;
            if(dumplength)
            {
                chpr = newline(1);
                hexdump(inptr,app0_offset,app_length + 2,dumplength,16,
                            indent + SMALLINDENT,SMALLINDENT);
                chpr = newline(1);
            }
        }
        else
        {
            if(PRINT_SECTION)
            {
                chpr += printf(" - (not dumped: use -A)");
                chpr = newline(chpr);
            }
        }
        max_offset = app0_offset + app_length + 2;
        /* close off the segment; a start "tag" was printed           */
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,app0_offset + app_length + 1,indent,"@");
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(max_offset);
}

/* Process (print) an APP1 (EXIF) section                             */

unsigned long
process_app1(FILE *inptr,unsigned long app1_offset,unsigned short tag,
                struct image_summary *summary_entry,char *parent_name,
                int indent)
{
    unsigned long max_offset = 0UL;
    unsigned long ifd_offset,fileoffset_base;
    unsigned long dumplength = 0UL;
    unsigned short app_length = 0;
    int chpr = 0;
    struct fileheader *header;
    char *app_string,*name;
    char *fulldirname = CNULL;

    if(inptr)
    {
        if(summary_entry)
        {
            summary_entry->filesubformat |= FILESUBFMT_APPN;
            summary_entry->filesubformatAPPN[1] = 1;
        }
        /* display the APP1 header                                    */
        name = tagname(tag);    /* never null                         */
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,app1_offset,indent,"@");
            chpr += printf("<%s> %#x ",name,tag);
        }

        app_length = read_ushort(inptr,TIFF_MOTOROLA,HERE);
        if(ferror(inptr) || feof(inptr))
        {
            clearerr(inptr);
            goto appdump;
        }
        if(PRINT_SECTION)
            chpr += printf("length %u, ",app_length);
        else if((PRINT_ENTRY))
        {
            /* ###%%% pseudo-tags fpr LIST Mode here */
            print_tag_address(ENTRY,HERE,indent,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%-14.14s","APP1");
            }
            if((PRINT_VALUE))
                chpr += printf(" = @%lu:%-9u",app1_offset,app_length);
            chpr = newline(chpr);
        }

        app_string = read_appstring(inptr,JPEG_APP1,HERE);
        if(ferror(inptr) || feof(inptr))
        {
            clearerr(inptr);
            goto appdump;
        }

        if(app_string)
        {
            if(PRINT_SECTION)
                chpr += printf("\'%s\'",app_string);

            /* The next thing we see should be a TIFF header tag      */
            /* indicating byte order used in the IFD, followed by the */
            /* offset from the start of the header to the start of    */
            /* the IFD (number of entries).                           */

            fileoffset_base = ftell(inptr);
            /* If this is a valid Exif segment, skip the pad byte.    */
            if(strncmp(app_string,"Exif",4) == 0)
            {
                fulldirname = splice(parent_name,".","APP1");
                ++fileoffset_base;
                if(PRINT_SECTION)
                {
                    chpr = newline(chpr);
                    print_tag_address(SECTION,fileoffset_base,indent + SMALLINDENT,"@");
                }
                header = read_imageheader(inptr,fileoffset_base);
                if(header && (print_header(header,SECTION) == 0) &&
                    (header->probe_magic == TIFF_MAGIC))
                {
                    ifd_offset = read_ulong(inptr,header->file_marker,HERE);
                    if(PRINT_SECTION)
                    {
                        chpr += printf(" ifd offset = %lu (+ %lu = %#lx/%lu)",ifd_offset,
                                    fileoffset_base,fileoffset_base + ifd_offset,
                                    fileoffset_base + ifd_offset);
                        chpr = newline(chpr);
                    }
                    /* ###%%% maybe should unlock the summary entry,  */
                    /* so that the tiff processor will use it? not    */
                    /* sure if that's appropriate for APP1 found in   */
                    /* jpeg sub-images.                               */
                    max_offset = process_tiff_ifd(inptr,header->file_marker,ifd_offset,
                                            fileoffset_base,0L,summary_entry,fulldirname,
                                            TIFF_IFD,0,-1,indent + SMALLINDENT);

                    if(max_offset < app1_offset + app_length + 2)
                        max_offset = app1_offset + app_length +2;
                }
                else
                {
                    max_offset = app1_offset + app_length + 2;
                    if(PRINT_SECTION)
                    {
                        extraindent(indent + ADDRWIDTH);
                        if(header)
                        {
                            chpr += printf(" INVALID MAGIC %lu (%s) where TIFF header should be",
                                         header->probe_magic,tagname(header->probe_magic));
                        }
                        else
                        {
                            chpr += printf(" INVALID HEADER for TIFF");
                        }
#define PRINT_A_BIT 48
                        if(app_length > PRINT_A_BIT)
                            dumplength = PRINT_A_BIT;
                        else
                            dumplength = app_length;
                        goto appdump;
                    }
                }
                if(fulldirname)
                    free(fulldirname);
            }
            else
            {
                max_offset = app1_offset + app_length + 2;
                if(PRINT_SECTION)
                {
                    chpr += printf(" - unknown format");
                    goto appdump;
                }
            }
        }
        else if(PRINT_SECTION)
        {
            chpr += printf("  No APP1 header name string found");
            chpr = newline(chpr);
        }
        /* offset of last byte of segment                             */
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,max_offset - 1,indent,"-");
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }
    return(max_offset);

appdump:
    if(app_length > 0L)
    {
        if((PRINT_APPNDUMP) || (dumplength > 0))
        {
            if((Max_appdump == DUMPALL) || (Max_appdump > (app_length + 2)))
                dumplength = app_length + 2;
            else if(Max_appdump > 0L)
                dumplength = Max_appdump;
            else if(dumplength == 0)
                dumplength = Max_undefined;
            if(dumplength)
            {
                chpr = newline(1);
                hexdump(inptr,app1_offset,app_length + 2,dumplength,16,
                            indent + SMALLINDENT,SMALLINDENT);
                chpr = newline(1);
            }
        }
        else
        {
            if(PRINT_SEGMENT)
            {
                chpr += printf(" - (not dumped: use -A)");
                chpr = newline(chpr);
            }
        }
        max_offset = app1_offset + app_length + 2;
        /* close off the segment; a start "tag" was printed           */
        print_tag_address(SECTION,app1_offset + app_length + 1,indent,"-");
        if(PRINT_SECTION)
        {
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(max_offset);
}

/* Process (print) an APP3 (Meta) section                             */

unsigned long
process_app3(FILE *inptr,unsigned long app3_offset,unsigned short tag,
                struct image_summary *summary_entry,char *parent_name,
                int indent)
{
    unsigned long max_offset = 0UL;
    unsigned long ifd_offset,fileoffset_base;
    unsigned long dumplength = 0UL;
    unsigned short app_length = 0;
    int chpr = 0;
    struct fileheader *header;
    char *app_string,*name;
    char *fulldirname = CNULL;

    if(inptr)
    {
        if(summary_entry)
        {
            summary_entry->filesubformat |= FILESUBFMT_APPN;
            summary_entry->filesubformatAPPN[3] = 1;
        }
        /* display the APP3 header                                    */
        name = tagname(tag);    /* never null                         */
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,app3_offset,indent,"@");
            chpr += printf("<%s> %#x ",name,tag);
        }

        app_length = read_ushort(inptr,TIFF_MOTOROLA,HERE);
        if(ferror(inptr) || feof(inptr))
        {
            clearerr(inptr);
            goto appdump;
        }
        if(PRINT_SECTION)
            chpr += printf("length %u, ",app_length);
        else if((PRINT_ENTRY))
        {
            /* ###%%% pseudo-tags fpr LIST Mode here */
            print_tag_address(ENTRY,HERE,indent,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%-14.14s","APP3");
            }
            if((PRINT_VALUE))
                chpr += printf(" = @%lu:%-9u",app3_offset,app_length);
            chpr = newline(chpr);
        }

        app_string = read_appstring(inptr,JPEG_APP3,HERE);
        if(ferror(inptr) || feof(inptr))
        {
            clearerr(inptr);
            goto appdump;
        }

        if(app_string)
        {
            if(PRINT_SECTION)
                chpr += printf("\'%s\'",app_string);

            /* The next thing we see should be a TIFF header tag      */
            /* indicating byte order used in the IFD, followed by the */
            /* offset from the start of the header to the start of    */
            /* the IFD (number of entries).                           */

            fileoffset_base = ftell(inptr);
            /* If this is a valid Meta segment, skip the pad byte.    */
            if(strncmp(app_string,"Meta",4) == 0)
            {
                fulldirname = splice(parent_name,".","APP3");
                ++fileoffset_base;
                if(PRINT_SECTION)
                {
                    chpr = newline(chpr);
                    print_tag_address(SECTION,fileoffset_base,indent + SMALLINDENT,"@");
                }
                header = read_imageheader(inptr,fileoffset_base);
                if(header && (print_header(header,SECTION) == 0) &&
                    (header->probe_magic == TIFF_MAGIC))
                {
                    ifd_offset = read_ulong(inptr,header->file_marker,HERE);
                    if(PRINT_SECTION)
                    {
                        chpr += printf(" ifd offset = %lu (+ %lu = %#lx/%lu)",ifd_offset,
                                    fileoffset_base,fileoffset_base + ifd_offset,
                                    fileoffset_base + ifd_offset);
                        chpr = newline(chpr);
                    }
                    max_offset = process_tiff_ifd(inptr,header->file_marker,ifd_offset,
                                            fileoffset_base,0L,summary_entry,fulldirname,
                                            TIFF_IFD,0,-1,indent + SMALLINDENT);

                    if(max_offset < app3_offset + app_length + 2)
                        max_offset = app3_offset + app_length +2;
                }
                else
                {
                    max_offset = app3_offset + app_length + 2;
                    if(PRINT_SECTION)
                    {
                        extraindent(indent + ADDRWIDTH);
                        if(header)
                        {
                            chpr += printf(" INVALID MAGIC %lu (%s) where TIFF header should be",
                                           header->probe_magic,tagname(header->probe_magic));
                        }
                        else
                        {
                            chpr += printf(" INVALID HEADER for TIFF");
                        }
#define PRINT_A_BIT 48
                        if(app_length > PRINT_A_BIT)
                            dumplength = PRINT_A_BIT;
                        else
                            dumplength = app_length;
                        goto appdump;
                    }
                }
                if(fulldirname)
                    free(fulldirname);
            }
            else
            {
                max_offset = app3_offset + app_length + 2;
                if(PRINT_SECTION)
                {
                    chpr += printf(" - unknown format");
                    goto appdump;
                }
            }
        }
        else if(PRINT_SECTION)
        {
            chpr += printf("  No APP3 header name string found");
            chpr = newline(chpr);
        }
        /* offset of last byte of segment                             */
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,max_offset - 1,indent,"-");
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }
    return(max_offset);

appdump:
    if(app_length > 0L)
    {
        if((PRINT_APPNDUMP) || (dumplength > 0))
        {
            if((Max_appdump == DUMPALL) || (Max_appdump > (app_length + 2)))
                dumplength = app_length + 2;
            else if(Max_appdump > 0L)
                dumplength = Max_appdump;
            else if(dumplength == 0)
                dumplength = Max_undefined;
            if(dumplength)
            {
                chpr = newline(1);
                hexdump(inptr,app3_offset,app_length + 2,dumplength,16,
                            indent + SMALLINDENT,SMALLINDENT);
                chpr = newline(1);
            }
        }
        else
        {
            if(PRINT_SEGMENT)
            {
                chpr += printf(" - (not dumped: use -A)");
                chpr = newline(chpr);
            }
        }
        max_offset = app3_offset + app_length + 2;
        /* close off the segment; a start "tag" was printed           */
        print_tag_address(SECTION,app3_offset + app_length + 1,indent,"-");
        if(PRINT_SECTION)
        {
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(max_offset);
}
#undef PRINT_A_BIT


/* Unknown APPn markers; hex dump them and move on.                   */

unsigned long
process_appn(FILE *inptr,unsigned long appn_offset,unsigned short tag,
                struct image_summary *summary_entry,char *parent_name,
                int indent)
{
    char *app_string,*name;
    unsigned short app_length = 0;
    unsigned long dumplength = 0L;
    int chpr = 0;
    int tagwidth = 13;

    if(inptr)
    {
        if(summary_entry)
        {
            summary_entry->filesubformat |= FILESUBFMT_APPN;
            summary_entry->filesubformatAPPN[tag & 0xf] = 1;
        }
        name = tagname(tag);    /* never null                         */
        app_length = read_ushort(inptr,TIFF_MOTOROLA,appn_offset + 2);
        print_tag_address(SECTION,appn_offset,indent,"@");
        if(PRINT_SECTION)
            chpr += printf("<%s> %#x length %u, ",name,tag,app_length);
        app_string = read_appstring(inptr,tag,HERE);
        if(PRINT_SECTION)
            chpr += printf("%s",app_string ? app_string : "NULL ID");
        else if((PRINT_ENTRY))
        {
            /* ###%%% pseudo-tags fpr LIST Mode here */
            print_tag_address(ENTRY,HERE,indent,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                tagwidth = 13;
                if((tag & 0xf) > 9)
                    --tagwidth;
                chpr += printf("%-*.*s%d",tagwidth,tagwidth,"APP",tag & 0xf);
            }
            if((PRINT_VALUE))
                chpr += printf(" = @%lu:%-9u",appn_offset,app_length);
            chpr = newline(chpr);
        }

        if(PRINT_APPNDUMP)
        {
            if((Max_appdump == DUMPALL) || (Max_appdump > (app_length + 2)))
                dumplength = app_length + 2;
            else if(Max_appdump > 0L)
                dumplength = Max_appdump;
            else
                dumplength = Max_undefined;
            if(dumplength)
            {
                chpr = newline(1);
                hexdump(inptr,appn_offset,app_length + 2,dumplength,16,
                            indent,SMALLINDENT);
                chpr = newline(1);
            }
        }
        else if(PRINT_SECTION)
        {
            chpr += printf(" - unknown format (not dumped: use -A)");
            chpr = newline(chpr);
        }

        print_tag_address(SECTION,appn_offset + app_length + 1,indent,"-");
        if(PRINT_SECTION)
        {
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
        fseek(inptr,appn_offset + app_length,SEEK_SET);
    }
    setcharsprinted(chpr);
    return(appn_offset + app_length + 2);
}

/* Print the string data from an APP12 segment, if hexdump hasn't     */
/* been asked. The "[fileinfo]" stuff is unknown format, but this     */
/* part should be useful.                                             */

unsigned long
process_app12(FILE *inptr,unsigned long app12_offset,unsigned short tag,
                struct image_summary *summary_entry,char *parent_name,
                int indent)
{
    char *app_string,*name;
    unsigned short app_length = 0;
    unsigned long dumplength = 0UL;
    unsigned long offset = 0UL;
    unsigned long end_offset = 0UL;
    unsigned long size = 0UL;
    int chpr = 0;
    int nextch;

    if(inptr)
    {
        if(summary_entry)
        {
            summary_entry->filesubformat |= FILESUBFMT_APPN;
            summary_entry->filesubformatAPPN[12] = 1;
        }
        name = tagname(tag);    /* never null                         */
        app_length = read_ushort(inptr,TIFF_MOTOROLA,app12_offset + 2);
        print_tag_address(SECTION,app12_offset,indent,"@");
        if(PRINT_SECTION)
            chpr += printf("<%s> %#x length %u, ",name,tag,app_length);
        else if((PRINT_ENTRY))
        {
            /* ###%%% pseudo-tags fpr LIST Mode here */
            print_tag_address(ENTRY,HERE,indent,"*");
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.",parent_name);
                chpr += printf("%-14.14s","APP12");
            }
            if((PRINT_VALUE))
                chpr += printf(" = @%lu:%-9u",app12_offset,app_length);
            chpr = newline(chpr);
        }
        app_string = read_appstring(inptr,tag,HERE);
        if(PRINT_SECTION)
            chpr += printf("\'%s\'",app_string ? app_string : "NULL ID");

        if(PRINT_APPNDUMP)
        {
            if((Max_appdump == DUMPALL) || (Max_appdump > (app_length + 2)))
                dumplength = app_length + 2;
            else if(Max_appdump > 0L)
                dumplength = Max_appdump;
            else
                dumplength = Max_undefined;
            if(dumplength)
            {
                if(PRINT_SECTION)
                    chpr = newline(chpr);
                hexdump(inptr,app12_offset,app_length + 2,dumplength,16,
                            indent + SMALLINDENT,SMALLINDENT);
                chpr = newline(1);
            }
        }
        else if((PRINT_APPENTRY) && (PRINT_ENTRY))
        {
            /* ###%%% in LIST mode, these should get an "APP12." prefix */
            end_offset = app12_offset + app_length + 1;
            offset = ftell(inptr);
            nextch = skip_past_newline(inptr,end_offset);
            offset = ftell(inptr);
            if(PRINT_SECTION)
                chpr = newline(chpr);
            if(offset < end_offset);
                nextch = putword(inptr,nextch,end_offset,indent);
            chpr = newline(chpr);
            offset = ftell(inptr);
            while(nextch && (nextch != EOF) && (offset < end_offset))
            {
                nextch = putword(inptr,nextch,end_offset,indent);
                if(nextch)
                    chpr = newline(chpr);
                offset = ftell(inptr);
            }
            while(offset < end_offset)
            {
                nextch = skip_to_bracket(inptr,end_offset);
                if(nextch == EOF)
                {
                    break;
                }
                size = ftell(inptr) - offset;
                if(nextch == '[')
                    --size;
                if(size > 1)
                {
                    chpr += printf(" length %lu",size);
                    if(Max_undefined)
                    {
                        chpr = newline(chpr);
                        hexdump(inptr,offset,size, Max_undefined,16,
                                    indent + SMALLINDENT,
                                    SMALLINDENT);
                        chpr = newline(1);
                        nextch = fgetc(inptr);
                        if((nextch == EOF) || (ftell(inptr) >= end_offset))
                            break;
                    }
                    else
                    {
                        printred(" (not dumped; use -U)");
                        chpr = newline(chpr);
                    }
                }
                else
                    chpr = newline(chpr);
                if(nextch)
                    nextch = putword(inptr,nextch,end_offset,indent);
                offset = ftell(inptr);
            }
        }

        print_tag_address(SECTION,app12_offset + app_length + 1,indent,"-");
        if(PRINT_SECTION)
        {
            chpr += printf("</%s>",name);
            chpr = newline(chpr);
        }
    }
    setcharsprinted(chpr);
    return(app12_offset + app_length + 2);
}

/* Read and discard until the next newline in the input stream, or    */
/* until 'max_offset' is reached. Return the last character read.     */

int
skip_past_newline(FILE *inptr,unsigned long max_offset)
{
    int ch;
    while(((unsigned long)ftell(inptr) < max_offset) && ((ch = fgetc(inptr)) != '\n') && ch != EOF)
        continue;
    return(fgetc(inptr));
}

/* Read and discard until the next '[' in the input stream, or until  */
/* 'max_offset' is reached. Return the last character read.           */

int
skip_to_bracket(FILE *inptr,unsigned long max_offset)
{
    int ch = EOF;

    while(((unsigned long)ftell(inptr) < max_offset) && ((ch = fgetc(inptr)) != '[') && ch != EOF)
        continue;
    if(ftell(inptr) >= max_offset)
        ch = 0;
    return(ch);
}

/* Print a word from an APP12 segment. The first character of the     */
/* word is passed in by the caller; if it's a left square bracket, it */
/* is assumed to start a "section" and is indented a little less.     */
/* This can print a lone open-bracket, but what the heck.             */

/* CRs are stripped out and nulls end a word. Newlines are stripped   */
/* as well, but cause the next character to be read and passed back   */
/* to the caller, to be used as the start of a new word. Otherwise    */
/* the last character read is returned.                               */

/* The routine is not allowed to read past 'max_offset', which should */
/* be the end of the segment.                                         */

int
putword(FILE *inptr,int firstch,unsigned long max_offset,int indent)
{
    int eol = 0;
    int chpr = 0;
    int ch = firstch;

    if(firstch)
        print_tag_address(APP_ENTRY,ftell(inptr),indent + SMALLINDENT,"@");
    if(firstch)
    {
        if(firstch != '[')
            extraindent(SMALLINDENT);
        putchar(firstch);
        ++chpr;
    }
    else
        ++eol;
    while(((unsigned long)ftell(inptr) < max_offset) && !eol)
    {
        ch = fgetc(inptr);
        switch(ch)
        {
        case '\r':
            break;
        case 0:
            ++eol;
            break;
        case '\n':
            ++eol;
            ch = fgetc(inptr);
            break;
        case EOF:
            ++eol;
            break;
        default:
            if(isprint(ch))
                putchar(ch);
            ++chpr;
            break;
        }
    }
    setcharsprinted(chpr);
    return(ch);
}



/* Determine if the values for an IFD entry are recorded at an        */
/* offset. Returns 0 if the value is recorded directly in the entry,  */
/* 1 if the value is offset.                                          */

int
is_offset(struct ifd_entry *entry_ptr)
{
    if((value_type_size(entry_ptr->value_type) * entry_ptr->count) > 4)
        return(1);
    return(0);
}

/* Process an Epson Print Image Matching section, included in many    */
/* cameras for the purpose of adjusting image parameters when         */
/* printing the image, according to values pre-determined by the      */
/* device manufacturer. According to Epson, parametes which may be    */
/* adjusted include:                                                  */

/* Gamma Level                                                        */
/* Color Space                                                        */
/* Color Balance                                                      */
/* Contrast                                                           */
/* Brightness                                                         */
/* Sharpness                                                          */
/* Saturation                                                         */
/* Shadow                                                             */
/* Highlight                                                          */

/* Pim version 2 also handles:                                        */

/* Noise Reduction                                                    */
/* Custom Scene setting                                               */

/* This routine knows just enough about the structure of PIM to print */
/* tag numbers and values, but interpretation of tags is not known.   */
/* Values are printed in decimal and hex.                             */

unsigned long
process_pim(FILE *inptr,unsigned short byteorder,unsigned long pim_offset,
                    unsigned long fileoffset_base, unsigned long count,
                    char *tagname,char *dirname,char *prefix,
                    int indent)
{
    unsigned long entry_offset,extra,value;
    unsigned long max_offset = 0L;
    unsigned long max_value_offset = 0L;
    unsigned char *pim_id,*pim_version,*valueptr;
    unsigned short tag;
    int tagwidth = PIMTAGWIDTH;
    int chpr = 0;
    int entry_num,num_entries;

    if(inptr == (FILE *)0)
    {
        fprintf(stderr,"%s: no open file pointer to read Print Image data\n",
                Progname);
        max_offset = 0L;
        return(0L);
    }

    entry_offset = pim_offset + fileoffset_base;
    max_offset = entry_offset + count;

    /* Always starts with "PrintIM\0"; read it as a sanity check      */
    pim_id = read_bytes(inptr,8,entry_offset);
    if((pim_id == (unsigned char *)0) || strncasecmp((char *)pim_id,"PrintIM",8) != 0)
    {
        /* Print the value as a comment here, since we are processing */
        /* no further.                                                */
        if(PRINT_SECTION)
        {
            print_tag_address(SECTION,entry_offset,indent,prefix);
            chpr += printf("%s",tagname);
            if((PRINT_VALUE))
                chpr += printf(": length %lu",count);
        }
        if(!(PRINT_SECTION))
        {
            chpr = newline(chpr);
            printred("# WARNING:");
        }
        printred(" INVALID PrintImage data...");
        return(0L);
    }
    chpr = newline(0);

    pim_version = read_bytes(inptr,6,HERE);
    num_entries = read_ushort(inptr,byteorder,HERE);

    if(PRINT_SECTION)
    {
        print_tag_address(SECTION,entry_offset,indent,prefix);
        chpr += printf("<%s> Version %s, size %lu, %d entries",tagname,pim_version,
                                                                count,num_entries);
        if((num_entries * 6) > (count - 16))
        {
            chpr = newline(chpr);
            num_entries = (count - 16) / 6;
            PUSHCOLOR(HI_RED);
            chpr += printf("# WARNING: PIM claims too many entries for section size, printing %d",num_entries);
            POPCOLOR();
        }
    }
    else
    {

        /* Make pseudo-tags for these                                 */
        if((PRINT_TAGINFO) || (PRINT_VALUE))
            print_tag_address(ENTRY,HERE,indent+SMALLINDENT,"*");
        if(PRINT_TAGINFO)
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",dirname);
            chpr += printf("%s.",tagname);
            chpr += printf("%-*.*s",tagwidth,tagwidth,"Offset");
        }
        if((PRINT_VALUE))
            chpr += printf(" = @%lu",entry_offset);
        chpr = newline(chpr);
        if((PRINT_TAGINFO) || (PRINT_VALUE))
            print_tag_address(ENTRY,HERE,indent+SMALLINDENT,"*");
        if(PRINT_TAGINFO)
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",dirname);
            chpr += printf("%s.",tagname);
            chpr += printf("%-*.*s",tagwidth,tagwidth,"Length");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %lu",count);
        chpr = newline(chpr);
        if((PRINT_TAGINFO) || (PRINT_VALUE))
            print_tag_address(ENTRY,HERE,indent+SMALLINDENT,"*");
        if(PRINT_TAGINFO)
        {
            if((PRINT_LONGNAMES))
                chpr += printf("%s.",dirname);
            chpr += printf("%s.",tagname);
            chpr += printf("%-*.*s",tagwidth,tagwidth,"Version");
        }
        if((PRINT_VALUE))
            chpr += printf(" = %s",pim_version);
        chpr = newline(chpr);
        if((num_entries * 6) > (count - 16))
        {
            chpr = newline(chpr);
            chpr += printf("# WARNING: PIM claims too many entries (%d) for section size; ",num_entries);
            num_entries = (count - 16) / 6;
            PUSHCOLOR(HI_RED);
            chpr += printf("printing %d",num_entries);
            POPCOLOR();
        }
    }
    
    max_value_offset = entry_offset = ftell(inptr);

    indent += SMALLINDENT;
    for(entry_num = 0; entry_num < num_entries; ++entry_num)
    {
        int i;

        chpr = newline(chpr);
        tag = read_ushort(inptr,byteorder,HERE);
        if(ferror(inptr) || feof(inptr))
            goto blewit;
        if((PRINT_ENTRY))
        {
            print_tag_address(ENTRY,entry_offset,indent,prefix);
            if((PRINT_TAGINFO))
            {
                if((PRINT_LONGNAMES))
                    chpr += printf("%s.%s.",dirname,tagname);
                else if((LIST_MODE))
                    chpr += printf("%s.",tagname);
                chpr += printf("PIM_%#06X",(int)tag);
                if((PRINT_VALUE))
                {
                    putindent(tagwidth - 10);
                    chpr += printf(" = ");
                }
            }
        }
        valueptr = read_bytes(inptr,4,HERE);
        value = read_ulong(inptr,byteorder,entry_offset + 2);

        if(PRINT_SECTION)
        {
            if((PRINT_VALUE))
            {
                for(i = 0; i < 4; ++i)
                    chpr += printf("%02x ",(unsigned int)(valueptr[i] & 0xff));
                chpr += printf(" |");
                for(i = 0; i < 4; ++i)
                {
                    if(isprint(valueptr[i]))
                        putchar(valueptr[i]);
                    else
                        putchar('.');  /* can't win 'em all  */
                }
                chpr += printf("| = %#lx/%lu",value,value);
            }
        }
        else if((PRINT_VALUE))
            chpr += printf("%#lx/%lu",value,value);

        max_value_offset = entry_offset = ftell(inptr);
    }

    if(PRINT_SECTION)
    {
        chpr = newline(chpr);
        if((max_value_offset < max_offset) && PRINT_VALUE_AT_OFFSET)
        {
            int printlength = 0;

            print_tag_address(SECTION,max_value_offset,indent,"*");
            PUSHCOLOR(RED);
            chpr += printf("---- End of values before end of %s section",tagname);
            POPCOLOR();
            chpr = newline(chpr);
            extra = max_offset - max_value_offset;
            /* dump the extra as UNDEFINED                            */
            if(Max_undefined)
            {
                if((Max_undefined == DUMPALL) || (Max_undefined > extra))
                    printlength = extra;
                else if(Max_undefined > 0L)
                    printlength = Max_undefined;
                hexdump(inptr,max_value_offset,extra,printlength,6,indent,
                                                    SMALLINDENT+8);
            }
            chpr = newline(chpr);
        }
        indent -= SMALLINDENT;
        print_tag_address(SECTION,max_offset - 1,indent,"-");
        chpr += printf("</%s>",tagname);
    }

    setcharsprinted(chpr);
    return(max_value_offset - 1);
blewit:
    clearerr(inptr);
    return(0L);
}
