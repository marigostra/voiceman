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

#include"emacspeak.h"
#include"LineParser.h"

static std::string combineStringVector(const StringVector& v)
{
  if (v.empty())
    return "";
  if (v.size() == 1)
    return v.front();
  std::string s = v.front();
  for(StringVector::size_type i = 1;i < v.size();i++)
    s += " " + v[i];
  return s;
}

void LineParser::addItem(const TextItem& item)
{
  if (m_items.empty())
    {
      m_items.push_back(item);
      return;
    }
  TextItem& last = m_items.back();
  if (!last.equalParamsTo(item))
    {
      m_items.push_back(item);
      return;
    }
  last.text += " ";
  last.text += item.text;
}

void LineParser::processDectalkDefineVoice(StringDelimitedIterator<std::string>& it, size_t& pitch)
{
  bool waitingAverPitch = 0;
  while(it.next())
    {
      const std::string s = toLower(it.str());
      if (s == "ap")
	{
	  waitingAverPitch = 1;
	  continue;
	}
      if (waitingAverPitch)
	{
	  waitingAverPitch = 0;
	  if (!checkTypeUnsignedInt(s))
	    continue;
	  const size_t value = parseAsUnsignedInt(s);
	  if (value < 50 || value > 350)
	    continue;
	  pitch = (value - 50) / 3;
	} //if(waitingAverPitch);
    } //while();
}

void LineParser::processDectalkCommand(const std::string& s, size_t& pitch)
{
  StringDelimitedIterator<std::string> it(s, " \t");
  while(it.next())
    {
      if (toLower(it.str()) == ":dv")
	{
	  processDectalkDefineVoice(it, pitch);
	  return;
	}
    }
}

void  LineParser::processCommand(const std::string& command, const StringVector& parameters)
{
  const std::string cmd = trim(toLower(command));
  if (cmd == "q")
    q(combineStringVector(parameters));
  if (cmd == "d")
    d();
  if (cmd == "s")
    s();
  if (cmd == "l")
    l(combineStringVector(parameters));
  if (cmd == "tts_say")
    tts_say(combineStringVector(parameters));
  if (cmd == "tts_set_speech_rate")
    tts_set_speech_rate(combineStringVector(parameters));
  if (cmd == "t")
    t(parameters);
  if (cmd == "tts_set_punctuations")
    tts_set_punctuations(combineStringVector(parameters));
  if (cmd == "tts_sync_state")
    tts_sync_state(parameters);
}

void LineParser::q(const std::string& s)
{
  size_t pitch = m_pitch;
  std::string::size_type i = 0;
  while(i < s.length())
    {
      std::string z;
      while(i < s.length() && s[i] != '[')
	z += s[i++];
      if (!z.empty())
	addItem(TextItem(z, pitch, m_rate, m_volume));
      if (i < s.length() && s[i] == '[')
	{
	  z.erase();
	  while(i < s.length() && s[i] != ']')
	    z += s[i++];
	  processDectalkCommand(z, pitch);
	  i++;
	} //dectalk commands processing;
    } //while(s[i]);
}

void LineParser::d()
{
  for(TextItemList::const_iterator it = m_items.begin();it != m_items.end();it++)
    {
      if (m_pitchShift >= 0 || it->pitch > (size_t)(-1 * m_pitchShift))
	m_connection.pitch(it->pitch + m_pitchShift); else
	m_connection.pitch(0);
      m_connection.rate(it->rate);
      m_connection.volume(it->volume);
      m_connection.text(it->text);
    }
  m_items.clear();
}

void LineParser::s()
{
  m_connection.stop();
}

void LineParser::l(const std::string& s)
{
  m_connection.letter(trim(s));
}

void LineParser::tts_say(const std::string& s)
{
  std::string text;
  std::string::size_type i = 0;
  while(i < s.length())
    {
      std::string z;
      while(i < s.length() && s[i] != '[')
	z += s[i++];
      if (!z.empty())
	{
	  text += " ";
	  text += z;
	}
      if (i < s.length() && s[i] == '[')
	{
	  z.erase();
	  while(i < s.length() && s[i] != ']')
	    z += s[i++];
	  //do nothing here with dectalk command;
	  i++;
	} //dectalk commands processing;
    } //while(s[i]);
  m_connection.stop();
  if (m_ttsSayAtMinRate)
    {
      m_connection.rate(0);
      m_connection.text(text);
      m_connection.rate(m_rate);
    } else
    m_connection.text(text);
}

void LineParser::tts_set_speech_rate(const std::string& s)
{
  if (!checkTypeUnsignedInt(s))
    return;
  m_rate = parseAsUnsignedInt(s) / m_rateFraction;
}

void LineParser::t(const StringVector& s)
{
  if (s.size() != 2)
    return;
  const std::string p1 = trim(s[0]), p2 = trim(s[1]);
  if (!checkTypeUnsignedInt(p1) || !checkTypeUnsignedInt(p2))
    return;
  m_connection.tone(parseAsUnsignedInt(p1), parseAsUnsignedInt(p2));
}

void LineParser::tts_set_punctuations(const std::string& s)
{
  m_connection.punc(s);
}

void LineParser::tts_sync_state(const StringVector& s)
{
  if (s.size() != 5)
    return;
  m_connection.punc(s[0]);
  const std::string ss = trim(s[4]);
  if (!checkTypeUnsignedInt(ss))
    return;
  m_rate = parseAsUnsignedInt(ss) / m_rateFraction;
}
