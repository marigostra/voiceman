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

#ifndef __VOICEMAN_CLIENT_FACTORY_H__
#define __VOICEMAN_CLIENT_FACTORY_H__

#include"Client.h"

/**\brief The new client object factory
 *
 * This class performs new client object creation. It contains initial
 * values for text processing parameters and sets them to all newly
 * created objects. The reference to this factory must be provided to
 * main loop class for proper creation of new client objects.
 *
 * \sa Client MainLoop
 */
class ClientFactory
{
public:
  /**\brief Creates new client object and sets initial values to all parameters
   *
   * This method creates new "real" client and initiates all its internal
   * variables. "Real" client means it is the client associated with the
   * socket. The also can be "fake" client used to store client specific
   * data but not associated with the socket.
   *
   * \param [in] socket The socket object new client must be associated with
   *
   * \return The prepared client object
   */
  auto_ptr<Client> createNewClient(auto_ptr<Socket> socket) const
  {
    auto_ptr<Client> client(new Client(socket));
    client->volume = defaultVolume;
    client->pitch = defaultPitch;
    client->rate = defaultRate;
    client->selectedTextProcessor = "all";
    return client;
  }

  /**\brief Creates new fake client object
   *
   * This method creates new fake client object. "Fake" means this object
   * is not associated with the real connection and only stores client
   * specific data.
   *
   * \return The new prepared fake client object
   */
  auto_ptr<Client> createFakeClient() const
  {
    auto_ptr<Client> client(new Client());
    client->volume = defaultVolume;
    client->pitch = defaultPitch;
    client->rate = defaultRate;
    client->selectedTextProcessor = "all";
    return client;
  }

public:
  /**\brief The default volume value for new connections*/
  TextParam defaultVolume;

  /**\brief The default pitch value for new connections*/
  TextParam defaultPitch;

  /**\brief The default rate value for new connections*/
  TextParam defaultRate;
}; //class ClientFactory;

#endif //__VOICEMAN_CLIENT_FACTORY_H__
