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

#ifndef __VOICEMAN_ABSTRACT_TEXT_PROCESSOR_H__
#define __VOICEMAN_ABSTRACT_TEXT_PROCESSOR_H__

#include"Lang.h"
#include"TextParam.h"
#include"TextItem.h"

enum {DigitsModeNormal = 0, DigitsModeNone = 1, DigitsModeSingle = 2}; //enum DigitsMode;

/**\brief The unified interface for all text processors
 *
 * This class declares abstract interface for text processors. Currently
 * there is only one implementation, but there can be more in future. 
 *
 * \sa TextProcessor
 */
class AbstractTextProcessor
{
public:
  /**\brief The destructor*/
  virtual ~AbstractTextProcessor() {}

  /**\brief Sets new value for default language
   *
   * This method sets new value for default language. Some characters can
   * be marked to be processed by the language of precedent text. Default
   * language is used if such characters was met but there is no preceding 
   * text.
   *
   * \param [in] langId The new default language ID
   */
  virtual void setDefaultLangId(LangId langId) = 0;

  /**\brief Specifies the characters to be processed by specified language
   *
   * This method selects which characters must be processed by some
   * language. Note, the language ID can have LANG_ID_NONE value. It means
   * to process characters by the same language as preceding text.
   *
   * \param [in] str Teh string of characters to associate with language
   * \param [in] langId The ID of a language to associate with
   */
  virtual void associate(const std::wstring& str, LangId langId) = 0;

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
  virtual void addReplacement(LangId langId, const std::wstring& oldValue, const std::wstring& newValue) = 0;

  /**\brief Set special  value to describe character
   *
   * Special value is used to describe the single letter on corresponding letter command.
   *
   * \param [in] c The character to set value for
   * \param [in] value The value to set
   */
  virtual void setSpecialValueFor(wchar_t c, const std::wstring& value) = 0;

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
  virtual void process(const TextItem& text, std::list<TextItem>& items) const = 0;

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
  virtual void processLetter(wchar_t c, TextParam volume, TextParam pitch, TextParam rate, TextItemList& items) const = 0;
}; //class AbstractTextProcessor;

/**\brief Creates new text processor object
 *
   * This function creates new text processor. Digits mode means
   * how numbers must be processed. There are three variants: replace by
   * usual words representation, replace each digit by corresponding single
   * word or do nothing and let speech synthesizer process numbers by
   * itself. Used word "Capitalization" is not proper term. It means
   * invocation of language-specific function to mark letters for phonetic
   * pronunciation. Separation feature inserts space after any non-capital
   * letter followed by capital letter (for example, can be very useful for
   * Hungarian notation processing). 
 *
 * \param [in] langIdResolver The object to translate language identifiers to valid language objects
 * \param [in] digitMode The mode of digits processing (can be DigitsModeNormal, DigitsModeSingle and DigitsModeNone)
 * \param [in] capitalization The capitalization mode
 * \param [in] separation The separation mode
 *
 * \return The pointer to newly created text processor
 *
 * \sa AbstractTextProcessor TextProcessor 
 */
auto_ptr<AbstractTextProcessor> createNewTextProcessor(const AbstractLangIdResolver& langIdResolver, int digitsMode, bool capitalization, bool separation);

#endif //__VOICEMAN_ABSTRACT_TEXT_PROCESSOR_H__
