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

#include"voiceman.h"
#include"RusLang.h"

static void addVectorItem(const std::string& str, WStringVector& items)
{
  std::string s = getDelimitedSubStr(str, 1, ':');
  if (trim(s).empty())
    {
      logMsg(LOG_WARNING, "There is line with empty second item in file with Russian constants");
      return;
    }
  if (!checkTypeUnsignedInt(s))
    {
      logMsg(LOG_WARNING, "There is line with an invalid second item in file with Russian constants. \'%s\' is not a valid unsigned integer number.", trim(s).c_str());
    return;
    }
  WStringVector::size_type index = parseAsUnsignedInt(s);
  if (items.size() <= index)
    items.resize(index + 1);
  items[index] = readUTF8(getDelimitedSubStr(str, 2, ':'));
}

void RusLang::load(const std::string& fileName)
{
  logMsg(LOG_DEBUG, "Loading Russian language constants from %s", fileName.c_str());
  std::string fileText = readTextFile(fileName);
  fileText = cutComments(fileText);
  StringList lines;
  splitTextFileLines(fileText, lines, 1, 1);//1 and 1 means trim lines and skip empty;
  for(StringList::const_iterator it = lines.begin();it != lines.end();it++)
    {
      std::string t = trim(::toLower(getDelimitedSubStr(*it, 0, ':')));
      if (t.empty())
	{
	  logMsg(LOG_WARNING, "There is line with empty first item in file with Russian constants");
	  continue;
	}
      if (t == "zero")
	{
	  if (!decodeUTF8(getDelimitedSubStr(*it, 1, ':'), 	m_zero))
	    logMsg(LOG_WARNING, "Value for \'zero\' in file with Russian constants contains an illegal UTF-8 sequence.");
	} else
      if (t == "vowels")
	{
	  if (!decodeUTF8(getDelimitedSubStr(*it, 1, ':'), 	m_vowels))
	    logMsg(LOG_WARNING, "Value for \'vowels\' in file with Russian constants contains an illegal UTF-8 sequence.");
	} else
      if (t == "chars")
	{
	  if (!decodeUTF8(getDelimitedSubStr(*it, 1, ':'), m_chars))
	    logMsg(LOG_WARNING, "Value for \'chars\' in file with Russian constants contains an illegal UTF-8 sequence.");
	} else
      if (t == "mlrds")
	addVectorItem(*it, m_mlrds); else
      if (t == "mlns")
	addVectorItem(*it, m_mlns); else
      if (t == "thnds")
	addVectorItem(*it, m_thnds); else
      if (t == "hundreds")
	addVectorItem(*it, m_hundreds); else
      if (t == "decimals")
	addVectorItem(*it, m_decimals); else
      if (t == "tens")
	addVectorItem(*it, m_tens); else
      if (t == "ones")
	addVectorItem(*it, m_ones); else
      if (t == "onesf")
	addVectorItem(*it, m_onesF); else
      if (t == "map")
	{
	  std::wstring v1, v2;
	  v1 = trim(readUTF8(getDelimitedSubStr(*it, 1, ':')));
	  v2 = trim(readUTF8(getDelimitedSubStr(*it, 2, ':')));
	  if (v1.length() != 1 || v2.length() != 1)
	    {
	      logMsg(LOG_WARNING, "File with Russian constants contains map item with bad strings. Length of string in such item should be one character. Line: \'%s\'", it->c_str());
	      continue;
	    }
	  m_toUpper[v2[0]] = v1[0];
	  m_toLower[v1[0]] = v2[0];
	} else
	logMsg(LOG_WARNING, "There is line with unknown first item \'%s\' in the file with Russian constants", t.c_str());
    } //for(lines);
}

Lang::CharType RusLang::getCharType(wchar_t c) const
{
  WCharToWCharMap::const_iterator it;
  it = m_toUpper.find(c);
  if (it != m_toUpper.end())
    return Lang::LowCase;
  it = m_toLower.find(c);
  if (it != m_toLower.end())
    return Lang::UpCase;
  return Other;
}

std::wstring RusLang::getAllChars() const
{
  return m_chars;
}

bool RusLang::equalChars(wchar_t c1, wchar_t c2) const
{
  return toLower(c1) == toLower(c2);
}

wchar_t RusLang::toUpper(wchar_t ch) const
{
  WCharToWCharMap::const_iterator it = m_toUpper.find(ch);
  if (it == m_toUpper.end())
    return ch;
  return it->second;
}

wchar_t RusLang:: toLower(wchar_t ch) const
{
  WCharToWCharMap::const_iterator it = m_toLower.find(ch);
  if (it == m_toLower.end())
    return ch;
  return it->second;
}

std::wstring RusLang::toUpper(const std::wstring& str) const
{
  std::wstring s;
  for(std::wstring::size_type i = 0;i < str.length();i++)
    s += toUpper(str[i]);
  return s;
}

std::wstring RusLang::toLower(const std::wstring& str) const
{
  std::wstring s;
  for(std::wstring::size_type i = 0;i < str.length();i++)
    s += toLower(str[i]);
  return s;
}

std::wstring RusLang::processHundred(const std::wstring& inStr, const WStringVector& items, bool female) const
{
  std::wstring s, str = inStr;
  assert(str.length()<=3);
  while(str.length() < 3)
    str = L'0' + str;
  std::string::size_type i;
  for(i = 0;i < 3;i++)
    if (str[i] != '0')
      break;
  if (i == str.length())
    return std::wstring();
  if (str[0] != '0')
    attachString(s, m_hundreds[str[0] - '0']);
  if (str[1] != '0' && str[1] != '1')
    attachString(s, m_decimals[str[1] - '0']);
  if (str[1] == '1')
    {
      attachString(s, m_tens[str[2] - '0']);
    } else
    {
      if (str[2] != '0')
	{
	  if (female)
	    attachString(s, m_onesF[str[2] - '0']); else
	    attachString(s, m_ones[str[2] - '0']);
	}
    }
  if (items.empty())
    return s;
  if (str[1] == '1')
    attachString(s, items[2]); else
    {
      if (str[2] == '1')
	attachString(s, items[0]); else
	if (str[2] >= '2' && str[2] <= '4')
	  attachString(s, items[1]); else
	  attachString(s, items[2]);
    }
  return s;
}

std::wstring RusLang::digitsToWords(const std::wstring& inStr) const
{
  WStringVector sList;
  std::wstring str;
  assert(!inStr.empty());
  std::wstring::size_type i;
  for(i = 0;i < inStr.length();i++)
    {
      assert(inStr[i] >= '0' && inStr[i] <= '9');
      if (inStr[i] != '0')
	break;
    }
  if (i == inStr.length())
    return m_zero;
  bool accepting = 0;
  for(i = 0;i < inStr.length();i++)
    {
      if (inStr[i] != '0')
	accepting = 1;
      if (accepting)
	str += inStr[i];
    }
  while(str.length())
    {
      if (str.length() >= 3)
	{
	  std::wstring ss;
	  ss += str[str.length() - 3];
	  ss += str[str.length() - 2];
	  ss += str[str.length() - 1];
	  sList.push_back(ss);
	  str.resize(str.size() - 3);
	  continue;
	}
      if (str.length() == 2)
	{
	  std::wstring ss;
	  ss += str[0];
	  ss += str[1];
	  sList.push_back(ss);
	  str.erase();
	  continue;
	}
      if (str.length() == 1)
	{
	  std::wstring ss;
	  ss = str[0];
	  sList.push_back(ss);
	  str.erase();
	  continue;
	}
    }
  str.erase();
  for(int j=sList.size()-1;j>=0;j--)//must be signed;
    {
      if (j > 3)
	attachString(str, processHundred(sList[j], WStringVector(), 0) ); else
	if (j == 3)
	  attachString(str, processHundred(sList[j], m_mlrds, 0) ); else
	  if (j == 2)
	    attachString(str, processHundred(sList[j], m_mlns, 0)); else
	    if (j == 1)
	      attachString(str, processHundred(sList[j], m_thnds, 1)); else
	      attachString(str, processHundred(sList[j], WStringVector(), 0));
    }
  return str;
}

void RusLang::expandNumbers(std::wstring& str, bool singleDigits) const
{
  std::wstring inStr = str;
  str.erase();
  if (singleDigits)
    {
      bool b = 0;
      for(std::wstring::size_type i = 0;i < inStr.length();i++)
	{
	  if (DIGIT_CHAR(inStr[i]))
	    {
	      b = 1;
	      if (inStr[i] == '0')
		attachString(str, m_zero); else
		attachString(str, m_ones[inStr[i]-'0']);
	    } else
	    {
	      if (b)
		{
		  str += ' ';
		  b = 0;
		}
	      str += inStr[i];
	    }
	}
      return;
    }
  bool d = 0;
  std::wstring sStr;
  for(std::wstring::size_type i = 0;i < inStr.length();i++)
    {
      if (DIGIT_CHAR(inStr[i]))
	{
	  d = 1;
	  sStr += inStr[i];
	} else
	{
	  if (d)
	    {
	      attachString(str, digitsToWords(sStr));
	      d = 0;
	      sStr.erase();
	      str += ' ';
	    }
	  str += inStr[i];
	}
    }
  if (d)
    attachString(str, digitsToWords(sStr));
}

std::wstring RusLang::separate(const std::wstring& text) const
{
  std::wstring s;
  for(std::wstring::size_type i = 0;i < text.length();i++)
    {
      if (i != 0 && getCharType(text[i]) == Lang::UpCase && getCharType(text[i - 1]) == Lang::LowCase)
	s += ' ';
      s += text[i];
    } //for();
  return s;
}

void RusLang::markCapitals(const std::wstring& text, std::vector<bool>& marks) const
{
  assert(text.length() == marks.size());
  StringIterator<std::wstring> i(text, getAllChars());
  while(i.next())
    {
      if (i.str().length() < 2)
	continue;
      if (!contains<std::wstring>(toLower(i.str()), m_vowels))
	{
	  for(std::wstring::size_type j = i.start();j < i.end();j++)
	    marks[j] = 1;
	}
    } //while();
}
