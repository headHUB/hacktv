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

#include <math.h>
#include <stdlib.h>
#include "hacktv.h"

/* AV test pattern source */
typedef struct {
	int width;
	int height;
	uint32_t *video;
	int16_t *audio;
	size_t audio_samples;
} av_test_t;

static uint32_t *_av_test_read_video(void *private)
{
	av_test_t *av = private;
	return(av->video);
}

static int16_t *_av_test_read_audio(void *private, size_t *samples)
{
	av_test_t *av = private;
	*samples = av->audio_samples;
	return(av->audio);
}

static int _av_test_close(void *private)
{
	av_test_t *av = private;
	if(av->video) free(av->video);
	if(av->audio) free(av->audio);
	free(av);
	return(HACKTV_OK);
}

int av_test_open(vid_t *s)
{
	uint32_t const bars[8] = {
		0x000000,
		0x0000FF,
		0xFF0000,
		0xFF00FF,
		0x00FF00,
		0x00FFFF,
		0xFFFF00,
		0xFFFFFF,
	};
	av_test_t *av;
	int c, x, y;
	double d;
	int16_t l;
	
	av = calloc(1, sizeof(av_test_t));
	if(!av)
	{
		return(HACKTV_OUT_OF_MEMORY);
	}
	
	/* Generate a basic test pattern */
	av->width = s->active_width;
	av->height = s->conf.active_lines;
	av->video = malloc(vid_get_framebuffer_length(s));
	if(!av->video)
	{
		free(av);
		return(HACKTV_OUT_OF_MEMORY);
	}
	
	for(y = 0; y < s->conf.active_lines; y++)
	{
		for(x = 0; x < s->active_width; x++)
		{
			if(y < 400)
			{
				/* 100% colour bars */
				c = 7 - x * 8 / s->active_width;
				c = bars[c];
			}
			else if(y < 420)
			{
				/* 100% red */
				c = 0xFF0000;
			}
			else if(y < 440)
			{
				/* Gradient black to white */
				c = x * 0xFF / (s->active_width - 1);
				c = c << 16 | c << 8 | c;
			}
			else
			{
				/* 8 level grey bars */
				c = x * 0xFF / (s->active_width - 1);
				c &= 0xE0;
				c = c | (c >> 3) | (c >> 6);
				c = c << 16 | c << 8 | c;
			}
			
			av->video[y * s->active_width + x] = c;
		}
	}
	
	/* Generate the 1khz test tones (BBC 1 style) */
	d = 1000.0 * 2 * M_PI / HACKTV_AUDIO_SAMPLE_RATE;
	y = HACKTV_AUDIO_SAMPLE_RATE * 64 / 100; /* 640ms */
	av->audio_samples = y * 10; /* 6.4 seconds */
	av->audio = malloc(av->audio_samples * 2 * sizeof(int16_t));
	if(!av->audio)
	{
		free(av);
		return(HACKTV_OUT_OF_MEMORY);
	}
	
	for(x = 0; x < av->audio_samples; x++)
	{
		l = sin(x * d) * INT16_MAX * 0.1;
		
		if(x < y)
		{
			/* 0 - 640ms, interrupt left channel */
			av->audio[x * 2 + 0] = 0;
			av->audio[x * 2 + 1] = l;
		}
		else if(x >= y * 2 && x < y * 3)
		{
			/* 1280ms - 1920ms, interrupt right channel */
			av->audio[x * 2 + 0] = l;
			av->audio[x * 2 + 1] = 0;
		}
		else if(x >= y * 4 && x < y * 5)
		{
			/* 2560ms - 3200ms, interrupt right channel again */
			av->audio[x * 2 + 0] = l;
			av->audio[x * 2 + 1] = 0;
		}
		else
		{
			/* Use both channels for all other times */
			av->audio[x * 2 + 0] = l; /* Left */
			av->audio[x * 2 + 1] = l; /* Right */
		}
	}
	
	/* Register the callback functions */
	s->av_private = av;
	s->av_read_video = _av_test_read_video;
	s->av_read_audio = _av_test_read_audio;
	s->av_close = _av_test_close;
	
	return(HACKTV_OK);
}

