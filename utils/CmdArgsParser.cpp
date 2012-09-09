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

#include<assert.h>
#include<string>
#include<vector>
#include<iostream>
#include"CmdArgsParser.h"

CmdArgsParser::CmdArgsParser(CmdArg* allParams)
  :files(m_files), m_availableParams(allParams), m_availableParamCount(0)
{
  while(m_availableParams[m_availableParamCount].shortName != ' ')
    m_availableParamCount++;
}

void CmdArgsParser::printHelp() const
{
  StringVector keys;
  for(size_t i = 0;i < m_availableParamCount;i++)
    {
      std::string s;
      bool hasParam = (m_availableParams[i].param != NULL && m_availableParams[i].param[0] != '\0');
      s = "  -";
      s += m_availableParams[i].shortName;
      if (hasParam)
	{
	  s += " ";
	  s += m_availableParams[i].param;
	}
      s += ", --";
      s += m_availableParams[i].longName;
      if (hasParam)
	{
	  s += " ";
	  s += m_availableParams[i].param;
	}
      keys.push_back(s);
    }
  std::string::size_type l = 0;
  for(StringVector::size_type i = 0;i < keys.size();i++)
    if (l < keys[i].length())
      l = keys[i].length();
  assert(keys.size() == m_availableParamCount);
  for(size_t i = 0;i < m_availableParamCount;i++)
    {
      using namespace std;
      std::string descr = m_availableParams[i].descr;
      cout << keys[i];
      for(std::string::size_type j = keys[i].length();j < l;j++)
	cout << " ";
      cout << " - " << descr << endl;
    }
}

bool CmdArgsParser::used(const std::string& name) const
{
  for(UsedParamVector::size_type i = 0;i < m_usedParams.size();i++)
    if (m_usedParams[i].name == name)
      return 1;
  return 0;
}

const std::string CmdArgsParser::operator[](const std::string& name) const
{
  assert(used(name));
  UsedParamVector::size_type i;
  for(i = 0; i < m_usedParams.size(); i++)
    if (m_usedParams[i].name == name)
      break;
  assert(i < m_usedParams.size() && m_usedParams[i].hasValue);
  return m_usedParams[i].value;
}

/*
 * This method returns:
 * -1 - if it is not a known parameter;
 * -2 - if it is not a valid command line item.
 */
size_t CmdArgsParser::identifyParam(char* p) const
{
  std::string s = p;
  if (s.length() < 2)
    return (size_t)-1;
  if (s[0] != '-')
    return (size_t)-1;
  if (s[1] != '-')
    {
      if (s.length() != 2)
	return (size_t)-1;
      for(size_t i = 0;i < m_availableParamCount;i++)
	if (s[1]  == m_availableParams[i].shortName)
	  return i;
      return (size_t)-2;
    }
  std::string ss;
  for(std::string::size_type i = 2;i < s.length();i++)
    ss += s[i];
  for(size_t i = 0;i < m_availableParamCount;i++)
    {
      std::string longName = m_availableParams[i].longName;
      if (ss == longName)
	return i;
    }
  return -2;
}

bool CmdArgsParser::parse(int argc, char* argv[])
{
  if (argc < 1)
    {
      std::cerr << "error: Too few arguments in argv[] array." << std::endl;
      return 0;
    }
  for(int i = 1;i < argc;i++)
    {
      size_t p = identifyParam(argv[i]);
      if (p == (size_t)-2)
	{
	  std::cerr << "Error:Unknown command line parameter \'" << argv[i] << "\'." << std::endl;
	  return 0;
	}
      if (p == (size_t)-1)
	{
	  m_files.clear();
	  for(int z = i;z < argc;z++)
	    m_files.push_back(argv[z]);
	  return 1;
	} //it was first file name (not a known argument);
      USEDPARAM up;
      up.name = m_availableParams[p].longName;
      if (m_availableParams[p].param[0])
	{
	  if (i + 1 >= argc)
	    {
	      std::cerr << "error: Argument \'" << argv[i] << "\' requires a additional parameter \'" << m_availableParams[p].param << "\', but it is last item in command line. Be careful." << std::endl;
	      return 0;
	    }
	  i++;
	  up.value = argv[i];
	  up.hasValue = 1;
	}
      m_usedParams.push_back(up);
    } //for(argv);
  return 1;
}
