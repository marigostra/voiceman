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

#ifndef __VOICEMAN_ENG_LANG_H__
#define __VOICEMAN_ENG_LANG_H__

#include"core/Lang.h"

/**\brief The class for English language operations
 *
 * This class is used to provide all language-specific information
 * for English language. VoiceMan server has associated
 * language object for all portions of text.Usually these objects are
 * used to get set of characters, translate digits into words
 * representation, perform case insensitive characters comparing and for
 * some others.
 *
 * \sa LangManager RusLang
 */
class EngLang: public Lang
{
public:
  /**\brief The destructor*/
  virtual ~EngLang() {}

  /**\brief Loads table of phonetic pronounciation strings
   *
   * This class has internal table to select characters for speak
   * phonetically.  This method loads it from external file.
   *
   * \param [in] fileName The name of file to read table from
   */
  void loadCaps(const std::string& fileName);

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

  /**\brief Returns all characters of English alphabet (in both cases)
   *
   * This method allows you to enumerate all characters
   * used in English language.
   *
   * \return All characters of English language
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
   * rules of English language. Single digits mode replaces just each digit
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
   * the middle of the word this method will insert space before it. It is
   * useful for Hungarian notation.
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
  struct CapItem
  {
    CapItem(): before(0), after(0) {}
    CapItem(const std::wstring& s, bool b, bool a)
      : str(s), before(b), after(a) {}

    std::wstring str;
    bool before, after;
  }; //struct CapItem;

  typedef std::vector<CapItem> CapItemVector;
  typedef std::list<CapItem> CapItemList;

private:
  std::wstring processHundred(const std::wstring& inStr, const wchar_t* const items[]) const;
  std::wstring digitsToWords(const std::wstring& inStr) const;
  bool checkCapList(const std::wstring& str, std::wstring::size_type pos, std::wstring& result) const;
  void processCapList(const std::wstring& str, BoolVector& marks) const;

private:
  CapItemList m_capItems;
};//class EngLang;

#endif //__VOICEMAN_ENG_H__
