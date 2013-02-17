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

#ifndef __VOICEMAN_RUS_LANG_H__
#define __VOICEMAN_RUS_LANG_H__

#include"core/Lang.h"

/**\brief The class for Russian language operations
 *
 * This class is used to provide all language-specific information
 * for Russian language. VoiceMan server has associated
 * language object for all portions of text.Usually these objects are
 * used to get set of characters, translate digits into words
 * representation, perform case insensitive characters comparing and for
 * some others.
 *
 * \sa LangManager EngLang
 */
class RusLang: public Lang
{
public:
  /**\brief The default constructor*/
  RusLang() {}

  /**\brief The destructor*/
  virtual ~RusLang() {}
  /**\brief Loads string constants from external file
   *
   * This method reads external file and takes from it all string constants
   * necessary for Russian language operations. These constants stored not
   * in source code because it is impossible to use strings not in US-ASCII
   * character set.
   *
   * \param [in] fileName The name of the file to take constants from
   */
  void load(const std::string& fileName);

  /**\brief Returns type of specified character
   *
   * This method allows you to get class of specified character. It returns
   * one of the following values: Lang::UpCase, Lang::LowCase or
   * Lang::Other.
   *
   * \param [in] c The character to get type of
   *
   * \return Type of specified character (Lang::UpCase, Lang::LowCase or Lang::Other)
   */
  Lang::CharType getCharType(wchar_t c) const;

  /**\brief Returns all characters of Russian alphabet (in both cases)
   *
   * This method allows you to enumerate all characters
   * used in Russian language.
   *
   * \return All characters of Russian language
   */
  std::wstring getAllChars() const;

  /**\brief Checks if specified characters are represent same letter
   *
   * Almost in every language each letter can be represented in different
   * cases and different characters are used. This method
   * checks, if specified characters are represent the same letter.
   *
   * \param [in] c1 The first character to compare
   * \param [in] c2 The second character to compare
   *
   * \return Non-zero if provided characters are the same letters or zero otherwise
   */
  bool equalChars(wchar_t c1, wchar_t c2) const;

  /**\brief Returns upper case of the specified character
   *
   * This method adjusts provided character to upper case.
   *
   * \param [in] ch The character to return upper case of
   *
   * \return The upper case of provided character
   */
  wchar_t toUpper(wchar_t ch) const;

  /**\brief Returns lower case of the specified character
   *
   * This method adjusts provided character to lower case.
   *
   * \param [in] ch The character to return lower case of
   *
   * \return The lower case of provided character
   */
  wchar_t toLower(wchar_t ch) const;

  /**\brief Returns upper case of the specified string
   *
   * This method adjusts characters of provided string to upper case.
   *
   * \param [in] str The string to return upper case of
   *
   * \return The upper case of provided string
   */
  std::wstring toUpper(const std::wstring& str) const;

  /**\brief Returns lower case of the specified string
   *
   * This method adjusts characters of provided string to lower case.
   *
   * \param [in] str The string to return lower case of
   *
   * \return The lower case of provided string
   */
  std::wstring toLower(const std::wstring& str) const;

  /**\brief Replaces numbers with their word representation
   *
   * This method replaces all digits by their words representation with the
   * rules of Russian language. Single digits mode replaces just each digit
   * with corresponding word. 
   *
   * \param [in/out] str The string to process
   * \param [in] singleDigits Process each digit as separate number
   */
  void expandNumbers(std::wstring& str, bool singleDigits) const;

  /**\brief Inserts additional spaces to better words processing
   *
   * This method inserts additional spaces to split words to avoid using
   * capital letters in the middle of the words. If capital letter are used in
   * the middle of the word this method will insert space before it.
   *
   * \param [in] text The string to process
   *
   * \return The string with splitted words
   */
  std::wstring separate(const std::wstring& text) const;

  /**\brief Marks letters to speak separately (usual acronims)
   *
   * This method marks characters to speak phonetically . Usually some
   * heuristic or predefined table are used.
   *
   * \param [in] text The string to process
   * \param [out] marks The boolean vector to save marks
   */
  void markCapitals(const std::wstring& text, BoolVector& marks) const;

private:
  std::wstring processHundred(const std::wstring& inStr, const WStringVector& items, bool female) const;
  std::wstring digitsToWords(const std::wstring& inStr) const;

private:
  typedef std::map<wchar_t, wchar_t> WCharToWCharMap;

  std::wstring m_chars, m_zero, m_vowels;
  WStringVector m_mlrds, m_mlns, m_thnds, m_hundreds, m_decimals, m_tens, m_ones, m_onesF;
  WCharToWCharMap m_toUpper, m_toLower;
};//class RusLang;

#endif //__VOICEMAN_RUS_LANG_H__
