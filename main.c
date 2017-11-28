/*
main module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#define LOCAL_PORT 60000

int main(int argc, char **argv)
{
  int listenfd;
  struct sockaddr_in listenaddr;

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(EXIT_FAILURE);
  }

  memset(&listenaddr, 0, sizeof(listenaddr));
  listenaddr.sin_family = AF_INET;
  listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  listenaddr.sin_port = htons(LOCAL_PORT);

  if ((bind(listenfd, (struct sockaddr*)&listenaddr, sizeof(listenaddr))) == -1) {
    perror("bind error");
    exit(EXIT_FAILURE);
  }

  if ((listen(listenfd, 5)) == -1) {
    perror("listen error");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
