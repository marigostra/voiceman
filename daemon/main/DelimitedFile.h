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

#ifndef __VOICEMAN_DELIMITED_FILE_H__
#define __VOICEMAN_DELIMITED_FILE_H__

/**\brief The class to parse file with colon-delimited lines
 *
 * VoiceMan daemon uses set of data tables stored in colon-delimited
 * files. Each line in these files contains some items, delimited by ":"
 * character. This class parses colon-delimited files and extracts stored
 * data.
 */
class DelimitedFile
{
public:
  /**\brief Reads and stores data from file
   *
   * This method can be called multiple times to read sequentially more
   * than one file. Results of previous operation are silently cleaned. 
   *
   * \param [in] fileName The name of a file to read data from
   */
  void read(const std::string& fileName);

  /**\brief The number of non-empty and non-commented lines read from source file
   *
   * Use this method to get number of accessible lines.
   *
   * \return Number of parsed lines
   */
  size_t getLineCount() const;

  /**\brief Returns text file line without any translations
   *
   * Use this method to get line as it is stored 
   * in lines vector.
   *
   * \param [in] index The index of a line to return
   *
   * \return Requested line without any processing (but without comments)
   */
  std::string getRawLine(size_t index) const;

  /**\brief Returns the number of delimited items in specified line
   *
   * Use this method to get number of items 
   * in specified line.
   *
   * \param [in] index The number of a line to count items in
   *
   * \return Number of items in specified line
   */
  size_t getItemCountInLine(size_t index) const;

  /**\brief Returns the required delimited item in required line
   *
   * Use this method to get colon-delimited item in line.
   *
   * \param [in] lineIndex The desired index of line
   * \param [in] itemIndex The number of desired item in specified line
   *
   * \return The requested colon-delimited item
   */
  std::string getItem(size_t lineIndex, size_t itemIndex) const;

private:
  StringVector m_lines;
}; //class DelimitedFile;

#endif //__VOICEMAN_DELIMITED_FILE_H__
