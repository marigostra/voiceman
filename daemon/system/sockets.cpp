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

#include"system.h"
#include"sockets.h"

void Socket::close()
{
  if (!m_opened)
    return;
  ::close(m_sock);
  m_opened=0;
}

ssize_t Socket::read(void* buf, size_t s) const
{
  assert(m_opened);
  return ::read(m_sock, buf, s);
}

ssize_t Socket::write(const void *buf, size_t s) const
{
  assert(m_opened);
  return ::write(m_sock, buf, s);
}

int Socket::getHandler() const
{
  assert(m_opened);
  return m_sock;
}

bool Socket::opened() const
{
  return m_opened;
}

ssize_t Socket::read(std::string &s) const
{
  char buf[2048];
  s.erase();
  const ssize_t c=read(buf, sizeof(buf));
  if (c <= 0)
    return c;
  for(ssize_t i=0;i<c;i++)
    s+=buf[i];
  return c;
}

void UnixSocket::open(const std::string &name)
{
  assert(!m_opened);
  unlink(name.c_str());
  struct sockaddr_un addr;
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  VM_SYS(fd!=-1, "socket(AF_UNIX, SOCK_STREAM, 0)");
  addr.sun_family=AF_UNIX;
  strncpy(addr.sun_path, name.c_str(), sizeof(addr.sun_path));
  if (bind(fd, (struct sockaddr*)&addr, sizeof(sockaddr_un)) == -1)
    {
      ::close(fd);
      throw SystemException("bind()");
    }
  if (listen(fd, 512) == -1)
    {
      ::close(fd);
      throw SystemException("listen()");
    }
  chmod(name.c_str(), 0666);
  m_sock=fd;
  m_opened=1;
}

void InetSocket::open(int port)
{
  assert(!m_opened);
  int fd=socket(AF_INET, SOCK_STREAM, 0);
  VM_SYS(fd!=-1, "socket(AF_INET, SOCK_STREAM, 0)");
  struct sockaddr_in addr;
  bzero(&addr, sizeof(struct sockaddr_in));
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=htonl(INADDR_ANY);
  addr.sin_port=htons(port);
  if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1)
    {
      ::close(fd);
      throw SystemException("bind()");
    }
  if (listen(fd, 512) == -1)
    {
      ::close(fd);
      throw SystemException("listen()");
    }
  m_sock=fd;
  m_opened=1;
}
