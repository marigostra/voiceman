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

#ifndef __VOICEMAN_CONFIGURATION_H__
#define __VOICEMAN_CONFIGURATION_H__

#include"CmdArgsParser.h"
#include"ConfigurationException.h"
#include"langs/LangManager.h"

struct OutputParamConfiguration
{
  OutputParamConfiguration()
    : numDigitsAfterDot(2), min(0), max(1), aver(0.5) {}

  size_t numDigitsAfterDot;
  double min, max, aver;
}; //struct OutputParamConfiguration;

struct OutputConfiguration
{
  OutputConfiguration()
    : langId(LANG_ID_NONE) {}

  std::string name, family;
  LangId langId;
  std::string synthCommand, alsaPlayerCommand, pulseaudioPlayerCommand, pcspeakerPlayerCommand;
  std::string replacementsFileName;
  WCharToWStringMap capList;
  OutputParamConfiguration volume, pitch, rate;
}; //struct OutputConfiguration;

typedef std::vector<OutputConfiguration> OutputConfigurationVector;
typedef std::list<OutputConfiguration> OutputConfigurationList;

struct Configuration 
{
  //logging;
  int logLevel;
  std::string logFileName;

  //sockets;
  std::string unixDomainSocketFileName; //empty means not used;
  bool useInetSocket;
  size_t inetSocketPort;

  //limits;
  size_t maxClients; //zero means not limited;
  size_t maxInputLine; //zero means not limited;
  size_t maxQueueSize; //zero means not limited;

  //text processing;
  int digitsMode;//constants are defined in core/AbstractTextProcessing.h;
  bool capitalization;
  bool separation;
  LangId defaultLangId;
  LangIdToWStringMap characters;

  //Playback;
  std::string executor;
  PlayerType playerType;

  //startup;
  bool daemonMode;
  std::string pidFileName;
  std::wstring sayModeText;
  std::wstring startUpMessage;
  bool testConfiguration;

  //Outputs;
  OutputConfigurationVector outputs;
  LangIdToStringMap defaultFamilies;
}; //struct Configuration;

void initConfigData(Configuration& c);
void prepareConfiguration(const LangManager& langs, const CmdArgsParser& cmdLine, Configuration& c);
void printConfiguration(const LangManager& langs, const Configuration& c);

#endif //__VOICEMAN_CONFIGURATION_H__
