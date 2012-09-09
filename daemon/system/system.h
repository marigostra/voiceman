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

#ifndef __VOICEMAN_SYSTEM_H__
#define __VOICEMAN_SYSTEM_H__

#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#include<string>
#include<vector>
#include<list>
#include<set>
#include<map>
//KILLME:#include<unordered_set>
//KILLME:#include<unordered_map>
#include<sstream>
#include<fstream>
#include<iostream>
#include<algorithm>
#include<memory>

#include<sys/types.h>
#include<unistd.h>
#include"syslog.h"
#include<dirent.h>
#include<errno.h>
#include<assert.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<signal.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<resolv.h>
#include<sys/un.h>
#include<pthread.h>
#include<fcntl.h>
#include<iconv.h>
#include<locale.h>
#include<time.h>

typedef std::vector<std::string> StringVector;
typedef std::list<std::string> StringList;

#define ERRNO_MSG (strerror(errno))

#endif //__VOICEMAN_SYSTEM_H__
