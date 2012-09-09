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

#ifndef __VOICEMAN_ABSTRACT_EXECUTOR_OUTPUT_H__
#define __VOICEMAN_ABSTRACT_EXECUTOR_OUTPUT_H__

/**\brief The interface for handling executor output in main loop class
 *
 * The VoiceMan executor as external process has stdout and stderr
 * streams to send data back to VoiceMan and report errors. These two
 * streams must be handled in general main loop via adding them into
 * pselect() call. This interface is used to get both file descriptors of
 * executor and send notification there is data to read from them.
 *
 * \sa ExecutorInterface MainLoop
 */
class AbstractExecutorOutput
{
public:
  /**\brief Returns the file descriptor of executor stdout stream
   *
   * This method returns file descriptor of executor stdout stream. This
   * descriptor must be added to the main pselect() call to know when we
   * have data to read.
   *
   * \return The file descriptor of executor stdout stream
   */
  virtual int getExecutorStdoutDescriptor() const = 0;

  /**\brief Returns the file descriptor of executor stderr stream
   *
   * This method returns file descriptor of executor stderr stream. This
   * descriptor must be added to the main pselect() call to know when we
   * have data to read.
   *
   * \return The file descriptor of executor stderr stream
   */
  virtual int getExecutorStderrDescriptor() const = 0;

  /**\brief Notifies there is data to read from executor stdout stream
   *
   * This method notifies implementation to read accessible data from
   * executor output stream. This notification is sent by MainLoop class
   * when it receives corresponding information from main pselect() system
   * call.
   */
  virtual void readExecutorStdoutData() = 0;

  /**\brief Notifies there is data to read from executor stderr stream
   *
   * This method notifies implementation to read accessible data from
   * executor error stream. This notification is sent by MainLoop class
   * when it receives corresponding information from main pselect() system
   * call.
   */
  virtual void readExecutorStderrData() = 0;
}; //class AbstractExecutorOutput;

#endif //__VOICEMAN_ABSTRACT_EXECUTOR_OUTPUT_H__
