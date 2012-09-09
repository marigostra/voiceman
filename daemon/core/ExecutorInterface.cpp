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
#include"ExecutorInterface.h"
#include"executorCommandHeader.h"

#define SHELL "/bin/sh"

ExecutorInterface::ExecutorInterface(AbstractExecutorCallback& callback, const OutputSet& outputSet, size_t maxQueueSize, const std::string& executorName, PlayerType playerType)
  : m_callback(callback), m_outputSet(outputSet), m_maxQueueSize(maxQueueSize), m_executorName(executorName), m_playerType(playerType), m_pid(0)
{
  VM_SYS(pipe(m_outputPipe) == 0, "pipe()");
  VM_SYS(pipe(m_errorPipe) == 0, "pipe()");
}

ExecutorInterface::~ExecutorInterface()
{
  close(m_outputPipe[0]);
  close(m_outputPipe[1]);
  close(m_errorPipe[0]);
  close(m_errorPipe[1]);
}

void ExecutorInterface::sayOrEnqueue(const TextItem& textItem)
{
  const std::string& outputName = textItem.getOutputName();
  if (trim(outputName).empty())
    {
      logMsg(LOG_WARNING, "Received text item to play, but output name is empty, ignoring...");
      return;
    }
  if (!m_outputSet.hasOutput(outputName))
    {
      logMsg(LOG_ERR, "Received text item, but output name is unknown (\'%s\'), ignoring...", outputName.c_str());
      return;
    }
  if (m_pid == 0)//executor is not started;
    {
      logMsg(LOG_DEBUG, "Having text to say, but executor is not running, Launching it...");
      runExecutor();
      if (m_pid == 0)
	{
	  logMsg(LOG_ERR, "Executor launch failed, probably there are problems! Text block will be ignored");
	  return;
	}
    }
  if (m_pipe == 0)
    {
      logMsg(LOG_ERR, "We are sure the executor is running, but pipe to it is not valid, stopping executor and waiting next text block.");
      stopExecutor();
      return;
    }
  const std::string synthCommand = m_outputSet.prepareSynthCommand(outputName, textItem);
  const std::string playerCommand = m_outputSet.preparePlayerCommand(outputName, m_playerType, textItem);
  const std::string text = m_outputSet.prepareText(outputName, textItem);
  if (trim(synthCommand).empty())
    {
      logMsg(LOG_WARNING, "Prepared synth command to be sent to executor is empty");
      return;
    }
  if (trim(playerCommand).empty())
    {
      logMsg(LOG_WARNING, "Prepared player command to be sent to executor is empty");
      return;
    }
  logMsg(LOG_DEBUG, "Text and command line prepared to be sent to executor:");
  logMsg(LOG_DEBUG, "Synth command line: %s;", synthCommand.c_str());
  logMsg(LOG_DEBUG, "Player command line: %s;", playerCommand.c_str());
  logMsg(LOG_DEBUG, "Text: %s.", text.c_str());
  CommandHeader header;
  header.code = COMMAND_SAY;
  header.param1 = synthCommand.length() + 1;//+1 to reflect ending zero;
  header.param2 = playerCommand.length() + 1;//+1 to reflect ending zero;
  header.param3 = text.length() + 1;//+1 to reflect ending zero;
  if (!sendBlockToExecutor(&header, sizeof(CommandHeader), "\'SAY\' command header"))
    return;
  if (!sendBlockToExecutor(synthCommand.c_str(), synthCommand.length() + 1, "synth command"))
    return;
  if (!sendBlockToExecutor(playerCommand.c_str(), playerCommand.length() + 1, "player command"))
    return;
  if (!sendBlockToExecutor(text.c_str(), text.length() + 1, "text"))
    return;
  logMsg(LOG_DEBUG, "Command was successfully sent to executor!");
}

void ExecutorInterface::stop()
{
  logMsg(LOG_DEBUG, "Sending \'STOP\' command to executor");
  if (m_pid == 0)//executor is not running;
    {
      logMsg(LOG_DEBUG, "We must send \'STOP\' command, but executor is not running");
      return;
    }
  if (m_pipe == 0)
    {
      logMsg(LOG_ERR, "Executor pid is non-zero but input pipe is not valid, error sending \'STOP\' command");
      return;
    }
  CommandHeader header;
  header.code = COMMAND_STOP;
  header.param1 = 0;
  header.param2 = 0;
  header.param3 = 0;
  sendBlockToExecutor(&header, sizeof(CommandHeader), "stop command header");
  logMsg(LOG_DEBUG, "\'STOP\' command was sent successfully");
}

void ExecutorInterface::tone(size_t freq, size_t duration)
{
  if (m_pid == 0)//executor is not started;
    {
      logMsg(LOG_DEBUG, "Having tone to play, but executor is not running, Launching it...");
      runExecutor();
      if (m_pid == 0)
	{
	  logMsg(LOG_ERR, "Executor launch failed, probably there are problems! tone will be ignored");
	  return;
	}
    }
  CommandHeader header;
  header.code = COMMAND_TONE;
  header.param1 = freq;
  header.param2 = duration;
  header.param3 = 0;
  sendBlockToExecutor(&header, sizeof(CommandHeader), "\'TONE\' command");
}

void ExecutorInterface::runExecutor()
{
  assert(m_pid == 0);
  m_pipe = 0;
  int pp[2];
  if (pipe(pp) == -1)
    {
      logMsg(LOG_ERR, "Could not create pipe for communications with executor (pipe() returned %s)", ERRNO_MSG);
      return;
    }
  logMsg(LOG_DEBUG, "starting executor as \'%s\'", m_executorName.c_str());
  m_pid = fork();
  if (m_pid == (pid_t)-1)
    {
      logMsg(LOG_ERR, "Error running child process for executor (fork() returned %s)", ERRNO_MSG);
      close(pp[0]);
      close(pp[1]);
      m_pid = 0;
      return;
    }
  if (m_pid == 0)
    {
      close(pp[1]);
      close(m_outputPipe[0]);
      close(m_errorPipe[0]);
      dup2(pp[0], STDIN_FILENO);
      dup2(m_outputPipe[1], STDOUT_FILENO);
      dup2(m_errorPipe[1], STDERR_FILENO);
      if (execlp(SHELL, SHELL, "-c", m_executorName.c_str(), (char*)0) == -1)
	exit(EXIT_FAILURE);
    } // child process;
  close(pp[0]);
  m_pipe = pp[1];
  CommandHeader header;
  header.code = COMMAND_SET_QUEUE_LIMIT;
  header.param1 = m_maxQueueSize;
  header.param2 = 0;
  header.param3 = 0;
  sendBlockToExecutor(&header, sizeof(CommandHeader), "\'SET_QUEUE_LIMIT\' command");
}

void ExecutorInterface::stopExecutor()
{
  if (m_pid == 0)
    {
      logMsg(LOG_DEBUG, "Could not stop executor, it is not running (pid == 0)");
      return;
    }
  close(m_pipe);
  m_pipe = 0;
  int status = 0;
  //Maybe it is good idea to add delay and send SIGKILL explicitly if executor does not died in one second after input pipe closing;
  const pid_t pid = waitpid(m_pid, &status, 0);
  m_pid = 0;
  if (pid == -1)
    {
      logMsg(LOG_ERR, "waitpid() for executor process has returned -1 (error is \'%s\')", ERRNO_MSG);
      return;
    }
  logMsg(LOG_DEBUG, "executor input pipe was closed and zombie was picked up (waitpid() status = %d)", status);
}

bool ExecutorInterface::sendBlockToExecutor(const void* buf, size_t size, const std::string& descr)
{
  assert(m_pid != 0);
  assert(m_pipe != 0);
  const ssize_t res = writeBuffer(m_pipe, buf, size);
  if (res == -1)
    {
      logMsg(LOG_ERR, "Error sending %s to executor, stopping it. It will be launched again at next text block (error was \'%s\')", descr.c_str(), ERRNO_MSG);
      stopExecutor();
      return 0;
    }
  assert(res == (ssize_t)size);
  return 1;
}

void ExecutorInterface::processExecutorOutputLine(const std::string& line) const
{
  if (trim(toLower(line)) == "silence")
    {
      logMsg(LOG_DEBUG, "Received \'SILENCE\' notification from executor");
      m_callback.onExecutorEvent(AbstractExecutorCallback::Silence);
      return;
    }
  if (trim(toLower(line)) == "stopped")
    {
      logMsg(LOG_DEBUG, "Received \'STOPPED\' notification from executor");
      m_callback.onExecutorEvent(AbstractExecutorCallback::Stopped);
      return;
    }
  if (trim(toLower(line)) == "QueueLimit")
    {
      logMsg(LOG_DEBUG, "Received \'QUEUELIMIT\' notification from executor");
      m_callback.onExecutorEvent(AbstractExecutorCallback::QueueLimit);
      return;
    }
  logMsg(LOG_WARNING, "Received unexpected line from executor \'%s\'", line.c_str());
}

void ExecutorInterface::processExecutorErrorLine(const std::string& line) const
{
  logMsg(LOG_ERR, "executor error:%s", line.c_str());
}

int ExecutorInterface::getExecutorStdoutDescriptor() const
{
  return m_outputPipe[0];
}

int ExecutorInterface::getExecutorStderrDescriptor() const
{
  return m_errorPipe[0];
}

void ExecutorInterface::readExecutorStdoutData()
{
  char buf[2048];
  const ssize_t res = ::read(m_outputPipe[0], buf, sizeof(buf));
  if (res == -1)
    {
      logMsg(LOG_ERR, "Cannot read data from executor stdout stream (%s)", ERRNO_MSG);
      return;
    }
  for(ssize_t i = 0;i < res;i++)
    m_executorOutputChain += buf[i];
  TextQueue<std::string> queue(m_executorOutputChain);
  std::string s;
  while (queue.next(s))
    {
      logMsg(LOG_DEBUG, "Received line from executor: \'%s\'", s.c_str());
      processExecutorOutputLine(s);
    }
  m_executorOutputChain = queue.chain();
}

void ExecutorInterface::readExecutorStderrData()
{
  char buf[2048];
  const ssize_t res = ::read(m_errorPipe[0], buf, sizeof(buf));
  if (res == -1)
    {
      logMsg(LOG_ERR, "Cannot read data from executor stderr stream (%s)", ERRNO_MSG);
      return;
    }
  for(ssize_t i = 0;i < res;i++)
    m_executorErrorChain += buf[i];
  TextQueue<std::string> queue(m_executorErrorChain);
  std::string s;
  while (queue.next(s))
    {
      logMsg(LOG_DEBUG, "Received error line from executor: \'%s\'", s.c_str());
      processExecutorErrorLine(s);
    }
  m_executorErrorChain = queue.chain();
}
