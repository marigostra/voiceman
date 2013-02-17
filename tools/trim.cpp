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

#include<stdio.h>
#include<iostream>
#include<string>
#include<stdlib.h>
#include <unistd.h>

#define INPUT_STREAM 0
#define OUTPUT_STREAM 1

template<typename T> T read();
template<typename T> void write(T c);

template<> char read<char>()
{
  char c;
  const ssize_t count = read(INPUT_STREAM, &c, sizeof(c));
  if (count == -1)
    {
      perror("read(stdin)");
      exit(EXIT_FAILURE);
    }
  if (count == 0)
    exit(EXIT_SUCCESS);
  return c;
}

template<> short read<short>()
{
  short c;
  const ssize_t count = read(INPUT_STREAM, &c, sizeof(c));
  if (count == -1)
    {
      perror("read(stdin)");
      exit(EXIT_FAILURE);
    }
  if (count < 2)
    exit(EXIT_SUCCESS);
  return c;
}

template<> void write<char>(char c)
{
  if (write(OUTPUT_STREAM, &c, sizeof(c)) == -1)
    {
      perror("write(stdout)");
      exit(EXIT_FAILURE);
    }
}

template<> void write<short>(short c)
{
  if (write(OUTPUT_STREAM, &c, sizeof(c)) == -1)
    {
      perror("write(stdout)");
      exit(EXIT_FAILURE);
    }
}

template<typename T> void run()
{
  T c;
  do {
    c=read<T>();
  } while(c==0);
  while(1)
    {
      write<T>(c);
      ssize_t k=-1;
      do {
	c=read<T>();
	k++;
      } while(c==0);
      for(;k>0;k--)
	write<T>(0);
    } // while(1);
}

int main(int argc, char *argv[])
{
  if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))
    {
      std::cout << "Utility to filter blank gaps in I/O streams." << std::endl;
      std::cout << "This utility is part of the VOICEMAN speech system." << std::endl;
      std::cout << "There are following command line options:" << std::endl;
      std::cout << "\t-h, --help - print this help;" << std::endl;
      std::cout << "\t-w, --words - set processing unit to two bytes." << std::endl;
      return 0;
    }
  if (argc == 2 && (std::string(argv[1]) == "--words" || std::string(argv[1]) == "-w"))
    run<short>(); else
    run<char>();
  return 0;
}
