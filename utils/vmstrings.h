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

#ifndef __VOICEMAN_STRINGS_H__
#define __VOICEMAN_STRINGS_H__

#define BLANK_CHAR(x) ((x)==10 || (x)==13 || (x)==9 || (x)==32)
#define DIGIT_CHAR(x) ((x)>='0' && (x)<='9')

/**\brief Merges two parts of file path in UNIX filesystem
 *
 * This function takes two strings and merges them into one avoiding
 * slash dublicating in the merging place.
 *
 * \param part1 The first part to merge
 * \param part2 The second part to merge
 *
 * \return The merged path string
 */
template<class TStr>
TStr concatUnixPath(const TStr& part1, const TStr& part2)
{
  if (part1.empty() && part2.empty())
    return TStr();
  if (part1.empty())
    return part2;
  if (part2.empty())
    return part1;
  const typename TStr::value_type lastChar1 = part1[part1.length() - 1], firstChar2 = part2[0];
  if (lastChar1 != '/' && firstChar2 != '/')
    {
      TStr res = part1;
      res += '/';
      res += part2;
      return res;
    }
  if ((lastChar1 == '/' && firstChar2 != '/') || (lastChar1 != '/' && firstChar2 == '/'))
    return part1 + part2;
  assert(lastChar1 == '/' && firstChar2 == '/');
  TStr res = part1;
  res.resize(res.length() - 1);
  res += part2;
  return part1;
}

/**\brief Removes all characters to begin new line from the specified string
 *
 * Use this function to remove all new line characters from the
 * string. It usually used to prepare string to logging.
 * 
 * \param [in/out] str The string to process
 */
template<class TStr>
void removeNewLineChars(TStr& str)
{
  typename TStr::size_type offset = 0;
  for(typename TStr::size_type i = 0;i < str.length();i++)
    {
      if (str[i] == '\n' || str[i] == '\r')
	{
	  offset++;
	  continue;
	}
      assert(offset <= i);
      if (offset > 0)
	str[i - offset] = str[i];
    }
  assert(offset <= str.length());
  str.resize(str.size() - offset);
}

/**\brief Checks if the string contains another one at the beginning
 *
 * This function checks if the specified string has the same characters
 * at the beginning as the characters of another string. It also returns
 * the string part after the checked sequence.
 *
 *
 * \param [in] str The string to check beginning of
 * \param [in] headToCheck The string to compare with
 * \param [out] tail The ending part of checked string
 *
 * \return Non-zero If the string contains another one at the beginning
 */
template<class TStr>
bool stringBegins(const TStr& str, const TStr& headToCheck, TStr& tail)
{
  if (str.length() < headToCheck.length())
    return 0;
  typename TStr::size_type i;
  for(i = 0;i < headToCheck.size();i++)
    if (str[i] != headToCheck[i])
      return 0;
  //now copying left characters to tail parameter;
  tail.clear();
  while(i < str.length())
    tail += str[i++];
  return 1;
}

/**\brief Removes blank characters from string beginning and end
 *
 * Use this function to reduce string length by removing needless blank
 * characters from string beginning and end.
 *
 * \param [in] str The string to process
 *
 * \return The provided string without blank characters at the beginning and end
 */
template<class T>
T trim(const T& str)
{
  //FIXME:Optimization;
  typename T::size_type l1=0, l2 = str.length();
  while(l1 < str.length() && BLANK_CHAR(str[l1]))
    l1++;
  while(l2 > l1 && BLANK_CHAR(str[l2-1]))
    l2--;
  T newStr;
  for(typename T::size_type i = l1;i < l2;i++)
    newStr += str[i];
  return newStr;
}

/**\brief Adjusts all English characters to upper case
 *
 * This function replaces English low case letters with their upper case
 * variants. Characters from outside of English alphabet are left
 * unchanged.
 *
 * \param [in] str The string to process
 *
 * \return The string with adjusted characters
 */
template<class T>
T toUpper(const T& str)
{
  typename T::size_type i;
  T s=str;
  for(i = 0;i < s.length();i++)
    if (s[i] >= 'a' && s[i] <= 'z')
      s[i] = 'A' + (s[i] - 'a');
  return s;
}

/**\brief Adjusts all English characters to low case
 *
 * This function replaces English upper case letters with their low case
 * variants. Characters from outside of English alphabet are left
 * unchanged.
 *
 * \param [in] str The string to process
 *
 * \return The string with adjusted characters
 */
template<class T>
T toLower(const T& str)
{
  typename T::size_type i;
  T s = str;
  for(i = 0;i < s.length();i++)
    if (s[i] >= 'A' && s[i] <= 'Z')
      s[i] = 'a'+(s[i] - 'A');
  return s;
}

/**\brief Checks if string contains characters from another string
 *
 * This function looks through provided string and checks if this string
 * contains one of the character set specified by another string.
 *
 * \param [in] str The string to check
 * \param [in] s The string to get character set from
 *
 * \return Non-zero if string contains one of the character
 */
template<class T>
bool contains(const T& str, const T& s)
{
  //Maybe not effective implementation;
  typename T::size_type i;
  for(i = 0;i < str.length();i++)
    {
      typename T::size_type j;
      for(j = 0;j < s.length();j++)
	if (str[i] == s[j])
	  return 1;
    }
  return 0;
}

/**\brief Builds string object of required type with specified content
 *
 * This function creates string object of the type specified by template
 * argument and fills it with data stored in unibyte sequence. Basically
 * this function was created for using in template code.
 *
 * \param [in] str The initial content for new string
 *
 * \return The string object of the specified type
 */
template<class T>
T adjust(const char* str)
{
  assert(str!=NULL);
  T s;
  for(size_t i = 0;str[i] != '\0';i++)
    s += str[i];
  return s;
}

/**\brief Checks if string contains one of the valid boolean values
 *
 * This function checks if the specified string contains any boolean
 * values. An empty string is not assumed as valid boolean value.
 *
 * \param [in] s The string to check
 *
 * \return Non-zero if string contains valid boolean value
 */
template<class T>
bool checkTypeBool(const T& s)
{
  T ss = trim(toLower(s));
  if (ss == adjust<T>("yes") ||
      ss == adjust<T>("no") ||
      ss == adjust<T>("true") ||
      ss == adjust<T>("false") ||
      ss == adjust<T>("0") ||
      ss == adjust<T>("1"))
    return 1;
  return 0;
}

/**\brief Checks if string contains valid unsigned integer value
 *
 * This function checks if the specified string contains unsigned integer
 * value. An empty string is not assumed as valid unsigned integer value.
 *
 * \param [in] s The string to check
 *
 * \return Non-zero if string contains valid unsigned integer value
 */
template<class T>
bool checkTypeUnsignedInt(const T& s)
{
  T ss = trim(s);
  if (ss.empty())
    return 0;
  typename T::size_type i = 0;
  if (ss[0] == '+')
    i=1;
  if (i >= ss.length())
    return 0;
  for(;i < ss.length();i++)
    if (ss[i] < '0' || ss[i] > '9')
      return 0;
  return 1;
}

/**\brief Checks if string contains valid signed integer value
 *
 * This function checks if the specified string contains signed integer
 * value. An empty string is not assumed as valid signed integer value.
 *
 * \param [in] s The string to check
 *
 * \return Non-zero if string contains valid signed integer value
 */
template<class T>
bool checkTypeInt(const T& s)
{
  T ss = trim(s);
  if (ss.empty())
    return 0;
  typename T::size_type i = 0;
  if (ss[0] == '+' || ss[0] == '-')
    i=1;
  if (i >= ss.length())
    return 0;
  for(;i < ss.length();i++)
    if (ss[i] < '0' || ss[i] > '9')
      return 0;
  return 1;
}

/**\brief Parses string with boolean value
 *
 * This function converts string to boolean value it stores. User must be
 * sure the string really contains a valid boolean value. No checking is
 * implemented.
 *
 * \param [in] s The string to parse
 *
 * \return The parsed boolean value
 */
template<class T>
bool parseAsBool(const T& s)
{
  T ss = trim(toLower(s));
  if (ss == adjust<T>("true") || ss == adjust<T>("yes") || ss == adjust<T>("1"))
    return 1;
  if (ss == adjust<T>("false") || ss == adjust<T>("no") || ss ==  adjust<T>("0"))
    return 0;
  assert(0);
  return 0;//Just to reduce warnings;
}

/**\brief Parses string with unsigned integer value
 *
 * This function converts string to unsigned integer value it stores. User must be
 * sure the string really contains a valid unsigned integer value. No checking is
 * implemented.
 *
 * \param [in] s The string to parse
 *
 * \return The parsed unsigned integer value
 */
template<class T>
unsigned int parseAsUnsignedInt(const T& s)
{
  T ss = trim(s);
  assert(!ss.empty());
  assert(checkTypeUnsignedInt(ss));
  unsigned int n = 0;
  typename T::size_type i=0;
  if (ss[0] == '+')
    i = 1;
  assert(i<ss.length());
  for(;i < ss.length();i++)
    {
      assert(ss[i] >= '0' || ss[i] <= '9');
      n *= 10;
      n += ss[i] - '0';
    }
  return n;
}

/**\brief Parses string with integer value
 *
 * This function converts string to integer value it stores. User must be
 * sure the string really contains a valid integer value. No checking is
 * implemented.
 *
 * \param [in] s The string to parse
 *
 * \return The parsed integer value
 */
template<class T>
int parseAsInt(const T& s)
{
  T ss = trim(s);
  assert(!ss.empty());
  assert(checkTypeInt(ss));
  int n = 0;
  typename T::size_type i = 0;
  bool b = 0;
  if (ss[0] == '-')
    {
      b = 1;
      i = 1;
    } else
      if (ss[0] == '+')
	i = 1;
  assert(i < ss.length());
  for(;i < ss.length();i++)
    {
      assert(ss[i] >= '0' && ss[i] <= '9');
      n *= 10;
      n += s[i]-'0';
    }
  if (b)
    n *= -1;
  return n;
}

/**\brief Checks if string contains valid double value
 *
 * This function checks if the specified string contains double
 * value. An empty string is not assumed as valid double value.
 *
 * \param [in] s The string to check
 *
 * \return Non-zero if string contains valid double value
 */
template<class T>
bool checkTypeDouble(const T& s)
{
  T ss = trim(s);
  int state=0;
  for(typename T::size_type i = 0;i < ss.length();i++)
    {
      if (state == 0 && (ss[i] == '-' || ss[i] == '+' || (ss[i] >= '0' && ss[i] <= '9')))
	{
	  state = 1;
	  continue;
	}
      if (state == 1 && ss[i] >= '0' && ss[i] <= '9')
	continue;
      if (state == 1 && ss[i] == '.')
	{
	  state=2;
	  continue;
	}
      if (state == 2 && ss[i] >= '0' && ss[i] <= '9')
	{
	  state = 3;
	  continue;
	}
      if (state == 3 && ss[i] >= '0' && ss[i] <= '9')
	continue;
      return 0;
    }// for();
  return (state==1 || state==3);
}

/**\brief Parses string with double value
 *
 * This function converts string to double value it stores. User must be
 * sure the string really contains a valid double value. No checking is
 * implemented.
 *
 * \param [in] s The string to parse
 *
 * \return The parsed double value
 */
template<class T>
double parseAsDouble(const T& s)
{
  assert(checkTypeDouble(s));
  T s1, s2, ss = trim(s);
  bool b = 0, sign = 0;
  typename T::size_type i;
  for(i = 0;i < ss.length();i++)
    {
      if (ss[i] == '.')
	{
	  b = 1;
	  continue;
	}
      if (ss[i] == '-')
	{
	  sign=1;
	  continue;
	}
      if (ss[i] < '0' || ss[i] > '9')
	continue;
      if (!b)
	s1 += ss[i];else 
	  s2 += ss[i];
    }
  assert(!s1.empty());
  double d1 = 0, d2 = 0;
  for(i = 0;i < s1.length();i++)
    {
      d1 *= 10;
      d1 += s1[i]-'0';
    }
  if (!s2.empty())
    {
      typename T::size_type z = 0;
      for(i = 0;i < s2.length();i++)
	if (s2[i] != '0')
	  z = i;
      if (s2[z] != '0')
	{
	  for(i = 0;i <= z;i++)
	    {
	      d2 *= 10;
	      d2 += s2[i]-'0';
	    }
	  for(i = 0;i <= z;i++)
	    d2 /= 10;
	}
    }
  d1 += d2;
  if (sign)
    d1*= -1;
  return d1;
}

/**\brief Constructs string representing double value
 *
 * This function creates new string object with value from double
 * variable. You can specify the desired number of digits afdter decimal
 * dot.
 *
 * \param [in] value The double value to convert to string
 * \param [in] format The desired number of digits after decimal dot
 *
 * \return The constructed string object
 */
template<class T>
T makeStringFromDouble(double value, typename T::size_type format)
{
  assert(format >= 0 && format <= 10);
  double ff = value;
  typename T::size_type i;
  for(i = format;i > 0;i--)
    ff *= 10;
  long c = (long)ff;
  bool sign = c < 0;
  if (sign)
    c *= -1;
  std::ostringstream ss;
  ss << c;
  T s = adjust<T>(ss.str().c_str());
  while(s.length() < format + 1)
    s = '0' + s;
  if (sign)
    s = '-' + s;
  if (format == 0)
    return s;
  assert(s.length() > format);
  typename T::size_type dotPosition = s.length() - format;//here was -1;
  s += ' ';
  for(typename T::size_type si = s.length() - 1;si >= dotPosition + 1;si--)
    s[si] = s[si - 1];
  s[dotPosition] = '.';
  return s;
}

/**\brief Attaches space to string end
 *
 * This function attaches space character to the string end. New space is
 * attached only if string is not empty and if last character in it is
 * not blank.
 *
 * \param [in/out] str The string to add space to
 */
template<class T>
void attachSpace(T& str)
{
  if (str.empty())
    return;
  if (BLANK_CHAR(str[str.length() - 1]))
    return;
  str += ' ';
}

/**\brief Attaches one string to another with space
 *
 * This function attaches one string to another inserting space if first
 * string doesn't end with blank character. If last character is blank,
 * string are concatenated without space. If first string is empty or
 *contains only blank characters this function just returns second one.
 *
 * \param [in/out] str The string to attach to
 * \param [in] toAttach The string to attach
 */
template<class T>
void attachString(T& str, const T& toAttach)
{
  if (trim(str).empty())
    {
      str = trim(toAttach);
      return;
    }
  if (BLANK_CHAR(str[str.length() - 1]))
    {
      str += trim(toAttach);
      return;
    }
  str += ' ';
  str += trim(toAttach);
}

/**\brief Adds new character to string preventing dublicating space characters doubling
 *
 * This function always adds provided character to string if it is not
 * blank. If provided character is blank it is added if string is not
 * empty and it is not ended with blank character.
 * \param [in/out] str The string to add to
 * \param [in] ch The character to add
 */
template<class T>
void attachCharWithoutDoubleSpaces(T& str, typename T::value_type ch)
{
  if (!BLANK_CHAR(ch))
    {
      str += ch;
      return;
    }
  if (str.empty())
    return;
  if (BLANK_CHAR(str[str.length() - 1]))
    return;
  str += ' ';
}

/**\brief Adds string to another one with space after it
 *
 * This function simply adds new string after another and one more space at the end.
 *
 * \param [in/out] str The string to add to
 * \param toAttach The string to attach
 */
template<class T>
void attachStringWithSpace(T& str, const T& toAttach)
{
  str += trim(toAttach);
  str += ' ';
}

/**\brief Adds character to string with following space 
 *
 * This function adds character to the string end with following space. If character 
 * to add is blank it is omitted.
 *
 * \param [in/out] str The string to add to
 * \param [in] ch The character to add
 */
template<class T>
void attachCharWithSpace(T& str, typename T::value_type ch)
{
  if (!BLANK_CHAR(ch))
    str += ch;
  str += ' ';
}

/**\brief Returns the substring delimited by specified character by its index 
 *
 * This function splits provided string by delimiting character and
 * returns item specified by index. If index is too large the empty
 * string is returned.
 *
 * \param [in] s The string to extract item from
 * \param [in] index The item index to extract
 * \param [in] delimiter 
 *
 * \return The delimited item by its index
 */
template<typename T>
T getDelimitedSubStr(const T& s, size_t index, char delimiter)
{
  assert(index >= 0);
  T ss = trim(s);
  typename T::size_type i;
  size_t k = 0;
  for(i = 0;i < ss.length() && k < index;i++)
    if (ss[i] == delimiter)
      k++;
  if (i >= ss.length())
    return T();
  T s1;
  for(;i < ss.length() && ss[i] != delimiter;i++)
    s1 += ss[i];
  return s1;
}

/**\brief Enumerates sequences in string of characters from specified set
 *
 * This class takes two string at the construction: first string
 * specifies string to look through, second string specifies set of
 * characters to search in first one. During the enumeration this class
 * returnes all substrings of maximum length with specified characters.
 *
 * \sa StringDelimitedIterator
 */
template<class T>
class StringIterator 
{
public:
  typedef typename T::value_type CHAR;

  /**\brief The constructor
   *
   * \param [in] str The string to look through
   * \param [in] chars The set of characters to extract substrings with
   */
  StringIterator(const T& str, const T& chars)
    : m_str(str), m_start(0), m_end(0)
  {
    assert(!chars.empty());
    for(typename T::size_type i = 0;i < chars.length();i++)
      m_chars.insert(chars[i]);
  }

  /**\brief Checks if specified character is in set of characters to find sequences with
   *
   * Use this method to check, if specified character 
   * can be included in sequences to return.
   *
   * \param [in] c The character to check
   *
   * \return Non-zero if character can be included in sequences to return
   */
    bool match(CHAR c) const
  {
    return m_chars.find(c) != m_chars.end();
  }

  /**\brief Go to next sequence of enumeration
   *
   * This method allows enumeration over all sequences of specified
   * string. Just call it until it is returned false.
   *
   * \return Non-zero if there is left sequences or zero otherwise
   */
  bool next()
  {
    assert(m_start <= m_end);
    if (m_start < m_end)//it is not a first iteration;
      m_start = m_end + 1;
    if (m_start >= m_str.length())
      return 0;
    while(m_start < m_str.length() && !match(m_str[m_start])) 
      m_start++;
    if (m_start >= m_str.length())
      return 0;
    m_end = m_start;
    while(m_end < m_str.length() && match(m_str[m_end])) 
      m_end++;
    assert(m_start < m_end);
    return 1;
  }

  /**\brief Returns current sequence
   *
   * Use this method to get value of the sequence iterator 
   * is points to.
   *
   * \return The current sequence
   */
  T str() const
  {
    assert(m_start >= 0 && m_end <= m_str.length() && m_start < m_end);
    T s;
    for(typename T::size_type i = m_start;i < m_end;i++)
      s += m_str[i];
    return s;
  }

  /**\brief Returns index of the starting character of current sequence
   *
   * Use this method to get index of starting character of the current sequence.
   *
   * \return Index of starting character of sequence in string
   */
  typename T::size_type start() const 
  { 
    assert(m_start >= 0 && m_end <= m_str.length() && m_start < m_end); 
    return m_start; 
  }

  /**\brief Returns index of the ending character of current sequence
   *
   * Use this method to get index of ending character of the current sequence.
   *
   * \return Index of ending character of sequence in string
   */
  typename T::size_type end() const 
  { 
    assert(m_start >= 0 && m_end <= m_str.length() && m_start < m_end); 
    return m_end; 
  }

private:
  const T& m_str;
  std::set<CHAR> m_chars;
  typename T::size_type m_start;
  typename T::size_type m_end;
};//class StringIterator;

/**\brief Enumerates sequences in string separated by characters from specified set
 *
 * This class takes two string at the construction: first string
 * specifies string to look through, second string specifies set of
 * characters to separate parts to enumerate. During the enumeration this class
 * returnes all substrings of maximum length separated by specified characters.
 *
 * \sa StringIterator
 */
template<class T>
class StringDelimitedIterator 
{
public:
  typedef typename T::value_type CHAR;

  /**\brief The constructor
   *
   * \param [in] str The string to look through
   * \param [in] chars The set of characters to separate substrings
   */
  StringDelimitedIterator(const T& str, const T& chars)
    : m_str(str), m_start(0), m_end(0)
  {
    assert(!chars.empty());
    for(typename T::size_type i = 0;i < chars.length();i++)
      m_chars.insert(chars[i]);
  }

  /**\brief Checks if specified character is in set of characters to find sequences with
   *
   * Use this method to check, if specified character 
   * can be included in sequences to return.
   *
   * \param [in] c The character to check
   *
   * \return Non-zero if character can be included in sequences to return
   */
    bool match(CHAR c) const
  {
    return m_chars.find(c) == m_chars.end();
  }

  /**\brief Go to next sequence of enumeration
   *
   * This method allows enumeration over all sequences of specified
   * string. Just call it until it is returned false.
   *
   * \return Non-zero if there is left sequences or zero otherwise
   */
  bool next()
  {
    assert(m_start <= m_end);
    if (m_start < m_end)//it is not a first iteration;
      m_start = m_end + 1;
    if (m_start >= m_str.length())
      return 0;
    while(m_start < m_str.length() && !match(m_str[m_start])) 
      m_start++;
    if (m_start >= m_str.length())
      return 0;
    m_end = m_start;
    while(m_end < m_str.length() && match(m_str[m_end])) 
      m_end++;
    assert(m_start < m_end);
    return 1;
  }

  /**\brief Returns current sequence
   *
   * Use this method to get value of the sequence iterator 
   * is points to.
   *
   * \return The current sequence
   */
  T str() const
  {
    assert(m_start >= 0 && m_end <= m_str.length() && m_start < m_end);
    T s;
    for(typename T::size_type i = m_start;i < m_end;i++)
      s += m_str[i];
    return s;
  }

  /**\brief Returns index of the starting character of current sequence
   *
   * Use this method to get index of starting character of the current sequence.
   *
   * \return Index of starting character of sequence in string
   */
  typename T::size_type start() const 
  { 
    assert(m_start >= 0 && m_end <= m_str.length() && m_start < m_end); 
    return m_start; 
  }

  /**\brief Returns index of the ending character of current sequence
   *
   * Use this method to get index of ending character of the current sequence.
   *
   * \return Index of ending character of sequence in string
   */
  typename T::size_type end() const 
  { 
    assert(m_start >= 0 && m_end <= m_str.length() && m_start < m_end); 
    return m_end; 
  }

private:
  const T& m_str;
  std::set<CHAR> m_chars;
  typename T::size_type m_start;
  typename T::size_type m_end;
};//class StringDelimitedIterator;

/**\brief Splits string object to the set of lines 
 *
 * This class gets set of lines (possibly incomplete) saved in one string
 * object and splits it the separate strings until it is possible. This
 * process can be easily continued later when complete data is
 * received. This class is very useful to read strings from pipe.
 */
template<class TStr>
class TextQueue
{
public:
  /**\brief The default constructor*/
  TextQueue() {}

  /**\brief The constructor with initial string specification
   *
   * \param [in] initialValue The string to split onto lines
   */
  TextQueue(const TStr& initialValue)
    : m_chain(initialValue) {}

  /**\brief Reads next line from string object
   *
   * This method checks if new line is accessible to read from string
   * object. If it is accessible it returns next line.
   *
   * \param [out] line The next line from string
   *
   * \return Non-zero if next line is accessible or zero otherwise
   */
  bool next(TStr& line)
  {
    TStr s1, s2;
    bool wasNewLine = 0;
    for(typename TStr::size_type i = 0;i < m_chain.length();i++)
      {
	if (m_chain[i] == '\r')
	  continue;
	if (m_chain[i] == '\n' && !wasNewLine)
	  {
	    wasNewLine = 1;
	    continue;
	  }
	if (!wasNewLine)
	  s1 += m_chain[i]; else 
	  s2 += m_chain[i];
      } //for();
    if (!wasNewLine)
      return 0;
    line = s1;
    m_chain = s2;
    return 1;
  }

  /**\brief Returns accessible part of incomplete line
   *
   * Use this method to get accessible part of of incomplete line.
   *
   * \return Accessible part of incomplete line 
   */
  TStr chain() const
  {
    return m_chain;
  }

private:
  TStr m_chain;
}; //class TextQueue;

#endif //__VOICEMAN_STRINGS_H__
