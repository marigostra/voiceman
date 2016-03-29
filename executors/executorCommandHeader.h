/*
	Copyright (c) 2000-2016 Michael Pozhidaev<michael.pozhidaev@gmail.com>
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

#ifndef __VOICEMAN_EXECUTOR_COMMAND_HEADER_H__
#define __VOICEMAN_EXECUTOR_COMMAND_HEADER_H__

/*
 * This header declares the CommandHeader structure used for executor
 * commands transmission. This structure has command code and three
 * unsigned integer parameters. Parameters purpose depends on command
 * code and is described below for each command separately.
 *
 * COMMAND_SAY: The command to initiate text block speak or enqueue this
 * text block if executor is busy. Three parameters contain string length
 * of synthesizer command, player command and text including trailing'\0'
 * character. Strings come just after the header in order as they were
 * mentioned above. Terminating '\0' character must be transmitted
 * explicitly for all strings. Text must be in the same character set as
 * synthesizer expects it, UTF-8 is preferable.
 *
 * COMMAND_STOP: Stop any playback and erase queue. Three parameters may
 * have any value.
 *
 * COMMAND_TONE: The command to produce signal of fixed frequency. The
 * executor must store this command in queue if there is speech playback
 * now, but may deferre following commands processing during until tone
 * signal playback is finished. The first parameter is the desired signal
 * frequency and the second one is the signal duration in milliseconds.
 *
 * COMMAND_SET_QUEUE_LIMIT: The command to set maximum queue size. Only
 * first parameter is used. It specifies number of possible items in
 * playback queue. Special value 0 can be used to disable queue size
 * limit checking.
 */

#define COMMAND_SAY 0
#define COMMAND_STOP 1
#define COMMAND_TONE 2
#define COMMAND_SET_QUEUE_LIMIT 3

typedef struct {
  int code;
  size_t param1;
  size_t param2;
  size_t param3;
} CommandHeader;

#endif
