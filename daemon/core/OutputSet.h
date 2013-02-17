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

#ifndef __VOICEMAN_OUTPUT_SET_H__
#define __VOICEMAN_OUTPUT_SET_H__

#include"Output.h"

/**\brief The set of active outputs
 *
 * This class is designed to store outputs prepared for functioning. It
 * allows silent reloading of the output set and hide any reference to
 * them from other classes.
 *
 * \sa Output
 */
class OutputSet
{
 public:
  /**\brief The default constructor*/
  OutputSet() {}

  /**\brief The destructor*/
  virtual ~OutputSet() {}

  /**\brief Fills set of available outputs with new data 
   *
   * This method sets new list of available outputs. It is called on server
   * configuration reloading and makes it silent and transparent for all
   * classes uses active outputs.
   *
   * \param [in] outputs New outputs to save
   */
  void reinit(const OutputList& outputs);

  /**\brief Checks output availability by its name
   *
   * This method allows you to be sure the output set contains an output
   * specified by its name.
   *
   * \param [in] outputName The name of the output to check
   *
   * \return Non-zero if specified output exists
   */
  bool hasOutput(const std::string& outputName) const;

  /**\brief Prepares the command line to invoke speech synthesizer of specified output
   *
   * This method generates a command line required to execute speech
   * synthesizer of some output. All required data is stored in output and
   * TextItem object. Synthesizer command line can contain various
   * parameters as speech volume, pitch and rate.
   *
   * \param [in] outputName The name of the output to generate command line by
   * \param [in] textItem The text item to generate command line for
   *
   * \return Generated synthesizer command line
   */
  std::string prepareSynthCommand(const std::string& outputName, const TextItem& textItem) const;

  /**\brief Generates command line to play portion of synthesized speech
   *
   * This method prepares command line to playback portion of synthesized
   * speech. Generated command line must accept audio data through its
   * stdin stream. Each output can contain three command line patterns for
   * three audio subsystems: alsa, pulseaudio and pc speaker. One of the
   * parameters chooses which one of them must be used.
   *
   *  \param [in] outputName The output name to generate command line with
   * \param [in] playerType The type of player to use, can be PlayerTypeAlsa, PlayerTypePulseaudio or PlayerTypePcspeaker
   * \param [in] textItem The text item to generate command line for
   *
   * \return The generated command line to execute player process
   */
  std::string preparePlayerCommand(const std::string& outputName, PlayerType playerType, const TextItem& textItem) const;

  /**\brief Prepares text to send to speech synthesizer
   *
   * This method makes all required output-specific text transformations to
   * send it to speech synthesizer. It can be any escaping or marks to
   * speak some characters phonetically.
   *
   * \param [in] outputName The name of the output to prepare text with
   * \param [in] textItem The text item to prepare text for
   *
   * \return The prepared text
   */
  std::string prepareText(const std::string& outputName, const TextItem& textItem) const;

  /**\brief Returns the output name corresponding to some voice family and language 
   *
   * This method looks through stored set of outputs and returns one with
   * corresponding voice family and language properties. It returns the
   * first one as it be found during the search, but this output must
   * exists. Use isValidFamily Name() method to be sure such output exists.
   *
   * \param [in] familyName The voice family of requested output
   * \param [in] langId The language identifier of requested output
   *
   * \return The name of requested output
   */
  std::string getOutputNameByFamilyAndLangId(const std::string& familyName, LangId langId) const;

  /**\brief Checks if some family name is valid for some language
   *
   * This method allows you to be sure the is available output with
   * corresponding voice family and associated language properties. 
   *
   * \param [in] langId The ID of the language to check family for
   * \param [in] familyName The voice family to check
   *
   * \return Non-zero if corresponding output exists in this set
   */
  bool isValidFamilyName(LangId langId, const std::string& familyName) const;

private:
  OutputVector m_outputs;
}; //class OutputSet;

#endif //__VOICEMAN_OUTPUT_SET_H__;
