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

#ifndef __VOICEMAN_EMACSPEAK_LINE_PARSER_H__
#define __VOICEMAN_EMACSPEAK_LINE_PARSER_H__

#include"Connection.h"

class TextItem
{
public:
  TextItem(const std::string& t, size_t p, size_t r, size_t v)
    : text(t), pitch(p), rate(r), volume(v) {}

  bool equalParamsTo(const TextItem& item) const
  {
    if (item.pitch != pitch)
      return 0;
    if (item.rate != rate)
      return 0;
    if (item.volume != volume)
      return 0;
    return 1;
  }

public:
  std::string text;
  size_t pitch;
  size_t rate;
  size_t volume;
}; //class TextItem;

typedef std::vector<TextItem> TextItemVector;
typedef std::list<TextItem> TextItemList;

class LineParser
{
public:
  LineParser()
    : m_pitch(50), m_rate(50), m_volume(50), m_rateFraction(5), m_pitchShift(0) {}

  virtual ~LineParser() {}

  void  processCommand(const std::string& command, const StringVector& parameters);

  void setFamily(const std::string& family)
  {
    m_connection.setFamily(family);
  }

  void setRateFraction(size_t value)
  {
    if (value != 0)
      m_rateFraction = value;
  }

  void setPitchShift(int value)
  {
    m_pitchShift = value;
  }

private:
  void addItem(const TextItem& item);
  void processDectalkCommand(const std::string& s, size_t& pitch);
  void processDectalkDefineVoice(StringDelimitedIterator<std::string>& it, size_t& pitch);

  void q(const std::string& s);
  void d();
  void s();
  void l(const std::string& s);
  void tts_say(const std::string& s);
  void tts_set_speech_rate(const std::string& s);
  void t(const StringVector& s);
  void tts_set_punctuations(const std::string& s);
  void tts_sync_state(const StringVector& s);

private:
  Connection m_connection;
  TextItemList m_items;
  size_t m_pitch, m_rate, m_volume;
  size_t m_rateFraction;
  int m_pitchShift;
}; //class LineParser;

#endif //__VOICEMAN_EMACSPEAK_LINE_PARSER_H__;
