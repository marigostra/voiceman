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

#ifndef __VOICEMAN_CONFIG_FILE_SECTION_H__
#define __VOICEMAN_CONFIG_FILE_SECTION_H__

/**\brief The configuration file section information
 *
 * This class stores everything about one configuration file section. It
 * consists of section name and the map from parameter key to parameter
 * value. 
 *
 * \sa ConfigFile
 */
class ConfigFileSection: public std::map<std::string, std::string>
{
public:
  /**\brief The default constructor*/
  ConfigFileSection() {}

  /**\brief The constructor with configuration file section name specification
   *
   * \param [in] name The name of new configuration file section
   */
  ConfigFileSection(const std::string& name)
    : m_name(name) {}

  /**\brief The destructor*/
  virtual ~ConfigFileSection() {}

  /**\brief Returns configuration file section name
   *
   * Use this method to get name of this section.
   *
   * \return The string with section name
   */
  std::string getName() const 
  {
    return m_name;
  }

  /**\brief Sets new configuration file section name
   *
   * Use this method to set new configuration file name.
   *
   * \param [in] name The name to set
   */
  void setName(const std::string& name)
  {
    m_name=name;
  }

  /**\brief Checks if the section has specified parameter
   *
   * This method allows you be sure the specified parameter is present 
   * in this section and its value is accessible.
   *
   * \param [in] name The name of parameter to check
   *
   * \return Non-zero if specified parameter is present in this section
   */
  bool has(const std::string& name) const 
  {
    return find(name) != end();
  }

  /**\brief Adds new parameter and its value to the section
   *
   * Use this method to add new value to the section map.
   *
   * \param [in] param The name of the parameter to add
   * \param [in] value The value of the parameter to add
   */
  void add(const std::string& param, const std::string& value)
  {
    insert(value_type(param, value));
  }

  /**\brief Returns parameter value
   *
   * Use this method to get parameter value, but be careful, this method
   * does not have any special value to notify about an error. If there is
   * no such value it returns an empty string. But it can happen either in
   * case of value exists and is empty or in case of there is no such value
   * at all.
   *
   * \param [in] name The name of an existing parameter to get value of
   *
   * \return The string with requested value
   */
  std::string operator [](const std::string& name) const
  {
    const_iterator it=find(name);
    assert(it!=end());
    if (it == end())
      return "";
    return it->second;
  }

private:
  std::string m_name;
}; // class ConfigFileSection;

typedef std::vector<ConfigFileSection> ConfigFileSectionVector;
typedef std::list<ConfigFileSection> ConfigFileSectionList;

#endif //__VOICEMAN_CONFIG_FILE_SECTION_H__
