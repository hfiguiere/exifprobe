/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: maker_extern.h,v 1.3 2005/06/25 15:11:43 alex Exp $ */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* General definitions for makernotes                                 */

#ifndef MAKER_EXTERN_INCLUDED
#define MAKER_EXTERN_INCLUDED

#include "maker_datadefs.h"

/* ====================== Maker functions ==========================  */

/* Generic                                                            */
extern void print_makertagid(struct ifd_entry *,int,char *,int,int);
extern void print_makertaginfo(struct ifd_entry *,char *,int,int,int);
extern int maker_value_is_offset(struct ifd_entry *,int,int);
extern char *maker_tagname(unsigned short,int,int);
extern int check_makernote(FILE*,unsigned long);
extern void print_scheme(FILE *,struct maker_scheme *);
extern struct maker_scheme *retrieve_makerscheme();
extern void clear_makerscheme();
extern int maker_number(char *);
extern int model_number(int,char *,char *);

extern struct maker_scheme *find_makerscheme(FILE *,unsigned long,
                                                        unsigned long,
                                                        unsigned short,
                                                        unsigned long,
                                                        int,int);
extern unsigned long print_makerentry(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,int,int,int);
extern void print_makervalue(struct ifd_entry *,int,int,char *);
extern unsigned long print_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);

extern void print_generic_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                char *,char *,
                                                int,int,int,int);

extern void print_camera_makes();
extern void print_camera_models(struct camera_id *,char *);
extern void find_maker_model(struct maker_scheme *,int,int);
extern void setnoteversion(int);
extern void setoptionnoteversion(int);
extern void setnotetagset(int);
extern int getnoteversion();
extern int getnotetagset();


/* Agfa */
extern char *maker_agfa_tagname(unsigned short,int);
extern char *maker_agfa1_tagname(unsigned short);
extern void print_agfa_makervalue(struct ifd_entry *,int,int,char *);
extern void print_agfa1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_agfa_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void agfa1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char*,
                                                int,int,int,int);
extern void agfa1_interpret_value(struct ifd_entry *);
extern void agfa1_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long);
extern int agfa_model_number(char *,char *);

/* Asahi/Pentax */
extern char *maker_asahi_tagname(unsigned short,int);
extern char *maker_asahi1_tagname(unsigned short,int);
extern char *maker_asahi2_tagname(unsigned short,int);
extern char *maker_asahi3_tagname(unsigned short,int);
extern void print_asahi_makervalue(struct ifd_entry *,int,int,char *);
extern void print_asahi1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_asahi_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void asahi1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char*,char *,
                                                int,int,int,int);
extern void asahi2_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char*,char *,
                                                int,int,int,int);
extern void asahi1_interpret_value(struct ifd_entry *);
extern void asahi2_interpret_value(struct ifd_entry *);
extern void asahi3_interpret_value(struct ifd_entry *);
extern int asahi_model_number(char *,char *);
extern int set_asahi_noteversion();
extern int maker_asahi_value_is_offset(struct ifd_entry *,int );

#include "canon_extern.h"

/* Casio */
extern char *maker_casio_tagname(unsigned short,int);
extern char *maker_casio1_tagname(unsigned short,int);
extern char *maker_casio2_tagname(unsigned short,int);
extern int set_casio_noteversion();

extern void print_casio_makervalue(struct ifd_entry *,int,int,char *);
extern void print_casio1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_casio_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void casio1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void casio2_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void casio1_interpret_value(struct ifd_entry *);
extern void casio2_interpret_value(struct ifd_entry *);
extern int casio_model_number(char *,char *);
extern int maker_casio_value_is_offset(struct ifd_entry *,int );

/* Epson */
extern char *maker_epson_tagname(unsigned short,int);
extern char *maker_epson1_tagname(unsigned short,int);
extern void print_epson_makervalue(struct ifd_entry *,int,int,char *);
extern void print_epson1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_epson_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void epson1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void epson1_interpret_value(struct ifd_entry *);
extern void epson1_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long);
extern int epson_model_number(char *,char *);

/* Fujifilm */
extern char *maker_fujifilm_tagname(unsigned short,int);
extern char *maker_fujifilm1_tagname(unsigned short,int);
extern void print_fujifilm_makervalue(struct ifd_entry *,int,int,char *);
extern void print_fujifilm1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_fujifilm_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void fujifilm1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                                int,int,int,int);
extern void fujifilm1_interpret_value(struct ifd_entry *);
extern int fujifilm_model_number(char *,char *);

/* HP */
extern int hp_model_number(char *,char *);

/* Kodak */
extern int kodak_model_number(char *,char *);

/* Konica */
extern char *maker_konica_tagname(unsigned short,int);
/* extern char *maker_konica1_tagname(unsigned short,int);            */
extern void print_konica_makervalue(struct ifd_entry *,int,int,char *);
extern void print_konica1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_konica_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void konica1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern int konica_model_number(char *,char *);


/* Kyocera */
extern char *maker_kyocera_tagname(unsigned short,int);
extern void print_kyocera_makervalue(struct ifd_entry *,int,int,char *);
extern void print_kyocera1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_kyocera_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void kyocera1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern int kyocera_model_number(char *,char *);

/* Leica */
extern char *maker_leica_tagname(unsigned short,int);
extern char *maker_leica1_tagname(unsigned short,int);
extern void print_leica_makervalue(struct ifd_entry *,int,int,char *);
extern void print_leica1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_leica_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void leica1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void leica1_interpret_value(struct ifd_entry *);
extern int leica_model_number(char *,char *);

/* Minolta */
extern char *maker_minolta_tagname(unsigned short,int);
extern char *maker_minolta1_tagname(unsigned short,int);
extern void print_minolta_makervalue(struct ifd_entry *,int,int,char *);
extern void print_minolta1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_minolta_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void minolta1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void minolta1_interpret_value(struct ifd_entry *,int model);
extern void minolta1_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long);
extern int minolta_model_number(char *,char *);
extern int maker_minolta_value_is_offset(struct ifd_entry *,int );
extern unsigned long minolta_camerasetting(FILE *,unsigned short,int,char *,
                                                        struct ifd_entry *,
                                                        unsigned long,int);

/* Nikon */
extern char *maker_nikon_tagname(unsigned short,int);
extern char *maker_nikon1_tagname(unsigned short,int);
extern char *maker_nikon2_tagname(unsigned short,int);
extern int set_nikon_noteversion();

extern void print_nikon_makervalue(struct ifd_entry *,int,int,char *);
extern void print_nikon_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void nikon1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void nikon2_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void nikon1_interpret_value(struct ifd_entry *);
extern void nikon2_interpret_value(struct ifd_entry *);
extern void nikon2_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,char *,
                                                char *,int);
extern void nikon_colorbalance(FILE *,struct ifd_entry *,unsigned short,
                                                    unsigned long,int);
extern int nikon_model_number(char *,char *);
extern int maker_nikon_value_is_offset(struct ifd_entry *,int );

/* Olympus */
extern char *maker_olympus_tagname(unsigned short,int);
extern char *maker_olympus1_tagname(unsigned short,int);
extern void print_olympus_makervalue(struct ifd_entry *,int,int,char *);
extern void print_olympus1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_olympus_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void olympus1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void olympus1_interpret_value(struct ifd_entry *);
extern void olympus1_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long);
extern int olympus_model_number(char *,char *);

extern void olympus_interpret_pe_value(struct ifd_entry *,unsigned short,char *);
extern void olympus2010_interpret_value(struct ifd_entry *,char *);
extern void olympus2020_interpret_value(struct ifd_entry *,char *);
extern void olympus2030_interpret_value(struct ifd_entry *,char *);
extern void olympus2040_interpret_value(struct ifd_entry *,char *);
extern void olympus2050_interpret_value(struct ifd_entry *,char *);

extern void olympus_offset_pe_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,unsigned short,
                                            struct image_summary *,
                                            char *,char*,int,int,int,int);
extern void olympus1_offset_pe_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,unsigned short,
                                            struct image_summary *,
                                            char *,char*,int,int,int,int);
extern void olympus2010_offset_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,
                                            struct image_summary *,
                                            char *,char*,
                                            int,int,int,int);
extern void olympus2020_offset_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,
                                            struct image_summary *,
                                            char *,char*,
                                            int,int,int,int);
extern void olympus2030_offset_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,
                                            struct image_summary *,
                                            char *,char*,
                                            int,int,int,int);
extern void olympus2040_offset_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,
                                            struct image_summary *,
                                            char *,char*,
                                            int,int,int,int);
extern void olympus2050_offset_value(FILE *,unsigned short,struct ifd_entry *,
                                            unsigned long,
                                            struct image_summary *,
                                            char *,char*,
                                            int,int,int,int);
extern void olympus1_interpret_offset_pe_value(FILE *,unsigned short,
                                            struct ifd_entry *,unsigned long,
                                            unsigned short);
extern void olympus2010_interpret_offset_value(FILE *,unsigned short,
                                            struct ifd_entry *,unsigned long);
extern void olympus2020_interpret_offset_value(FILE *,unsigned short,
                                            struct ifd_entry *,unsigned long);
extern void olympus2030_interpret_offset_value(FILE *,unsigned short,
                                            struct ifd_entry *,unsigned long);
extern void olympus2040_interpret_offset_value(FILE *,unsigned short,
                                            struct ifd_entry *,unsigned long);
extern void olympus2050_interpret_offset_value(FILE *,unsigned short,
                                            struct ifd_entry *,unsigned long);

extern char *olympus_private_tagname(unsigned short,unsigned short);
extern char *olympus_private_2010_tagname(unsigned short);
extern char *olympus_private_2020_tagname(unsigned short);
extern char *olympus_private_2030_tagname(unsigned short);
extern char *olympus_private_2040_tagname(unsigned short);
extern char *olympus_private_2050_tagname(unsigned short);

extern int olympus_private_value_is_offset(unsigned short,unsigned short);
extern int olympus2020_value_is_offset(unsigned short);

/* Panasonic */
extern char *maker_panasonic_tagname(unsigned short,int);
extern char *maker_panasonic1_tagname(unsigned short,int);
extern void print_panasonic_makervalue(struct ifd_entry *,int,int,char *);
extern void print_panasonic1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_panasonic_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void panasonic1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void panasonic1_interpret_value(struct ifd_entry *);
extern void panasonic1_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long);
extern int panasonic_model_number(char *,char *);

/* Ricoh */
extern int ricoh_model_number(char *,char *);

/* Sanyo */
extern char *maker_sanyo_tagname(unsigned short,int);
extern char *maker_sanyo1_tagname(unsigned short,int);
extern void print_sanyo_makervalue(struct ifd_entry *,int,int,char *);
extern void print_sanyo1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_sanyo_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void sanyo1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void sanyo1_interpret_value(struct ifd_entry *);
extern void sanyo1_interpret_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long);
extern int sanyo_model_number(char *,char *);

/* Sigma */
extern char *maker_sigma_tagname(unsigned short,int);
extern char *maker_sigma1_tagname(unsigned short,int);
extern void print_sigma_makervalue(struct ifd_entry *,int,int,char *);
extern void print_sigma1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_sigma_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void sigma1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern int sigma_model_number(char *,char *);

/* Sony */
extern char *maker_sony_tagname(unsigned short,int);
/* extern char *maker_sony1_tagname(unsigned short,int);              */
extern void print_sony_makervalue(struct ifd_entry *,int,int,char *);
extern void print_sony1_makervalue(struct ifd_entry *,int,int,char *);
extern void print_sony_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void sony1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern int sony_model_number(char *,char *);

/* Toshiba */
extern char *maker_toshiba_tagname(unsigned short,int);
/* extern char *maker_toshiba1_tagname(unsigned short,int);           */
extern void print_toshiba_makervalue(struct ifd_entry *,int,int,char *);
extern void print_toshiba1_makervalue(struct ifd_entry *,int,int,char *);

extern void print_toshiba_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void toshiba1_offset_makervalue(FILE *,unsigned short,
                                                struct ifd_entry *,
                                                unsigned long,
                                                struct image_summary *,
                                                char *,char *,
                                               int,int,int,int);
extern void toshiba1_interpret_value(struct ifd_entry *);
extern int toshiba_model_number(char *,char *);

/* Traveler */
extern int traveler_model_number(char *,char *);
#endif /* MAKER_EXTERN_INCLUDED */
