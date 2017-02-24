/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002, 2005 by Duane H. Hesser. All rights reserved.  */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef lint
static char *ModuleId = "@(#) $Id: options.c,v 1.21 2005/07/24 17:18:27 alex Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "version.h"
#include "defs.h"
#include "summary.h"
#include "datadefs.h"
#include "extern.h"
#include "maker_extern.h"

int Debug = 0;

int
process_options(int argc,char **argv)
{
    int opt;
    unsigned long bits;
    int nv;

    if((CHECK_TTY_FOR_COLOR))
    {
        if(isatty(fileno(stdout)) == 0)
            Print_options &= ~PCOLOR;
    }
    while((opt = getopt(argc,argv,"ace:hil:m:np:s:tuA:B:C:DILM:N:O:RSU:VXY:Z")) != EOF)
    {
        switch(opt & 0xff)
        {
            case 'I':   /* Indent: after -> before -> none -> after   */
                        if(PRINT_INDENT_AFTER_ADDRESS)
                        {
                            Print_options &= ~INDENT_AFTER_ADDRESS;
                            Print_options |= INDENT_BEFORE_ADDRESS;
                        }
                        else if(PRINT_INDENT_BEFORE_ADDRESS)
                            Print_options &= ~(INDENT_BEFORE_ADDRESS|INDENT_AFTER_ADDRESS);
                        else
                        {
                            Print_options &= ~INDENT_BEFORE_ADDRESS;
                            Print_options |= INDENT_AFTER_ADDRESS;
                        }
                        break;
            /* Modes: structural (default), report, or list           */
            case 'S':   Print_options = DEFAULT_OPTIONS; break;
            case 'R':   Print_options = REPORT_OPTIONS; break;
            case 'L':   Print_options = LIST_OPTIONS; break;


            /* Want hex?...or decimal                                 */
            case 'D':   bits = Print_options & HEX_ONLY;
                        bits = (bits << 1) & DECIMAL_ONLY;
                        Print_options &= ~HEX_ONLY;
                        Print_options |= bits;
                        break;
            case 'X':   bits = Print_options & DECIMAL_ONLY;
                        bits = (bits >> 1) & HEX_ONLY;
                        Print_options &= ~DECIMAL_ONLY;
                        Print_options |= bits;
                        break;

            /* Zero all print options; leaves only image summary and  */
            /* filetype/fileformat. Leave LONGNAMES on.               */
            case 'Z':   Print_options = LONGNAMES_FLAG; break;

            /* toggle use of color highlighting                       */
            case 'c':   Print_options = toggle(Print_options,PCOLOR); break;
            /* toggle printing of offset values in IFDs "(i)nline"    */
            /* with the directory entry (not at the value offset)     */
            case 'i':   Print_options = toggle(Print_options,VALUE_AT_OFFSET); break;

            case 'a':   Print_options = toggle(Print_options,HEXADDRESS | DECADDRESS); break;

            /* 'e'ntry options; which parts of an entry to            */
            /* (de)activate                                           */
            case 'e':   while(*optarg)
                        {
                            switch(*optarg)
                            {
                                case 't': Print_options = toggle(Print_options,TAGNAME); break;
                                case 'n': Print_options = toggle(Print_options,DECTAGNO); break;
                                case 'N': Print_options = toggle(Print_options,HEXTAGNO); break;
                                case 'T': Print_options = toggle(Print_options,TYPE); break;
                                case 'v': Print_options = toggle(Print_options,DECVALUE); break;
                                case 'V': Print_options = toggle(Print_options,HEXVALUE); break;
                                case 'o': Print_options = toggle(Print_options,DECOFFSET); break;
                                case 'O': Print_options = toggle(Print_options,HEXOFFSET); break;
                                case 'r': Print_options = toggle(Print_options,RELOFFSET); break;
                                case 'R': Print_options = toggle(Print_options,RAW_VALUES); break;
                                case 'a': Print_options = toggle(Print_options,ASCII_IGNORE_LENGTH); break;
                                case 'A': Print_options = toggle(Print_options,MULTIVAL_PRINT_ALL); break;
                            }
                            ++optarg;
                        }
                        break;

            case 'n': Print_options = toggle(Print_options,FILENAMES); break;

            /* 'p'rint options; which sections/features to            */
            /* (de)activeate                                          */
            case 'p':   while(*optarg)
                        {
                            switch(*optarg)
                            {
                                case 'l': Print_options = toggle(Print_options,LONGNAMES_FLAG); break;
                                case 's': Print_options = toggle(Print_options,SECTION); break;
                                case 'a': Print_options = toggle(Print_options,APP_ENTRY); break;
                                case 'g': Print_options = toggle(Print_options,SEGMENT); break;
                                case 'm': Print_options |= MAKERNOTE_SCHEME; break;
                                case 'M': Print_options |= MAKERNOTE_SCHEME|SCHEME_DEBUG; break;
                                case 'e': Print_options = toggle(Print_options,ENTRY); break;
                            }
                            ++optarg;
                        }
                        break;

            case 'B':   /* dump binary image segments and failed Jpeg */
                        /* image segments                             */
                        if(optarg)
                        {
                            if(*optarg == 'a')
                                Max_imgdump = DUMPALL;
                            else
                                Max_imgdump = strtoul(optarg,NULL,0);
                        }
                        else
                            Max_imgdump = DUMPALL;
                        break;
            case 'U':   /* dump UNDEFINED segments                    */
                        /* Also affects raw image sections            */
                        if(optarg)
                        {
                            if(*optarg == 'a')
                                Max_undefined = DUMPALL;
                            else
                                Max_undefined = strtoul(optarg,NULL,0);
                        }
                        else
                            Max_undefined = DUMPALL;
                        break;
            case 'M':   /* dump MakerNotes                            */
                        Print_options |= MAKERDUMP;
                        if(optarg)
                        {
                            if(*optarg == 'a')
                                Max_makerdump = DUMPALL;
                            else
                                Max_makerdump = strtoul(optarg,NULL,0);
                        }
                        break;
            case 'N':   /* force a noteversion                        */
                        if(optarg)
                        {
                            nv = atoi(optarg);
                            setoptionnoteversion(nv);
                        }
                        break;
            case 'A':   /* dump APPn segments                         */
                        Print_options |= APPNDUMP;
                        if(optarg)
                        {
                            if(*optarg == 'a')
                                Max_appdump = DUMPALL;
                            else
                                Max_appdump = strtoul(optarg,NULL,0);
                        }
                        break;
            /* Options to force makernote routines to use a specific  */
            /* set of routines to interpret MakerNotes. Scheme        */
            /* detection is automatic and (largely) unaffected        */
            /* (except for certain details which are unavoidably      */
            /* maker-specific). This may be used to check whether a   */
            /* set of MakerNote tags for an undocumented camera       */
            /* actually match a set for which routines exist for      */
            /* another camera                                         */

            /* "Software" has proven unreliable for identification    */
            /* purposes, and is not used for anything. Changing it    */
            /* here will accomplish nothing.                          */

            case 'm':   Use_Make_name = optarg;
                        break;
            case 'l':   Use_Model_name = optarg;
                        break;
            case 's':   Use_Software_name = optarg; /* useless        */
                        break;

            /* help and version                                       */
            case 'h':   usage(); exit(0); break;
            case 'V':   print_version(); break;

            /* Y is this happpening? Random debug for development     */
            /* only; e.g bit 0x4 shows summary debug, bit 0x8 shows   */
            /* noteversion debug...                                   */
            case 'Y':   Debug = strtoul(optarg,(char **)0,0); break;

            /* Print make and/or model names of digital cameras       */
            /* "known" to the program. The format is -C make+model,   */
            /* where 'make' and 'model' are strings subjected to a    */
            /* simple substring match with known info. A missing      */
            /* pattern matches everything, so "-C +" prints all known */
            /* makes and models. As a special case "-C -" prints all  */
            /* makes. Make and Model are significant only for         */
            /* MakerNotes.                                            */
            case 'C':   Camera_name_pattern = optarg;
                        Print_options = toggle(Print_options,CAMERA_NAMES);
                        break;
            case 'O':   Start_offset = strtoul(optarg,0,0);
                        break;
            case 't':   Print_options = toggle(Print_options,TTY_COLOR_CHECK); 
                        break;
            case 'u':   Print_options = toggle(Print_options,UNICODE_FLAG); 
                        break;
            default:    usage();
                        exit(1);
                        break;
        }
    }
    return(optind);
}


/* Grab options from the environment.                                 */
#define MAXARGV 256

extern int vector();

void
env_options()
{
    char *argv[MAXARGV];
    char *envopts;
    int argc;

    envopts = getenv("EXIFPROBE_OPTIONS");
    if(envopts && *envopts)
    {
        char *argstr = calloc(strlen(envopts) + 3, 1);
        strcpy(argstr, "e ");
        strcat(argstr, envopts);
        optind = 0;
        /* make an argument vector                                    */
        argc = vector(argstr, argv, " \t", 0, MAXARGV);
        (void)process_options(argc,argv);
        optind = 1;
        free(argstr);
    }
}

void
usage()
{
    printf("Usage:\n%s [options] filenames(s)\n",Progname);
    printf("\t-h - print this help message\n");
    printf("\t-V - print program version and copyright\n");
    (void)putchar('\n');
    printf("\t-R - Report mode: only tagnames and decimal values, indented, inline\n");
    printf("\t-S - Structure mode: everything, offset values not inline (default)\n");
    printf("\t-L - List mode: list all tags and values (only); no structure\n");
    printf("\t-Z - Zero (turn off) all output flags\n");
    (void)putchar('\n');
    printf("\t-a - toggle print addresses in hex and decimal\n");
    printf("\t-D - toggle print enabled addresses, tag numbers and values in decimal only\n");
    printf("\t-X - toggle print enabled addresses, tag numbers and values in hex only\n");
    printf("\t-I - toggle indent (after address -> before -> none)\n");
    printf("\t-i - toggle \"inline\" print of IFD values\n");
    printf("\t-n - toggle printing of filename at start of each output line\n");
    printf("\t-c - toggle use of color to highlight certain sections\n");
    printf("\t-u - print all 16 bits of unicode data\n");
    (void)putchar('\n');

    printf("\t-p[items] - toggle print identifiers for:\n");
    printf("\t\ts - sections\n");
    printf("\t\tg - segments\n");
    printf("\t\te - IFD entries\n");
    printf("\t\ta - expand known entries in APP0...APPN segents\n");
    printf("\t\tm - print MakerNote scheme detection info\n");
    printf("\t\tM - debug MakerNote scheme detection info\n");
    printf("\t\tl - long tagnames (default in List mode)\n");
    (void)putchar('\n');

    printf("\t-e[items] - toggle print IFD entry items:\n");
    printf("\t\tt - tagname\n");
    printf("\t\tn - tag number in decimal\n");
    printf("\t\tN - tag number in hex\n");
    printf("\t\tT - entry type\n");
    printf("\t\tv - value in decimal\n");
    printf("\t\tV - value in hex\n");
    printf("\t\to - file offset to value in decimal\n");
    printf("\t\tO - file offset to value in hex\n");
    printf("\t\tr - relative (unadjusted) offset in decimal\n");
    printf("\t\tR - print \"raw\" values where expansion of values is needed\n");
    printf("\t\ta - print ascii strings until null, rather than by length\n");
    printf("\t\tA - print ALL elements of multiple-value tags\n");
    (void)putchar('\n');

    printf("\t-M[len|a] - hex/ascii dump 'len' (or all) bytes of unknown MakerNotes\n");
    printf("\t-A[len|a] - hex/ascii dump 'len' (or all) bytes of unknown APPn segments\n");
    printf("\t-U[len|a] - hex/ascii dump 'len' (or all) bytes of UNDEFINED data of unknown format\n");
    printf("\t-B[len|a] - hex/ascii dump 'len' (or all) bytes of binary images or invalid JPEG data\n");
    printf("\t-N[num]   - force noteversion 'num' for MakerNote interpretation\n");
    printf("\t-m[name]  - force use of maker 'name' to select MakerNote interpretation routines\n");
    printf("\t-l[model] - force use of 'model' to select MakerNote interpretation routines\n");
    (void)putchar('\n');
    printf("\t-O[offset]       - start processing at 'offset' in file\n");
    printf("\t-C[make]+[model] - print makes matching 'make', models matching 'model' (substrings)\n");
    (void)putchar('\n');
    print_version();
}

extern char *Comptime;

void
print_version()
{
    printf("\tProgram: '%s' version %s",Progname,Program_version);
    if(PATCHLEVEL > 0)
        printf(" patchlevel %d",PATCHLEVEL);
    (void)putchar('\n');
    if(Comptime)
        printf("\tCompiled: %s\n",Comptime);
    printf("\t%s\n",Copyright);
    printf("\t\t(open source; see LICENSE.EXIFPROBE)\n");
}
