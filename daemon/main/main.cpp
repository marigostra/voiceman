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
#include"langs/LangManager.h"
#include"core/core.h"
#include"configuration.h"
#include"system/sockets.h"
#include"core/AbstractTextProcessor.h"
#include"DelimitedFile.h"

#define CHARS_TABLE_FILE_NAME "chars-table"
#define REPLACEMENTS_ALL_FILE_NAME "replacements.all"
#define REPLACEMENTS_SOME_FILE_NAME "replacements.some"
#define REPLACEMENTS_NONE_FILE_NAME "replacements.none"

#define TONE_FREQ_MIN 20
#define TONE_FREQ_MAX 20000
#define TONE_DURATION_MIN 20
#define TONE_DURATION_MAX 4000

CmdArg cmdLineParams[] = {
  {'c', "config", "FILE_NAME", "Use specified configuration file;"},
  {'d', "daemon", "", "run the server in daemon mode;"},
  {'e', "executor", "PATH", "use specified executor;"},
  {'h', "help", "", "show this help screen and exit;"},
  {'l', "loglevel", "LEVEL", "set log filtering level (fatal, error, warn, info, debug);"},
  {'m', "message", "TEXT", "set startup message text;"},
  {'o', "port", "NUMBER", "enable inet socket at port NUMBER;"},
  {'p', "pidfile", "FILE_NAME", "put pid of server process to FILE_NAME;"},
  {'P', "player", "TYPE", "use specified player type (can be \'alsa\', \'pulseaudio\' or \'pcspeaker\');"},
  {'s', "socket", "FILE_NAME", "enable UNIX domain socket at path FILE_NAME;"},
  {'S', "say", "TEXT", "say TEXT and exit;"},
  {'t', "test", "", "only load and show configuration information."},
  {' ', NULL, NULL, NULL}
};

CmdArgsParser cmdLine(cmdLineParams);

volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r)
{
  wasSigHup = 1;
}

volatile sig_atomic_t wasSigPipe = 0;
void sigPipeHandler(int r)
{
  wasSigPipe = 1;
}

void fillOutputListByConfiguration(const OutputConfigurationVector& outputConfigurations, OutputList& outputList)
{
  outputList.clear();
  for(OutputConfigurationVector::size_type i = 0;i < outputConfigurations.size();i++)
    {
      Output o;
      const OutputConfiguration& oc = outputConfigurations[i];
      o.setName(oc.name);
      o.setFamily(oc.family);
      o.setLangId(oc.langId);
      o.setLang(langManager.getLangById(oc.langId));
      o.setSynthCommand(oc.synthCommand);
      o.setAlsaPlayerCommand(oc.alsaPlayerCommand);
      o.setPulseaudioPlayerCommand(oc.pulseaudioPlayerCommand);
      o.setPcspeakerPlayerCommand(oc.pcspeakerPlayerCommand);
      o.setPitchFormat(oc.pitch.numDigitsAfterDot, oc.pitch.min, oc.pitch.aver, oc.pitch.max);
      o.setRateFormat(oc.rate.numDigitsAfterDot, oc.rate.min, oc.rate.aver, oc.rate.max);
      o.setVolumeFormat(oc.volume.numDigitsAfterDot, oc.volume.min, oc.volume.aver, oc.volume.max);
      if (!trim(oc.replacementsFileName).empty())
	{
	  DelimitedFile f;
	  f.read(oc.replacementsFileName);
	  logMsg(LOG_DEBUG, "Read %u records from %s", f.getLineCount(), oc.replacementsFileName.c_str());
	  for(size_t i = 0;i < f.getLineCount();i++)
	    {
	      if (f.getItemCountInLine(i) != 2)
		{
		  logMsg(LOG_WARNING, "In \'%s\' there is line with %u colon-delimited items, but 2 required", oc.replacementsFileName.c_str(), f.getItemCountInLine(i));
		  continue;
		}
	      const std::string fromString = f.getItem(i, 0);
	      const std::string toString = f.getItem(i, 1);
	      if (fromString.empty())
		{
		  logMsg(LOG_WARNING, "There is the record with empty string to replace in file \'%s\'; skipping...", oc.replacementsFileName.c_str());
		  continue;
		}
	      o.addReplacement(readUTF8(fromString), readUTF8(toString));
	    } //for(lines in replacements file);
	} //replacements reading;
      for(WCharToWStringMap::const_iterator it = oc.capList.begin();it != oc.capList.end();it++)
	o.addCapMapItem(it->first, it->second);
      logMsg(LOG_DEBUG, "Adding output \'%s\' to output set", outputConfigurations[i].name.c_str());
      outputList.push_back(o);
    } //for(m_configuration.outputs);
}

/**\brief Processes all client commands
 *
 * ProtocolHandler class processes all commands received from all
 * clients. It makes all required transformations on the text to say,
 * splits it onto sequence of text items with text processing classes and
 * sends everything to ExecutorInterface. Also ProtocolHandler class
 * handles commands to modify any client-specific settings and some
 *others. It contains methods for each client command. They are called
 * by VoicemanProtocol class during client data handling. 
 *
 * \sa VoicemanProtocol AbstractProtocolHandler ExecutorInterface
 */
class ProtocolHandler: public AbstractProtocolHandler
{
public:
  typedef std::map<std::string, AbstractTextProcessor*> StringToTextProcMap;

  /**\brief The constructor
   *
   * \param [in] outputSet A reference to used OutputSet object
   * \param [in] executorInterface A reference to used ExecutorInterface object
   * \param [in] lettersAtMinRate A flag to speak letters at maximum possible speed
   */
  ProtocolHandler(const OutputSet& outputSet,
		  ExecutorInterface& executorInterface,
bool lettersAtMinRate)
    : m_outputSet(outputSet),
      m_executorInterface(executorInterface),
      m_lettersAtMinRate(lettersAtMinRate) {}

  /**\brief the destructor*/
  virtual ~ProtocolHandler() 
  {
    destroyAllTextProcessors();
  }

  /**\brief Initializes all internal structures
   *
   * This method fills all internal structures with settings from provided
   * configuration data. This method can be called so many times as it
   * required. For example, it is called each time on cerver configuration
   * reloading.
   *
   * \param [in] c The configuration data to take settings from
   */
  void reinit(const Configuration& c)
  {
    destroyAllTextProcessors();
    auto_ptr<AbstractTextProcessor> textProc;
    textProc = prepareTextProcessor("all", c, concatUnixPath<std::string>(VOICEMAN_DATADIR, REPLACEMENTS_ALL_FILE_NAME), concatUnixPath<std::string>(VOICEMAN_DATADIR, CHARS_TABLE_FILE_NAME));
    m_textProcessors.insert(StringToTextProcMap::value_type("all", textProc.get()));
    textProc.release();
    textProc = prepareTextProcessor("some", c, concatUnixPath<std::string>(VOICEMAN_DATADIR, REPLACEMENTS_SOME_FILE_NAME), concatUnixPath<std::string>(VOICEMAN_DATADIR, CHARS_TABLE_FILE_NAME));
    m_textProcessors.insert(StringToTextProcMap::value_type("some", textProc.get()));
    textProc.release();
    textProc = prepareTextProcessor("none", c, concatUnixPath<std::string>(VOICEMAN_DATADIR, REPLACEMENTS_NONE_FILE_NAME), concatUnixPath<std::string>(VOICEMAN_DATADIR, CHARS_TABLE_FILE_NAME));
    m_textProcessors.insert(StringToTextProcMap::value_type("none", textProc.get()));
    textProc.release();
    m_defaultFamilies = c.defaultFamilies;
  }

  /**\brief Notifies the command to say text was received from client
   *
   * The protocol implementation class calls this method on every command
   * to say part of text and provides the client object this command was
   * transmitted from.
   *
   * \param [in] client The client object the command was received from
   * \param [in] t The text to say
   */
  void onText(Client& client, const std::wstring& t)
  {
    const AbstractTextProcessor* textProc = selectTextProc(client.selectedTextProcessor);
    if (textProc == NULL)
      return;//all log messages must be in selectTextProc();
    logMsg(LOG_DEBUG, "Processing \'TEXT\' command with processor \'%s\'", client.selectedTextProcessor.c_str());
    //Preparing text item to provide into text processor;
    TextItem textItem(t);
    textItem.setPitch(client.pitch);
    textItem.setRate(client.rate);
    textItem.setVolume(client.volume);
    TextItemList textItemList;
    textProc->process(textItem, textItemList);
    logMsg(LOG_DEBUG, "Text processor generated %u text item(s)", textItemList.size());
    //OK, now we have the set of splitted items, but output information is omitted in it, only language specifications;
    TextItemList preparedTextItems;
    assignOutput(client, textItemList, preparedTextItems);
    for(TextItemList::const_iterator it = preparedTextItems.begin();it != preparedTextItems.end();it++)
      m_executorInterface.sayOrEnqueue(*it);
  }

  /**\brief Notifies the command to say one letter was received from client
   *
   * The protocol implementation calls this method each time when it
   * receives command to say one letter. The client object this command was
   * received from is provided.
   *
   * \param [in] client The client object this command was received from
   * \param [in] c The character to say
   */
  void onLetter(Client& client, wchar_t c)
  {
    const AbstractTextProcessor* textProc = selectTextProc(client.selectedTextProcessor);
    if (textProc == NULL)
      return;//all log messages must be in selectTextProc();
    logMsg(LOG_DEBUG, "Processing \'LETTER\' command with processor \'%s\'", client.selectedTextProcessor.c_str());
    //Preparing text item to provide into text processor;
    TextItemList textItemList;
    textProc->processLetter(c, client.volume, client.pitch, m_lettersAtMinRate?0:client.rate, textItemList);
    logMsg(LOG_DEBUG, "Text processor generated %u text items", textItemList.size());
    //OK, now we have the set of splitted items, but output information is omitted in it, only language specifications;
    TextItemList preparedTextItems;
    assignOutput(client, textItemList, preparedTextItems);
    for(TextItemList::const_iterator it = preparedTextItems.begin();it != preparedTextItems.end();it++)
      m_executorInterface.sayOrEnqueue(*it);
  }

  /**\brief Notifies new command to stop playback was received from client
   *
   * The protocol implementation calls this function when it receives the
   * command to stop any playback. The client object new command was
   * received from is provided.
   *
   * \param [in] client The client object this command was received from
   */
  void onStop(Client& client)
  {
    logMsg(LOG_DEBUG, "Processing \'STOP\' command");
    m_executorInterface.stop();
  }

  /**\brief Notifies there was a command to set new value of speech parameter
   *
   * This method is called by protocol implementation object on every new
   * command to change speech attribute value. Speech attribute can be one
   * of three types: pitch, rate and volume. 
   *
   * \param [in] client The client object this command was received from
   * \param [in] paramType The attribute type to change, can be ParamPitch, ParamRate and ParamVolume
   * \param [in] value The new value to set
   */
  void onParam(Client& client, int paramType, TextParam value)
  {
    switch(paramType)
      {
      case ParamVolume:
	logMsg(LOG_DEBUG, "Setting volume value to %u", value.getValue());
	client.volume = value;
	break;
      case ParamPitch:
	logMsg(LOG_DEBUG, "Setting pitch value to %u", value.getValue());
	client.pitch = value;
	break;
      case ParamRate:
	logMsg(LOG_DEBUG, "Setting rate value to %u", value.getValue());
	client.rate = value;
	break;
      default:
	assert(0);
      } //switch(paramType);
  }

  /**\brief Notifies the new command to produce tone signal was received from client
   *
   * This method is called by protocol implementation object each time when
   * new command to produce tone signal is received from a client. The
   * client object this command was received from is provided.
   *
   * \param [in] client The client object this command was received from
   * \param [in] freq The frequency of tone signal to produce
   * \param [in] duration The new signal duration
   */
  void onTone(Client& client, size_t freq, size_t duration)
  {
    if (freq < TONE_FREQ_MIN || freq > TONE_FREQ_MAX)
      {
	logMsg(LOG_WARNING, "Tone command has illegal frequency value %u", freq);
	return;
      }
    if (duration < TONE_DURATION_MIN || duration > TONE_DURATION_MAX)
      {
	logMsg(LOG_WARNING, "Tone command has illegal duration value %u", duration);
	return;
      }
    logMsg(LOG_DEBUG, "Sending \'TONE\' command with frequency %u and duration %u", freq, duration);
    m_executorInterface.tone(freq, duration);
  }

  /**\brief Notifies the command to select another processing mode was received
   *
   * This method is called by protocol implementation class on each command
   * to select new text processing mode. The client object new command was
   * received from and new mode name are provided.
   *
   * \param [in] client The client this command was received from
   * \param [in] procMode The name of new text processing mode
   */
  void onProcMode(Client& client, const std::string& procMode)
  {
    const std::string value = trim(toLower(procMode));
    StringToTextProcMap::const_iterator it = m_textProcessors.find(value);
    if (it == m_textProcessors.end())
      {
	logMsg(LOG_WARNING, "Unknown text processing mode \'%s\', rejecting client command", procMode.c_str());
	return;
      }
    logMsg(LOG_DEBUG, "Selecting text processing mode \'%s\'", value.c_str());
    client.selectedTextProcessor = value;
  }

  /**\brief Notifies the command to select new voice family was received from client
   *
   * The protocol implementation class cals this method each time when the
   * command to select new voice family was received from client. The
   * client object this command was received from, the language name to
   * select family for and new voice family name are provided. The language
   * string can be empty and it means new family must be selected for all
   * languages in which it is registered.
   *
   * \param [in] client The client this command was received from
   * \param [in] lang The language name to select family for, can be empty
   * \param [in] family The voice family to select
   */
  void onFamily(Client& client, const std::string lang, const std::string& family)
  {
    if (trim(family).empty())
      {
	logMsg(LOG_WARNING, "Received command to select new voice family but family name is empty");
	return;
      }
    LangId langId = LANG_ID_NONE;
    if (!trim(lang).empty())
      {
	if (!langManager.hasLanguage(lang))
	  {
	    logMsg(LOG_WARNING, "Received command to select family for unknown language \'%s\'", lang.c_str());
	    return;
	  }
	langId = langManager.getLangId(lang);
      }
    if (langId == LANG_ID_NONE)
      {
	//Using m_defaultFamilies map to enumerate all available languages;
	for(LangIdToStringMap::const_iterator it = m_defaultFamilies.begin();it != m_defaultFamilies.end();it++)
	  {
	    if (!m_outputSet.isValidFamilyName(it->first, family))
	      continue;
	    LangIdToStringMap::iterator clientIt = client.selectedFamilies.find(it->first);
	    if (clientIt != client.selectedFamilies.end())
	      clientIt->second = family; else 
	      client.selectedFamilies.insert(LangIdToStringMap::value_type(it->first, family));
	  } //for(m_defaultFamilies);
	logMsg(LOG_DEBUG, "Selecting family \'%s\' for all languages with corresponding output", family.c_str());
	return;
      }
    if (!m_outputSet.isValidFamilyName(langId, family))
      {
	logMsg(LOG_WARNING, "Received command to select new family for language \'%s\' but family name \'%s\' is unknown", lang.c_str(), family.c_str());
	return;
      }
    LangIdToStringMap::iterator it = client.selectedFamilies.find(langId);
    if (it != client.selectedFamilies.end())
      it->second = family; else 
      client.selectedFamilies.insert(LangIdToStringMap::value_type(langId, family));
    logMsg(LOG_DEBUG, "Selected family \'%s\' for language \'%s\'", family.c_str(), lang.c_str());
  }

private:
  void assignOutput(const Client& client, TextItemList& textItemList, TextItemList& preparedItems) const
  {
    preparedItems.clear();
    for(TextItemList::iterator it = textItemList.begin();it != textItemList.end();it++)
      {
	const TextItem& ti = *it;
	//Now we must find  proper output to process;
	const LangId langId = ti.getLangId();
	if (langManager.getLangById(langId) == NULL)
	  {
	    logMsg(LOG_WARNING, "Text item received from text processor has unknown  language identifier");
	    continue;
	  }
	LangIdToStringMap::const_iterator familyIt = client.selectedFamilies.find(langId);
	const std::string familyName = familyIt != client.selectedFamilies.end()?familyIt->second:getDefaultFamilyByLangId(langId);
	if (trim(familyName).empty())
	  {
	    logMsg(LOG_WARNING, "Could not find proper voice family for text item, skipping...");
	    continue;
	  }
	if (!m_outputSet.isValidFamilyName(langId, familyName))
	  {
	    logMsg(LOG_ERR, "Output set had rejected family name \'%s\', skipping text item...", familyName.c_str());
	    continue;
	  }
	it->setOutputName(m_outputSet.getOutputNameByFamilyAndLangId(familyName, langId));
	preparedItems.push_back(*it);
      } //for(items);
  }

  const AbstractTextProcessor* selectTextProc(const std::string& textProcName) const
  {
    if (trim(textProcName).empty())
      {
	logMsg(LOG_ERR, "Cannot select proper text processor, text processor string is empty");
	return NULL;
      }
    StringToTextProcMap::const_iterator it = m_textProcessors.find(textProcName);
    if (it == m_textProcessors.end())
      {
	logMsg(LOG_ERR, "Cannot select text processor, text processor name \'%s\' is unknown", textProcName.c_str());
	return NULL;
      }
    return it->second;
  }

  auto_ptr<AbstractTextProcessor> prepareTextProcessor(const std::string& name, const Configuration& c, const std::string& replacementsFileName, const std::string& charsTableFileName)
  {
    logMsg(LOG_DEBUG, "Creating \'%s\' text processor (replacementsFileName =%s, charsTableFIleName=%s)", name.c_str(), replacementsFileName.c_str(), charsTableFileName.c_str());
    auto_ptr<AbstractTextProcessor> textProc = createNewTextProcessor(langManager, c.digitsMode, c.capitalization, c.separation);
    LangIdSet langIdSet;
    //Using only really required languages;
    for(OutputConfigurationVector::size_type i = 0;i < c.outputs.size();i++)
      langIdSet.insert(c.outputs[i].langId);
    for(LangIdSet::const_iterator it = langIdSet.begin();it != langIdSet.end();it++)
      {
	logMsg(LOG_DEBUG, "Adding \'%s\' language support to text processor \'%s\'", langManager.getLangName(*it).c_str(), name.c_str());
	const Lang* lang = langManager.getLangById(*it);
	assert(lang != NULL);
	std::wstring characters = lang->getAllChars();
	textProc->associate(characters, *it);
	LangIdToWStringMap::const_iterator charIt = c.characters.find(*it);
	if (charIt != c.characters.end())
	  textProc->associate(charIt->second, *it);
      } //for(used languages);
    if (c.defaultLangId != LANG_ID_NONE)
      {
	if (langIdSet.find(c.defaultLangId) != langIdSet.end())
	  {
	    logMsg(LOG_DEBUG, "Default language is \'%s\'", langManager.getLangName(c.defaultLangId).c_str());
	    textProc->setDefaultLangId(c.defaultLangId);
	    LangIdToWStringMap::const_iterator defaultLangCharactersIt = c.characters.find(LANG_ID_NONE);
	    if (defaultLangCharactersIt != c.characters.end())
	      textProc->associate(defaultLangCharactersIt->second, LANG_ID_NONE);
	  } else
	  logMsg(LOG_WARNING, "Language \'%s\' selected as default, but there are no outputs for it. Text processor \'%s\' will have not default language", langManager.getLangName(c.defaultLangId).c_str(), name.c_str());
      } //default language selecting;
    //processing replacements file;
    DelimitedFile f;
    f.read(replacementsFileName);
    logMsg(LOG_DEBUG, "Read %u records from %s", f.getLineCount(), replacementsFileName.c_str());
    for(size_t i = 0;i < f.getLineCount();i++)
      {
	if (f.getItemCountInLine(i) != 3)
	  {
	    logMsg(LOG_WARNING, "In \'%s\' there is line with %u colon-delimited items, but 3 required", replacementsFileName.c_str(), f.getItemCountInLine(i));
	    continue;
	  }
	const std::string langName = toLower(trim(f.getItem(i, 0)));
	const std::string fromString = f.getItem(i, 1);
	const std::string toString = f.getItem(i, 2);
	if (!langManager.hasLanguage(langName))
	  {
	    logMsg(LOG_WARNING, "File \'%s\' contains reference to unknown language \'%s\'", replacementsFileName.c_str(), langName.c_str());
	    continue;
	  }
	const LangId langId = langManager.getLangId(langName);
	assert(langId != LANG_ID_NONE);
	if (fromString.empty())
	  {
	    logMsg(LOG_WARNING, "There is the record with empty string to replace in file \'%s\'; skipping...", replacementsFileName.c_str());
	    continue;
	  }
	textProc->addReplacement(langId, readUTF8(fromString), readUTF8(toString));
      } //for(lines in replacements file);
    //processing characters table file;
    f.read(charsTableFileName);
    logMsg(LOG_DEBUG, "Read %u records from \'%s\'", f.getLineCount(), charsTableFileName.c_str());
    for(size_t i = 0;i < f.getLineCount();i++)
      {
	if (f.getItemCountInLine(i) != 2)
	  {
	    logMsg(LOG_WARNING, "In \'%s\' there is line with %u colon-delimited items, but 2 required", charsTableFileName.c_str(), f.getItemCountInLine(i));
	    continue;
	  }
	const std::string charFrom = trim(f.getItem(i, 0));
	const std::string toString = trim(f.getItem(i, 1));
	const std::wstring wCharFrom = readUTF8(charFrom);
	if (wCharFrom.length() != 1)
	  {
	    logMsg(LOG_WARNING, "File \'%s\' contains record without character specification or for more than one character", charsTableFileName.c_str());
	    continue;
	  }
	if (toString.empty())
	  {
	    logMsg(LOG_WARNING, "File \'%s\' contains the record with missed value", charsTableFileName.c_str());
	    continue;
	  }
	textProc->setSpecialValueFor(wCharFrom[0], readUTF8(toString));
      } //for(lines in characters table file);
    return textProc;
  }

  void destroyAllTextProcessors()
  {
    for(StringToTextProcMap::iterator it = m_textProcessors.begin();it != m_textProcessors.end();it++)
      delete it->second;
    m_textProcessors.clear();
  }

  std::string getDefaultFamilyByLangId(LangId langId) const
  {
    LangIdToStringMap::const_iterator it = m_defaultFamilies.find(langId);
    return it != m_defaultFamilies.end()?it->second:"";
  }

private:
  const OutputSet& m_outputSet;
  ExecutorInterface& m_executorInterface;
  StringToTextProcMap m_textProcessors;
  LangIdToStringMap m_defaultFamilies;
  bool m_lettersAtMinRate;
}; //class ProtocolHandler;

/**\brief The general handler of signals to daemon process
 *
 * This class receives notifications from server main loop about received
 * system signals. Main loop does not provide any information about the
 * signal was received. This class must check flags set by functions
 * called automatically by kernel on each signal and decide which exact
 * signal must be handled.
 */
class SystemSignalHandler: public AbstractSignalHandler
{
public:
  /**\brief The constructor
   *
   * \param [in] The reference to list of connected clients
   * \param [in] outputSet The reference to used output set object
   * \param [in] protocolHandler The reference to used protocol handler object
   * \param [in] executorInterface The reference to executor interface object
   */
  SystemSignalHandler(ClientList& clients, OutputSet& outputSet, ProtocolHandler& protocolHandler, ExecutorInterface& executorInterface)
    : m_clients(clients), m_outputSet(outputSet), m_protocolHandler(protocolHandler), m_executorInterface(executorInterface) {}

  /**\brief The destructor*/
  virtual ~SystemSignalHandler() {}

  /**\brief Notifies there was system signal and it must be handled
   *
   * This method is called by main loop class each time when pselect()
   * system call is interrupted with EINTR exit code. It means the process
   * have received system signal and it must be handled.
   */
  void onSystemSignal()
  {
    if (wasSigPipe)
      {
	wasSigPipe = 0;
	logMsg(LOG_WARNING, "SIGPIPE signal registered, usually it is caused by unexpected executor death");
	m_executorInterface.stopExecutor();
      }
    if (wasSigHup)
      {
	wasSigHup = 0;
	logMsg(LOG_DEBUG, "SIGHUP registered, reloading configuration");
	Configuration c;
	initConfigData(c);
	try {
	  prepareConfiguration(langManager, cmdLine, c);
	}
	catch(const VoicemanException& e)
	  {
	    e.makeLogReport(LOG_ERR);
	    return;
	  }
	catch(std::bad_alloc)
	  {
	    logMsg(LOG_ERR, "No enough free memory to reload configuration, operation aborted");
	    return;
	  }
	OutputList outputList;
	fillOutputListByConfiguration(c.outputs, outputList);
	m_outputSet.reinit(outputList);
	m_protocolHandler.reinit(c);
	logMsg(LOG_DEBUG, "resetting families preferences for %u clients", m_clients.size());
	for(ClientList::iterator it = m_clients.begin();it != m_clients.end();it++)
	  (*it)->selectedFamilies.clear();
	logMsg(LOG_INFO, "New configuration was successfully reloaded!");
      }
  }

private:
  ClientList& m_clients;
  OutputSet& m_outputSet;
  ProtocolHandler& m_protocolHandler;
  ExecutorInterface& m_executorInterface;
}; //class SystemSignalHandler;


/**\brief Processes raw lines of data from clients
 *
 * This class is the intermediate point between main loop and protocol
 * parser. It is purposed to control incomplete lines and line length
 * exceeding.
 *
 * \sa VoicemanProtocol
 */
class ClientDataHandler: public AbstractClientDataHandler
{
public:
  /**\brief The constructor
   *
   * \param [in] protocol The reference to protocol handling object
   * \param [in] maxInputLine The maximum length of input line (0 - not limited)
   */
  ClientDataHandler(VoicemanProtocol& protocol, size_t maxInputLine)
    : m_protocol(protocol), m_maxInputLine(maxInputLine) {}

  /**\brief Processes new part of data from the client
   *
   * This method receives new portion of data as single line of text,
   * splits it to the proper line set, processes it with protocol object
   * and controls incomplete line part.
   *
   * \param [in] client The reference to client object to handle data for
   * \param [in] data The data to process
   */
  void processClientData(Client& client, const std::string& data)
  {
    std::wstring text = readUTF8(data);
    std::wstring::size_type offset = 0;
    for(std::wstring::size_type i = 0;i < text.size();i++)
      if (text[i] != '\r')
	{
	  if (offset > 0)
	    text[i - offset] = text[i];
	} else
	offset++;
    assert(offset <= text.size());
    text.resize(text.size() - offset);
    std::wstring buf = client.chain;
    buf.reserve(buf.length() + text.length());
    for(std::wstring::size_type i = 0;i < text.length();i++)
      {
	if (text[i] == '\n')
	  {
	    if (!client.rejecting)
	      m_protocol.process(buf, client);
	    client.rejecting = 0;
	    buf.clear();
	    continue;
	  } //'\n';
	if (client.rejecting)
	  continue;
	buf += text[i];
	if (m_maxInputLine > 0 && buf.length() > m_maxInputLine )
	  {
	    logMsg(LOG_DEBUG, "Input line exceeds input line length limit. Truncating...");
	    client.rejecting = 1;
	    m_protocol.process(buf, client);
	    buf.clear();
	  }
      } //for();
    client.chain = buf;
  }

private:
  VoicemanProtocol& m_protocol;
  const size_t m_maxInputLine;
}; //class ClientDataHandler;

/**\brief The central class of server point
 *
 * This class receives execution just after basic initialization of
 * application is done. It controls communication between all basic parts
 * of server such as executor, client data handling, signal processing,
 * etc.
 */
class VoicemanServer: private AbstractExecutorCallback
{
public:
  /**\brief The default constructor
   *
   * \param [in] c The configuration of server to be used
   */
  VoicemanServer(const Configuration& c)
    : m_configuration(c), m_sayMode(!trim(c.sayModeText).empty()), m_terminationFlag(0) {}

  /**\brief The destructor*/
  virtual ~VoicemanServer()
  {
    closeAllConnections();
    closeSockets();
  }

  /**\brief The main method to execute server
   *
   * This method receives control just after configuration loading,
   *prepares of basic parts of server and performs execution of main loop.
   */
  void run()
  {
    logMsg(LOG_DEBUG, "Installing signal handlers");
    installSignalProcessing();
    logMsg(LOG_DEBUG, "Starting server initialization: charset for I/O operation: %s", transcoding.getIOCharset().c_str());
    logMsg(LOG_DEBUG, "Initializing languages with datadir=%s", VOICEMAN_DATADIR);
    langManager.load(VOICEMAN_DATADIR);
    logMsg(LOG_DEBUG, "Language set was initialized, preparing executor interface (%s)", m_configuration.executor.c_str());
    OutputSet outputSet;
    ExecutorInterface executorInterface(*this, outputSet, m_configuration.maxQueueSize, m_configuration.executor, m_configuration.playerType);
    logMsg(LOG_DEBUG, "Executor was prepared successfully, filling set of outputs and protocol handler");
    //Filling set of outputs;
    OutputList outputList;
    fillOutputListByConfiguration(m_configuration.outputs, outputList);
    outputSet.reinit(outputList);
    logMsg(LOG_DEBUG, "Initializing protocol handler");
    ProtocolHandler protocolHandler(outputSet, executorInterface, m_configuration.lettersAtMinRate);
    logMsg(LOG_DEBUG, "Initializing text processing");
    protocolHandler.reinit(m_configuration);
    logMsg(LOG_DEBUG, "Text processing initialized");
    VoicemanProtocol protocol(protocolHandler);
    ClientFactory clientFactory;
    ClientDataHandler clientDataHandler(protocol, m_configuration.maxInputLine);
    SystemSignalHandler systemSignalHandler(m_clients, outputSet, protocolHandler, executorInterface);
    MainLoop mainLoop(clientFactory, m_clients, m_configuration.maxClients, clientDataHandler, systemSignalHandler, executorInterface, m_terminationFlag);
    if (!m_sayMode)
      {
	logMsg(LOG_DEBUG, "Initializing sockets...");
	initSockets(trim(m_configuration.unixDomainSocketFileName), m_configuration.useInetSocket, m_configuration.inetSocketPort);
      }
    if (!m_sayMode && !trim(m_configuration.startUpMessage).empty())
      {
	auto_ptr<Client> client = clientFactory.createFakeClient();
	protocolHandler.onText(*client.get(), m_configuration.startUpMessage);
      }
    if (m_sayMode)
      {
	logMsg(LOG_INFO, "Speaking text \'%s\'", WString2IO(m_configuration.sayModeText).c_str());
	auto_ptr<Client> client = clientFactory.createFakeClient();
	protocolHandler.onText(*client.get(), m_configuration.sayModeText);
      }
    if (!m_sayMode && !trim(m_configuration.pidFileName).empty())
      {
	std::ofstream f(m_configuration.pidFileName.c_str());
	if (f)
	  {
	    f << getpid() << std::endl;
	    logMsg(LOG_DEBUG, "Writing pid %d to \'%s\'", getpid(), m_configuration.pidFileName.c_str());
	  } else
	  logMsg(LOG_ERR, "Could not save pid to \'%s\'", m_configuration.pidFileName.c_str());
      } //server pid saving;
    if (!m_sayMode)
      logMsg(LOG_INFO, "VoiceMan server is ready, welcome new connections!");
    mainLoop.run(m_sockets, &m_origMask);
    closeSockets();
  }

private://AbstractExecutorCallback;
  void onExecutorEvent(int event)
  {
    if (m_sayMode && event == AbstractExecutorCallback::Silence)
      {
	logMsg(LOG_DEBUG, "Registering silence command in say mode, terminationFlag=1");
	m_terminationFlag = 1;
      }
  }

private:
  void installSignalProcessing()
  {
    struct sigaction sa;
    //SIGHUP handler installation;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);
    //SIGPIPE handler installation;
    sigaction(SIGPIPE, NULL, &sa);
    sa.sa_handler = sigPipeHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGPIPE, &sa, NULL);
    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigaddset(&blockedMask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &blockedMask, &m_origMask);
  }

  void closeAllConnections()
  {
    for (ClientList::iterator it = m_clients.begin();it != m_clients.end();it++)
      delete *it;//sockets will be closed automatically in destructor;
    m_clients.clear();
  }

  void initSockets(const std::string unixSocketPath, bool useInetSocket, size_t inetSocketPort)
  {
    assert(m_sockets.empty());
    if (!trim(unixSocketPath).empty())
      {
	auto_ptr<UnixSocket> socket(new UnixSocket());
	socket->open(trim(unixSocketPath));
	m_sockets.push_back(socket.get());
	socket.release();
	logMsg(LOG_DEBUG, "Unix domain socket was successfully opened as \'%s\'", trim(unixSocketPath).c_str());
      } //UNIX domain socket initialization;
    if (useInetSocket)
      {
	assert(inetSocketPort < 65536);
	auto_ptr<InetSocket> socket(new InetSocket());
	socket->open(inetSocketPort);
	m_sockets.push_back(socket.get());
	socket.release();
	logMsg(LOG_DEBUG, "Accepting TCP/IP connections at port %d", inetSocketPort);
      } //inet socket initialization;
  }

  void closeSockets()
  {
    SocketList::iterator it;
    for(it = m_sockets.begin();it != m_sockets.end();it++)
      {
	(*it)->close();
	delete *it;
      }
    m_sockets.clear();
  }

private:
  const Configuration& m_configuration;
  ClientList m_clients;
  SocketList m_sockets;
  bool m_sayMode;
  bool m_terminationFlag;
  sigset_t m_origMask;
}; //class VoicemanServer;

void printLogo(std::ostream& s, bool shortly)
{
  s << "VoiceMan speech processing system. Version: " << PACKAGE_VERSION << "." << std::endl;
  if (!shortly)
    s << "Press Ctrl+C to terminate this server." << std::endl;
  s << std::endl;
}

/*
  if (!configuration.useInetSocket && trim(configuration.unixSocket).empty())
    throw ConfigurationException("no sockets are specified in the configuration file to listen");
 */

bool processConfiguration(Configuration& c)
{
  try {
    prepareConfiguration(langManager, cmdLine, c);
  }
  catch(const VoicemanException& e)
    {
      printLogo(std::cerr, 1);
      std::cerr << "There are problems with processing your configuration data:"  << std::endl;
      std::cerr << e.getMessage() << std::endl;
      return 0;
    }
  catch(std::bad_alloc)
    {
      printLogo(std::cerr, 1);
      std::cerr << "There is no enough available memory for server configuration processing." << std::endl;
      return 0;
    }
  return 1;
}

int main(int argc, char *argv[])
{
  if (!cmdLine.parse(argc, argv))
    return 1;
  if (cmdLine.used("help"))
    {
      printLogo(std::cout, 1);
      std::cout << "Command line arguments:" << std::endl;
      cmdLine.printHelp();
      return 0;
    }
  Configuration configuration;
  initConfigData(configuration);
  if (!processConfiguration(configuration))
    return 1;
  if (configuration.testConfiguration)
    {
      printLogo(std::cout, 1);
      printConfiguration(langManager, configuration);
      return 0;
    }
  if (configuration.daemonMode)
    {
      pid_t serverPid = fork();
      if (serverPid == (pid_t)-1)
	{
	  printLogo(std::cerr, 1);
	  std::cerr << "Server cannot be started in daemon mode due to child process creation problems:" << std::endl;
	  std::cerr << ERRNO_MSG << std::endl;
	  return 1;
	} //fork problems;
      if (serverPid == (pid_t)0)
	{
	  initLogging(configuration.logFileName, 0, configuration.logLevel);
	  try {
	    VoicemanServer server(configuration);
	    server.run();
	  }
	  catch(const VoicemanException& e)
	    {
	      e.makeLogReport(LOG_CRIT);
	      exit(EXIT_FAILURE);
	    }
	  catch(std::bad_alloc)
	    {
	      logMsg(LOG_CRIT, "No enough free memory for server process, exiting...");
	      exit(EXIT_FAILURE);
	    }
	  exit(EXIT_SUCCESS);
	} //child process;
      return 0;
    } //daemon mode;
  printLogo(std::cout, !trim(configuration.sayModeText).empty());
  initLogging(configuration.logFileName, 1, configuration.logLevel);
  try {
    VoicemanServer server(configuration);
    server.run();
  }
  catch(const VoicemanException& e)
    {
      e.makeLogReport(LOG_CRIT);
      return 1;
    }
  catch(std::bad_alloc)
    {
      logMsg(LOG_CRIT, "No enough free memory for server process, exiting...");
      return 2;
    }
  return 0;
}
