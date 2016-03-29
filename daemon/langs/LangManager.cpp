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
#include"LangManager.h"
#include"EngLang.h"
#include"RusLang.h"

LangManager langManager;

enum {LangIdEng = 1, LangIdRus = 2};

static EngLang engLang;
static RusLang rusLang;

void LangManager::getSupportedLanguageNames(StringList& stringList) const
{
  stringList.clear();
  stringList.push_back("eng");
  stringList.push_back("rus");
}

bool LangManager::hasLanguage(const std::string& name) const
{
  if (trim(toLower(name)) == "eng")
    return 1;
  if (trim(toLower(name)) == "rus")
    return 1;
  return 0;
}

LangId LangManager::getLangId(const std::string& name) const 
{
  if (trim(toLower(name)) == "eng")
    return LangIdEng;
  if (trim(toLower(name)) == "rus")
    return LangIdRus;
  return LANG_ID_NONE;
}

std::string LangManager::getLangName(LangId langId) const
{
  if (langId == LangIdEng)
    return "eng";
  if (langId == LangIdRus)
    return "rus";
  return "";
}

const Lang* LangManager::getLangById(LangId langId) const
{
  if (langId == LangIdEng)
    return &engLang;
  if (langId == LangIdRus)
    return &rusLang;
  return NULL;
}

void LangManager::load(const std::string& dataDir)
{
  engLang.loadCaps(concatUnixPath<std::string>(dataDir, "caps"));
  rusLang.load(concatUnixPath<std::string>(dataDir, "ru_const"));
}
