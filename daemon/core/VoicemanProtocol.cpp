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

#include"voiceman.h"
#include"VoicemanProtocol.h"

void VoicemanProtocol::process(const std::wstring& s, Client& client)
{
  std::string infoStr = encodeUTF8(s);
  removeNewLineChars(infoStr);
  logMsg(LOG_DEBUG, "Protocol parser is parsing string \'%s\'", infoStr.c_str());
  wchar_t cmd;
  std::wstring arg;//probably argument must be parsed as std:;string;
  if (!split(s, cmd, arg))
    {
      logMsg(LOG_WARNING, "rejecting invalid client command: protocol parse error (line=\'%s\')", WString2IO(s).c_str());
      return;
    }
  switch(cmd)
    {
    case 'T':
      m_handler.onText(client, arg);
      break;
    case 'L':
      parseLetter(arg, client);
      break;
    case 'S':
      m_handler.onStop(client);
      break;
    case 'V':
      parseParam(ParamVolume, arg, client);
      break;
    case 'P':
      parseParam(ParamPitch, arg, client);
      break;
    case 'R':
      parseParam(ParamRate, arg, client);
      break;
    case 'B':
      parseTone(arg, client);
      break;
    case 'F':
      parseFamily(arg, client);
      break;
    case 'M':
      m_handler.onProcMode(client, encodeUTF8(arg));
      break;
    default:
      logMsg(LOG_WARNING, "Rejecting client command with unknown command code %d (line=\'%s\')", cmd, WString2IO(s).c_str());
    } //switch(cmd);
}

bool VoicemanProtocol::split(const std::wstring& s, wchar_t& cmd, std::wstring& arg)
{
  if (s.length() < 2)
    return 0;
  if (s[1] != ':')
    return 0;
  cmd=s[0];
  arg.erase();
  for(std::wstring::size_type i=2;i<s.length();i++)
    arg+=s[i];
  return 1;
}

void VoicemanProtocol::parseLetter(const std::wstring& value, Client& client)
{
  if (value.length() != 1)
    {
      logMsg(LOG_WARNING, "Argument of LETTER command has an invalid length. (arg=%s), ignoring...", WString2IO(value).c_str());
      return;
    }
  m_handler.onLetter(client, value[0]);
}

void VoicemanProtocol::parseParam(int paramType, const std::wstring& value, Client& client)
{
  if (!checkTypeUnsignedInt(value))
    {
      logMsg(LOG_WARNING, "Received illegal parameter value from client: \'%s\' is not an unsigned integer number, ignoring...", WString2IO(value).c_str());
      return;
    }
  TextParam p(parseAsUnsignedInt(value));
  m_handler.onParam(client, paramType, p);
}

void VoicemanProtocol::parseTone(const std::wstring& value, Client& client)
{
  std::wstring p1, p2;
  p1=trim(getDelimitedSubStr(value, 0, ':'));
  p2=trim(getDelimitedSubStr(value, 1, ':'));
  if (p1.empty() || p2.empty())
    {
      logMsg(LOG_WARNING, "Missed one of the parameters of the \'TONE\' command. received from client, ignoring...");
      return;
    }
  if (!checkTypeUnsignedInt(p1) || !checkTypeUnsignedInt(p2))
    {
      logMsg(LOG_WARNING, "\'TONE\' command from client has invalid parameters (\'%s\' and \'%s\').", WString2IO(p1).c_str(), WString2IO(p2).c_str());
      return;
    }
  m_handler.onTone(client, parseAsUnsignedInt(p1), parseAsUnsignedInt(p2));
}

void VoicemanProtocol::parseFamily(const std::wstring& arg, Client& client)
{
  const std::string value  = encodeUTF8(arg);
  bool wasSemi = 0;
  std::string lang, family;
  for(std::string::size_type i = 0;i < value.size();i++)
    {
      if (value [i] == ':')
	{
	  wasSemi = 1;
	  continue;
	}
      if (wasSemi)
	lang += value[i]; else
	family += value[i];
    }
  if (wasSemi)
    m_handler.onFamily(client, "", family); else
    m_handler.onFamily(client, lang, family);
}
