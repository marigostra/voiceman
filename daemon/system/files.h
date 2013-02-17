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

#ifndef __VOICEMAN_FILES_H__
#define __VOICEMAN_FILES_H__

#include"SystemException.h"

/**\brief Reads block of exact size
 *
 * This function is a workaround for short read possibility in standart
 * read() function. It returns number of read bytes less than it was
 * specified only in case of read near the end of stream or file, so it
 * means there is no more data at all. It performs sequence of read()
 * function calls until complete possible block is read or returns -1 if
 * there was an error.
 *
 * \param [in] fd The handle to read data from
 * \param [out] buf The buffer to read data into
 * \param [in] bufSize Number of bytes to read
 *
 * \return Number of read bytes or -1 in case of error
 */
ssize_t readBlock(int fd, void* buf, size_t bufSize);

/**\brief Writes block of exact size
 *
 * This function is a workaround for short rwrite possibility in standart
 * write() function. It never returns result bytes count less than than it
 * was specified in call. It performs sequence of rwrite() function calls
 * until complete block is written or returns -1 if there was an error.
 *
 * \param [in] fd The handle to rwrite data to
 * \param [in] buf The buffer to read data from
 * \param [in] bufSize Number of bytes to write
 *
 * \return Number of written bytes (cannot be less than bufSize) or -1 in case of error
 */
ssize_t writeBlock(int fd, const void* buf, size_t bufSize);

/**\brief Reads buffer of unlimited length
 *
 * This function splits read operation into sequence of calls with fixed
 * size block. It avoids one long operation and caller may not care about
 * size of read operation. This function returns number of bytes less
 * than is was specified only in case of read operation near the end of
 * stream of file.
 *
 * \param [in] fd The handler to read data from
 * \param [out] buf The buffer to read data into
 * \param [in] bufSize The number of bytes to read
 *
 * \return The number of bytes successfully read or -1 in case of error 
 */
ssize_t readBuffer(int fd, void* buf, size_t bufSize);

/**\brief Writes buffer of unlimited length
 *
 * This function splits write operation into sequence of calls with fixed
 * size block. It avoids one long operation and caller may not care about
 * size of write operation. This function never returns number of bytes less
 * than it was specified in call.
 *
 * \param [in] fd The handler to write data to
 * \param [in] buf The buffer to write data from
 * \param [in] bufSize The number of bytes to write
 *
 * \return The number of bytes successfully written )can be only equal to bufSize( or -1 in case of error 
 */
ssize_t writeBuffer(int fd, const void* buf, size_t bufSize);

/**\brief Excludes comments from text file
 *
 * This function processes all lines and removes from them any text after
 * '#' character. It does not process string constants. It used for
 * simple configuration files, but not appropriate for main config.
 *
 * \param [in] str The string to exclude comments from
 *
 * \return The text without comments
 */
std::string cutComments(const std::string& str);

/**\brief Reads text file
 *
 * This function reads data from text file and returns text in one string
 * variable. It throws SystemException on any errors. No transformations
 * (like UTF-8 decoding) is made on text.
 *
 * \param [in] fileName The name of a file to read
 *
 * \return The text read from file
 *
 * \sa SystemException
 */
std::string readTextFile(const std::string& fileName);

/**\brief Split text from one string variable into a list of lines
 *
 * This function simplifies text files processing. It takes one long
 * string variable and generates string list with one item for every
 * line. Additional feature to trim lines and skip empty are also
 * available.
 *
 * \param [in] source The source text to split
 * \param [out] lines The list of strings to receive splitted lines
 * \param [in] performTrim Cut blank characters in lines beginning and end
 * \param [in]  skipEmpty Not save empty lines (with trim operation, if it was required)
 *
 * \sa readTextFile
 */
void splitTextFileLines(const std::string& source, StringList& lines, bool performTrim, bool skipEmpty);

#endif //__VOICEMAN_FILES_H__
