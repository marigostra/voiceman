
#include<assert.h>
#include<iostream>
#include<fstream>
#include<string>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<locale.h>
#include"executorCommandHeader.h"

//Control macro;
//#define PRINT_SENTENCES

#define ERROR_PREFIX "stress:"
#define IO_BUF_SIZE 2048
#define EXECUTOR_COMMAND_LINE "./voiceman-executor"
#define SEQ_MODE_QUEUE_SIZE 5
#define NORMAL_MAX_DELAY 2000000
#define NIGHTMARE_MAX_DELAY 1000

std::string syncCmdLine, playerCmdLine;
pid_t pid = 0;
int pp = 0;
size_t maxDelay = 0;

void onSystemCallError(const char* descr, int errorCode)
{
  assert(descr != NULL);
  std::cerr << ERROR_PREFIX << descr << ":" << strerror(errorCode) << std::endl;
  exit(1);
}

ssize_t writeBlock(int fd, const void* buf, size_t bufSize)
{
  assert(buf);
  char* b = (char*)buf;
  size_t c = 0;
  while(c < bufSize)
    {
      ssize_t res = write(fd, &b[c], bufSize - c);
      if (res == -1)
	return -1;
      assert(res >= 0);
      c += (size_t)res;
    } //while();
  assert(c == bufSize);
  return (ssize_t)c;
}

ssize_t writeBuffer(int fd, const void* buf, size_t bufSize)
{
  assert(buf);
  char* b = (char*)buf;
  size_t c = 0;
  while(c < bufSize)
    {
      const size_t requiredSize = bufSize > c + IO_BUF_SIZE?IO_BUF_SIZE:(size_t)(bufSize - c);
      const ssize_t res = writeBlock(fd, &b[c], requiredSize);
      if (res == -1)
	return -1;
      assert(res == (ssize_t)requiredSize);
      c += (size_t)res;
    } //while();
  assert(c == bufSize);
  return (ssize_t)c;
}

void sendStopCommand()
{
  CommandHeader header;
  header.code = COMMAND_STOP;
  ssize_t res = writeBlock(pp, &header, sizeof(CommandHeader));
  if (res == -1)
    onSystemCallError("write()", errno);
  assert(res == sizeof(CommandHeader));
}

void sendSayCommand(const std::string& command, const std::string& player, const std::string& text)
{
  CommandHeader header;
  header.code = COMMAND_SAY;
  header.param1 = command.length() + 1;
  header.param2 = player.length() + 1;
  header.param3 = text.length() + 1;
  ssize_t res = writeBlock(pp, &header, sizeof(CommandHeader));
  if (res == -1)
    onSystemCallError("write()", errno);
  assert(res == sizeof(CommandHeader));
  res = writeBuffer(pp, command.c_str(), command.length() + 1);
  if (res == -1)
    onSystemCallError("write()", errno);
  assert(res == (ssize_t)(command.length() + 1));
  res = writeBuffer(pp, player.c_str(), player.length() + 1);
  if (res == -1)
    onSystemCallError("write()", errno);
  assert(res == (ssize_t)(player.length() + 1));
  res = writeBuffer(pp, text.c_str(), text.length() + 1);
  if (res == -1)
    onSystemCallError("write()", errno);
  assert(res == (ssize_t)(text.length() + 1));
}

void execute(const std::string& cmdLine)
{
  int p[2];
  assert(pid == 0);
  if (pipe(p) == -1)
    onSystemCallError("pipe()", errno);
  pid = fork();
  if (pid == (pid_t)-1)
    onSystemCallError("fork()", errno);
  if (pid == 0)/*The child process*/
    {
      close(p[1]);/*Closing pipe input side*/
      dup2(p[0], STDIN_FILENO);
      if (execlp("/bin/sh", "/bin/sh", "-c", cmdLine.c_str(), NULL) == -1)
	{
	  perror("execlp(/bin/sh)");
	  exit(3);
	}
    } /* child process*/
  close(p[0]);/*Closing output side of pipe*/
  pp = p[1];
}

void processLine(const std::string& s)
{
  sendSayCommand(syncCmdLine, playerCmdLine, s);
  assert(maxDelay != 0);
  const int delay = rand() % maxDelay;
  usleep(delay);
  sendStopCommand();
}

bool readFile(const std::string& fileName, bool seqMode)
{
  size_t seqCounter = 0;
  std::ifstream fs(fileName.c_str());
  if (!fs)
    return 0;
  std::string s;
  char c;
  while (fs.get(c))
    {
      if (c == '\r')
	continue;
      if (c == '\n')
	{
	  s += ' ';
	  continue;
	}
      if (c >=0 && c < 32)
	continue;
      if (c == '.' || c == '?' || c == '!')
	{
	  s += c;
	  if (seqMode)
	    {
	      if (seqCounter >= SEQ_MODE_QUEUE_SIZE)
		while(1);//endless loop;
	      sendSayCommand(syncCmdLine, playerCmdLine, s);
#ifdef PRINT_SENTENCES
	      std::cout << s << std::endl << std::endl;
#endif //PRINT_SENTENCES;
	      seqCounter++;
	    } else
	    processLine(s);
	  s.erase();
	  continue;
	}
      s += c;
    }
  processLine(s);
  std::cerr << "Finished!!!" << std::endl;
  return 1;
}

int main(int argc, char* argv[])
{
  setlocale(LC_ALL, "");
  if (argc == 2 && std::string(argv[1]) == "--help")
    {
      std::cout << "Usage: stress <mode> <synth_command_line> <player_command_line> <input_file_name>" << std::endl;
      std::cout << "Valid modes are: seq, normal, nightmare;" << std::endl;
      return 0;
    }
  if (argc < 5)
    {
      std::cerr << ERROR_PREFIX << "too few arguments" << std::endl;
      return 1;
    }
  const std::string mode = argv[1];
  if (mode != "seq" && mode != "normal" && mode != "nightmare")
    {
      std::cout << "Unknown mode \'" << mode << "\'" << std::endl;
      return 1;
    }
  syncCmdLine = argv[2];
  playerCmdLine = argv[3];
  execute(EXECUTOR_COMMAND_LINE);
  if (mode == "seq")
    {
      if (!readFile(argv[4], 1))//1 means seq mode;
	return 1;
      return 0;
    }
  if (mode == "normal")
    {
      maxDelay = NORMAL_MAX_DELAY;
      if (!readFile(argv[4], 0))//0 means non-seq mode;
	return 1;
      return 0;
    }
  if (mode == "nightmare")
    {
      maxDelay = NIGHTMARE_MAX_DELAY;
      if (!readFile(argv[4], 0))//0 means non-seq mode;
	return 1;
      return 0;
    }
  return 5;
}
