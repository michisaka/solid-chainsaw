/*
bouncer module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "bouncer.h"

#define BUFFER_LENGTH 256

void bouncer_process(int connectfd, config *config)
{
  char buf[BUFFER_LENGTH + 1];
  ssize_t n;

  buf[BUFFER_LENGTH] = '\0';

  while ((n = read(connectfd, buf, sizeof(buf))) > 0) {
    if (n == -1 && errno == EINTR) {
      continue;
    }
    buf[n] = '\0';
    printf("%ld, %s", n, buf);
    write(connectfd, buf, n);
  }
  return;
}
