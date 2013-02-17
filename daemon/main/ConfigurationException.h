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

#ifndef __VOICEMAN_CONFIGURATION_EXCEPTION_H__
#define __VOICEMAN_CONFIGURATION_EXCEPTION_H__

/**\brief The exception class to handle configuration errors
 *
 * This class is used to notify about an error during configuration data
 * processing and store all necessary information. This class must not be
 * confused with configuration file parsing errors. This class is used
 * only for semantics errors.
 *
 * \sa ConfigFileException ConfigFileValueTypeException
 */
class ConfigurationException: public VoicemanException
{
public:
  /**\brief The constructor
   *
   * \param [in] comment The single-line error description
   */
  ConfigurationException(const std::string& comment)
    : m_comment(comment) {}

  /**\brief The destructor*/
  virtual ~ConfigurationException() {}

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
    return m_comment;
  }

  /**\brief Makes report to logging system
   *
   * This function writes to log a report about error object. It may be not
   * exactly the same value as produced by getMessage() method. The level
   * of message must be specified, because the same error can have
   * different levels in different situations. 
   *
   * \param level The level of error log report
   */
  void makeLogReport(int level) const
  {
    logMsg(level, "configuration problem:%s", getMessage().c_str());
  }

private:
  const std::string m_comment;
}; //class ConfigurationException;

#endif //__VOICEMAN_CONFIGURATION_EXCEPTION_H__
