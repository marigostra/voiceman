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

#ifndef __VOICEMAN_CONFIG_FILE_VALUE_TYPE_EXCEPTION_H__
#define __VOICEMAN_CONFIG_FILE_VALUE_TYPE_EXCEPTION_H__

#include"system/logging.h"
#include"VoicemanException.h"

/**\brief The exception class to notify about parameter value type mismatch
 *
 * This class notifies user about all errors in value type checking
 * procedure. All parameters in configuration file must be checked for
 * type matching automatically to simplify parsed values processing.
 *
 * \sa ConfigFile ConfigFileException
 */
class ConfigFileValueTypeException: public VoicemanException
{
public:
  /**\brief The constructor
   *
   * \param [in] descr The error description
   */
  ConfigFileValueTypeException(const std::string& descr)
    : m_descr(descr) {}

  /**\brief The destructor*/
  virtual ~ConfigFileValueTypeException() {}
  /**\brief Returns exception description
   *
   * This method generates single line description of the error
   * object. returned string can be printed to console or to log as error
   * report.
   *
   * \return The single-line error description
   */
  std::string getMessage() const
  {
    return m_descr;
  }

  /**\brief Makes report to logging system
   *
   * This function writes to log a report about error object. It may be not
   * exactly the same value as produced by getMessage() method. The level
   * of message must be specified, because the same error can have
   * different levels in different situations. For example, configuration
   * file error is critical error during server startup process due to to
   * requirement have first configuration information, but be just a usual
   * error in configuration reloading, because in this case server can
   * successfully use its previous configuration data.
   *
   * \param level The level of error log report
   */
  void makeLogReport(int level) const
  {
    logMsg(level, "config file value type mismatch:%s", getMessage().c_str());
  }

private:
  const std::string m_descr;
}; //class ConfigFileValueTypeException;

#endif //__VOICEMAN_CONFIG_FILE_VALUE_TYPE_EXCEPTION_H__
