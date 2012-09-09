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
#include"ConfigFile.h"

static inline bool validIdentChar(char c)
{
  if (c >= 'a' && c <= 'z')
    return 1;
  if (DIGIT_CHAR(c))
    return 1;
  if (c == '_' || c == '-' || c == '.')
    return 1;
  return 0;
}

std::string readTextFile(const std::string& fileName);

static std::string collectFiles(const std::string& dirName)
{
  StringVector items;
  DIR* dir = opendir(dirName.c_str());
  if (dir == NULL)
    return std::string();
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL)
    {
      std::string name(ent->d_name);
      if (name == "." || name == "..")
	continue;
      struct stat s;
      std::string nameToCheck;
      if (dirName.empty() || dirName[dirName.length() - 1] == '/')
	nameToCheck = dirName + name; else
	nameToCheck = dirName + "/" + name;
      if (stat(nameToCheck.c_str(), &s) != 0)
	continue;
      if (!S_ISREG(s.st_mode))
	continue;
      items.push_back(name);
    }
  closedir(dir);
  std::string res;
  std::sort(items.begin(), items.end());
  for(StringVector::size_type i = 0;i < items.size();i++)
    {
      std::string nameToRead;
      if (dirName.empty() || dirName[dirName.length() - 1] == '/')
	nameToRead = dirName + items[i]; else
	nameToRead = dirName + "/" + items[i];
      char c;
      std::string s;
      std::ifstream f(nameToRead.c_str());
      if (!f)
	continue;
      while(f.get(c))
	s += c;
      s += '\n';
      res += s;
    } //for files;
  return res;
}

static std::string getExtensionsDirName(const std::string& baseName)
{
  bool dotFound = 0;
  std::string::size_type lastDotPos = 0;
  std::string::size_type i;
  for(i = 0;i < baseName.length();i++)
    {
      if (baseName[i] == '.')
	{
	  lastDotPos = i;
	  dotFound = 1;
	} else
	if (baseName[i] == '/')
	  dotFound = 0;
    }
  if (!dotFound)
    return baseName + ".d";
  std::string res;
  for(i = 0;i < lastDotPos;i++)
    res += baseName[i];
  return res + ".d";
}

ConfigFileSectionVector::size_type ConfigFile::getSectionCount() const
{
  return m_sections.size();
}

const ConfigFileSection& ConfigFile::getSection(ConfigFileSectionVector::size_type index) const
{
  assert(index < m_sections.size());
  return m_sections[index];
}

const ConfigFileSection& ConfigFile::findSection(const std::string& name) const
{
  for(ConfigFileSectionVector::size_type i = 0;i < m_sections.size();i++)
    if (m_sections[i].getName() == toLower(trim(name)))
      return m_sections[i];
  assert(0);
  return m_sections.front();
}

bool ConfigFile::hasSection(const std::string& name) const
{
  for(ConfigFileSectionVector::size_type i = 0;i < m_sections.size();i++)
    if (m_sections[i].getName() == toLower(trim(name)))
      return 1;
  return 0;
}

void ConfigFile::load(const std::string& fileName)
{
  std::string s = readTextFile(fileName);
  s += collectFiles(getExtensionsDirName(fileName));
  StringVector lines;
  std::string ss;
  for(std::string::size_type i = 0;i < s.length();i++)
    {
      if (s[i] == '\r')
	continue;
      if (s[i] == '\n')
	{
	  lines.push_back(ss);
	  ss.erase();
	  continue;
	}
      ss += s[i];
    }// lines splitting;
  lines.push_back(ss);
  ConfigFileSection sec;
  bool secInited = 0;
  for(StringVector::size_type i = 0;i < lines.size();i++)
    {
      std::string s1, s2;
      int typeCode;
      const int resultCode = process(lines[i], typeCode, s1, s2);
      if (resultCode != 0)
	throw ConfigFileException(resultCode, fileName, i + 1);
      if (typeCode == 0)//nothing;
	continue;
      if (typeCode == 1)//section head;
	{
	  if (secInited)
	    m_sections.push_back(sec);
	  secInited = 1;
	  if (s1.empty())
	    throw ConfigFileException(ConfigFileException::EmptySectionName, fileName, i + 1);
	  sec.setName(s1);
	  sec.clear();
	  continue;
	} // new section;
      if (typeCode == 2)//value;
	{
	  if (!secInited)
	    throw ConfigFileException(ConfigFileException::ParameterWithoutSection, fileName, i + 1);
	  if (sec.find(s1) != sec.end())
	    throw ConfigFileException(ConfigFileException::DublicatedKey);
	  sec.add(trim(s1), s2);
	  continue;
	} // param;
      assert(0);
    } // for lines;
  if (secInited)
    m_sections.push_back(sec);
}

/*
 * This method parses configuration file line. It returns error code or
 * zero if line was successfully parsed. An error code, returned by this
 * method can be passed to ConfigFileException object. 
 *
 * Through the code parameter this method returns determined line
 * type. The following values are used:
 *  0 - it is an empty line and must be ignored;
 *  1 - it is a section header line;
 *  2 - it is parameter value line.
 */
int ConfigFile::process(const std::string& line, int& code, std::string& str1, std::string& str2) const
{
  const std::string loLine = toLower(line);
  int state = 0;
  str1.erase();
  str2.erase();
  code = 0;
  for(std::string::size_type i = 0;i < line.length();i++)
    {
      if (state == 0 && BLANK_CHAR(line[i]))
	continue;
      if (state == 0 && line[i] == '#')
	return 0;
      if (state == 0 && line[i] == '[')
	{
	  code = 1;//it is section header;
	  //Parsing section header;
	  for(i++;i < line.length();i++)
	    {
	      if (BLANK_CHAR(line[i]))
		continue;
	      if (line[i] == ']')
		return 0;
	      if (loLine[i] >= 'a' && loLine[i] <= 'z')
		{
		  str1 += loLine[i];
		  continue;
		}
	      if (line[i] >= '0' && line[i] <= '9')
		{
		  str1 += loLine[i];
		  continue;
		}
	      return ConfigFileException::InvalidSectionHeader;
	    }  // for;
	} // section header;
      if (state == 0 && loLine[i] >= 'a' && loLine[i] <= 'z')
	{
	  str1 += loLine[i];
	  code = 2;
	  state = 1;
	  continue;
	}
      if (state == 1 && BLANK_CHAR(line[i]))
	continue;
      if (state == 1 && validIdentChar(loLine[i]))
	{
	  str1 += loLine[i];
	  continue;
	}
      if (state == 1 && line[i] == '=')
	{
	  state = 2;
	  continue;
	}
      if (state == 2 && BLANK_CHAR(line[i]))
	continue;
      if (state == 2 && validIdentChar(loLine[i]))
	{
	  for(;i < line.length();i++)
	    {
	      if (validIdentChar(loLine[i]))
		str2 += line[i]; else
		  break;
	    } // for;
	  for(;i < line.length();i++)
	    {
	      if (BLANK_CHAR(line[i]))
		continue;
	      if (line[i] == '#')
		return 0;
	      return ConfigFileException::InvalidUnquotedValue;
	    }
	  return 0;
	} // no quotas;
      if (state == 2 && line[i] == '\"')
	{
	  state = 3;
	  continue;
	}
      if (state == 3 && line[i] != '\"')
	{
	  str2 += line[i];
	  continue;
	}
      if (state == 3 && line[i] == '\"')
	{
	  state = 4;
	  continue;
	}
      if (state == 4 && line[i] == '\"')
	{
	  state = 3;
	  str2 += '\"';
	  continue;
	}
      if (state == 4 && line[i] != '\"')
	return 0;
      switch(state)
	{
	case 0:
	  return ConfigFileException::InvalidLineBeginning;
	case 1:
	  return ConfigFileException::InvalidParameterName;
	case 2:
	  return ConfigFileException::InvalidCharAfterEquals;
	default:
	  assert(0);
	}
    } // for;
  assert(state!=0 || code==0);
  if (state == 4 || state == 0)
    return 0;
  return ConfigFileException::UnexpectedLineEnd;
}

int ConfigFile::params(const std::string& section, const std::string& param) const
{
  return AnyValue;
}

void ConfigFile::checkParamsInSection(const ConfigFileSection& section) const
{
  for(ConfigFileSection::const_iterator it = section.begin();it != section.end();it++)
    {
      int requiredType = params(section.getName(), it->first);
      const std::string& value = it->second;
      if (requiredType == AnyValue)
	continue;
      if (requiredType == InvalidValue)
	throw ConfigFileValueTypeException("unknown parameter \'" + it->first + "\' in section \'" + section.getName() + "\'");
      if (requiredType == BooleanValue && !checkTypeBool(value))
	throw ConfigFileValueTypeException("parameter \'" + it->first + "\' in section \'" + section.getName() + "\' should be boolean. value \'" + trim(value) + "\' is illegal");
      if (requiredType == StringValue && trim(value).empty())
	throw ConfigFileValueTypeException("parameter \'" + it->first + "\' in section \'" + section.getName() + "\' cannot have an empty value");
      if (requiredType == IntValue && !checkTypeInt(value))
	throw ConfigFileValueTypeException("parameter \'" + it->first + "\' in section \'" + section.getName() + "\' should be an integer number. Value \'" + trim(value) + "\' is illegal");
      if (requiredType == UintValue && !checkTypeUnsignedInt(value))
	throw ConfigFileValueTypeException("parameter \'" + it->first + "\' in section \'" + section.getName() + "\' should be an unsigned integer number. Value \'" + trim(value) + "\' is illegal");
      if (requiredType == DoubleValue && !checkTypeDouble(value))
	throw ConfigFileValueTypeException("parameter \'" + it->first + "\' in section \'" + section.getName() + "\' should be an floating point number. Value \'" + trim(value) + "\' is illegal");
    } //for();
}

void ConfigFile::checkParams() const
{
  for(ConfigFileSectionVector::size_type i = 0;i < m_sections.size();i++)
    checkParamsInSection(m_sections[i]);
}
