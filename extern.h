/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: extern.h,v 1.33 2005/07/24 21:18:02 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef EXTERN_INCLUDED
#define EXTERN_INCLUDED

extern int optind;
extern char *optarg;
extern int getopt();

extern int Debug;

extern char *Progname;
extern long Header_offset;
extern unsigned long Max_undefined;
extern unsigned long Max_makerdump;
extern unsigned long Max_appdump;
extern unsigned long Max_imgdump;
extern unsigned long Print_options;

extern unsigned long Start_offset;

extern char *Make_name;
extern char *Model_name;
extern char *Software_name;

/* Used when forcing camera make, model, or software (-m, -l, -s)     */
extern char *Use_Make_name;
extern char *Use_Model_name;
extern char *Use_Software_name;

extern char *Camera_name_pattern;

/* =================== Function prototypes =========================  */
extern int process_options(int,char **);
extern void env_options();
extern void print_byteorder(unsigned short,int);
extern void print_magic(unsigned long,int);

extern unsigned long process_tiff_ifd(FILE *,unsigned short,unsigned long,
                                        unsigned long,unsigned long,
                                        struct image_summary *,char *,
                                        int,int,int,int);
extern unsigned long process_exif_ifd(FILE *,unsigned short,unsigned long,
                                        unsigned long,unsigned long,
                                        struct image_summary *,char *,
                                        int,int);
extern unsigned long process_gps_ifd(FILE *,unsigned short,unsigned long,
                                        unsigned long,unsigned long,
                                        struct image_summary *,char *,
                                        int,int);
extern unsigned long process_subifd(FILE *,unsigned short,struct ifd_entry *,
                                        unsigned long,unsigned long,
                                        struct image_summary *,char *,
                                        int,int,int,int);
extern unsigned long process_makernote(FILE *,unsigned short,struct ifd_entry *,
                                        unsigned long,unsigned long,
                                        struct image_summary *,char *,
                                        int);
extern unsigned long process_app0(FILE *,unsigned long,unsigned short,
                                                struct image_summary *,
                                                char *,int);
extern unsigned long process_app1(FILE *,unsigned long,unsigned short,
                                            struct image_summary *,
                                            char *,int);
extern unsigned long process_app3(FILE *,unsigned long,unsigned short,
                                            struct image_summary *,
                                            char *,int);
extern unsigned long process_app12(FILE *,unsigned long,unsigned short,
                                                struct image_summary *,
                                                char *,int);
extern unsigned long process_appn(FILE *,unsigned long,unsigned short,
                                                struct image_summary *,
                                                char *,int);
extern unsigned long process_jpeg_segments(FILE *,unsigned long,
                                            unsigned short,unsigned long,
                                            struct image_summary *,char *,
                                            char *,int);
extern unsigned long process_pim(FILE *,unsigned short,unsigned long,
                                        unsigned long, unsigned long,char *,
                                        char *,char *,int);
extern int jpeg_status(int);
extern void print_jpeg_status();

extern int skip_past_newline(FILE *,unsigned long);
extern int skip_to_bracket(FILE *,unsigned long);
extern int putword(FILE *,int,unsigned long,int);
extern int invalid_ifd_entry(FILE *,unsigned long,unsigned long,int,unsigned short);
extern unsigned short valid_tiffhdr(FILE *,unsigned long,int);
extern char *check_printable(char *,int);
extern char *splice(char *,char *,char *);
extern int newline(int);
extern void setcharsprinted(int);
extern int charsprinted();
extern void putcolorchar(unsigned short);

extern struct ifd_entry *read_ifd_entry(FILE *,unsigned short,unsigned long);
extern unsigned long read_ulong(FILE *,unsigned short,unsigned long);
extern unsigned long long read_ulong64(FILE *,unsigned short,unsigned long);
extern unsigned long to_ulong(unsigned char *,unsigned short);
extern unsigned long long to_ulong64(unsigned char *,unsigned short);
extern unsigned short read_ushort(FILE *,unsigned short,unsigned long);
extern unsigned short to_ushort(unsigned char *,unsigned short);
extern float read_float(FILE *,unsigned short,unsigned long);
extern float to_float(unsigned long);
extern double read_double(FILE *,unsigned short,unsigned long);
extern double to_double(unsigned char *,unsigned short);
extern unsigned short read_ubyte(FILE *,unsigned long);
extern struct fileheader *read_imageheader(FILE *,unsigned long);

extern unsigned char *read_bytes(FILE *,unsigned long,unsigned long);
extern char *read_appstring(FILE *,unsigned short,unsigned long);
extern char *read_string(FILE *,unsigned long,unsigned long);

extern void print_ubytes(FILE *,unsigned long,unsigned long);
extern void print_sbytes(FILE *,unsigned long,unsigned long);
extern void print_ushort(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_sshort(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_ulong(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_slong(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_urational(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_srational(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_float(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_double(FILE *,unsigned long,unsigned short,unsigned long);
extern void print_user_comment(FILE *,unsigned long,unsigned long,unsigned short);
extern void print_cfapattern(FILE *,unsigned long,unsigned short,unsigned long,
                                                                char *,char *);
extern void print_version();
extern void print_startvalue();
extern void print_endvalue();

extern int print_filetype(unsigned long,unsigned short);
extern unsigned long get_filesize(FILE *inptr);
extern int ateof(FILE *);

extern void setfilename(char *);
extern void print_filename();
extern char *find_camera_name(int);
extern void clear_memory();
extern void usage();

extern void print_summary(struct image_summary *);
extern int print_image_summary_entry(struct image_summary *,char *);
extern int scan_summary(struct image_summary *);
extern void print_imageformat(struct image_summary *);
extern void print_imagecompression(struct image_summary *);
extern void print_tiff_compression(struct image_summary *);
extern void print_jpeg_compression(struct image_summary *);
extern void print_jp2c_compression(struct image_summary *);
extern void print_crw_compression(struct image_summary *);
extern void print_raf_compression(struct image_summary *);
extern void print_imagesubformat(struct image_summary *);
extern void print_imagesubtype(struct image_summary *);
extern void print_imagesize(struct image_summary *);
extern void print_location(struct image_summary *);
extern char *tiff_compression_string(unsigned long);
extern struct image_summary *new_summary_entry(struct image_summary *,int,int);
extern struct image_summary *last_summary_entry(struct image_summary *);
extern struct image_summary *destroy_summary(struct image_summary *);
extern int lock_number(struct image_summary *);
extern void print_fileformat(struct image_summary *);

extern void print_tag_address(unsigned long,unsigned long,int,char *);
extern unsigned long print_entry(FILE*,unsigned short,struct ifd_entry *,
                                    unsigned long,struct image_summary *,
                                    char *,int,int,int,int);
extern int print_ascii(FILE *,unsigned long,unsigned long);
extern int print_unicode(FILE *,unsigned long,unsigned long,unsigned short);
extern int show_string(char *,int,int);
extern int show_string(char *,int,int);
extern int print_header(struct fileheader *,unsigned long);
extern int print_tiff_header(unsigned short,unsigned short,unsigned long);
extern int value_type_size(unsigned short);
extern void print_tagid(struct ifd_entry *,int,int);
extern void print_taginfo(struct ifd_entry *,char *,int,int,int,int);
extern void print_value(struct ifd_entry *,char *);
extern unsigned long print_offset_value(FILE *,unsigned short,
                                        struct ifd_entry *,unsigned long,
                                        char *,int,int,int);
extern unsigned long sum_strip_bytecounts(FILE *,unsigned short,unsigned long,
                                                unsigned long,unsigned short);
extern void display_end_of_ifd(unsigned long,int,int,int,int,char *,char *);

extern int is_offset(struct ifd_entry *);
extern int is_a_long_offset(struct ifd_entry *);
extern char *value_type_name(unsigned short);

extern char *tagname(unsigned short);
extern char *interop_tagname(unsigned short);
extern char *gps_tagname(unsigned short);
extern char *get_type_name(int);

extern void putindent(int);
extern void extraindent(int);
extern void pushcolor(int);
extern void popcolor();
extern void printred(char *);
extern void why(FILE *);
extern unsigned long toggle(unsigned long,unsigned long);
extern void hexdump(FILE *,unsigned long,unsigned long,unsigned long,
                                                        int,int,int);

/* =============== Routines to interpret values ====================  */
extern void interpret_value(struct ifd_entry *,struct image_summary *);
extern void interpret_offset_value(FILE *,struct ifd_entry *,unsigned short,
                                                            unsigned long);

/* ================= Private IFDs ====================                */
extern unsigned long process_private_ifd(FILE *,unsigned short,unsigned long,
                                            unsigned long,unsigned long,
                                            unsigned short,struct image_summary *,
                                            char *,char *,int,int,int);
extern unsigned long print_private_entry(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,unsigned short,
                                            struct image_summary *,
                                            char *,char *,int,int,int);
extern void print_private_taginfo(struct ifd_entry *,unsigned short,char *,int,int);
extern void print_private_tagid(struct ifd_entry *,unsigned short,int,int);
extern void interpret_pe_value(struct ifd_entry *,unsigned short,int,char *);
extern unsigned long print_offset_pe_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,unsigned short,
                                            struct image_summary *,char *,char*,
                                            int,int,int,int);
extern char *private_tagname(int,unsigned short,unsigned short);
extern int private_value_is_offset(int,unsigned short,unsigned short);
extern void print_private_offset_value(FILE *,unsigned short,struct ifd_entry *,
                                        unsigned long,unsigned short,char *,
                                        char *,int,int,int,int);

extern int print_raf_header(FILE *,struct fileheader *,unsigned long);
extern unsigned long process_raf(FILE *,unsigned long, struct image_summary *, char *,int);
extern void dumpsection(FILE *,unsigned long,unsigned long,int);
#endif /* EXTERN_INCLUDED */
