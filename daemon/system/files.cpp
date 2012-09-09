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
#include"files.h"
#include"vmstrings.h"

#define IO_BUF_SIZE 2048

ssize_t readBlock(int fd, void* buf, size_t bufSize)
{
  assert(buf);
  char* b = (char*)buf;
  size_t c = 0;
  while(c < bufSize)
    {
      ssize_t res = read(fd, &b[c], bufSize - c);
      if (res == -1)
	return -1;
      if (res == 0)
	break;
      assert(res > 0);
      c += (size_t)res;
    } //while();
  return (ssize_t)c;
}

ssize_t writeBlock(int fd, const void* buf, size_t bufSize)
{
  assert(buf);
  char* b = (char*)buf;
  size_t c = 0;
  while(c < bufSize)
    {
      ssize_t res = write(fd, &b[c], bufSize - c);
      if (res == -1)
	return -1;
      assert(res >= 0);
      c += (size_t)res;
    } //while();
  assert(c == bufSize);
  return (ssize_t)c;
}

ssize_t readBuffer(int fd, void* buf, size_t bufSize)
{
  assert(buf);
  char* b = (char*)buf;
  size_t c = 0;
  while(c < bufSize)
    {
      const size_t requiredSize = bufSize > c + IO_BUF_SIZE?IO_BUF_SIZE:(size_t)(bufSize - c);
      const ssize_t res = readBlock(fd, &b[c], requiredSize);
      if (res == -1)
	return -1;
      c += (size_t)res;
      if (res < (ssize_t)requiredSize)
	break;
    } //while();
  return (ssize_t)c;
}

ssize_t writeBuffer(int fd, const void* buf, size_t bufSize)
{
  assert(buf);
  char* b = (char*)buf;
  size_t c = 0;
  while(c < bufSize)
    {
      const size_t requiredSize = bufSize > c + IO_BUF_SIZE?IO_BUF_SIZE:(size_t)(bufSize - c);
      const ssize_t res = writeBlock(fd, &b[c], requiredSize);
      if (res == -1)
	return -1;
      assert(res == (ssize_t)requiredSize);
      c += (size_t)res;
    } //while();
  assert(c == bufSize);
  return (ssize_t)c;
}

std::string cutComments(const std::string& str)
{
  std::string newStr;
  bool skipping = 0;
  for(std::string::size_type i = 0;i < str.length();i++)
    {
      if (str[i] == '#')
	skipping =1;
 if (str[i] == '\n')
   skipping = 0;
 if (!skipping)
   newStr += str[i];
    } //for();
  return newStr;
}

std::string readTextFile(const std::string& fileName)
{
  const int fd = open(fileName.c_str(), O_RDONLY);
  VM_SYS(fd!= -1, fileName);
  std::string s;
  char buf[2048];
  ssize_t readCount;
  do {
    readCount = readBlock(fd, buf, sizeof(buf));
    VM_SYS(readCount>=0, "read()");
    for(ssize_t i = 0;i < readCount;i++)
      s += buf[i];
  } while(readCount);
  close(fd);
  return s;
}

void splitTextFileLines(const std::string& source, StringList& lines, bool performTrim, bool skipEmpty)
{
  std::string s;
  for(std::string::size_type i = 0;i < source.length();i++)
    {
      if (source[i] == '\r')
	continue;
      if (source[i] == '\n')
	{
	  if (performTrim)
	    s = trim(s);
	  if (skipEmpty && s.empty())
	    continue;
	  lines.push_back(s);
	  s.erase();
	  continue;
	} // if '\n';
      s += source[i];
    } //for();
  if (performTrim)
    s = trim(s);
  if (!skipEmpty && !s.empty())
    lines.push_back(s);
}
