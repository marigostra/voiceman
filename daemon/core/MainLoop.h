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

#ifndef __VOICEMAN_MAIN_LOOP_H__
#define __VOICEMAN_MAIN_LOOP_H__

#include "ClientFactory.h"
#include"AbstractExecutorOutput.h"

/**\brief The abstract interface for system signal processing classes
 *
 * This class declares method being called in cases when pselect() system
 * call is interrupted by system signal. Usual signal handlers must
 * contain only registration which signal was caught but real signal
 * processing must be implemented in the derived classes of this one. 
 *
 * \sa MainLoop 
 */
class AbstractSignalHandler
{
public:
  /**\brief The destructor*/
  virtual ~AbstractSignalHandler() {}

  /**\brief Notifies there was system signal and it must be handled
   *
   * This method is called by main loop class each time when pselect()
   * system call is interrupted with EINTR exit code. It means the process
   * have received system signal and it must be handled.
   */
  virtual void onSystemSignal() = 0;
}; //class AbstractSignalHandler;

/**\brief The abstract class to handle client data
 *
 * This class declares an interface for objects to process data received
 * from clients. The data sent through this interface is not prepared and
 * is not checked. It is provided from MainLoop class. The strings even
 * must not be bounded to line ends.
 *
 * \sa MainLoop ClientDataHandler
 */
class AbstractClientDataHandler
{
public:
  /**\brief Callback method to notify new data was received
   *
   * This method notifies there is new data received from client and it must
   * be handled. The provided string is the raw data block, it can be not
   * bounded to line end, so handler must wait next one until it will be
   * sure the complete string is received.
   *
   * \param [in] client The client object data was received from
   * \param [in] data The received data
   */
  virtual void processClientData(Client& client, const std::string& data) = 0;
}; //class AbstractClientDataHandler;

/**\brief Main class to manage client connections
 *
 * This class manages clients connections and handles any data received
 * from clients. It contains main pselect() system call to wait new data
 * or new connections. This class uses list of currently accepted clients
 * but all clients must be closed explicitly on this classs destruction. It is not
 * recommended to have two instances of this class because of behavior
 * may depend on process signal handling. Also this class handles system signal checking 
 * and processing data received back from the executor process.
 *
 * \sa Client ExecutorInterface VoicemanServer
 */
class MainLoop
{
public:
  /**\brief The constructor
   *
   * \param [in] clientFactory The reference to a factory for new client object creation
   * \param [in] clients The list to store connected client set
   * \param [in] maxClients The maximum number of accepted clients (0 - not limited)
   * \param [in] clientDataHandler The reference to a object to handle data read from client connection
   * \param [in] signalHandler The reference to a object to handle registered signals
   * \param [in] executorOutput The reference to an object to provide information about executor output streams
   * \param [in] terminationFlag The reference to termination flag variable
   */
  MainLoop(const ClientFactory& clientFactory, ClientList& clients, size_t maxClients, AbstractClientDataHandler& clientDataHandler, AbstractSignalHandler& signalHandler, AbstractExecutorOutput& executorOutput, bool& terminationFlag)
    : m_clientFactory(clientFactory), m_connectedClients(clients), m_maxClients(maxClients), m_clientDataHandler(clientDataHandler), m_signalHandler(signalHandler), m_executorOutput(executorOutput), m_terminationFlag(terminationFlag) {}

  /**\brief The main method to execute loop and handle data
   *
   * Use this method to launch main loop and start client accepting.
   *
   * \param [in] sockets The list of sockets objects to listen
   * \param [in] sigMask The sigmal mask to operate with
   */
  void run(const SocketList& sockets, sigset_t* sigMask);

private:
  const ClientFactory& m_clientFactory;
  ClientList& m_connectedClients;
  const size_t m_maxClients;
  AbstractClientDataHandler& m_clientDataHandler;
  AbstractSignalHandler& m_signalHandler;
  AbstractExecutorOutput& m_executorOutput;
  bool& m_terminationFlag;
}; //class MainLoop;

#endif //__VOICEMAN_MAIN_LOOP_H__
