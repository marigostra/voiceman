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

#include"voiceman.h"
#include"MainLoop.h"

void MainLoop::run(const SocketList& sockets, sigset_t* sigMask)
{
  assert(sigMask != NULL);
  while(!m_terminationFlag)
    {
      fd_set fds;
      SocketList::const_iterator socketIt;
      ClientList::iterator clientIt;
      int maxFd = 0;
      FD_ZERO(&fds);
      for(socketIt = sockets.begin();socketIt != sockets.end();socketIt++)
	{
	  const int fd=(*socketIt)->getHandler();
	  if (fd > maxFd)
	    maxFd = fd;
	  FD_SET(fd, &fds);
	} //for (sockets);
      for(clientIt = m_connectedClients.begin();clientIt != m_connectedClients.end();clientIt++)
	{
	  const int fd=(*clientIt)->socket->getHandler();
	  if (fd > maxFd)
	    maxFd = fd;
	  FD_SET(fd, &fds);
	} //for (clients);
      const int executorStdout = m_executorOutput.getExecutorStdoutDescriptor();
      const int executorStderr = m_executorOutput.getExecutorStderrDescriptor();
      FD_SET(executorStdout, &fds);
      FD_SET(executorStderr, &fds);
      if (maxFd < executorStdout)
	maxFd = executorStdout;
      if (maxFd < executorStderr)
	maxFd = executorStderr;
      if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, sigMask) == -1)
	{
	  const int errorCode = errno;
	  logMsg(LOG_DEBUG, "pselect() has returned -1, checking what the reason...");
	  if (errorCode == EINTR)
	    {
	      logMsg(LOG_DEBUG, "pselect() call was interrupted by system signal, going to next iteration...");
	      m_signalHandler.onSystemSignal();
	      continue;
	    } //EINTR;
	  logMsg(LOG_DEBUG, "pselect() has returned an unexpected error, stopping main loop... ");
	  throw SystemException(errorCode, "pselect()");
	} //select has returned an error;
      if (FD_ISSET(executorStdout, &fds))
	{
	  logMsg(LOG_DEBUG, "New data available on executor stdout stream");
	  m_executorOutput.readExecutorStdoutData();
	  continue;
	}
      if (FD_ISSET(executorStderr, &fds))
	{
	  logMsg(LOG_DEBUG, "New data available on executor stderr stream");
	  m_executorOutput.readExecutorStderrData();
	  continue;
	}
      bool wasNewClients = 0;
      for(socketIt = sockets.begin();socketIt != sockets.end();socketIt++)
	{
	  const int fd = (*socketIt)->getHandler();
	  if (!FD_ISSET(fd, &fds))
	    continue;
	  //OK:we found new connected client;
	  logMsg(LOG_DEBUG, "New connection registered");
	  wasNewClients=1;
	  const int newClientFd = ::accept(fd, NULL, NULL);
	  if (newClientFd == -1)
	    {
	      logMsg(LOG_ERR, "New client cannot be accepted, accept() says \'%s\'", ERRNO_MSG);
	      continue;
	    }
	  logMsg(LOG_DEBUG, "New client connection was established (fd=%d)", newClientFd);
	  auto_ptr<Socket> newSocket(new Socket(newClientFd));
	  if (m_maxClients > 0 && m_connectedClients.size() >= m_maxClients)
	    {
	      newSocket->close();
	      logMsg(LOG_WARNING, "Client count limit reached, closing new connection (already have %u clients)", m_connectedClients.size());
	    } else//adding new record to the connected clients list;
	    {
	      auto_ptr<Client> newClient = m_clientFactory.createNewClient(newSocket);
	      m_connectedClients.push_back(newClient.get());
	      newClient.release();
	      logMsg(LOG_INFO, "New connection was successfully accepted and added to the list of connected clients (fd=%d)", newClientFd);
	    }
	} //new clients handling;
      if (wasNewClients)
	continue;//resetting set of file descriptors with new sockets;
      //now checking list of clients, is there any data from them;
      for(clientIt = m_connectedClients.begin();clientIt != m_connectedClients.end();clientIt++)
	{
	  int fd = (*clientIt)->socket->getHandler();
	  if (!FD_ISSET(fd, &fds))
	    continue;
	  std::string data;
	  bool toClose=0;
	  ssize_t readBytes = (*clientIt)->socket->read(data);
	  if (readBytes > 0)
	    logMsg(LOG_DEBUG, "Read %u bytes from client (fd=%d)", data.length(), fd);
	  if (readBytes <= 0)//if connection closed or there was reading problem;
	    toClose = 1;
	  if (readBytes < 0)//there was reading problem;
	    logMsg(LOG_ERR, "Problem reading data from client, connection will be closed (read(fd=%d) returned %s)", fd, ERRNO_MSG);
	  if (toClose)
	    {
	      (*clientIt)->socket->close();
	      delete *clientIt;
	      m_connectedClients.erase(clientIt);
	      logMsg(LOG_INFO, "Client was closed and its data destroyed (fd=%d)", fd);
	      break;//do not handle other clients;
	    } // closing connection;
	  m_clientDataHandler.processClientData(**clientIt, data);
	} // for (clients);
    } // while(!m_terminationFlag);
  for(ClientList::iterator it = m_connectedClients.begin();it != m_connectedClients.end();it++)
    delete *it;
  m_connectedClients.clear();
}
