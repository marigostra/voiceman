/*
	Copyright (c) 2000-2016 Michael Pozhidaev<michael.pozhidaev@gmail.com>
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
#include"configuration.h"
#include"config_file/ConfigFile.h"
#include"core/AbstractTextProcessor.h"//only for digits mode constants;

#define VMC_STOP(x) throw ConfigurationException(x)

class VoicemanConfigFile: public ConfigFile
{
public:
  VoicemanConfigFile() {}
  virtual ~VoicemanConfigFile() {}

  int params(const std::string& section, const std::string& param) const;
}; //class VoicemanConfigFile;

VOICEMAN_BEGIN_PARAM_TABLE(VoicemanConfigFile)

//Gloabl section;
VOICEMAN_DECLARE_PARAM("global", "startupmessage");
VOICEMAN_DECLARE_STRING_PARAM("global", "loglevel");
VOICEMAN_DECLARE_STRING_PARAM("global", "logfile");

VOICEMAN_DECLARE_STRING_PARAM("global", "socket");
VOICEMAN_DECLARE_UINT_PARAM("global", "inetsocketport");

VOICEMAN_DECLARE_UINT_PARAM("global", "maxclients");
VOICEMAN_DECLARE_UINT_PARAM("global", "maxinputline");
VOICEMAN_DECLARE_UINT_PARAM("global", "maxqueuesize");

VOICEMAN_DECLARE_STRING_PARAM("global", "digitsmode");
VOICEMAN_DECLARE_STRING_PARAM("global", "defaultlanguage");
VOICEMAN_DECLARE_BOOLEAN_PARAM("global", "separation");
VOICEMAN_DECLARE_BOOLEAN_PARAM("global", "capitalization");
VOICEMAN_DECLARE_BOOLEAN_PARAM("global", "lettersatminrate");

//output;
VOICEMAN_DECLARE_STRING_PARAM("output", "name");
VOICEMAN_DECLARE_STRING_PARAM("output", "lang");
VOICEMAN_DECLARE_STRING_PARAM("output", "type");
VOICEMAN_DECLARE_STRING_PARAM("output", "synthcommand");
VOICEMAN_DECLARE_STRING_PARAM("output", "alsaplayercommand");
VOICEMAN_DECLARE_STRING_PARAM("output", "pulseaudioplayercommand");
VOICEMAN_DECLARE_STRING_PARAM("output", "pcspeakerplayercommand");
VOICEMAN_DECLARE_STRING_PARAM("output", "replacements");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "pitchmin");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "pitchmax");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "pitchaver");
VOICEMAN_DECLARE_UINT_PARAM("output", "pitchnumdigitsafterdot");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "ratemin");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "ratemax");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "rateaver");
VOICEMAN_DECLARE_UINT_PARAM("output", "ratenumdigitsafterdot");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "volumemin");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "volumemax");
VOICEMAN_DECLARE_DOUBLE_PARAM("output", "volumeaver");
VOICEMAN_DECLARE_UINT_PARAM("output", "volumenumdigitsafterdot");
VOICEMAN_DECLARE_STRING_PARAM("output", "caplist");

//Playback;
VOICEMAN_DECLARE_STRING_PARAM("playback", "executor");
VOICEMAN_DECLARE_STRING_PARAM("playback", "player");

VOICEMAN_RELAX_SECTION("characters");
VOICEMAN_RELAX_SECTION("families");

VOICEMAN_END_PARAM_TABLE

void initConfigData(Configuration& c)
{
  c.logLevel = LOG_WARNING;
  c.logFileName = "syslog";
  c.unixDomainSocketFileName = "";
  c.useInetSocket = 0;
  c.inetSocketPort = VOICEMAN_DEFAULT_PORT;
  c.maxClients = 16;
  c.maxInputLine = 2048;
  c.maxQueueSize = 128;
  c.digitsMode = DigitsModeNormal;
  c.separation = 1;
  c.capitalization = 1;
  c.lettersAtMinRate = 0;
  c.defaultLangId = LANG_ID_NONE;
  c.daemonMode = 0;
  c.pidFileName = "";
  c.sayModeText = L"";
  c.startUpMessage = L"";
  c.testConfiguration = 0;
  c.executor = VOICEMAN_DEFAULT_EXECUTOR;
  c.playerType = PlayerTypeAlsa;
  c.outputs.clear();
  c.characters.clear();
}

static void processDoubleParameter(const ConfigFileSection& section, double& value, const std::string& sectionParamName, const std::string& paramName, const std::string& outputName)
{
  if (!section.has(sectionParamName))
    return;
  if (!checkTypeDouble(section[sectionParamName]))
    VMC_STOP("Parameter \'" + paramName + "\' for output \'" + outputName + "\' has invalid value; \'" + section[sectionParamName] + "\' is not a double value");
  value = parseAsDouble(section[sectionParamName]);
}

static void processUnsignedIntParameter(const ConfigFileSection& section, size_t& value, const std::string& sectionParamName, const std::string& paramName, const std::string& outputName)
{
  if (!section.has(sectionParamName))
    return;
  if (!checkTypeUnsignedInt(section[sectionParamName]))
    VMC_STOP("Parameter \'" + paramName + "\' for output \'" + outputName + "\' has invalid value; \'" + section[sectionParamName] + "\' is not a integer number value");
  value = parseAsUnsignedInt(section[sectionParamName]);
}

static int convertLogLevel(const std::string& logLevel)
{
  if (trim(logLevel).empty())
    VMC_STOP("Log level specification contains an empty string");
  std::string l = toLower(trim(logLevel));
  if (l == "fatal")
    return LOG_CRIT;
  if (l == "error")
    return LOG_ERR;
  if (l == "warn")
    return LOG_WARNING;
  if (l == "info")
    return LOG_INFO;
  if (l == "debug")
    return LOG_DEBUG;
  VMC_STOP("Used unknown log level identifier \'" + l + "\'");
  return 0;
}

static void processOutputsConfiguration(const LangManager& langs, const VoicemanConfigFile& config, Configuration& c)
{
  for(ConfigFileSectionVector::size_type i = 0;i < config.getSectionCount();i++)
    {
      const ConfigFileSection& section = config.getSection(i);
      if (trim(toLower(section.getName())) != "output")
	continue;
      OutputConfiguration outputConfiguration;
      if (!section.has("name"))
	VMC_STOP("There is output without name in your configuration file");
      outputConfiguration.name = trim(toLower(section["name"]));
      if (!section.has("type"))
	VMC_STOP("Output \'" + outputConfiguration.name + "\' has no \'type\' parameter");
      if (trim(toLower(section["type"])) != "command")
	VMC_STOP("Output \'" + outputConfiguration.name + "\' has unsupported type \'" + section["type"] + "\'");
      if (!section.has("lang"))
	VMC_STOP("Output \'" + outputConfiguration.name + "\' has no \'lang\' parameter");
      const std::string lang = trim(toLower(section["lang"]));
      if (!langs.hasLanguage(lang))
	VMC_STOP("Output \'" + outputConfiguration.name + "\' has unknown language \'" + lang + "\'");
      outputConfiguration.langId = langs.getLangId(lang);
      assert(outputConfiguration.langId != LANG_ID_NONE);
      if (!section.has("synthcommand"))
	VMC_STOP("Output \'" + outputConfiguration.name + "\' has no \'synth command\' parameter");
      outputConfiguration.synthCommand = trim(section["synthcommand"]);
      if (outputConfiguration.synthCommand.empty())
	VMC_STOP("Output \'" + outputConfiguration.name + "\' has empty synthesizer command");
      if (section.has("replacements"))
	{
	  std::string replacements = section["replacements"];
	  if (trim(replacements).empty())
	    VMC_STOP("Output \'" + outputConfiguration.name + "\' has empty \'replacements\' parameter");
	  if (replacements[0] != '/')
	    replacements = concatUnixPath<std::string>(VOICEMAN_DATADIR, replacements);
	  outputConfiguration.replacementsFileName = replacements;
	}
      if (section.has("alsaplayercommand"))
	outputConfiguration.alsaPlayerCommand = trim(section["alsaplayercommand"]);
      if (section.has("pulseaudioplayercommand"))
	outputConfiguration.pulseaudioPlayerCommand = trim(section["pulseaudioplayercommand"]);
      if (section.has("pcspeakerplayercommand"))
	outputConfiguration.pcspeakerPlayerCommand = trim(section["pcspeakerplayercommand"]);
      //pitch;
      processDoubleParameter(section, outputConfiguration.pitch.min, "pitchmin", "pitch min", outputConfiguration.name);
      processDoubleParameter(section, outputConfiguration.pitch.max, "pitchmax", "pitch max", outputConfiguration.name);
      processDoubleParameter(section, outputConfiguration.pitch.aver, "pitchaver", "pitch aver", outputConfiguration.name);
      processUnsignedIntParameter(section, outputConfiguration.pitch.numDigitsAfterDot, "pitchnumdigitsafterdot", "pitch num digits after dot", outputConfiguration.name);
      if (outputConfiguration.pitch.numDigitsAfterDot >= 10)
	VMC_STOP("Parameter \'pitch num digits after dot\' must be integer number in range from 0 to 9; Value \'" + section["pitchnumdigitsafterdot"] + "\' for output \'" + outputConfiguration.name + "\' is illegal;");
      //rate;
      processDoubleParameter(section, outputConfiguration.rate.min, "ratemin", "rate min", outputConfiguration.name);
      processDoubleParameter(section, outputConfiguration.rate.max, "ratemax", "rate max", outputConfiguration.name);
      processDoubleParameter(section, outputConfiguration.rate.aver, "rateaver", "rate aver", outputConfiguration.name);
      processUnsignedIntParameter(section, outputConfiguration.rate.numDigitsAfterDot, "ratenumdigitsafterdot", "rate num digits after dot", outputConfiguration.name);
      if (outputConfiguration.rate.numDigitsAfterDot >= 10)
	VMC_STOP("Parameter \'rate num digits after dot\' must be integer number in range from 0 to 9; Value \'" + section["ratenumdigitsafterdot"] + "\' for output \'" + outputConfiguration.name + "\' is illegal;");
      //volume;
      processDoubleParameter(section, outputConfiguration.volume.min, "volumemin", "volume min", outputConfiguration.name);
      processDoubleParameter(section, outputConfiguration.volume.max, "volumemax", "volume max", outputConfiguration.name);
      processDoubleParameter(section, outputConfiguration.volume.aver, "volumeaver", "volume aver", outputConfiguration.name);
      processUnsignedIntParameter(section, outputConfiguration.volume.numDigitsAfterDot, "volumenumdigitsafterdot", "volume num digits after dot", outputConfiguration.name);
      if (outputConfiguration.volume.numDigitsAfterDot >= 10)
	VMC_STOP("Parameter \'volume num digits after dot\' must be integer number in range from 0 to 9; Value \'" + section["volumenumdigitsafterdot"] + "\' for output \'" + outputConfiguration.name + "\' is illegal;");
      if (section.has("caplist"))
	{
	  const std::wstring value = trim(readUTF8(section["caplist"]));
	  StringDelimitedIterator<std::wstring> it(value, L" ");
	  while(it.next())
	    {
	      const std::wstring s1 = it.str();
	  if (!it.next())
	    VMC_STOP("Incomplete pair in parameter \'cap list\' in section \'" + outputConfiguration.name + "\'");
	  const std::wstring s2 = it.str();
	  if (s1.length() != 1)
	    VMC_STOP("Output \'" + outputConfiguration.name + "\' has invalid pair item in parameter \'cap list\'; string \'" + encodeUTF8(s1) + "\' must contain only the single character");
	  outputConfiguration.capList.insert(WCharToWStringMap::value_type(s1[0], s2));
	    }
	}
      c.outputs.push_back(outputConfiguration);
    } //for(output);
  //Checking if output set is valid;
  if (c.outputs.empty())
    VMC_STOP("No outputs are defined in your configuration, nothing to process");
  //Dublicated output name checking;
  for(OutputConfigurationVector::size_type i = 0;i < c.outputs.size();i++)
    for(OutputConfigurationVector::size_type j = i + 1;j < c.outputs.size();j++)
      if (trim(toLower(c.outputs[i].name)) == trim(toLower(c.outputs[j].name)))
	VMC_STOP("There are more than one output with name \'" + c.outputs[i].name + "\' in your configuration file");
}

static void processFamiliesConfiguration(const LangManager& langs, const VoicemanConfigFile& config, Configuration& c)
{
  if (config.hasSection("families"))
    {
      const ConfigFileSection& sec = config.findSection("families");
      for(ConfigFileSection::const_iterator it = sec.begin();it != sec.end();it++)
	{
	  const std::string name = trim(toLower(it->first));
	  const std::string value = trim(toLower(it->second));
	  std::string langName;
	  if (stringBegins<std::string>(name, "default", langName))
	    {
	      langName = trim(toLower(langName));
	      if (!langs.hasLanguage(langName))
		VMC_STOP("Parameter \'" + name + "\' is not valid for section \'families\'");
	      const LangId langId = langs.getLangId(langName);
	      if (c.defaultFamilies.find(langId) != c.defaultFamilies.end())
		VMC_STOP("Parameter \'default " + langName + "\' is specified more than one time in section \'families\'");
	      c.defaultFamilies.insert(LangIdToStringMap::value_type(langId, value));
	      continue;
	    } //default family specification;
	  //Output -> family mapping;
	  OutputConfigurationVector::size_type outputIndex;
	  for(outputIndex = 0;outputIndex < c.outputs.size();outputIndex++)
	    if (c.outputs[outputIndex].name == name)
	      break;
	  if (outputIndex >= c.outputs.size())
	    VMC_STOP("There is no output with name \'" + name + "\' to match it with the parameter in section \'families\'");
	  assert(outputIndex < c.outputs.size());
	  if (!c.outputs[outputIndex].family.empty())
	    VMC_STOP("There are more than one family specifications for output \'" + c.outputs[outputIndex].name + "\'");
	  c.outputs[outputIndex].family = value;
	}//for all parameters of section "families";
    }//has section "families";
  //Family values adjusting and checking;
  for(OutputConfigurationVector::size_type i = 0;i < c.outputs.size();i++)
    if (trim(c.outputs[i].family).empty())
      c.outputs[i].family = c.outputs[i].name;
  //There can not be more than one output with same family and language;
  for(OutputConfigurationVector::size_type i = 0;i < c.outputs.size();i++)
    for(OutputConfigurationVector::size_type j = i + 1;j < c.outputs.size();j++)
      if (c.outputs[i].family == c.outputs[j].family && c.outputs[i].langId == c.outputs[j].langId)
	VMC_STOP("Outputs \'" + c.outputs[i].name + "\' and \'" + c.outputs[j].name + "\' could not have same family");
  ////Checking if we have enough information to select default family;
  StringList supportedLanguages;
  langs.getSupportedLanguageNames(supportedLanguages);
  for(StringList::const_iterator it = supportedLanguages.begin();it != supportedLanguages.end();it++)
    {
      const LangId langId = langs.getLangId(*it);
      assert(langId != LANG_ID_NONE);
      //First of all be must count outputs with language langId;
      size_t count = 0;
      std::string family;
      for(OutputConfigurationVector::size_type i = 0;i < c.outputs.size();i++)
	if (c.outputs[i].langId == langId)
	  {
	    count++;
	    family = c.outputs[i].family;
	  }
      if (count > 0)
	{
	  LangIdToStringMap::const_iterator mapIt = c.defaultFamilies.find(langId);
	  if (mapIt != c.defaultFamilies.end())
	    {
	      //default entry present, we must be sure it is valid;
	      OutputConfigurationVector::size_type outputIndex;
	      for(outputIndex = 0;outputIndex < c.outputs.size();outputIndex++)
		if (c.outputs[outputIndex].langId == langId && c.outputs[outputIndex].family == mapIt->second)
		  break;
	      if (outputIndex >= c.outputs.size())
		VMC_STOP("Default family for language \'" + *it + "\' is not valid");
	      //OK, now we sure there is proper default family for langId;
	    } else 
	    {
	      if (count > 1)
		VMC_STOP("There are more than one output for language \'" + *it + "\' but none of them is selected as default");
	      //There is just one output for langId and we can successfully select it as default;
	      c.defaultFamilies.insert(LangIdToStringMap::value_type(langId, family));
	    } //default entry is absent;
	} else//there are outputs with language equal to langId;
	{
	  if (c.defaultFamilies.find(langId) != c.defaultFamilies.end())
	    VMC_STOP("Default family for language \'" + *it + " is selected, but there are no outputs for this language");
	} //there is no entries with language equal to langId;
    } //for(supportedLanguages;)
}

static void processPlaybackConfiguration(const CmdArgsParser& cmdLine, const VoicemanConfigFile& config, Configuration& c)
{
  std::string executor, player;
  if (config.hasSection("playback"))
    {
      const ConfigFileSection& sec = config.findSection("playback");
      if (sec.has("executor"))
	executor = sec["executor"];
      if (sec.has("player"))
	player = sec["player"];
    }
  if (cmdLine.used("executor"))
    executor = cmdLine["executor"];
  if (cmdLine.used("player"))
    player = cmdLine["player"];
  if (!trim(executor).empty())
    c.executor = executor;
  if (!trim(player).empty())
    {
      player = trim(toLower(player));
      if (player == "alsa")
	c.playerType = PlayerTypeAlsa; else
      if (player == "pulseaudio")
	c.playerType = PlayerTypePulseaudio; else
      if (player == "pcspeaker")
	c.playerType = PlayerTypePcspeaker; else
	VMC_STOP("Unknown player type \'" + player + "\'");
    }
}

static void processCharactersSection(const LangManager& langs, const VoicemanConfigFile& config, Configuration& c)
{
  if (config.hasSection("characters"))
    {
      const ConfigFileSection& sec = config.findSection("characters");
      for(ConfigFileSection::const_iterator it = sec.begin();it != sec.end();it++)
	{
	  const std::string langName = trim(toLower(it->first));
	  LangId langId = LANG_ID_NONE;
	  if (langName != "default")
	    {
	      if (!langs.hasLanguage(langName))
		VMC_STOP("There are no language \'" + it->first + "\' used in section \'characters\'");
	      langId = langs.getLangId(langName);
	      assert(langId != LANG_ID_NONE);
	    }
	  c.characters.insert(LangIdToWStringMap::value_type(langId, readUTF8(it->second)));
	}
    }
}

void prepareConfiguration(const LangManager& langs, const CmdArgsParser& cmdLine, Configuration& c)
{
  //Where we should get our configuration?;
  std::string configFileName = VOICEMAN_DEFAULT_CONFIG;
  if (cmdLine.used("config"))
    configFileName = cmdLine["config"];
  if (trim(configFileName).empty())
    VMC_STOP("Configuration file name is empty");
  VoicemanConfigFile config;
  config.load(configFileName);
  config.checkParams();
  //OK, configuration file loaded, parsed and all data types were checked;
  // We cannot work without 'Global' section;
  if (!config.hasSection("global"))
    VMC_STOP("Configuration file does not contain \'global\' section");
  //General parameters processing ('Global' section);
  const ConfigFileSection& global = config.findSection("global");
  if (cmdLine.used("loglevel"))
    c.logLevel = convertLogLevel(cmdLine["loglevel"]); else
    if (global.has("loglevel"))
      c.logLevel = convertLogLevel(global["loglevel"]);
  c.daemonMode = cmdLine.used("daemon");
  c.testConfiguration = cmdLine.used("test");
  if (cmdLine.used("pidfile"))
    c.pidFileName = cmdLine["pidfile"];
  if (cmdLine.used("say"))
    c.sayModeText = trim(readUTF8(cmdLine["say"]));
  if (cmdLine.used("message"))
    c.startUpMessage = readUTF8(cmdLine["message"]); else 
    if (global.has("startupmessage"))
      c.startUpMessage = readUTF8(global["startupmessage"]);
  if (cmdLine.used("socket"))
    c.unixDomainSocketFileName = cmdLine["socket"]; else
    if (global.has("socket"))
      c.unixDomainSocketFileName = global["socket"];
  if (cmdLine.used("port"))
    {
      const std::string& value = cmdLine["value"];
      if (!checkTypeUnsignedInt(value))
	VMC_STOP("Illegal port number for inet socket \'" + value + "\'");
      c.useInetSocket = 1;
      c.inetSocketPort = parseAsUnsignedInt(value);
    } else
    if (global.has("inetsocketport"))
      {
	c.useInetSocket = 1;
	c.inetSocketPort = parseAsUnsignedInt(global["inetsocketport"]);
      }
  if (c.useInetSocket && c.inetSocketPort >= 65536)
    VMC_STOP("Inet socket port too large (" + trim(global["inetsocketport"]) + " >= 65536)");
  if (global.has("logfile"))
    c.logFileName = global["logfile"];  if (global.has("maxclients"))
    c.maxClients = parseAsUnsignedInt(global["maxclients"]);
  if (global.has("maxinputline"))
    c.maxInputLine = parseAsUnsignedInt(global["maxinputline"]);
  if (global.has("maxqueuesize"))
    c.maxQueueSize = parseAsUnsignedInt(global["maxqueuesize"]);
  if (global.has("digitsmode"))
    {
      std::string value = trim(toLower(global["digitsmode"]));
      if (value == "normal")
	c.digitsMode = DigitsModeNormal; else
	if (value == "none")
	  c.digitsMode = DigitsModeNone; else 
	  if (value == "single")
	    c.digitsMode = DigitsModeSingle; else 
	    VMC_STOP("Digits mode parameter has an invalid value \'" + value + "\'");
    } //digits mode;
  if (global.has("capitalization"))
    c.capitalization = parseAsBool(global["capitalization"]);
  if (global.has("lettersatminrate"))
    c.lettersAtMinRate = parseAsBool(global["lettersatminrate"]);
  if (global.has("separation"))
    c.separation = parseAsBool(global["separation"]);
  if (global.has("defaultlanguage"))
    {
      const std::string value = trim(toLower(global["defaultlanguage"]));
      if (!langs.hasLanguage(value))
	VMC_STOP("There is no language \'" + value + "\' to be used as default language");
      c.defaultLangId = langs.getLangId(value);
      assert(c.defaultLangId != LANG_ID_NONE);
    }
  processOutputsConfiguration(langs, config, c);
  processFamiliesConfiguration(langs, config, c);
  processPlaybackConfiguration(cmdLine, config, c);
  processCharactersSection(langs, config, c);
}

static std::string logLevelToString(int logLevel)
{
  if (logLevel == LOG_CRIT)
    return "FATAL";
  if (logLevel == LOG_ERR)
    return "ERROR";
  if (logLevel == LOG_WARNING)
    return "WARN";
  if (logLevel == LOG_INFO)
    return "INFO";
  if (logLevel == LOG_DEBUG)
    return "DEBUG";
  assert(0);
  return "";
}

static std::string boolToString(bool value)
{
  if (value)
    return "YES";
  return "no";
}

static void printOutputParamConfiguration(const OutputParamConfiguration& params, const std::string& name)
{
  std::cout << name << " min = " << params.min << std::endl;
  std::cout << name << " max = " << params.max << std::endl;
  std::cout << name << " aver = " << params.aver << std::endl;
  std::cout << name << " num digits after dot = " << params.numDigitsAfterDot << std::endl;
}

static void printOutputsConfiguration(const LangManager& langs, const Configuration& c)
{
  for(OutputConfigurationVector::size_type i = 0;i < c.outputs.size();i++)
    {
      const OutputConfiguration& o = c.outputs[i];
      std::cout << std::endl;
      std::cout  << "Output \'" << o.name << "\' attributes:" << std::endl;
      std::cout << "type = command" << std::endl;
      std::cout << "lang = " << langs.getLangName(o.langId) << std::endl;
      std::cout << "family = " << o.family << std::endl;
      std::cout << "synth command = " << o.synthCommand << std::endl;
      std::cout << "alsa player command = " << o.alsaPlayerCommand << std::endl;
      std::cout << "pulseaudio player command = " << o.pulseaudioPlayerCommand << std::endl;
      std::cout << "pc speaker player command = " << o.pcspeakerPlayerCommand << std::endl;
      std::cout << "replacements = " << o.replacementsFileName << std::endl;
      printOutputParamConfiguration(o.pitch, "pitch");
      printOutputParamConfiguration(o.rate, "rate");
      printOutputParamConfiguration(o.volume, "volume");
      if (!o.capList.empty())
	{
	  std::cout << "cap list = ";
	  for(WCharToWStringMap::const_iterator it = o.capList.begin();it != o.capList.end();it++)
	    {
	      std::wstring s1;
	      s1 += it->first;
	      std::cout << "\'" << s1 << "\' -> \'" << it->second << "\'; ";
	    }
	  std::cout << std::endl;
	}
    } //for(outputs);
}

void printConfiguration(const LangManager& langs, const Configuration& c)
{
  std::cout << "Predefined settings:" << std::endl;
  std::cout << "default configuration file = " << VOICEMAN_DEFAULT_CONFIG << std::endl;
  std::cout << "default socket =" << VOICEMAN_DEFAULT_SOCKET << std::endl;
  std::cout << "default port = " << VOICEMAN_DEFAULT_PORT << std::endl;
  std::cout << "data directory = " << VOICEMAN_DATADIR << std::endl;
  std::cout << "default executor = " << VOICEMAN_DEFAULT_EXECUTOR << std::endl;
    std::cout << std::endl;
  std::cout << "Global attributes:" << std::endl;
  std::cout << "log level = " << logLevelToString(c.logLevel) << std::endl;
  std::cout << "log file = " << c.logFileName << std::endl;
  std::cout << "socket = " << c.unixDomainSocketFileName << std::endl;
  std::cout << "use inet socket = " << boolToString(c.useInetSocket) << std::endl;
  std::cout << "inet socket port = " << c.inetSocketPort << std::endl;
  std::cout << "max clients = " << c.maxClients << (c.maxClients != 0?"":" (not limited)") << std::endl;
  std::cout << "max input line = " << c.maxInputLine << (c.maxInputLine != 0?"":" (not limited)") << std::endl;
  std::cout << "max queue size = " << c.maxQueueSize << (c.maxQueueSize != 0?"":" (not limited)") << std::endl;
  std::cout << "digits mode = ";
  switch(c.digitsMode)
    {
    case DigitsModeNormal:
      std::cout << "NORMAL";
      break;
    case DigitsModeNone:
      std::cout << "NONE";
      break;
    case DigitsModeSingle:
      std::cout << "SINGLE";
      break;
    default:
      assert(0);
    }; //switch(c.digitsMode);
  std::cout << std::endl;
  std::cout << "capitalization = " << boolToString(c.capitalization) << std::endl;
  std::cout << "separation = " << boolToString(c.separation) << std::endl;
  std::cout << "default language = " << (c.defaultLangId != LANG_ID_NONE?langs.getLangName(c.defaultLangId):"none") << std::endl;
  std::cout << "daemon mode = " << boolToString(c.daemonMode) << std::endl;
  std::cout << "pid file name = " << c.pidFileName << std::endl;
  std::cout << "say mode text = " << c.sayModeText << std::endl;
  std::cout << "test configuration = " << boolToString(c.testConfiguration) << std::endl;
  std::cout << "executor = " << c.executor << std::endl;
  std::cout << "player = ";
  switch(c.playerType)
    {
    case PlayerTypeAlsa:
      std::cout << "alsa" << std::endl;
      break;
    case PlayerTypePulseaudio:
      std::cout << "pulseaudio" << std::endl;
      break;
    case PlayerTypePcspeaker:
      std::cout << "pcspeaker" << std::endl;
      break;
    default:
      assert(0);
      std::cout << std::endl;
    }; //switch(c.playerType);
  if (!c.characters.empty())
    {
      std::cout << std::endl;
      std::cout << "Characters map:" << std::endl;
      for(LangIdToWStringMap::const_iterator it = c.characters.begin();it != c.characters.end();it++)
	std::cout << (it->first != LANG_ID_NONE?langs.getLangName(it->first):"default") << " = " << it->second << std::endl;
    }
  printOutputsConfiguration(langs, c);
  if (!c.defaultFamilies.empty())
    {
      std::cout << std::endl;
      std::cout << "Default families for languages:" << std::endl;
      for(LangIdToStringMap::const_iterator it = c.defaultFamilies.begin();it != c.defaultFamilies.end();it++)
	std::cout << langs.getLangName(it->first) << " = " << it->second << std::endl;
    } //default families;
  if (!c.characters.empty())
    {
      std::cout << std::endl;
      std::cout << "Characters map:" << std::endl;
	for(LangIdToWStringMap::const_iterator it = c.characters.begin();it != c.characters.end();it++)
	  std::cout << (it->first != LANG_ID_NONE?langs.getLangName(it->first):"default") << " = " << it->second << std::endl;
    } //characters;
}
