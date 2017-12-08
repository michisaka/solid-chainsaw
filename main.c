/*
main module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>

#include "config.h"
#include "bouncer.h"

typedef void Sigfunc(int);

Sigfunc *setup_signal(int signo, Sigfunc *func);
void sigchld_handler(int signo);


int main(int argc, char **argv)
{
  config config;
  struct addrinfo ga_hints, *ga_res, *ga_ite;
  int listenfd, connectfd;
  struct sockaddr_in clientaddr;
  socklen_t len;
  pid_t child_pid;
  int ret;
  char *bind_address;

  build_config(argc, argv, &config);

  memset(&ga_hints, 0, sizeof(ga_hints));
  ga_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICHOST;
  ga_hints.ai_family = AF_INET;
  ga_hints.ai_socktype = SOCK_STREAM;

  if (strlen(config.bind_address)) {
    bind_address = config.bind_address;
  } else {
    bind_address = NULL;
  }

  if ((ret = getaddrinfo(bind_address, config.bind_port , &ga_hints, &ga_res)) != 0) {
    fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  for (ga_ite = ga_res; ga_ite != NULL; ga_ite = ga_ite->ai_next) {
    listenfd = socket(ga_ite->ai_family, ga_ite->ai_socktype, 0);
    if (listenfd == -1) {
      continue;
    }
    if ((bind(listenfd, ga_ite->ai_addr, ga_ite->ai_addrlen)) == 0) {
      break;
    }
    perror("bind error");
    close(listenfd);
  }

  freeaddrinfo(ga_res);

  if (ga_ite == NULL) {
    fprintf(stderr, "no available server\n");
    exit(EXIT_FAILURE);
  }

  if ((listen(listenfd, 5)) == -1) {
    perror("listen error");
    exit(EXIT_FAILURE);
  }

  setup_signal(SIGCHLD, sigchld_handler);
  setup_signal(SIGPIPE, SIG_IGN);

  for (;;) {
    len = sizeof(clientaddr);
    if ((connectfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len)) == -1) {
      if (errno == EINTR) {
	continue;
      }
      perror("accept error");
      exit(EXIT_FAILURE);
    }

    child_pid = fork();
    switch (child_pid) {
    case -1:
      perror("fork error");
      exit(EXIT_FAILURE);
    case 0:
      close(listenfd);
      bouncer_process(connectfd, &config);
      close(connectfd);
      exit(EXIT_SUCCESS);
    default:
      close(connectfd);
    }
  }

  exit(EXIT_SUCCESS);
}

Sigfunc *setup_signal(int signo, Sigfunc *func)
{
  struct sigaction	act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
  } else {
#ifdef	SA_RESTART
    act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
  }
  if (sigaction(signo, &act, &oact) < 0)
    return(SIG_ERR);
  return(oact.sa_handler);
}

void sigchld_handler(int signo)
{
  pid_t pid;
  int stat;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    ;
  }

  return;
}
