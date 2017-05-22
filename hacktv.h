/* hacktv - Analogue video transmitter for the HackRF                    */
/*=======================================================================*/
/* Copyright 2017 Philip Heron <phil@sanslogic.co.uk>                    */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _HACKTV_H
#define _HACKTV_H

#include <stdint.h>
#include "video.h"

/* Return codes */
#define HACKTV_OK             0
#define HACKTV_ERROR         -1
#define HACKTV_OUT_OF_MEMORY -2

/* Integer types */
#define HACKTV_INT16_COMPLEX 0
#define HACKTV_INT16_REAL    1

/* AV source function prototypes */
typedef uint32_t *(*hacktv_av_read_video_t)(void *private);
typedef int16_t *(*hacktv_av_read_audio_t)(void *private, size_t samples);
typedef int (*hacktv_av_close_t)(void *private);

/* RF output function prototypes */
typedef int (*hacktv_rf_write_t)(void *private, int16_t *iq_data, size_t samples);
typedef int (*hacktv_rf_close_t)(void *private);

/* Program state */
typedef struct {
	
	/* Configuration */
	char *output_type;
	char *output;
	char *mode;
	int samplerate;
	float gamma;
	int repeat;
	int verbose;
	unsigned long int frequency;
	int amp;
	int gain;
	
	/* Video encoder state */
	vid_t vid;
	
	/* AV source interface */
	void *av_private;
	hacktv_av_read_video_t av_read_video;
	hacktv_av_read_audio_t av_read_audio;
	hacktv_av_close_t av_close;
	
	/* RF sink interface */
	void *rf_private;
	hacktv_rf_write_t rf_write;
	hacktv_rf_close_t rf_close;
	
} hacktv_t;

#endif
