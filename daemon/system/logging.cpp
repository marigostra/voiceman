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

#include"system.h"
#include"logging.h"
#include"vmstrings.h"

#define FILENAME_SYSLOG "syslog"
#define VOICEMAN_LOGGER "voiceman"

static std::string configLogFileName;
static bool configLogConsole=1;
static int configLogLevel=LOG_WARNING;

/**\brief Generates string representation of current system time*/
static std::string getCurrentTime()
{
  std::string s;
  time_t t;
  time(&t);
  s=ctime(&t);
  std::string ss;
  for(std::string::size_type i = 0;i < s.length();i++)
    if (s[i] != 10 && s[i] != 13)
      ss += s[i];
  return ss;
}

/**\brief Prints log line on the system console
 *
 * This function checks configLogConsole variable and 
 * do nothing if it has zero value.
 *
 * \param [in] level The error level of the message
 * \param [in] line The message text
 */
static void printLogLine(int level, const char* line)
{
  if (!configLogConsole)
    return;
  if (level <= LOG_CRIT)
    {
      std::cerr << "FATAL:" << line << std::endl;
      return;
    }
  if (level <= LOG_ERR)
    {
      std::cerr << "ERROR:" << line << std::endl;
      return;
    }
  if (level <= LOG_WARNING)
    {
      std::cerr << "WARN:" << line << std::endl;
      return;
    }
  std::cout << line << std::endl;
}

/**\brief Saves the log message in log file or sends it to syslog
 *
 * \param [in] level The error level of the message
 * \param [in] line The error text
 */
static void saveLogLine(int level, const char* line)
{
  assert(line != NULL);
  if (configLogFileName.empty())
    return;
  if (configLogFileName == FILENAME_SYSLOG)
    {
      syslog(level, "%s", line);
      return;
    }
  std::ofstream f(configLogFileName.c_str(), std::ios_base::out | std::ios_base::app);
  if (!f)
    return;
  f << getCurrentTime() << ':';
  if (level <= LOG_CRIT)
    f << "FATAL:";  else
  if (level <= LOG_ERR)
    f << "ERROR:"; else
  if (level <= LOG_WARNING)
    f << "WARNING:"; else
  if (level <= LOG_INFO)
    f << "INFO:"; else
    f << "TRACE:";
  f << line << std::endl;
}

/**\brief Processes constructed log message
 *
 * \param [in] level The error level of the message
 * \param [in] line The message text
 */
static void logLine(int level, const char* line)
{
#ifndef VOICEMAN_DEBUG
  if (level > configLogLevel)
    return;
#endif //VOICEMAN_DEBUG
  printLogLine(level, line);
  saveLogLine(level, line);
}

void logMsg(int level, const char* format, ...)
{
  if (!format)
    return;
  va_list args;
  va_start(args, format);
  char buf[4096];
  vsnprintf(buf, sizeof(buf), format, args);
  buf[sizeof(buf)-1]='\0';
  va_end(args);
  std::string str(buf);
  removeNewLineChars(str);
  logLine(level, str.c_str());
}

void initLogging(const std::string& logFileName, bool logConsole, int logLevel)
{
  configLogFileName = logFileName;
  configLogConsole = logConsole;
  configLogLevel = logLevel;
  if (configLogFileName == FILENAME_SYSLOG)
    openlog(VOICEMAN_LOGGER, LOG_PID, LOG_DAEMON);
}
