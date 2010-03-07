/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/*          EXIFPROBE - TIFF/JPEG/EXIF image file probe               */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* Copyright (C) 2002 by Duane H. Hesser. All rights reserved.        */
/*                                                                    */
/* See the file LICENSE.EXIFPROBE for terms of use.                   */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* @(#) $Id: camera_names.h,v 1.2 2005/06/15 21:55:20 alex Exp $" */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef CAMERA_NAMES_INCLUDED
#define CAMERA_NAMES_INCLUDED

#include "maker_datadefs.h"

#include "agfa_datadefs.h"
#include "pentax_datadefs.h"
#include "canon_datadefs.h"
#include "casio_datadefs.h"
#include "epson_datadefs.h"
#include "fujifilm_datadefs.h"
#include "hp_datadefs.h"
#include "konica_datadefs.h"
#include "kodak_datadefs.h"
#include "kyocera_datadefs.h"
#include "leica_datadefs.h"
#include "minolta_datadefs.h"
#include "nikon_datadefs.h"
#include "olympus_datadefs.h"
#include "panasonic_datadefs.h"
#include "ricoh_datadefs.h"
#include "sanyo_datadefs.h"
#include "sigma_datadefs.h"
#include "sony_datadefs.h"
#include "toshiba_datadefs.h"
#include "traveler_datadefs.h"

struct camera_name Camera_make[] = {
    { "Agfa Gevaert",4, MAKER_AGFA, agfa_model_id },
    { "Asahi Optical Co.,Ltd",5, MAKER_ASAHI, pentax_model_id},
    { "Canon",5, MAKER_CANON, canon_model_id},
    { "CASIO",5, MAKER_CASIO, casio_model_id},
    { "Eastman Kodak Company",7, MAKER_KODAK, kodak_model_id },
    { "Fujifilm",4, MAKER_FUJIFILM, fujifilm_model_id},
    { "Hewlett-Packard",7, MAKER_HP, hp_model_id },
    { "Konica Minolta Camera",14, MAKER_MINOLTA, minolta_model_id},
    { "Konica Corporation",6, MAKER_KONICA, konica_model_id},
    { "Kyocera",7, MAKER_KYOCERA, kyocera_model_id},
    { "Leica",5, MAKER_LEICA, leica_model_id},
    { "Minolta",7, MAKER_MINOLTA, minolta_model_id},
    { "Panasonic",9, MAKER_PANASONIC, panasonic_model_id},
    { "Pentax Corporation",6, MAKER_PENTAX, pentax_model_id},
    { "Nikon Corporation",5, MAKER_NIKON, nikon_model_id},
    { "Olympus Optical Co.,Ltd",7, MAKER_OLYMPUS, olympus_model_id},
    { "Ricoh",5, MAKER_RICOH, ricoh_model_id },
    { "Sanyo Electric Co.,Ltd.",5, MAKER_SANYO, sanyo_model_id},
    { "Seiko Epson Corp.",5, MAKER_EPSON, epson_model_id},
    { "Sigma",5, MAKER_SIGMA, sigma_model_id},
    { "Sony",4, MAKER_SONY, sony_model_id},
    { "Toshiba",8, MAKER_TOSHIBA, toshiba_model_id },
    { "Traveler Optical Co,Ltd",8, MAKER_TRAVELER, traveler_model_id},
    { 0, 0, 0,0  }
};

#endif  /* CAMERA_NAMES_INCLUDED */
