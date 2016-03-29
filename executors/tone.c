/*
	Copyright (c) 2000-2013 Michael Pozhidaev<msp@altlinux.org>
   This file is part of the VoiceMan speech service.

   VoiceMan speech service is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   VoiceMan speech service is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#include<assert.h>
#include<stdlib.h>
#include<math.h>
#include<strings.h>
#include<string.h>
#include<ao/ao.h>

#define VOICEMAN_LIBAO_DRIVER "VOICEMAN_LIBAO_DRIVER"

static char* libAoDriver = NULL;

void toneInit()
{
  libAoDriver = getenv(VOICEMAN_LIBAO_DRIVER);
  if (libAoDriver != NULL && strlen(libAoDriver) < 1)
    libAoDriver = NULL;
  ao_initialize();
}

void toneClose()
{
  ao_shutdown();
}

void playTone(size_t fr, size_t lengthMs)
{
  ao_device *device = NULL;
  ao_sample_format format;
  int driver;
  float freq=fr;
  size_t bufSize = 0;
  char* buffer = NULL;
  size_t samplesToPlay = 0;
  size_t i;
  assert(fr >= 0 && lengthMs >= 10);
  bzero(&format, sizeof(ao_sample_format));
  driver = libAoDriver != NULL?ao_driver_id(libAoDriver):ao_default_driver_id();
  format.bits = 16;
  format.channels = 2;
  format.rate = 44100;
  format.byte_format = AO_FMT_LITTLE;
  samplesToPlay = format.rate*lengthMs/1000;
  device = ao_open_live(driver, &format, NULL);
  if (device == NULL) 
    {
      ao_shutdown();
      return;
    }
  bufSize = format.bits/8 * format.channels * samplesToPlay;
  buffer = (char*)calloc(bufSize, sizeof(char));
  if (buffer == NULL)
    {
      ao_close(device);
      ao_shutdown();
      return;
    }
  for (i = 0; i < samplesToPlay; i++) 
    {
      int sample = (int)(0.75 * 32768.0 *
			 sin(2 * M_PI * freq * ((float) i/format.rate)));
      buffer[4*i] = buffer[4*i+2] = sample & 0xff;
      buffer[4*i+1] = buffer[4*i+3] = (sample >> 8) & 0xff;
    }
  ao_play(device, buffer, bufSize);
  free(buffer);
  ao_close(device);
}
