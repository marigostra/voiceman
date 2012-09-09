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

#include<stdlib.h>
#include<string>
#include<iostream>
#include<sys/types.h>
#include<errno.h>
#include<iconv.h>
#include"Transcoding.h"

#define ICONV_UTF8_ID "utf8"
#define ICONV_WSTRING_ID "utf32le"
#define ICONV_BLOCK_SIZE 50
#define WSTRING_BAD_CHAR L'?'
#define STRING_BAD_CHAR '?'

Transcoding transcoding;

Transcoding::Transcoding()
{
  if (!initCurIO())
    {
      std::cerr << "FATAL:There are some problems with selecting charset for I/O operations. Exiting..." << std::endl;
      std::cerr << "Probably environment variable $LANG is not set correctly and contains an invalid value." << std::endl;
      exit(1);
    }
  if (!initIConv())
    {
      std::cerr << "Sorry, there are some problems with preparing iconv library." << std::endl;
      std::cerr << "Please, ensure you have this library support in your system." << std::endl;
      std::cerr << "Exiting..." << std::endl;
      exit(1);
    }
}

std::string Transcoding::getIOCharset() const
{
  return m_curIO;
}

bool Transcoding::initIConv()
{
  m_iconvIO2WString = iconv_open(ICONV_WSTRING_ID, m_curIO.c_str());
  if (m_iconvIO2WString == (iconv_t)-1)
    return 0;
  m_iconvWString2IO = iconv_open(m_curIO.c_str(), ICONV_WSTRING_ID);
  if (m_iconvWString2IO==(iconv_t)-1)
    return 0;
  m_iconvUTF82WString = iconv_open(ICONV_WSTRING_ID, ICONV_UTF8_ID);
  if (m_iconvUTF82WString==(iconv_t)-1)
    return 0;
  m_iconvWString2UTF8 = iconv_open(ICONV_UTF8_ID, ICONV_WSTRING_ID);
  if (m_iconvWString2UTF8==(iconv_t)-1)
    return 0;
  return 1;
}

bool Transcoding::initCurIO()
{
  setlocale(LC_ALL, "");
  std::string lang;
  char* l=getenv("LANG");
  if (!l)
    lang="POSIX"; else
      lang=l;
  ssize_t d=-1;
  std::string::size_type i;
  for(i = 0;i < lang.length();i++)
    if (lang[i]=='.')
      d=i;
  if (d < 0)
    {
      m_curIO="US-ASCII";
      return 1;
    }
  std::string cp;
  for(i = d + 1;i < lang.length();i++)
    cp += lang[i];
  m_curIO = cp;
  return 1;
}

std::ostream& operator <<(std::ostream& s, const std::wstring& ws)
{
  s << WString2IO(ws);
  return s;
}

std::wstring Transcoding::trIO2WString(const std::string& s) const
{
  std::wstring res;
  size_t i;
  char* b = new char[s.length()];
  char* bb = b;
  for(i = 0;i < s.length();i++)
    b[i] = s[i];
  std::string::size_type bSize = s.length();
  while(bSize)
    {
      wchar_t* r = new wchar_t[ICONV_BLOCK_SIZE];
      char* rr=(char*)r;
      size_t rsize=ICONV_BLOCK_SIZE*sizeof(wchar_t);
      if (iconv(m_iconvIO2WString, &b, &bSize, &rr, &rsize) == (size_t)(-1))
	{
	  if (errno == EILSEQ)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      res += WSTRING_BAD_CHAR;
	      b++;
	      bSize--;
	      delete[] r;
	      continue;
	    } // bad sequence;
	  if (errno == EINVAL)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      res += WSTRING_BAD_CHAR;
	      delete[] r;
	      break;
	    }
	  if (errno == E2BIG)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      delete[] r;
	      continue;
	    }
	} // (size_t)(-1);
      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
	res += r[i];
      delete[] r;
    } // while;
  delete[] bb;
  return res;
}

std::string Transcoding::trWString2IO(const std::wstring& s) const
{
  std::string res;
  size_t i;
  wchar_t* b = new wchar_t[s.length()];
  char* bb = (char*)b;
  for(i = 0;i < s.length();i++)
    b[i] = s[i];
  std::string::size_type bSize = s.length()*sizeof(wchar_t);
  while(bSize)
    {
      char* r = new char[ICONV_BLOCK_SIZE],* rr = r;
      size_t rsize = ICONV_BLOCK_SIZE;
      if (iconv(m_iconvWString2IO, &bb, &bSize, &rr, &rsize) == (size_t)(-1))
	{
	  if (errno == EILSEQ)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
		res += r[i];
	      res += STRING_BAD_CHAR;
	      for(i = 0;i < sizeof(wchar_t);i++)
		bb++;
	      bSize -= sizeof(wchar_t);
	      delete[] r;
	      continue;
	    } // bad sequence;
	  if (errno == EINVAL)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
		res += r[i];
	      res += STRING_BAD_CHAR;
	      delete[] r;
	      break;
	    }
	  if (errno == E2BIG)
	    {
	      for( i= 0;i < ICONV_BLOCK_SIZE - rsize;i++)
		res += r[i];
	      delete[] r;
	      continue;
	    }
	} // (size_t)(-1);
      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
	res += r[i];
      delete[] r;
    } // while;
  delete[] b;
  return res;
}

std::string Transcoding::trEncodeUTF8(const std::wstring& s) const
{
  std::string res;
  size_t i;
  wchar_t* b = new wchar_t[s.length()];
  char* bb = (char*)b;
  for(i = 0;i < s.length();i++)
    b[i] = s[i];
  std::string::size_type bSize = s.length()*sizeof(wchar_t);
  while(bSize)
    {
      char* r = new char[ICONV_BLOCK_SIZE], *rr = r;
      size_t rsize = ICONV_BLOCK_SIZE;
      if (iconv(m_iconvWString2UTF8, &bb, &bSize, &rr, &rsize) == (size_t)(-1))
	{
	  if (errno == EILSEQ)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
		res += r[i];
	      res += STRING_BAD_CHAR;
	      for(i = 0;i < sizeof(wchar_t);i++)
		bb++;
	      bSize -= sizeof(wchar_t);
	      delete[] r;
	      continue;
	    } // bad sequence;
	  if (errno == EINVAL)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
		res += r[i];
	      res += STRING_BAD_CHAR;
	      delete[] r;
	      break;
	    }
	  if (errno == E2BIG)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
		res += r[i];
	      delete[] r;
	      continue;
	    }
	} // (size_t)(-1);
      for(i = 0;i < ICONV_BLOCK_SIZE - rsize;i++)
	res += r[i];
      delete[] r;
    } // while;
  delete[] b;
  return res;
}

bool Transcoding::trDecodeUTF8(const std::string& s, std::wstring& res) const
{
  res.erase();
  size_t i;
  char* b = new char[s.length()];
  char* bb = b;
  for(i = 0;i < s.length();i++)
    b[i] = s[i];
  std::string::size_type bSize = s.length();
  while(bSize)
    {
      wchar_t* r = new wchar_t[ICONV_BLOCK_SIZE];
      char* rr = (char*)r;
      size_t rsize = ICONV_BLOCK_SIZE*sizeof(wchar_t);
      if (iconv(m_iconvUTF82WString, &b, &bSize, &rr, &rsize) == (size_t)(-1))
	{
	  if (errno == EILSEQ)
	    {
	      delete[] r;
	      delete[] bb;
	      return 0;
	    } // bad sequence;
	  if (errno == EINVAL)
	    {
	      delete[] r;
	      delete[] bb;
	      break;
	    }
	  if (errno == E2BIG)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      delete[] r;
	      continue;
	    }
	} // (size_t)(-1);
      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
	res += r[i];
      delete[] r;
    } // while;
  delete[] bb;
  return 1;
}

std::wstring Transcoding::trReadUTF8(const std::string& s) const
{
  std::wstring res;
  size_t i;
  char* b = new char[s.length()];
  char* bb = b;
  for(i = 0;i < s.length();i++)
    b[i] = s[i];
  std::string::size_type bSize = s.length();
  while(bSize)
    {
      wchar_t* r = new wchar_t[ICONV_BLOCK_SIZE];
      char* rr = (char*)r;
      size_t rsize = ICONV_BLOCK_SIZE*sizeof(wchar_t);
      if (iconv(m_iconvUTF82WString, &b, &bSize, &rr, &rsize) == (size_t)(-1))
	{
	  if (errno == EILSEQ)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      res += WSTRING_BAD_CHAR;
	      res += WSTRING_BAD_CHAR;
	      res += WSTRING_BAD_CHAR;
	      delete[] r;
	      break;
	    } // bad sequence;
	  if (errno == EINVAL)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      res += WSTRING_BAD_CHAR;
	      delete[] r;
	      break;
	    }
	  if (errno == E2BIG)
	    {
	      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
		res += r[i];
	      delete[] r;
	      continue;
	    }
	} // (size_t)(-1);
      for(i = 0;i < ICONV_BLOCK_SIZE - rsize / sizeof(wchar_t);i++)
	res += r[i];
      delete[] r;
    } // while;
  delete[] bb;
  return res;
}
