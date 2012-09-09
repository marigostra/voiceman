/*
  This is the simple demonstration of voiceman connection.
  This program must say 'Hello!'.
To compile it run 'gcc -oexample1 example1.c -lvmclient'.
*/

#include<stdio.h>
#include<errno.h>
#include"vmclient/vmclient.h"

int main()
{
    vm_connection_t con=vm_connect();
  if (con == VOICEMAN_BAD_CONNECTION)
    {
      perror("vm_connect_t()");
      return 1;
    }
  if (vm_text(con, "Hello!")==VOICEMAN_ERROR)
    {
      perror("vm_text()");
      return 1;
    }
  vm_close(con);
  return 0;
}
