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

#ifndef __VOICEMAN_EXECUTOR_INTERFACE_H__
#define __VOICEMAN_EXECUTOR_INTERFACE_H__

#include"OutputSet.h"
#include"AbstractExecutorOutput.h"

/**\brief The interface for executor event handlers
 *
 * This class declares abstract interface to transmit notifications about
 * events sent by executor process. The most important event is the
 * notification there are no items in queue to say and it is silence
 * now. In future notifications about speaking part of text also must be
 * implemented.
 */
class AbstractExecutorCallback
{
public:
  enum {Silence = 0, Stopped = 1, QueueLimit = 2};

  /**\brief The function to notify about new executor event
   *
   * This method notifies implementation new event was received from
   * executor. ExecutorInterface class parses received information and
   * provides event code. In future information provided by executor can be
   * extended.
   *
   * \param [in] event The identifier about occurred event 
   */
  virtual void onExecutorEvent(int event) = 0;
}; //class abstractExecutorCallback;

/**\brief The interface to executor process
 *
 * This class performs all communication tasks with external executor
 * process. Executor process receives such commands as"say", "stop" and
 * "tone". It implements queue behavior and stores "say" command in
 * internal queue if if was received during playback. Executor sends some
 * feedback info, like "silence" message, if there are no more items in
 * queue to speak, or notifications about queue size limit exceeds.  It
 * stored in separated executable file and can be changed via
 * configuration file parameter.
 *
 * \sa AbstractExecutorCallback AbstractExecutorOutput
 */
class ExecutorInterface: public AbstractExecutorOutput
{
public:
  /**\brief The constructor
   *
   * \param [in] callback The reference to object for executor events handling
   * \param [in] outputSet The set of outputs to generate command lines and text preprocessing
   * \param [in] maxQueueSize The maximum number of items in queue (0 - not limited)
   * \param executorName The file name of executor to run
   * \param [in] playerType Used player type (alsa, pulseaudio, pcspeaker)
   */
  ExecutorInterface(AbstractExecutorCallback& callback, const OutputSet& outputSet, size_t maxQueueSize, const std::string& executorName, PlayerType playerType);

  /**\brief The destructor*/
  virtual ~ExecutorInterface();

  /**\brief Sends command to say part of text
   *
   * This method sends the "SAY" command to executor process. If executor
   * is in idle mode, part of text being sent will be spoken immediately or
   * will be stored in queue otherwise.
   *
   * \param [in] textItem The text item to enqueue
   */
  void sayOrEnqueue(const TextItem& textItem);

  /**\brief Sends command to stop speech and clear queue
   *
   * This method sends "STOP" command to executor process. On receiving it
   * executor must stop any playback and clear queue.
*/
  void stop();

  /**\brief Sends tone command
   *
   * This method sends command to queue to produce tone signal by specified
   * duration and frequency. This signal is not speech signal it is
   * generated with sin() function and then played via audio output
   * library.
   *
   * \param [in] freq The desired frequency of tone signal 
   * \param [in] duration The desired duration of tone signal in milliseconds
   */
  void tone(size_t freq, size_t duration);

  /**\brief Stops executor process and picks up zombie
   *
   * This method closes executor input pipe and waits its termination to
   * pick up zombie. It is called on SIGPIPE signal receiving or at write()
   * function failure. Executor process stopping does not mean daemon
   * disfunctioning. Executor is always automatically spawned on "SAY" or
   * "TONE" commands.
   */
  void stopExecutor();

public://AbstractExecutorOutput;
  /**\brief Returns the file descriptor of executor stdout stream
   *
   * This method returns file descriptor of executor stdout stream. This
   * descriptor must be added to the main pselect() call to know when we
   * have data to read.
   *
   * \return The file descriptor of executor stdout stream
   *
   * \sa AbstractExecutorOutput
   */
  int getExecutorStdoutDescriptor() const;

  /**\brief Returns the file descriptor of executor stderr stream
   *
   * This method returns file descriptor of executor stderr stream. This
   * descriptor must be added to the main pselect() call to know when we
   * have data to read.
   *
   * \return The file descriptor of executor stderr stream
   *
   * \sa AbstractExecutorOutput
   */
  int getExecutorStderrDescriptor() const;

  /**\brief Notifies there is data to read from executor stdout stream
   *
   * This method notifies implementation to read accessible data from
   * executor output stream. This notification is sent by MainLoop class
   * when it receives corresponding information from main pselect() system
   * call.
   *
   * \sa AbstractExecutorOutput
   */
  void readExecutorStdoutData();

  /**\brief Notifies there is data to read from executor stderr stream
   *
   * This method notifies implementation to read accessible data from
   * executor error stream. This notification is sent by MainLoop class
   * when it receives corresponding information from main pselect() system
   * call.
   *
   * \sa abstractExecutorOutput
   */
  void readExecutorStderrData();

private:
  void processExecutorOutputLine(const std::string& line) const;
  void processExecutorErrorLine(const std::string& line) const;
  void runExecutor();
  //The descr parameter is used only for proper logging output;
  bool sendBlockToExecutor(const void* buf, size_t size, const std::string& descr);

private:
  AbstractExecutorCallback& m_callback;
  const OutputSet& m_outputSet;
  const size_t m_maxQueueSize;
  const std::string m_executorName;
  const PlayerType m_playerType;
  pid_t m_pid;
  int m_pipe;
  int m_outputPipe[2], m_errorPipe[2];
  std::string m_executorOutputChain, m_executorErrorChain;
}; //class ExecutorInterface;

#endif //__VOICEMAN_EXECUTOR_INTERFACE_H__;
