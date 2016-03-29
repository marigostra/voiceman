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

#ifndef __VOICEMAN_H__
#define __VOICEMAN_H__

#include"system/system.h"
#include"vmstrings.h"
#include"Transcoding.h"
#include"system/logging.h"
#include"system/SystemException.h"
#include"system/files.h"

using std::auto_ptr;

typedef std::vector<bool> BoolVector;

typedef std::vector<std::wstring> WStringVector;
typedef std::list<std::wstring> WStringList;

typedef std::map<std::string, std::string> StringToStringMap;
typedef std::map<std::wstring, std::wstring> WStringToWStringMap;
typedef std::map<std::string, std::wstring> StringToWStringMap;

typedef std::set<char> CharSet;
typedef std::set<wchar_t> WCharSet;
  typedef std::map<wchar_t, std::wstring> WCharToWStringMap;

#define LANG_ID_NONE 0
typedef int LangId;
typedef std::set<LangId> LangIdSet;
typedef std::map<LangId, std::string> LangIdToStringMap;
typedef std::map<LangId, std::wstring> LangIdToWStringMap;

typedef int PlayerType;
enum {PlayerTypeAlsa = 0, PlayerTypePulseaudio = 1, PlayerTypePcspeaker = 2};

#endif //__VOICEMAN_H__
