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

#include"emacspeak.h"
#include"LineParser.h"

struct EmacspeakLine
{
  EmacspeakLine()
    : brLevel(0) {}

  std::string cmd;
  StringVector params;
  size_t brLevel;
}; //struct EmacspeakLine;

EmacspeakLine currentEmacspeakLine;
std::string traceFileName;

std::string describeCommand(const EmacspeakLine& line)
{
  std::string s = line.cmd;
  if (!line.params.empty())
    {
    s += " (";
    for(StringVector::size_type i = 0;i < line.params.size();i++)
      {
	s += "\'" + line.params[i] + "\'";
	if (i != line.params.size() - 1)
	  s += ", ";
      }
    s += ")";
    }
  return s;
}

void splitEmacspeakLine(const std::string& str, EmacspeakLine& line)
{
  size_t &level = line.brLevel;
  std::string next;
  bool toParams;
  //Initialization;
  if (level != 0)
    {
      if (!line.params.empty())
	{
	  next = line.params.back();
	  line.params.pop_back();
	  toParams = 1;
	} else
	{
	  next = line.cmd;
	  line.params.clear();
	  line.cmd.erase();
	  toParams = 0;
	}
    } /*if (level != 0)*/ else
    {
      line.cmd.erase();
      line.params.clear();
      toParams = 0;
    }
  //End of initialization, processing now;
  int state = level != 0?4:2;
  for(std::string::size_type i = 0;i < str.length();i++)
    {
      if (str[i] == ' ')
	{
	  if (state == 2)
	    continue;
	  if (state == 1)
	    {
	      state = 2;
	      continue;
	    }
	  if (state == 4)
	    {
	      next+=' ';
	      continue;
	    }
	  if (state == 0 || state == 5)
	    {
	      if (toParams)
		line.params.push_back(next); else
		  line.cmd = next;
	      next.erase();
	      toParams=1;
	      state = 1;
	      continue;
	    }
	  assert(0);
	} //space;
      if (str[i] == '{')
	{
	  if (state == 1 || state == 2)
	    {
	      next.erase();
	      state = 4;
	      level = 1;
	      continue;
	    }
	  if (state == 4)
	    {
	      next += str[i];
	      level++;
	      continue;
	    }
	  if (state == 0)
	    {
	      next += str[i]; 
	      continue;
	    }
	  continue;
	} // opening bracket;
      if (str[i] == '}')
	{
	  if (state == 4)
	    {
	      if (level == 1)
		{
		  state = 5;
		  level = 0;
		} else
		  if (level > 1)
		    {
		      next += str[i]; 
		      level--;
		    } else
		      {
			assert(0);
		      }
	      continue;
	    }
	  if (state == 0)
	    {
	      next += str[i];
	      continue;
	    }
	  continue;
	} // closing bracket;
      // all other characters;
      if (state == 0 || state == 4)
	{
	  next += str[i]; 
	  continue;
	}
      if (state == 1 || state == 2)
	{
	  next = str[i]; 
	  state = 0;
	}
    } // for();
  if (state == 0 || state == 4 || state== 5)
    {
      if (toParams)
	line.params.push_back(next); else
	  line.cmd = next;
    }
}

void handleLine(const std::string& line, LineParser& parser)
{
  if (line.empty())
    return;
  splitEmacspeakLine(line, currentEmacspeakLine);
  if (currentEmacspeakLine.brLevel > 0)
    return;
  if (!traceFileName.empty())
    {
      std::ofstream f(traceFileName.c_str(), std::ios_base::out | std::ios_base::app);
      if (f)
	f << describeCommand(currentEmacspeakLine) << std::endl;
    }
  parser.processCommand(currentEmacspeakLine.cmd, currentEmacspeakLine.params);
}

int readInput(int file, LineParser& parser)
{
  std::string line;
  while(1)
    {
      char buf[2048];
      const ssize_t readCount = read(file, buf, sizeof(buf));
      if (readCount < 0)
	{
	  perror("read()");
	  return EXIT_FAILURE;
	}
      if (readCount == 0)
	return EXIT_SUCCESS;
      for(size_t i = 0;i < (size_t)readCount;i++)
	{
	  if (buf[i] == '\n')
	    {
	      handleLine(line, parser);
	      line.erase();
	      continue;
	    }
	  if (buf[i] == '\r')
	    continue;
	  line += buf[i];
	} //for(buf);
    } //while(1);
}

void printHelp()
{
  std::cout << "Emacspeak client for Voiceman speech service. Version: " << PACKAGE_VERSION << "." << std::endl;
  std::cout << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << "\tvoiceman-emacspeak [--help]" << std::endl;
  std::cout << std::endl;
  std::cout << "Used environment variables:" << std::endl;
    std::cout << "\t$VOICEMAN - specifies server location;" << std::endl;
    std::cout << "\t$VOICEMAN_EMACSPEAK_FAMILY - sets default voice family for emacspeak connections;" << std::endl;
    std::cout << "\t$VOICEMAN_EMACSPEAK_RATE_FRACTION - fraction to control rate value range (default is 5);" << std::endl;
    std::cout << "\t$VOICEMAN_EMACSPEAK_RPITCH_SHIFT - the value to add to all pitch commands (default is 0);" << std::endl;
    std::cout << "\t$VOICEMAN_EMACSPEAK_TRACE - sets file to save emacspeak data." << std::endl;
      std::cout << std::endl;
      std::cout << "NOTE:Used protocol type: Dectalk." << std::endl;
}

std::string getEnv(const std::string& name)
{
  const char* v = getenv(name.c_str());
  if (!v)
    return "";
  return std::string(v);
}

int main(int argc, char *argv[])
{
  for(int k = 1;k < argc;k++)
    {
      const std::string param = trim(std::string(argv[k]));
      if (param == "--help" || param == "-h")
	{
	  printHelp();
	  return EXIT_SUCCESS;
	}
    } //for(argv);
  traceFileName = trim(getEnv("VOICEMAN_EMACSPEAK_TRACE"));
  LineParser parser;
  parser.setFamily(trim(getEnv("VOICEMAN_EMACSPEAK_FAMILY")));
  const std::string rateFraction = trim(getEnv("VOICEMAN_EMACSPEAK_RATE_FRACTION"));
  if (!rateFraction.empty() && checkTypeUnsignedInt(rateFraction))
    parser.setRateFraction(parseAsUnsignedInt(rateFraction));
  const std::string pitchShift = trim(getEnv("VOICEMAN_EMACSPEAK_PITCH_SHIFT"));
  if (!pitchShift.empty() && checkTypeInt(pitchShift))
    parser.setPitchShift(parseAsInt(pitchShift));
  return readInput(STDIN_FILENO, parser);
}
