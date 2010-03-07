/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: global.h,v 1.6 2005/06/03 13:23:25 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef GLOBAL_INCLUDED
#define GLOBAL_INCLUDED

char *Progname;
unsigned long Print_options = DEFAULT_OPTIONS;
unsigned long Max_undefined = 0UL;
unsigned long Max_makerdump = 0UL;
unsigned long Max_appdump = 0UL;
unsigned long Max_imgdump = 0UL;

unsigned long Start_offset =  0UL;

char *Make_name = (char *)0;
char *Model_name = (char *)0;
char *Software_name = (char *)0;

/* Used for undocumented -m, -l, -s options.                          */
char *Use_Make_name = (char *)0;
char *Use_Model_name = (char *)0;
char *Use_Software_name = (char *)0;

char *Camera_name_pattern = (char *)0;

#endif
