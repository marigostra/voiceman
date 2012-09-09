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
#include"Connection.h"

bool Connection::connectionAvailable()
{
  if (m_con != VOICEMAN_BAD_CONNECTION)
    return 1;
  m_con = vm_connect();
  if (m_con == VOICEMAN_BAD_CONNECTION)
    return 0;
  if (!m_family.empty())
    vm_family(m_con, VOICEMAN_LANG_NONE, (char*)m_family.c_str());
  return 1;
}

void Connection::text(const std::string& t)
{
  if (!connectionAvailable())
    return;
  if (trim(IO2WString(t)).length() == 1)//Just for Orca hack!!!
    {
      letter(trim(t));
      return;
    }
  const std::string s=encodeUTF8(IO2WString(t));
  vm_text(m_con, (char*)s.c_str());
}

void Connection::letter(const std::string& l)
{
  if (!connectionAvailable())
    return;
  std::wstring t=IO2WString(l);
  if (t.length() > 1)
    t.resize(1);
  const std::string s=encodeUTF8(t);
  vm_letter(m_con, (char*)s.c_str());
}

void Connection::stop()
{
  if (!connectionAvailable())
    return;
  vm_stop(m_con);
}

void Connection::tone(size_t freq, size_t duration)
{
  if (!connectionAvailable())
    return;
  vm_tone(m_con, freq, duration);
}

void Connection::pitch(size_t value)
{
  assert(value != NoValue);
  if (m_lastPitchValue == value)
    return;
  if (!connectionAvailable())
    return;
  vm_pitch(m_con, value < 100?value:100);
  m_lastPitchValue = value;
}

void Connection::rate(size_t value)
{
  assert(value != NoValue);
  if (m_lastRateValue == value)
    return;
  if (!connectionAvailable())
    return;
  vm_rate(m_con, value < 100?value:100);
  m_lastRateValue = value;
}

void Connection::volume(size_t value)
{
  assert(value != NoValue);
  if (m_lastVolumeValue == value)
    return;
  if (!connectionAvailable())
    return;
  vm_volume(m_con, value < 100?value:100);
  m_lastVolumeValue = value;
}

void Connection::family(const std::string& value)
{
  if (!connectionAvailable())
    return;
  const std::string s = encodeUTF8(IO2WString(value));
  vm_family(m_con, VOICEMAN_LANG_NONE, (char*)s.c_str());
}

void Connection::punc(const std::string& mode)
{
  if (!connectionAvailable())
    return;
  const std::string m = trim(toLower(mode));
  if (m == "all")
    vm_procmode(m_con, VOICEMAN_PROCMODE_ALL);
  if (m == "some")
    vm_procmode(m_con, VOICEMAN_PROCMODE_SOME);
  if (m == "none")
    vm_procmode(m_con, VOICEMAN_PROCMODE_NONE);
}
