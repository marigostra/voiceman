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

#ifndef __VOICEMAN_EMACSPEAK_CONNECTION_H__
#define __VOICEMAN_EMACSPEAK_CONNECTION_H__

class Connection
{
public:
  enum {NoValue = (size_t)-1};

  Connection() 
    : m_con(VOICEMAN_BAD_CONNECTION), m_lastPitchValue(NoValue), m_lastRateValue(NoValue), m_lastVolumeValue(NoValue) {}
  virtual ~Connection() {}

  void text(const std::string& t);
  void letter(const std::string& l);
  void stop();
  void tone(size_t freq, size_t duration);
  void pitch(size_t value);
  void rate(size_t value);
  void volume(size_t value);
  void family(const std::string& value);
  void punc(const std::string& mode);

  void setFamily(const std::string& family)
  {
    m_family = family;
  }

private:
  bool connectionAvailable();

private:
  vm_connection_t m_con;
  size_t m_lastPitchValue, m_lastRateValue, m_lastVolumeValue;
  std::string m_family;
}; //class Connection;

#endif //__VOICEMAN_EMACSPEAK_CONNECTION_H__;
