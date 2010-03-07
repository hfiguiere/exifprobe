/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: maker.h,v 1.18 2005/07/24 21:17:17 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* General definitions for makernotes                                 */

#ifndef MAKER_INCLUDED
#define MAKER_INCLUDED

#define MAX_IDLEN   24      /* max makernote id length to look for    */
#define MAKERTAGWIDTH    25

#define UNKNOWN_SCHEME                  0x0
#define PLAIN_IFD_SCHEME                0x1
#define HAS_ID_SCHEME                   0x2
#define HAS_ID_PLUS_TIFFHDR_SCHEME      0x4

/* offset values relative to...                                       */
#define START_OF_TIFFHEADER   0
#define START_OF_NOTE         1
#define START_OF_ENTRY        2

/* Magic numbers for maker                                            */
/* ###%%% these should go into maker-specific includes                */

#define NO_MAKE             0
#define NO_MODEL            0
#define MAKER_AGFA          1
#define MAKER_ASAHI         2
#define MAKER_CANON         3
#define MAKER_CASIO         4
#define MAKER_KODAK         5
#define MAKER_FUJIFILM      6
#define MAKER_HP            7
#define MAKER_MINOLTA       8
#define MAKER_KONICA        9
#define MAKER_KYOCERA       10
#define MAKER_LEICA         11
#define MAKER_NIKON         12
#define MAKER_OLYMPUS       13
#define MAKER_PANASONIC     14
#define MAKER_PENTAX        15
#define MAKER_RICOH         16
#define MAKER_SANYO         17
#define MAKER_EPSON         18
#define MAKER_SIGMA         19
#define MAKER_SONY          20
#define MAKER_TOSHIBA       21
#define MAKER_TRAVELER      22

/* Agfa model magic                                                   */
#define AGFA_SR841          1

/* Canon model magic                                                  */
#define CANON_IXUS              1
#define CANON_EOS1D             2
#define CANON_EOS1DMARKII       3
#define CANON_EOS1DS            4
#define CANON_EOS1DSMARKII      5
#define CANON_EOS10D            6
#define CANON_EOS20D            7
#define CANON_EOSD30            8
#define CANON_EOSD60            9
#define CANON_EOS350            10
#define CANON_EOSDIGITALREBEL   11
#define CANON_POWERSHOT_A1      12
#define CANON_POWERSHOT_A400    13
#define CANON_POWERSHOT_A5      14
#define CANON_POWERSHOT_G2      15
#define CANON_POWERSHOT_PRO1    16
#define CANON_POWERSHOT_S70     17

/* Casio model magic                                                  */
#define CASIO_QV2000UX      1
#define CASIO_QV3000EX      2
#define CASIO_QV4000        3
#define CASIO_QV8000SX      4
#define CASIO_QVR51         5
#define CASIO_EX_P505       6
#define CASIO_EX_P600       7
#define CASIO_EX_P700       8
#define CASIO_EX_S100       9
#define CASIO_EX_Z55        10
#define CASIO_GV_20         11

/* Eastman Kodak                                                      */
#define KODAK_DC200         1
#define KODAK_DC210         2
#define KODAK_DCS720X       3
#define KODAK_DCS760C       4
#define KODAK_DK120         5
#define KODAK_DC240         6
#define KODAK_DC4800        7
#define KODAK_DC25          8
#define KODAK_LS443         9
#define KODAK_DC50          10

/* Epson model magic                                                  */
#define EPSON_850Z          1
#define EPSON_3000Z         2
#define EPSON_3100Z         3
#define EPSON_L500V         4

/* Fujifilm model magic                                               */
#define FUJI_DX10           1   /* No makernote                       */
#define FUJI_FP_A204        2
#define FUJI_40i            3
#define FUJI_4900ZOOM       4
#define FUJI_E550           5
#define FUJI_F601ZOOM       6
#define FUJI_S1PRO          7   /* No makernote                       */
#define FUJI_S2PRO          8
#define FUJI_S20PRO         9
#define FUJI_S5000          10
#define FUJI_MX1700ZOOM     11  /* No makernote                       */
#define FUJI_SP2000         12


/* HP model magic                                                     */
#define HP_C912             1
#define HP_R707             2

/* Konica model magic                                                */
#define KONICA_QM100        1
#define KONICA_KD400Z       2

/* Kyocera model magic                                                */
#define KYOCERA_FCS3        1
#define KYOCERA_FINECAMS5   2

/* Leica model magic                                                  */
#define LEICA_DIGILUX2      1
#define LEICA_DIGILUX43     2   /* Identical to FUJI                  */
#define LEICA_R9_DB_DMR     3   /* Digital back                       */

/* Minolta model magic                                                */
#define MINOLTA_DIMAGE5     1
#define MINOLTA_DIMAGE7     2
#define MINOLTA_DIMAGE7Hi   3
#define MINOLTA_DIMAGE7i    4
#define MINOLTA_DIMAGEA1    5
#define MINOLTA_DIMAGEA2    6
#define MINOLTA_DIMAGEA200  7
#define MINOLTA_DIMAGEEX    8
#define MINOLTA_DIMAGEF100  9
#define MINOLTA_DIMAGEF200  10
#define MINOLTA_DIMAGEF300  11
#define MINOLTA_DIMAGES404  12
#define MINOLTA_DIMAGES414  13
#define MINOLTA_DIMAGEX     14
#define MINOLTA_DIMAGEX20   15
#define MINOLTA_DYNAX7D     16
#define MINOLTA_MAXXUM7D    17

/* Nikon Model magic                                                  */
#define NIKON_700           1
#define NIKON_775           2
#define NIKON_800           3
#define NIKON_885           4
#define NIKON_900           5
#define NIKON_950           6
#define NIKON_990           7
#define NIKON_995           8
#define NIKON_3700          9
#define NIKON_5000          10
#define NIKON_5200          11
#define NIKON_5700          12
#define NIKON_8700          13
#define NIKON_8800          14
#define NIKON_D1            15
#define NIKON_D1X           16
#define NIKON_D70           17
#define NIKON_D100          18
#define NIKON_D2H           19
#define NIKON_D2X           20
#define NIKON_SQ            21

/* Olympus model magic                                                */
#define OLYMPUS_700UZ       1
#define OLYMPUS_C70ZC7000Z  2
#define OLYMPUS_960Z        3
#define OLYMPUS_2040Z       4
#define OLYMPUS_C2500L      5
#define OLYMPUS_3030Z       6
#define OLYMPUS_3040Z       7
#define OLYMPUS_C8080WZ     8
#define OLYMPUS_E_1         9
#define OLYMPUS_E_300       10
#define OLYMPUS_u40D        11

/* Asahi/Pentax model magic                                           */
#define ASAHI_PENTAX330     1
#define ASAHI_PENTAX330RS   2
#define ASAHI_PENTAX430     3
#define ASAHI_PENTAX430RS   4
#define PENTAX_OPTIO550     5
#define PENTAX_OPTIO750Z    6
#define PENTAX_OPTIOS       7
#define PENTAX_OPTIOS5i     8
#define PENTAX_STARIST_D    9
#define PENTAX_STARIST_DS   10

/* Panasonic model magic                                              */
#define PANASONIC_DMCFX7    1
#define PANASONIC_DMCFZ1    2
#define PANASONIC_DMCFZ2    3
#define PANASONIC_DMCFZ3    4
#define PANASONIC_DMCFZ10   5
#define PANASONIC_DMCFZ15   6
#define PANASONIC_DMCFZ20   7
#define PANASONIC_DMCLC5    8
#define PANASONIC_DMCLC33   9
#define PANASONIC_DMCLC40   10
#define PANASONIC_DMCLC43   11

/* Ricoh model magic                                                  */
#define RICOH_RDC5300       1
#define RICOH_RDC6000       2

/* Sanyo model magic                                                  */
#define SANYO_SR6           1
#define SANYO_SX113         2
#define SANYO_SX215         3

/* Sigma model magic                                                  */
#define SIGMA_SD9           1
#define SIGMA_SD10          2

/* Sony model magic                                                   */
#define SONY_CYBERSHOT      1
#define SONY_DIGITALMAVICA  2
#define SONY_DSCD700        3
#define SONY_DSCV3          4
#define SONY_MAVICA         5

/* Toshiba model magic                                                */
#define TOSHIBA_PDR_3300    1

/* Traveler model magic                                               */
#define TRAVELER_SX330Z     1

#endif
