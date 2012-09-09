/*
  This program must say some text and produce several sounds.
  To compile it run 'gcc -oexample2 example2.c -lvmclient'.
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
  vm_text(con, "Now we will make some beeps");
  vm_tone(con, 150, 200);
  vm_tone(con, 450, 200);
  vm_tone(con, 450, 200);
  vm_tone(con, 750, 200);
  vm_close(con);
  return 0;
}
