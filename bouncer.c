/*
bouncer module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "bouncer.h"

#define BUFFER_LENGTH 256

void bouncer_process(int connectfd, config *config)
{
  char buf[BUFFER_LENGTH + 1];
  ssize_t n;
  int ret, maxfddp1, srv_enable, cli_enable;
  struct addrinfo ga_hints, *ga_res, *ga_ite;
  int remotefd;
  fd_set readfdset;
  struct sockaddr_in *remoteaddr;

  buf[BUFFER_LENGTH] = '\0';

  memset(&ga_hints, 0, sizeof(ga_hints));
  ga_hints.ai_flags = AI_ADDRCONFIG;
  ga_hints.ai_family = AF_INET;
  ga_hints.ai_socktype = SOCK_STREAM;

  if ((ret = getaddrinfo(config->server_hostname, config->server_port , &ga_hints, &ga_res)) != 0) {
    fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(ret));
    return;
  }

  for (ga_ite = ga_res; ga_ite != NULL; ga_ite = ga_ite->ai_next) {
    remotefd = socket(ga_ite->ai_family, ga_ite->ai_socktype, 0);
    if (remotefd == -1) {
      continue;
    }
    if (connect(remotefd, ga_ite->ai_addr, ga_ite->ai_addrlen) == 0) {
      break;
    }
    perror("connect error");
    close(remotefd);
  }

  freeaddrinfo(ga_res);

  if (ga_ite == NULL) {
    fprintf(stderr, "no available server\n");
    return;
  }

  FD_ZERO(&readfdset);
  maxfddp1 = (connectfd > remotefd ? connectfd : remotefd) + 1;

  srv_enable = 1;
  cli_enable = 1;

  while ( srv_enable || cli_enable ) {
    if (srv_enable) {
      FD_SET(remotefd, &readfdset);
    }

    if (cli_enable) {
      FD_SET(connectfd, &readfdset);
    }

    if ((ret = select(maxfddp1, &readfdset, NULL, NULL, NULL)) == -1) {
      if (ret == EINTR) {
	continue;
      }
    }

    if (FD_ISSET(remotefd, &readfdset)) {
      switch (n = read(remotefd, buf, sizeof(buf))) {
      case -1:
	if (errno == EINTR) {
	  continue;
	}
      case 0:
	srv_enable = 0;
	FD_CLR(remotefd, &readfdset);
	shutdown(connectfd, SHUT_WR);
	break;
      default:
	buf[n] = '\0';
	printf("> %3ld, %s", n, buf);
	write(connectfd, buf, n);
      }
    }

    if (FD_ISSET(connectfd, &readfdset)) {
      switch (n = read(connectfd, buf, sizeof(buf))) {
      case -1:
	if (errno == EINTR) {
	  continue;
	}
      case 0:
	cli_enable = 0;
	FD_CLR(connectfd, &readfdset);
	shutdown(remotefd, SHUT_WR);
	break;
      default:
	buf[n] = '\0';
	printf("< %3ld, %s", n, buf);
	write(remotefd, buf, n);
      }
    }
  }
  close(remotefd);
  return;
}
