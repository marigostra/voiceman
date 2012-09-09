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

#include"voiceman.h"
#include"DelimitedFile.h"

static std::string cutComment(const std::string& line)
{
  std::string s;
  bool inQuotes = 0;
  for(std::string::size_type i = 0;i < line.length();i++)
    {
      if (!inQuotes && line[i] == '\"')
	{
	  s += '\"';
	  inQuotes = 1;
	  continue;
	}
      if (inQuotes && line[i] == '\"')
	{
	  if (i + 1 < line.length() && line[i + 1] == '\"')
	    {
	      s += "\"\"";
	      i++;
	      continue;
	    }
	  s += '\"';
	  inQuotes = 0;
	  continue;
	}
      if (line[i] == '#')
	{
	  if (!inQuotes)
	    return s;
	  s += '#';
	  continue;
	}
      s += line[i];
    } //for();
  return s;
}

void DelimitedFile::read(const std::string& fileName)
{
  m_lines.clear();
  m_lines.clear();
  std::string s = readTextFile(fileName);
  s += '\n';
  std::string ss;
  for(std::string::size_type i = 0;i < s.length();i++)
    {
      if (s[i] == '\r')
	continue;
      if (s[i] == '\n')
	{
	  ss = cutComment(ss);
	  if (!trim(ss).empty())
	    m_lines.push_back(ss);
	  ss.erase();
	  continue;
	} // if '\n';
      ss+= s[i];
    } // for;
}

size_t DelimitedFile::getItemCountInLine(size_t index) const
{
  assert(index < m_lines.size());
  const std::string& line=m_lines[index];
  size_t k = 0;
  bool inQuotes = 0;
  for(std::string::size_type i = 0;i < line.length();i++)
    {
      if (!inQuotes && line[i] == '\"')
	{
	  inQuotes = 1;
	  continue;
	}
      if (inQuotes && line[i] == '\"')
	{
	  if (i + 1 < line.length() && line[i + 1] == '\"')
	    {
	      i++;
	      continue;
	    }
	  inQuotes = 0;
	  continue;
	}
      if (!inQuotes && line[i] == ':')
	k++;
      assert(inQuotes || line[i] != '#');
    } //for();
  return k + 1;
}

std::string DelimitedFile::getItem(size_t lineIndex, size_t itemIndex) const
{
  assert(lineIndex < m_lines.size());
  const std::string& line = m_lines[lineIndex];
  std::string value;
  size_t k = 0;
  bool inQuotes = 0;
  for(std::string::size_type i = 0;i < line.length();i++)
    {
      assert(line[i] != '\n' && line[i] != '\r');
      if (!inQuotes && line[i] == '\"')
	{
	  inQuotes = 1;
	  continue;
	}
      if (inQuotes && line[i] == '\"')
	{
	  if (i + 1 < line.length() && line[i + 1] == '\"')
	    {
	      value += '\"';
	      i++;
	      continue;
	    }
	  inQuotes = 0;
	  continue;
	}
      if (inQuotes)
	{
	  value += line[i];
	  continue;
	}
      if (line[i] == ':')
	{
	  if (k == itemIndex)
	    return value;
	  value.erase();
	  k++;
	  continue;
	}
      value += line[i];
    } //for(i);
  if (k == itemIndex)
    return value;
  assert(0);
  return "";//just to reduce warning messages;
}

size_t DelimitedFile::getLineCount() const
{
  return m_lines.size();
}

std::string DelimitedFile::getRawLine(size_t index) const
{
  assert(index <= m_lines.size());
  return m_lines[index];
}
