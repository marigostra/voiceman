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
#include"EngLang.h"

#define ENG_LETTERS L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ENG_ZERO L"zero"

static const wchar_t* const engOnes[10] = {L"",
					   L"one",
					   L"two",
					   L"three",
					   L"four",
					   L"five",
					   L"six",
					   L"seven",
					   L"eight",
					   L"nine"
};

static const wchar_t* const engTens[10] = {L"ten",
					   L"eleven",
					   L"twelve",
					   L"thirteen",
					   L"fourteen",
					   L"fifteen",
					   L"sixteen",
					   L"seventeen",
					   L"eighteen",
					   L"nineteen"
};

static const wchar_t* const engDecimals[10] = {L"", L"",
					       L"twenty",
					       L"thirty",
					       L"forty",
					       L"fifty",
					       L"sixty",
					       L"seventy",
					       L"eighty",
					       L"ninety"
};

static const wchar_t* const engMlrds[2] = {
  L"milliard",
  L"milliards"
};

static const wchar_t* const engMlns[2] = {
  L"million",
  L"millions"
};

static const wchar_t* const engThnds[2] = {
  L"thousand",
  L"thousands"
};

static const wchar_t* const engHundreds[2] = {
  L"hundred",
  L"hundreds"
};

Lang::CharType EngLang::getCharType(wchar_t c) const
{
  if (c >= 'a' && c <= 'z')
    return Lang::LowCase;
  if (c >= 'A' && c <= 'Z')
    return Lang::UpCase;
  return Lang::Other;
}

std::wstring EngLang::getAllChars() const
{
  return ENG_LETTERS;
}

bool EngLang::equalChars(wchar_t c1, wchar_t c2) const
{
  return toLower(c1) == toLower(c2);
}

wchar_t EngLang::toUpper(wchar_t ch) const
{
  if (getCharType(ch) == Lang::LowCase)
    return 'A' + (ch - 'a');
  return ch;
}

wchar_t EngLang::toLower(wchar_t ch) const
{
  if (getCharType(ch) == Lang::UpCase)
    return 'a' + (ch - 'A');
  return ch;
}

std::wstring EngLang::toUpper(const std::wstring& str) const
{
  std::wstring newStr;
  for(std::wstring::size_type i = 0;i < str.length();i++)
    newStr += toUpper(str[i]);
  return newStr;
}

std::wstring EngLang::toLower(const std::wstring& str) const
{
  std::wstring newStr;
  for(std::wstring::size_type i = 0;i < str.length();i++)
    newStr += toLower(str[i]);
  return newStr;
}

std::wstring EngLang::processHundred(const std::wstring& inStr, const wchar_t* const items[]) const
{
  std::wstring s;
  std::wstring str = inStr;
  assert(str.length() <= 3);
  std::wstring::size_type i;
  for(i = 0;i < str.length();i++)
    {
      assert(str[i] >= '0' && str[i] <= '9');
      if (str[i] != '0')
	break;
    }
  if (i >= str.length())
    return std::wstring();
  while(str.length() < 3)
    str = L'0' + str;
  if (str[0] != '0')
    {
      attachString<std::wstring>(s, engOnes[str[0] - '0']);
      if (str[0] == '1')
	attachString<std::wstring>(s, engHundreds[0]); else
	attachString<std::wstring>(s, engHundreds[1]);
      if (str[1] != '0' || str[2] != '0')
	attachString<std::wstring>(s, L"and");
    }
  if (str[1] != '0' && str[1] != '1')
    attachString<std::wstring>(s, engDecimals[str[1] - '0']);
  if (str[1] == '1')
    attachString<std::wstring>(s, engTens[str[2] - '0']); else
    {
      attachString<std::wstring>(s, engOnes[str[2] - '0']);
    }
  if (!items)
    return s;
  if (str[1] == '1')
    attachString<std::wstring>(s, items[1]); else
    if (str[2] == '0')
      attachString<std::wstring>(s, items[1]); else
      if (str[2] == '1')
	attachString<std::wstring>(s, items[0]); else
	attachString<std::wstring>(s, items[1]);
  return s;
}

std::wstring EngLang::digitsToWords(const std::wstring& inStr) const
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
  if (i >= inStr.length())
    return ENG_ZERO;
  bool accepting = 0;
  for(i = 0;i < inStr.length();i++)
    {
      if (inStr[i] != '0')
	accepting = 1;
      if (accepting)
	str += inStr[i];
    }
  assert(accepting);
  while(str.length())
    {
      if (str.length() >= 3)
	{
	  std::wstring ss;
	  ss += str[str.length() - 3];
	  ss  += str[str.length() - 2];
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
      assert(0);
    }
  str.erase();
  for(int j = sList.size() - 1;j >= 0;j--)//Loop counter must be exactly signed;
    {
      if (j > 3)
	attachString<std::wstring>(str, processHundred(sList[j], NULL)); else
	if (j == 3)
	  attachString<std::wstring>(str, processHundred(sList[j], engMlrds)); else
	  if (j == 2)
	    attachString<std::wstring>(str, processHundred(sList[j], engMlns)); else
	    if (j == 1)
	      attachString<std::wstring>(str, processHundred(sList[j], engThnds)); else
	      attachString<std::wstring>(str, processHundred(sList[j], NULL));
    }
  return str;
}

void EngLang::expandNumbers(std::wstring& str, bool singleDigits) const
{
  const std::wstring inStr = str;
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
		attachString<std::wstring>(str, ENG_ZERO); else
		attachString<std::wstring>(str, engOnes[inStr[i] - '0']);
	      continue;
	    } //it was a digit;
	  if (b)//previous character was a digit;
	    {
	      str += ' ';
	      b = 0;
	    }
	  str += inStr[i];
	} //for();
      return;
    } //single digits;
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
	      attachString<std::wstring>(str, digitsToWords(sStr));
	      d = 0;
	      sStr.erase();
	      str += ' ';
	    }
	  str += inStr[i];
	}
    }
  if (d)
    attachString<std::wstring>(str, digitsToWords(sStr));
}

std::wstring EngLang::separate(const std::wstring& text) const
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

bool EngLang::checkCapList(const std::wstring& str, std::wstring::size_type pos, std::wstring& result) const
{
  for(CapItemList::const_iterator i = m_capItems.begin();i != m_capItems.end();i++)
    {
      std::wstring s = toLower(i->str);
      if (str.length() < s.length() + pos)//must be (str.length() - pos < s.length()), but it can cause sign problems;
	continue;
      std::wstring::size_type j;
      for(j = 0;j < s.length();j++)
      if (str[pos + j] != s[j])
	  break;
      if (j < s.length())
	continue;
      if (!i->before && pos > 0 && getCharType(str[pos - 1]) != Lang::Other)
	continue;
      if (!i->after && pos+s.length() < str.length() && getCharType(str[pos + s.length()]) != Lang::Other)
	continue;
      result = i->str;
      return 1;
    } // for();
  return 0;
}

void EngLang::processCapList(const std::wstring& str, BoolVector& marks) const
{
  assert(marks.size() >= str.length());
  for(std::wstring::size_type i = 0;i < str.length();i++)
    {
      std::wstring s;
      if (!checkCapList(toLower(str), i, s))
	continue;
      for(std::wstring::size_type j = 0;j < s.length();j++)
	if (getCharType(s[j]) == Lang::UpCase)// Capital letter;
	  marks[i + j]=1;
      i += s.length() - 1;
    } // for;
}

void EngLang::markCapitals(const std::wstring& text, BoolVector& marks) const
{
  assert(text.length() == marks.size());
  StringIterator<std::wstring> i(text, getAllChars());
  while(i.next())
    {
      if (i.str().length() < 2)
	continue;
      if (!contains<std::wstring>(i.str(), L"eEuUiIoOaAyY"))
	{
	  for(std::wstring::size_type j = i.start();j < i.end();j++)
	    marks[j] = 1;
	}
    } //while();
  processCapList(text, marks);
}

void EngLang::loadCaps(const std::string& fileName)
{
  std::string text = readTextFile(fileName);
  text = cutComments(text);
  StringList lines;
  splitTextFileLines(text, lines, 1, 1);// 1 and 1 means trim lines and skip empty;
  for(StringList::const_iterator it = lines.begin();it != lines.end();it++)
    {
      const std::string& s = *it;
      std::string::size_type l, r;
      assert(!trim(s).empty());
      if (s[0] == '+')
	l = 1; else
	l = 0;
      if (s[s.length() - 1] == '+')
	r = s.length() - 1; else
	r=s.length();
      if (l >= r)
	{
	  logMsg(LOG_WARNING, "Skipping cap line for eng language ('%s')", s.c_str());
	  continue;
	}
      std::string value;
      std::string::size_type i;
      for(i = l;i < r;i++)
	{
	  if (getCharType(s[i]) == Lang::Other)
	    break;
	  value += s[i];
	}
      if (i < r)
	{
	  logMsg(LOG_WARNING, "Line in caps file for eng language has incorrect format ('%s')", s.c_str());
	  continue;
	}
      m_capItems.push_back(CapItem(readUTF8(value), s[0] == '+', s[s.length() - 1] == '+'));
    } //for(lines);
}
