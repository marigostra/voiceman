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

#ifndef __VOICEMAN_LOGGING_H__
#define __VOICEMAN_LOGGING_H__

/**\brief Prepares log output
 *
 * This function makes all operations required to prepare logging
 * output. It can be called only once. The logging level can be specified
 * in syslog constants as they used in Linux applications. The name of a
 * file to save log messages in can be "syslog" and it means to translate
 * all messages to usual syslog mechanism. Also it can be empty and in
 * this case messages will not be saved at all.
 * 
 * \param [in] logFileName The name of a file to save logging information in
 * \param [in] logConsole Print logging messages on system console
 * \param [in] logLevel The level to filter logging information
 */
void initLogging(const std::string& logFileName, bool logConsole, int logLevel); 

/**\brief Processes log message
 *
 * This is the main function to make one log message. The message text
 * can be specified as printf() formatted string with consequent
 * parameters. Log level value can be provided as syslog level constants.
 *
 * \param [in] level The error level for this message
 * \param [in] format The message text
 */
void logMsg(int level, const char* format,... );

#endif //__VOICEMAN_LOGGING_H__
