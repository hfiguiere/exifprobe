/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: defs.h,v 1.29 2005/07/24 17:04:17 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* General defines, used everywhere                                   */

#ifndef DEFS_INCLUDED
#define DEFS_INCLUDED

#define HERE    (unsigned long)0xffffffff

#define FNULL           (FILE *)0
#define CNULL           (char *)0
#define NULLSTRING      ""
#define MAXBUFLEN       8192
#define MAXIFDS         1024
#define MAXINDENT       80
#define NOINDENT        0
#define SUBINDENT       2
#define SMALLINDENT     2
#define MEDIUMINDENT    4
#define LARGEINDENT     8
#define ADDRWIDTH       8
#define DUMPALL         0xffffffffUL

#define DEF_DUMPLENGTH          64      /* Used for unknown files     */
#define DEF_SHORTDUMP           36      /* Used for automatic dumps   */


#define HEXTAGNO                0x00000001UL
#define DECTAGNO                0x00000002UL
#define TAGNO                   0x00000003UL

#define HEXVALUE                0x00000010UL
#define DECVALUE                0x00000020UL
#define VALUE                   0x00000030UL

#define CAMERA_NAMES            0x00000400UL
#define FILENAMES               0x00000800UL

#define HEXOFFSET               0x00000100UL
#define DECOFFSET               0x00000200UL
#define OFFSET                  0x00000300UL

#define TAGNAME                 0x00001000UL
#define TYPE                    0x00002000UL

#define TAG_OPTIONS             (TAGNO|TAGNAME|TYPE)

#define MAKERNOTE_SCHEME        0x00000004UL
#define SCHEME_DEBUG            0x00000008UL


#define HEXADDRESS              0x00010000UL
#define DECADDRESS              0x00020000UL
#define ADDRESS                 0x00030000UL
#define SECTION                 0x00040000UL
#define SEGMENT                 0x00080000UL

#define APP_ENTRY               0x00100000UL
#define MAKERDUMP               0x00200000UL
#define APPNDUMP                0x00400000UL
#define IMAGEDUMP               0x00800000UL

#define VALUE_AT_OFFSET         0x01000000UL   
#define RAW_VALUES              0x02000000UL
#define RELOFFSET               0x04000000UL
#define MULTIVAL_PRINT_ALL      0x08000000UL

#define INDENT_AFTER_ADDRESS    0x10000000UL
#define INDENT_BEFORE_ADDRESS   0x20000000UL
#define ASCII_IGNORE_LENGTH     0x40000000UL
#define TTY_COLOR_CHECK         0x00000080UL
#define UNICODE_FLAG            0x00004000UL    /* write both bytes   */

#define ENTRY                   (TAG_OPTIONS|VALUE|OFFSET|TYPE|RELOFFSET)
#define DEF_ENTRY               (TAG_OPTIONS|DECVALUE|OFFSET|TYPE)

#define LONGNAMES_FLAG          0x00000040UL
#define LIST_OPTIONS            (TAGNAME|DECVALUE|ASCII_IGNORE_LENGTH|LONGNAMES_FLAG|APP_ENTRY)

#define DEFAULT_OPTIONS         (SECTION|SEGMENT|DEF_ENTRY|APP_ENTRY|VALUE_AT_OFFSET|DECADDRESS|HEXADDRESS|PCOLOR|INDENT_AFTER_ADDRESS)
#define REPORT_OPTIONS          (TAGNAME|DECVALUE|PCOLOR|SECTION|SEGMENT|APP_ENTRY|ASCII_IGNORE_LENGTH|INDENT_AFTER_ADDRESS)

#define HEX_ONLY                (HEXTAGNO|HEXOFFSET|HEXVALUE|HEXADDRESS)
#define DECIMAL_ONLY            (DECTAGNO|DECOFFSET|DECVALUE|DECADDRESS)

#define LIST_MODE               ((Print_options & (SECTION|VALUE_AT_OFFSET)) == 0)

#define PRINT_ADDRESS           (Print_options & (ADDRESS))
#define PRINT_HEX_ADDRESS       (Print_options & HEXADDRESS)
#define PRINT_DEC_ADDRESS       (Print_options & DECADDRESS)
#define PRINT_BOTH_ADDRESS      ((PRINT_ADDRESS) == (ADDRESS))

#define PRINT_TAGNO             (Print_options & (HEXTAGNO|DECTAGNO))
#define PRINT_HEX_TAGNO         (Print_options & HEXTAGNO)
#define PRINT_DEC_TAGNO         (Print_options & DECTAGNO)
#define PRINT_BOTH_TAGNO        ((PRINT_TAGNO) == (HEXTAGNO|DECTAGNO))


#define PRINT_VALUE             (Print_options & (HEXVALUE|DECVALUE))
#define PRINT_HEX_VALUE         (Print_options & HEXVALUE)
#define PRINT_DEC_VALUE         (Print_options & DECVALUE)
#define PRINT_BOTH_VALUE        ((PRINT_VALUE) == (HEXVALUE|DECVALUE))

#define PRINT_OFFSET            (Print_options & (HEXOFFSET|DECOFFSET))
#define PRINT_HEX_OFFSET        (Print_options & HEXOFFSET)
#define PRINT_DEC_OFFSET        (Print_options & DECOFFSET)
#define PRINT_BOTH_OFFSET       ((PRINT_OFFSET) == (HEXOFFSET|DECOFFSET))


#define PRINT_TAGNAME           (Print_options & TAGNAME)
#define PRINT_TYPE              (Print_options & TYPE)
#define PRINT_TAGINFO           (Print_options & TAG_OPTIONS)

#define PRINT_LONGNAMES         (Print_options & LONGNAMES_FLAG)
#define PRINT_UNICODE           (Print_options & UNICODE_FLAG)
#define PRINT_ENTRY             (Print_options & ENTRY)
#define PRINT_SEGMENT           (Print_options & SEGMENT)
#define PRINT_SECTION           (Print_options & SECTION )
#define PRINT_APPENTRY          (Print_options & APP_ENTRY)
#define PRINT_ANYTHING          (Print_options & (ENTRY|SEGMENT|SECTION))

#define PRINT_INDENT_BEFORE_ADDRESS (Print_options & INDENT_BEFORE_ADDRESS)
#define PRINT_INDENT_AFTER_ADDRESS  (Print_options & INDENT_AFTER_ADDRESS)
#define PRINT_INDENT                (Print_options & (INDENT_BEFORE_ADDRESS|INDENT_AFTER_ADDRESS))
#define PRINT_RAW_VALUES            (Print_options & RAW_VALUES)
#define PRINT_ENTRY_RELOFFSET       (Print_options & RELOFFSET)
#define PRINT_VALUE_AT_OFFSET       (Print_options & VALUE_AT_OFFSET)
#define PRINT_ASCII_IGNORE_LENGTH   (Print_options & ASCII_IGNORE_LENGTH)
#define PRINT_MAKERNOTE_SCHEME      (Print_options & MAKERNOTE_SCHEME)
#define PRINT_SCHEME_DEBUG          (Print_options & SCHEME_DEBUG)
#define PRINT_MAKERDUMP             (Print_options & MAKERDUMP)
#define PRINT_APPNDUMP              (Print_options & APPNDUMP)
#define PRINT_MULTIVAL_ALL          (Print_options & MULTIVAL_PRINT_ALL)
#define PRINT_CAMERA_NAMES          (Print_options & CAMERA_NAMES)
#define PRINT_FILENAMES             (Print_options & FILENAMES)
#define USE_COLOR                   (Print_options & PCOLOR)
#define CHECK_TTY_FOR_COLOR         (Print_options & TTY_COLOR_CHECK)

#define PREFIX                      ""

#define UNKNOWN_TYPE            -1
#define POSSIBLE_PRIMARY_TYPE    0   /* can't be sure which is primary     */
#define PRIMARY_TYPE             1
#define THUMBNAIL_TYPE           2
#define PAGE_TYPE                3
#define MASK_TYPE                4
#define REDUCED_RES_TYPE         5   /* not full size or marked thumbnail  */


#define UNKNOWN_STRING              NULLSTRING
#define POSSIBLE_PRIMARY_STRING     "primary?"
#define PRIMARY_STRING              "primary"
#define THUMBNAIL_STRING            "thumbnail"
#define PAGE_STRING                 "image page"
#define MASK_STRING                 "image mask"
#define REDUCED_RES_STRING          "reduced-resolution"
#define UNSPECIFIED_STRING          ""
#define QSTRING                     "???"
#define UNKNOWN_CAMERA_STRING       "unknown camera"

/* Status of the jpeg processor                                       */
#define JPEG_NO_ERROR    0x0
#define JPEG_EARLY_EOI   0x1      /* found eoi before end of section  */
#define JPEG_NO_EOI      0x2      /* soft error                       */
#define JPEG_NO_SOI      0x4      /* didn't find jpeg                 */
#define JPEG_HAD_ERROR   0x8      /* failed somewhere after SOI       */

/* How many consecutive invalid IFD entries will be tolerated before  */
/* admitting that a TIFF IFD is unreadable?                           */
#define MAX_INVALID_ENTRIES 6

#define TAGWIDTH    27
#define PIMTAGWIDTH 17

/* IFD types                                                          */
#define TIFF_IFD        0
#define TIFF_SUBIFD     1
#define EXIF_IFD        2
#define INTEROP_IFD     3
#define GPS_IFD         4
#define MAKER_IFD       5
#define MAKER_SUBIFD    6


#ifdef COLOR
#define PCOLOR          0x80000000UL
#define PUSHCOLOR(a)      pushcolor(a)
#define POPCOLOR()       popcolor()
#else
#define PCOLOR          0UL
#define PUSHCOLOR(a)
#define POPCOLOR()
#endif

#ifdef COLOR

/* numerical values for ANSI color print sequences                    */
#define GREEN       32  /* "\033[32m"                                 */
#define HI_GREEN    92  /* "\033[92m"                                 */
#define BLUE        34  /* "\033[34m"                                 */
#define HI_BLUE     94  /* "\033[94m"                                 */
#define RED         31  /* "\033[31m"                                 */
#define HI_RED      91  /* "\033[91m"                                 */
#define MAGENTA     35  /* "\033[35m"                                 */
#define HI_MAGENTA  95  /* "\033[95m"                                 */

#define BLACK       30  /* "\033[30m"                                 */
#define HI_BLACK    90  /* "\033[90m"                                 */

#define BG_BLACK    40  /* "\033[40m"                                 */
#define BG_HI_BLACK 100 /* "\033[100m"                                */
#define BG_WHITE    47  /* "\033[47m"                                 */
#define BG_HI_WHITE 107 /* "\033[107m"                                */

#define END_COLOR   0   /* "\033[m"                                   */
#define BOLD_COLOR  1   /* "\033[1m"                                  */


/* select colors for file sections                                    */
#define SUMMARY_COLOR   HI_BLACK
#define EXIF_COLOR      MAGENTA
#define GPS_COLOR       BLACK
#define MAKER_COLOR     HI_BLUE
#define INTEROP_COLOR   GREEN
#define JPEG_COLOR      BLACK
#define UNCERTAIN_COLOR HI_BLACK

#else
#define GREEN       0
#define HI_GREEN    0
#define BLUE        0
#define HI_BLUE     0
#define BOLD_BLUE   0
#define RED         0
#define HI_RED      0
#define BOLD_RED    0

#define BLACK       0
#define HI_BLACK    0
#define BOLD_BLACK  0

#define BG_WHITE    0
#define BG_HI_WHITE 0
#define END_COLOR   0   

#define IM_COLOR        0
#define MAKER_COLOR     0
#define EXIF_COLOR      0
#define GPS_COLOR       0
#define INTEROP_COLOR   0
#define JPEG_COLOR      0

#endif  /* COLOR */

/* ###%%% temporary?                                                  */
#define JPEG_MARKER_DEBUG       0x01
#define JPEG_EOI_DEBUG          0x02
#define SCAN_DEBUG              0x04
#define NOTE_DEBUG              0x08
#define END_OF_SECTION_DEBUG    0x10
#define OUT_DEBUG               0x20

#endif  /* DEFS_INCLUDED */
