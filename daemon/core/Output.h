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

#ifndef __VOICEMAN_OUTPUT_H__
#define __VOICEMAN_OUTPUT_H__

#include"TextItem.h"
#include"Lang.h"

/**\brief A replacement item, associated with some output
 *
 * Each output can have set of replacements associated with it to fix
 * some text pronounciation. These replacements must not be confused with
 * replacements used for language-specific operations in TextProcessor
 * class.
 *
 * \sa Output
 */
struct OutputReplacement 
{
  /**\brief The default constructor*/
  OutputReplacement() {}

  /**\brief The constructor
   *
   * \param [in] oldstring The string to replace
   * \param [in] newstring The string to replace with
   */
  OutputReplacement(const std::wstring& oldString, const std::wstring& newString)
    : oldValue(oldString), newValue(newString) {}

  /**\brief The string to replace*/
  std::wstring oldValue;

  /**\brief The string to replace with*/
  std::wstring newValue;
}; //struct OutputReplacement;

typedef std::vector<OutputReplacement> OutputReplacementVector;
typedef std::list<OutputReplacement> OutputReplacementList;

/**\brief The class with complete information about valid and ready to use output
 *
 * The output in VoiceMan terminology means one available speech
 * synthesizer with all its corresponding settings, required to invoke it
 * and make speech generation. One available in your system synthesizer
 * must not be represented exactly by one output, there can be several
 * outputs for each TTS witrh different parameters. This class does not
 * perform synthesizer execution, it only stores information about it and
 * makes some data preprocessing. External synthesizer calls are made by
 * separate process, called executor. It is managed by ExecutorInterface
 * class.
 *
 *  \sa OutputSet
 */
class Output
{
public:
  /**\brief The constructor*/
  Output()
    : m_langId(LANG_ID_NONE), m_lang(NULL) {}

  /**\brief The destructor*/
  virtual ~Output() {}

  /**\brief Returns the language ID this output is processing text of
   *
   * Use this method to get the ID of a language 
   * this output is associated with.
   *
   * \return The ID of a language this class is associated with
   */
  LangId getLangId() const
  {
    return m_langId;
  }

  /**\brief Sets new language ID this output is processing text of
   *
   * Use this method to set new language ID this output 
   * can process text of.
   *
   * \param [in] langId The ID of the language to set
   */
  void setLangId(LangId langId)
  {
    m_langId = langId;
  }

  /**\brief Sets new pointer to language object
   *
   * Language object is used by Output class for miscellaneous text
   * processing before sending to executor. This method allows you get
   * current stored value of Lang pointer.
   *
   * \return The pointer to used language object
   */
  const Lang* getLang() const
  {
    return m_lang;
  }

  /**\brief Sets new pointer to language object
   *
   * Language object is used by Output class for miscellaneous text
   * processing before sending to executor. This method allows you set
   * current stored value of Lang pointer.
   * The used language object must be correspondent to language ID  witch 
   * can be set separately.
   *
   * \param [in] lang The pointer to language object to set
   */
  void setLang(const Lang* lang)
  {
    m_lang = lang;
  }

  /**\brief Returns the name of this output
   *
   * Use this method to get name of this output.
   *
   * \return The output name
   */
  std::string getName() const
  {
    return m_name;
  }

  /**\brief Sets new name for this output
   *
   * Use this method to set new name for this output.
   *
   * \param [in] name The new name to set
   */
  void setName(const std::string& name)
  {
    m_name = name;
  }

  /**\brief Returns the voice family of this output
   *
   * Use this method to get voice family of this output.
   *
   * \return The voice family of this output.
   */
  std::string getFamily() const
  {
    return m_family;
  }

  /**\brief Sets new voice family of this output
   *
   * Use this method to set new voice family to this output.
   *
   * \param [in] family The new voice family to set
   */
  void setFamily(const std::string& family)
  {
    m_family = family;
  }

  /**\brief Generates the command line to execute speech synthesizer 
   *
   * This method can generate command line to execute speech 
   * synthesizer with attributes corresponding to some text item.
   *
   * \param [in] textItem The part of text data to generate command line for
   *
   * \return The generated command line
   */
  std::string prepareSynthCommand(const TextItem& textItem)const;


  /**\brief Generates the command line to execute ALSA player
   *
   * This method can generate command line to execute ALSA player
   * with attributes corresponding to some text item.
   *
   * \param [in] textItem The part of text data to generate command line for
   *
   * \return The generated command line
   */
  std::string prepareAlsaPlayerCommand(const TextItem& textItem)const;

  /**\brief Generates the command line to execute PulseAudio player
   *
   * This method can generate command line to execute Pulse Audio player
   * with attributes corresponding to some text item.
   *
   * \param [in] textItem The part of text data to generate command line for
   *
   * \return The generated command line
   */
  std::string preparePulseaudioPlayerCommand(const TextItem& textItem)const;

  /**\brief Generates the command line to execute PC speaker player
   *
   * This method can generate command line to execute PC speaker player
   * with attributes corresponding to some text item.
   *
   * \param [in] textItem The part of text data to generate command line for
   *
   * \return The generated command line
   */
  std::string preparePcspeakerPlayerCommand(const TextItem& textItem)const;

  /**\brief Prepares text to send to speech synthesizer
   *
   * This method makes all necessary operations with text 
   * to prepare it for transmission to speech synthesizer.
   *
   * \param [in] textItem The text item to prepare text of
   *
   * \return The prepared text
   */
  std::string prepareText(const TextItem& textItem) const;

  /**\brief Sets new command line template to run speech synthesizer
   *
   * This method sets new command line pattern. The pattern can contain
   * special character sequences to mark places where various speech
   * parameters must be inserted. These sequences are "%p" to put voice
   * pitch, "%r" to put voice rate and "%v" to put voice volume.
   *
   * \param [in] cmdLine The command line pattern to set
   */
  void setSynthCommand(const std::string& cmdLine)
  {
    m_synthCommand = cmdLine;
  }

  /**\brief Sets new command line template to run ALSA playe 
   *
   * This method sets new command line pattern. The pattern can contain
   * special character sequences to mark places where various speech
   * parameters must be inserted. These sequences are "%p" to put voice
   * pitch, "%r" to put voice rate and "%v" to put voice volume.
   *
   * \param [in] cmdLine The command line pattern to set
   */
  void setAlsaPlayerCommand(const std::string& cmdLine)
  {
    m_alsaPlayerCommand = cmdLine;
  }

  /**\brief Sets new command line template to run Pulse Audio player
   *
   * This method sets new command line pattern. The pattern can contain
   * special character sequences to mark places where various speech
   * parameters must be inserted. These sequences are "%p" to put voice
   * pitch, "%r" to put voice rate and "%v" to put voice volume.
   *
   * \param [in] cmdLine The command line pattern to set
   */
  void setPulseaudioPlayerCommand(const std::string& cmdLine)
  {
    m_pulseaudioPlayerCommand = cmdLine;
  }

  /**\brief Sets new command line template to run PC speaker player
   *
   * This method sets new command line pattern. The pattern can contain
   * special character sequences to mark places where various speech
   * parameters must be inserted. These sequences are "%p" to put voice
   * pitch, "%r" to put voice rate and "%v" to put voice volume.
   *
   * \param [in] cmdLine The command line pattern to set
   */
  void setPcspeakerPlayerCommand(const std::string& cmdLine)
  {
    m_pcspeakerPlayerCommand = cmdLine;
  }

  /**\brief Adds new replacement to mark capitalized letter
   *
   * Every letter can have special sequence used to speak this letter
   * phonetically. This method sets such sequence for some letter.
   *
   * \param [in] c The letter being marked
   * \param [in] value The text string to replace with
   */
  void addCapMapItem(wchar_t c, const std::wstring& value);

  /**\brief Sets the format of pitch value used in command lines
   *
   * This value will be used in synthesizer and player calls during speech
   * generation. Purpose of each parameter is describe in TextParam
   * documentation. If different players require different form of some
   * parameter, you should avoid it specification in players command line
   * and use it only in command line of synthesizer.
   * 
   * \param [in] digits Number of digits after decimal dot 
   * \param [in] min The number to translate minimal parameter value into
   * \param [in] aver The number to translate average parameter value into
   * \param [in] max The number to translate maximum parameter value into
   *
   * \sa TextParam
   */
  void setPitchFormat(size_t digits, double min, double aver, double max);

  /**\brief Sets the format of rate value used in command lines
   *
   * This value will be used in synthesizer and player calls during speech
   * generation. Purpose of each parameter is describe in TextParam
   * documentation. If different players require different form of some
   * parameter, you should avoid it specification in players command line
   * and use it only in command line of synthesizer.
   * 
   * \param [in] digits Number of digits after decimal dot 
   * \param [in] min The number to translate minimal parameter value into
   * \param [in] aver The number to translate average parameter value into
   * \param [in] max The number to translate maximum parameter value into
   *
   * \sa TextParam
   */
  void setRateFormat(size_t digits, double min, double aver, double max);

  /**\brief Sets the format of volume value used in command lines
   *
   * This value will be used in synthesizer and player calls during speech
   * generation. Purpose of each parameter is describe in TextParam
   * documentation. If different players require different form of some
   * parameter, you should avoid it specification in players command line
   * and use it only in command line of synthesizer.
   * 
   * \param [in] digits Number of digits after decimal dot 
   * \param [in] min The number to translate minimal parameter value into
   * \param [in] aver The number to translate average parameter value into
   * \param [in] max The number to translate maximum parameter value into
   *
   * \sa TextParam
   */
  void setVolumeFormat(size_t digits, double min, double aver, double max);

  /**\brief Adds new output replacement item
   *
   * Use this method to add new replacement item to fix text processing.
   *
   * \param [in] oldString The string to replace
   * \param [in] newString The string to replace with
   */
  void addReplacement(const std::wstring& oldString, const std::wstring& newString)
  {
    m_replacements.push_back(OutputReplacement(oldString, newString));
  }

private:
  struct FloatValueFormat
  {
    FloatValueFormat()
      : digits(2), min(0), aver(0.5), max(1) {}

    size_t digits;
    double min;
    double aver;
    double max;
  }; //struct FloatValueFormat;

private:
  bool findReplacement(const std::wstring& str, std::wstring::size_type pos, OutputReplacementVector::size_type& result) const;
  std::wstring insertReplacements(const std::wstring& str) const;
  std::wstring makeCaps(const TextItem& textItem) const;
  std::string prepareCommandLine(const std::string& pattern, const TextItem& textItem) const;
  std::string prepareFloatValue(TextParam value, const FloatValueFormat& format) const;

private:
  typedef std::map<wchar_t, std::wstring> WCharToWStringMap;

  LangId m_langId;
  const Lang* m_lang;
  std::string m_name, m_family;
  WCharToWStringMap m_capList;
  std::string m_synthCommand;
  std::string m_alsaPlayerCommand, m_pulseaudioPlayerCommand, m_pcspeakerPlayerCommand;
  FloatValueFormat m_pitchFormat;
  FloatValueFormat m_rateFormat;
  FloatValueFormat m_volumeFormat;
  OutputReplacementVector m_replacements;
}; //class Output;

typedef std::vector<Output> OutputVector;
typedef std::list<Output> OutputList;

#endif // __VOICEMAN_OUTPUT_H__
