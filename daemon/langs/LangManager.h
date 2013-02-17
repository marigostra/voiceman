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

#ifndef __VOICEMAN_LANG_MANAGER_H__
#define __VOICEMAN_LANG_MANAGER_H__

#include"core/Lang.h"

/**\brief The class to manage set of available language processing objects
 *
 * This class contains a set of available language objects, fills it with
 * required initial data and provides interface to get language
 * identifier by language name or to get prepared language object by
 * language identifier. Languages must have references by names only in
 * configuration file processing and by identifiers in all other
 * cases. General daemon code may not distinguish languages by itself
 * because in general case their set does not have exact
 * definition. Future version of VoiceMan daemon may have plug-in
 * implementation of supported languages and set of them will be defined in
 * run-time during configuration files processing.
 *
 * \sa Lang EngLang RusLang
 */
class LangManager: public AbstractLangIdResolver
{
public:
  /**\brief The default constructor*/
  LangManager() {}

  /**\brief The destructor*/
  virtual ~LangManager() {}

  /**\brief Fills provided string list with names of supported languages
   *
   * This method lets you know which languages are currently supported by the server.
   *
   * \param [out] stringList The string list to receive the result
   */
  void getSupportedLanguageNames(StringList& stringList) const;

  /**\brief Checks if language is supported
   *
   * This method must be used during configuration file processing to be
   * sure if some language name is valid and corresponding
   * language object is available.
   *
   * \param [in] name Name of a language to check
   *
   * \return Non-zero if specified language is supported
   */
  bool hasLanguage(const std::string& name) const;

  /**\brief Returns language identifier by language name
   *
   * This method can be used to retrieve valid language identifier to
   * distinguish languages in daemon code except configuration files
   * processing.
*
   * \param [in] name The language name to get ID of
   *
   * \return The valid language identifier if language name is supported or LANG_ID_NONE value otherwise
   */
  LangId getLangId(const std::string& name) const;

  /**\brief Returns string with language name by language ID
   *
   * This method returns short string with language name. These names
   * usually used in configuration files processing. The string returned by
   * this method can be empty if provided language ID is not valid.
   *
   * \param [in] langId The ID of the language to get name of
   *
   * \return Name of the language or an empty string if provided language ID is not valid
   */
  std::string getLangName(LangId langId) const;

  /**\brief Fills language objects with initial data
   *
   * Language objects contain various data such as string constants and
   * uses it in different language operations. In most cases this data
   * cannot be included in source code directly because it contains UNICODE
   * characters. This method loads all required structures for all
   * available languages from files stored in the specified directory.
   *
   * \param [in] dataDir The directory with data files
   */
  void load(const std::string& dataDir);

public://AbstractLangIdResolver;
  /**\brief Returns language object by language ID
   *
   * This method checks if provided language identifier is valid and
   * returns a pointer to corresponding language object or NULL otherwise.
   *
   * \param [in] langId The language identifier to get object by
   *
   * \return The pointer to language object or NULL if language ID is unknown
   */
  const Lang* getLangById(LangId langId) const;
}; //class LangManager;

extern LangManager langManager;

#endif //__VOICEMAN_LANG_MANAGER_H__
