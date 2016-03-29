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

#ifndef __VOICEMAN_PROTOCOL_H__
#define __VOICEMAN_PROTOCOL_H__

#include"Client.h"

/**\brief The abstract class to receive callback with parsed data
 *
 * This abstract class declares interface to receive events with data
 * transmitted by a client. Every object for handling connection data
 * must implement this interface. The reference to it must be provided into
 * protocol implementation class.
 *
 * \sa VoicemanProtocol ProtocolHandler
 */
class AbstractProtocolHandler
{
public:
  /**\brief Notifies the command to say text was received from client
   *
   * The protocol implementation class calls this method on every command
   * to say part of text and provides the client object this command was
   * transmitted from.
   *
   * \param [in] client The client object the command was received from
   * \param [in] t The text to say
   */
  virtual void onText(Client& client, const std::wstring& t) = 0;

  /**\brief Notifies the command to say one letter was received from client
   *
   * The protocol implementation calls this method each time when it
   * receives command to say one letter. The client object this command was
   * received from is provided.
   *
   * \param [in] client The client object this command was received from
   * \param [in] c The character to say
   */
  virtual void onLetter(Client& client, wchar_t c) = 0;

  /**\brief Notifies new command to stop playback was received from client
   *
   * The protocol implementation calls this function when it receives the
   * command to stop any playback. The client object new command was
   * received from is provided.
   *
   * \param [in] client The client object this command was received from
   */
  virtual void onStop(Client& client) = 0;

  /**\brief Notifies there was a command to set new value of speech parameter
   *
   * This method is called by protocol implementation object on every new
   * command to change speech attribute value. Speech attribute can be one
   * of three types: pitch, rate and volume. 
   *
   * \param [in] client The client object this command was received from
   * \param [in] paramType The attribute type to change, can be ParamPitch, ParamRate and ParamVolume
   * \param [in] value The new value to set
   */
  virtual void onParam(Client& client, int paramType, TextParam value) = 0;

  /**\brief Notifies the new command to produce tone signal was received from client
   *
   * This method is called by protocol implementation object each time when
   * new command to produce tone signal is received from a client. The
   * client object this command was received from is provided.
   *
   * \param [in] client The client object this command was received from
   * \param [in] freq The frequency of tone signal to produce
   * \param [in] duration The new signal duration
   */
  virtual void onTone(Client& client, size_t freq, size_t duration) = 0;

  /**\brief Notifies the command to select another processing mode was received
   *
   * This method is called by protocol implementation class on each command
   * to select new text processing mode. The client object new command was
   * received from and new mode name are provided.
   *
   * \param [in] client The client this command was received from
   * \param [in] procMode The name of new text processing mode
   */
  virtual void onProcMode(Client& client, const std::string& procMode) = 0;

  /**\brief Notifies the command to select new voice family was received from client
   *
   * The protocol implementation class cals this method each time when the
   * command to select new voice family was received from client. The
   * client object this command was received from, the language name to
   * select family for and new voice family name are provided. The language
   * string can be empty and it means new family must be selected for all
   * languages in which it is registered.
   *
   * \param [in] client The client this command was received from
   * \param [in] lang The language name to select family for, can be empty
   * \param [in] family The voice family to select
   */
  virtual void onFamily(Client& client, const std::string lang, const std::string& family) = 0;
}; //class AbstractProtocolCallback;

/**\brief The VoiceMan protocol implementation
 *
 * This is the main class to parse data received from a client and
 * convert it to sequence of commands for handling class. Handling class
 * must implement AbstractProtocolHandler interface and each command is
 * sent as corresponding method call. The Client object the line was
 * received from must be provided.
 *
 * \sa AbstractProtocolHandler ProtocolHandler
 */
class VoicemanProtocol
{
public:
  /**\brief The constructor
   *
   * \param [in] handler The reference to parsed data handling object 
   */
  VoicemanProtocol(AbstractProtocolHandler& handler)
    : m_handler(handler) {}

  /**\brief The destructor*/
  ~VoicemanProtocol() {}

  /**\brief Parses command string received from the client
   *
   * Each complete line of data received from client must be processed with
   * this method. It parses command and its parameters and invokes handling
   * method in handling class.
   *
   * \param [in] s The string to parse
   * \param [in] client The reference to the client object the string was received from
   */
  void process(const std::wstring& s, Client& client);

private:
  void parseLetter(const std::wstring& value, Client& client);
  void parseParam(int paramType, const std::wstring& value, Client& client);
  void parseTone(const std::wstring& value, Client& client);
  void parseFamily(const std::wstring& arg, Client& client);
  bool split(const std::wstring& s, wchar_t& cmd, std::wstring& arg);

private:
  AbstractProtocolHandler& m_handler;
}; //class VoicemanProtocol;

#endif //__VOICEMAN_PROTOCOL_H__
