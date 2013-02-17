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
#include"Output.h"

std::string Output::prepareSynthCommand(const TextItem& textItem)const
{
  return prepareCommandLine(m_synthCommand, textItem);
}

std::string Output::prepareAlsaPlayerCommand(const TextItem& textItem)const
{
  return prepareCommandLine(m_alsaPlayerCommand, textItem);
}

std::string Output::preparePulseaudioPlayerCommand(const TextItem& textItem)const
{
  return prepareCommandLine(m_pulseaudioPlayerCommand, textItem);
}

std::string Output::preparePcspeakerPlayerCommand(const TextItem& textItem)const
{
  return prepareCommandLine(m_pcspeakerPlayerCommand, textItem);
}

std::string Output::prepareText(const TextItem& textItem) const
{
  std::wstring text = makeCaps(textItem);
  text = insertReplacements(text);
  std::string s = encodeUTF8(text);
  s += '\n';
  return s;
}

void Output::addCapMapItem(wchar_t c, const std::wstring& value)
{
  m_capList.insert(WCharToWStringMap::value_type(c, value));
}

void Output::setPitchFormat(size_t digits, double min, double aver, double max)
{
  m_pitchFormat.digits = digits;
  m_pitchFormat.min = min;
  m_pitchFormat.aver = aver;
  m_pitchFormat.max = max;
}

void Output::setRateFormat(size_t digits, double min, double aver, double max)
{
  m_rateFormat.digits = digits;
  m_rateFormat.min = min;
  m_rateFormat.aver = aver;
  m_rateFormat.max = max;
}

void Output::setVolumeFormat(size_t digits, double min, double aver, double max)
{
  m_volumeFormat.digits = digits;
  m_volumeFormat.min = min;
  m_volumeFormat.aver = aver;
  m_volumeFormat.max = max;
}

std::wstring Output::makeCaps(const TextItem& textItem) const
{
  assert(textItem.getLangId() == m_langId);
  std::wstring oldText = textItem.getText();
  std::wstring text;
  for(std::wstring::size_type i = 0;i < oldText.length();i++)
    {
      if (textItem.isMarked(i))
	{
	  wchar_t c = oldText[i];
	  if (m_lang)
	    c = m_lang->toLower(c);
	  attachSpace(text);
	  WCharToWStringMap::const_iterator it = m_capList.find(c);
	  if (it != m_capList.end())
	    attachStringWithSpace(text, it->second); else
	      attachCharWithSpace(text, oldText[i]);
	} else
	  attachCharWithoutDoubleSpaces(text, oldText[i]);
    } //for();
  return trim(text);
}

std::string Output::prepareCommandLine(const std::string& pattern, const TextItem& textItem) const
{
  std::string volumeStr, pitchStr, rateStr;
  volumeStr = prepareFloatValue(textItem.getVolume(), m_volumeFormat);
  pitchStr = prepareFloatValue(textItem.getPitch(), m_pitchFormat);
  rateStr = prepareFloatValue(textItem.getRate(), m_rateFormat);
  std::string s;
  for(std::string::size_type i = 0;i < pattern.length();i++)
    {
      if (pattern[i] != '%' || i+1 >= pattern.length())
	{
	  s += pattern[i];
	  continue;
	}
      i++;
      switch(pattern[i])
	{
	case 'v':
	  s += volumeStr;
	  break;
	case 'p':
	  s += pitchStr;
	  break;
	case 'r':
	  s += rateStr;
	  break;
	default:
	  s += '%';
	  s += pattern[i];
	} //switch();
    } //for();
  return s;
}

std::string Output::prepareFloatValue(TextParam value, const FloatValueFormat& format) const
{
  assert(format.digits >= 0 && format.digits <= 10);
  double floatValue = value.getValue(format.min, format.aver, format.max);
  return makeStringFromDouble<std::string>(floatValue, format.digits);
}

bool Output::findReplacement(const std::wstring& str, std::wstring::size_type pos, OutputReplacementVector::size_type& result) const
{
  //Case sensitive search;
  for(OutputReplacementVector::size_type i = 0;i < m_replacements.size();i++)
    {
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
  if (m_lang == NULL)
    return 0;
  //Case insensitive search;
  for(OutputReplacementVector::size_type i = 0;i < m_replacements.size();i++)
    {
      if (str.length() - pos < m_replacements[i].oldValue.length())
	continue;
      std::wstring::size_type j;
      for(j = 0;j < m_replacements[i].oldValue.length();j++)
	if (!m_lang->equalChars(str[pos + j], m_replacements[i].oldValue[j]))
	    break;
      if (j == m_replacements[i].oldValue.length())
	{
	  result = i;
	  return 1;
	}
    } //for(replacements);
  return 0;
}

std::wstring Output::insertReplacements(const std::wstring& str) const
{
  std::wstring newStr;
  for(std::wstring::size_type i = 0;i < str.length();i++)
    {
      OutputReplacementVector::size_type k;
      if (findReplacement(str, i, k))
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
