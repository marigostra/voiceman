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

#ifndef __VOICEMAN_TEXT_PROCESSOR_H__
#define __VOICEMAN_TEXT_PROCESSOR_H__

#include"TextItem.h"
#include"AbstractTextProcessor.h"

/**\brief Stores all information about a replacement item
 *
 * There are lists of replacements associated with each
 * language. Replacements can be used for punctuation processing or any
 * other purpose. This struct contains all necessary information about
 * one replacement. String fields must store exact values, no pattern or
 * expressions are allowed.
 */
struct Replacement 
{
  /**\brief The default constructor*/
  Replacement()
    : langId(LANG_ID_NONE) {}

  /**\brief The constructor
   *
   * \param [in] lang The ID of the language this replacement is associated with
   * \param [in] oldstring The string to replace
   * \param [in] newstring The string to replace with
   */
  Replacement(LangId lang, const std::wstring& oldString, const std::wstring& newString)
    : langId(lang),oldValue(oldString), newValue(newString) {}

  /**\brief The language this replacement is associated with*/
  LangId langId;

  /**\brief The string to replace*/
  std::wstring oldValue;

  /**\brief The string to replace with*/
  std::wstring newValue;
}; //struct Replacement;

typedef std::vector<Replacement> ReplacementVector;
typedef std::list<Replacement> ReplacementList;

/**\brief makes general text processing
 *
 * This class plays the major role in text processing. It splits input
 * string to a sequence of items by language settings, makes replacements
 * and invokes language-specific operations such numbers processing and
 * selecting characters for phonetic pronunciation. It is almost complete
 * required text handling except output assignment to generated text
 * items and output-specific processing. Output assignment must be
 * performed later because all clients can have own preferences which
 * output must serve some language.
 *
 * \sa Output ProtocolHandler
 */
class TextProcessor: public AbstractTextProcessor
{
public:
  /**\brief The constructor
   *
   * \param [in] langIdResolver The class to translate language identifiers to a valid language objects
   */
  TextProcessor(const AbstractLangIdResolver& langIdResolver)
    : m_langIdResolver(langIdResolver), m_defaultLangId(LANG_ID_NONE), m_digitsMode(DigitsModeNormal), m_capitalization(1), m_separation(1) {}

  /**\brief The destructor*/
  virtual ~TextProcessor() {}

  /**\brief Sets some text processing parameters
   *
   * This method sets some text processing parameters. Digits mode means
   * how numbers must be processed. There are three variants: replace by
   * usual words representation, replace each digit by corresponding single
   * word or do nothing and let speech synthesizer process numbers by
   * itself. Used word "Capitalization" is not proper term. It means
   * invocation of language-specific function to mark letters for phonetic
   * pronunciation. Separation feature inserts space after any non-capital
   * letter followed by capital letter (for example, can be very useful for
   * Hungarian notation processing). 
   *
   * \param [in] digitsMode The mode of digits processing (can be DigitsModeNormal, DigitsModeSingle or DigitsModeNone)
   * \param [in] capitalization The capitalization mode
   * \param [in] separation The separation mode
   */
  void setMode(int digitsMode, bool capitalization, bool separation);

  /**\brief Sets new value for default language
   *
   * This method sets new value for default language. Some characters can
   * be marked to be processed by the language of precedent text. Default
   * language is used if such characters was met but there is no preceding 
   * text.
   *
   * \param [in] langId The new default language ID
   */
  void setDefaultLangId(LangId langId);

  /**\brief Specifies the characters to be processed by specified language
   *
   * This method selects which characters must be processed by some
   * language. Note, the language ID can have LANG_ID_NONE value. It means
   * to process characters by the same language as preceding text.
   *
   * \param [in] str Teh string of characters to associate with language
   * \param [in] langId The ID of a language to associate with
   */
  void associate(const std::wstring& str, LangId langId);

  /**\brief Adds new text replacement
   *
   * This method adds new replacement, associated with some language.
   *
   * \param [in] langId The ID of a language to add replacement for
   * \param [in] oldValue The text to be replaced
   * \param [in] newValue The text to replace with
   *
   * \sa Replacement
   */
  void addReplacement(LangId langId, const std::wstring& oldValue, const std::wstring& newValue);

  /**\brief Set special  value to describe character
   *
   * Special value is used to describe the single letter on corresponding letter command.
   *
   * \param [in] c The character to set value for
   * \param [in] value The value to set
   */
  void setSpecialValueFor(wchar_t c, const std::wstring& value);

  /**\brief Processes text before speaking
   *
   * This method splits text string onto sequence of text items regarding
   * current language settings. Produced items is almost ready to be sent
   * to speech sinthesizer but exact output is not selected. It must be
   * choosed explicitly according with current client preferences.
   *
   * \param [in] text The text to process
   * \param [out] items The reference to text item list to receive a result
   */
  void process(const TextItem& text, std::list<TextItem>& items) const;

  /**\brief Processes single letter
   *
   * This method handles the "LETTER" command and generates corresponding text item.
   *
   * \param [in] c The character to process
   * \param [in] volume The desired volume value
   * \param [in] pitch The desired pitch value
   * \param [in] rate The desired rate value
   * \param [out] items The reference to text item list to receive a result
   */
  void processLetter(wchar_t c, TextParam volume, TextParam pitch, TextParam rate, TextItemList& items) const;

private:
  const Lang* getLangById(LangId langId) const;
  bool findReplacement(const std::wstring& str, std::wstring::size_type pos, LangId langId, ReplacementVector::size_type& result) const;
  std::wstring insertReplacements(const std::wstring& str, LangId langId) const;
  void processItem(TextItem& text) const;
  void split(const std::wstring& text, TextItemList& items) const;

private:
  typedef std::map<wchar_t, LangId> WCharToLangIdMap;

  const AbstractLangIdResolver& m_langIdResolver;
  LangId m_defaultLangId;
  int m_digitsMode;
  bool m_capitalization, m_separation;
  WCharToLangIdMap m_charsTable;
  WCharToWStringMap m_specialValues;
  ReplacementVector m_replacements;
}; //class TextProcessor;

#endif //__VOICEMAN_TEXT_PROCESSOR_H__
