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

#ifndef __VOICEMAN_SOCKETS_H__
#define __VOICEMAN_SOCKETS_H__

#include"SystemException.h"

/**\brief The base class for all sockets
 *
 * This class is the ancestor of all socket objects. In all socket
 * operations only server role is implemented. Considering the VoiceMan
 * daemon does not need any client functions. This class contains general
 * I/O operations and respond for automatic socket closing on object
 * destruction.
 *
 * \sa UnixSocket InetSocket
 */
class Socket
{
public:
  /**\brief The default constructor*/
  Socket()
    : m_sock(0), m_opened(0) {}

  /**\brief The constructor with file descriptor specification
   *
   * \param [in] fd The file descriptor new socket object must be associated with
   */
  Socket(int fd)
    : m_sock(fd), m_opened(1) {}

  /**\brief The destructor
   *
   * This destructor closes socket if it was opened and not closed.
   */
  virtual ~Socket() 
  {
    close();
  }

  /**\brief Reads data from the socket
   *
   * This method performs one read() call with appropriate parameters. The
   * value returned by this function is the native value from read() system
   * call.
   *
   * \param [out] buf The buffer to receive data
   * \param [in] s The size of a buffer to receive data
   *
   * \return The number of read bytes or -1 if there was an error (use errno for error code)
   */
  ssize_t read(void *buf, size_t s) const;

  /**\brief Writes data to socket
   *
   * This method performs one write() call with appropriate parameters. The
   * value returned by this function is the native value from write() system
   * call.
   *
   * \param [in] buf The a buffer with data to write
   * \param [in] s The number of bytes to write
   *
   * \return The number of bytes successfully written or -1 if there was an error (use errno to get error code)
   */
  ssize_t write(const void *buf, size_t s) const;

  /**\brief Reads data and saves it in string object
   *
   * This method tries to read 2048 bytes and saves available data in
   * string object. The number of read bytes does not have any meaning are
   * there more bytes to read or not. You should explicitly use select()
   * function or ioctrl(FIONREAD) calls to determine this.
*
   * \param [out] s The string object to receive data
   *
   * \return Number of read bytes or -1 if there was an error (use errno to get error code)
   */
  ssize_t read(std::string &s) const;

  /**\brief Closes connection
   *
   * This method will be called automatically 
   * on socket object deletion if it was not closed explicitly. You can call this method
   * in any situation, there is checking, not opened object will not be closed.
   */
  void close();

  /**\brief Returns system connection handler
   *
   * This method just returned file descriptor associated 
   * with current connection, but it may not be called for not opened sockets.
   *
   * \return The associated file descriptor
   */
  int getHandler() const;

  /**\brief Returns non-zero if connection was opened
   *
   * This method checks internal variables and let you 
   * know is this object is ready for I/O operations or not.
   *
   * \return Non-zero if connection is opened or zero otherwise
   */
  bool opened() const;

protected:
  int m_sock;
  bool m_opened;
}; //class Socket;

typedef std::list<Socket*> SocketList;

/**\brief The UNIX domain socket
 *
 * This class is the interface to create server UNIX domain socket. No
 * client behavior is implemented. Only file name of socket is required
 * to prepare object of this class.
 */
class UnixSocket: public Socket
{
public:
  /**\brief The default constructor*/
  UnixSocket() {}

  /**\brief The destructor*/
  virtual ~UnixSocket() {}

  /**\brief Creates new UNIX domain socket
   *
   * This method creates new UNIX domain socket and prepares it for
   * functioning. YOu should provide file name for new socket. This method
   * does not return any exit code. All errors are reported with
   * SystemException.
   *
   * \param [in] name The path to new UNIX domain socket
   *
   * \sa SystemException
   */
  void open(const std::string& name);
}; //class UnixSocket;

/**\brief The TCP/IP socket
 *
 * This class is the interface to create server UTCP/IP socket. No
 * client behavior is implemented. Only port number of socket is required
 * to prepare object of this class.
 */
class InetSocket: public Socket
{
public:
  /**\brief The default constructor*/
  InetSocket() {}

  /**\brief The destructor*/
  virtual ~InetSocket() {}

  /**\brief Creates new TCP/IP socket
   *
   * This method creates new TCP/IP socket and prepares it for
   * functioning. YOu should provide file name for new socket. This method
   * does not return any exit code. All errors are reported with
   * SystemException.
   * 
   * \param [in] port The port for new socket
   *
   * \sa SystemException
   */
  void open(int port);
}; //class InetSocket;

#endif // __VOICEMAN_SOCKETS_H__
