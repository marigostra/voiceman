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

#ifndef __VOICEMAN_TEXTITEM_H__
#define __VOICEMAN_TEXTITEM_H__

#include"TextParam.h"
#include"Lang.h"

/**\brief Portion of text with all necessary attributes
 *
 * This class contains portion of text for further translation to speech
 * with all necessary attributes, such used language, pitch, rate, volume
 * etc. Also it contains marks which letters in this part of text must be
 * spoken phonetically to get proper acronyms pronunciation. Not every of
 * these attributes must have proper values during entire lifetime
 * cycle. Some of them like output name are not set at object
 * creation. This class was designed to get convenient data storing. So
 * some information in it has redundant meaning.
 *
 * AbstractTextProcessor Executor Interface
 */
class TextItem
{
public:
  /**\brief The default constructor*/
  TextItem()
    : m_langId(LANG_ID_NONE) {}

  /**\brief The constructor with text specification
   *
   * \param [in] text The text string for the new text item
   */
  TextItem(const std::wstring& text)
    : m_text(text), m_langId(LANG_ID_NONE) {}

  /**\brief The constructor with language and text specification
   *
   * \param [in] langId The language for the new item
   * \param [in] text The text string for the new text item
   */
 TextItem(LangId langId, const std::wstring& text)
    : m_text(text), m_langId(langId) {}

  /**\brief The constructor with output name and text string specification
   *
   * \param [in] outputName The name of a output new item will be associated with
   * \param [in] text The text string for the new item
   */
  TextItem(const std::string& outputName, const std::wstring& text)
    : m_outputName(outputName), m_text(text), m_langId(LANG_ID_NONE) {}

  /**\brief The constructor with text and parameters specification
   *
   * \param [in] text The text string for the new item
   * \param [in] volume The volume value for the new item
   * \param [in] pitch The pitch value for the new item
   * \param [in] rate The rate value for the new item
   */
  TextItem(const std::wstring& text, TextParam volume, TextParam pitch, TextParam rate)
    : m_text(text), m_volume(volume), m_pitch(pitch), m_rate(rate), m_langId(LANG_ID_NONE) {}

  /**\brief The constructor with output name, text and parameters specification
   *
   * \param [in] outputName The name of the output new item will be associated with
   * \param [in] text The text string for the new item
   * \param [in] volume The volume value for the new item
   * \param [in] pitch The pitch value for the new item
   * \param [in] rate The rate value for the new item
   */
  TextItem(const std::string& outputName, const std::wstring& text, TextParam volume, TextParam pitch, TextParam rate)
    : m_outputName(outputName), m_text(text), m_volume(volume), m_pitch(pitch), m_rate(rate), m_langId(LANG_ID_NONE) {}

  /**\brief The constructor with language , text and parameters specification
   *
   * \param [in] langId The ID of a language for new item
   * \param [in] text The text string for the new item
   * \param [in] volume The volume value for the new item
   * \param [in] pitch The pitch value for the new item
   * \param [in] rate The rate value for the new item
   */
  TextItem(LangId langId, const std::wstring& text, TextParam volume, TextParam pitch, TextParam rate)
    : m_text(text), m_volume(volume), m_pitch(pitch), m_rate(rate), m_langId(langId) {}

  /**\brief Returns the text string of current text item
   *
   *The method to retreive text stored in this item.
   *
   * \return The text of this item 
   */
  std::wstring getText() const;

  /**\brief Sets new value of this item text string
   *
   * This method sets new text content for item. Be careful, the acronym
   * marks are always set to their initial values with this method call,
   * but all other attributes will be left unchanged.
   *
   * \param [in] text New text value
   */
  void setText(const std::wstring& text);

  /**\brief Resets all text item speech parameters to their default values
   *
   * This method resets three main speech parameters to their default
   * values. The attributes to reset are: volume, pitch and rate.
   */
  void resetParams();

  /**\brief Sets a mark to say phonetically to specified letter 
   *
   * This method marks one letter by its index to say it phonetically.
   *
   * \param [in] index The index of a letter to mark
   */
  void mark(size_t index);

  /**\brief removes mark from the specified letter
   *
   * This method removes mark from the letter by its index. It means not to
   * say it phonetically.
   *
   * \param [in] index The index of the letter to remove mark from
   */
  void unmark(size_t index);

  /**\brief Checks if the specified letter is marked
   *
   * This method can be used to check if specified letter must be spoken
   * phonetically.
   *
   * \param [in] index The index of the letter to check mark of
   *
   * \return Non-zero if specified letter has a mark
   */
  bool isMarked(size_t index) const;

  /**\brief Returnes volume for this text item
   *
   * Use this method to retreive volume value of this item.
   *
   * \return The volume value of this item
   */
  TextParam getVolume() const;

  /**\brief Sets volume for this item
   *
   * This method sets new volume value for this text item
   *
   * \param [in] volume The value to set
   */
  void setVolume(TextParam volume);

  /**\brief Returnes pitch for this text item
   *
   * Use this method to retreive pitch value of this item.
   *
   * \return The pitch value of this item
   */
  TextParam getPitch() const;

  /**\brief Sets pitch for this item
   *
   * This method sets new pitch value for this text item
   *
   * \param [in] pitch The value to set
   */
  void setPitch(TextParam pitch);

  /**\brief Returnes rate for this text item
   *
   * Use this method to retreive rate value of this item.
   *
   * \return The rate value of this item
   */
  TextParam getRate() const;

  /**\brief Sets rate for this item
   *
   * This method sets new rate value for this text item
   *
   * \param [in] rate The value to set
   */
  void setRate(TextParam rate);

  /**\brief Returns output name for this text item
   *
   * Use this method to retrieve name of the associated output. The output
   * name can by an empty string. It is a valid case, the output name can
   * be set later during further processing of this text item.
   *
   * \return The output name associated with this text item
*/
  std::string getOutputName() const;

  /**\brief Sets new value of output name
   *
   * This method sets new name of associated output
   *
   * \param [in] outputName The value to set
   */
  void setOutputName(const std::string& outputName);

  /**\brief Returns language of this text item
   *
   * Use this method to get language of this item by language ID.
   *
   * \return The language of this text item
   */
  LangId getLangId() const;

  /**\brief Sets new language for this item
   *
   * This method sets new language indication by the language ID.
   *
   * \param [in] langId The ID of the desired language
   */
  void setLangId(LangId langId);

private:
  void ensureMarksSize(size_t index);

private:
  std::string m_outputName;
  std::wstring m_text;
  BoolVector m_marks;
  TextParam m_volume, m_pitch, m_rate;
  LangId m_langId;
}; //class TextItem;

typedef std::vector<TextItem> TextItemVector;
typedef std::list<TextItem> TextItemList;

#endif //__VOICEMAN_TEXTITEM__
