/*
  This program says some text with different parameters.
  To compile it run 'gcc -oexample3 example3.c -lvmclient'.
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
  vm_text(con, "Hello! This is a normal text. But it is");
  vm_rate(con , 40);
  vm_text(con, "faster");
  vm_rate(con, 30);
  vm_text(con, "and faster");
  vm_rate(con, 20);
  vm_text(con, "and more faster");
  vm_pitch(con, 70);
  vm_text(con, "and more high");
  vm_pitch(con, 100);
  vm_text(con, "and more high");
  vm_rate(con, 50);
  vm_pitch(con, 50);
  vm_text(con, "and it is a normal again. Good bye!");
  vm_close(con);
  return 0;
}
