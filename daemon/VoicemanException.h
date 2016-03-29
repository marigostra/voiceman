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

#ifndef __VOICEMAN_EXCEPTION_H__
#define __VOICEMAN_EXCEPTION_H__

/**\brief The basic class for all VoiceMan exceptions
 *
 * This is the ancestor of all exception classes used for error handling
 * during VoiceMan server functioning. Catching errors with this root
 * class allows get the code for processing all types of possible errors.
 * 
 * \sa SystemException ConfigFileException ConfigFileValueTypeException ConfigurationException
 */
class VoicemanException
{
public:
  /**\brief The default constructor*/
  VoicemanException() {}

  /**\brief The destructor*/
  virtual ~VoicemanException() {}

  /**\brief Returns exception description
   *
   * This method generates single line description of the error
   * object. returned string can be printed to console or to log as error
   * report.
   *
   * \return The single-line error description
   */
  virtual std::string getMessage() const = 0;

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
  virtual void makeLogReport(int level) const = 0;
}; //class VoicemanException;

#endif //__VOICEMAN_EXCEPTION_H__
