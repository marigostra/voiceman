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

#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/signal.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>
#include<locale.h>
#include"executorCommandHeader.h"

#define ERROR_PREFIX "voiceman-executor:"

#define NULL_DEVICE "/dev/null"
#define IO_BUF_SIZE 2048

#define QUEUE_ITEM_TEXT 1
#define QUEUE_ITEM_TONE 2

void toneInit();
void playTone(size_t freq, size_t lengthMs);
void toneClose();

typedef struct QueueItem_  
{
  int type;
  char* synthCommand;
  char* playerCommand;
  char* text;
  size_t freq;
  size_t duration;
  struct QueueItem_* next;
} QueueItem;

pid_t pid = 0;
pid_t playerPid = 0;
QueueItem* queueHead = NULL;
QueueItem* queueTail = NULL;
size_t queueSize = 0;
size_t maxQueueSize = 0;
volatile sig_atomic_t wasSigChld = 0;

void sigChldHandler(int n)
{
  wasSigChld = 1;
}

/*Reads block of specified length and produces subsequent calls of read() in case of short read operation*/
ssize_t readBlock(int fd, void* buf, size_t bufSize)
{
  char* b = (char*)buf;
  size_t c = 0;
  assert(buf);
  while(c < bufSize)
    {
      ssize_t res = read(fd, &b[c], bufSize - c);
      if (res == -1)
	return -1;
      if (res == 0)
	break;
      assert(res > 0);
      c += (size_t)res;
    } /*while();*/
  return (ssize_t)c;
}

/*RWrites block of specified length and produces subsequent calls of write() in case of short write operation*/
ssize_t writeBlock(int fd, void* buf, size_t bufSize)
{
  char* b = (char*)buf;
  size_t c = 0;
  assert(buf);
  while(c < bufSize)
    {
      ssize_t res = write(fd, &b[c], bufSize - c);
      if (res == -1)
	return -1;
      assert(res >= 0);
      c += (size_t)res;
    } /*while();*/
  assert(c == bufSize);
  return (ssize_t)c;
}

/*Reads buffer of any length producing subsequent calls to read fixed length blocks*/
ssize_t readBuffer(int fd, void* buf, size_t bufSize)
{
  char* b = (char*)buf;
  size_t c = 0;
  assert(buf);
  while(c < bufSize)
    {
      size_t requiredSize = bufSize > c + IO_BUF_SIZE?IO_BUF_SIZE:(size_t)(bufSize - c);
      ssize_t res = readBlock(fd, &b[c], requiredSize);
      if (res == -1)
	return -1;
      c += (size_t)res;
      if (res < (ssize_t)requiredSize)
	break;
    } /*while();*/
  return (ssize_t)c;
}

/*Writes buffer of any length producing subsequent calls to write fixed length blocks*/
ssize_t writeBuffer(int fd, void* buf, size_t bufSize)
{
  char* b = (char*)buf;
  size_t c = 0;
  assert(buf);
  while(c < bufSize)
    {
      size_t requiredSize = bufSize > c + IO_BUF_SIZE?IO_BUF_SIZE:(size_t)(bufSize - c);
      ssize_t res = writeBlock(fd, &b[c], requiredSize);
      if (res == -1)
	return -1;
      assert(res == (ssize_t)requiredSize);
      c += (size_t)res;
    } /*while();*/
  assert(c == bufSize);
  return (ssize_t)c;
}

void onNoMemError()
{
  fprintf(stderr, "%sno enough free memory\n", ERROR_PREFIX);
  fflush(stderr);
  exit(EXIT_FAILURE);
}

void onSystemCallError(const char* descr, int errorCode)
{
  assert(descr != NULL);
  fprintf(stderr, "%s%s:%s", ERROR_PREFIX, descr, strerror(errorCode));
  fflush(stderr);
  exit(EXIT_FAILURE);
}

void putTextItemToQueue(char* synthCommand, char* playerCommand, char* text)
{
  QueueItem* newItem = NULL;
  assert(synthCommand);
  assert(playerCommand);
  assert(text);
  if (maxQueueSize > 0 && queueSize >= maxQueueSize)
    {
      printf("queuelimit\n");
      fflush(stdout);
      free(synthCommand);
      free(playerCommand);
      free(text);
      return;
    }
  newItem = (QueueItem*)malloc(sizeof(QueueItem));
  if (!newItem)
    onNoMemError();
  newItem->type = QUEUE_ITEM_TEXT;
  newItem->synthCommand = synthCommand;
  newItem->playerCommand = playerCommand;
  newItem->text = text;
  newItem->freq = 0;
  newItem->duration = 0;
  newItem->next = NULL;
  queueSize++;
  if (!queueHead)/*there are no items in queue at all*/
    {
      queueHead = newItem;
      queueTail = newItem;
      return;
    }
  assert(queueTail);
  queueTail->next = newItem;
  queueTail = newItem;
}

void putToneItemToQueue(size_t freq, size_t duration)
{
  QueueItem* newItem = NULL;
  if (maxQueueSize > 0 && queueSize >= maxQueueSize)
    {
      printf("queuelimit\n");
      fflush(stdout);
      return;
    }
  newItem = (QueueItem*)malloc(sizeof(QueueItem));
  if (!newItem)
    onNoMemError();
  newItem->type = QUEUE_ITEM_TONE;
  newItem->synthCommand = NULL;
  newItem->playerCommand = NULL;
  newItem->text = NULL;
  newItem->freq = freq;
  newItem->duration = duration;
  newItem->next = NULL;
  queueSize++;
  if (!queueHead)/*there are no items in queue at all*/
    {
      queueHead = newItem;
      queueTail = newItem;
      return;
    }
  assert(queueTail);
  queueTail->next = newItem;
  queueTail = newItem;
}

void popQueueFront()
{
  if (!queueHead)
    {
      assert(!queueTail);
      return;
    }
  assert(queueSize > 0);
  if (queueHead->synthCommand)
    free(queueHead->synthCommand);
  if (queueHead->playerCommand)
    free(queueHead->playerCommand);
  if (queueHead->text)
    free(queueHead->text);
  queueSize--;
  if (queueHead->next)/*it is not the single item in queue*/
    {
      QueueItem* p = queueHead;
      queueHead = queueHead->next;
      free(p);
      return;
    }
  assert(queueHead == queueTail);
  free(queueHead);
  queueHead = NULL;
  queueTail = NULL;
}

void eraseQueue()
{
  while(queueHead)
    popQueueFront();
  queueSize = 0;
}

char isPlaying()
{
  return pid != (pid_t)0 || playerPid != (pid_t)0;
}

void execute(char* synthCommand, char* playerCommand, char* text)
{
  int pp[2];
  int interPp[2];
  size_t textLen = strlen(text);
  ssize_t res;
  assert(synthCommand);
  assert(playerCommand);
  assert(text);
  assert(pid == (pid_t)0);
  assert(playerPid == (pid_t)0);
  if (pipe(pp) == -1)
    {
      perror("pipe()");
      fflush(stderr);
      return;
    }
  if (pipe(interPp) == -1)
    {
      perror("pipe()");
      fflush(stderr);
      close(pp[0]);
      close(pp[1]);
      return;
    }
  pid = fork();
  if (pid == (pid_t)-1)
    {
      perror("fork()");
      fflush(stderr);
      close(pp[0]);
      close(pp[1]);
      close(interPp[0]);
      close(interPp[1]);
      pid = 0;
      return;
    }
  if (pid == (pid_t)0)/*The child process*/
    {
      int fd = open(NULL_DEVICE, O_WRONLY);
      if (fd == -1)
	exit(EXIT_FAILURE);
      setpgrp();
      close(pp[1]);/*Closing pipe input end*/
      close(interPp[0]);/*Closing pipe output end*/
      dup2(pp[0], STDIN_FILENO);
      dup2(interPp[1], STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      if (execlp("/bin/sh", "/bin/sh", "-c", synthCommand, NULL) == -1)
	exit(EXIT_FAILURE);
    } /* child process*/
  playerPid = fork();
  if (playerPid == (pid_t)-1)
    {
      perror("fork()");
      fflush(stderr);
      /*We cannot create new child process for player, closing all pipes and wait synth process*/
      close(pp[0]);
      close(pp[1]);
      close(interPp[0]);
      close(interPp[1]);
      waitpid(pid, NULL, 0);
      pid = 0;
      playerPid = 0;
      return;
    }
  if (playerPid == (pid_t)0)/*player child process*/
    {
      int fd = open(NULL_DEVICE, O_WRONLY);
      if (fd == -1)
	exit(EXIT_FAILURE);
      setpgrp();
      close(pp[0]);
      close(pp[1]);
      close(interPp[1]);/*Closing pipe input end*/
      dup2(interPp[0], STDIN_FILENO);
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      if (execlp("/bin/sh", "/bin/sh", "-c", playerCommand, NULL) == -1)
	exit(EXIT_FAILURE);
    } /*player child process*/
  close(interPp[0]);
  close(interPp[1]);
  close(pp[0]);/*Closing output side of pipe*/
  res = writeBuffer(pp[1], text, textLen);
  if (res < 0)
    {
      perror("write()");
      fflush(stderr);
      close(pp[1]);
      return;
    }
  assert(res == (ssize_t)textLen);
  if (writeBlock(pp[1], "\n", 1) == -1)
    {
      perror("write()");
      fflush(stderr);
    }
  close(pp[1]);
}

void playNext()
{
  while(queueHead && queueHead->type == QUEUE_ITEM_TONE)
    {
      playTone(queueHead->freq, queueHead->duration);
      popQueueFront();
    }
  if (queueHead == NULL)/*No more queue items to play*/
    {
      /*we must notify, there are no more items to play*/
      printf("silence\n");
      fflush(stdout);
      return;
    }
  execute(queueHead->synthCommand, queueHead->playerCommand, queueHead->text);
  popQueueFront();
}

/*This function frees provided string buffers if necessary*/
void play(char* synthCommand, char* playerCommand, char* text)
{
  assert(synthCommand);
  assert(playerCommand);
  assert(text);
  if (isPlaying())/*playback in progress now*/
    {
      putTextItemToQueue(synthCommand, playerCommand, text);
      return;
    }
  execute(synthCommand, playerCommand, text);
  free(synthCommand);
  free(playerCommand);
  free(text);
}

void stop()
{
  eraseQueue();
  if (!isPlaying())/*There is no playback now*/
    return;
  if (playerPid != (pid_t)0)
    {
      kill(playerPid, SIGINT);
      killpg(playerPid, SIGINT);
      kill(playerPid, SIGKILL);
      killpg(playerPid, SIGKILL);
      waitpid(playerPid, NULL, 0);
      while(waitpid(-1 * playerPid, NULL, 0) >= 0);
      playerPid = 0;
    }
  if (pid != (pid_t)0)
    {
      kill(pid, SIGKILL);
      killpg(pid, SIGKILL);
      waitpid(pid, NULL, 0);
      while(waitpid(-1 * pid, NULL, 0) >= 0);
      pid = 0;
    }
  printf("stopped\n");
  fflush(stdout);
}

/*This function returnes zero if there is no more data and handle can be closed*/
char processInputCommand(int fd)
{
  CommandHeader header;
  ssize_t res;
  res = readBlock(fd, &header, sizeof(header));
  if (res < 0)
    onSystemCallError("read()", errno);
  if (res < sizeof(CommandHeader))
    return 0;
  if (header.code == COMMAND_STOP)
    {
      stop();
      return 1;
    } /*COMMAND_STOP*/
  if (header.code == COMMAND_SAY)
    {
      char* synthCommand;
      char* playerCommand;
      char* text;
      synthCommand = (char*)malloc(header.param1);
      if (!synthCommand)
	onNoMemError();
      playerCommand = (char*)malloc(header.param2);
      if (!playerCommand)
	onNoMemError();
      text = (char*)malloc(header.param3);
      if (text == NULL)
	onNoMemError();
      /*reading synth command line*/
      res = readBuffer(fd, synthCommand, header.param1);
      if (res < 0)
	onSystemCallError("read()", errno);
      if (res < header.param1)
	{
	  free(synthCommand);
	  free(playerCommand);
	  free(text);
	  return 0;
	}
      /*reading player command line*/
      res = readBuffer(fd, playerCommand, header.param2);
      if (res < 0)
	onSystemCallError("read()", errno);
      if (res < header.param2)
	{
	  free(synthCommand);
	  free(playerCommand);
	  free(text);
	  return 0;
	}
      /*reading text to say*/
      res = readBuffer(fd, text, header.param3);
      if (res < 0)
	onSystemCallError("read()", errno);
      if (res < header.param3)
	{
	  free(synthCommand);
	  free(playerCommand);
	  free(text);
	  return 0;
	}
      play(synthCommand, playerCommand, text);
      return 1;
    } /*COMMAND_EXECUTE*/
  if (header.code == COMMAND_TONE)
    {
      if (!isPlaying())
	playTone(header.param1, header.param2); else 
	putToneItemToQueue(header.param1, header.param2);
      return 1;
    } /*COMMAND_TONE*/
  if (header.code == COMMAND_SET_QUEUE_LIMIT)
    {
      maxQueueSize = header.param1;
      return 1;
    } /*COMMAND_SET_QUEUE_LIMIT*/
  fprintf(stderr, "%s unknown command %X(%zu,%zu,%zu)\n", ERROR_PREFIX,
	  header.code, header.param1,
	  header.param2, header.param3);
  fflush(stderr);
  return 1;
}

void handleSignals()
{
  if (wasSigChld)
    {
      wasSigChld = 0;
      if (!isPlaying())
	return;
      /*synthesizer group processing*/
      if (pid != (pid_t)0)
	{
	  pid_t pp = 0;
	  pid_t p = waitpid(pid, NULL, WNOHANG);
	  while(1)
	    {
	      pp = waitpid(-1 * pid, NULL, WNOHANG);
	      if (pp <= (pid_t)0)/*All zombies are collected, but there can be live processes*/
		break;
	      /*yes, we have picked up real zombie and must try new waitpid(), there can be more*/
	    } /*while()*/
	  if (p != (pid_t)0 && pp != (pid_t)0)
	    pid = 0;
	} /*synthesizer group processing*/
      /*player group processing*/
      if (playerPid != (pid_t)0)
	{
	  pid_t pp = 0;
	  pid_t p = waitpid(playerPid, NULL, WNOHANG);
	  while(1)
	    {
	      pp = waitpid(-1 * playerPid, NULL, WNOHANG);
	      if (pp <= (pid_t)0)/*All zombies are collected, but there can be live processes*/
		break;
	      /*yes, we have picked up real zombie and must try new waitpid(), there can be more*/
	    } /*while()*/
	  if (p != (pid_t)0 && pp != (pid_t)0)
	    playerPid = 0;
	} /*player group processing*/
      if (isPlaying())
	return;
      playNext();
    }
}

int mainLoop(int fd, sigset_t* sigMask)
{
  /*Endless loop for all commands*/
  while(1)
    {
      /*Preparing for pselect() call*/
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(fd, &fds);
      /*Calling pselect()*/
      if (pselect(fd + 1, &fds, NULL, NULL, NULL, sigMask) == -1)
	{
	  /*pselect() has returned -1, what the problem*/
	  int errorCode = errno;
	  if (errorCode == EINTR)
	    {
	      /*Now we must handle every signal we caught*/
	      handleSignals();
	      /*Trying to call pselect() one more time*/
	      continue;
	    }
	  /*it is an unexpected system call error, we must stop processing*/
	  onSystemCallError("pselect()", errorCode);
	} /*if (pselect() == -1)*/
      assert(FD_ISSET(fd, &fds));
      if (!processInputCommand(fd))/*this function returnes zero if fd was closed*/
	return 0;
    } /*while(1)*/
}

int main(int argc, char* argv[])
{
  int exitCode = 0;
  struct sigaction sa;
  sigset_t origMask, blockedMask;
  setlocale(LC_ALL, "");
  /*Installing SIGCHLD signal handler*/
  sigaction(SIGCHLD, NULL, &sa);
  sa.sa_handler = sigChldHandler;
  sa.sa_flags |= SA_RESTART;
  sigaction(SIGCHLD, &sa, NULL);
  /*Preparing blocking signal mask with disabled SIGCHLD*/
  sigemptyset(&blockedMask);
  sigaddset(&blockedMask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &blockedMask, &origMask);
  toneInit();
  exitCode = mainLoop(STDIN_FILENO, &origMask);
  toneClose();
  return exitCode;
}
