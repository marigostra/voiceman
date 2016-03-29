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

#ifndef __VOICEMAN_LANG_H__
#define __VOICEMAN_LANG_H__

/**\brief The abstract interface for all language objects
 *
 * Language objects are used to provide all language-specific information
 * and unify all language operations. VoiceMan server has associated
 * language object for all portions of text.Usually these objects are
 * used to get set of characters, translate digits into words
 * representation, perform case insensitive characters comparing and for
 * some others.
 *
 * \sa LangManager EngLang RusLang
 */
class Lang
{
public:
  enum CharType {UpCase = 0, LowCase = 1, Other = 2};

  /**\brief The destructor*/
  virtual ~Lang() {}

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
  virtual CharType getCharType(wchar_t c) const = 0;

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
  virtual bool equalChars(wchar_t c1, wchar_t c2) const = 0;

  /**\brief Replaces numbers with their word representation
   *
   * This method replaces all digits by their words representation with the
   * rules of some language. Single digits mode replaces just each digit
   * with corresponding word. 
   *
   * \param [in/out] str The string to process
   * \param [in] singleDigits Process each digit as separate number
   */
  virtual void expandNumbers(std::wstring& str, bool singleDigits) const = 0;

  /**\brief Returns all characters of this language alphabet (in all cases)
   *
   * This method allows you to enumerate all characters
   * used in corresponding language.
   *
   * \return All characters of language
   */
  virtual std::wstring getAllChars() const = 0;

  /**\brief Returns upper case of the specified character
   *
   * This method adjusts provided character to upper case.
   *
   * \param [in] ch The character to return upper case of
   *
   * \return The upper case of provided character
   */
  virtual wchar_t toUpper(wchar_t ch) const = 0;

  /**\brief Returns lower case of the specified character
   *
   * This method adjusts provided character to lower case.
   *
   * \param [in] ch The character to return lower case of
   *
   * \return The lower case of provided character
   */
  virtual wchar_t toLower(wchar_t ch) const = 0;

  /**\brief Returns upper case of the specified string
   *
   * This method adjusts characters of provided string to upper case.
   *
   * \param [in] str The string to return upper case of
   *
   * \return The upper case of provided string
   */
  virtual std::wstring toUpper(const std::wstring& str) const = 0;

  /**\brief Returns lower case of the specified string
   *
   * This method adjusts characters of provided string to lower case.
   *
   * \param [in] str The string to return lower case of
   *
   * \return The lower case of provided string
   */
  virtual std::wstring toLower(const std::wstring& str) const = 0;

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
  virtual std::wstring separate(const std::wstring& text) const = 0;

  /**\brief Marks letters to speak separately (usual acronims)
   *
   * This method marks characters to speak phonetically . Usually some
   * heuristic or predefined table are used.
   *
   * \param [in] text The string to process
   * \param [out] marks The boolean vector to save marks
   */
  virtual void markCapitals(const std::wstring& text, BoolVector& marks) const = 0;
};//class Lang;

typedef std::vector<const Lang*> ConstLangPtrVector;

/**\brief The abstract interface for all classes to get language objects by language identifier
 *
 * In daemon code language objects are usually identified by special
 * language ID values . This interface declares method to translate such
 * language identifiers to a valid language objects for further
 * operations.
 * \sa Lang LangManager
 */
class AbstractLangIdResolver
{
public:
  /**\brief The destructor*/
  virtual ~AbstractLangIdResolver() {}

  /**\brief Returns language object by language ID
   *
   * This method checks if provided language identifier is valid and
   * returns a pointer to corresponding language object or NULL otherwise.
   *
   * \param [in] langId The language identifier to get object by
   *
   * \return The pointer to language object or NULL if language ID is unknown
   */
  virtual const Lang* getLangById(LangId langId) const = 0;
}; //class AbstractLangIdResolver;

#endif //__VOICEMAN_LANG_H__
