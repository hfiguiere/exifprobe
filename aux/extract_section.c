/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002,2005 by Duane H. Hesser. All rights reserved.   */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* This program extracts an arbitrary section of a file from          */
/* 'start_offset' to 'end_offset', writing it to a file named with    */
/* the original filename suffixed by ".%d-%d" start,end.              */

/* section to be extracted may be specified by a second argument      */
/* "@start:length" or by second and third arguments "start end".      */

/* This is intended to extract sections from image files, and is much */
/* faster than 'dd' for the purpose.                                  */

#ifndef lint
static char *ModuleId = "@(#) $Id: extract_section.c,v 1.1 2005/06/30 17:50:27 alex Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <values.h>

#define END_OF_FILE MAXINT
#define COPYBUFSIZE 16384
#define NAMELENGTH  1024

extern char *optarg;
char *Progname;

main(int argc,char **argv)
{
    char copybuf[COPYBUFSIZE];
    char outfilename[NAMELENGTH];
    char *arg,*infilename,*colon;
    unsigned long start_offset = 0UL;
    unsigned long end_offset = END_OF_FILE;
    int opt;
    FILE *ifp,*ofp;
    int readsize = 0;
    int bytesread = 0;
    int byteswritten = 0;
    int bytescopied = 0;
    int sizeleft = 0;
    int onamelength = 0;

    Progname = *argv;
    if(argc >= 2)
    {
        infilename = *++argv;
        arg = *++argv;
        if(arg && (*arg == '@'))
        {
            start_offset = strtoul(++arg,(char **)0,0);
            colon = strchr(arg,':');
            if(colon)
                end_offset = start_offset + strtoul(++colon,(char **)0,0) - 1;
        }
        else if(arg)
        {
            start_offset = strtoul(arg,(char **)0,0);
            if(*++argv)
            {
                arg = *argv;
                end_offset = strtoul(arg,(char **)0,0);
            }
        }
    }
    if(infilename)
    {
        if(end_offset == END_OF_FILE)
            onamelength = snprintf(outfilename,NAMELENGTH,"%s.%d-eof",infilename,start_offset);
        else
            onamelength = snprintf(outfilename,NAMELENGTH,"%s.%d-%d",infilename,start_offset,end_offset);
        if(onamelength < (strlen(infilename) + 2))
        {
            fprintf(stderr,"%s: can't make output name",Progname);
            perror("\tbecause");
            exit(2);
        }
    }
    else
    {
        fprintf(stderr,"%s: no input file name\n",Progname);
        exit(1);
    }

    printf("copy from %s to %s\n",infilename,outfilename);
    printf("start_offset = %lu, ",start_offset);
    sizeleft =  end_offset - start_offset + 1;
    if(end_offset == END_OF_FILE)
        printf("end_offset = end of file\n");
    else
        printf("end_offset = %d, copy length = %d\n",end_offset,sizeleft);

    if((ifp = fopen(infilename,"r")) != (FILE *)0)
    {
        if((ofp = fopen(outfilename,"w")) == (FILE *)0)
        {
            fprintf(stderr,"%s: could not open output file \"%s\"\n",Progname,outfilename);
            perror("\tbecause");
            exit(3);
        }
        fseek(ifp,start_offset,SEEK_SET);
        if(sizeleft < COPYBUFSIZE)
            readsize = sizeleft;
        else
            readsize = COPYBUFSIZE;
        while(sizeleft && ((bytesread = fread(copybuf,1,readsize,ifp)) > 0))
        {
            byteswritten = fwrite(copybuf,1,bytesread,ofp);
            sizeleft -= byteswritten;
            bytescopied += byteswritten;
            if(sizeleft < COPYBUFSIZE)
                readsize = sizeleft;
        }
        clearerr(ifp);
        printf("copied %d bytes from %d to %d\n",bytescopied,start_offset,ftell(ifp) - 1);

    }
    else
    {
        fprintf(stderr,"%s: could not open input file \"%s\"\n",Progname,infilename);
        perror("\tbecause");
        exit(3);
    }
    exit(0);
}
