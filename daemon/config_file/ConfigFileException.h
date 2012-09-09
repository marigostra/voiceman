/*
	Copyright (c) 2000-2012 Michael Pozhidaev<msp@altlinux.org>
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

#ifndef __VOICEMAN_CONFIG_FILE_EXCEPTION_H__
#define __VOICEMAN_CONFIG_FILE_EXCEPTION_H__

#include"system/logging.h"
#include"VoicemanException.h"

/**\brief The exception object to indicate the configuration file parsing error
 *
 * This class is used to indicate an error during configuration file
 * parsing. It contains information about file name and line number with
 * invalid string. Also it has general error code to produce error
 * description. Usually this class notifies about syntax errors.
 *
 * \sa ConfigFileValueTypeException
 */
class ConfigFileException: public VoicemanException
{
public:
  enum {
    InvalidSectionHeader = 1,
    EmptySectionName = 2,
    ParameterWithoutSection = 3,
    InvalidUnquotedValue = 4,
    InvalidLineBeginning = 5,
    InvalidParameterName = 6,
    InvalidCharAfterEquals = 7,
    UnexpectedLineEnd = 8,
    DublicatedKey = 9
  };

  /**\brief The constructor with error code only specification
   *
   * \param [in] code The error code for new exception object
   */
  ConfigFileException(int code)
    : m_code(code), m_line(0) {}

  /**\brief The constructor with error code, file name and line number specification
   *
   * \param [in] code The error code for new exception object
   * \param [in] fileName Name of a file with error
   * \param [in] line The line number with error (must not be zero)
   */
  ConfigFileException(int code, const std::string& fileName, size_t line)
    : m_code(code), m_fileName(fileName), m_line(line) 
  {
    assert(m_line > 0);
  }

  /**\breif The destructor*/
  virtual ~ConfigFileException() {}

  /**\brief Returns error code
   *
   * Use this method to get error code of current exception object.
   *
   * \return The error code of error occurred
   */
  int getCode() const 
  {
    return m_code;
  }

  /**\brief Returns line number
   *
   * This method has assert() to check current object 
   * was created through the constructor with line number specification.
   *
   * \return The line number with the error
   */
  size_t getLine() const
  {
    assert(m_line > 0);
    return m_line;
  }

  /**\brief Returns name of a file with error
   *
   * Use this method to get file name being processed.
   *
   * \return The name of a file with error occurred
   */
  std::string getFileName() const
  {
    return m_fileName;
  }

  /**\brief Returns the string with error description
   *
   * This method analyzes current error code and returns a string with
   * corresponding error description.
   */
  std::string getDescr() const
  {
    switch(m_code)
      {
      case InvalidSectionHeader:
	return "invalid section header format";
      case EmptySectionName:
	return "missed section name";
      case ParameterWithoutSection:
	return "config file section is not opened";
      case InvalidUnquotedValue:
	return "invalid character in value without quotas";
      case InvalidLineBeginning:
	return "invalid character before parameter name";
      case InvalidParameterName:
	return "invalid character in parameter name";
      case InvalidCharAfterEquals:
	return "invalid character after \'=\' sign";
      case UnexpectedLineEnd:
	return "unexpected line end or line is incomplete";
      case DublicatedKey:
	return "dublicated parameter key in section";
      default:
	assert(0);
      } //switch(m_code);
    return "";
  }

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
    if (m_line == 0)
      return getDescr();
    std::ostringstream s;
    s << m_fileName << "(" << m_line << "):" << getDescr();
    return s.str();
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
    logMsg(level, "config file error:%s", getMessage().c_str());
  }

private:
  const int m_code;
  const std::string m_fileName;
  const size_t m_line;
}; // class ConfigFileException;

#endif //__VOICEMAN_CONFIG_FILE_EXCEPTION_H__
