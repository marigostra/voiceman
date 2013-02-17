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
#include"TextProcessor.h"

#define CAP_OVERHEAD 50

void TextProcessor::split(const std::wstring& text, TextItemList& items) const
{
  items.clear();
  bool hasCurrentLangId = 0;
  LangId currentLangId = LANG_ID_NONE;
  std::wstring currentText;
  for(std::wstring::size_type i = 0;i < text.length();i++)
    {
      const wchar_t let = text[i];
      if (BLANK_CHAR(let))
	{
	  if (!hasCurrentLangId)//it is not a first char in item;
	    continue;
	  attachSpace(currentText);
	  continue;
	} // space;
      WCharToLangIdMap::const_iterator it = m_charsTable.find(let);
      if (it == m_charsTable.end())
	continue;//character is not present in characters table and can be silently skipped;
      if (it->second == LANG_ID_NONE)//the default language must be used for this letter;
	{
	  if (hasCurrentLangId)
	    {
	      currentText += let;
	      continue;
	    }
	  if (m_defaultLangId == LANG_ID_NONE)//default language is not set, we cannot handle current letter;
	    continue;
	  currentLangId = m_defaultLangId;
	  hasCurrentLangId = 1;
	  currentText += let;
	  continue;
	} //char of the default language;for the default output;
      const LangId langId = it->second;
      if (hasCurrentLangId && currentLangId != langId)
	{
	  items.push_back(TextItem(currentLangId, currentText));
	  currentText.erase();
	}
      hasCurrentLangId = 1;
      currentLangId = langId;
      currentText += let;
    } //for;
  if (!trim(currentText).empty())
    {
      assert(hasCurrentLangId);
      items.push_back(TextItem(currentLangId, currentText));
    }
}

void TextProcessor::processLetter(wchar_t c, TextParam volume, TextParam pitch, TextParam rate, TextItemList& items) const
{
  LangId langId = LANG_ID_NONE;
  WCharToLangIdMap::const_iterator it = m_charsTable.find(c);
  if (it != m_charsTable.end())
    {
      if (it->second == LANG_ID_NONE)//it is letter for defautl language;
    langId = m_defaultLangId; else
	langId = it->second;
    } //there is entry in characters table;
  if (langId == LANG_ID_NONE)//we cannot determine the language for this letter, probable it has special value;
    {
      WCharToWStringMap::const_iterator specialValueIt=m_specialValues.find(c);
      if (specialValueIt != m_specialValues.end())
	process(TextItem(specialValueIt->second, volume, pitch, rate), items);
      return;
    }
  const Lang* lang = getLangById(langId);
  TextParam p = pitch;
  if (lang != NULL && lang->getCharType(c) == Lang::UpCase)
    p+=CAP_OVERHEAD;
  WCharToWStringMap::const_iterator specialValueIt=m_specialValues.find(c);
  if (specialValueIt != m_specialValues.end())
    {
      process(TextItem(specialValueIt->second, volume, p, rate), items);
      return;
    }
  std::wstring s;
  s += c;
  TextItem item(langId, s, volume, p, rate);
  item.mark(0);
  items.clear();
  items.push_back(item);
}

void TextProcessor::process(const TextItem& text, TextItemList& items) const
{
  split(text.getText(), items);
  logMsg(LOG_DEBUG, "Splitter produced %u item(s)", items.size());
  TextItemList::iterator it;
  for(it = items.begin();it != items.end();it++)
    {
      it->setVolume(text.getVolume());
      it->setPitch(text.getPitch());
      it->setRate(text.getRate());
      processItem(*it);
    }
}

void TextProcessor::processItem(TextItem& text) const
{
  BoolVector marks;
  std::wstring toSend = insertReplacements(text.getText(), text.getLangId());
  const Lang* lang = getLangById(text.getLangId());
  if (lang != NULL)
    {
      if (m_separation)
	toSend=lang->separate(toSend);
      switch(m_digitsMode)
	{
	case DigitsModeNormal:
	  logMsg(LOG_DEBUG, "Performing normal digits processing");
	  lang->expandNumbers(toSend, 0);
	  break;
	case DigitsModeSingle:
	  logMsg(LOG_DEBUG, "Performing digits processing in single-digits mode");
	  lang->expandNumbers(toSend, 1);
	  break;
	case DigitsModeNone:
	  logMsg(LOG_DEBUG, "Skipping digits processing because of none mode");
	  break;
	default:
	  logMsg(LOG_WARNING, "Found unexpected digits mode (%d)", m_digitsMode);
	  assert(0);
	}
      marks.resize(toSend.length());
      for(BoolVector::size_type i = 0;i < marks.size();i++)
	marks[i] = 0;
      if (m_capitalization)
	lang->markCapitals(toSend, marks);
    } else
    logMsg(LOG_WARNING, "Processing text items without language information");
  text.setText(toSend);
  for(BoolVector::size_type i = 0;i < marks.size();i++)
    if (marks[i])
      text.mark(i);
}

bool TextProcessor::findReplacement(const std::wstring& str, std::wstring::size_type pos, LangId langId, ReplacementVector::size_type& result) const
{
  const Lang* lang = getLangById(langId);
  if (!lang)
    return 0;
  //Case sensitive search;
  for(ReplacementVector::size_type i = 0;i < m_replacements.size();i++)
    {
      if (m_replacements[i].langId != langId)
	continue;
      if (str.length() - pos < m_replacements[i].oldValue.length())
	continue;
      std::wstring::size_type j;
      for(j = 0;j < m_replacements[i].oldValue.length();j++)
	if (str[pos + j] != m_replacements[i].oldValue[j])
	    break;
      if (j == m_replacements[i].oldValue.length())
	{
	  result = i;
	  return 1;
	}
    } //for(replacements);
  //Case insensitive search;
  for(ReplacementVector::size_type i = 0;i < m_replacements.size();i++)
    {
      if (m_replacements[i].langId != langId)
	continue;
      if (str.length() - pos < m_replacements[i].oldValue.length())
	continue;
      std::wstring::size_type j;
      for(j = 0;j < m_replacements[i].oldValue.length();j++)
	if (!lang->equalChars(str[pos + j], m_replacements[i].oldValue[j]))
	    break;
      if (j == m_replacements[i].oldValue.length())
	{
	  result = i;
	  return 1;
	}
    } //for(replacements);
  return 0;
}

std::wstring TextProcessor::insertReplacements(const std::wstring& str, LangId langId) const
{
  std::wstring newStr;
  for(std::wstring::size_type i = 0;i < str.length();i++)
    {
      ReplacementVector::size_type k;
      if (findReplacement(str, i, langId, k))
	{
	  assert(k < m_replacements.size());
	  const std::wstring& newValue = m_replacements[k].newValue;
	  if (newValue.length() > 1)
	    {
	      if (BLANK_CHAR(newValue[0]))
		attachSpace(newStr);
	      newStr += trim(newValue);
	      if (BLANK_CHAR(newValue[newValue.length() - 1]))
		attachSpace(newStr);
	    } else 
	    newStr += newValue;
	  i += m_replacements[k].oldValue.length()-1;
	} else
	attachCharWithoutDoubleSpaces(newStr, str[i]);
    }
  return newStr;
}

void TextProcessor::addReplacement(LangId langId, const std::wstring& oldValue, const std::wstring& newValue)
{
  m_replacements.push_back(Replacement(langId, oldValue, newValue));
}

void TextProcessor::setMode(int digitsMode, bool capitalization, bool separation)
{
  m_digitsMode = digitsMode;
  m_capitalization = capitalization;
  m_separation = separation;
}

const Lang* TextProcessor::getLangById(LangId langId) const
{
  return m_langIdResolver.getLangById(langId);
}

void TextProcessor::setDefaultLangId(LangId langId)
{
  assert(langId != LANG_ID_NONE);
  m_defaultLangId = langId;
}

void TextProcessor::setSpecialValueFor(wchar_t c, const std::wstring& value)
{
  m_specialValues[c] = trim(value);
}

void TextProcessor::associate(const std::wstring& str, LangId langId)
{
  for(std::wstring::size_type i=0;i<str.length();i++)
    m_charsTable[str[i]] = langId;
}

auto_ptr<AbstractTextProcessor> createNewTextProcessor(const AbstractLangIdResolver& langIdResolver, int digitsMode, bool capitalization, bool separation)
{
  auto_ptr<TextProcessor> processor(new TextProcessor(langIdResolver));
  processor->setMode(digitsMode, capitalization, separation);
  return auto_ptr<AbstractTextProcessor>(processor.release());
}
