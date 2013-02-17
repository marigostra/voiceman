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

#ifndef __VOICEMAN_CLIENT_H__
#define __VOICEMAN_CLIENT_H__

#include"TextParam.h"
#include"system/sockets.h"

/**\brief The class to store client specific data
 *
 * This class stores all information about client connection. It contains
 * current values for all text attributes, assigned for this connection
 * and a socket object for data exchange. One client can has only one
 * instance of this object because it automatically closes socket on
 *object destruction.
 *
 * \sa ClientFactory
 */
class Client
{
public:
  /**\brief The default constructor*/
  Client()
    : rejecting(0) {}

  /**\brief The constructor with socket object specification
   *
   * \param [in] s The socket object for data exchanging
   */
  Client(auto_ptr<Socket> s)
    : rejecting(0), socket(s) {}

  /**\brief The destructor*/
  virtual ~Client() {}

public:
  /**\brief The chain of received incomplete data*/
  std::wstring chain;

  /**\brief The current volume value associated with the connection*/
  TextParam volume;

  /**\brief The current pitch value associated with the connection*/
  TextParam pitch;

  /**\brief The current rate value associated with the connection*/
  TextParam rate;

  /** The voice families for different language text processing*/
  LangIdToStringMap selectedFamilies;

  /**\brief The name of selected text processor*/
  std::string selectedTextProcessor;

  /**\brief Is server skip all data from the client due to line length exceed*/
  bool rejecting;

  /**\brief The client connection socket*/
  auto_ptr<Socket> socket;
}; //class Client;

typedef std::list<Client*> ClientList;

#endif // __VOICEMAN_CLIENT_H__
