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
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<vector>
#include<set>
#include<map>
#include<iostream>
#include<sstream>
#include"CmdArgsParser.h"
#include"vmstrings.h"
#include"vmclient.h"
#include<iconv.h>
#include"Transcoding.h"
#include<signal.h>

#define ERROR_PREFIX "voiceman:"

#define VOLUME_HEAD "volume"
#define PITCH_HEAD "pitch"
#define RATE_HEAD "rate"
#define PUNC_HEAD "punc"
#define FAMILY_HEAD "family"

CmdArg cmdLineParams[] = {
  {'f', "family", "NAME", "set voice family to NAME;"},
  {'h', "help", "", "show this help screen and exit;"},
  {'H', "host", "HOST[:PORT]", "connect to server via TCP/IP;"},
  {'p', "pitch", "VALUE", "set speech pitch to VALUE (using numbers from 0 to 100);"},
  {'P', "punc", "MODE", "set punctuation mode to MODE;"},
  {'r', "rate", "VALUE", "set speech rate to VALUE (using numbers from 0 to 100);"},
  {'S', "say", "", "say text and exit;"},
  {'s', "socket", "FILE_NAME", "connect to server via UNIX domain socket;"},
  {'q', "stop", "", "send stop command and exit (can be mixed with \'--say\');"},
  {'v', "volume", "VALUE", "set speech volume to VALUE (using numbers from 0 to 100)."},
  {' ', NULL, NULL, NULL}
};

class ConnectionAutoClosing
{
public:
  ConnectionAutoClosing(vm_connection_t con)
    : m_con(con) {}

  ~ConnectionAutoClosing()
  {
    if (m_con != VOICEMAN_BAD_CONNECTION)
      vm_close(m_con);
  }

private:
  vm_connection_t m_con;
}; //class ConnectionAutoClosing;

void sigPipeHandler(int r)
{
  std::cerr << ERROR_PREFIX << "connection was closed by foreign side" << std::endl;
  exit(EXIT_FAILURE);
}

CmdArgsParser cmdLine(cmdLineParams);
vm_connection_t con=VOICEMAN_BAD_CONNECTION;

void process(const std::string& line)
{
  if (line.empty())
    {
      vm_stop(con);
      return;
    }
  const std::wstring s = IO2WString(line);
  if (s.length() == 1)
    {
      vm_letter(con, (char*)encodeUTF8(s).c_str());
      return;
    }
  std::wstring p;
  if (stringBegins(s, IO2WString(VOLUME_HEAD), p))
    {
      p = trim(p);
      if (!checkTypeUnsignedInt(p))
	{
	  std::cout << ERROR_PREFIX << "\'" << p << "\' is not a valid volume value" << std::endl;
	  return;
	}
      const size_t pp = parseAsUnsignedInt(p);
      if (pp > 100)
	{
	  std::cout << ERROR_PREFIX << "argument must be a value from 0 to 100" << std::endl;
	  return;
	}
      vm_volume(con, (unsigned char)pp);
      return;
    }
  if (stringBegins(s, IO2WString(PITCH_HEAD), p))
    {
      p = trim(p);
      if (!checkTypeUnsignedInt(p))
	{
	  std::cout << ERROR_PREFIX << "\'" << p << "\' is not a valid pitch value" << std::endl;
	  return;
	}
      const size_t pp = parseAsUnsignedInt(p);
      if (pp > 100)
	{
	  std::cout << ERROR_PREFIX << "argument must be a value from 0 to 100" << std::endl;
	  return;
	}
      vm_pitch(con, (unsigned char)pp);
      return;
    }
  if (stringBegins(s, IO2WString(RATE_HEAD), p))
    {
      p = trim(p);
      if (!checkTypeUnsignedInt(p))
	{
	  std::cout << ERROR_PREFIX << "\'" << p << "\' is not a valid rate value" << std::endl;
	  return;
	}
      const size_t pp = parseAsUnsignedInt(p);
      if (pp > 100)
	{
	  std::cout << ERROR_PREFIX << "argument must be a value from 0 to 100" << std::endl;
	  return;
	}
      vm_rate(con, (unsigned char)pp);
      return;
    }
  if (stringBegins(s, IO2WString(PUNC_HEAD), p))
    {
      p = trim(toLower(p));
      if (p == L"all")
	vm_procmode(con, VOICEMAN_PROCMODE_ALL); else
      if (p == L"some")
	vm_procmode(con, VOICEMAN_PROCMODE_SOME); else
      if (p == L"none")
	vm_procmode(con, VOICEMAN_PROCMODE_NONE); else
	{
	  std::cout << ERROR_PREFIX << "\'" << p << "\' is not a valid punctuation mode. Please use \'all\', \'some\' or \'none\' values" << std::endl;
	  return;
	}
      return;
    }
  if (stringBegins(s, IO2WString(FAMILY_HEAD), p))
    {
      p = trim(p);
      if (p.empty ())
	{
	  std::cout << ERROR_PREFIX << "this command requires an argument" << std::endl;
	  return;
	}
      vm_family(con, VOICEMAN_LANG_NONE, (char*)encodeUTF8(p).c_str());
      return;
    }
  vm_text(con, (char*)encodeUTF8(s).c_str());
}

void printHelp()
{
  std::cout << "Shell client for VOICEMAN speech system. Version: " << PACKAGE_VERSION << "." << std::endl;
  std::cout << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << "\tvoiceman [OPTIONS] [SAY_COMMAND_TEXT]" << std::endl;
  std::cout << std::endl;
  std::cout << "Command line options:" << std::endl;
  cmdLine.printHelp();
}

int main(int argc, char* argv[])
{
  if (!cmdLine.parse(argc, argv))
    return 1;
  if (cmdLine.used("help"))
    {
    printHelp();
    return 0;
    }

  //Establishing connection;
  if (cmdLine.used("host"))
    {
      const std::string param = cmdLine["host"];
      std::string h=getDelimitedSubStr(param, 0, ':');
      const std::string p=getDelimitedSubStr(param, 1, ':');
      if (trim(h).empty())
	h="localhost";
      size_t port;
      if (!trim(p).empty())
	{
	  if (!checkTypeUnsignedInt(p))
	    {
	      std::cerr << ERROR_PREFIX << "\'" << trim(p) << "\' is not a valid port number." << std::endl;
	      return 1;
	    }
	  port = parseAsUnsignedInt(p);
	} else 
	port=VOICEMAN_DEFAULT_PORT;
      con = vm_connect_inet((char*)h.c_str(), port);
      if (con == VOICEMAN_BAD_CONNECTION)
	{
	  std::cerr << ERROR_PREFIX << "ERROR:Could not connect to host \'" << trim(h) << "\' with port " << port << "." << std::endl;
	  return 1;
	}
    } else
    if (cmdLine.used("socket"))
      {
	std::string p = cmdLine["socket"];
	if (trim(p).empty())
	  {
	    std::cerr << ERROR_PREFIX << "Missed name of UNIX domain socket." << std::endl;
	    return 1;
	  }
	if (trim(p) == "-")
	  p = VOICEMAN_DEFAULT_SOCKET;
	con = vm_connect_unix((char*)p.c_str());
	if (con == VOICEMAN_BAD_CONNECTION)
	  {
	    std::cerr << ERROR_PREFIX << "Could not connect to server via UNIX domain socket \'" << p << "\'." << std::endl;
	    return 1;
	  }
      } else
      {
	con = vm_connect();
	if (con == VOICEMAN_BAD_CONNECTION)
	  {
	    std::cerr << ERROR_PREFIX << "Could not connect to voicemand with default settings." << std::endl;
	    return 1;
	  }
      }

  ConnectionAutoClosing autoClosing(con);
  struct sigaction sa;
  sigaction(SIGPIPE, NULL, &sa);
  sa.sa_handler = sigPipeHandler;
  sa.sa_flags |= SA_RESTART;
  sigaction(SIGPIPE, &sa, NULL);



  //INitial connection parameters;
  assert(con != VOICEMAN_BAD_CONNECTION);
  if (cmdLine.used("stop"))
    vm_stop(con);
  if (cmdLine.used("pitch"))
    {
      const std::string value = trim(cmdLine["pitch"]);
      if (!checkTypeUnsignedInt(value))
	{
	  std::cerr << ERROR_PREFIX << "\'" << value << "\' is not a valid pitch value" << std::endl;
	  return 1;
	}
      vm_pitch(con, parseAsUnsignedInt(value));
    }
  if (cmdLine.used("rate"))
    {
      const std::string value = trim(cmdLine["rate"]);
      if (!checkTypeUnsignedInt(value))
	{
	  std::cerr << ERROR_PREFIX << "\'" << value << "\' is not a valid rate value" << std::endl;
	  return 1;
	}
      vm_rate(con, parseAsUnsignedInt(value));
    }
  if (cmdLine.used("volume"))
    {
      const std::string value = trim(cmdLine["volume"]);
      if (!checkTypeUnsignedInt(value))
	{
	  std::cerr << ERROR_PREFIX << "\'" << value << "\' is not a valid volume value" << std::endl;
	  return 1;
	}
      vm_volume(con, parseAsUnsignedInt(value));
    }
  if (cmdLine.used("family"))
    {
      //Neither trim() nor toLower() functions must be applied to teh value;
      //Let do it by the server itself;
      const std::string value = cmdLine["family"];
      if (trim(value).empty())
	{
	  std::cerr << ERROR_PREFIX << "voice family specification has an empty string value" << std::endl;
	  return 1;
	}
      for(std::string::size_type i = 0;i < value.length();i++)
	if (value[i] == ':' || value[i] == '\n')
	  {
	    std::cerr << ERROR_PREFIX << "voice family specification cannot contain \':\' and new line characters" << std::endl;
	    return 1;
	  }
      vm_family(con, VOICEMAN_LANG_NONE, (char*)value.c_str());
    }
  if (cmdLine.used("punc"))
    {
      const std::string value = trim(toLower(cmdLine["punc"]));
      if (value == "all")
	vm_procmode(con, VOICEMAN_PROCMODE_ALL); else
      if (value == "some")
	vm_procmode(con, VOICEMAN_PROCMODE_SOME); else
      if (value == "none")
	vm_procmode(con, VOICEMAN_PROCMODE_NONE); else
	{
	  std::cerr << ERROR_PREFIX << "punctuation mode can be only \'all\', \'some\' or \'none\'" << std::endl;
	  return 1;
	}
    }
  if (cmdLine.used("say"))
    {
      std::string value;
      for(size_t i = 0;i < cmdLine.files.size();i++)
	attachStringWithSpace(value, cmdLine.files[i]);
      vm_text(con, (char*)encodeUTF8(IO2WString(trim(value))).c_str());
      return 0;
    }
  if (cmdLine.used("stop"))
    return 0;

  std::cout << "VOICEMAN speech system. Version: " << PACKAGE_VERSION << "." << std::endl;
  std::cout << "Type \'quit\' or press Ctrl+D to leave this prompt." << std::endl;

  while(1)
    {
      std::string l;
      bool toQuit = 0;
      std::cout << "voiceman>";
      while(1)
	{
	  char c;
	  if (!std::cin.get(c))
	    {
	      toQuit = 1;
	      break;
	    }
	  if (c == '\r')
	    continue;
	  if (c == '\n')
	    break;
	  l += c;
	}
      if (toQuit || trim(toLower(l))  == "quit")
	break;
      process(l);
    }
  std::cout << std::endl;
  std::cout << "Bye!!!" << std::endl;
  return 0;
}
