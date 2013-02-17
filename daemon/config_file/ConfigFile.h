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

#ifndef __VOICEMAN_CONFIG_FILE_H__
#define __VOICEMAN_CONFIG_FILE_H__

#include"ConfigFileException.h"
#include"ConfigFileValueTypeException.h"
#include"ConfigFileSection.h"

#define VOICEMAN_BEGIN_PARAM_TABLE(cl) int cl::params(const std::string& section, const std::string& param) const {
#define VOICEMAN_RELAX_SECTION(s) if (section==(s)) return AnyValue
#define VOICEMAN_DECLARE_PARAM(s,p) if (section==(s) && param==(p)) return AnyValue
#define VOICEMAN_DECLARE_BOOLEAN_PARAM(s,p) if (section==(s) && param==(p)) return BooleanValue
#define VOICEMAN_DECLARE_STRING_PARAM(s,p) if (section==(s) && param==(p)) return StringValue
#define VOICEMAN_DECLARE_INT_PARAM(s,p) if (section==(s) && param==(p)) return IntValue
#define VOICEMAN_DECLARE_UINT_PARAM(s,p) if (section==(s) && param==(p)) return UintValue
#define VOICEMAN_DECLARE_DOUBLE_PARAM(s,p) if (section==(s) && param==(p)) return DoubleValue
#define VOICEMAN_END_PARAM_TABLE return InvalidValue;}

/**\brief The configuration file parser
 *
 * This is the main class to parse configuration file. It reads all data
 * from file, constructs the set of ConfigFileSection objects and fills
 * them with read data. User can analyze prepared information and perform
 * required handling. This class can also check data types by the data
 * types table, defined in child class. None of the data semantics is
 * processed by this class.
 *
 * \sa ConfigFileSection ConfigFileException ConfigFileValueTypeException
 */
class ConfigFile
{
public:
  /**\brief The default constructor*/
  ConfigFile() {}

  /**\brief The destructor*/
  virtual ~ConfigFile() {}

  /**\brief Loads configuration file content
   *
   * This method reads configuration file and saves processed data in the
   * internal structures inside of this class. Any data can be retrieved
   * later.  On any error it throws ConfigFileException.  This method does
   * not perform any value type checking. You should explicitly call
   * checkParams() method to do it. 
   *
   * \param [in] fileName The name of a file to read data from
   */
  void load(const std::string& fileName);

  /**\brief Checks values type matching
   *
   * This method looks through read values and checks its types. If error
   * is found the ConfigFileValueTypeException is thrown. Checking is
   * performed by the data provided by params() method. You should override
   * this method in your child classe.  There are predefined macroses to
   * define data types table.
   */
  void checkParams() const;

  /**\brief Returns the number of read sections
   *
   * Use this method to get number of parsed sections.
   *
   * \return Number of parsed sections
   */
  ConfigFileSectionVector::size_type getSectionCount() const;

  /**\brief Returns the reference to the section object by the section index
   *
   * Use this method to get desired section object.
   *
   * \param [in] index The index of a section to return
   *
   * \return The reference to desired section object
   */
  const ConfigFileSection& getSection(ConfigFileSectionVector::size_type index) const;

  /**\brief Returns the reference to the section object by the section name
   *
   *
   * This method looks through stored sections vector and finds
   * U corresponding by section name. You must be strongly sure the required
   * section exists. This method does not have any way to notify about an
   * error.
   *
   * \param [in] name The name of a section to return
   *
   * \return The reference to desired section object
   */
  const ConfigFileSection& findSection(const std::string& name) const;

  /**\brief Checks if the specified section is accessible
   *
   * Use this method to be sure the desired section exists.
   *
   * \param [in] name The name of a section to check
   *
   * \return Non-zero if specified section exists
   */
  bool hasSection(const std::string& name) const;

protected:
  enum {AnyValue = -1, InvalidValue = 0, BooleanValue = 1, StringValue = 2, IntValue = 3, UintValue = 4, DoubleValue = 5};
  virtual int params(const std::string& section, const std::string& param) const = 0;

private:
  int process(const std::string& line, int& code, std::string& str1, std::string& str2) const;
  void checkParamsInSection(const ConfigFileSection& section) const;

private:
  ConfigFileSectionVector m_sections;
}; //class ConfigFile;

#endif // __VOICEMAN_CONFIG_FILE_H__
