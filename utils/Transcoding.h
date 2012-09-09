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

#ifndef __VOICEMAN_TRANSCODING_H__
#define __VOICEMAN_TRANSCODING_H__

/**\brief The class to perform character set translations for string objects*/
class Transcoding 
{
public:
  /**\brief The default constructor
   *
   * This constructor explicitly calls exit() function 
   * on initialization errors and prints error information.
   * This behavior was implemented to use this class as static object with 
   * automatic initialization.
   */
  Transcoding();

  /**\brief The destructor*/
  virtual ~Transcoding() {}

  /**\brief Returns the coding system used in the current environment for I/O operations*/
  std::string getIOCharset() const;

  /**\brief Converts string from default I/O charset to UNICODE
   *
   * \param [in] s The string to translate
   */
  std::wstring trIO2WString(const std::string& s) const;

  /**\brief Converts UNICODE string to the default charset of I/O operations*/
  std::string trWString2IO(const std::wstring& s) const;

  /**\brief Encodes UNICODE string with UTF-8 coding system
   *
   * \param [in] s The string to encode
   */
  std::string trEncodeUTF8(const std::wstring& s) const;

  /**\brief Decodes UTF-8 string to standart UNICODE format
   *
   * \param [in] s The string to convert 
   * \param [out] The converted string
   *
   * \return Non-zero if there no errors or zero if s is not a valid UTF-8 string
   */
  bool trDecodeUTF8(const std::string& s, std::wstring& res) const;

  /**\brief Decodes UTF-88 string with no error indications
   *
   * This method decodes string and if the input value is not 
   * a valid UTF-8 sequence it will add questions marks in illegal places
   *
   * \param [in] s The string to decode
   */
  std::wstring trReadUTF8(const std::string& s) const;

private:
  bool initIConv();
  bool initCurIO();

private:
  std::string m_curIO;
  iconv_t m_iconvIO2WString, m_iconvWString2IO, m_iconvUTF82WString, m_iconvWString2UTF8;
}; //class Transcoding;

extern Transcoding transcoding;

#define IO2WString transcoding.trIO2WString
#define WString2IO transcoding.trWString2IO
#define encodeUTF8 transcoding.trEncodeUTF8
#define decodeUTF8 transcoding.trDecodeUTF8
#define readUTF8 transcoding.trReadUTF8

std::ostream& operator <<(std::ostream& s, const std::wstring& ws);

#endif //__VOICEMAN_TRANSCODING_H__
